#pragma once
#include "FastLED.h"

#include <datatypes.hpp>
#include <config.hpp> // for NUM_LEDS

void randomPULSE(const LightSaberEnabled& lightSaberEnabled, const float& k);
void setPixel(int Pixel, const LightSaberColor& color);
void setAll(const LightSaberColor& color);
void light_up();
void light_down();
void setColor(byte color);
void hit_flash();
void Fire2012();

extern CRGB leds[NUM_LEDS];
// extern byte red, green, blue;
extern LightSaberColor lightSaberColor;
extern byte nowColor;