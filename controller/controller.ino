#include <Wire.h>
#include <Adafruit_Sensor.h>

#include <Adafruit_BNO055.h>
#include <Servo.h>

#include <IBusBM.h>
#include <SoftwareSerial.h>

#include "Wire.h"

Servo ch1;
Servo ch2;
Servo ch3;
Servo ch4;
Servo ch5;

Adafruit_BNO055 bno = Adafruit_BNO055(55);
IBusBM IBus;    // IBus object

int channels[14];

void setup() {
  //Serial.begin(115200);
  IBus.begin(Serial);

  ch1.attach(2);
  ch2.attach(3);
  ch3.attach(4);
  ch4.attach(5);
  ch5.attach(6);

  if(!bno.begin())
  {
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    //while(1);
  }

  Wire.begin(8);                // join i2c bus with address #8
  Wire.onRequest(requestEvent); // register event
  
}
void loop() {

  ch1.write(90);
  ch2.write(90);
  ch3.write(90);
  ch4.write(90);
  ch5.write(90);

  sensors_event_t event;
  bno.getEvent(&event);

  float yaw = event.orientation.x;
  float pitch = event.orientation.y;
  float roll = event.orientation.z;
  
  /*
  Serial.print(yaw, 4);
  Serial.print(" ");
  Serial.print(pitch, 4);
  Serial.print(" ");
  Serial.print(roll, 4);
  Serial.println("");

  Serial.print("\n");
  */


  ch1.write(pitch + 90);


  for(int i = 0; i < 14; i++){
    channels[i] = IBus.readChannel(i);
  }

  //val = 17;
  //Serial.print(val);
  //Serial.print("\n");

  delay(50);
  
}

void requestEvent() {

  for(int i = 0; i < 14; i++){
    Wire.write(highByte(channels[i]));
    Wire.write(lowByte(channels[i]));
  }
}