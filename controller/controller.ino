#include <Wire.h>
#include <Adafruit_Sensor.h>

#include <Adafruit_BNO055.h>
#include <Servo.h>

#include <IBusBM.h>

#include <ELECHOUSE_CC1101_SRC_DRV.h>

#include <TinyGPS++.h>
#include <SoftwareSerial.h>

//Servo ch1;
//Servo ch2;
//Servo ch3;
//Servo ch4;
//Servo ch5;

Adafruit_BNO055 bno = Adafruit_BNO055(55);

IBusBM IBus;

TinyGPSPlus gps;
SoftwareSerial ss(7, 8); // GPS


#define DEBUG false
#define PERFORMANCE false
#define ANG_VEL false
#define MAG false
#define GPS false

bool bno_status = false;
bool ibus_status = false;
bool cc1101_status = false;

unsigned int frame = 0;

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


float ang_yaw;
float ang_pitch;
float ang_roll;

Packet p;

float fixes = 0;


void setup() {
  p.mode = 0;

  // attach the motors to corresponding pins
  //ch1.attach(2);
  //ch2.attach(3);
  //ch3.attach(4);
  //ch4.attach(5);
  //ch5.attach(6);

  pinMode(3, OUTPUT);
  //pinMode(5, OUTPUT);
  //pinMode(6, OUTPUT);
  pinMode(9, OUTPUT);


  if(DEBUG){
    Serial.begin(115200);
  }

  IBus.begin(Serial, IBUSBM_NOTIMER); // use Serial to read flysky receiver iBUS

  if(!bno.begin()){
    bno_status = false;
    if(DEBUG){
      Serial.println("BNO055: Connection ERROR");
    }
  }else {
    bno_status = true;
  
    delay(1000);

    bno.setExtCrystalUse(true);
  }


  if (!ELECHOUSE_cc1101.getCC1101()){        // Check the CC1101 Spi connection.
    cc1101_status = false;
    if(DEBUG){
      Serial.println("CC1101: Connection ERROR");
    }
  }else{
    cc1101_status = true;
  }


  ss.begin(9600);

  // Disable NMEA
  byte sentances [][16] = {
    //{0xB5,0x62,0x06,0x01,0x08,0x00,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x24}, // GxGGA off
    {0xB5,0x62,0x06,0x01,0x08,0x00,0xF0,0x01,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x2B}, // GxGLL off
    {0xB5,0x62,0x06,0x01,0x08,0x00,0xF0,0x02,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x32}, // GxGSA off
    {0xB5,0x62,0x06,0x01,0x08,0x00,0xF0,0x03,0x00,0x00,0x00,0x00,0x00,0x01,0x03,0x39}, // GxGSV off
    {0xB5,0x62,0x06,0x01,0x08,0x00,0xF0,0x04,0x00,0x00,0x00,0x00,0x00,0x01,0x04,0x40}, // GxRMC off
    {0xB5,0x62,0x06,0x01,0x08,0x00,0xF0,0x05,0x00,0x00,0x00,0x00,0x00,0x01,0x05,0x47} // GxVTG off
  };

  for (unsigned int j = 0; j < sizeof(sentances); j++){
    for(unsigned int i = 0; i < sizeof(sentances[i]); i++) {  
      ss.write(sentances[j][i]);
    }
  }

  const unsigned char UBLOX_INIT[] PROGMEM = {
      //0xB5,0x62,0x06,0x08,0x06,0x00,0x64,0x00,0x01,0x00,0x01,0x00,0x7A,0x12, //(10Hz)
      0xB5,0x62,0x06,0x08,0x06,0x00,0xC8,0x00,0x01,0x00,0x01,0x00,0xDE,0x6A //(5Hz)
      //0xB5,0x62,0x06,0x08,0x06,0x00,0xE8,0x03,0x01,0x00,0x01,0x00,0x01,0x39 //(1Hz)
    };

  // send configuration data in UBX protocol
  for(unsigned int i = 0; i < sizeof(UBLOX_INIT); i++) {                        
    ss.write(UBLOX_INIT[i]); // 5Hz refresh rate
  }


  // change baudrate to 19200
  ss.write("$PUBX,41,1,0007,0003,19200,0*25\r\n");
  ss.flush();
  ss.end();

  delay(500);

  ss.begin(19200);




  



  ELECHOUSE_cc1101.Init();              // must be set to initialize the cc1101!
  ELECHOUSE_cc1101.setCCMode(1);       // set config for internal transmission mode.
  ELECHOUSE_cc1101.setModulation(0);  // set modulation mode. 0 = 2-FSK, 1 = GFSK, 2 = ASK/OOK, 3 = 4-FSK, 4 = MSK.
  ELECHOUSE_cc1101.setMHZ(433.92);   // Here you can set your basic frequency. The lib calculates the frequency automatically (default = 433.92).The cc1101 can: 300-348 MHZ, 387-464MHZ and 779-928MHZ. Read More info from datasheet.
  ELECHOUSE_cc1101.setSyncMode(2);  // Combined sync-word qualifier mode. 0 = No preamble/sync. 1 = 16 sync word bits detected. 2 = 16/16 sync word bits detected. 3 = 30/32 sync word bits detected. 4 = No preamble/sync, carrier-sense above threshold. 5 = 15/16 + carrier-sense above threshold. 6 = 16/16 + carrier-sense above threshold. 7 = 30/32 + carrier-sense above threshold.
  // ELECHOUSE_cc1101.setPA(10);      // set TxPower. The following settings are possible depending on the frequency band.  (-30  -20  -15  -10  -6    0    5    7    10   11   12) Default is max!
  ELECHOUSE_cc1101.setCrc(1);     // 1 = CRC calculation in TX and CRC check in RX enabled. 0 = CRC disabled for TX and RX.


}


void loop() {

  frame++;

  unsigned long t1 = millis();

  if(bno_status){
  

    // read BNO055 attitude data
    imu::Vector<3> vector1;
    vector1 = bno.getVector(Adafruit_BNO055::VECTOR_EULER);

    p.quat = bno.getQuat();
    vector1 = p.quat.toEuler();
    yaw   = vector1.x() * 180 / 3.1415; 
    pitch = vector1.y() * 180 / 3.1415;
    roll  = vector1.z() * 180 / 3.1415;

    // read BNO055 lateral accelerations
    imu::Vector<3> vector2;
    vector2 = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);

    p.ax = vector2.x();
    p.ay = vector2.y();
    p.az = vector2.z();


    // read BNO055 angular velocities
    imu::Vector<3> vector3;
    vector3 = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
    ang_roll = vector3.x();
    ang_pitch = vector3.y();
    ang_yaw = vector3.z();
    
    //  read BNO055 magnetometer
    imu::Vector<3> vector4;
    vector4 = bno.getVector(Adafruit_BNO055::VECTOR_MAGNETOMETER);
    float mag_x = vector4.x();
    float mag_y = vector4.y();
    float mag_z = vector4.z();

    if(DEBUG && ANG_VEL){
      Serial.print(ang_roll);
      Serial.print(" ");
      Serial.print(ang_pitch);
      Serial.print(" ");
      Serial.print(ang_yaw);
      Serial.println();
    }

    if(DEBUG && MAG) {
      Serial.print(mag_x);
      Serial.print(" ");
      Serial.print(mag_y);
      Serial.print(" ");
      Serial.print(mag_z);
      Serial.println();
      Serial.println(atan2(mag_y, mag_x) * 180 / 3.1415 + 180);

    }

  } else {
    p.mode = 0; // IMU unavailable - default to manual mode
  }


  unsigned long t2 = millis();

  while (ss.available() > 0){
    gps.encode(ss.read());
    //Serial.write(ss.read());
  }

  if (gps.location.isUpdated()){


    p.latitude = gps.location.lat();
    p.longitude = gps.location.lng();
    p.altitude = gps.altitude.meters();

    p.satellites = gps.satellites.value();

    if (DEBUG && GPS){
      Serial.print("Latitude= "); 
      Serial.print(p.latitude, 6);
      Serial.print(" Longitude= "); 
      Serial.print(p.longitude, 6);
      Serial.print(" Altitude= ");
      Serial.print((float) p.altitude, 0);
      Serial.print(" Satellites= ");
      Serial.println(p.satellites);
    
    
      fixes++;
      Serial.print("Fixes per second: ");
      Serial.println(fixes / p.time * 1000);
    }
  }

  unsigned long t3 = millis();

  IBus.loop();
  // read flysky receiver data
  for(int i = 0; i < 7; i++){
    p.channels[i] = IBus.readChannel(i);
  }

  // if any channel is 0, iBus is disconnected
  if(p.channels[6] != 0){ //check only for channel 6
    ibus_status = true;
    if(p.channels[6] == 2000){
      p.mode = 1;
    }else{
      p.mode = 0;
    }
  } else {
    ibus_status = false;
    p.mode = 255; // iBus is unavailable, enter recovery mode
  }


  if(p.mode == 1) {
    // take-off mode
    // target 10 deg nose up, wings level

    float P = 2000 / 90; // max deflection at 90 deg error
    float D = 50 / (4 * 3.14); // max deflection at 4*pi rad/s angular velocity

    p.channels[1] = 1500 + (pitch - 10) * P + ang_pitch*D;

    p.channels[0] = 1500 - roll*P - ang_roll*D;
    p.channels[4] = 1500 - roll*P - ang_roll*D;

    p.channels[3] = 1500;

  } else if(p.mode == 2){
    // fly-by-wire mode
    // target angular velocities

    float D = 100 / (8 * 3.14); // max deflection is at 8*pi rad/s angular velocity error

    // max desired angular velocity is 2*pi rad/s
    p.channels[1] = ((p.channels[1] - 1500) * 2*3.14 / 500 + ang_pitch) * D;

    p.channels[0] = ((p.channels[0] - 1500) * 2*3.14 / 500 - ang_roll) * D;
    p.channels[4] = ((p.channels[3] - 1500) * 2*3.14 / 500 - ang_roll) * D;

    p.channels[3] = 1500;

  }else if(p.mode == 255){  
    // recovery mode
    // target 10 deg nose down, wings level

    float P = 4.0 / 90         * 500; // max deflection at 90 deg error
    float D = 0.1 / (4 * 3.14) * 500; // max deflection at 4*pi rad/s angular velocity

    p.channels[1] = 1500 + (pitch + 10) * P + ang_pitch*D;

    p.channels[0] = 1500 - roll*P - ang_roll*D;
    p.channels[4] = 1500 - roll*P - ang_roll*D;

    p.channels[3] = 1500;
  }

  // limit the travel of servos
  // channels have min of 1000 and max 2000

  for(int i = 0; i < 7; i++){
    if(p.channels[i] < 1000){
      p.channels[i] = 1000;
    }

    if(p.channels[i] > 2000){
      p.channels[i] = 2000;
    }
  }


  // write received values to servos
  
  //ch1.writeMicroseconds(p.channels[0]); // aileron 1
  //ch2.writeMicroseconds(p.channels[1]); // elevator
  //ch3.writeMicroseconds(p.channels[2]); // throttle
  //ch4.writeMicroseconds(p.channels[3]); // rudder
  //ch5.writeMicroseconds(p.channels[4]); // aileron 2
  
  
  analogWrite(3, ((float) p.channels[0])*255.0/2000); // both ailerons
  //analogWrite(5, ((float) p.channels[3])*255.0/2000); // rudder
  analogWrite(9, ((float) p.channels[1])*255.0/2000); // horizontal stabilizer

  

  unsigned long t4 = millis();

  p.time = millis();

  // send data over radio (CC1101)
  if(cc1101_status){
    ELECHOUSE_cc1101.SendData((void*) &p, sizeof(p), 8);
  }

  unsigned long t5 = millis();

  if(DEBUG && PERFORMANCE){
    Serial.println("DEBUG: Performance");

    Serial.print("  IMU: ");
    Serial.println(t2 - t1);

    Serial.print("  GPS: ");
    Serial.println(t3 - t2);


    Serial.print("  iBUS: ");
    Serial.println(t4 - t3);

    Serial.print("  Telemetry: ");
    Serial.println(t5 - t4);

    Serial.println();
  }

}