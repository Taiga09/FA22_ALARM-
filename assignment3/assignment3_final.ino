#include "M5CoreInk.h"
#include <Adafruit_NeoPixel.h>
#include "TimeFunctions.h"

Ink_Sprite PageSprite(&M5.M5Ink);

RTC_TimeTypeDef RTCtime, RTCTimeSave;
RTC_TimeTypeDef AlarmTime;
uint8_t second = 0, minutes = 0;

//Colors
#define LED_PIN    26
#define LED_COUNT 18
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
const int GREEN = strip.Color(0,255,0);
const int YELLOW = strip.Color(128,127,0,0);
const int ORANGE = strip.Color(192,63,0,0);
const int RED = strip.Color(255,0,0,0);
int my_colors[] = {GREEN,YELLOW,ORANGE,RED,GREEN,YELLOW,ORANGE,RED,GREEN,YELLOW,ORANGE,RED};

const int STATE_DEFAULT = 0;
const int STATE_EDIT_HOURS = 1;
const int STATE_EDIT_MINUTES = 2;
const int STATE_ALARM = 4;
const int STATE_ALARM_FINISHED = 5;
int program_state = STATE_DEFAULT;

unsigned long rtcTimer = 0;
unsigned long underlineTimer = 0;
bool underlineOn = false;

unsigned long ledBlinkTimer = 0;
bool ledBlinkOn = false;

const int sensorPin = 25;  // 4-wire bottom connector input pin used by M5 units
int sensorVal = 0;
int proximityVal = 0;
int brightnessVal = 100;
unsigned long sensorTimer = 0;
unsigned long beepTimer = 0;
unsigned long ledTimer = 0;

int ledCounter = 0;

//const int rgbledPin = 32; 
/*
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(
    3, // number of LEDs
    rgbledPin, // pin number
    NEO_GRB + NEO_KHZ800);  // LED type
*/

void setup() {
  M5.begin(false,false,true);
  strip.begin();
  pinMode(sensorPin, INPUT);
  //pinMode(rgbledPin, OUTPUT);
  Serial.begin(9600);

  M5.rtc.GetTime(&RTCTimeSave);
  AlarmTime = RTCTimeSave;
  AlarmTime.Minutes = AlarmTime.Minutes + 2;  // set alarm 2 minutes ahead 
  beepTimer = millis();
  ledTimer = millis();
  M5.update();
  
  M5.M5Ink.clear();
  delay(500);

  checkRTC();
  PageSprite.creatSprite(0, 0, 200, 200);
  drawTime();
  /*
  pixels.begin();           // initialize NeoPixel strip object 
  pixels.show();            // turn OFF all pixels 
  pixels.setBrightness(50); // set brightness to about 1/5 (max = 255)
  */
}

void loop() {
  // check if data has been received on the Serial port:
  if(Serial.available() > 0)
  {
    // input format is "hh:mm" 
    char input[6];
    int charsRead = Serial.readBytes(input, 6);    // read 6 characters 
    if(charsRead == 6 && input[2] == ':') {
      int mm = int(input[4] - '0') + int(input[3] - '0')*10; 
      Serial.print("minutes: ");
      Serial.println(mm);
      int hr = int(input[1] - '0') + int(input[0] - '0')*10; 
      Serial.print("hours: ");
      Serial.println(hr);
      RTCtime.Minutes = mm;
      RTCtime.Hours = hr;
      M5.rtc.SetTime(&RTCtime);      
    }
    else {
      Serial.print("received wrong time format.. ");
      Serial.println(input);
    }
  }

  //proximity sensor
  sensorVal = analogRead(sensorPin);
  proximityVal = map(sensorVal, 0, 4095, 0, 255);
  Serial.print("proximityVal = ");
  Serial.println(proximityVal);

  if( program_state == STATE_DEFAULT) {
    // state behavior: check and update time every second
    if(millis() > rtcTimer + 1000) {
      updateTime();
      drawTime();
      drawTimeToAlarm();      
      rtcTimer = millis();
    }
    /*
    // state behavior: read sensor and print its value to Serial port
    if(millis() > sensorTimer + 100) {
      sensorVal = analogRead(sensorPin);
      brightnessVal = map(sensorVal, 0, 4095, 0, 255);
      Serial.println(brightnessVal);
      sensorTimer = millis();    
    }
    // (OPTIONAL) state behavior: change RGB LEDs green level according to sensor value:
    for( int i=0; i<3; i++) {
      pixels.setPixelColor(i, pixels.Color(0, brightnessVal, 0)); 
      pixels.show(); 
    }
    */

    // state transition: alarm time equals real time clock 
    if(AlarmTime.Hours == RTCtime.Hours && AlarmTime.Minutes == RTCtime.Minutes) {
      program_state = STATE_ALARM;
      Serial.println("program_state => STATE_ALARM");
    }
    
    // state transition: MID button 
    if ( M5.BtnMID.wasPressed()) {
      AlarmTime = RTCtime;
      program_state = STATE_EDIT_MINUTES;
      Serial.println("program_state => STATE_EDIT_MINUTES");
    }
   
  }
    
  else if( program_state == STATE_EDIT_MINUTES) {
    // state behavior: blink underline under alarm minutes
    if( millis() > underlineTimer + 250 ) {
      underlineOn = !underlineOn;
      PageSprite.drawString(30, 20, "SET ALARM MINUTES:");
      drawAlarmTime();      
      if(underlineOn)
        PageSprite.FillRect(110, 110, 80, 6, 0); // underline minutes black
      else
        PageSprite.FillRect(110, 110, 80, 6, 1); // underline minutes white
      PageSprite.pushSprite();
      underlineTimer = millis();
    }
    // state transition: UP button to increase alarm minutes
    if( M5.BtnUP.wasPressed()) {
      Serial.println("BtnUP wasPressed!");
      if(AlarmTime.Minutes < 59) {
        AlarmTime.Minutes++;
        drawAlarmTime();
      }        
    }
    // another state transition: DOWN button to decrease alarm minutes
    else if( M5.BtnDOWN.wasPressed()) {
      Serial.println("BtnDOWN wasPressed!");
      if(AlarmTime.Minutes > 0) {
        AlarmTime.Minutes--;
        drawAlarmTime();
      }        
    }
    // another state transition: MID button to edit alarm hour
    else if (M5.BtnMID.wasPressed()) {
      PageSprite.FillRect(110, 110, 80, 6, 1); // underline minutes white
      program_state = STATE_EDIT_HOURS;
      Serial.println("program_state => STATE_EDIT_HOURS");
    }
  }
  else if(program_state == STATE_EDIT_HOURS) {
    // state behavior: blink underline under alarm hours
    if(millis() > underlineTimer + 250) {
      PageSprite.drawString(30, 20, " SET ALARM HOURS: ");
      underlineOn = !underlineOn;
      drawAlarmTime();
      if(underlineOn)
        PageSprite.FillRect(10, 110, 80, 6, 0); // underline hours black
      else
        PageSprite.FillRect(10, 110, 80, 6, 1); // underline hours white
      PageSprite.pushSprite();
      underlineTimer = millis();
    }
    // state behavior: increase alarm hour with UP button
    if( M5.BtnUP.wasPressed()) {
      Serial.println("BtnUP wasPressed!");
      if(AlarmTime.Hours < 23) {
        AlarmTime.Hours++;
        drawAlarmTime();
      }        
    }
    // state behavior: decrease alarm hour with DOWN button 
    else if( M5.BtnDOWN.wasPressed()) {
      Serial.println("BtnDOWN wasPressed!");
      if(AlarmTime.Hours > 0) {
        AlarmTime.Hours--;
        drawAlarmTime();
      }        
    }
    // state transition: MID button to go back to default state 
    else if (M5.BtnMID.wasPressed()) {
      //PageSprite.FillRect(10, 110, 80, 6, 1); // underline hours white
      M5.M5Ink.clear();
      PageSprite.clear(CLEAR_DRAWBUFF | CLEAR_LASTBUFF);
      M5.rtc.GetTime(&RTCtime);
      drawTime();
      
      program_state = STATE_DEFAULT;
      Serial.println("program_state => STATE_DEFAULT");
    }
  }
  else if( program_state == STATE_ALARM) {
    // state behavior: check and update time every second
    if(millis() > rtcTimer + 1000) {
      updateTime();
      drawTime();
      drawTimeToAlarm();      
      rtcTimer = millis();
    }

    //beep sound 
    if (millis()>beepTimer + 800){
      M5.Speaker.setBeep(500,500);
      M5.Speaker.beep();
      beepTimer = millis();
    }
    
    //strip led animation
    if (millis() > ledTimer + 200) { //350) {
        int myColorValue1 = my_colors[ledCounter];
        int myColorValue2 = my_colors[ledCounter+1];
        int myColorValue3 = my_colors[ledCounter+2];
        strip.setPixelColor(0, myColorValue1);//pixels.Color(myColorValue));
        strip.setPixelColor(1, myColorValue2);//pixels.Color(myColorValue));
        strip.setPixelColor(2, myColorValue3);//pixels.Color(myColorValuel));
        strip.setPixelColor(3, myColorValue1);//pixels.Color(myColorValuel));
        strip.setPixelColor(4, myColorValue2);//pixels.Color(myColorValuel));
        strip.setPixelColor(5, myColorValue3);//pixels.Color(myColorValuel));
        strip.setPixelColor(7, myColorValue1);//pixels.Color(myColorValuel));
        strip.setPixelColor(8, myColorValue2);//pixels.Color(myColorValuel));
        strip.setPixelColor(9, myColorValue3);//pixels.Color(myColorValuel));
        strip.setPixelColor(10, myColorValue1);//pixels.Color(myColorValuel));
        strip.setPixelColor(11, myColorValue2);//pixels.Color(myColorValuel));
        strip.setPixelColor(12, myColorValue3);//pixels.Color(myColorValuel));
        strip.setPixelColor(13, myColorValue1);//pixels.Color(myColorValuel));
        strip.setPixelColor(14, myColorValue2);//pixels.Color(myColorValuel));
        strip.setPixelColor(15, myColorValue3);//pixels.Color(myColorValuel));
        strip.setPixelColor(16, myColorValue1);//pixels.Color(myColorValuel));
        strip.setPixelColor(17, myColorValue2);//pixels.Color(myColorValuel));
        strip.setPixelColor(18, myColorValue3);//pixels.Color(myColorValuel));
        strip.show();
        
        if(ledCounter < 11)
           ledCounter++;
         else
           ledCounter = 0;
      ledTimer = millis();
    }
    
    //proximity sensor
    sensorVal = analogRead(sensorPin);
    proximityVal = map(sensorVal, 0, 4095, 0, 255);
    Serial.print("proximityVal = ");
    Serial.println(proximityVal);
    
    // state transition: hold a hand for 7 secs to finish alarm
    if(proximityVal > 120){
      if (millis() > sensorTimer + 6000) {
        Serial.println("Alarm is off!!");
        M5.M5Ink.clear();
        PageSprite.clear(CLEAR_DRAWBUFF | CLEAR_LASTBUFF);
        program_state = STATE_ALARM_FINISHED;
        Serial.println("program_state => STATE_ALARM_FINISHED");
        
        //turn of the light
        strip.setPixelColor(0, 0);
        strip.setPixelColor(1, 0);
        strip.setPixelColor(2, 0);
        strip.setPixelColor(3, 0);
        strip.setPixelColor(4, 0);
        strip.setPixelColor(5, 0);
        strip.setPixelColor(7, 0);
        strip.setPixelColor(8, 0);
        strip.setPixelColor(9, 0);
        strip.setPixelColor(10, 0);
        strip.setPixelColor(11, 0);
        strip.setPixelColor(12, 0);
        strip.setPixelColor(13, 0);
        strip.setPixelColor(14, 0);
        strip.setPixelColor(15, 0);
        strip.setPixelColor(16, 0);
        strip.setPixelColor(17, 0);
        strip.setPixelColor(18, 0);
        strip.show();
      }
    }
    
    M5.update();
  }
  
  
  else if(program_state == STATE_ALARM_FINISHED) {
    // state behavior: check and update time every second
    if(millis() > rtcTimer + 1000) {
      updateTime();
      drawTime();
      PageSprite.drawString(50, 120, "ALARM IS OFF");
      PageSprite.pushSprite();    
      rtcTimer = millis();
    }
    // state transition: MID button 
    if ( M5.BtnMID.wasPressed()) {
      AlarmTime = RTCtime;
      program_state = STATE_EDIT_MINUTES;
      Serial.println("program_state => STATE_EDIT_MINUTES");
    }
  }
  
  M5.update();
}