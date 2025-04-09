#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "secret.h"

//#include "ledc.h"
//#include <analogWrite.h>
//#define red 27
//#define yell 26
//#define gre 25
//#define pwmPin 33

//PWM Motor
#define PWM_FREQUENCY   1000
#define PWM_RESOLUTION  8
#define M1A_PWM_CHANNEL 6

// MQTT topic for IR sensor
const char* topic_publish_dht22 = "esp32/dht22";
const char* topic_publish_mq135 = "esp32/mq135"; 
//const char* topic_publish_crispVal = "esp32/crispVal";
const char* topic_publish_fanSpeed = "esp32/fanSpeed";
const char* topic_subscribe = "esp32/receive";

String fanSpeedState = "";

// IR Sensor details
const int dht22 = 34; 
const int mq135 = 35;
const int pwmPin = 26;
const int hvPin = 27;
// Create instances
WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);

// Variables for timing
long previous_time = 0;


bool modeAuto = true;
bool changeMode = false;
bool hvStat = false;
double aPred[9] = {0};
double z[9] = {0};
double aTot = 0;
double zTot = 0;
double zRes = 0;    
uint8_t rule = 0;
int speedRef = 0;
int manualSpeedRef = 0;
int lastSpeedVal = 0;
