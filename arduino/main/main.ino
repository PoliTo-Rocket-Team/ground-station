void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
    randomSeed(analogRead(0));
    
}

void loop() {
  // put your main code here, to run repeatedly:
  int randNumber;
  // COM CHECK
  Serial.write('C');
    // Emulating errors
    randNumber = random(40);
  if (randNumber > 30) {
    Serial.write('E');
    Serial.write('1');
    return;
  }

  if (randNumber > 20) {
    Serial.write('E');
    Serial.write('2');
    return;
  }

  if (randNumber > 10) {
    Serial.write('E');
    Serial.write('3');
    return;
  }

  // ALL OK
  Serial.write('O');
  delay(5000);
}
