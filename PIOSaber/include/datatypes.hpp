#pragma once

#include <Arduino.h> // byte

struct LightSaberEnabled {
  bool current;
  bool changed;
};

struct LightSaberColor {
  byte red;
  byte green;
  byte blue;
};