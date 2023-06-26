
#pragma once
//#ifndef WIFI_INIT_H
//#define WIFI_INIT_H
//#include <wlan.hpp>
//#include <WiFi.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
///#include <ESP8266WiFi.h>

extern const char* ssidmqtt;
extern const char* passwordmqtt;
extern const char* mqtt_server;

extern WiFiClient espClient;
extern PubSubClient client;
void initWiFi();
void connected();
//#endif
