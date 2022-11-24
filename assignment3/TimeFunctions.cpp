#include "TimeFunctions.h"
#include "Numbers_55x40.h"

image_t num55[11] = {
  {40, 55, 1, image_num_01}, {40, 55, 1, image_num_02},
  {40, 55, 1, image_num_03}, {40, 55, 1, image_num_04},
  {40, 55, 1, image_num_05}, {40, 55, 1, image_num_06},
  {40, 55, 1, image_num_07}, {40, 55, 1, image_num_08},
  {40, 55, 1, image_num_09}, {40, 55, 1, image_num_10},
  {24, 55, 1, image_num_11},
};

Ink_Sprite TimeSprite(&M5.M5Ink);

void drawImageToSprite(int posX, int posY, image_t *imagePtr,
    Ink_Sprite *sprite) {
  sprite->drawBuff(posX, posY, imagePtr->width, imagePtr->height,
    imagePtr->ptr);
}

void updateTime() {
  M5.rtc.GetTime(&RTCtime);
  if (minutes != RTCtime.Minutes) {
    M5.rtc.GetTime(&RTCtime);
    //M5.rtc.GetDate(&RTCDate);

    if (RTCtime.Minutes % 10 == 0) {
      M5.M5Ink.clear();
      PageSprite.clear(CLEAR_DRAWBUFF | CLEAR_LASTBUFF);
    }
    //drawTime(&RTCtime);
    drawTime();
    //drawDate(&RTCDate);
    PageSprite.pushSprite();
    minutes = RTCtime.Minutes;
  }
}

// draw time using pointer
void drawTimePtr(RTC_TimeTypeDef *time) {
  drawImageToSprite(10, 48, &num55[time->Hours / 10], &PageSprite);
  drawImageToSprite(50, 48, &num55[time->Hours % 10], &PageSprite);
  drawImageToSprite(90, 48, &num55[10], &PageSprite);
  drawImageToSprite(110, 48, &num55[time->Minutes / 10], &PageSprite);
  drawImageToSprite(150, 48, &num55[time->Minutes % 10], &PageSprite);
}

void drawTime() {
  drawImageToSprite(10, 48, &num55[RTCtime.Hours / 10], &PageSprite);
  drawImageToSprite(50, 48, &num55[RTCtime.Hours % 10], &PageSprite);
  drawImageToSprite(90, 48, &num55[10], &PageSprite);
  drawImageToSprite(110, 48, &num55[RTCtime.Minutes / 10], &PageSprite);
  drawImageToSprite(150, 48, &num55[RTCtime.Minutes % 10], &PageSprite);
  PageSprite.pushSprite();
}

void drawAlarmTime() {
  drawImageToSprite(10, 48, &num55[AlarmTime.Hours / 10], &PageSprite);
  drawImageToSprite(50, 48, &num55[AlarmTime.Hours % 10], &PageSprite);
  drawImageToSprite(90, 48, &num55[10], &PageSprite);
  drawImageToSprite(110, 48, &num55[AlarmTime.Minutes / 10], &PageSprite);
  drawImageToSprite(150, 48, &num55[AlarmTime.Minutes % 10], &PageSprite);
  PageSprite.pushSprite();
}

void drawTimeToAlarm() {
  char buf[8];
  int hr_diff = AlarmTime.Hours - RTCtime.Hours;     
  if( hr_diff < 0) {
    hr_diff = 23 + hr_diff;
  }
  int min_diff = AlarmTime.Minutes - RTCtime.Minutes;
  if( min_diff < 0) {
    min_diff = 59 + min_diff;
    if( hr_diff == 0) {
      hr_diff = 23 + hr_diff;
    }
  }
  sprintf(buf, "%02d:%02d", hr_diff, min_diff);
  PageSprite.drawString(45, 130, buf);
  PageSprite.drawString(95, 130, "TO ALARM");
  PageSprite.pushSprite();
}

void drawTimePage() {
  M5.rtc.GetTime(&RTCtime);
  //drawTime(&RTCtime);
  drawTime();
  minutes = RTCtime.Minutes;
  //M5.rtc.GetDate(&RTCDate);
  //drawDate(&RTCDate);
  PageSprite.pushSprite();
}

/*
void flushTimePage() {
  // M5.M5Ink.clear();
  // PageSprite.clear( CLEAR_DRAWBUFF | CLEAR_LASTBUFF );
  drawTimePage();
  while (1) {
    M5.rtc.GetTime(&RTCtime);
    if (minutes != RTCtime.Minutes) {
      M5.rtc.GetTime(&RTCtime);
      //M5.rtc.GetDate(&RTCDate);

      if (RTCtime.Minutes % 10 == 0) {
        M5.M5Ink.clear();
        PageSprite.clear(CLEAR_DRAWBUFF | CLEAR_LASTBUFF);
      }
      drawTime(&RTCtime);
      //drawDate(&RTCDate);
      PageSprite.pushSprite();
      minutes = RTCtime.Minutes;
    }

    delay(10);
    M5.update();
    if (M5.BtnPWR.wasPressed()) {
      digitalWrite(LED_EXT_PIN, LOW);
      M5.shutdown();
    }
    if (M5.BtnDOWN.wasPressed() || M5.BtnUP.wasPressed()) break;
  }
  M5.M5Ink.clear();
  PageSprite.clear(CLEAR_DRAWBUFF | CLEAR_LASTBUFF);
}
*/

void checkRTC() {
  M5.rtc.GetTime(&RTCtime);
  if (RTCtime.Seconds == RTCTimeSave.Seconds) {
    //drawWarning("RTC Error");
    Serial.println("RTC Error");
    while (1) {
      if (M5.BtnMID.wasPressed()) return;
      delay(10);
      M5.update();
    }
  }
}