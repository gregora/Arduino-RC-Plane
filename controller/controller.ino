#include <Wire.h>
#include <Adafruit_Sensor.h>

#include <Adafruit_BNO055.h>
#include <Servo.h>

#include <IBusBM.h>
#include <SoftwareSerial.h>


#include <ELECHOUSE_CC1101_SRC_DRV.h>

Servo ch1;
Servo ch2;
Servo ch3;
Servo ch4;
Servo ch5;

Adafruit_BNO055 bno = Adafruit_BNO055(55);

IBusBM IBus;

bool bno_status = false;
bool cc1101_status = false;

struct Packet{
  float yaw;
  float pitch;
  float roll;
  int channels[14];
};

Packet p;


void setup() {
  // attach the motors to corresponding pins
  ch1.attach(2);
  ch2.attach(3);
  ch3.attach(4);
  ch4.attach(5);
  ch5.attach(6);
  
  //Serial.begin(115200);

  IBus.begin(Serial); // use Serial to read flysky receiver iBUS

  if(!bno.begin()){
    bno_status = false;
    //Serial.println("BNO055: Connection ERROR");
  }

  if (ELECHOUSE_cc1101.getCC1101()){        // Check the CC1101 Spi connection.
    //Serial.println("CC1101: Connection OK");
    cc1101_status = true;
  }else{
    //Serial.println("CC1101: Connection ERROR");
  }

  ELECHOUSE_cc1101.Init();              // must be set to initialize the cc1101!
  ELECHOUSE_cc1101.setCCMode(1);       // set config for internal transmission mode.
  ELECHOUSE_cc1101.setModulation(0);  // set modulation mode. 0 = 2-FSK, 1 = GFSK, 2 = ASK/OOK, 3 = 4-FSK, 4 = MSK.
  ELECHOUSE_cc1101.setMHZ(433.92);   // Here you can set your basic frequency. The lib calculates the frequency automatically (default = 433.92).The cc1101 can: 300-348 MHZ, 387-464MHZ and 779-928MHZ. Read More info from datasheet.
  ELECHOUSE_cc1101.setSyncMode(2);  // Combined sync-word qualifier mode. 0 = No preamble/sync. 1 = 16 sync word bits detected. 2 = 16/16 sync word bits detected. 3 = 30/32 sync word bits detected. 4 = No preamble/sync, carrier-sense above threshold. 5 = 15/16 + carrier-sense above threshold. 6 = 16/16 + carrier-sense above threshold. 7 = 30/32 + carrier-sense above threshold.
  // ELECHOUSE_cc1101.setPA(10);      // set TxPower. The following settings are possible depending on the frequency band.  (-30  -20  -15  -10  -6    0    5    7    10   11   12) Default is max!
  ELECHOUSE_cc1101.setCrc(1);     // 1 = CRC calculation in TX and CRC check in RX enabled. 0 = CRC disabled for TX and RX.

}
void loop() {


  // center the motors
  ch1.write(90);
  ch2.write(90);
  ch3.write(90);
  ch4.write(90);
  ch5.write(90);

  // read BNO055 attitude data
  if(bno_status){
    sensors_event_t event;
    bno.getEvent(&event);

    p.yaw = event.orientation.x;
    p.pitch = event.orientation.y;
    p.roll = event.orientation.z;
  }

  // read flysky receiver data
  for(int i = 0; i < 14; i++){
    p.channels[i] = IBus.readChannel(i);
  }

  // send data over radio
  if(cc1101_status){
    ELECHOUSE_cc1101.SendData((void*) &p, sizeof(p), 100);
  }

  delay(50);  
}