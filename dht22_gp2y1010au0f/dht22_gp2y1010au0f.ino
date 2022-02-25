#include "DHT.h"
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>

#define wifi_ssid "Domek100gora"
#define wifi_password "1922319223"

//#define mqtt_host "pht-weather-station.mywire.org"
#define mqtt_host IPAddress(178, 43, 42, 67)
#define mqtt_port 1883

//topics
#define mqtt_pub_temperature "pht/city/3/sensor/1"
#define mqtt_pub_humidity "pht/city/3/sensor/2"
#define mqtt_pub_avg_dust "pht/city/3/sensor/3"

//dht22
#define dht_pin 14  
#define dht_type DHT22   
DHT dht(dht_pin, dht_type);
float c_temperature;
float f_temperature;
float humidity;

//gp2y1010au0f
#define min_voltage 600
#define vref 3000
#define pin_led 2
#define pin_analog 0
#define max_iters 10
int adc_value;
int iter;
float voltage;
float dust;
float avg_dust;

AsyncMqttClient mqtt_client;
Ticker mqtt_reconnect_timer;

WiFiEventHandler wifi_connect_handler;
WiFiEventHandler wifi_disconnect_handler;
Ticker wifi_reconnect_timer;

//last time read
unsigned long previousMillis = 0;   
const long interval = 10000;        

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
  mqtt_reconnect_timer.detach(); 
  wifi_reconnect_timer.once(2, connectToWifi);
}

//connect ESP8266 to MQTT
void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqtt_client.connect();
}

//run after starting a session with mqtt broker
void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
}

//if ESP8266 loses connection with MQTT 
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqtt_reconnect_timer.once(2, connectToMqtt);
  }
}

//publish a message
void onMqttPublish(uint16_t packetId) {
  Serial.print("PacketId: ");
  Serial.println(packetId);
  Serial.print("\n");
}

void setup() {
  Serial.begin(9600);
  Serial.println();

  dht.begin();

  Serial.println(mqtt_host);


  pinMode(pin_led, OUTPUT);
  digitalWrite(pin_led, LOW);

  //in case the connection is lost
  wifi_connect_handler = WiFi.onStationModeGotIP(onWifiConnect);
  wifi_disconnect_handler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqtt_client.onConnect(onMqttConnect);
  mqtt_client.onDisconnect(onMqttDisconnect);
  mqtt_client.onPublish(onMqttPublish);
  mqtt_client.setServer(mqtt_host, mqtt_port);
  
  connectToWifi();
}

float computeDust()
{
  digitalWrite(pin_led, HIGH);
  delayMicroseconds(280);
  adc_value = analogRead(pin_analog);
  digitalWrite(pin_led, LOW);

  voltage = (vref / 1024.0) * adc_value * 11;

  if(voltage > min_voltage){
    return (voltage - min_voltage) * 0.2;
  }
  return 0;
}



void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    humidity = dht.readHumidity();
    //Celsius
    c_temperature = dht.readTemperature();
    //Fahrenheit
    //f_temperature = dht.readTemperature(true);

    if(isnan(humidity) || isnan(c_temperature)) {
      Serial.println("FAILED");
    }

   /* avg_dust = 0;
    iter = 0;

    while (iter < max_iters){
    dust = computeDust();
    if (dust > 0){
      avg_dust += dust;
      iter++;
      delay(50);
      }
    }*/

    /*avg_dust /= max_iters;
    Serial.print("D = ");
    Serial.print(avg_dust);
    Serial.println("ug/m3");*/

    uint16_t packet_id_pub1 = mqtt_client.publish(mqtt_pub_temperature, 1, true, String(c_temperature).c_str());                            
    Serial.printf("Publishing on topic %s , packet_id %i: ", mqtt_pub_temperature, packet_id_pub1);
    Serial.printf("Temperature: %.2f \n", c_temperature);

    uint16_t packet_id_pub2 = mqtt_client.publish(mqtt_pub_humidity, 1, true, String(humidity).c_str());                            
    Serial.printf("Publishing on topic %s , packet_id %i: ", mqtt_pub_humidity, packet_id_pub2);
    Serial.printf("Humidity: %.2f \n", humidity);

    /*uint16_t packet_id_pub3 = mqtt_client.publish(mqtt_pub_avg_dust, 1, true, String(avg_dust).c_str());                            
    Serial.printf("Publishing on topic %s , packet_id %i: ", mqtt_pub_avg_dust, packet_id_pub3);
    Serial.printf("Dust: %.2f \n", avg_dust);*/

    Serial.println("________________________");
    Serial.println(c_temperature);
    Serial.println(humidity);
    Serial.println(avg_dust);
  }
}
