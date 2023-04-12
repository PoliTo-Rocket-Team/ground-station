bool backend_connected = false;
byte frequency = 0xFF;

void setup() {

  Serial.begin(9600);
  randomSeed(analogRead(0));
  // Sending Ready Signal to Backend
  Serial.print('R');
}

void loop() {
/*
  // waiting for backend ready signal
  if (backend_connected && frequency != 0xFF) {

    int randNumber;
    // COM CHECK

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
  */
}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs.
*/
void serialEvent() {
  while (Serial.available()) {
    char* inChar = (char*)malloc(1);
    Serial.readBytes(inChar, 1);
    switch (*inChar) {
      case 'B':
        {
          backend_connected = true;
          break;
        }
      case 'F':
        {
          Serial.readBytes(inChar, 1);
          frequency = *inChar;
          // emulating COM CHECK or no response
          int randNumber = random(10);
          if (randNumber > 5)
            Serial.write('C');
          break;
        }
    }
  }
}
