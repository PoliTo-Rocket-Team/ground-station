#include "LoRa_E220.h"

LoRa_E220 e220ttl(&Serial1, 2, 5, 7);  //  RX AUX M0 M1

bool backend_connected = false;
byte frequency = 0xFF;


struct RocketData {
  char code;
  float bar1, bar2;
  float temp1, temp2;
  float al_x;
  float al_y;
  float al_z;
  float aa_x;
  float aa_y;
  float aa_z;
} packet;

void setup() {
  randomSeed(analogRead(0));
  Serial.begin(9600);
  e220ttl.begin();
  delay(500);

  ResponseStructContainer c = e220ttl.getConfiguration();
  Configuration configuration = *((Configuration *)c.data);
  configuration.CHAN = 23;
  e220ttl.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
  c.close();
}

void loop() {
  if (Serial.available()) {
    char inChar;
    Serial.readBytes(&inChar, 1);
    switch (inChar) {
      case 'B':
        backend_connected = true;
        break;
      case 'F':
        {
          Serial.readBytes(&inChar, 1);
          frequency = inChar;
          char msg[2] = { 'F', frequency };
          // Sending msg to the rocket on current freq to change freq
          ResponseStatus rs = e220ttl.sendMessage(msg);
          // Switching freq on GS Antenna to new freq
          ResponseStructContainer c = e220ttl.getConfiguration();
          Configuration configuration = *((Configuration *)c.data);

          configuration.CHAN = frequency;
          rs = e220ttl.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
          c.close();

          Serial.write(0xAA);
          Serial.write('C');
          Serial.write(0xBB);
          //Serial.println("C");
          break;
        }
    }
  }

  if (!backend_connected) {
    // Sending Ready Signal to Backend
    Serial.write(0xAA);
    Serial.write('R');
    Serial.write(0xBB);
    delay(250);
    return;
  }

  //delay(350);

  // Serial.println("Antenna code brach");
  if (e220ttl.available() > 0) {

    ResponseStructContainer rsc = e220ttl.receiveMessageRSSI(sizeof(RocketData));
    packet = *(RocketData*)rsc.data;
    rsc.close();
    //Serial.println(packet.code);
    switch (packet.code) {
      case 'C':
        {
          e220ttl.sendMessage("C");
          Serial.write(0xAA);
          Serial.write('C');
          Serial.write(0xBB);
          break;
        }
      case 'D':
        {
          // handleData(packet);
          // printData(packet);
          Serial.write(0xAA);
          Serial.write((byte*) &packet, sizeof(RocketData));
          Serial.write(0xBB);
          break;
        }
      default:
        break;
    }
  }
  delay(500);
}

float randomFloat(float minf, float maxf) {
  return minf + random(1UL << 31) * (maxf - minf) / (1UL << 31);  // use 1ULL<<63 for max double values)
}
