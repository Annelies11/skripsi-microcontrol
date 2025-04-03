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
  
  uint8_t dht_int = map(analogRead(dht22), 0, 4095, 10, 90);
  uint8_t mq_int = map(analogRead(mq135), 0, 4095, 50, 110);
  
  String dhtVal = String(dht_int);
  String mqVal = String(mq_int);

  //          asap  and suhu     kecepatan
  //rule 1 : rendah and normal = normal
  rule = 0;
  aPred[rule] = min(crispValMQ(mq_int, 'r'),crispValDht(dht_int, 'n'));
  z[rule] = min(aPred[rule] == 0.00 ? 80 : 180 - (100 * aPred[rule]), double(240));
  //rule 2 : rendah and hangat = normal
  rule = 1;
  aPred[rule] = min(crispValMQ(mq_int, 'r'),crispValDht(dht_int, 'h'));
  z[rule] = min(aPred[rule] == 0.00 ? 80 : 180 - (100 * aPred[rule]), double(240));
  //rule 3 : rendah and panas  = normal
  rule = 2;
  aPred[rule] = min(crispValMQ(mq_int, 'r'),crispValDht(dht_int, 'p'));
  z[rule] = min(aPred[rule] == 0.00 ? 80 : 180 - (100 * aPred[rule]), double(240));
  //rule 4 : sedang and normal = normal
  rule = 3;
  aPred[rule] = min(crispValMQ(mq_int, 's'),crispValDht(dht_int, 'n'));
  z[rule] = min(aPred[rule] == 0.00 ? 80 : 180 - (100 * aPred[rule]), double(240));
  //rule 5 : sedang and hangat = tinggi
  rule = 4;
  aPred[rule] = min(crispValMQ(mq_int, 's'),crispValDht(dht_int, 'h'));
  z[rule] = min(aPred[rule] == 0.00 ? 110 : 110 + (130 * aPred[rule]), double(240));
  //rule 6 : sedang and panas  = tinggi
  rule = 5;
  aPred[rule] = min(crispValMQ(mq_int, 's'),crispValDht(dht_int, 'p'));
  z[rule] = min(aPred[rule] == 0.00 ? 110 : 110 + (130 * aPred[rule]), double(240));
  //rule 7 : tinggi and normal = normal
  rule = 6;
  aPred[rule] = min(crispValMQ(mq_int, 't'),crispValDht(dht_int, 'n'));
  z[rule] = min(aPred[rule] == 0.00 ? 80 : 180 - (100 * aPred[rule]), double(240));
  //rule 8 : tinggi and hangat = tinggi
  rule = 7;
  aPred[rule] = min(crispValMQ(mq_int, 't'),crispValDht(dht_int, 'h'));
  z[rule] = min(aPred[rule] == 0.00 ? 110 : 110 + (130 * aPred[rule]), double(240));
  //rule 9 : tinggi and panas  = tinggi
  rule = 8;
  aPred[rule] = min(crispValMQ(mq_int, 't'),crispValDht(dht_int, 'p'));
  z[rule] = min(aPred[rule] == 0.00 ? 110 : 110 + (130 * aPred[rule]), 240.00);
  
  aTot = 0;
  zTot = 0;
  for(int i = 0; i < 9; i++){
    aTot = aTot + aPred[i];
    zTot = zTot + (aPred[i]*z[i]);
  }
  // Pemberian nilai default jika tidak ada rule aktif
  if (aTot > 0) {
    zRes = zTot / aTot;
  } else {
    zRes = 80; 
  }
//  zRes = zTot / aTot;
  String fanSpeed = String(zRes);
  long now = millis();
  if (now - previous_time > 1000) { // Publish every 1 seconds
    previous_time = now;
  
    Serial.println(String()+F("Suhu : ")+dhtVal+F("°C | Asap : ")+mqVal+F(" ppm = Fan Speed : ")+zRes);
    Serial.print("a=");
    for (int i = 0; i < 8; i++){
      Serial.print(aPred[i]);
      Serial.print("|");
    }
    Serial.println(aPred[8]);
    Serial.print("z=");
    for (int i = 0; i < 8; i++){
      Serial.print(z[i]);
      Serial.print("|");
    }
    Serial.println(z[8]);   
    
    // Publish the sensor value to the MQTT topic
    mqttClient.publish(topic_publish_dht22, dhtVal.c_str());
    mqttClient.publish(topic_publish_mq135, mqVal.c_str());
    mqttClient.publish(topic_publish_fanSpeed, fanSpeed.c_str());
  }
  
}
