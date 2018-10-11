
#include <Servo.h>

Servo servoL;                     //instantiate servos
Servo servoR;
unsigned int sensor_values[3];   //store sensor values
int          wall_value;         //store front wall sensor value
int          side_wall_value;    //store right wall sensor value
int change = 0;                  //reset temp variable noting change from white/black
int threshold = 300;            //cutoff value b/w white and not white
bool turn_complete = true;
int countdown = 8000;
bool right_wall = false;

void setup() {
// put your setup code here, to run once:
Serial.begin(9600);
servoL.attach(3);
servoR.attach(9);

}


void loop() {
// put your main code here, to run repeatedly:

// Read line sensor values continuously
    sensor_values[0] = analogRead(A0); //left line sensor
    sensor_values[1] = analogRead(A1); //right line sensor
    sensor_values[2] = analogRead(A2); //rear line sensor


/*Wall Following Cases:
 * Nothing on right or front: drive straight
 * Only in the front: turn right
 * In front and right: turn left
 * Following wall on right then hit intersection: turn right
 */

    //Case: reaches intersection   
      if (sensor_values[0] < 300 && sensor_values[1] < 300){
        Serial.println("Intersection!");
        wall_value       = analogRead(A3);
        side_wall_value  = analogRead(A4);
        
        if (wall_value > 100 && side_wall_value > 100) {    //Case where wall is detected in front and on the right
          turn_left(); // make this turn left in place
        }

        else if (wall_value > 100) {   //this wall sensor threshold value is that which was set by last year's team 1, corresponding to approximately 20 cm
          turn_right();   // make this turn right in place
          right_wall = true;
        }
        else if (side_wall_value > 100) {
          drive_straight();  //right hand wall following
       }
       else if (wall_value < 100 && side_wall_value < 100) {
         if (right_wall) {
          turn_right();
          right_wall = false;   //reset afer turning
         }
         drive_straight(); // default case, then switches back to line following 
       }
     }
    
      //Case:s traveling along line --> drive straight
      else if (sensor_values[0] > 300 && sensor_values[1] > 300) { drive_straight();}
      
      //Case: drifting off to the right --> correct left
      else if (sensor_values[0] < 300) { veer_left(); }

      //Case: drifting off to the left --> correct right
      else if (sensor_values[1] < 300) { veer_right(); }
      
      // Default: drive straight
      else {drive_straight();}

      Serial.println("LEFT:");
      Serial.println(sensor_values[0]);
      Serial.println("RIGHT:");
      Serial.println(sensor_values[1]);
      Serial.println("MIDDLE");
      Serial.println(sensor_values[2]);

      right_wall = false;
      
}

//HELPER FUNCTIONS

void veer_left(){
  servoL.write(80);
  servoR.write(55);
  sensor_values[0] = analogRead(A0);
  sensor_values[1] = analogRead(A1);
  sensor_values[2] = analogRead(A2);
}

void veer_right(){
  servoL.write(125);
  servoR.write(100);
  sensor_values[0] = analogRead(A0);
  sensor_values[1] = analogRead(A1);
  sensor_values[2] = analogRead(A2);
}

void turn_left(){     
  while (countdown > 0) {
    servoL.write(87);
    servoR.write(35);
    countdown = countdown - 1;
  }
    
   while (sensor_values[2] > 300){
       servoL.write(87);
       servoR.write(35);
       sensor_values[0] = analogRead(A0);
       sensor_values[1] = analogRead(A1);
       sensor_values[2] = analogRead(A2);//delay(500);
       Serial.println("Turning left!"); 
    } 
   countdown = 8000;

     /*while (sensor_values[2] < (threshold+10)) {
        servoL.write(90);
        servoR.write(55);
        Serial.println("Turning left! - on white");
     }*/
}

void turn_right(){
  while(countdown > 0){
         servoL.write(105);
        servoR.write(93);
        countdown = countdown -1;
  }
  while (sensor_values[2] > threshold){
        servoL.write(105);
        servoR.write(93);
        //delay(500); 
            sensor_values[0] = analogRead(A0);
          sensor_values[1] = analogRead(A1);
         sensor_values[2] = analogRead(A2);
        Serial.println("Turning right!");
        Serial.println(sensor_values[2]);
    }

  countdown = 8000;
    
    /*while (sensor_values[2] < (threshold+10)) {  
        servoL.write(125);
        servoR.write(90);
        //delay(500);
        Serial.println("Turning right!- on white"); 
    }*/
}

void drive_straight(){
  servoL.write(95);
  servoR.write(85);  
   sensor_values[0] = analogRead(A0);
    sensor_values[1] = analogRead(A1);
    sensor_values[2] = analogRead(A2);   
}

void stop_drive(){
  servoL.write(90);
  servoR.write(90);
    sensor_values[0] = analogRead(A0);
    sensor_values[1] = analogRead(A1);
    sensor_values[2] = analogRead(A2);
}
