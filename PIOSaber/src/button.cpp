#include "button.hpp"

#include <leds.hpp>
#include <sounds.hpp>
#include <config.hpp>

void btnTick(LightSaberEnabled& lightSaberEnabled, bool& tenshiModeEnabled, bool& isLightsaberHumming, byte& nowColor, bool& storeColorAndHumToEEPROM) {
  // BTN_TIMEOUT, BTN_HOLD_TIMEOUT
  static unsigned long lastClickStartedOn = 0;
  static unsigned char clickCounter = 0;
  static bool lastClickState = false;
  static bool isClickHeld = false;
  bool currentClickState = digitalRead(BTN);
  if (currentClickState and !lastClickState) {
    lastClickState = true;
    clickCounter++;
    lastClickStartedOn = millis();
    if (DEBUG) {
      Serial.print(F("BTN PRESS (STATE "));
      Serial.print(currentClickState);
      Serial.print(F("; COUNT "));
      Serial.print(clickCounter);
      Serial.println(F(")"));
    }
  } else if (!currentClickState and lastClickState) {
    lastClickState = false;
    isClickHeld = false;
  }

  // ...if button is held
  if (currentClickState and clickCounter == 1 and (millis() - lastClickStartedOn > BTN_HOLD_TIME) and !isClickHeld) {
    if (DEBUG)
      Serial.println(F("BTN HELD -> TRIGGER"));
    lightSaberEnabled.changed = true; // flag to change saber state (on/off)
    isClickHeld = true;
  }

  // Only process the click if it is being held long enough
  if (!currentClickState and clickCounter > 0 and (millis() - lastClickStartedOn > BTN_TIMEOUT) and !isClickHeld) {
    if (DEBUG)
      Serial.println(F("BTN CLICK PROCESSING..."));
    if (lightSaberEnabled.current) {
      if (clickCounter == 3) {
        tenshiModeEnabled = false;  // 3 press count
        nowColor++;          // change color
        if (nowColor >= 6)
          nowColor = 0;
        setColor(nowColor);
        setAll(lightSaberColor);
        storeColorAndHumToEEPROM = true;
      }
      if (clickCounter == 4) {
        tenshiModeEnabled = true;
        dfPlayer.playFolder(1, 33);
      }
      if (clickCounter == 5) {  // 5 press count
        isLightsaberHumming = !isLightsaberHumming;
        if (isLightsaberHumming) {
          if (!tenshiModeEnabled) { dfPlayer.loop(1); }
        } else {
        }
        storeColorAndHumToEEPROM = true;
      }
    }

    if (DEBUG)
      Serial.println(F("BTN TIMEOUT -> COUNTER RESET"));
    clickCounter = 0;
  }
}