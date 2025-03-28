#include "header.h"
#include "functions.h"

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  if (message == "HVON") {
    hvOn();
  } else if (message == "HVOFF") {
    hvOff();
  } else if (message == "FANINC") {
    fanInc();
  } else if (message == "FANDEC") {
    fanDec();
  }
}

void setupMQTT() {
  mqttClient.setServer(mqtt_broker, mqtt_port);
  mqttClient.setCallback(mqttCallback);
}

void reconnect() {
  Serial.println("Connecting to MQTT Broker...");
  while (!mqttClient.connected()) {
    Serial.println("Reconnecting to MQTT Broker...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    if (mqttClient.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Connected to MQTT Broker.");
      mqttClient.subscribe(topic_subscribe);
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(red, OUTPUT);
  pinMode(yell, OUTPUT);
  pinMode(gre, OUTPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to Wi-Fi");

  // Initialize secure WiFiClient
  wifiClient.setInsecure(); // Use this only for testing, it allows connecting without a root certificate
  
  setupMQTT();

  pinMode(dht22, INPUT);
  pinMode(mq135, INPUT);
}

void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();
  
  uint8_t dht_int = map(analogRead(dht22), 0, 4095, 0, 100);
  uint8_t mq_int = map(analogRead(mq135), 0, 4095, 0, 100);
  double crispVal_double = fuzzy(double(dht_int), double(mq_int));
  String dhtVal = String(dht_int);
  String mqVal = String(mq_int);
  String crispVal = String(crispVal_double);
  fanSpeedState = (crispVal_double > 0 && crispVal_double < 50) ? "Normal" : "Cepat";

  long now = millis();
  if (now - previous_time > 1000) { // Publish every 1 seconds
    previous_time = now;
        
    // Publish the sensor value to the MQTT topic
    Serial.print(dhtVal);
    Serial.print(" : ");
    Serial.print(mqVal);
    Serial.print(" : ");
    Serial.print(crispVal);
    Serial.print(" : ");
    Serial.println(fanSpeedState);
    mqttClient.publish(topic_publish_dht22, dhtVal.c_str());
    mqttClient.publish(topic_publish_mq135, mqVal.c_str());
    mqttClient.publish(topic_publish_crispVal, crispVal.c_str());
    mqttClient.publish(topic_publish_fanSpeed, fanSpeedState.c_str());
  }
}
