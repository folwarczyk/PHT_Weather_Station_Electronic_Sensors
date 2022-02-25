#include "DHT.h"
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>

#define wifi_ssid ""
#define wifi_password "" 

#define mqtt_host IPAddress(192, 168, 1, 000)
#define mqtt_port 1883

#define mqtt_pub_temperature "esp/dht/temperature"
#define mqtt_pub_humidity "esp/dht/humidity"

#define dht_pin 4
#define dht_type DHT22
DHT dht(dht_pin, dht_type);
float c_temperature;
float f_temperature;
float humidity;



AsyncMqttClient mqtt_client;
Ticker mqtt_reconnect_timer;

WiFiEventHandler wifi_connect_handler;
WiFiEventHandler wifi_disconnect_handler;
Ticker wifi_reconnect_timer;

unsigned long previous_millis = 0;
const long interval = 10000;

//connect to WiFi
void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(wifi_ssid, wifi_password);
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqtt_reconnect_timer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifi_reconnect_timer.once(2, connectToWifi);
}

//connect ESP8266 to MQTT broker
void connectToMqtt(){
  Serial.println("Coneccting to MQTT...");
  mqtt_client.connect();
}

//run after starting a session with mqtt broker
void onMqttConnect(bool session){
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(session);
}

//if ESP8266 loses connection with MQTT broker
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason){
  Serial.println("Disconnected from MQTT.");

  if(WiFi.isConnected()){
    mqtt_reconnect_timer.once(2, connectToMqtt);
  }
}

//publish a message
void onMqttPublish(uint16_t packet_id){
  Serial.print("Packet ID: ");
  Serial.println(packet_id);
}

void setup() {
  
  dht.begin();

  //in case the connection is lost
  wifi_connect_handler = WiFi.onStationModeGotIP(onWifiConnect);
  wifi_disconnect_handler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqtt_client.onConnect(onMqttConnect);
  mqtt_client.onDisconnect(onMqttDisconnect);
  //mqttClient.onSubscribe(onMqttSubscribe);
  //mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqtt_client.onPublish(onMqttPublish);
  mqtt_client.setServer(mqtt_host, mqtt_port);

  //mqtt_client.setCredentials("REPlACE_WITH_YOUR_USER", "REPLACE_WITH_YOUR_PASSWORD");
  connectToWifi();

}

void loop() {
  
 unsigned long current_millis = millis();
 if (current_millis - previous_millis >= interval) {
  previous_millis = current_millis;
  humidity = dht.readHumidity();
  //celsius
  c_temperature = dht.readTemperature();
  //fahrenheit
  f_temperature = dht.readTemperature(true);

  //publish the readings
 uint16_t packet_id_pub1 = mqtt_client.publish(mqtt_pub_humidity, 1, true, String(humidity).c_str());
 uint16_t packet_id_pub2 = mqtt_client.publish(mqtt_pub_temperature, 1, true, String(c_temperature).c_str());
 uint16_t packet_id_pub3 = mqtt_client.publish(mqtt_pub_temperature, 1, true, String(f_temperature).c_str());
 }
}
