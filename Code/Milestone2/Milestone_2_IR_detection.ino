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
int line_threshold = 400;            //cutoff value b/w white and not white
bool turn_complete = true;
int countdown = 5000;
//bool wall_catching = true;
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

  pinMode(4,OUTPUT); //front wall detection
  pinMode(5,OUTPUT); //right wall detection
  pinMode(6,OUTPUT); //robot detection
  
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
    if (sensor_values[0] < line_threshold && sensor_values[1] < line_threshold ){
        Serial.println("Intersection!");
        front_wall_value  = analogRead(A3);
        right_wall_value  = analogRead(A4);
        //stop_drive();
        //delay(100);
       if (right_wall_value>=75){ // *|
        digitalWrite(5,HIGH);
          if (front_wall_value > 100){ //-|
            digitalWrite(4,HIGH);
            turn_left();
            Serial.println("front wall");
          }
          else{ // |
            //fft_detect(0); //check if need to turn left
            digitalWrite(4,LOW);
            drive_straight();
            Serial.println("following");
          }
          wall_before = true;
          //wall_catching = false;
          Serial.println("right wall");
       }else{ //no right wall
          digitalWrite(5,LOW);
          if (wall_before){
            turn_right();
          }
          else if (front_wall_value >100){ // front wall
             digitalWrite(4,HIGH);
             turn_left();
          }
          else {  //no front wall, check if need to turn right 
            digitalWrite(4,LOW);
            drive_straight(); 
          }             
          wall_before = false;
          Serial.println("no right wall");
       }
      
     }
    
      //Case:s traveling along line --> drive straight
      else if (sensor_values[0] > line_threshold && sensor_values[1] > line_threshold ) { drive_straight();}
      
      //Case: drifting off to the right --> correct left
      else if (sensor_values[0] < line_threshold ) {
        veer_left(); 
        }

      //Case: drifting off to the left --> correct right
      else if (sensor_values[1] < line_threshold ) { veer_right(); }
      
      // Default: drive straight
      else {
        drive_straight();
        Serial.println("driving");
      }
    digitalWrite(6, LOW);
/*    
     Serial.println("FRONT:");
     Serial.println(front_wall_value);
     Serial.println("RIGHT:");
     Serial.println(right_wall_value);
*/
 //     right_wall = false;
      
}

//HELPER FUNCTIONS
void check_wall(){
   front_wall_value  = analogRead(A3);
   right_wall_value  = analogRead(A4);
  // Serial.println(wall_before);
     if (right_wall_value>=75){ // *|  
      digitalWrite(5, HIGH);
         if (front_wall_value > 100){ //-|
            digitalWrite(4, HIGH);
            move_back();
            turn_left();
            Serial.println("front wall");
          }
         else{ // |
            digitalWrite(4, LOW);
            //fft_detect(0); //check if need to turn left
            drive_straight();
            Serial.println("following");
         }
         Serial.println("right wall");
       }else{ //no right wall
          digitalWrite(5, LOW);
          if (wall_before){
            move_back();
            turn_right();
          }
          else if (front_wall_value >100){ // front wall
            digitalWrite(4, HIGH);
            move_back();
            turn_left();
          }
          else {drive_straight(); }             //no front wall, check if need to turn right 
            digitalWrite(4, LOW);
          Serial.println("no right wall");
       } 

    /* Serial.println("FRONT:");
     Serial.println(front_wall_value);
     Serial.println("RIGHT:");
     Serial.println(right_wall_value);
     */
}

void move_back(){
     front_wall_value  = analogRead(A3);
   right_wall_value  = analogRead(A4);
  Serial.println("move back");
 if (sensor_values[0] < line_threshold ) {
      veer_left(); 
  }
      //Case: drifting off to the left --> correct right
  else if (sensor_values[1] < line_threshold ) { veer_right(); }
  else{
  while (!(sensor_values[0] < line_threshold  && sensor_values[1] < line_threshold )){
    servoL.write(85);
    servoR.write(95);  
    sensor_values[0] = analogRead(A0);
    sensor_values[1] = analogRead(A1);
    sensor_values[2] = analogRead(A2);
   }
  }
  //delay(25);
}

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
   front_wall_value  = analogRead(A3);
   right_wall_value  = analogRead(A4); 
  Serial.println("turn left");   
  while (countdown > 0) {
    servoL.write(88);
    servoR.write(80);
    countdown = countdown - 1;
    //Serial.println(countdown);
  }
    
   while (sensor_values[2] > line_threshold ){
       servoL.write(88);
       servoR.write(80);
       sensor_values[0] = analogRead(A0);
       sensor_values[1] = analogRead(A1);
       sensor_values[2] = analogRead(A2);//delay(500);
       Serial.println("Turning left!"); 
    }
   countdown = 5000;

     /*while (sensor_values[2] < (threshold+10)) {
        servoL.write(90);
        servoR.write(55);
        Serial.println("Turning left! - on white");
     }*/
   fft_detect(0);
   check_wall();
}

void turn_right(){
   front_wall_value  = analogRead(A3);
   right_wall_value  = analogRead(A4);
  Serial.println("turn right");
  while(countdown > 0){
        servoL.write(100);
        servoR.write(92);
        countdown = countdown -1;
        //Serial.println(countdown);
  }
  while (sensor_values[2] > line_threshold ){
        servoL.write(100);
        servoR.write(92);
        //delay(500); 
        sensor_values[0] = analogRead(A0);
        sensor_values[1] = analogRead(A1);
        sensor_values[2] = analogRead(A2);
        Serial.println("Turning right!");
        //Serial.println(sensor_values[2]);
    }
  countdown = 5000;
    
    /*while (sensor_values[2] < (threshold+10)) {  
        servoL.write(125);
        servoR.write(90);
        //delay(500);
        Serial.println("Turning right!- on white"); 
    }*/
  fft_detect(1);
  //delay(25);
  check_wall();
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
       digitalWrite(6, HIGH);
      /* if (x) turn_right(); 
       else turn_left();
       */
       stop_drive();
       delay(5000);
       //digitalWrite(4, HIGH); //turn on indicator LED
    }
    else {
       //digitalWrite(4, LOW); //turn off indicator LED
       drive_straight();
    }
}
