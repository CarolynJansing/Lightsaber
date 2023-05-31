
#pragma once
//#ifndef WIFI_INIT_H
//#define WIFI_INIT_H
#include <wlan.hpp>
#include <WiFi.h>
#include <PubSubClient.h>
///#include <ESP8266WiFi.h>

char* ssidmqtt = "WLANrouterRPPT";
char* passwordmqtt = "bbsbrinkstrasse";
const char* MQTT_BROKER = "Moquitto";

WiFiClient espClient;
PubSubClient client(espClient);

void initWiFi();
void connected();
//#endif
