int analogPin = 0;
int ledPin = 3;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int val = analogRead(analogPin);
  float convert = (float(val)/1023) * 255;
  int pwm = int(convert);
  Serial.println(val);
  Serial.println(pwm);
  analogWrite(ledPin, pwm);
  delay(250);
}
