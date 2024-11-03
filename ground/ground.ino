#include <Wire.h>
#include <ELECHOUSE_CC1101_SRC_DRV.h>

struct Packet{
  unsigned long time;

  float yaw;
  float pitch;
  float roll;

  float ax;
  float ay;
  float az;

  int channels[14];
};


Packet p;

void printPacket(Packet p){


  Serial.print("Time: ");
  Serial.println(p.time);

  Serial.print("Yaw: ");
  Serial.println(p.yaw);

  Serial.print("Pitch: ");
  Serial.println(p.pitch);

  Serial.print("Roll: ");
  Serial.println(p.roll);

  Serial.print("ax: ");
  Serial.println(p.ax);

  Serial.print("ay: ");
  Serial.println(p.ay);

  Serial.print("az: ");
  Serial.println(p.az);

  Serial.print("Channels: \n");
  for(int i = 0; i < 14; i++){
    Serial.print(p.channels[i]);
    Serial.print(" ");
  }
  Serial.println();
}

void setup() {

  Serial.begin(115200);  // start serial for output

  if (ELECHOUSE_cc1101.getCC1101()){        // Check the CC1101 Spi connection.
    Serial.println("CC1101: Connection OK");
  }else{
    Serial.println("CC1101: Connection ERROR");
  }

  ELECHOUSE_cc1101.Init();              // must be set to initialize the cc1101!
  ELECHOUSE_cc1101.setCCMode(1);       // set config for internal transmission mode.
  ELECHOUSE_cc1101.setModulation(0);  // set modulation mode. 0 = 2-FSK, 1 = GFSK, 2 = ASK/OOK, 3 = 4-FSK, 4 = MSK.

  ELECHOUSE_cc1101.setMHZ(433.92);   // Here you can set your basic frequency. The lib calculates the frequency automatically (default = 433.92).The cc1101 can: 300-348 MHZ, 387-464MHZ and 779-928MHZ. Read More info from datasheet.
  ELECHOUSE_cc1101.setSyncMode(2);  // Combined sync-word qualifier mode. 0 = No preamble/sync. 1 = 16 sync word bits detected. 2 = 16/16 sync word bits detected. 3 = 30/32 sync word bits detected. 4 = No preamble/sync, carrier-sense above threshold. 5 = 15/16 + carrier-sense above threshold. 6 = 16/16 + carrier-sense above threshold. 7 = 30/32 + carrier-sense above threshold.
  ELECHOUSE_cc1101.setCrc(1);      // 1 = CRC calculation in TX and CRC check in RX enabled. 0 = CRC disabled for TX and RX.

}


void loop() {
    //Checks whether something has been received.
    //When something is received we give some time to receive the message in full.(time in millis)
    if (ELECHOUSE_cc1101.CheckRxFifo(100)){
      if (ELECHOUSE_cc1101.CheckCRC()){    //CRC Check. If "setCrc(false)" crc returns always OK!
        
        Serial.println("GND PACKET");
        Serial.print("Rssi: ");
        Serial.println(ELECHOUSE_cc1101.getRssi());
        Serial.print("LQI: ");
        Serial.println(ELECHOUSE_cc1101.getLqi());
        
        int len = ELECHOUSE_cc1101.ReceiveData((void*) &p);

        printPacket(p);

        Serial.println();

      }
    }
}