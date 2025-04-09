void hvOn(){ //Menyalakan prototip 
  Serial.println("Electrostatis Nyala");
  digitalWrite(hvPin, HIGH);
  hvStat = true;
}

void hvOff(){ //mematikan prototip 
  Serial.println("Electrostatis Mati");
  digitalWrite(hvPin, LOW);
  hvStat = false;
}

void fanInc(){ //menambah kecepatan kipas jika dirubah menjadi mode manual
  if (manualSpeedRef <= 240){
    manualSpeedRef+=10;
  }
}

void fanDec(){ //mengurangi kecepatan kipas jika dirubah menjadi mode manual
  if (manualSpeedRef >= 20){
    manualSpeedRef-=10;
  }
}

void autoMan(uint8_t state){
  if(state == 1) {
    Serial.println("Mode Auto");
    modeAuto = true;
  }
  if(state == 2){
    Serial.println("Mode Manual");
    modeAuto = false;
  }
}

double crispValDht(double dhtVal, char state){  // n : normal | h : hangat | p : panas 
  double x = 0;
  if(state == 'n' || state == 'N'){     //normal
    if (dhtVal >= 50) x = 0;
    else if (dhtVal <= 30) x = 1;
    else  x = (50 - dhtVal) / 20;
  }
  if(state == 'h' || state == 'H'){     //Hangat
    if (dhtVal <= 37 || dhtVal >= 67) x = 0;
    else if (dhtVal <= 53) x = (dhtVal - 37) / 16;
    else x = (67 - dhtVal) / 14;
  }
  if(state == 'p' || state == 'P'){     //panas
    if (dhtVal <= 53) x = 0;
    else if (dhtVal >= 75) x = 1;
    else x = (dhtVal - 53) / 22;
  }
  return x;
}

double crispValMQ(double mqVal, char state){ //r : Rendah | s : Sedang | t : Tinggi
  double x = 0;
  if(state == 'r' || state == 'R'){     //rendah
    if (mqVal <= 60) x = 1;
    else if (mqVal >= 86) x = 0;
    else x = (86 - mqVal) / 26;
  }
  if(state == 's' || state == 'S'){     //sedang
    if (mqVal <= 73 || mqVal >= 100) x = 0;
    else if (mqVal <= 86) x = (mqVal - 73) / 13;
    else x = (100 - mqVal) / 14;
  }
  if(state == 't' || state == 'T'){     //tinggi
    if (mqVal <= 86) x = 0;
    else if (mqVal >= 100) x = 1;
    else x = (mqVal - 86) / 14;
  }
  return x;
}
