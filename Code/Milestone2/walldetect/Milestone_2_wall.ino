#include <Servo.h>

#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft
#include <FFT.h> // include the library

Servo servoL;                     //instantiate servos
Servo servoR;
unsigned int sensor_values[3];   //store sensor values
int          front_wall_value;         //store front wall sensor value
int          right_wall_value;    //store right wall sensor value
int change = 0;                  //reset temp variable noting change from white/black
int threshold = 300;            //cutoff value b/w white and not white
bool turn_complete = true;
int countdown = 6000;
bool wall_catching = true;
bool wall_before = false;

int l = 0;
int count = 0;
boolean start = 0;
int fft_direction = 0; //0 is left, 1 is right

void setup() {
// put your setup code here, to run once:
  Serial.begin(9600);
  servoL.attach(3);
  servoR.attach(9);

  ADCSRA &= ~(bit (ADPS0) | bit (ADPS1) | bit (ADPS2)); // clear prescaler bits 
  ADCSRA |= bit (ADPS2); // set ADC prescalar to be eight times faster than default

}


void loop() {
// put your main code here, to run repeatedly:

// Read line sensor values continuously
    sensor_values[0] = analogRead(A0); //left line sensor
    sensor_values[1] = analogRead(A1); //right line sensor
    sensor_values[2] = analogRead(A2); //rear line sensor

//Detecting start signal
   /*   if (start == 0) { 
         cli();
         for (int i = 0 ; i < 512 ; i += 2) {
          fft_input[i] = analogRead(A5); // <-- NOTE THIS LINE
          fft_input[i+1] = 0;
         }
        fft_window();
        fft_reorder();
        fft_run();
        fft_mag_log();
        sei();
        if (fft_log_out[3] > 70){
          l = l + 1;
          digitalWrite(2, LOW);
        }
        else {
          l = 0;
          digitalWrite(2, LOW);
        }
        if (l >= 10) {
          count = count + 1;
          start = 1;
          digitalWrite(2, HIGH);  //flip select bit
          digitalWrite(3, HIGH);  //turn on indicator LED
          Serial.println("660 HURTS !!!!!");
        }
      }
  
       */
    start = 1;
    digitalWrite(2, HIGH);
    if (sensor_values[0] < 300 && sensor_values[1] < 300){
        Serial.println("Intersection!");
        front_wall_value  = analogRead(A3);
        right_wall_value  = analogRead(A4);
        
        /*if (front_wall_value > 100 && side_front_wall_value > 100) {    //Case where wall is detected in front and on the right
          turn_left(); // make this turn left in place
        }

        else if (front_wall_value > 100) {   //this wall sensor threshold value is that which was set by last year's team 1, corresponding to approximately 20 cm
          turn_right();   // make this turn right in place
          right_wall = true;
        }
        else if (side_front_wall_value > 100) {
          drive_straight();  //right hand wall following
       }
       else if (front_wall_value < 100 && side_front_wall_value < 100) {
         if (right_wall) {
          turn_right();
          right_wall = false;   //reset afer turning
         }
         drive_straight(); // default case, then switches back to line following 
       }*/
       if (right_wall_value>=100){ // *|
          if (front_wall_value > 100){ //-|
            turn_left();
            Serial.println("front wall");
          }
          else{ // |
            //fft_detect(0); //check if need to turn left
            drive_straight();
            Serial.println("following");
          }
          wall_before = true;
          wall_catching = false;
          Serial.println("right wall");
       }

       if (right_wall_value < 100){ //no right wall
          if (wall_before){
            turn_right();
          }
          else if (front_wall_value >100){ // front wall
            while (front_wall_value >100){
              turn_left();
            }
          }
          else {drive_straight(); }             //no front wall, check if need to turn right 
          wall_before = false;
          Serial.println("no right wall");
       }
      
     }
    
      //Case:s traveling along line --> drive straight
      else if (sensor_values[0] > 300 && sensor_values[1] > 300) { drive_straight();}
      
      //Case: drifting off to the right --> correct left
      else if (sensor_values[0] < 300) {
        veer_left(); 
        }

      //Case: drifting off to the left --> correct right
      else if (sensor_values[1] < 300) { veer_right(); }
      
      // Default: drive straight
      else {
        drive_straight();
        Serial.println("driving");
      }
    

    //  Serial.println("FRONT:");
     // Serial.println(front_wall_value);
    //  Serial.println("RIGHT:");
    //  Serial.println(right_wall_value);

 //     right_wall = false;
      
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
    servoL.write(89);
    servoR.write(35);
    countdown = countdown - 1;
  }
    
   while (sensor_values[2] > 300){
       servoL.write(89);
       servoR.write(35);
       sensor_values[0] = analogRead(A0);
       sensor_values[1] = analogRead(A1);
       sensor_values[2] = analogRead(A2);//delay(500);
       Serial.println("Turning left!"); 
    } 
   countdown = 6000;

     /*while (sensor_values[2] < (threshold+10)) {
        servoL.write(90);
        servoR.write(55);
        Serial.println("Turning left! - on white");
     }*/
   // fft_detect(0);
}

void turn_right(){
  while(countdown > 0){
         servoL.write(105);
        servoR.write(91);
        countdown = countdown -1;
  }
  while (sensor_values[2] > threshold){
        servoL.write(105);
        servoR.write(91);
        //delay(500); 
        sensor_values[0] = analogRead(A0);
        sensor_values[1] = analogRead(A1);
        sensor_values[2] = analogRead(A2);
        Serial.println("Turning right!");
        Serial.println(sensor_values[2]);
    }

  countdown = 6000;
    
    /*while (sensor_values[2] < (threshold+10)) {  
        servoL.write(125);
        servoR.write(90);
        //delay(500);
        Serial.println("Turning right!- on white"); 
    }*/
  //fft_detect(1);
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

void fft_detect( int x ){
    cli();
    
    for (int i = 0 ; i < 512 ; i += 2) {
      fft_input[i] = analogRead(A5); // <-- NOTE THIS LINE
      fft_input[i+1] = 0;
    }
    
    fft_window();
    fft_reorder();
    fft_run();
    fft_mag_log();
    sei();

    if (fft_log_out[26] > 60 || fft_log_out[25] > 60 || fft_log_out[27] > 60){
       Serial.println("6KHz !!!!!");
       if (x) turn_right(); 
       turn_left();
       //digitalWrite(4, HIGH); //turn on indicator LED
    }
    else {
       //digitalWrite(4, LOW); //turn off indicator LED
       drive_straight();
    }
}
