#include "Arduino.h"
#include "LoRa_E220.h"
#define ENABLE_RSSI true

LoRa_E220 e220ttl(&Serial1, 2, 4, 6); //  RX AUX M0 M1

bool backend_connected = false;
byte frequency = 0xFF;

void setup() {

  Serial.begin(9600);
  randomSeed(analogRead(0));
  // Sending Ready Signal to Backend
  e220ttl.begin();
  pinMode(8, OUTPUT);
  pinMode(6,OUTPUT);
  pinMode(4,OUTPUT);
  delay(500);
  Serial.print('R');
}

void loop() {

  if (!backend_connected || frequency == 0xFF) return;

  if (e220ttl.available() > 1) {

    #ifdef ENABLE_RSSI 
      ResponseContainer rc = e220ttl.receiveMessageRSSI();
    #endif

    Serial.println(rc.status.getResponseDescription());    
    Serial.println(rc.data); 
    
    Serial.print("RSSI: "); 
    Serial.println(rc.rssi, DEC);
    Serial.println();

    digitalWrite(8, LOW);    

  } else {
    Serial.println("Waiting...");
    Serial.println();

    digitalWrite(8, HIGH);
    delay(500);
  }
  
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

          auto c = e220ttl.getConfiguration();
          Configuration configuration = *((Configuration*) c.data);
          configuration.CHAN = frequency;
          auto rs = e220ttl.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
          Serial.println(rs.getResponseDescription());
          Serial.println(rs.code);
          c.close();
          // code ti da un errore forse?
          // emulating COM CHECK or no response
          int randNumber = random(10);
          if (randNumber > 5)
            Serial.write('C');
          break;
        }
    }
    free(inChar);
  }
}
