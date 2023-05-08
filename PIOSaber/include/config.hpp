#pragma once

// ---------------------------- SETTINGS -------------------------------
#define NUM_LEDS 40         // number of microcircuits WS2811 on LED strip (note: one WS2811 controls 3 LEDs!)
#define BTN_TIMEOUT 1200    // button hold delay, ms
#define BTN_HOLD_TIME 2000  // button hold delay, ms
#define BRIGHTNESS 255      // max LED brightness (0 - 255)

#define SWING_TIMEOUT 500  // timeout between swings
#define SWING_L_THR 150    // swing angle speed threshold
#define SWING_THR 300      // fast swing angle speed threshold
#define STRIKE_THR 150     // hit acceleration threshold
#define STRIKE_S_THR 320   // hard hit acceleration threshold
#define FLASH_DELAY 80     // flash time while hit

#define PULSE_ALLOW 1   // blade pulsation (1 - allow, 0 - disallow)
#define PULSE_AMPL 20   // pulse amplitude
#define PULSE_DELAY 30  // delay between pulses

#define BATTERY_SAFE 0  // battery monitoring (1 - allow, 0 - disallow)

#define DEBUG 1  // debug information in Serial (1 - allow, 0 - disallow)
// ---------------------------- SETTINGS -------------------------------

#define LED_PIN D4
#define BTN D7
#define BTN_LED D3 // -1 -> disable, 4 original
//#define SD_ChipSelectPin 10