#include "header.h"
#include "functions.h"
//#include "setup.h"

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
  } else if (message == "AUTO") {
    autoMan(1);
  } else if (message == "MANUAL") {
    autoMan(2);
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
  //PWM Motor
  ledcSetup(M1A_PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttachPin(pwmPin, M1A_PWM_CHANNEL);
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
  mlx.begin();  //Sensor Suhu
  MQ135.setRegressionMethod(1); //Sensor Asap 
  MQ135.init();
  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ135.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
  }
  MQ135.setR0(calcR0/10);
  Serial.println("  done!.");
  if(isinf(calcR0)) {Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply"); while(1);}
  if(calcR0 == 0){Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply"); while(1);}
//  dht.begin();
  pinMode(dht22, INPUT);
  pinMode(mq135, INPUT);
  pinMode(hvPin, OUTPUT);
  digitalWrite(hvPin, HIGH);
}

void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();
  MQ135.update(); // Update data, the arduino will read the voltage from the analog pin
//  MQ135.setA(605.18); MQ135.setB(-3.937);    // Configure the equation to calculate CO concentration value
  MQ135.setA(110.47); MQ135.setB(-2.862);  // Configure the equation to calculate CO2 concentration value
//  float CO = MQ135.readSensor();
  float CO_raw = MQ135.readSensor();
  float CO_filtered = movingAverage(CO_raw);
  float CO_log = log10(CO_filtered + 1) * 100;  
//  objTemp = mlx.readObjectTempC();
  //  ambientTemp = mlx.readAmbientTempC();
  
//  uint8_t tempVal = map(analogRead(dht22), 0, 4095, 10, 90);
//  uint8_t tempVal = dht.readTemperature();
  float tempVal = mlx.readObjectTempC();
  float coVal = constrain(CO_log, 10, 300);
//  uint8_t coVal = map(analogRead(mq135), 0, 4095, 50, 110);

//  if (isnan(tempVal)) tempVal = 0;
//  if (tempVal > 90) tempVal = 90;
  
  String dhtVal = String(tempVal);
  String mqVal = String(coVal);

  //          asap  and suhu     kecepatan
  //rule 1 : rendah and normal = normal
  rule = 0;
  aPred[rule] = min(crispValMQ(coVal, 'r'),crispValDht(tempVal, 'n'));
  z[rule] = min(aPred[rule] == 0.00 ? 80 : 180 - (100 * aPred[rule]), double(240));
  //rule 2 : rendah and hangat = normal
  rule = 1;
  aPred[rule] = min(crispValMQ(coVal, 'r'),crispValDht(tempVal, 'h'));
  z[rule] = min(aPred[rule] == 0.00 ? 80 : 180 - (100 * aPred[rule]), double(240));
  //rule 3 : rendah and panas  = normal
  rule = 2;
  aPred[rule] = min(crispValMQ(coVal, 'r'),crispValDht(tempVal, 'p'));
  z[rule] = min(aPred[rule] == 0.00 ? 80 : 180 - (100 * aPred[rule]), double(240));
  //rule 4 : sedang and normal = normal
  rule = 3;
  aPred[rule] = min(crispValMQ(coVal, 's'),crispValDht(tempVal, 'n'));
  z[rule] = min(aPred[rule] == 0.00 ? 80 : 180 - (100 * aPred[rule]), double(240));
  //rule 5 : sedang and hangat = tinggi
  rule = 4;
  aPred[rule] = min(crispValMQ(coVal, 's'),crispValDht(tempVal, 'h'));
  z[rule] = min(aPred[rule] == 0.00 ? 110 : 110 + (130 * aPred[rule]), double(240));
  //rule 6 : sedang and panas  = tinggi
  rule = 5;
  aPred[rule] = min(crispValMQ(coVal, 's'),crispValDht(tempVal, 'p'));
  z[rule] = min(aPred[rule] == 0.00 ? 110 : 110 + (130 * aPred[rule]), double(240));
  //rule 7 : tinggi and normal = normal
  rule = 6;
  aPred[rule] = min(crispValMQ(coVal, 't'),crispValDht(tempVal, 'n'));
  z[rule] = min(aPred[rule] == 0.00 ? 80 : 180 - (100 * aPred[rule]), double(240));
  //rule 8 : tinggi and hangat = tinggi
  rule = 7;
  aPred[rule] = min(crispValMQ(coVal, 't'),crispValDht(tempVal, 'h'));
  z[rule] = min(aPred[rule] == 0.00 ? 110 : 110 + (130 * aPred[rule]), double(240));
  //rule 9 : tinggi and panas  = tinggi
  rule = 8;
  aPred[rule] = min(crispValMQ(coVal, 't'),crispValDht(tempVal, 'p'));
  z[rule] = min(aPred[rule] == 0.00 ? 110 : 110 + (130 * aPred[rule]), double(240));
  
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

  if (hvStat == false){            //Jika hv menyala, maka kipas akan menyala dan secara default mengguanakn perhitungan fuzzy
    if(modeAuto == true) {        //Ketika modeAuto true, maka kecepatan referensi berasal dari perhitungan fuzzy
      speedRef = int(zRes);       //Inisiasi speedRef dengan nilai dari perhitungan fuzzy
      if (changeMode == false) changeMode = true;
    } else {
      speedRef = manualSpeedRef;  //Inisialisasi speedRef dengan kecepatan manual
      if (changeMode == true) {   //Ketika modeAuto false, 
       changeMode = false;        //mentrigger perubahan mode,
       manualSpeedRef = zRes;     //untuk mengambil nilai terakhir dari perhitungan fuzzy
      } 
    }
  } else {                        //Jika hv mati, maka kipas mati
    speedRef = 0;
  }
  
  String publishedSpeed = String(speedRef);
  long now = millis();
  
  if (now - previous_time > 1000) { //Mempublish setiap satu detik
    previous_time = now;
    
    if(modeAuto == true){
      Serial.print("Mode : Auto | ");
    } else {
      Serial.print("Mode : Manual | ");
    }
    
    if(hvStat == true){
      Serial.print("HV : Mati | ");
    } else {
      Serial.print("HV : Nyala | ");
    }
    
    Serial.println(String()+F("Speed : ")+speedRef);
    // Publish the sensor value to the MQTT topic
    mqttClient.publish(topic_publish_dht22, dhtVal.c_str());
    mqttClient.publish(topic_publish_mq135, mqVal.c_str());
    mqttClient.publish(topic_publish_fanSpeed, publishedSpeed.c_str());
  }
  
  ledcWrite(M1A_PWM_CHANNEL, min(speedRef, 255)); //Mengendalikan kecepatan kipas pada pin kipas
  
}


//DUMP
//    Serial.println(String()+F("Suhu : ")+dhtVal+F("°C | Asap : ")+mqVal+F(" ppm = Fan Speed : ")+zRes+F("Mode : ")+modeAuto);
//    Serial.print("a=");
//    for (int i = 0; i < 8; i++){
//      Serial.print(aPred[i]);
//      Serial.print("|");
//    }
//    Serial.println(aPred[8]);
//    Serial.print("z=");
//    for (int i = 0; i < 8; i++){
//      Serial.print(z[i]);
//      Serial.print("|");
//    }
//    Serial.println(z[8]);   
//    
//    Serial.println(String()+F(" | Speed : ")+zRes);
