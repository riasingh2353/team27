#include <Servo.h>

Servo servoL;
Servo servoR;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  servoL.attach(3);
  servoR.attach(9);
}

void loop() {
  // put your main code here, to run repeatedly:
  servoL.write(180);
  servoR.write(0);
  delay(1000);
  servoL.write(180);
  servoR.write(90);
  delay(1000); 
}
