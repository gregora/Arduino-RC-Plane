#include <Wire.h>
#include <Adafruit_Sensor.h>

#include <Adafruit_BNO055.h>
#include <Servo.h>

Servo ch1;
Servo ch2;
Servo ch3;
Servo ch4;
Servo ch5;

Adafruit_BNO055 bno = Adafruit_BNO055(55);

void setup() {
  Serial.begin(9600);

  ch1.attach(2);
  ch2.attach(3);
  ch3.attach(4);
  ch4.attach(5);
  ch5.attach(6);

  if(!bno.begin())
  {
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }


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
  
  Serial.print(yaw, 4);
  Serial.print(" ");
  Serial.print(pitch, 4);
  Serial.print(" ");
  Serial.print(roll, 4);
  Serial.println("");

  

  ch1.write(pitch + 90);


  delay(50);

}