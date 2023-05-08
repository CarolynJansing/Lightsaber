#pragma once
#include "DFRobotDFPlayerMini.h"
#include "SoftwareSerial.h"

#include <datatypes.hpp>

void on_off_sound(LightSaberEnabled& lightSaberEnabled, bool& tenshiModeEnabled, bool& storeColorAndHumToEEPROM, bool& isLightsaberHumming, byte& nowColor);
void getFreq(const LightSaberEnabled& lightSaberEnabled, const float& k);
void printDetail(uint8_t type, int value);

extern SoftwareSerial dfPlayerSerial;
extern DFRobotDFPlayerMini dfPlayer;
extern unsigned long soundHumStartedTime, soundBzzStartedTime;
extern bool isLightsaberSwinging, isLightsaberStriking;

// If have no idea what those do ↓↓↓
extern unsigned long ACC, GYR, COMPL;