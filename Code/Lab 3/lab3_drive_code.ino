#include <Servo.h>

#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft
#include <FFT.h> // include the library

Servo servoL;                     //instantiate servos
Servo servoR;
unsigned int sensor_values[3];   //store sensor values
int          front_wall_value;         //store front wall sensor value
int          right_wall_value;    //store right wall sensor value
int          left_wall_value;     //store left wall sensor value
int change = 0;                  //reset temp variable noting change from white/black
int line_threshold = 300;            //cutoff value b/w white and not white
int wall_threshold = 150;         //NOT IMPLEMENTED EVERYWHERE. A wall exists if a
//wall sensor detects a value above this
bool turn_complete = true;
int countdown = 3000;
bool wall_before = false;
bool start = 0;

int dir = 1;                    //direction the robot is traveling in.
//0 -> N; 1 -> E; 2 -> S; 3 -> W;
//We assume the robot starts in the northwest corner
// traveling east by default

int l = 0;
int count = 0;
int fft_direction = 0; //0 is left, 1 is right

void setup() {
  Serial.begin(9600);
  servoL.attach(3);
  servoR.attach(5);

  //Digital Output Pins
  pinMode(1, OUTPUT); //Wall sensor mux select bit
  //LOW reads right sensor, HIGH reads left sensor
  pinMode(2, OUTPUT); //FFT mux select bit
  //LOW reads microphone, HIGH reads IR circuit


  //LEDs:
  pinMode(4, OUTPUT); //front wall detection
  pinMode(6, OUTPUT); //robot detection
  pinMode(7, OUTPUT); //right wall detection

  ADCSRA &= ~(bit (ADPS0) | bit (ADPS1) | bit (ADPS2)); // clear prescaler bits
  ADCSRA |= bit (ADPS2); // set ADC prescalar to be eight times faster than default
}


void loop() {

  // Read line sensor values continuously
  sensor_values[0] = analogRead(A0); //left line sensor
  sensor_values[1] = analogRead(A1); //right line sensor
  sensor_values[2] = analogRead(A2); //rear line sensor

  while (start == 0) {
    fft_detect();
  }

  if (sensor_values[0] < line_threshold && sensor_values[1] < line_threshold ) {
    Serial.println("Intersection!");
    get_wall_values();
    if (right_wall_value >= 150) { // *|
      digitalWrite(7, HIGH);
      if (front_wall_value > 150) { //-|
        digitalWrite(4, HIGH);
        turn_left();
      }
      else { //
        fft_detect(0); //check if need to turn left
        digitalWrite(4, LOW);
        drive_straight();
      }
      wall_before = true;
    }

    if (right_wall_value < 150) { //no right wall
      digitalWrite(7, LOW);
      if (wall_before) {
        turn_right();
      }
      else if (front_wall_value > 150) { // front wall
        digitalWrite(4, HIGH);
        turn_left();
      }
      else {  //no front wall, check if need to turn right
        digitalWrite(4, LOW);
        fft_detect(1);
        drive_straight();
      }
      wall_before = false;
    }

  }

  //Case:s traveling along line --> drive straight
  else if (sensor_values[0] > line_threshold && sensor_values[1] > line_threshold ) {
    drive_straight();
  }

  //Case: drifting off to the right --> correct left
  else if (sensor_values[0] < line_threshold ) {
    veer_left();
  }

  //Case: drifting off to the left --> correct right
  else if (sensor_values[1] < line_threshold ) {
    veer_right();
  }

  // Default: drive straight
  else {
    drive_straight();
  }
  wall_before = false;

}

//HELPER FUNCTIONS

void check_wall() {
  delay(25);
  get_wall_values();
  if (right_wall_value >= 150) { // *|
    digitalWrite(5, HIGH);
    if (front_wall_value > 150) { //-|
      digitalWrite(4, HIGH);
      move_back();
      turn_left();
    }
    else { // |
      digitalWrite(4, LOW);
      fft_detect(0); //check if need to turn left
      drive_straight();
    }
  }

  if (right_wall_value < 150) { //no right wall
    digitalWrite(5, LOW);
    if (wall_before) {
      move_back();
      turn_right();
    }
    else if (front_wall_value > 150) { // front wall
      digitalWrite(4, HIGH);
      move_back();
      turn_left();
    }
    else {
      fft_detect(1);
      drive_straight();
    }             //no front wall, check if need to turn right
    digitalWrite(4, LOW);
  }
}

void move_back() {
  get_wall_values();
  if (sensor_values[0] < line_threshold ) {
    veer_left();
  }
  //Case: drifting off to the left --> correct right
  else if (sensor_values[1] < line_threshold ) {
    veer_right();
  }
  else {
    while (!(sensor_values[0] < line_threshold  && sensor_values[1] < line_threshold )) {
      servoL.write(85);
      servoR.write(95);
      sensor_values[0] = analogRead(A0);
      sensor_values[1] = analogRead(A1);
      sensor_values[2] = analogRead(A2);
    }
  }
}

void veer_left() {
  servoL.write(80);
  servoR.write(55);
  sensor_values[0] = analogRead(A0);
  sensor_values[1] = analogRead(A1);
  sensor_values[2] = analogRead(A2);
}

void veer_right() {
  servoL.write(125);
  servoR.write(100);
  sensor_values[0] = analogRead(A0);
  sensor_values[1] = analogRead(A1);
  sensor_values[2] = analogRead(A2);
}

void turn_left() {
  get_wall_values();
  while (countdown > 0) {
    servoL.write(88);
    servoR.write(80);
    countdown = countdown - 1;
  }

  while (sensor_values[2] > line_threshold && sensor_values[0] < line_threshold && sensor_values[1] < line_threshold ) {
    servoL.write(88);
    servoR.write(80);
    sensor_values[0] = analogRead(A0);
    sensor_values[1] = analogRead(A1);
    sensor_values[2] = analogRead(A2);//delay(500);
  }
  countdown = 3000;
  update_direction(dir, 1);
  fft_detect(0);
  check_wall();

}

void turn_right() {
  get_wall_values();
  while (countdown > 0) {
    servoL.write(100);
    servoR.write(92);
    countdown = countdown - 1;
  }
  while (sensor_values[2] > line_threshold && sensor_values[0] < line_threshold && sensor_values[1] < line_threshold) {
    servoL.write(100);
    servoR.write(92);
    sensor_values[0] = analogRead(A0);
    sensor_values[1] = analogRead(A1);
    sensor_values[2] = analogRead(A2);
  }
  countdown = 3000;
  update_direction(dir, 0);
  fft_detect(1);
  check_wall();

}

void drive_straight() {
  servoL.write(95);
  servoR.write(85);
  sensor_values[0] = analogRead(A0);
  sensor_values[1] = analogRead(A1);
  sensor_values[2] = analogRead(A2);
}

void stop_drive() {
  servoL.write(90);
  servoR.write(90);
  sensor_values[0] = analogRead(A0);
  sensor_values[1] = analogRead(A1);
  sensor_values[2] = analogRead(A2);
}


//detect fft signal
void fft_detect() {
  cli();

  for (int i = 0 ; i < 512 ; i += 2) {
    fft_input[i] = analogRead(A5); // <-- NOTE THIS LINE
    fft_input[i + 1] = 0;
  }

  fft_window();
  fft_reorder();
  fft_run();
  fft_mag_log();
  sei();

  if (fft_log_out[3] > 70) {
    l = l + 1;
  }
  else {
    l = 0;
  }
  if (l >= 10) {
    count = count + 1;
    start = 1;
    drive_straight();
    digitalWrite(2, HIGH);  //flip select bit
    //Serial.println("660 HURTS !!!!!");
  }


if (start) {
  if (fft_log_out[26] > 60 || fft_log_out[25] > 60 || fft_log_out[27] > 60) {
    Serial.println("6KHz !!!!!");
    stop_drive();
    //digitalWrite(6, HIGH); //turn on indicator LED
    delay(2500);
    //else turn_left();
  }
  else {
    //digitalWrite(6, LOW); //turn off indicator LED
    drive_straight();
  }
  }
}


//packs information about a given intersection (presence of
// /treasures at walls, direction, etc) into a 24-bit array
// refer to github for encoding of this array
//NOTE: get_wall_values() must be called beforehand
byte[3] pack_intersection_info() {
  byte[3] info = {0, 0, 0};   //stores maze info
  info[0] = pack_bit_one(dir);
  //implement code to find treasures once we install the camera.
}

//helper for pack_intersection_info()
//takes the direction the robot is facing as an input (pass global dir var to this function)
//returns a byte in the following form:  [0|0|DIR|DIR|N|E|S|W]
// where N,E,S, and W are 1 if walls exist in those directions, 0 o/w
// DIR is a 2 bit value indicating the direction the robot is facing
// 00 -> N; 01 -> E; 10 -> S; 11 -> W;

byte pack_bit_one(int facing) {
  byte info = 0;
  int n = 0;
  int e = 0;
  int s = 0;
  int w = 0;
  int lwall = 0;
  int rwall = 0;
  int fwall = 0;
  if (left_wall_value > wall_threshold) {
    lwall = 1;
  }
  if (front_wall_value > wall_threshold) {
    fwall = 1;
  }
  if (right_wall_value > wall_threshold) {
    rwall = 1;
  }
  switch (facing) {
    case 0; //ROBOT IS FACING NORTH
      w = lwall;
      n = fwall;
      e = rwall;
      //(i know i don't need to explicitly write zeros to locations
      //initialized to be zero but it makes it more clear what is happening)
      bitWrite(info, 4, 0);
      bitWrite(info, 5, 0);
    case 1: //ROBOT IS FACING EAST
      n = lwall;
      e = fwall;
      s = rwall;
      bitWrite(info, 4, 1);
      bitWrite(info, 5, 0);
    case 2: //ROBOT IS FACING SOUTH
      e = lwall;
      s = fwall;
      w = rwall;
      bitWrite(info, 4, 0);
      bitWrite(info, 5, 1);
    case 3: //ROBOT IS FACING WEST
      s = lwall;
      w = fwall;
      n = rwall;
      bitWrite(info, 4, 1);
      bitWrite(info, 5, 1);
  }
  bitWrite(info, 0, w);
  bitWrite(info, 1, s);
  bitWrite(info, 2, e);
  bitWrite(info, 3, n);
  return info;
}

//call this after a turn to update global dir (direction) variable.
//takes as input the direction the robot is facing (pass global dir var),
//and the direction the robot is turning (0 -> Right turn, 1-> left turn)
void update_direction(int facing, int turn_dir) {
  switch (facing) {
    case 0; //ROBOT IS FACING NORTH
      if (turn_dir == 0) { // if robot is turning right
        dir = 1;
      }
      else {               // if robot is turning left
        dir = 3;
      }
    case 1: //ROBOT IS FACING EAST
      if (turn_dir == 0) { // if robot is turning right
        dir = 2;
      }
      else {               // if robot is turning left
        dir = 0;
      }
    case 2: //ROBOT IS FACING SOUTH
      if (turn_dir == 0) { // if robot is turning right
        dir = 3;
      }
      else {               // if robot is turning left
        dir = 1;
      }
    case 3: //ROBOT IS FACING WEST
      if (turn_dir == 0) { // if robot is turning right
        dir = 0;
      }
      else {               // if robot is turning left
        dir = 2;
      }
  }
}
}
