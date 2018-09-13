#include <Servo.h>

Servo servoL;
Servo servoR;
int lastError = 0;
int Lspeed = 100;
int Rspeed = 80;
int adj = 0;
unsigned int sensor_values[2];
int countdown = 12;
int turn_num = 0;

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
      
      if (sensor_values[0] > 300 && sensor_values[1] > 300) { 
                //drive straight
          drive_straight();
        //Serial.println("DRIVING STRAIGHT !!!!!");
      }//if robot veers left
      else if (sensor_values[0] < 300 && sensor_values[1]<300){
        //Serial.println("Intersection!");
        switch(turn_num){
          case 0: turn_left();
          case 1: turn_right();
          case 2: turn_right();
          case 3: turn_right();
          case 4: turn_right();
          case 5: turn_left();
          case 6: turn_left();
          default: drive_straight();
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
      else {
        //veer right
          veer_right();
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
        servoL.write(87);
        servoR.write(55);
        Serial.println("LEFT!");
        delay(1200);

}

void turn_right(){
        servoL.write(125);
        servoR.write(93); 
        Serial.println("RIGHT!");
        delay(1200);
        
}

void drive_straight(){
        servoL.write(95);
        servoR.write(85);
        
}
