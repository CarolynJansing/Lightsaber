#include <wlan.hpp>

char* ssidmqtt = "WLANrouterRPPT";
char* passwordmqtt = "bbsbrinkstrasse";
char* MQTT_BROKER = "192.168.1.2";

WiFiClient espClient;
PubSubClient client(espClient);

void initWiFi() {
  WiFi.begin(ssidmqtt,passwordmqtt);
  
  Serial.print("Verbinde mit WLAN...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.print("Verbunden mit WLAN. IP-Adresse: ");
  Serial.println(WiFi.localIP());
}
void connected(){
  if(!client.connected()){
      client.connect("ESP8266Client");
      delay(100);
  }
  client.loop();
}