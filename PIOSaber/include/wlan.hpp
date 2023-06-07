
#pragma once
//#ifndef WIFI_INIT_H
//#define WIFI_INIT_H
#include <wlan.hpp>
#include <WiFi.h>
#include <PubSubClient.h>
///#include <ESP8266WiFi.h>

extern char* ssidmqtt;
extern char* passwordmqtt;
extern char* MQTT_BROKER;

extern WiFiClient espClient;

void initWiFi();
void connected();
//#endif
