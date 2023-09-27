#include "LoRa_E220.h"

LoRa_E220 e220ttl(&Serial1, 2, 4, 6);  //  RX AUX M0 M1

bool backend_connected = false;
byte frequency = 0xFF;

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

void setup() {
  randomSeed(analogRead(0));
  Serial.begin(9600);
  e220ttl.begin();
  delay(500);
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

          configuration.CHAN = 23;
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

    //Serial.println("R");
    return;
  }

  //delay(350);

  if (e220ttl.available() > 1) {
    struct AntennaData {  // Partial structure without type and start sequence
      byte bar[4];
      byte temp[4];
      byte al_x[4];
      byte al_y[4];
      byte al_z[4];
      byte aa_x[4];
      byte aa_y[4];
      byte aa_z[4];
    };
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
        ResponseStructContainer rsc = e220ttl.receiveMessageRSSI(sizeof(AntennaData));
        struct AntennaData data = *(AntennaData *)rsc.data;
        float bar = *(float *)data.bar;
        float temp = *(float *)data.temp;
        float al_x = *(float *)data.al_x;
        float al_y = *(float *)data.al_y;
        float al_z = *(float *)data.al_z;
        float aa_x = *(float *)data.aa_x;
        float aa_y = *(float *)data.aa_y;
        float aa_z = *(float *)data.aa_z;

        struct Packet packet = { 0xAA, code, bar, temp, al_x, al_y, al_z, aa_x, aa_y, aa_z, 0xBB };
        memcpy(&packet.bar, (float *)data.bar, sizeof(float));
        memcpy(&packet.temp, (float *)data.temp, sizeof(float));
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


void printParameters(struct Configuration configuration) {
  DEBUG_PRINTLN("----------------------------------------");

  DEBUG_PRINT(F("HEAD : "));
  DEBUG_PRINT(configuration.COMMAND, HEX);
  DEBUG_PRINT(" ");
  DEBUG_PRINT(configuration.STARTING_ADDRESS, HEX);
  DEBUG_PRINT(" ");
  DEBUG_PRINTLN(configuration.LENGHT, HEX);
  DEBUG_PRINTLN(F(" "));
  DEBUG_PRINT(F("AddH : "));
  DEBUG_PRINTLN(configuration.ADDH, HEX);
  DEBUG_PRINT(F("AddL : "));
  DEBUG_PRINTLN(configuration.ADDL, HEX);
  DEBUG_PRINTLN(F(" "));
  DEBUG_PRINT(F("Chan : "));
  DEBUG_PRINT(configuration.CHAN, DEC);
  DEBUG_PRINT(" -> ");
  DEBUG_PRINTLN(configuration.getChannelDescription());
  DEBUG_PRINTLN(F(" "));
  DEBUG_PRINT(F("SpeedParityBit     : "));
  DEBUG_PRINT(configuration.SPED.uartParity, BIN);
  DEBUG_PRINT(" -> ");
  DEBUG_PRINTLN(configuration.SPED.getUARTParityDescription());
  DEBUG_PRINT(F("SpeedUARTDatte     : "));
  DEBUG_PRINT(configuration.SPED.uartBaudRate, BIN);
  DEBUG_PRINT(" -> ");
  DEBUG_PRINTLN(configuration.SPED.getUARTBaudRateDescription());
  DEBUG_PRINT(F("SpeedAirDataRate   : "));
  DEBUG_PRINT(configuration.SPED.airDataRate, BIN);
  DEBUG_PRINT(" -> ");
  DEBUG_PRINTLN(configuration.SPED.getAirDataRateDescription());
  DEBUG_PRINTLN(F(" "));
  DEBUG_PRINT(F("OptionSubPacketSett: "));
  DEBUG_PRINT(configuration.OPTION.subPacketSetting, BIN);
  DEBUG_PRINT(" -> ");
  DEBUG_PRINTLN(configuration.OPTION.getSubPacketSetting());
  DEBUG_PRINT(F("OptionTranPower    : "));
  DEBUG_PRINT(configuration.OPTION.transmissionPower, BIN);
  DEBUG_PRINT(" -> ");
  DEBUG_PRINTLN(configuration.OPTION.getTransmissionPowerDescription());
  DEBUG_PRINT(F("OptionRSSIAmbientNo: "));
  DEBUG_PRINT(configuration.OPTION.RSSIAmbientNoise, BIN);
  DEBUG_PRINT(" -> ");
  DEBUG_PRINTLN(configuration.OPTION.getRSSIAmbientNoiseEnable());
  DEBUG_PRINTLN(F(" "));
  DEBUG_PRINT(F("TransModeWORPeriod : "));
  DEBUG_PRINT(configuration.TRANSMISSION_MODE.WORPeriod, BIN);
  DEBUG_PRINT(" -> ");
  DEBUG_PRINTLN(configuration.TRANSMISSION_MODE.getWORPeriodByParamsDescription());
  DEBUG_PRINT(F("TransModeEnableLBT : "));
  DEBUG_PRINT(configuration.TRANSMISSION_MODE.enableLBT, BIN);
  DEBUG_PRINT(" -> ");
  DEBUG_PRINTLN(configuration.TRANSMISSION_MODE.getLBTEnableByteDescription());
  DEBUG_PRINT(F("TransModeEnableRSSI: "));
  DEBUG_PRINT(configuration.TRANSMISSION_MODE.enableRSSI, BIN);
  DEBUG_PRINT(" -> ");
  DEBUG_PRINTLN(configuration.TRANSMISSION_MODE.getRSSIEnableByteDescription());
  DEBUG_PRINT(F("TransModeFixedTrans: "));
  DEBUG_PRINT(configuration.TRANSMISSION_MODE.fixedTransmission, BIN);
  DEBUG_PRINT(" -> ");
  DEBUG_PRINTLN(configuration.TRANSMISSION_MODE.getFixedTransmissionDescription());


  DEBUG_PRINTLN("----------------------------------------");
}
