//#include <Arduino.h>
//#include "DHT.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <MQUnifiedsensor.h>
#include "secret.h"

//#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

//PWM Motor
#define PWM_FREQUENCY   1000
#define PWM_RESOLUTION  8
#define M1A_PWM_CHANNEL 6

//MQ properties
#define placa "ESP32"
#define Voltage_Resolution 3.3
#define pin 36 //Analog input 0 of your arduino
#define type "MQ-135" //MQ135
#define ADC_Bit_Resolution 12 // For arduino UNO/MEGA/NANO
#define RatioMQ135CleanAir 3.6//RS / R0 = 3.6 ppm  
//#define calibration_button 13 //Pin to calibrate your sensor
#define FILTER_SIZE 10

// MQTT topic for IR sensor
const char* topic_publish_dht22 = "esp32/dht22";
const char* topic_publish_mq135 = "esp32/mq135"; 
//const char* topic_publish_crispVal = "esp32/crispVal";
const char* topic_publish_fanSpeed = "esp32/fanSpeed";
const char* topic_subscribe = "esp32/receive";

String fanSpeedState = "";

// Pin Instance
const int dht22 = 34; 
const int mq135 = 36;
const int pwmPin = 4;
const int hvPin = 25;

// Create instances
WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();  //Sensor Suhu
MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);  //Sensor Asap
//DHT dht(dht22, DHTTYPE);

// Variables for timing
long previous_time = 0;

//float objTemp = 0;
//float ambientTemp = 0;

bool modeAuto = true;
bool changeMode = false;
bool hvStat = true;
double aPred[9] = {0};
double z[9] = {0};
double aTot = 0;
double zTot = 0;
double zRes = 0;    
uint8_t rule = 0;
int speedRef = 0;
int manualSpeedRef = 0;
int lastSpeedVal = 0;
//Filter MQ Value
float filterBuffer[FILTER_SIZE];
int filterIndex = 0;
float mq = 0;
