#include "mqtt.hpp"
#include "wlan.hpp"

void new_mqtt_event(MQTT_EVENT type) {
    if(!client.connected())
        return; // if not connected, don't publish

    if (type==STRIKE)
    {
        client.publish("caro_pub", "strike");
    }
    if (type==BTN_PRESS)
    {
        client.publish("caro_pub", "BTN_PRESS");
    }
    if (type==STRIKE_S)
    {
        client.publish("caro_pub", "STRIKE_S");
    }
    if (type==SWING_NORMAL)
    {
        client.publish("caro_pub", "SWING_NORMAL");
    }
    if (type==SWING_LARGE)
    {
        client.publish("caro_pub", "SWING_LARGE");
    }
    if (type==OFF)
    {
        client.publish("caro_pub", "OFF");
    }
    if (type==ON)
    {
        client.publish("caro_pub", "ON");
    }
    if (type==TENSHI)
    {
        client.publish("caro_pub", "TENSHI");
    }
    if (type==COLORCHANGE)
    {
        client.publish("caro_pub", "COLORCHANGE");
    }
    
}