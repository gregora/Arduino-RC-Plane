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

IBusBM IBus;
int channels[14]; // buffer to store read values

void setup() {
  // attach the motors to corresponding pins
  ch1.attach(2);
  ch2.attach(3);
  ch3.attach(4);
  ch4.attach(5);
  ch5.attach(6);

  IBus.begin(Serial); // use Serial to read flysky receiver iBUS

  Wire.begin(8); // join i2c bus with address #8
  Wire.onRequest(dataRequest); // add interrupt
  
}
void loop() {

  // center the motors
  ch1.write(90);
  ch2.write(90);
  ch3.write(90);
  ch4.write(90);
  ch5.write(90);

  // read BNO055 attitude data
  sensors_event_t event;
  bno.getEvent(&event);

  float yaw = event.orientation.x;
  float pitch = event.orientation.y;
  float roll = event.orientation.z;
  
  // read flysky receiver data
  for(int i = 0; i < 14; i++){
    channels[i] = IBus.readChannel(i);
  }


  delay(50);  
}


void dataRequest() {
  // send flysky receiver data
  for(int i = 0; i < 14; i++){
    Wire.write(highByte(channels[i]));
    Wire.write(lowByte(channels[i]));
  }
}