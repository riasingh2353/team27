#include <Servo.h>

Servo servoL;
Servo servoR;
int lastError = 0;
int Lspeed = 100;
int Rspeed = 80;
int adj = 0;
unsigned int sensor_values[3];
int countdown = 12;
int turn_num = 0;
int change = 0;

int THRESH = 700;

void setup() {
// put your setup code here, to run once:
Serial.begin(9600);
servoL.attach(3);
servoR.attach(9);

}

// left, right, right, right, right, left, left

void loop() {

// put your main code here, to run repeatedly:
    sensor_values[0] = analogRead(A0);
    sensor_values[1] = analogRead(A1);
    sensor_values[2] = analogRead(A2);

    Serial.println(sensor_values[2]);

      if (sensor_values[0] > 300 && sensor_values[1] > 300) { 
                //drive straight
          drive_straight();
        //Serial.println("DRIVING STRAIGHT !!!!!");
      }//if robot veers left
      else if (sensor_values[0] < 300 && sensor_values[1] < 300){
        Serial.println("Intersection!");
        //Serial.println(turn_num);
        switch(turn_num){
          case 0:turn_left(); break;
          case 1:turn_right(); break;
          case 2:turn_right(); break;
          case 3:turn_right(); break;
          case 4:turn_right(); break;
          case 5:turn_left(); break;
          case 6:turn_left(); break;
          default: drive_straight(); break;
        }
        
        turn_num += 1;
        if (turn_num == 7){
          turn_num = 0;
        }
        
      }
      else if (sensor_values[0] < 300) { //if robot veers right
        //veer left 
        veer_left();
        //Serial.println("VEERING LEFT !!!!!");
      }
      else if (sensor_values[1] < 300) { //if robot veers right
        //veer left 
        veer_right();
        //Serial.println("VEERING LEFT !!!!!");
      }
      else {
        //veer right
          drive_straight();
        //Serial.println("VEERING RIGHT !!!!!");
      }
      
      //delay(1);
      //Serial.println("LEFT:");
      /*Serial.println(sensor_values[0]);
      Serial.println("RIGHT:");
      Serial.println(sensor_values[1]);
      //make tolerance 50?
      //Serial.println(diff);
      delay(1);

      */
}

void veer_left(){
        servoL.write(80);
        servoR.write(55);
}

void veer_right(){
        servoL.write(125);
        servoR.write(100);
}

void turn_left(){

  if (change == 0) {
     while (sensor_values[2] < (THRESH+10)) {
          servoL.write(90);
          servoR.write(55);
          Serial.println("LEFT!");
          delay(500);
          Serial.println(sensor_values[2]);
      }
     servoL.write(90);
     servoR.write(90);
     Serial.println(change);
     change = change + 1;
     while (sensor_values[2] > THRESH) {
           servoL.write(90);
          servoR.write(35);
          delay(500);
          Serial.println(sensor_values[2]);
          
       
     }

  }
}

void turn_right(){
  Serial.println(sensor_values[2]);
  if (change == 0) {
  while (sensor_values[2] < (THRESH+10)) {  
        servoL.write(125);
        servoR.write(90);
        delay(500);
        Serial.println("RIGHT!"); 
  }
  Serial.println(change);
  change = change + 1;
  while (sensor_values[2] > THRESH) {
    servoL.write(105);
    servoR.write(90);
    delay(500); 
    Serial.println("RIGHT!");
  }
  }
}



void drive_straight(){
        servoL.write(95);
        servoR.write(85);
        
}


