#include <Wire.h>

#include <Adafruit_Sensor.h>

#include <Adafruit_BNO055.h>

#include <ELECHOUSE_CC1101_SRC_DRV.h>


struct Packet {
  unsigned long time;

  imu::Quaternion quat; // orientation quaternion

  float ax;
  float ay;
  float az;

  float latitude;
  float longitude;
  int altitude; // gps altitude in meters

  int satellites; // number of satellites

  int channels[7];


  /*
  MODES:
  0 - manual
  1 - take-off
  2 - fly-by-wire

  255 - recovery
  */
  int mode;

};

float yaw;
float pitch;
float roll;


Packet p;

// Seems to be the same as Adafruit's implementation
void quat2ZYX(imu::Quaternion q, float &yaw, float &pitch, float &roll) {
  double sinr_cosp = 2 * (q.w() * q.x() + q.y() * q.z());
  double cosr_cosp = 1 - 2 * (q.x() * q.x() + q.y() * q.y());
  roll = atan2(sinr_cosp, cosr_cosp);

  // pitch (y-axis rotation)
  double sinp = sqrt(1 + 2 * (q.w() * q.y() - q.x() * q.z()));
  double cosp = sqrt(1 - 2 * (q.w() * q.y() - q.x() * q.z()));
  pitch = 2 * atan2(sinp, cosp) - M_PI / 2;

  // yaw (z-axis rotation)
  double siny_cosp = 2 * (q.w() * q.z() + q.x() * q.y());
  double cosy_cosp = 1 - 2 * (q.y() * q.y() + q.z() * q.z());
  yaw = atan2(siny_cosp, cosy_cosp);

  // Convert radians to degrees
  yaw   *= 180 / 3.1415;
  pitch *= 180 / 3.1415;
  roll  *= 180 / 3.1415;
}

void printPacket(Packet p){

  imu::Vector<3> euler;
  euler = p.quat.toEuler();

  yaw   = euler.x() * 180 / 3.1415;
  pitch = euler.y() * 180 / 3.1415;
  roll  = euler.z() * 180 / 3.1415;

  quat2ZYX(p.quat, yaw, pitch, roll);

  Serial.print("Time: ");
  Serial.println(p.time);

  Serial.print("Yaw: ");
  Serial.println(yaw, 4);

  Serial.print("Pitch: ");
  Serial.println(pitch, 4);

  Serial.print("Roll: ");
  Serial.println(roll, 4);

  Serial.print("q1: ");
  Serial.println(p.quat.w(), 10);

  Serial.print("q2: ");
  Serial.println(p.quat.x(), 10);

  Serial.print("q3: ");
  Serial.println(p.quat.y(), 10);

  Serial.print("q4: ");
  Serial.println(p.quat.z(), 10);

  // max precision for acceleration is 2 decimals
  Serial.print("ax: ");
  Serial.println(p.ax, 2);

  Serial.print("ay: ");
  Serial.println(p.ay, 2);

  Serial.print("az: ");
  Serial.println(p.az, 2);

  Serial.print("Latitude: ");
  Serial.println(p.latitude, 10);

  Serial.print("Longitude: ");
  Serial.println(p.longitude, 10);

  Serial.print("Altitude: ");
  Serial.println((float) p.altitude, 3);

  Serial.print("Satellites: ");
  Serial.println(p.satellites);
  
  Serial.print("Channels: \n");
  for(int i = 0; i < 7; i++){
    Serial.print(p.channels[i]);
    Serial.print(" ");
  }
  Serial.println();

  Serial.print("Mode: ");
  Serial.println((int) p.mode);

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




  for (int i = 0x04; i <= 0x28; i++){
    uint8_t reg = ELECHOUSE_cc1101.SpiReadReg(i);
    Serial.print("TI_write_reg(0x");
    Serial.print(i, HEX);
    Serial.print(", 0x");
    Serial.print(reg, HEX);
    Serial.print(");\n");
  }



}

void loop() {
    //Checks whether something has been received.
    //When something is received we give some time to receive the message in full.(time in millis)
    if (ELECHOUSE_cc1101.CheckRxFifo(8)){
      
     
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