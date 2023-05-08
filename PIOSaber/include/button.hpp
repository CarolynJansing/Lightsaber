#pragma once

#include <Arduino.h> // datatypes -> byte
#include <datatypes.hpp>

void btnTick(LightSaberEnabled& lightSaberEnabled, bool& tenshiModeEnabled, bool& isLightsaberHumming, byte& nowColor, bool& storeColorAndHumToEEPROM);