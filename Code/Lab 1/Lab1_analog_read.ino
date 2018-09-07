int analogPin = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int val = analogRead(analogPin);
  float inV = val*.0049;
  Serial.print(inV);
  Serial.println (" V");
  delay(1000);
}
