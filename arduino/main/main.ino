#include "LoRa_E220.h"

LoRa_E220 e220ttl(&Serial1, 2, 5, 7);  //  RX AUX M0 M1

bool backend_connected = false;
byte frequency = 0xFF;
bool reference_flag = True;

struct Packet {
  char startSeq;
  char code;
  byte bar[4];
  byte temp[4];
  byte al_x[4];
  byte al_y[4];
  byte al_z[4];
  byte aa_x[4];
  byte aa_y[4];
  byte aa_z[4];
  char endSeq;
};

struct RocketData {
  byte bar1[4], bar2[4];
  byte temp1[4], temp2[4];
  byte al_x[4];
  byte al_y[4];
  byte al_z[4];
  byte aa_x[4];
  byte aa_y[4];
  byte aa_z[4];
};

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
          // TODO: REMOVE
          
          struct Packet packet = { 0xAA, 'C', { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, 0xBB };
          Serial.write((byte *)&packet, sizeof(Packet));

          //Serial.println("C");
          break;
        }
    }
  }

  if (!backend_connected) {
    // Sending Ready Signal to Backend
    
    struct Packet packet = { 0xAA, 'R', { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, 0xBB };
    Serial.write((byte *)&packet, sizeof(Packet));
    delay(250);

    //Serial.println("R");
    return;
  }

  //delay(350);

  // Serial.println("Antenna code brach");
  if (e220ttl.available() > 0) {
    Serial.println("Received");
    char code;  // first part of structure
    ResponseContainer rs = e220ttl.receiveInitialMessage(sizeof(code));
    // Put string in a char array (not needed)
    memcpy(&code, rs.data.c_str(), sizeof(code));
    /*
    Serial.print("CODE: ");
    Serial.println(code);
*/
    switch (code) {
      case 'C':
        // Listening for 'C' from the rocket, meaning succesful freq switch
        delay(450);
        e220ttl.sendMessage("C");
        Serial.write('C');
        Serial.write('\0');
        break;
      default:
        // Read the rest of structure
        ResponseStructContainer rsc = e220ttl.receiveMessageRSSI(sizeof(RocketData));
        struct RocketData data = *(RocketData *)rsc.data;
        float bar = *(float *)data.bar1;
        float temp = *(float *)data.temp1;
        float al_x = *(float *)data.al_x;
        float al_y = *(float *)data.al_y;
        float al_z = *(float *)data.al_z;
        float aa_x = *(float *)data.aa_x;
        float aa_y = *(float *)data.aa_y;
        float aa_z = *(float *)data.aa_z;

        struct Packet packet = { 0xAA, code, bar, temp, al_x, al_y, al_z, aa_x, aa_y, aa_z, 0xBB };
        memcpy(&packet.bar, (float *)data.bar1, sizeof(float));
        memcpy(&packet.temp, (float *)data.temp1, sizeof(float));
        memcpy(&packet.al_x, (float *)data.al_x, sizeof(float));
        memcpy(&packet.al_y, (float *)data.al_y, sizeof(float));
        memcpy(&packet.al_z, (float *)data.al_z, sizeof(float));
        memcpy(&packet.aa_x, (float *)data.aa_x, sizeof(float));
        memcpy(&packet.aa_y, (float *)data.aa_y, sizeof(float));
        memcpy(&packet.aa_z, (float *)data.aa_z, sizeof(float));
        /*
        Serial.print('D');
        Serial.print(',');
        Serial.print(bar);
        Serial.print(',');

        Serial.print(temp);
        Serial.print(',');
        Serial.print(al_x);
        Serial.print(',');
        Serial.print(al_y);
        Serial.print(',');
        Serial.print(al_z);
        Serial.print(',');
        Serial.print(aa_x);
        Serial.print(',');
        Serial.print(aa_y);
        Serial.print(',');
        Serial.print(aa_z);
        Serial.println();
        
        Serial.println(*(float *)data.bar, 1);
        Serial.println(*(float *)data.temp, 1);
        Serial.println(*(float *)data.al_x, 1);
        Serial.println(*(float *)data.al_y, 1);
        Serial.println(*(float *)data.al_z, 1);
        Serial.println(*(float *)data.aa_x, 1);
        Serial.println(*(float *)data.aa_y, 1);
        Serial.println(*(float *)data.aa_z, 1);
*/
        Serial.write((byte *)&packet, sizeof(Packet));
        rsc.close();
        break;
    }
  }
  /*
  Packet packet = { 0xAA, 'D', randomFloat(300,1100),randomFloat(1,10), randomFloat(1,10), randomFloat(1,10), randomFloat(1,10), randomFloat(1,10), randomFloat(1,10), randomFloat(1,10), 0xBB };
 
  Serial.write((byte *)&packet, sizeof(Packet));
  */
  delay(500);
}

float randomFloat(float minf, float maxf) {
  return minf + random(1UL << 31) * (maxf - minf) / (1UL << 31);  // use 1ULL<<63 for max double values)
}

void serializeData(struct RocketData packet){
  if (reference_flag) {
    reference = (packet.bar1+packet.bar2)*0.5;
    reference_flag = false;
  }
  float altitude = 44330 * (1.0 - pow((packet.bar1+packet.bar2)*0.5 / reference, 0.1903));
  Serial.print(altitude);
   Serial.print(" ");
  // Serial.print(",PressureAverage:");
   Serial.print((packet.bar1+packet.bar2)/2);
   Serial.print(" ");
  //Serial.print(",TemperatureAverage:");
   Serial.println((packet.temp1+packet.temp2)/2);
   Serial.print(" ");
  // Serial.print(",ax:");
   Serial.println(packet.ax);
   Serial.print(" ");
  // Serial.print(",ay:");
   Serial.println(packet.ay);
   Serial.print(" ");
  // Serial.print(",az:");
   Serial.println(packet.az);
   Serial.print(" ");
  // Serial.print(",gx:");
   Serial.println(packet.gx);
   Serial.print(" ");
  // Serial.print(",gy:");
   Serial.println(packet.gy);
   Serial.print(" ");
  // Serial.print(",gz:");
   Serial.println(packet.gz);
}