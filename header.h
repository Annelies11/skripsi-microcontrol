#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#define red 27
#define yell 26
#define gre 25
#include "secret.h"

// MQTT topic for IR sensor
const char* topic_publish_dht22 = "esp32/dht22";
const char* topic_publish_mq135 = "esp32/mq135"; 
const char* topic_publish_crispVal = "esp32/crispVal";
const char* topic_publish_fanSpeed = "esp32/fanSpeed";
const char* topic_subscribe = "esp32/receive";

String fanSpeedState = "";

// IR Sensor details
const int dht22 = 34; 
const int mq135 = 35;

// Create instances
WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);

// Variables for timing
long previous_time = 0;
