void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int sensL = analogRead(A0);
  int sensR = analogRead(A1);
  int sensB = analogRead(A2);

  Serial.print("Left Sensor Value: ");
  Serial.println(sensL);
  Serial.print("Right Sensor Value: ");
  Serial.println(sensR);
  Serial.print("Back Sensor Value: ");
  Serial.println(sensB);
  

  delay(1000);
}
