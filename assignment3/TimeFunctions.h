#include "M5CoreInk.h"

typedef struct {
  int width;
  int height;
  int bitCount;
  unsigned char *ptr;
} image_t;

extern RTC_TimeTypeDef RTCtime;
extern RTC_TimeTypeDef RTCTimeSave;
extern RTC_TimeTypeDef AlarmTime;

extern uint8_t minutes;
extern Ink_Sprite PageSprite;

extern int program_state;

void drawImageToSprite(int posX, int posY, image_t *imagePtr, Ink_Sprite *sprite);
void updateTime();
//void drawTime(RTC_TimeTypeDef *time);
void drawTime();
void drawAlarmTime();
void drawTimeToAlarm();
void drawTimePage();
void checkRTC();