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
typedef enum { role_ping_out = 1, role_pong_back } role_e;
role_e role = role_ping_out;

Servo servoL;                     //instantiate servos
Servo servoR;

bool start = 0;                  //0 if 660Hz has not been detected. 1 o/w
int l = 0;
unsigned int sensor_values[3];   //store sensor values
                                 //sensor_values[0] -> left sensor; sensor_values[1] -> right sensor;
                                 //sensor_values[2] -> rear sensor.
int front_wall_value;            //store front wall sensor value
int right_wall_value;            //store right wall sensor value
int left_wall_value;             //store left wall sensor value

int line_threshold = 300;        //cutoff value b/w white and not white
int wall_threshold = 150;        //A wall exists if a wall sensor reads a value greater than this threshold

int countdown = 5000;
int max_countdown_value = 5000;
int dir = 1;                     //direction the robot is traveling in.
                                 //0 -> N; 1 -> E; 2 -> S; 3 -> W;
                                 //We assume the robot starts in the northwest corner
                                 // traveling east by default
int width = 9;
int height = 9;
int maze_size = width * height;
byte visited[81]; //each entry in this array refers to a square
int pos    = 0;   //position in maze in raster coordinates

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
  radio.setPALevel(RF24_PA_MIN);
  //RF24_250KBPS for 250kbs, RF24_1MBPS for 1Mbps, or RF24_2MBPS for 2Mbps
  radio.setDataRate(RF24_250KBPS);

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
}

void loop() {
  line_follow_until_intersection();
  dfs(dir);
  turn_around(); //after dfs completes the maze, we need to turn around to return to our starting position
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
  radio_transmit();
  update_position(dir, 1);
  update_direction(dir, 1);
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
  radio_transmit();
  update_position(dir, 0);
  update_direction(dir, 0);
}

void turn_around() {
  Serial.println("Turning Around");

  drive_straight();
  delay(200);

  servoL.write(90);
  servoR.write(90);
  delay(500);

  servoL.write(100);
  servoR.write(92);
  delay(300);

  servoL.write(90);
  servoR.write(90);
  delay(500);

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

  servoL.write(80);
  servoR.write(100);
  delay(500);

  servoL.write(90);
  servoR.write(90);
  delay(500);

  servoL.write(110);
  servoR.write(90);
  delay(700);

  servoL.write(90);
  servoR.write(90);
  delay(500);

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
  radio_transmit();
  update_direction(dir, 1);
  update_direction(dir, 1);
  update_position(dir, 2); //WOW I REALLY DON'T KNOW IF THIS IS GONNA WORK
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

void line_follow_until_intersection() {
  get_line_values();

  if (sensor_values[0] < line_threshold && sensor_values[1] < line_threshold ) { //INTERSECTION
    return;
  }
  //Case: traveling along line --> drive straight
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
  info[0] = pack_bit_one(dir);
  Serial.println("VALUE SENT TO BASE:");
  Serial.println(info[0]);
  radio.stopListening();
  // NOTE: the maze array is defined here
  // Send the maze in a single payload
  if (role == role_ping_out) {
    // First, stop listening so we can talk.
    radio.stopListening();
    radio.openWritingPipe(pipes[0]);
    bool ok = radio.write( info, sizeof(info) );
    Serial.println("OK:");
    Serial.println(ok);
    if (ok) {
      //printf("ok, sending. \n");
      for (int i = 0; i < 3; i++) {
        //  Serial.println(transmit[i],BIN);
      };
    }
    else {
      //printf("failed.\n\r");
      for (int i = 0; i < 3; i++) {
        //Serial.println(transmit[i]);
      }
    }
    radio.startListening();
    delay(1000); //give time for other end to receive
  }
}

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
  if (dir == 0) {//ROBOT IS FACING NORTH
    w = lwall;
    n = fwall;
    e = rwall;
    //(i know i don't need to explicitly write zeros to locations
    //initialized to be zero but it makes it more clear what is happening)
    bitWrite(info, 4, 0);
    bitWrite(info, 5, 0);
  }
  if (dir == 1) {//ROBOT IS FACING EAST
    n = lwall;
    e = fwall;
    s = rwall;
    bitWrite(info, 4, 1);
    bitWrite(info, 5, 0);
  }
  if (dir == 2) { //ROBOT IS FACING SOUTH
    e = lwall;
    s = fwall;
    w = rwall;
    bitWrite(info, 4, 0);
    bitWrite(info, 5, 1);
  }
  if (dir == 3) { //ROBOT IS FACING WEST
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

//MUST BE CALLED BEFORE DIRECITON HAS BEEN UPDATED
//updates global position variable ('pos')
//takes as input the direction the robot is facing (pass global dir var),
//and the direction the robot is turning (0 -> Right turn, 1-> left turn, 2-> no turn)
void update_position(int facing, int turn_dir) {
  if (facing == 0) {//ROBOT IS FACING NORTH
    if (turn_dir == 0) { // if robot is turning right
      //x++
      pos = pos + 1;
    }
    else if (turn_dir == 1) { // if robot is turning left
      //x--
      pos = pos - 1;
    }
    else {               // if robot is driving straight
      //y++
      pos = pos - width;
    }
  }
  if (facing == 1) {//ROBOT IS FACING EAST
    if (turn_dir == 0) { // if robot is turning right
      //y--
      pos = pos + width;
    }
    else if (turn_dir == 1) { // if robot is turning left
      //y++
      pos = pos - width;
    }
    else {               // if robot is driving straight
      //x++
      pos = pos + 1;
    }
  }
  if (facing == 2) {//ROBOT IS FACING SOUTH
    if (turn_dir == 0) { // if robot is turning right
      //x--
      pos = pos - 1;
    }
    else if (turn_dir == 1) { // if robot is turning left
      //x++
      pos = pos + 1;
    }
    else {               // if robot is driving straight
      //y--
      pos = pos + width;
    }
  }
  if (facing == 3) {//ROBOT IS FACING WEST
    if (turn_dir == 0) { // if robot is turning right
      //y++
      pos = pos - width;
    }
    else if (turn_dir == 1) { // if robot is turning left
      //y--
      pos = pos + width;
    }
    else {               // if robot is driving straight
      //x--
      pos = pos - 1;
    }
  }
}

//call at the end of turn_left() and turn_right() turn to update global dir (direction) variable.
//takes as input the direction the robot is facing (pass global dir var),
//and the direction the robot is turning (0 -> Right turn, 1-> left turn)
void update_direction(int facing, int turn_dir) {
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
    if (fft_log_out[3] > 80) {
      l = l + 1;
    }
    else {
      l = 0;
    }
    if (l >= 15) {
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
      stop_drive;
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

//please note: refers to global visited matrix + global position variable
//the direction the robot was travelling when dfs was called is passed to the function
void dfs(int calling_dir) {
  //label intersection as visited
  visited[pos] = 1;

  //check where you can move -- store this information in array "options"
  byte options[4]; //index 0 corresponds with N, 1 with E, 2 with S, 3 with W.
  //stores 1 if you can move in that direction, 0 o/w
  get_wall_values();
  if (right_wall_value) {
    if (dir == 3) { //if facing west, indicate that you can move north
      options[0] = 1;
    }
    else {
      options[dir + 1] = 1;
    }
  }
  if (front_wall_value) options[dir] = 1;

  if (left_wall_value) {
    if (dir == 0) {//if facing north, indicate that you can move west
      options[3] = 1;
    }
    else {
      options[dir - 1] = 1;
    }
  }

  //for each turn option:
  //if not yet visited, then visit
  for (int k = 0; k < 4; k++) {
    if (options[k] == 1 && visited[k] == 0) {
      //explore in direction k
      if (dir == k) {//if direction k is straight ahead
        radio_transmit(); //need to explicitly call radio_transmit here, as there is no turn.
        update_position(dir, 2); //also need to explicitly call this
        drive_straight();
      }
      if (dir == k + 1 || (dir == 0 && k == 3)) {//if direction k is to the left
        turn_left();
      }
      if (dir == k - 1 || (dir == 3 && k == 0)) { //if direction k is to the right
        turn_right();
      }
      //pretty sure we shouldn't have the condition where we have to turn 180°
      line_follow_until_intersection();

      //INTERSECTION
      stop_drive();
      //NEED TO UPDATE POSITION -- PROBABLY SHOULD HAVE HELPER FUNCTION FOR THIS -- MAYBE ALREADY WRITTEN IN BASE STATION CODE
      dfs(dir);
    }

    //backtrack:
    //move back to space this function was recursively called from:
    //turn so that direction is the opposite of the direction the robot was travelling
    //at the time of the aforementioned recursive call

    //these turns probably shouldn't transmit anything??
    if (dir == calling_dir) {
      turn_around();
    }
    else if (dir == calling_dir + 1 || (dir == 0 && calling_dir == 3)) {
      turn_right();
    }
    else if (dir == calling_dir - 1 || (dir == 3 && calling_dir == 0)) {
      turn_left();
    }
    line_follow_until_intersection();
  }
}
