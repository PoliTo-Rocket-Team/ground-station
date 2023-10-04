#include "LoRa_E220.h"

#define FCHANGE_TIMEOUT 2000

LoRa_E220 e220ttl(&Serial1, 2, 5, 7);  //  RX AUX M0 M1

bool backend_connected = false;
byte frequency = 0xFF;

void changeFrequency(byte);
void sendCharToApp(char);

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
          
          byte new_frequency;
          Serial.readBytes(&new_frequency, 1);
          if(new_frequency == 0xFF) {
            // if first time, simply set freq
            ResponseStructContainer c = e220ttl.getConfiguration();
            Configuration configuration = *((Configuration *)c.data);
            configuration.CHAN = frequency = new_frequency;
            e220ttl.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
            c.close();
            sendCharToApp('C');
          }
          else {
            if(frequency != new_frequency) {
              changeFrequency(new_frequency);
            }
          }
          break;
        }
    }
  }

  if (!backend_connected) {
    // Sending Ready Signal to Backend
    sendCharToApp('R');
    delay(250);
    return;
  }

  if (e220ttl.available() > 0) {

    ResponseStructContainer rsc = e220ttl.receiveMessageRSSI(sizeof(RocketData));
    packet = *(RocketData*)rsc.data;
    rsc.close();
    switch (packet.code) {
      case 'C':
        {
          e220ttl.sendMessage("C");
          sendCharToApp('C');
          break;
        }
      case 'D':
        {
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

void sendCharToApp(char c) {
  Serial.write(0xAA);
  Serial.write(c);
  Serial.write(0xBB);
}

void changeFrequency(byte new_freq) {
  ResponseStructContainer c;
  Configuration config;
  ResponseStructContainer incoming;
  bool ok;
  unsigned int old_freq = frequency;

  // c = e220ttl.getConfiguration();
  // config = *(Configuration*)c.data;
  // c.close();
  // old_freq = config.CHAN;

  char msg[] = "F0";
  msg[1] = new_freq;

  ok = false;
  int counter = 0;
  while (counter < 10) {
    for (int i = 0; i < 5; i++) {
      e220ttl.sendMessage(msg);
      delay(50);
    }
    config.CHAN = new_freq;
    e220ttl.setConfiguration(config, WRITE_CFG_PWR_DWN_SAVE);
    long start = millis();
    do {
      // Waiting for ack
      delay(100);
      if (e220ttl.available()) {
        // I received something 
        incoming = e220ttl.receiveMessage(sizeof(RocketData));
        packet = *(RocketData*)incoming.data;
        incoming.close();
        if (packet.code == 'C' || packet.code == 'D') {
          e220ttl.sendMessage("C");
          sendCharToApp('C');
          ok = true;
        }
      }
    } while (!ok && millis() - start < FCHANGE_TIMEOUT);

    if (ok) break;
    // switching back to old frequency
    config.CHAN = old_freq;
    e220ttl.setConfiguration(config, WRITE_CFG_PWR_DWN_SAVE);
    counter++;
  }
  frequency = ok ? new_freq : 0xFF;
}