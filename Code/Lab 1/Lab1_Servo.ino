#include <Servo.h>

Servo servo;
int analogPin = 0;

void setup() {
  Serial.begin(9600);
  servo.attach(3);
}

void loop() {
  // put your main code here, to run repeatedly:
  int val = analogRead(analogPin);
  float convert = (float(val)/679) * 180;
  int pwm = int(convert);
  Serial.println(val);
  Serial.println(pwm);
  servo.write(pwm);
  delay(250);
}
