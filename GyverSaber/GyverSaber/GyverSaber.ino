/*
  ARDUINO BASED MULTICOLOR SOUND PLAYING LIGHTSABER!
    HARDWARE:
      Addressable LED strip (WS2811) to get any blade color and smooth turn on effect
      DFPlaye module to play some sounds
      IMU MPU6050 (accel + gyro) to play sound matching to the swings
    CAPABILITIES:
      Smooth turning on/off with lightsaber-like sound effect
      Randomly pulsing color (you can turn it off)
      Sounds:
        MODE 1: generated hum. Frequency depends on angle velocity of blade
        MODE 2: hum sound from SD card
          Slow swing - long hum sound (randomly from 4 sounds)
          Fast swing - short hum sound (randomly from 5 sounds)
      Bright white flash when hitting
      Play one of 16 hit sounds, when hit
        Weak hit - short sound
        Hard hit - long "bzzzghghhdh" sound
      After power on blade shows current battery level from 0 to 100 percent
      Battery safe mode:
        Battery is drain BEFORE TURNING ON: GyverSaber will not turn on, button LED will PULSE a couple of times
        Battery is drain AFTER TURNING ON: GyverSaber will be turned off automatically
      CONTROL BUTTON:
        HOLD - turn on / turn off GyverSaber
        TRIPLE CLICK - change color (red - green - blue - yellow - pink - ice blue)
        QUARTER CLICK- flame blade mode inspired by Touhou Tenshi Hinanawi
        QUINARY CLICK - change sound mode (hum generation - hum playing)
        Selected color and sound mode stored in EEPROM (non-volatile memory)
*/

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
#define IMU_GND A1
#define SD_GND A0
#define VOLT_PIN A6
#define BTN_LED D3 // -1 -> disable, 4 original
#define SD_ChipSelectPin 10

// -------------------------- LIBS ---------------------------
#include <avr/pgmspace.h>  // PROGMEM library
#include <SD.h>
#include <TMRpcm.h>  // audio from SD library
#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050.h"
#include <toneAC.h>   // hum generation library
#include "FastLED.h"  // addressable LED library
#include <EEPROM.h>
#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

TMRpcm tmrpcm;
MPU6050 accelgyro;
SoftwareSerial mySoftwareSerial(D5, D6);  // RX, TX

DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

CRGB leds[NUM_LEDS];

// -------------------------- LIBS ---------------------------

// ------------------------------ VARIABLES ---------------------------------

int16_t ax, ay, az;
int16_t gx, gy, gz;
unsigned long ACC, GYR, COMPL;
int gyroX, gyroY, gyroZ, accelX, accelY, accelZ, freq, freq_f = 20;
float k = 0.2;
unsigned long humTimer = -9000, mpuTimer, nowTimer;
int stopTimer;
boolean bzzz_flag, ls_chg_state, ls_state;
unsigned long PULSE_timer, swing_timer, swing_timeout, battery_timer, bzzTimer;
byte nowNumber;
byte LEDcolor;  // 0 - red, 1 - green, 2 - blue, 3 - pink, 4 - yellow, 5 - ice blue
byte nowColor, red, green, blue, redOffset, greenOffset, blueOffset;
boolean eeprom_flag, swing_flag, swing_allow, strike_flag, HUMmode;
float voltage;
int PULSEOffset;
boolean tenshiMode;
// ------------------------------ VARIABLES ---------------------------------

// --------------------------------- SOUNDS ----------------------------------

int strike_time[8] = { 779, 563, 687, 702, 673, 661, 666, 635 };

int strike_s_time[8] = { 270, 167, 186, 250, 252, 255, 250, 238 };

int swing_time[8] = { 389, 372, 360, 366, 337 };

int swing_time_L[8] = { 636, 441, 772, 702 };

int BUFFER[10];
// --------------------------------- SOUNDS ---------------------------------

void setup() {
  pinMode(D7, INPUT_PULLUP);
  Serial.begin(115200);

  FastLED.addLeds<WS2811, LED_PIN, BRG>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(255);  // ~40% of LED strip brightness
  setAll(0, 0, 0);             // and turn it off

  Wire.begin();
  // Audio SetUp
  mySoftwareSerial.begin(9600);
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  if (!myDFPlayer.begin(mySoftwareSerial)) {  // Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
  }

  Serial.println(F("DFPlayer Mini online."));

  //----Set volume----
  myDFPlayer.volume(20);  // Set volume value (0~30).

  //----Set different EQ----
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);

  // ---- PIN-Settings ----
  pinMode(BTN, INPUT_PULLUP);
  pinMode(IMU_GND, OUTPUT);
  pinMode(SD_GND, OUTPUT);
  if (BTN_LED > 0)
    pinMode(BTN_LED, OUTPUT);
  digitalWrite(IMU_GND, 0);
  digitalWrite(SD_GND, 0);
  if (BTN_LED > 0)
    digitalWrite(BTN_LED, 1);
  //---- PIN-Settings ----

  randomSeed(analogRead(2));  // starting point for random generator

  //IMU initialization
  accelgyro.initialize();
  accelgyro.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
  accelgyro.setFullScaleGyroRange(MPU6050_GYRO_FS_250);
  if (DEBUG)
  {
    if (accelgyro.testConnection())
      Serial.println(F("MPU6050 OK"));
    else
      Serial.println(F("MPU6050 fail"));
  }

  // SD initialization was replaced by DFPlayer
  /* tmrpcm.speakerPin = 9;
   tmrpcm.setVolume(5);
   tmrpcm.quality(1);
   if (DEBUG) {
     if (SD.begin(8)) Serial.println(F("SD OK"));
     else Serial.println(F("SD fail"));
   } else {
     SD.begin(8);
   }*/

  if ((EEPROM.read(0) >= 0) && (EEPROM.read(0) <= 5)) {  // check first start
    nowColor = EEPROM.read(0);                           // remember color
    HUMmode = EEPROM.read(1);                            // remember mode
  } else {                                               // first start
    EEPROM.write(0, 0);                                  // set default
    EEPROM.write(1, 0);                                  // set default
    nowColor = 0;                                        // set default
  }

  setColor(nowColor);
  byte capacity = voltage_measure();                        // get battery level
  capacity = map(capacity, 100, 0, (NUM_LEDS / 2 - 1), 1);  // convert into blade lenght
  if (DEBUG) {
    Serial.print(F("Battery: "));
    Serial.println(capacity);
  }

  for (char i = 0; i <= capacity; i++) {  // show battery level
    setPixel(i, red, green, blue);
    setPixel((NUM_LEDS - 1 - i), red, green, blue);
    FastLED.show();
    delay(25);
  }
  delay(1000);  // 1 second to show battery level
  setAll(0, 0, 0);
  FastLED.setBrightness(BRIGHTNESS);  // set bright
}

// --- MAIN LOOP---
void loop() {
  if (!tenshiMode) {
    randomPULSE();
    getFreq();
    on_off_sound();
    btnTick();
    strikeTick();
    swingTick();
  } else {
    on_off_sound();
    btnTick();
    Fire2012();      // run simulation frame
    FastLED.show();  // display this frame
    FastLED.delay(1000 / 60);
  }
  batteryTick();
}
// --- MAIN LOOP---

void btnTick() {
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
    ls_chg_state = 1;  // flag to change saber state (on/off)
    isClickHeld = true;
  }

  // Only process the click if it is being held long enough
  if (!currentClickState and clickCounter > 0 and (millis() - lastClickStartedOn > BTN_TIMEOUT) and !isClickHeld) {
    if (DEBUG)
      Serial.println(F("BTN CLICK PROCESSING..."));
    if (ls_state) {
      if (clickCounter == 3) {
        tenshiMode = false;  // 3 press count
        nowColor++;          // change color
        if (nowColor >= 6)
          nowColor = 0;
        setColor(nowColor);
        setAll(red, green, blue);
        eeprom_flag = 1;
      }
      if (clickCounter == 4) {
        tenshiMode = true;
        myDFPlayer.playFolder(1, 33);
      }
      if (clickCounter == 5) {  // 5 press count
        HUMmode = !HUMmode;
        if (HUMmode) {
          if (!tenshiMode) { myDFPlayer.loop(1); }
        } else {
        }
        eeprom_flag = 1;
      }
    }

    if (DEBUG)
      Serial.println(F("BTN TIMEOUT -> COUNTER RESET"));
    clickCounter = 0;
  }
}

void on_off_sound() {
  if (ls_chg_state) {  // if change flag
    if (!ls_state) {   // if GyverSaber is turned off
      if (voltage_measure() > 10 || !BATTERY_SAFE) {
        if (DEBUG)
          Serial.println(F("SABER ON"));
        myDFPlayer.playFolder(1, 3);  // Play On sound
        delay(200);
        light_up();
        delay(200);
        bzzz_flag = 1;
        ls_state = true;  // remember that turned on
        if (HUMmode) {
          if (!tenshiMode) { myDFPlayer.loop(1); }
        } else {
        }
      } else {
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
    } else {  // if GyverSaber is turned on
      tenshiMode = false;
      bzzz_flag = 0;
      myDFPlayer.playFolder(1, 3);  // play off sound
      delay(300);
      light_down();
      myDFPlayer.disableLoop();
      delay(300);
      if (DEBUG)
        Serial.println(F("SABER OFF"));
      ls_state = false;
      if (eeprom_flag) {
        eeprom_flag = 0;
        EEPROM.write(0, nowColor);  // write color in EEPROM
        EEPROM.write(1, HUMmode);   // write mode in EEPROM
      }
    }
    ls_chg_state = 0;
  }

  if (((millis() - humTimer) > 9000) && bzzz_flag && HUMmode) {
    if (!tenshiMode) { myDFPlayer.loop(1); }
    humTimer = millis();
    swing_flag = 1;
    strike_flag = 0;
  }
  long delta = millis() - bzzTimer;
  if ((delta > 3) && bzzz_flag && !HUMmode) {
    if (strike_flag) {
      tmrpcm.disable();
      strike_flag = 0;
    }
    toneAC(freq_f, 1);
    bzzTimer = millis();
  }
}

void randomPULSE() {

  if (PULSE_ALLOW && ls_state && (millis() - PULSE_timer > PULSE_DELAY)) {
    PULSE_timer = millis();
    PULSEOffset = PULSEOffset * k + random(-PULSE_AMPL, PULSE_AMPL) * (1 - k);
    if (nowColor == 0)
      PULSEOffset = constrain(PULSEOffset, -15, 5);
    redOffset = constrain(red + PULSEOffset, 0, 255);
    greenOffset = constrain(green + PULSEOffset, 0, 255);
    blueOffset = constrain(blue + PULSEOffset, 0, 255);
    setAll(redOffset, greenOffset, blueOffset);
  }
}

void strikeTick() {
  if ((ACC > STRIKE_THR) && (ACC < STRIKE_S_THR)) {
    if (!HUMmode)
      noToneAC();
    nowNumber = random(8);
    // read the name of the sound from PROGMEM
    // strcpy_P(BUFFER, (int*)pgm_read_word(&(strikes_short[nowNumber])));
    Serial.println("Strike play");
    myDFPlayer.playFolder(1, random(12, 20));
    hit_flash();
    if (!HUMmode)
      bzzTimer = millis() + strike_s_time[nowNumber] - FLASH_DELAY;
    else
      humTimer = millis() - 9000 + strike_s_time[nowNumber] - FLASH_DELAY;
    strike_flag = 1;
  }
  if (ACC >= STRIKE_S_THR) {
    if (!HUMmode)
      noToneAC();
    nowNumber = random(8);
    // read the name of the sound from PROGMEM
    // strcpy_P(BUFFER, (char*)pgm_read_word(&(strikes[nowNumber])));
    Serial.println("Strike play S");
    myDFPlayer.playFolder(1, random(4, 12));
    hit_flash();
    if (!HUMmode)
      bzzTimer = millis() + strike_time[nowNumber] - FLASH_DELAY;
    else
      humTimer = millis() - 9000 + strike_time[nowNumber] - FLASH_DELAY;
    strike_flag = 1;
  }
}

void swingTick() {
  if (GYR > 80 && (millis() - swing_timeout > 100) && HUMmode) {
    Serial.println("SWINGIN");
    swing_timeout = millis();
    if (((millis() - swing_timer) > SWING_TIMEOUT) && swing_flag && !strike_flag) {
      if (GYR >= SWING_THR) {
        Serial.println("SWING NORMAL");
        nowNumber = random(5);
        // read the name of the sound from PROGMEM
        // strcpy_P(BUFFER, (char*)pgm_read_word(&(swings[nowNumber])));
        myDFPlayer.playFolder(1, random(24, 29));
        humTimer = millis() - 9000 + swing_time[nowNumber];
        swing_flag = 0;
        swing_timer = millis();
        swing_allow = 0;
      }
      if ((GYR > SWING_L_THR) && (GYR < SWING_THR)) {
        Serial.println("SWING LARGE");

        nowNumber = random(5);
        // read the name of the sound from PROGMEM
        // strcpy_P(BUFFER, (char*)pgm_read_word(&(swings_L[nowNumber])));
        myDFPlayer.playFolder(1, random(20, 24));
        humTimer = millis() - 9000 + swing_time_L[nowNumber];
        swing_flag = 0;
        swing_timer = millis();
        swing_allow = 0;
      }
    }
  }
}

void getFreq() {
  if (ls_state) {  // if GyverSaber is on
    if (millis() - mpuTimer > 500) {
      accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

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

void setPixel(int Pixel, byte red, byte green, byte blue) {
  leds[Pixel].r = red;
  leds[Pixel].g = green;
  leds[Pixel].b = blue;
}

void setAll(byte red, byte green, byte blue) {
  for (int i = 0; i < NUM_LEDS; i++) {
    setPixel(i, red, green, blue);
  }
  FastLED.show();
}

void light_up() {
  for (char i = 0; i <= (NUM_LEDS / 2 - 1); i++) {
    setPixel(i, red, green, blue);
    setPixel((NUM_LEDS - 1 - i), red, green, blue);
    FastLED.show();
    delay(25);
  }
}

void light_down() {
  for (char i = (NUM_LEDS / 2 - 1); i >= 0; i--) {
    setPixel(i, 0, 0, 0);
    setPixel((NUM_LEDS - 1 - i), 0, 0, 0);
    FastLED.show();
    delay(25);
  }
}

void hit_flash() {
  setAll(255, 255, 255);
  delay(FLASH_DELAY);
  setAll(red, blue, green);
}

void setColor(byte color) {
  switch (color) {
    // 0 - red, 1 - green, 2 - blue, 3 - pink, 4 - yellow, 5 - ice blue
    case 0:
      red = 255;
      green = 0;
      blue = 0;
      break;
    case 1:
      red = 0;
      green = 0;
      blue = 255;
      break;
    case 2:
      red = 0;
      green = 255;
      blue = 0;
      break;
    case 3:
      red = 255;
      green = 0;
      blue = 255;
      break;
    case 4:
      red = 255;
      green = 255;
      blue = 0;
      break;
    case 5:
      red = 0;
      green = 255;
      blue = 255;
      break;
  }
}

void batteryTick() {
  if (millis() - battery_timer > 30000 && ls_state && BATTERY_SAFE) {
    if (voltage_measure() < 15) {
      ls_chg_state = 1;
    }
    battery_timer = millis();
  }
}

byte voltage_measure() {
  int R1 = 51000;
  int R2 = 100000;
  voltage = 0;
  for (int i = 0; i < 10; i++) {
    voltage += (float)analogRead(VOLT_PIN) * 5 / 1023 * (R1 + R2) / R2;
  }
  voltage = voltage / 10;
  int volts = voltage / 3 * 100;  // 3 cells!!!
  if (volts > 387)
    return map(volts, 420, 387, 100, 77);
  else if ((volts <= 387) && (volts > 375))
    return map(volts, 387, 375, 77, 54);
  else if ((volts <= 375) && (volts > 368))
    return map(volts, 375, 368, 54, 31);
  else if ((volts <= 368) && (volts > 340))
    return map(volts, 368, 340, 31, 8);
  else if (volts <= 340)
    return map(volts, 340, 260, 8, 0);
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
