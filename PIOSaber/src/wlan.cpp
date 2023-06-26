#include <wlan.hpp> 

const char* ssidmqtt = "FRITZ!Box 6591 Cable FW";
const char* passwordmqtt = "23268987090163585102";
const char* mqtt_server = "192.168.178.70";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;


void initWiFi() {
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
}

void reconnect() {
  // Loop until we're reconnected
  Serial.print("Attempting MQTT connection...");
  client.setServer(mqtt_server, 1883);
  String clientId = "esp_lightsaber_pub";
  if (client.connect(clientId.c_str())) { //client ID festlegen
    Serial.println("connected");
    // Once connected, publish an announcement...
    client.publish("caro_pub", "erste Nachricht"); // festlegen des Topics & festlegen, was die erste Nachricht ist
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
    // Wait 2 seconds before retrying
    delay(2000);
  }
  
}
void connected() {
  if (!client.connected()) {
    Serial.println("Reconnecting to MQTT(this may cause the lightsaber to hang temporarly)...");
    reconnect();
  }
}