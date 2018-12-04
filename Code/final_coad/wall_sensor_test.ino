int          front_wall_value;   //store front wall sensor value
int          right_wall_value;   //store right wall sensor value
int          left_wall_value;    //store left wall sensor value
void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
  pinMode(6,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  get_wall_values();
  Serial.println("Front value");
  Serial.println(front_wall_value);
  Serial.println("right value");
  Serial.println(right_wall_value);
  Serial.println("left value");
  Serial.println(left_wall_value);
  delay(1000);
}

void get_wall_values() {
      digitalWrite(6, LOW);//set wall sensor select bit to read right wall sensor
      right_wall_value  = analogRead(A4);
      digitalWrite(6, HIGH);//set wall sensor select bit to read left wall sensor
      front_wall_value  = analogRead(A3);
      left_wall_value   = analogRead(A4);
}
