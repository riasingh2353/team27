#include <Servo.h>

Servo servoL;                     //instantiate servos
Servo servoR;
unsigned int sensor_values[3];   //store sensor values

int turn_num = 0;                //reset number of turns          
int change = 0;                  //reset temp variable noting change from white/black
int threshold = 700;            //cutoff value b/w white and not white

void setup() {
// put your setup code here, to run once:
Serial.begin(9600);
servoL.attach(3);
servoR.attach(9);

}

// left, right, right, right, right, left, left

void loop() {
// put your main code here, to run repeatedly:

// Read line sensor values continuously
    sensor_values[0] = analogRead(A0);
    sensor_values[1] = analogRead(A1);
    sensor_values[2] = analogRead(A2);

    //Case: reaches intersection
    if (sensor_values[0] < 300 && sensor_values[1] < 300){
        Serial.println("Intersection!");
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
        
        turn_num += 1;    // increment number of turns
        
        if (turn_num == 7){ turn_num = 0;}   // reset number of turns after figure 8 completed
      
      }

      //Case: traveling along line --> drive straight
      else if (sensor_values[0] > 300 && sensor_values[1] > 300) { drive_straight();}
      
      //Case: drifting off to the right --> correct left
      else if (sensor_values[0] < 300) { veer_left(); }

      //Case: drifting off to the left --> correct right
      else if (sensor_values[1] < 300) { veer_right(); }
      
      // Default: drive straight
      else {drive_straight();}
      
      delay(500);
      Serial.println("LEFT:");
      Serial.println(sensor_values[0]);
      Serial.println("RIGHT:");
      Serial.println(sensor_values[1]);
      Serial.println("MIDDLE");
      Serial.println(sensor_values[2]);
      
}

//HELPER FUNCTIONS

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
     while (sensor_values[2] < (threshold+10)) {
        servoL.write(90);
        servoR.write(55);
        Serial.println("Turning left! - on white");
      }
      
     servoL.write(90);
     servoR.write(90);
     Serial.println(change);
     change = change + 1;
   
   while (sensor_values[2] > threshold) {
       servoL.write(90);
       servoR.write(35);
       delay(500);
       Serial.println("Turning left!- on black"); 
    }
  }
}

void turn_right(){
  if (change == 0) {
    while (sensor_values[2] < (threshold+10)) {  
        servoL.write(125);
        servoR.write(90);
        delay(500);
        Serial.println("Turning right!- on white"); 
    }
    
    Serial.println(change);
    change = change + 1;
  
    while (sensor_values[2] > threshold) {
        servoL.write(105);
        servoR.write(90);
        delay(500); 
        Serial.println("Turning right!- on black");
    }
  }
}

void drive_straight(){
  servoL.write(95);
  servoR.write(85);     
}


