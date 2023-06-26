#pragma once


enum MQTT_EVENT: int {
   BTN_PRESS,
   STRIKE,
   STRIKE_S,
   SWING_NORMAL,
   SWING_LARGE,
   OFF,
   ON,
   TENSHI,
   COLORCHANGE,
};

void new_mqtt_event(MQTT_EVENT type);