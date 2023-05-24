#include <wlan.hpp>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "IhrWLANName";
const char* password = "IhrWLANPasswort";

void initWiFi() {
  WiFi.begin(ssid, password);
  
  Serial.print("Verbinde mit WLAN...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.print("Verbunden mit WLAN. IP-Adresse: ");
  Serial.println(WiFi.localIP());
}