XInput2XBOX
=====
![License](https://img.shields.io/badge/License-GPLv3-blue.svg)

This project is a modification of XBOXPadMicro to forward xinput data from a windows computer to an Original XBOX.

## Installation

This project has 3 major components. A program to take input data from XInput controllers, a microcontroller to take that (now serialised) data and split it into 4 channels over i2c lines, which go to to up to 4 Atmega32u4 chips emulating XBox classic controllers. Every emulated controller goes to a dedicated port on the XBox.

The setup this was designed for is built with 1 Arduino Uno, acting as the splitter, and 4 Arduino Leonardos. Every leonardo must have a unique .hex file running on it, as every device has a unique I2C address.

### The XInput Relay

The relay program takes in the XInput data and feeds it into the splitter. The host machine must be running Windows 8.1 or higher.
This console program is located in the xbox_uart3 directory.
NOTE: The COM port is hard coded. You may need to edit the setting in program.cpp so that it matches up with the COM port of your MCU.

Once the program is running it will detect any gamepads connected to the host machine. This includes any gamepads from any connected Parsec clients, which is what this project was built for.

### The Splitter

The splitter can be any Arduino MCU with TWI capability. It will run the sketch splitter.eno, located in the main repository directory.
When connected to the correct serial port, the relay program will connect to it and send gamepad poll data to it.

The splitter also communicates with the emulated controllers over I2C. These are analog pins A4 (SDA) and A5 (SCL) on the Arduino Uno.
On the Leonardo, these are pins 2 (SDA) and 3 (SCL).

All SDA pins on all microcontrollers must be linked together, similarly for the SCL line.

NOTE: Two 2k Ohm resistors must also connect each I2C line to +5v. It doesn't really matter if this comes from the splitter or one of the emulated controllers.

### The Emulated Controllers

These trick the XBox into thinking they are actual XBox controllers. An adapter is needed to convert the serial interface on the Arduino Leonardo to an XBox classic USB port.

All of the emulators share the I2C lines to the splitter.



Once everything is connected, each controller will send XInput data to the XBox. Up to 4 gamepads can be connected to the host machine, enabling use of XBox 360/One controllers to play XBox Classic games. This will work with any XInput controllers, though testing has only been done on 360 and One controllers.




## Software Info

If you want to modify the source code, some of the information in here may be helpful.

```
XInput Gamepad | I2C Address | Char Identifier
===============|=============|=================
      0        |    0x10     |       'a'
      1        |    0x12     |       'b'
      2        |    0x14     |       'c'
      3        |    0x16     |       'd'
```

As well as unique addresses, every Leo has a character associated with it. This character must be alphabetical (more specifically, non-numeric), as this distinguishes the start of serialised packets. Packets do not have a length; the 4 Leos will simply collect the bytes trailing their unique header character until their 8-byte buffers are full.

The buffers are arranged in the same order as the descriptor used in actual xbox controllers:


```
Byte | Associated Value
=====|==================
  0  | LEFT STICK  X
  1  | LEFT STICK  Y
  2  | RIGHT STICK X
  3  | RIGHT STICK Y
  4  | LEFT TRIGGER
  5  | RIGHT TRIGGER
  6  | BUTTONS (Digital)
  7  | BUTTONS (Analog)
```
  
