void hvOn(){ 
  Serial.println("Merah Nyala");
  digitalWrite(red, HIGH);
  digitalWrite(yell, LOW);
  digitalWrite(gre, LOW);
}

void hvOff(){
  Serial.println("Kuning Nyala");
  digitalWrite(red, LOW);
  digitalWrite(yell, HIGH);
  digitalWrite(gre, LOW);
}

void fanInc(){
  Serial.println("Hijau Nyala");
  digitalWrite(red, LOW);
  digitalWrite(yell, LOW);
  digitalWrite(gre, HIGH);
}

void fanDec(){
  Serial.println("Semua Nyala");
  digitalWrite(red, HIGH);
  digitalWrite(yell, HIGH);
  digitalWrite(gre, HIGH);
}

//double fuzzy(double dhtVal, double mqVal){
//  
//  return 0.9;
//}

double crispValDht(double dhtVal, char state){  // n : normal | h : hangat | p : panas 
  double x = 0;
  if(state == 'n' || state == 'N'){     //normal
    if(dhtVal > 50){
      x = 0;
    } else if(dhtVal < 30){
      x = 1;
    } else {
      x = (50 - dhtVal) / 20;
    }
  }
  if(state == 'h' || state == 'H'){     //Hangat
    if(dhtVal <= 37 || dhtVal >=67){
      x = 0;
    } else {
      if(dhtVal <= 53){
        x = (dhtVal - 37) / 16;
      } else if (dhtVal > 53){
        x = (67 - dhtVal) / 14;
      }
    }
  }
  if(state == 'p' || state == 'P'){     //panas
    if(dhtVal < 54){
      x = 0;
    } else if(dhtVal > 75){
      x = 1;
    } else {
      x = (dhtVal - 54) / 22;
    }
  }
  return x;
}

double crispValMQ(double mqVal, char state){ //r : Rendah | s : Sedang | t : Tinggi
  double x = 0;
  if(state == 'r' || state == 'R'){     //rendah
    if(mqVal <= 60){
      x = 1;
    } else if(mqVal >= 86){
      x = 0;
    } else {
      x = (86 - mqVal) / 26;
    }
  }
  if(state == 's' || state == 'S'){     //sedang
    if(mqVal <= 73 || mqVal >=100){
      x = 0;
    } else {
      if(mqVal <= 86){
        x = (mqVal - 74) / 13;
      } else {
        x = (100 - mqVal) / 14;
      }
    }
  }
  if(state == 't' || state == 'T'){     //tinggi
    if(mqVal <= 86){
      x = 0;
    } else if (mqVal >= 101){
      x = 1;
    } else {
      x = (mqVal - 86) / 14;
    }
  }
  return x;
}
