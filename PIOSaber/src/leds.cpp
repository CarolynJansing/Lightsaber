#include "leds.hpp"

CRGB leds[NUM_LEDS];
// byte red, green, blue;
LightSaberColor lightSaberColor;
byte nowColor;
static unsigned long PULSE_timer;
static int PULSEOffset;

void setPixel(int Pixel, const LightSaberColor& color) {
  leds[Pixel].r = color.red;
  leds[Pixel].g = color.green;
  leds[Pixel].b = color.blue;
}

void setAll(const LightSaberColor& color) {
  for (int i = 0; i < NUM_LEDS; i++) {
    setPixel(i, color);
  }
  FastLED.show();
}

void light_up() {
  for (char i = 0; i <= (NUM_LEDS / 2 - 1); i++) {
    setPixel(i, lightSaberColor);
    setPixel((NUM_LEDS - 1 - i), lightSaberColor);
    FastLED.show();
    delay(25);
  }
}

void light_down() {
  for (char i = (NUM_LEDS / 2 - 1); i >= 0; i--) {
    setPixel(i, {0, 0, 0});
    setPixel((NUM_LEDS - 1 - i), {0, 0, 0});
    FastLED.show();
    delay(25);
  }
}

void hit_flash() {
  setAll({255, 255, 255});
  delay(FLASH_DELAY);
  setAll(lightSaberColor);
}

/**
 * @brief Set a color for the lightsaber from a list of colors
 * 
 * @param color 0 - red, 1 - green, 2 - blue, 3 - pink, 4 - yellow, 5 - ice blue
 */
void setColor(byte color) {
  switch (color) {
    case 0:
      lightSaberColor.red = 255;
      lightSaberColor.green = 0;
      lightSaberColor.blue = 0;
      break;
    case 1:
      lightSaberColor.red = 0;
      lightSaberColor.green = 0;
      lightSaberColor.blue = 255;
      break;
    case 2:
      lightSaberColor.red = 0;
      lightSaberColor.green = 255;
      lightSaberColor.blue = 0;
      break;
    case 3:
      lightSaberColor.red = 255;
      lightSaberColor.green = 0;
      lightSaberColor.blue = 255;
      break;
    case 4:
      lightSaberColor.red = 255;
      lightSaberColor.green = 255;
      lightSaberColor.blue = 0;
      break;
    case 5:
      lightSaberColor.red = 0;
      lightSaberColor.green = 255;
      lightSaberColor.blue = 255;
      break;
  }
}

// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100
#define COOLING 55

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 120

void Fire2012() {

  bool gReverseDirection = false;
  // Array of temperature readings at each simulation cell
  static uint8_t heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
  for (int i = 0; i < NUM_LEDS; i++) {
    heat[i] = qsub8(heat[i], random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for (int k = NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if (random8() < SPARKING) {
    int y = random8(7);
    heat[y] = qadd8(heat[y], random8(160, 255));
  }

  // Step 4.  Map from heat cells to LED colors
  for (int j = 0; j < NUM_LEDS; j++) {
    CRGB color = HeatColor(heat[j]);
    int pixelnumber;
    if (gReverseDirection) {
      pixelnumber = (NUM_LEDS - 1) - j;
    } else {
      pixelnumber = j;
    }
    leds[pixelnumber] = color;
  }
}

void randomPULSE(const LightSaberEnabled& lightSaberEnabled, const float& k) {
  if (PULSE_ALLOW && lightSaberEnabled.current && (millis() - PULSE_timer > PULSE_DELAY)) {
    PULSE_timer = millis();
    PULSEOffset = PULSEOffset * k + random(-PULSE_AMPL, PULSE_AMPL) * (1 - k);
    if (nowColor == 0)
      PULSEOffset = constrain(PULSEOffset, -15, 5);
    LightSaberColor offset;
    offset.red = constrain(lightSaberColor.red + PULSEOffset, 0, 255);
    offset.green = constrain(lightSaberColor.green + PULSEOffset, 0, 255);
    offset.blue = constrain(lightSaberColor.blue + PULSEOffset, 0, 255);
    setAll(offset);
  }
}