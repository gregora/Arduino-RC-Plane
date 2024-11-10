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

#define DEBUG false
#define PERFORMANCE false
#define ANG_VEL false

bool bno_status = false;
bool cc1101_status = false;

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

float ang_yaw;
float ang_pitch;
float ang_roll;


Packet p;


void setup() {
  // attach the motors to corresponding pins
  ch1.attach(2);
  ch2.attach(3);
  ch3.attach(4);
  ch4.attach(5);
  ch5.attach(6);
  
  if(DEBUG){
    Serial.begin(115200);
  }

  IBus.begin(Serial); // use Serial to read flysky receiver iBUS

  if(!bno.begin()){
    bno_status = false;
    if(DEBUG){
      Serial.println("BNO055: Connection ERROR");
    }
  }else {
    bno_status = true;
  }

  if (!ELECHOUSE_cc1101.getCC1101()){        // Check the CC1101 Spi connection.
    cc1101_status = false;
    if(DEBUG){
      Serial.println("CC1101: Connection ERROR");
    }
  }else{
    cc1101_status = true;
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

  unsigned long t1 = millis();


  if(bno_status){

    // read BNO055 attitude data

    sensors_event_t event;
    imu::Vector<3> vector;
    bno.getEvent(&event);

    p.yaw = event.orientation.x;
    p.pitch = event.orientation.y;
    p.roll = event.orientation.z;

    vector = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);

    p.ax = vector.x();
    p.ay = vector.y();
    p.az = vector.z();

    // read BNO055 angular velocities
    vector = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
    ang_roll = vector.x();
    ang_pitch = vector.y();
    ang_yaw = vector.z();

    if(DEBUG && ANG_VEL){
      Serial.print(ang_roll);
      Serial.print(" ");
      Serial.print(ang_pitch);
      Serial.print(" ");
      Serial.print(ang_yaw);
      Serial.println();
    }
  }

  unsigned long t2 = millis();

  // read flysky receiver data
  for(int i = 0; i < 14; i++){
    p.channels[i] = IBus.readChannel(i);
  }

  // write received values to servos
  ch1.writeMicroseconds(p.channels[0]); // aileron 1
  ch2.writeMicroseconds(p.channels[1]); // horizontal stabilizer
  ch3.writeMicroseconds(p.channels[2]); // throttle
  ch4.writeMicroseconds(p.channels[3]); // aileron 2
  ch5.writeMicroseconds(p.channels[4]); // vertical stabilizer

  unsigned long t3 = millis();

  p.time = millis();

  // send data over radio (CC1101)
  if(cc1101_status){
    ELECHOUSE_cc1101.SendData((void*) &p, sizeof(p), 8);
  }

  unsigned long t4 = millis();

  if(DEBUG && PERFORMANCE){
    Serial.println("DEBUG: Performance");

    Serial.print("  IMU: ");
    Serial.println(t2 - t1);

    Serial.print("  iBUS: ");
    Serial.println(t3 - t2);

    Serial.print("  Telemetry: ");
    Serial.println(t4 - t3);

    Serial.println();
  }

}