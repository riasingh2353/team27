void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(2, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
        Serial.println("loop");
         digitalWrite(6,HIGH);
        int line_l = analogRead(A0);
        int line_r = analogRead(A1);
        int line_m = analogRead(A2);
        int front_wall_value  = analogRead(A3);
        int right_wall_value  = analogRead(A4);
        Serial.print("Left Line: ");
        Serial.println(line_l);
        Serial.print("Right Line: ");
        Serial.println(line_r);
        Serial.print("Middle Line: ");
        Serial.println(line_m);
        Serial.print("Front: ");
        Serial.println(front_wall_value);
        Serial.print("Left: ");
        Serial.println(right_wall_value);
        delay(500);
}
