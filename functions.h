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

double fuzzy(double dhtVal, double mqVal){
  
  return 0.9;
}
