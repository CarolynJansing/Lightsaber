#include "sounds.hpp"

#include <accelerometer.hpp>
#include <leds.hpp>
#include <config.hpp>

#include <EEPROM.h>

SoftwareSerial dfPlayerSerial(D6, D5); // RX, TX
DFRobotDFPlayerMini dfPlayer;
unsigned long ACC, GYR, COMPL;
unsigned long soundHumStartedTime = -900, soundBzzStartedTime;
bool isLightsaberSwinging, isLightsaberStriking;

static unsigned long mpuTimer;
static int16_t ax, ay, az;
static int16_t gx, gy, gz;
static int gyroX, gyroY, gyroZ, accelX, accelY, accelZ, freq, freq_f = 20;
static bool bzzz_flag;

void getFreq(const LightSaberEnabled& lightSaberEnabled, const float& k) {
  if (lightSaberEnabled.current) {  // if GyverSaber is on
    if (millis() - mpuTimer > 500) {
      accelerometer.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

      // find absolute and divide on 100
      gyroX = abs(gx / 100);
      gyroY = abs(gy / 100);
      gyroZ = abs(gz / 100);
      accelX = abs(ax / 100);
      accelY = abs(ay / 100);
      accelZ = abs(az / 100);

      // vector sum
      ACC = sq((long)accelX) + sq((long)accelY) + sq((long)accelZ);
      ACC = sqrt(ACC);
      GYR = sq((long)gyroX) + sq((long)gyroY) + sq((long)gyroZ);
      GYR = sqrt((long)GYR);
      COMPL = ACC + GYR;

      // debugging the IMU
        //  Serial.print("$");
        //  Serial.print(gyroX);
        //  Serial.print(" ");
        //  Serial.print(gyroY);
        //  Serial.print(" ");
        //  Serial.print(gyroZ);
        //  Serial.println(";");
        //  Serial.println(GYR);

      freq = (long)COMPL * COMPL / 1500;  // parabolic tone change
      freq = constrain(freq, 18, 300);
      freq_f = freq * k + freq_f * (1 - k);  // smooth filter
      mpuTimer = micros();
    }
  }
}

void on_off_sound(LightSaberEnabled& lightSaberEnabled, bool& tenshiModeEnabled, bool& storeColorAndHumToEEPROM, bool& isLightsaberHumming, byte& nowColor) {
  if (lightSaberEnabled.changed) {  // if change flag
    if (!lightSaberEnabled.current) {   // if GyverSaber is turned off
        if (!BATTERY_SAFE) {
            if (DEBUG){
                Serial.println(F("SABER ON"));
            }
            dfPlayer.playFolder(1, 3);  // Play On sound
            delay(200);
            light_up();
            delay(200);
            bzzz_flag = 1;
            lightSaberEnabled.current = true;  // remember that turned on
            if (isLightsaberHumming) {
            if (!tenshiModeEnabled) { dfPlayer.loop(1); }
            } 
       }
       else 
       {
        if (DEBUG)
          Serial.println(F("LOW VOLTAGE!"));
        for (int i = 0; i < 5; i++) {
          if (BTN_LED > 0)
            digitalWrite(BTN_LED, 0);
          delay(400);
          if (BTN_LED > 0)
            digitalWrite(BTN_LED, 1);
          delay(400);
        }
        }
    } else { // if GyverSaber is turned on
      tenshiModeEnabled = false;
      bzzz_flag = 0;
      dfPlayer.playFolder(1, 3); // play off sound
      delay(300);
      light_down();
      dfPlayer.disableLoop();
      delay(300);
      if (DEBUG)
        Serial.println(F("SABER OFF"));
      lightSaberEnabled.current = false;
      if (storeColorAndHumToEEPROM) {
        storeColorAndHumToEEPROM = false;
        EEPROM.write(0, nowColor); // write color in EEPROM
        EEPROM.write(1, isLightsaberHumming); // write mode in EEPROM
      }
    }
    lightSaberEnabled.changed = false;
  }

  if (((millis() - soundHumStartedTime) > 9000) && bzzz_flag && isLightsaberHumming) {
    if (!tenshiModeEnabled) { dfPlayer.loop(1); }
    soundHumStartedTime = millis();
    isLightsaberSwinging = true;
    isLightsaberStriking = false;
  }
  long delta = millis() - soundBzzStartedTime;
  if ((delta > 3) && bzzz_flag && !isLightsaberHumming) {
    isLightsaberStriking = false;
    soundBzzStartedTime = millis();
  }
}