bool backend_connected = false;

void setup() {

  Serial.begin(9600);
  randomSeed(analogRead(0));
  // Sending Ready Signal to Backend
  Serial.print('R');
}

void loop() {

  // waiting for backend ready signal
  if (backend_connected) {

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
}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs.
*/
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == 'B') {
      backend_connected = true;
    }
  }
}
