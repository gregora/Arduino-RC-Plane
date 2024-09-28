#include <Wire.h>

int channels[14];

void setup() {

  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output

}


void loop() {

  Wire.requestFrom(8, 28);
  
  for(int i = 0; i < 14; i++){
    byte x1 = Wire.read();
    byte x2 = Wire.read();
    int val = ((int) x1 << 8) | ((int) x2);
    channels[i] = val;
    Serial.print(val);
    Serial.print(" ");
  }
  Serial.print("\n");


  delay(50);

}