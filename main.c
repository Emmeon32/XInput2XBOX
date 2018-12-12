/*
     .k8GOGGNqkSFS5XkqXPSkSkkqXXFS5kSkSS15U22F2515U2uujuu1U1u2U1U2uUuFS.
   :0qE     JS5uuJuuFFX51jU2SSk12jU2SSXF5uuu15SFS5k12ujj21S5kFS5S12jJYu11
  5XS:        1UYYLu.   vUUX    U22r     SUF         SUF           ;YYLuU5
 1F5i  NNSkS7  2uLJui   51u     S5.      .PX         .XX           LJvLLu1.
 kUk  0iLk5FFu vuYY2:   5F    Xkk7        78    E0    i0    GEXPXk2uLLvLLuk
X25, 8O   2kX0  5YJUi   M    555    PkXk   i    q1FU   7    ONNkP12YLvLvLYS
S25  8888  888  5uY5         FuS    PS50   .    FuUU   7          uJvLvLLJ2i
kUF             SJjU.      P02UF    P25k   .    Su2Y   v          2LLvLvLL17
S21  XJj88  0u  1uY2.        X2k           .    k11E   v    7;ii:JuJvLvLvJ2:
2257 jqv   Pqq  1LJur         PP.          7    EX:    q    OqqXP51JYvLvYYS.
 X2F  kXkXSXk  kJYLU:   O     ,Z    0PXZ   i    ii    q0    i:::,,.jLLvLLuF'
 ik1k  ;qkPj  .uJvYu:   UN      :   XU2F   :         S5S           iJLLvjUF8
  :PSq       72uLLLui   uSi    .;   2uY1   r.       72j1           LYYLYJSU88
    XqE2   rP12juJuu1FX55U5FqXXSXkXF1juUkkPSXSPXPXPF1Jju5FkFSFXFSF5uujUu5j28V
      .uGOZESS5S5SFkkPkPkXkPXqXPXqXXFkSkkPXPXPkqSkSS1521252121U2u2u12Suv7

*
* Arduino Micro (Leonardo) Classic XBOX Pad emulator firmware
*
* Copyright (c) 2016
* Bruno Freitas - bruno@brunofreitas.com
* Jon Wilson    - degenatrons@gmail.com
* Kevin Mackett - kevin@sharpfork.com
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <avr/io.h>
#include "XBOXPad.h"
#include "I2CSlave.h"
#include "util.h"


#define I2C_ADDR 0x10
#define PAD_IDX 'a'
#define BUT_DPAD_UP 0
#define BUT_DPAD_DOWN 1
#define BUT_DPAD_LEFT 2
#define BUT_DPAD_RIGHT 3
#define BUT_START 4
#define BUT_BACK 5
#define BUT_LEFTSTICK 6
#define BUT_RIGHTSTICK 7

#define BUT_LEFTBUMP 1
#define BUT_RIGHTBUMP 2
// two values are skipped here
#define BUT_A 4
#define BUT_B 5
#define BUT_X 6
#define BUT_Y 7



void setup_pins(void);
void twiTest(void);
void I2C_receieved(uint8_t data);
void I2C_requested(void);
void forwardStateBuffer(void);

volatile char stateBuffer[8];


int main(void) {
	uint8_t pad_up, pad_down, pad_left, pad_right, pad_y, pad_b, pad_x, pad_a, pad_black,
	pad_white, pad_start, pad_select, pad_l3, pad_r3, pad_l, pad_r, pad_left_analog_x,
	pad_left_analog_y, pad_right_analog_x, pad_right_analog_y;

	// Set clock @ 16Mhz
	CPU_PRESCALE(0);

	// Disable JTAG
	bit_set(MCUCR, 1 << JTD);
	bit_set(MCUCR, 1 << JTD);

	// Setup pins
	//setup_pins();

	// Init XBOX pad emulation
	xbox_init(true);
	
	twiTest();


	// Pins polling and gamepad status updates
	for (;;) {
		xbox_reset_watchdog();

		pad_up = !bit_check(PINC, 7);
		pad_down = !bit_check(PINB, 2);
		pad_left = !bit_check(PINB, 0);
		pad_right = !bit_check(PIND, 3);
		pad_y = !bit_check(PIND, 2);
		pad_b = !bit_check(PIND, 1);
		pad_x = !bit_check(PIND, 0);
		pad_a = !bit_check(PIND, 4);
		pad_black =  !bit_check(PINC, 6);
		pad_white =  !bit_check(PIND, 7);
		pad_start =  !bit_check(PINE, 6);
		pad_select =  !bit_check(PINB, 4);
		pad_l3 =  !bit_check(PINB, 5);
		pad_r3 =  !bit_check(PINB, 6);
		pad_l = !bit_check(PINB, 7);
		pad_r = !bit_check(PIND, 6);

		pad_left_analog_x = pad_left_analog_y = pad_right_analog_x = pad_right_analog_y = 0x7F;

		if(!bit_check(PINB, 1)) {
			pad_left_analog_x = 0x00;
		} else if(!bit_check(PINB, 3)) {
			pad_left_analog_x = 0xFF;
		}

		if(!bit_check(PINF, 0)) {
			pad_left_analog_y = 0x00;
		} else if(!bit_check(PINF, 1)) {
			pad_left_analog_y = 0xFF;
		}

		if(!bit_check(PINF, 4)) {
			pad_right_analog_x = 0x00;
		} else if(!bit_check(PINF, 5)) {
			pad_right_analog_x = 0xFF;
		}

		if(!bit_check(PINF, 6)) {
			pad_right_analog_y = 0x00;
		} else if(!bit_check(PINF, 7)) {
			pad_right_analog_y = 0xFF;
		}

		gamepad_state.a = pad_a * 0xFF;
		gamepad_state.b = pad_b * 0xFF;
		gamepad_state.x = pad_x * 0xFF;
		gamepad_state.y = pad_y * 0xFF;

		gamepad_state.black = pad_black * 0xFF;
		gamepad_state.white = pad_white * 0xFF;

		pad_up    ? bit_set(gamepad_state.digital_buttons, XBOX_DPAD_UP)    : bit_clear(gamepad_state.digital_buttons, XBOX_DPAD_UP);
		pad_down  ? bit_set(gamepad_state.digital_buttons, XBOX_DPAD_DOWN)  : bit_clear(gamepad_state.digital_buttons, XBOX_DPAD_DOWN);
		pad_left  ? bit_set(gamepad_state.digital_buttons, XBOX_DPAD_LEFT)  : bit_clear(gamepad_state.digital_buttons, XBOX_DPAD_LEFT);
		pad_right ? bit_set(gamepad_state.digital_buttons, XBOX_DPAD_RIGHT) : bit_clear(gamepad_state.digital_buttons, XBOX_DPAD_RIGHT);

		pad_start  ? bit_set(gamepad_state.digital_buttons, XBOX_START)       : bit_clear(gamepad_state.digital_buttons, XBOX_START);
		pad_select ? bit_set(gamepad_state.digital_buttons, XBOX_BACK)        : bit_clear(gamepad_state.digital_buttons, XBOX_BACK);
		pad_l3     ? bit_set(gamepad_state.digital_buttons, XBOX_LEFT_STICK)  : bit_clear(gamepad_state.digital_buttons, XBOX_LEFT_STICK);
		pad_r3     ? bit_set(gamepad_state.digital_buttons, XBOX_RIGHT_STICK) : bit_clear(gamepad_state.digital_buttons, XBOX_RIGHT_STICK);

		gamepad_state.l_x = pad_left_analog_x * 257 + -32768;
		gamepad_state.l_y = pad_left_analog_y * -257 + 32767;
		gamepad_state.r_x = pad_right_analog_x * 257 + -32768;
		gamepad_state.r_y = pad_right_analog_y * -257 + 32767;

		gamepad_state.l = pad_l * 0xFF;
		gamepad_state.r = pad_r * 0xFF;

		xbox_send_pad_state();
	}
}

volatile uint8_t recData;

volatile int byteIdx = 0;
volatile bool receiving = false;



void I2C_received(uint8_t data)
{
	recData = data;
	PORTC = 0xFF;
	if((char)data == PAD_IDX)
	{
		// Start receiving:
		byteIdx = 0;
	}else
	{
		if(byteIdx < 8)
			stateBuffer[byteIdx] = data;
		
		byteIdx++;
	}

	if(byteIdx == 8)
	{
		PORTC = 0xFF;
		forwardStateBuffer();
	}
}

void forwardStateBuffer()
{
	// Copy the state buffer to the emulated gamepad
	xbox_reset_watchdog();
	
	// Analog controls
	gamepad_state.l_x = (((int)stateBuffer[0]) * 0xFF) + 0x8000;
	gamepad_state.l_y = (((int)stateBuffer[1]) * 0xFF) - 0x8000;
	gamepad_state.r_x = (((int)stateBuffer[2]) * 0xFF) + 0x8000;
	gamepad_state.r_y = (((int)stateBuffer[3]) * 0xFF) - 0x8000;
	gamepad_state.l   = (int)stateBuffer[4];
	gamepad_state.r   = (int)stateBuffer[5];
	
	// Buttons
	gamepad_state.digital_buttons = stateBuffer[7];
	
	if(stateBuffer[6] & 0x1) { gamepad_state.white = 0xFF; }else{ gamepad_state.white = 0x00; }
	if(stateBuffer[6] & 0x2) { gamepad_state.black = 0xFF; }else{ gamepad_state.black = 0x00; }
	
	if(stateBuffer[6] & 0x10){ gamepad_state.a = 0xFF; }else{ gamepad_state.a = 0x00; }
        if(stateBuffer[6] & 0x20){ gamepad_state.b = 0xFF; }else{ gamepad_state.b = 0x00; }
        if(stateBuffer[6] & 0x40){ gamepad_state.x = 0xFF; }else{ gamepad_state.x = 0x00; }
        if(stateBuffer[6] & 0x80){ gamepad_state.y = 0xFF; }else{ gamepad_state.y = 0x00; }


	xbox_send_pad_state();
}

void I2C_requested()
{
	// transmit the last received byte
	I2C_transmitByte(recData);
}

void twiTest()
{
	DDRC = 0xff;
	PORTC = 0x00;
	I2C_init(8);
	I2C_setCallbacks(I2C_received, I2C_requested);
	
	while(true)
	{
		PORTC = 0xFF;
	}
}





void setup_pins(void) {

	// Setup pins
	bit_clear(DDRF, 1 << 7);
	bit_set(PORTF, 1 << 7);

	bit_clear(DDRF, 1 << 6);
	bit_set(PORTF, 1 << 6);

	bit_clear(DDRF, 1 << 5);
	bit_set(PORTF, 1 << 5);

	bit_clear(DDRF, 1 << 4);
	bit_set(PORTF, 1 << 4);

	bit_clear(DDRF, 1 << 1);
	bit_set(PORTF, 1 << 1);

	bit_clear(DDRF, 1 << 0);
	bit_set(PORTF, 1 << 0);

	bit_clear(DDRB, 1 << 3);
	bit_set(PORTB, 1 << 3);

	bit_clear(DDRB, 1 << 1);
	bit_set(PORTB, 1 << 1);

	bit_clear(DDRD, 1 << 6);
	bit_set(PORTD, 1 << 6);

	bit_clear(DDRB, 1 << 7);
	bit_set(PORTB, 1 << 7);

	bit_clear(DDRB, 1 << 6);
	bit_set(PORTB, 1 << 6);

	bit_clear(DDRB, 1 << 5);
	bit_set(PORTB, 1 << 5);

	bit_clear(DDRB, 1 << 4);
	bit_set(PORTB, 1 << 4);

	bit_clear(DDRE, 1 << 6);
	bit_set(PORTE, 1 << 6);

	bit_clear(DDRD, 1 << 7);
	bit_set(PORTD, 1 << 7);

	bit_clear(DDRC, 1 << 6);
	bit_set(PORTC, 1 << 6);

	bit_clear(DDRD, 1 << 4);
	bit_set(PORTD, 1 << 4);

	bit_clear(DDRD, 1 << 0);
	bit_set(PORTD, 1 << 0);

	bit_clear(DDRD, 1 << 1);
	bit_set(PORTD, 1 << 1);

	bit_clear(DDRD, 1 << 2);
	bit_set(PORTD, 1 << 2);

	bit_clear(DDRD, 1 << 3);
	bit_set(PORTD, 1 << 3);

	bit_clear(DDRB, 1 << 0);
	bit_set(PORTB, 1 << 0);

	bit_clear(DDRB, 1 << 2);
	bit_set(PORTB, 1 << 2);

	bit_clear(DDRC, 1 << 7);
	bit_set(PORTC, 1 << 7);
}
