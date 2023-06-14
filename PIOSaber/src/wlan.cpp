#include <wlan.hpp>

char* ssidmqtt = "raspi-webgui";
char* passwordmqtt = "ChangeMe";
char* mqtt_server = "169.254.203.208";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;


void initWiFi() {
  
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssidmqtt);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssidmqtt, passwordmqtt);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer("Moquitto", 1883);
}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "esp_tschorn_pub";
    if (client.connect(clientId.c_str())) { //client ID festlegen
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("pub_tschorn", "erste Nachricht"); // festlegen des Topics & festlegen, was die erste Nachricht ist
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      //delay(5000);
    }
  }
}
void connected() {
//if (!client.connected()) {
//  Serial.print("UwU");
//    reconnect();
//  }
  //client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("caro_pub", "UwU");
    Serial.println("UwU");  }
}