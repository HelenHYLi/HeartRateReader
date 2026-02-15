#include <LedControl.h>

#include "DFRobot_Heartrate.h"
#define heartratePin A1

#include <SPI.h>
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#define RST_PIN -1
#define RTN_CHECK 1
#define I2C_ADDRESS 0x3C

// OLED Display
SSD1306AsciiWire display;
String message[] = {"Heart Rate <3", "Your signals are too weak :/", "Your phase difference \n is zero", "Your heart is in the \n saturation region"};

// Heart Rate
DFRobot_Heartrate heartrate(DIGITAL_MODE);   // ANALOG_MODE or DIGITAL_MODE
int setHeartRate = 1;     // used for heart blinking
int delayBlink = 10000;    
int empty = 1;            // 1 for empty
bool valid = false;

// Motor
int IN1 = 5;
int IN2 = 4;
int ENA = 6; //PWM

// 8x8 Matrix
int DIN = 12;
int CS = 11;
int CLK = 10;
LedControl lc(DIN, CLK, CS, 1);
byte heart[8] = {
B01110000,
B11111000,
B11111100,
B01111110,
B01111110,
B11111100,
B11111000,
B01110000};

byte emptyHeart[8] = {
B00111000,
B01000100,
B01000010,
B00100001,
B00100001,
B01000010,
B01000100,
B00111000};


void setup() {
  Serial.begin(115200);     // Heart Rate
  lc.shutdown(0, false);    // remove energy saving
  lc.setIntensity (0, 1);   // set 0-15 chose 8
  lc.clearDisplay(0);
  DisplayBytes(emptyHeart);

  //Motor
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);

  // OLED Display
  Wire.begin();
  Wire.setClock(400000L);
  #if RST_PIN >= 0
  display.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
  #else // RST_PIN >= 0
  display.begin(&Adafruit128x64, I2C_ADDRESS);
  #endif // RST_PIN >= 0
  display.setFont(System5x7);
  display.clear();

}

void loop() {
  display.clear();
  display.setCursor(0,0);

  // Get heart rate value every 10 pulses
  int count = 0;
  uint8_t rateValue;
  while(valid == false) {
    heartrate.getValue(heartratePin);   // A1 foot sampled values
    rateValue = heartrate.getRate();   // Get heart rate value
    if(rateValue)  {
      Serial.println(rateValue);
      if (rateValue < 300 && rateValue > 50){
        setHeartRate = rateValue;
        valid = true;
      }
    }
  }
  valid = false;
 
  // Set motor speed, message, and delay speed from current heart rate
  delayBlink =  60000.0 / setHeartRate;
  int motorSpeed;
  int messageNum;
  if (setHeartRate < 50) {
    motorSpeed = 40;
    messageNum = 1;
  } else if (setHeartRate < 100) {
    motorSpeed = 70;
    messageNum = 2;
  } else {
    motorSpeed = 90;
    messageNum = 3;
  }

  display.println(message[0]);  // display heartrate on OLED
  display.println();
  display.println(setHeartRate);
  display.println();
  display.println(message[messageNum]);  // display heartrate on OLED

  // Display 8x8 matrix and start and stop motor
  count = 0;
  while (count < 10) {
    if (empty == 1) {    
     DisplayBytes(heart);
      empty = 0;
    } else {
      DisplayBytes(emptyHeart);
      empty = 1;
    }


    digitalWrite(IN1, HIGH);    // control motor speed
    digitalWrite(IN2, LOW);
    analogWrite(ENA, motorSpeed);
    delay(delayBlink);
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, HIGH);
    count = count + 1;
  }
}


// --------------------------------------------------------------------------------------------------------------------------------------------
void DisplayBytes(byte image[]) {
  for (int i = 0; i <=7; i++){
    lc.setRow(0, i, image[i]);
  }
}
