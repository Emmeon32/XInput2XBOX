#include <Wire.h>

// Incoming data is in the format "[MSG_TYPE] [LTX] [LTY] [RTX] [RTY] [LT] [RT] [BUTTONS]"
// MSG_TYPE is a single char

// dpad
const unsigned int DPAD_UP = 0x1;
const unsigned int DPAD_DOWN = 0x2;
const unsigned int DPAD_LEFT = 0x4;
const unsigned int DPAD_RIGHT = 0x8;

// buttons
const unsigned int START = 0x10;
const unsigned int BACK  = 0x20;
const unsigned int LT_CLICK = 0x40;
const unsigned int RT_CLICK = 0x80;
const unsigned int LEFT_BUMPER = 0x100;
const unsigned int RIGHT_BUMPER = 0x200;
const unsigned int A_BUTTON = 0x1000;
const unsigned int B_BUTTON = 0x2000;
const unsigned int X_BUTTON = 0x4000;
const unsigned int Y_BUTTON = 0x8000;
const unsigned int OP_RECEIVE = 0;
const unsigned int OP_TRANSMIT = 1;

char bufLTX[4][4] = {"\n\n\n\n", "\n\n\n\n", "\n\n\n\n", "\n\n\n\n"};
char bufLTY[4][4] = {"\n\n\n\n", "\n\n\n\n", "\n\n\n\n", "\n\n\n\n"};
char bufRTX[4][4] = {"\n\n\n\n", "\n\n\n\n", "\n\n\n\n", "\n\n\n\n"};
char bufRTY[4][4] = {"\n\n\n\n", "\n\n\n\n", "\n\n\n\n", "\n\n\n\n"};
char bufLT [4][4] = {"\n\n\n\n", "\n\n\n\n", "\n\n\n\n", "\n\n\n\n"};
char bufRT [4][4] = {"\n\n\n\n", "\n\n\n\n", "\n\n\n\n", "\n\n\n\n"};
char bufButtons[4][6] = {"\n\n\n\n\n\n", "\n\n\n\n\n\n", "\n\n\n\n\n\n", "\n\n\n\n\n\n"};

int momentaryButtons = 0;
int pIdx;
int bufIdx = 0;
int bufSubIdx = 0;
int operationMode = 0;
const int sclPin = 4;
const int sdaPin = 5;
bool receiving = false;
byte emulatorStatuses;

// DEBUG
unsigned int debugReceivedCount = 0;
int testPin = 3;
int debugInputIdx = 0;
bool dPadPressedLastFrame = false;

void setup() {
  Wire.begin();
  Serial.begin(115200);    // Initialize UART
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);

  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(13, OUTPUT);
  
  //Serial.print("Initialised.");
}

void loop()
{
}

// SerialEvent occurs every time we receive RX interrupt
void serialEvent() {
  if(operationMode != OP_RECEIVE)
  {
    return;
  }

  char c;
  
  while (Serial.available()) {
    int parsedInt;
    
    c = (char)Serial.read();  // Read character

    if(isDigit(c))
    {
      // If we never recieved the header, ignore this RX to protect the integrity
      // of the buffers
      if(!receiving)
        return;
      
      switch(bufIdx)
      {
        case 1: // LX
          bufLTX[pIdx][bufSubIdx] = c;
          break;
        case 2: // LY
          bufLTY[pIdx][bufSubIdx] = c;
          break;
        case 3: // RX
          bufRTX[pIdx][bufSubIdx] = c;
          break;
        case 4: // RY
          bufRTY[pIdx][bufSubIdx] = c;
          break;
        case 5: // LT
          bufLT[pIdx][bufSubIdx] = c;
          break;
        case 6: // RT
          bufRT[pIdx][bufSubIdx] = c;
          break;
        case 7: // Buttons
          bufButtons[pIdx][bufSubIdx] = c;
          break;
      }
      
      bufSubIdx++;
      if((bufIdx < 7 && bufSubIdx >= 3) || (bufSubIdx >= 5) )
      {
        bufSubIdx = 0;
        bufIdx++;
        if(bufIdx > 7)
        {
          bufIdx = 0;
          //receiving = false; // stop receiving as we reached the end of the packet
        }
      }
    }else if(isAlpha(c))
    {
        // This is the index of the player
        pIdx = (int)(((byte)c) - ((byte)'a'));
        //pIdx = 1;
        bufIdx = 1;
        bufSubIdx = 0;
        receiving = true;
    }
    
    // Newline means we've reached the end of the buffer. Reset the indices
    if (receiving && c == '\n') {
      receiving = false;
      onInputUpdated(pIdx);
      //Serial.print(pIdx + " OK\n");
      bufIdx = 0;
      bufSubIdx = 0;
    }
  }
}

void onInputUpdated(int gamepad)
{
  
  if(gamepad == 1){
    digitalWrite(13, HIGH);
  }else
  {
    digitalWrite(13, LOW);
  }
  // Forward input to the Leos via i2c
  Wire.beginTransmission(8 + gamepad*2);
  
  Wire.write('a' + (byte)gamepad);
  Wire.write((byte)atoi((const char*)&bufLTX[gamepad]));
  Wire.write((byte)atoi((const char*)&bufLTY[gamepad]));
  Wire.write((byte)atoi((const char*)&bufRTX[gamepad]));
  Wire.write((byte)atoi((const char*)&bufRTY[gamepad]));
  Wire.write((byte)atoi((const char*)&bufLT [gamepad]));
  Wire.write((byte)atoi((const char*)&bufRT [gamepad]));
  
  int buttons = atoi((const char*)&bufButtons[gamepad]);
  
  Wire.write((byte)((buttons >> 8)));
  Wire.write((byte)(((buttons << 8) >> 8)));
  
  Wire.endTransmission();
  /*
  Wire.beginTransmission(1);
  Wire.write((byte)atoi(bufRTX[0]));
  Wire.endTransmission();*/
  
}
