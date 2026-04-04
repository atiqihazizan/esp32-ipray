#include "buttons.h"
#include "sound.h"
#include "app_fsm.h"
#include "config.h"
#include "utils.h"

void handleButtons() {
  if (digitalRead(BTN_MENU) == LOW) {
    delay(200);
    if (currentState == SLEEP)
      currentState = HOME;
    else if (currentState == HOME)
      currentState = MENU;
    else if (currentState == MENU)
      currentState = HOME;
  }
  if (digitalRead(BTN_UP) == LOW) {
    delay(200);
    if (currentState == MENU)
      menuIndex = (menuIndex - 1 + menuItems) % menuItems;
  }
  if (digitalRead(BTN_DOWN) == LOW) {
    delay(200);
    if (currentState == MENU)
      menuIndex = (menuIndex + 1) % menuItems;
  }
  if (digitalRead(BTN_SET) == LOW) {
    delay(200);
    if (currentState == SLEEP)
      currentState = HOME;
    else if (currentState == HOME)
      currentState = SLEEP;
    else if (currentState == MENU) {
      if (menuIndex == 0)
        currentState = SET_TAKWIM;
      else if (menuIndex == 1)
        currentState = SET_DATE;
      else if (menuIndex == 2)
        currentState = SET_TIME;
      else if (menuIndex == 3)
        currentState = SET_WIFI;
    }
  }
  if (digitalRead(BTN_RET) == LOW) {
    delay(200);
    if (currentState == SLEEP)
      currentState = HOME;
    else if (currentState == MENU)
      currentState = HOME;
    else if (currentState != HOME)
      currentState = MENU;
  }
  if (digitalRead(BTN_LAYOUT) == LOW) {
    delay(200);
    if (currentState == HOME) {
      displayLayout = (displayLayout == 1) ? 2 : 1;

      if (preferences.begin("cfg", false)) {
        preferences.putInt("layout", displayLayout);
        preferences.end();
      }

      if (dfPlayerReady && !currentMinuteMatchesAnySolat(rtc.now()))
        appFsmEnqRoot(TRACK_SD_BEEP, 100);
      Serial.printf("[layout] switched to layout %d\n", displayLayout);
    }
  }
}
