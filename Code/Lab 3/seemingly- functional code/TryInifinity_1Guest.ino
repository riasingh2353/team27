#include <Servo.h>

#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft
#include <FFT.h> // include the library

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
RF24 radio(9, 10);
//pipe addresses
const uint64_t pipes[2] = { 0x0000000048LL, 0x0000000049LL };

//byte transmit[3] = {10, 20, 30};//wall_encodings[x][3];

int count = 0;
typedef enum { role_ping_out = 1, role_pong_back } role_e;
role_e role = role_ping_out;
byte ftran = 1;

Servo servoL;                     //instantiate servos
Servo servoR;

bool start = 1;                  //0 if 660Hz has not been detected. 1 o/w
int l = 0;
unsigned int sensor_values[3];   //store sensor values
//sensor_values[0] -> left sensor; sensor_values[1] -> right sensor;
//sensor_values[2] -> rear sensor.
int          front_wall_value;   //store front wall sensor value
int          right_wall_value;   //store right wall sensor value
int          left_wall_value;    //store left wall sensor value

int line_threshold = 300;        //cutoff value b/w white and not white
int wall_threshold = 150;        //A wall exists if a wall sensor reads a value greater than this threshold

int countdown = 5000;
int max_countdown_value = 5000;
bool wall_before = false;
int dir = 1;                     //direction the robot is traveling in.
int dir_old = 1;
//0 -> N; 1 -> E; 2 -> S; 3 -> W;
//We assume the robot starts in the northwest corner
// traveling east by default
bool success = false;


void setup() {
  ADCSRA &= ~(bit (ADPS0) | bit (ADPS1) | bit (ADPS2)); // clear prescaler bits
  ADCSRA |= bit (ADPS2); // set ADC prescalar to be eight times faster than default
  Serial.begin(57600);

  // Setup and configure rf radio
  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15, 15);
  radio.setAutoAck(true);
  // set the channel
  radio.setChannel(0x50);
  // set the power
  // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  radio.setPALevel(RF24_PA_HIGH);
  //RF24_250KBPS for 250kbs, RF24_1MBPS for 1Mbps, or RF24_2MBPS for 2Mbps
  radio.setDataRate(RF24_1MBPS);

  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1, pipes[1]);

  servoL.attach(3);
  servoR.attach(5);

  //Digital Output Pins
  pinMode(6, OUTPUT); //Wall sensor mux select bit
  //  //LOW reads right sensor, HIGH reads left sensor
  pinMode(7, OUTPUT); //FFT mux select bit
  //  //LOW reads microphone, HIGH reads IR circuit
  digitalWrite(7, LOW);
        
        
  servoL.attach(3);
  servoR.attach(5);

  servoL.write(90);
  servoR.write(90);

  get_wall_values();
  radio_transmit(); //receiver always seems to drop first sent payload
  //byte zeros[3] = {0,0,0};
  //bool ok = radio.write(zeros,3);
  //Serial.println(ok);
}

void loop() {
  //
    while (start == 0) {
      Serial.println("waiting");
      fft_detect();
    }
  // Read line sensor values continuously
  get_line_values();

  if (sensor_values[0] < line_threshold && sensor_values[1] < line_threshold ) { //INTERSECTION
    get_line_values();
    if (sensor_values[0] < line_threshold && sensor_values[1] < line_threshold ) {
      Serial.println("Intersection!");
      intersection();
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
}
////////////////////
//HELPER FUNCTIONS//
////////////////////

//////////////////////////////////////
//TURNING + DRIVING HELPER FUNCTIONS//
//////////////////////////////////////

void veer_left() {
  servoL.write(80);
  servoR.write(55);
  get_line_values();
}

void veer_right() {
  servoL.write(125);
  servoR.write(100);
  get_line_values();
}
void turn_left() {

  drive_straight();
  delay(200);

  servoL.write(88);
  servoR.write(80);
  delay(300);

  servoL.write(88);
  servoR.write(80);
  get_line_values();
  while (sensor_values[2] > line_threshold - 75) {
    get_line_values();
    //Serial.println("TURNING UNTIL MIDDLE SENSOR REACHES LINE");
  }
  fft_detect();
  update_direction(dir, 1);
  radio_transmit();
}

void turn_right() {
  drive_straight();
  delay(200);


  servoL.write(100);
  servoR.write(92);
  delay(300);


  servoL.write(100);
  servoR.write(92);
  get_line_values();
  while (sensor_values[2] > line_threshold - 75) {
    get_line_values();
    //Serial.println("TURNING UNTIL MIDDLE SENSOR REACHES LINE");
  }
  fft_detect();
  update_direction(dir, 0);
  radio_transmit();
}

void turn_around() {
  Serial.println("Turning Around");

  drive_straight();
  delay(200);

  //servoL.write(90);
  //servoR.write(90);
  //delay(500);

  servoL.write(100);
  servoR.write(92);
  delay(300);

  //servoL.write(90);
  //servoR.write(90);
  //delay(500);

  servoL.write(100);
  servoR.write(92);
  get_line_values();
  while (sensor_values[2] > line_threshold - 75) {
    get_line_values();
    //Serial.println("TURNING UNTIL MIDDLE SENSOR REACHES LINE");
  }
  //servoL.write(90);
  //servoR.write(90);
  //delay(500);

  servoL.write(80);
  servoR.write(100);
  delay(500);

  //servoL.write(90);
  //servoR.write(90);
  //delay(500);

  servoL.write(110);
  servoR.write(90);
  delay(700);

  //servoL.write(90);
  //servoR.write(90);
  //delay(500);

  servoL.write(100);
  servoR.write(92);
  get_line_values();
  while (sensor_values[2] > line_threshold - 75) {
    get_line_values();
    //Serial.println("TURNING UNTIL MIDDLE SENSOR REACHES LINE");
  }

  servoL.write(90);
  servoR.write(90);
  delay(500);
  update_direction_turn_around();
  radio_transmit();
}

void drive_straight() {
  servoL.write(95);
  servoR.write(85);
  get_line_values();
}

void stop_drive() {
  servoL.write(90);
  servoR.write(90);
  get_line_values();
}

///////////
//GETTERS//
///////////

//obtains front_wall_value, right_wall_value and left_wall_value
//also writes to wall detection indicator LEDs
void get_wall_values() {
  digitalWrite(6, LOW);//set wall sensor select bit to read right wall sensor
  right_wall_value  = analogRead(A4);
  digitalWrite(6, HIGH);//set wall sensor select bit to read left wall sensor
  front_wall_value  = analogRead(A3);
  left_wall_value   = analogRead(A4);
}

//obtains left, right, and rear wall sensor values
void get_line_values() {
  sensor_values[0] = analogRead(A0); //left line sensor
  sensor_values[1] = analogRead(A1); //right line sensor
  sensor_values[2] = analogRead(A2); //rear line sensor
  //Serial.println(sensor_values[0]);
  //Serial.println(sensor_values[1]);
  //Serial.println(sensor_values[2]);
}


//packs information about a given intersection (presence of walls,
//treasures at walls, direction, etc) into a 24-bit array
//then sends this array to the base station
// refer to github for encoding of this array
//NOTE: get_wall_values() must be called beforehand
void radio_transmit() {
  // First, stop listening so we can talk.
  byte info[3] = {0, 0, 0};   //stores maze info.
  info[0] = pack_bit_one(dir_old);
  Serial.println("VALUE SENT TO BASE:");
  Serial.println(info[0]);
  radio.stopListening();
  // NOTE: the maze array is defined here
  // Send the maze in a single payload
  if (role == role_ping_out) {
    // First, stop listening so we can talk.
    radio.stopListening();
    radio.openWritingPipe(pipes[0]);
    servoL.write(90);
    servoR.write(90);
      bool ok = radio.write( info, sizeof(info) );

    //servoL.write(90);
    //servoR.write(90);
    radio.startListening();
    Serial.println(ok);
    //delay(250);
    //while(!success) {
    if (!ok) {
      radio.stopListening();
      bool ok = radio.write(info, sizeof(info));
    }
    radio.startListening();
    drive_straight();
    //delay(1000); //give time for other end to receive
  }
}

/*
void radio_transmit() {
  // First, stop listening so we can talk.
  Serial.println("transmitting......");
  //byte info[3] = {0, 0, 0};   //stores maze info.
  //info[0] = pack_bit_one(dir);
  //radio.stopListening();
  // NOTE: the maze array is defined here
  // Send the maze in a single payload
  // Serial.println(count);
  if (role == role_ping_out) {
    // First, stop listening so we can talk.
    radio.stopListening();
    // NOTE: the maze array is defined here
    // Send the maze in a single payload
     Serial.println("Sending");
    bool ok = radio.write( &transmit, sizeof(transmit) );
    if (ok) {
      Serial.println("ok, sending");
      for (int i = 0; i < 3; i++) {
        //  Serial.println(transmit[i],BIN);
      }
     //  Serial.println(count);
    }
    else {
      Serial.println("failed");
      for (int i = 0; i < 3; i++) {
        //  Serial.println(transmit[i]);
      }
    }
    delay(20); //give time for other end to receive
  }
   if ( role == role_pong_back )
  {
    // if there is data ready
    if ( radio.available() )
    {
      // Dump the payloads until we've gotten everything
      unsigned long got_time;
      bool done = false;
      while (!done)
      {
        // Fetch the payload, and see if this was the last one.
        done = radio.read( &got_time, sizeof(unsigned long) );
        // Spew it
        printf("Got payload %lu...",got_time);
        // Delay just a little bit to let the other unit
        // make the transition to receiver
        delay(20);
      }
      // First, stop listening so we can talk
      radio.stopListening();
      // Send the final one back.
      radio.write( &got_time, sizeof(unsigned long) );
      printf("Sent response.\n\r");
      // Now, resume listening so we catch the next packets.
      radio.startListening();
    }
  }
}*/

//helper for radio_transmit()
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
  if (facing == 0) {//ROBOT IS FACING NORTH
      w = lwall;
      n = fwall;
      e = rwall;
      if(ftran == 1){
        s = 1;
        ftran = 0;
      }
      //(i know i don't need to explicitly write zeros to locations
      //initialized to be zero but it makes it more clear what is happening)
  }
  if (facing == 1) {//ROBOT IS FACING EAST
      n = lwall;
      e = fwall;
      s = rwall;
      if(ftran == 1){
        w = 1;
        ftran = 0;
      }
  }
  if (facing == 2){//ROBOT IS FACING SOUTH
      e = lwall;
      s = fwall;
      w = rwall;
      if(ftran == 1){
        n = 1;
        ftran = 0;
      }
  }
  if (facing == 3){//ROBOT IS FACING WEST
      s = lwall;
      w = fwall;
      n = rwall;
      if(ftran == 1){
        e = 1;
        ftran = 0;
      }
  }
  if (dir == 0) {
      bitWrite(info, 4, 0);
      bitWrite(info, 5, 0);
  }
  if (dir == 1) {
      bitWrite(info, 4, 1);
      bitWrite(info, 5, 0);
  }
  if (dir == 2) {
      bitWrite(info, 4, 0);
      bitWrite(info, 5, 1);
  }
  if (dir == 3) {
      bitWrite(info, 4, 1);
      bitWrite(info, 5, 1);
  }
  bitWrite(info, 0, w);
  bitWrite(info, 1, s);
  bitWrite(info, 2, e);
  bitWrite(info, 3, n);
/*
  Serial.println("DIR =");
  Serial.println(dir);
  Serial.println("Lwall =");
  Serial.println(lwall);
  Serial.println("Rwall =");
  Serial.println(rwall);
  Serial.println("Fwall =");
  Serial.println(fwall);
  Serial.println("n =");
  Serial.println(n);
  Serial.println("e =");
  Serial.println(e);
  Serial.println("s =");
  Serial.println(s);
  Serial.println("w =");
  Serial.println(w);*/

    return info;

}

///////////////////////////////////////////
////MAZE TRAVERSAL LOGIC HELPER FUNCTIONS//
///////////////////////////////////////////

//call at the end of turn_left() and turn_right() turn to update global dir (direction) variable.
//takes as input the direction the robot is facing (pass global dir var),
//and the direction the robot is turning (0 -> Right turn, 1-> left turn)
void update_direction(int facing, int turn_dir) {
    dir_old = dir;
    if (facing == 0) {//ROBOT IS FACING NORTH
      if (turn_dir == 0) { // if robot is turning right
        dir = 1;
      }
      else {               // if robot is turning left
        dir = 3;
      }
    }
    if (facing == 1) {//ROBOT IS FACING EAST
      if (turn_dir == 0) { // if robot is turning right
        dir = 2;
      }
      else {               // if robot is turning left
        dir = 0;
      }
    }
    if (facing == 2) {//ROBOT IS FACING SOUTH
      if (turn_dir == 0) { // if robot is turning right
        dir = 3;
      }
      else {               // if robot is turning left
        dir = 1;
      }
    }
    if (facing == 3) {//ROBOT IS FACING WEST
      if (turn_dir == 0) { // if robot is turning right
        dir = 0;
      }
      else {               // if robot is turning left
        dir = 2;
      }
    }
}
void update_direction_turn_around() {
  dir_old = dir;
  if (dir == 0) {
    dir = 2;
  }else if (dir == 1) {
    dir = 3;
  }else if (dir == 2) {
    dir = 0;
  }else{
    dir = 1;
  }
}

//Called when the robot reaches an intersection. reads wall sensor values and determines
//whether to turn or drive straight. Also sets wall_before to appropriate values
void intersection() {
  get_wall_values();
  fft_detect();
  //determine whether to turn
  if (front_wall_value > wall_threshold) {
    if (right_wall_value > wall_threshold && left_wall_value > wall_threshold) {
      Serial.println("LEFT WALL VALUE");
      Serial.println(left_wall_value);
      Serial.println("RIGHT WALL VALUE");
      Serial.println(right_wall_value);
      turn_around();
      wall_before = true;
    }
    else if (right_wall_value > wall_threshold) {//WALLS ON FRONT AND RIGHT
      Serial.println("TURNING LEFT");
      turn_left();
      wall_before = true;
    }
    else if (left_wall_value > wall_threshold) {//WALLS ON FRONT AND LEFT
      turn_right();
      wall_before = false;
    }
    else {//WALL ON FRONT ONLY
      turn_left(); //arbitrary. the robot could also turn right here
      wall_before = false;
    }
  }
  else if (right_wall_value < wall_threshold && wall_before) { //SPECIAL CASE
    turn_right();
    wall_before = false;
  }
  else {//default case -- occurs when there is no wall, or wall only on right/left side
    Serial.println("DRIVING STRAIGHT");
    radio_transmit(); //need to manually call radio_transmit here, as there is no turn.
    drive_straight();
    delay(200);
    if (right_wall_value > wall_threshold) {
      wall_before = true;
    }
    else {
      wall_before = false;
    }
  }
}

/////////
///MISC//
/////////

void copy(byte* src, byte* dst, int len) {
  memcpy(dst, src, sizeof(src[0])*len);
}

void fft_detect() {
  servoL.detach();
  servoR.detach();

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
  if (!start) {
    digitalWrite(7, LOW);
    Serial.println(fft_log_out[3]);
    if (fft_log_out[3] > 75) {
      l = l + 1;
    }
    else {
      l = 0;
    }
    if (l >= 10) {
      digitalWrite(7, HIGH);
      servoL.attach(3);
      servoR.attach(5);
      drive_straight();
      start = 1;
      //flip select bit
      Serial.println("660 HURTS !!!!!");
    }
  }
  else if (start) {
    if (fft_log_out[26] > 60 || fft_log_out[25] > 60 || fft_log_out[27] > 60) {
      Serial.println("6KHz !!!!!");
      servoL.attach(3);
      servoR.attach(5);
      stop_drive();
      //digitalWrite(6, HIGH); //turn on indicator LED
      delay(2500);
      //else turn_left();
    }
    else {
      //digitalWrite(6, LOW); //turn off indicator LED
      servoL.attach(3);
      servoR.attach(5);
      drive_straight();
    }
  }
  Serial.println(start);
}
