#include "battery.hpp"

#include <Arduino.h>
#include <config.hpp>

static unsigned long battery_timer;

void batteryTick(LightSaberEnabled& lightSaberEnabled) {
  if (millis() - battery_timer > 30000 && lightSaberEnabled.current && BATTERY_SAFE) {
    // if (voltage_measure() < 15) {
    //   ls_chg_state = 1;
    // }
    battery_timer = millis();
  }
}