//
// CH559 USB host helper on M5Stack
// 

#include <M5Stack.h>

// #include "GFX.h"
// #include "ST7789.h"
// #include <SPI.h>


// Arduino_ST7789 tft = Arduino_ST7789(22, 21, -1, -1);

// LCD 320x240

#define MAX_X 320
#define MAX_Y 240

uint8_t  uartRxBuff[1024];
int  rxPos = 0;
int  cmdLength = 0;
uint8_t  cmdType = 0;
long lastRxReceive = 0;

int x = 160, y = 120, oldx, oldy, shift, newkey, oldkey, button, button1;

String deviceType[] = {"UNKNOWN", "POINTER", "MOUSE", "RESERVED", "JOYSTICK", "GAMEPAD", "KEYBOARD", "KEYPAD", "MULTI_AXIS", "SYSTEM"};
String keyboardstring;
void setup(void) {
  M5.begin();

  M5.Power.begin();

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
  Serial.begin(115200);
  Serial2.begin(1000000); // , SERIAL_8N1, 16, 17);
  Serial.println("OK There");

  // tft.init(240, 240);
  clearscreen();

}

void clearscreen() {
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.fillScreen(GREEN);
  M5.Lcd.setTextColor(BLACK, GREEN);
  M5.Lcd.setTextWrap(true);
  M5.Lcd.println("USB Host Example");
  // M5.Lcd.println("");
  // M5.Lcd.println("X:   , Y:");
  // M5.Lcd.println("");
  // M5.Lcd.println("Y:");
}

void loop() {
  while (Serial2.available())
  {
    lastRxReceive = millis();
    //Serial.print("h0x");//Only for Debug
    //Serial.print(Serial2.peek(),HEX);//Only for Debug
    //Serial.print(" ");//Only for Debug
    uartRxBuff[rxPos] = Serial2.read();

    if (rxPos == 0 && uartRxBuff[rxPos] == 0xFE) { // 0xFE: message start mark
      cmdType = 1;
    } else if (rxPos == 1 && cmdType == 1) {
      cmdLength = uartRxBuff[rxPos];
    } else if (rxPos == 2 && cmdType == 1) {
      cmdLength += (uartRxBuff[rxPos] << 8);
      //printf( "Length: %i\n", cmdLength);//Only for Debug
    } else if (cmdType == 0 && uartRxBuff[rxPos] == '\n') {
      printf("No COMMAND Received\n");
      for (uint8_t i = 0; i < rxPos; i ++ )
      {
        printf( "0x%02X ", uartRxBuff[i]);
      }
      printf("\n");
      rxPos = 0;
      cmdType = 0;
    }

    if (rxPos > 0 && rxPos == cmdLength + 11 && cmdType || rxPos > 1024) {
      // full msgbuffer
      filterCommand(cmdLength, uartRxBuff);
      for (int i = 0; i < rxPos; i ++ )
      {
        //printf( "0x%02X ", uartRxBuff[i]);//Only for Debug
      }
      //printf("\n");//Only for Debug
      rxPos = 0;
      cmdType = 0;
    } else {
      // receive message
      rxPos++;
    }

  }
  rxPos = 0;

  if (Serial.available())
  {
    Serial2.write(Serial.read());
  }
  if (button1) {
    clearscreen();
    button1 = 0;
  }

  if (x != oldx || y != oldy) {
    M5.Lcd.fillTriangle(oldx, oldy, oldx + 6, oldy + 3, oldx + 3, oldy + 6, GREEN);
    M5.Lcd.fillTriangle(x, y, x + 6, y + 3, x + 3, y + 6, BLACK);
    if (button)M5.Lcd.drawLine(oldx - 1, oldy - 1, x - 1, y - 1, BLACK);
#if 1
    M5.Lcd.setCursor(0, 16);
    M5.Lcd.print("X:"); M5.Lcd.print(x); M5.Lcd.print("   ");
    M5.Lcd.setCursor(80, 16);
    M5.Lcd.print("Y:"); M5.Lcd.print(y); M5.Lcd.print("   ");
#else
    M5.Lcd.setCursor(16, 16);
    M5.Lcd.print(x);
    M5.Lcd.println("  ");
    M5.Lcd.setCursor(16, 32);
    M5.Lcd.print(y);
    M5.Lcd.println("  ");
#endif
    oldx = x;
    oldy = y;
  }
  if (newkey != oldkey) {
    if (newkey != 0) {
      if (newkey == 0x2c) {
        keyboardstring += ' ';
      } else if (newkey == 0x36) {
        keyboardstring += ',';
      } else if (newkey == 0x37) {
        keyboardstring += '.';
      } else if (newkey == 0x38) {
        keyboardstring += '-';
      } else if (newkey == 0x30) {
        keyboardstring += '+';
      } else if (newkey == 0x28) {
        keyboardstring += '\n';
      } else if (newkey == 0x2A) {
        keyboardstring = keyboardstring.substring(0, keyboardstring.length() - 1);
      } else
        keyboardstring += char((unsigned char)newkey + (shift ? 61 : 93));
      M5.Lcd.setCursor(0, 160);
      M5.Lcd.print(keyboardstring);
      M5.Lcd.print(" ");
    }
    oldkey = newkey;
  }
}

#define MSG_TYPE_CONNECTED      0x01
#define MSG_TYPE_DISCONNECTED   0x02
#define MSG_TYPE_ERROR          0x03
#define MSG_TYPE_DEVICE_POLL    0x04
#define MSG_TYPE_DEVICE_STRING  0x05
#define MSG_TYPE_DEVICE_INFO    0x06
#define MSG_TYPE_HID_INFO       0x07
#define MSG_TYPE_STARTUP        0x08



void filterCommand(int buffLength, unsigned char *msgbuffer) {
  int cmdLength = buffLength;
  unsigned char msgType = msgbuffer[3];
  unsigned char devType = msgbuffer[4];
  unsigned char device = msgbuffer[5];
  unsigned char endpoint = msgbuffer[6];
  unsigned char idVendorL = msgbuffer[7];
  unsigned char idVendorH = msgbuffer[8];
  unsigned char idProductL = msgbuffer[9];
  unsigned char idProductH = msgbuffer[10];

  switch (msgType) {
  case MSG_TYPE_CONNECTED:
    Serial.print("Device Connected on port");
    Serial.println(device);
    break;
  case MSG_TYPE_DISCONNECTED:
    Serial.print("Device Disconnected on port");
    Serial.println(device);
    break;
  case MSG_TYPE_ERROR:
    Serial.print("Device Error ");
    Serial.print(device);
    Serial.print(" on port ");
    Serial.println(devType);
    break;
  case MSG_TYPE_DEVICE_POLL: // sendHidPollMsg
#if 1
    Serial.print("Device HID Data from port: ");
    Serial.print(device);
    Serial.print(" , Length: ");
    Serial.print(cmdLength);
    Serial.print(" , Type: ");
    Serial.print (deviceType[devType]);
    Serial.print(" , ID: ");
    for (int j = 0; j < 4; j++) {
      Serial.print("0x");
      Serial.print(msgbuffer[j + 7], HEX);
      Serial.print(" ");
    }
    Serial.print(" ,  ");
    for (int j = 0; j < cmdLength; j++) {
      Serial.print("0x");
      Serial.print(msgbuffer[j + 11], HEX);
      Serial.print(" ");
    }
    Serial.println();
#endif
    if (devType == 2) {
      // mouse ?
      x += (int8_t)msgbuffer[11 + 2]; // ((uint8_t)msgbuffer[11 + 2] + ((uint8_t)msgbuffer[11 + 3] << 8));
      y += (int8_t)msgbuffer[11 + 3]; // ((uint8_t)msgbuffer[11 + 4] + ((uint8_t)msgbuffer[11 + 5] << 8));
      if (x > MAX_X) x = MAX_X;
      if (x < 0) x = 0;
      if (y > MAX_Y) y = MAX_Y;
      if (y < 0) y = 0;
      if (msgbuffer[11 + 1] & 1) {
	button = 1;
      } else button = 0;
      if (msgbuffer[11 + 1] & 2) {
	button1 = 1;
      } else button1 = 0;
    }
    if (devType == 6) {
      // keyboard ?
      if (msgbuffer[11 + 0] == 2) shift = 1;
      else shift = 0;
      newkey = msgbuffer[11 + 2];
    }
    //Serial.println();
    //Serial.println("X= " + String(x) + " Y= " + String(y));
    break;
  case MSG_TYPE_DEVICE_STRING:
    Serial.print("Device String port ");
    Serial.print(devType);
    Serial.print(" Name: ");
    for (int j = 0; j < cmdLength; j++)
      Serial.write(msgbuffer[j + 11]);
    Serial.println();
    break;
  case MSG_TYPE_DEVICE_INFO:
    Serial.print("Device info from port");
    Serial.print(device);
    Serial.print(", Descriptor: ");
    for (int j = 0; j < cmdLength; j++) {
      Serial.print("0x");
      Serial.print(msgbuffer[j + 11], HEX);
      Serial.print(" ");
    }
    Serial.println();
    break;
  case MSG_TYPE_HID_INFO:
    Serial.print("HID info from port");
    Serial.print(device);
    Serial.print(", Descriptor: ");
    for (int j = 0; j < cmdLength; j++) {
      Serial.print("0x");
      Serial.print(msgbuffer[j + 11], HEX);
      Serial.print(" ");
    }
    Serial.println();
    break;
  case MSG_TYPE_STARTUP:
    Serial.println("USB host ready");
    break;

  }
}
