// Other files
#include "leds.hpp"
#include "sounds.hpp"
#include "button.hpp"
#include "battery.hpp"
#include "accelerometer.hpp"
#include "wlan.hpp"
#include <config.hpp>

#include <EEPROM.h>



/**
 * HEY CARO!
 *
 * Ich habe nun den Code aufgeräumt & (zumindest oberflächlich) neu strukturiert.
 * Viel Spaß beim Nutzen (ich hoffe, ich habe nichts kaputt gemacht :D)
 * 
 * Du solltest auf jeden Fall die neuen Variablen-namen verstehen, da diese
 * hoffentlich etwas aussagekräftiger sind...
 */



// Variables shared with other files
LightSaberEnabled lightSaberEnabled;
bool storeColorAndHumToEEPROM, isLightsaberHumming;
bool tenshiModeEnabled;

// Variables for main.cpp
float k = 0.2;
unsigned long soundSwingStartedTime, soundSwingLastTick;

byte soundIndex;
const int soundTimeStrike[8] = { 779, 563, 687, 702, 673, 661, 666, 635 };
const int soundTimeStrikes[8] = { 270, 167, 186, 250, 252, 255, 250, 238 };
const int soundTimeSwing[8] = { 389, 372, 360, 366, 337 };
const int soundTimeSwingLong[8] = { 636, 441, 772, 702 };

void strikeTick() {
  if ((ACC > STRIKE_THR) && (ACC < STRIKE_S_THR)) {
    soundIndex = random(8);
    Serial.println("Strike play");
    dfPlayer.playFolder(1, random(12, 20));
    hit_flash();
    if (!isLightsaberHumming)
      soundBzzStartedTime = millis() + soundTimeStrikes[soundIndex] - FLASH_DELAY;
    else
      soundHumStartedTime = millis() - 9000 + soundTimeStrikes[soundIndex] - FLASH_DELAY;
    isLightsaberStriking = true;
  }
  if (ACC >= STRIKE_S_THR) {
    soundIndex = random(8);
    Serial.println("Strike play S");
    dfPlayer.playFolder(1, random(4, 12));
    hit_flash();
    if (!isLightsaberHumming)
      soundBzzStartedTime = millis() + soundTimeStrike[soundIndex] - FLASH_DELAY;
    else
      soundHumStartedTime = millis() - 9000 + soundTimeStrike[soundIndex] - FLASH_DELAY;
    isLightsaberStriking = true;
  }
}

void swingTick() {
  if (GYR > 80 && (millis() - soundSwingLastTick > 100) && isLightsaberHumming) {
    Serial.println("SWINGIN");
    soundSwingLastTick = millis();
    if (((millis() - soundSwingStartedTime) > SWING_TIMEOUT) && isLightsaberSwinging && !isLightsaberStriking) {
      if (GYR >= SWING_THR) {
        Serial.println("SWING NORMAL");
        soundIndex = random(5);
        dfPlayer.playFolder(1, random(24, 29));
        soundHumStartedTime = millis() - 9000 + soundTimeSwing[soundIndex];
        isLightsaberSwinging = false;
        soundSwingStartedTime = millis();
      }
      if ((GYR > SWING_L_THR) && (GYR < SWING_THR)) {
        Serial.println("SWING LARGE");

        soundIndex = random(5);
        dfPlayer.playFolder(1, random(20, 24));
        soundHumStartedTime = millis() - 9000 + soundTimeSwingLong[soundIndex];
        isLightsaberSwinging = false;
        soundSwingStartedTime = millis();
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  initWiFi();
  FastLED.addLeds<WS2811, LED_PIN, BRG>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(255);  // ~40% of LED strip brightness
  setAll({0, 0, 0});             // and turn it off

  // Audio SetUp
  dfPlayerSerial.begin(9600);
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  if (!dfPlayer.begin(dfPlayerSerial)) {  // Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
  }
  Serial.println(F("DFPlayer Mini online."));

  //----Set volume----
  dfPlayer.volume(20);  // Set volume value (0~30).

  //----Set different EQ----
  dfPlayer.EQ(DFPLAYER_EQ_NORMAL);
  dfPlayer.outputDevice(DFPLAYER_DEVICE_SD);

  // ---- PIN-Settings ----
  pinMode(BTN, INPUT_PULLUP); // to check if button is pressed (shorts to ground)
  if (BTN_LED > 0)
    pinMode(BTN_LED, OUTPUT);
    
  if (BTN_LED > 0)
    digitalWrite(BTN_LED, 1);
  //---- PIN-Settings ----

  randomSeed(analogRead(2));  // starting point for random generator

  // join I2C bus (I2Cdev library doesn't do this automatically) -> preparation for IMU
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    Wire.begin();
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
    Fastwire::setup(400, true);
  #endif

  // IMU initialization
  accelerometer.initialize();
  if (accelerometer.testConnection())
    Serial.println(F("MPU6050 OK"));
  else
    Serial.println(F("MPU6050 fail (try hard-resetting the lightsaber); proceeding anyway..."));
  accelerometer.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
  accelerometer.setFullScaleGyroRange(MPU6050_GYRO_FS_250);

  // Read last state from EEPROM
  if ((EEPROM.read(0) >= 0) && (EEPROM.read(0) <= 5)) {
    nowColor = EEPROM.read(0);
    isLightsaberHumming = EEPROM.read(1);
  } else {
    EEPROM.write(0, 0);
    EEPROM.write(1, 0);
    nowColor = 0;
  }

  setColor(nowColor);
  // byte capacity = voltage_measure();                        // get battery level
  // capacity = map(capacity, 100, 0, (NUM_LEDS / 2 - 1), 1);  // convert into blade lenght
  if (DEBUG) {
    Serial.print(F("Battery: "));
    // Serial.println(capacity);
  }

  // for (char i = 0; i <= capacity; i++) {  // show battery level
  //   setPixel(i, {red, green, blue});
  //   setPixel((NUM_LEDS - 1 - i), {red, green, blue});
  //   FastLED.show();
  //   delay(25);
  // }
  // delay(1000);  // 1 second to show battery level
  setAll({0, 0, 0});
  FastLED.setBrightness(BRIGHTNESS);  // set bright
}

void loop() {
  if (!tenshiModeEnabled) {
    randomPULSE(lightSaberEnabled, k);
    getFreq(lightSaberEnabled, k);
    on_off_sound(lightSaberEnabled, tenshiModeEnabled, storeColorAndHumToEEPROM, isLightsaberHumming, nowColor);
    btnTick(lightSaberEnabled, tenshiModeEnabled, isLightsaberHumming, nowColor, storeColorAndHumToEEPROM);
    strikeTick();
    swingTick();
  } else {
    on_off_sound(lightSaberEnabled, tenshiModeEnabled, storeColorAndHumToEEPROM, isLightsaberHumming, nowColor);
    btnTick(lightSaberEnabled, tenshiModeEnabled, isLightsaberHumming, nowColor, storeColorAndHumToEEPROM);
    Fire2012(); // run simulation frame
    FastLED.show(); // display this frame
    FastLED.delay(1000 / 60);
  }
  // batteryTick(lightSaberEnabled);
}