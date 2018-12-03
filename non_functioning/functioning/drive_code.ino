/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * Example for Getting Started with nRF24L01+ radios.
 *
 * This is an example of how to use the RF24 class.  Write this sketch to two
 * different nodes.  Put one of the nodes into 'transmit' mode by connecting
 * with the serial monitor and sending a 'T'.  The ping node sends the current
 * time to the pong node, which responds by sending the value back.  The ping
 * node can then see how long the whole cycle took.
 */
#include <Servo.h>

#define LOG_OUT 1 // use the log output function
#define FFT_N 128 // set to 256 point fft
#include <FFT.h> // include the library
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(9,10);
const uint64_t pipes[2] = { 0x0000000048LL, 0x0000000049LL };
typedef enum { role_ping_out = 1, role_pong_back } role_e;
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};
role_e role = role_pong_back;

unsigned long time;
unsigned long vals[4] = {1000, 9000, 2222, 192903};
int count = 0;


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

int line_threshold = 350;        //cutoff value b/w white and not white
int wall_threshold = 160;        //A wall exists if a wall sensor reads a value greater than this threshold

int countdown = 5000;
int max_countdown_value = 5000;
int dir = 2;                     //direction the robot is traveling in.
//0 -> N; 1 -> E; 2 -> S; 3 -> W;
//We assume the robot starts in the northwest corner
// traveling east by default
int width = 9;
int height = 9;
int maze_size = width * height;
byte visited[81]; //each entry in this array refers to a square
byte pos = 0;   //position in maze in raster coordinates
                  //NOTE: WE START AT POSITION 1 -- I.E. east of the intersection at position 0

int loffset= -1;  //add this value to the left wheel's speed when driving straight to adjust for
                 //veering

byte b1 = 0;  //second bit to be sent to base station
byte b2 = 0;  //third bit to be sent to base station
byte colors[4]; //stores the color/shape info at each wall any any given position

byte last = 0;//parity bit for radio transmission


void setup(void){
  //
  // Print preamble
  //
  ADCSRA &= ~(bit (ADPS0) | bit (ADPS1) | bit (ADPS2)); // clear prescaler bits
  ADCSRA |= bit (ADPS2); // set ADC prescalar to be eight times faster than default
  Serial.begin(57600);
  printf_begin();
  printf("\n\rRF24/examples/GettingStarted/\n\r");
  printf("ROLE: %s\n\r",role_friendly_name[role]);
  printf("*** PRESS 'T' to begin transmitting to the other node\n\r");

  //
  // Setup and configure rf radio
  //

  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);
  radio.setAutoAck(true);
  // set the channel
  radio.setChannel(0x50);
  // set the power
  // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  radio.setPALevel(RF24_PA_MIN);
  //RF24_250KBPS for 250kbs, RF24_1MBPS for 1Mbps, or RF24_2MBPS for 2Mbps
  radio.setDataRate(RF24_250KBPS);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  //radio.setPayloadSize(8);

  //
  // Open pipes to other nodes for communication
  //

  // This simple sketch opens two pipes for these two nodes to communicate
  // back and forth.
  // Open 'our' pipe for writing
  // Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)

  if ( role == role_ping_out )
  {
    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);
  }
  else
  {
    radio.openWritingPipe(pipes[1]);
    radio.openReadingPipe(1,pipes[0]);
  }

  //
  // Start listening
  //

  radio.startListening();

  //
  // Dump the configuration of the rf unit for debugging
  //

  radio.printDetails();
  time = 500;
  servoL.attach(3);
  servoR.attach(5);
  for (int k = 0; k<1; k++) {
    Serial.println(visited[k]);
  }
    //Digital Output Pins
  pinMode(6, OUTPUT); //Wall sensor mux select bit
  //  //LOW reads right sensor, HIGH reads left sensor
  pinMode(7, OUTPUT); //FFT mux select bit
  //  //LOW reads microphone, HIGH reads IR circuit
  digitalWrite(7, LOW);

}

void loop(void)
{
  audio_begin();
  line_follow_until_intersection();
  dfs(dir, 0);
  turn_around(); //after dfs completes the maze, we need to turn around to return to our starting position
  servoL.write(90);
  servoR.write(90);
  delay(100000); //stop for 100s
  Serial.println("END OF LOOP!");
  //IR_detect();

}
void transmit(unsigned long val) {
    // First, stop listenings so we can talk.
    radio.stopListening();
    // Take the time, and send it.  This will block until complete
    
    printf("Now sending %lu...",val);
    if (count < 3) {
      count = count + 1;
    }
    else {
      count = 0;
    }
    bool ok = radio.write( &val, sizeof(unsigned long) );

    if (ok)
      printf("ok...");
    else
      printf("failed.\n\r");

    // Now, continue listening
    radio.startListening();

    // Wait here until we get a response, or timeout (250ms)
    unsigned long started_waiting_at = millis();
    bool timeout = false;
    while ( ! radio.available() && ! timeout )
      if (millis() - started_waiting_at > 200 )
        timeout = true;

    // Describe the results
    if ( timeout )
    {
      printf("Failed, response timed out.\n\r");
    }
    else
    {
      // Grab the response, compare, and send to debugging spew
      unsigned long got_time;
      radio.read( &got_time, sizeof(unsigned long) );

      // Spew it
      printf("Got response %lu, round-trip delay: %lu\n\r",got_time,millis()-got_time);
    }

    // Try again 1s later
    delay(1000);
}


////////////////////
//HELPER FUNCTIONS//
////////////////////

//////////////////////////////////////
//TURNING + DRIVING HELPER FUNCTIONS//
//////////////////////////////////////
void spin_left() {
  drive_straight();
  delay(400);

  Serial.println("turning to get off of line");
  get_line_values();
  while (sensor_values[2] < line_threshold - 75) {
    servoL.write(80);
    servoR.write(80);
    get_line_values();
  }
  servoL.write(80);
  servoR.write(80);
  delay(250);

  get_line_values();
  Serial.println(sensor_values[2]);
  while (sensor_values[2] > line_threshold - 75) {
    servoL.write(80);
    servoR.write(80);
    get_line_values();
    //Serial.println("TURNING UNTIL MIDDLE SENSOR REACHES LINE");
  }
  //turn a lil bit more so the robot is aligned w the line
  delay(55);
  //back up until intersection
  servoL.write(85);
  servoR.write(95);
  get_line_values();
  while (!(sensor_values[0] < line_threshold && sensor_values[1] < line_threshold)) { // NOT INTERSECTION
      get_line_values();
  }
  stop_drive();
  delay(300);
  update_direction(dir, 1);
}

void spin_right() {
  drive_straight();
  delay(400);

  Serial.println("turning to get off of line");
  get_line_values();
  while (sensor_values[2] < line_threshold - 75) {
    servoL.write(100);
    servoR.write(100);
    get_line_values();
  }
  servoL.write(100);
  servoR.write(100);
  delay(250);

  get_line_values();
  Serial.println(sensor_values[2]);
  while (sensor_values[2] > line_threshold - 75) {
    servoL.write(100);
    servoR.write(100);
    get_line_values();
    //Serial.println("TURNING UNTIL MIDDLE SENSOR REACHES LINE");
  }
  //turn a lil bit more so the robot is aligned w the line
  delay(50);
  //back up until intersection
  servoL.write(85);
  servoR.write(95);
  get_line_values();
  while (!(sensor_values[0] < line_threshold && sensor_values[1] < line_threshold)) { // NOT INTERSECTION
      get_line_values();
  }
  stop_drive();
  delay(300);
  update_direction(dir, 0);
}
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
  delay(400);

  Serial.println("turning to get off of line");
  get_line_values();
  while (sensor_values[2] < line_threshold - 75) {
    servoL.write(80);
    servoR.write(80);
    get_line_values();
  }
  servoL.write(80);
  servoR.write(80);
  delay(250);

  get_line_values();
  Serial.println(sensor_values[2]);
  while (sensor_values[2] > line_threshold - 75) {
    servoL.write(80);
    servoR.write(80);
    get_line_values();
    //Serial.println("TURNING UNTIL MIDDLE SENSOR REACHES LINE");
  }
  //turn a lil bit more so the robot is aligned w the line
  delay(50);
  //fft_detect();
  update_position(dir, 1);
  update_direction(dir, 1);
}

void turn_right() {
  drive_straight();
  delay(400);
  
  Serial.println("turning to get off of line");
  get_line_values();
  while (sensor_values[2] < line_threshold - 75) {
    servoL.write(119);
    servoR.write(110);
    get_line_values();
  }
  servoL.write(110);
  servoR.write(110);
  delay(200);

  get_line_values();
  while (sensor_values[2] > line_threshold - 75) {
    servoL.write(100);
    servoR.write(100);
    get_line_values();
    //Serial.println("TURNING UNTIL MIDDLE SENSOR REACHES LINE");
  }
  //turn a lil bit more so the robot is aligned w the line
  delay(50);
  
  //fft_detect();
  update_position(dir, 0);
  update_direction(dir, 0);
}

void turn_around() {
  Serial.println("Turning Around");

  //drive straight to get the robot off of the intersection
  Serial.println("DRIVING STRAIGHT TO GET OFF INTERSECTION");
  drive_straight();
  delay(200);
  
  /*servoL.write(90);
  servoR.write(90);
  delay(500);*/

  //turn in place
  Serial.println("TURNING IN PLACE TAKE 1");
  servoL.write(110);
  servoR.write(110);
  delay(600); //delay so that the following while loop does 
              //not immedieately terminate
  get_line_values();
  while (sensor_values[2] > line_threshold - 75) {
    get_line_values();
    //Serial.println("TURNING UNTIL MIDDLE SENSOR REACHES LINE");
  }

  servoL.write(90);
  servoR.write(90);
  delay(50);
  update_direction(dir, 1);
  update_direction(dir, 1);
  update_position(dir, 2); //WOW I REALLY DON'T KNOW IF THIS IS GONNA WORK
}

void drive_straight() {
  servoL.write(95+loffset);
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
  while (1) {
    if (sensor_values[0] < line_threshold && sensor_values[1] < line_threshold ) { //INTERSECTION
      stop_drive();
      if (!IR_detect()) {//if no robot is detected
        Serial.println("NO ROBOT DETECTED");
        return;
      }
      else{
        Serial.println("ROBOT DETECTED");
        delay(1000);
      }
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
}

///////////
//GETTERS//
///////////

//obtains front_wall_value, right_wall_value and left_wall_value
//also writes to wall detection indicator LEDs
void get_wall_values() {
  digitalWrite(6, LOW);//set wall sensor select bit to read right wall sensor
  right_wall_value  = analogRead(A4);
  right_wall_value  = analogRead(A4);
  delay(50);
  digitalWrite(6, HIGH);//set wall sensor select bit to read left wall sensor
  delay(50);
  front_wall_value  = analogRead(A3);
  left_wall_value   = analogRead(A4);
  left_wall_value   = analogRead(A4);
  delay(50);
  digitalWrite(6, LOW);
  Serial.println("LEFT WALL!!!");
  Serial.println(left_wall_value);
  Serial.println("RIGHT WALL!!!");
  Serial.println(right_wall_value);
  Serial.println("FRONT WALL!!!");
  Serial.println(front_wall_value);
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
void radio_transmit(int n, int e, int s, int w) {
  // First, stop listening so we can talk.
  byte info[4] = {0, 0, 0, 0};   //stores maze info.
  info[0] = pack_bit_one(n, e, s, w);
  info[1] = pack_bit_two(colors[0],colors[1]);
  info[2] = pack_bit_three(colors[2],colors[3]);

  //LOOK WE JUST TRANSMIT THE RASTER POSITION NOW!
  info[3] = pos;
  
  Serial.println("VALUE SENT TO BASE:");
  Serial.println();
  unsigned long long_info = 0;
  for (int k = 0; k < 8; k++) {
    bitWrite(long_info, k, bitRead(info[0], k));
    bitWrite(long_info, k+8, bitRead(info[1], k));
    bitWrite(long_info, k+16, bitRead(info[2], k));
    bitWrite(long_info, k+24, bitRead(info[3], k));
  }
  Serial.println("VALUE SENT TO BASE:");
  for (int k = 0; k < 8; k++) {
      Serial.print(bitRead(info[0], 7-k));
  }

  Serial.println();
  transmit(long_info);
}

//helper for radio_transmit()
//takes the direction the robot is facing as an input (pass global dir var to this function)
//returns a byte in the following form:  [parity|0|DIR|DIR|N|E|S|W]
// where N,E,S, and W are 1 if walls exist in those directions, 0 o/w
// DIR is a 2 bit value indicating the direction the robot is facing
// 00 -> N; 01 -> E; 10 -> S; 11 -> W;
byte pack_bit_one(int n, int e, int s, int w) {
  byte info = 0;
  if (dir == 0) s = 0;
  if (dir == 1) w = 0;
  if (dir == 2) n = 0;
  if (dir == 3) e = 0;
  if (n || pos<width) {//IF NORTH WALL
    bitWrite(info, 3, 1);
  }
  if (e || ((pos+1)%width == 0)) {//IF EAST WALL
    bitWrite(info, 2, 1);
  }
  if (s || pos >= (height - 1)*width) {//IF SOUTH WALL
    bitWrite(info, 1, 1);
  }
  if (w || pos%width == 0) {//IF WEST WALL
    bitWrite(info, 0, 1);
  }

  //write direction to info. shouldn't be necessary but it can't hurt
  if (dir == 1 || dir == 3){
    bitWrite(info, 4, 1);
  }
  if (dir == 2 || dir == 3){
    bitWrite(info, 5, 1);
  }

  bitWrite(info, 7, last);
  last = (last + 1)%2; //switch last b/w 0 and 1
  
  return info;
}

//bit two is of the form [Es|Es|Ec|Ec|Ns|Ns|Nc|Nc]
//cn is the color/shape info from get_FPGA_data at the north wall
//ce is the color/shape info from get_FPGA_data at the east wall
byte pack_bit_two(byte cn,byte ce) {
  byte b_two = 0;
  for (int k = 0; k<4; k++){
    int temp_n = bitRead(cn, k);
    int temp_e = bitRead(ce, k);
    bitWrite(b_two, k, temp_n);
    bitWrite(b_two, k+4, temp_e);
  }
  return b_two;
}

//bit three is of the form [Ws|Ws|Wc|Wc|Ss|Ss|Sc|Sc]
//cs is the color/shape info from get_FPGA_data at the south wall
//cw is the color/shape info from get_FPGA_data at the west wall
byte pack_bit_three(byte cs, byte cw) {
  byte b_three = 0;
  for (int k = 0; k<4; k++){
    int temp_s = bitRead(cs, k);
    int temp_w = bitRead(cw, k);
    bitWrite(b_three, k, temp_s);
    bitWrite(b_three, k+4, temp_w);
  }
  return b_three;
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
  Serial.println("Direction is:");
  Serial.print(dir);
  Serial.println();
}

/////////
///MISC//
/////////

void copy(byte* src, byte* dst, int len) {
  memcpy(dst, src, sizeof(src[0])*len);
}

//please note: refers to global visited matrix + global position variable
//the direction the robot was travelling when dfs was called is passed to the function
void dfs(int calling_dir, bool backtrack) {
  stop_drive();
  //label intersection as visited
  Serial.println("DFS Called");
  Serial.print("Direction is:");
  Serial.println(dir);

  //check where you can move -- store this information in array "options"
  byte options[4]; //index 0 corresponds with N, 1 with E, 2 with S, 3 with W.
                   //stores 1 if you can move in that direction, 0 o/w
  for (int k = 0; k<4;k++) {
    options[k] = 0;
  }
  
  get_wall_values();
  
  if (right_wall_value < wall_threshold) {
    if (dir == 3) { //if facing west, indicate that you can move north
      options[0] = 1;
    }
    else {
      options[dir + 1] = 1;
    }
  }
  if (front_wall_value < wall_threshold) options[dir] = 1;

  if (left_wall_value < wall_threshold) {
    if (dir == 0) {//if facing north, indicate that you can move west
      options[3] = 1;
    }
    else {
      options[dir - 1] = 1;
    }
  }

  //THIS BLOCK RIGHT HERE NEEDS A LOTTA WORK
  //get colors:
  int tempdir = (dir+2)%4; //direction opposite of where the robot is facing 
  


  //if the space has not yet been visited, transmit info about it
  if (!visited[pos]) {
    radio_transmit(!options[0], !options[1], !options[2], !options[3]);
    Serial.println("TRANSMITTING!!!");
  }

    //block off visited spaces
  if (options[0]) {
    if (pos >= width && visited[pos - width]) {
      options[0] = 0;
      Serial.println("Visited[0] is: ");
      Serial.print(visited[pos - width]);
      Serial.println();
    }
  }
  if (options[1]) {
    if(pos != maze_size &&  visited[pos + 1]) {
      options[1] = 0;
    }
  }
  if (options[2]) {
    if((pos + width) < maze_size && visited[pos + width]) {
      options[2] = 0;
    }
  }
  if (options[3]) {
    if(pos != 0 && visited[pos - 1]) {
      options[3] = 0;
    }
  }

  Serial.println("Options:");
  for (int j = 0; j<4;j++) {
    Serial.println(options[j]);
  }

  //need to update visited after checking for colors + transmitting
  visited[pos] = 1;
  Serial.print("Visited[");
  Serial.print(pos);
  Serial.print("] is 1");
  Serial.println();

  
  for (int i = 0; i < 4; i++) {
    //block off visited spaces
    if (options[0]) {
      if (pos >= width && visited[pos - width]) {
        options[0] = 0;
        Serial.println("Visited[0] is: ");
        Serial.print(visited[pos - width]);
        Serial.println();
      }
    }
    if (options[1]) {
      if(pos != maze_size &&  visited[pos + 1]) {
        options[1] = 0;
        Serial.println("RESET");
      }
    }
    if (options[2]) {
      if((pos + width) < maze_size && visited[pos + width]) {
        options[2] = 0;
      }
    }
    if (options[3]) {
      if(pos != 0 && visited[pos - 1]) {
        options[3] = 0;
      }
    }

    if (options[i] == 1) { //NEEDS TO BE A CONDITION ABOUT WHETHER THE SPACE HAS BEEN VISITED
      //determine raster index if you're to move in direction k
      int posnext;
      if (i == 0) {
        posnext = pos - width;
      }
      if (i == 1) {
        posnext = pos + 1;
      }
      if (i == 2) {
        posnext = pos + width;
      }
      if (i == 3) {
        posnext = pos - 1;
      }
      //Serial.print("Posnext is: ");
      //Serial.print(posnext);
      //Serial.println();
      if (posnext > -1 && posnext < maze_size && visited[posnext] == 0)  {
        Serial.print("Travelling in direction: ");
        Serial.println(i);
        //explore in direction k
        int dir_old = dir; //HACKY FIX FOR BACKTRACKING ISSUE
        if (dir == i) {//if direction k is straight ahead
          update_position(dir, 2); //also need to explicitly call this
          drive_straight();
          Serial.println("Driving Straight");
          delay(1000);
        }
        if (dir == i + 1 || (dir == 0 && i == 3)) {//if direction k is to the left
          turn_left();
        }
        if (dir == i - 1 || (dir == 3 && i == 0)) { //if direction k is to the right
          turn_right();
        }
      //pretty sure we shouldn't have the condition where we have to turn 180°
      line_follow_until_intersection();
      Serial.println("Intersection");
      

      //INTERSECTION
      stop_drive();
      
      dfs(dir, 1);
      }
    }
  }
  
    //backtrack:
    //move back to space this function was recursively called from:
    //turn so that direction is the opposite of the direction the robot was travelling
    //at the time of the aforementioned recursive call

    if (backtrack) {
      Serial.println("backtracking");
      if (dir == calling_dir) {
        turn_around();
      }
      else if (dir == calling_dir + 1 || (dir == 0 && calling_dir == 3)) {
        Serial.println("Calling dir:");
        Serial.print(calling_dir);
        Serial.println("dir:");
        Serial.print(dir);
        Serial.println();
        
        turn_right();
      }
      else if (dir == calling_dir - 1 || (dir == 3 && calling_dir == 0)) {
        turn_left();
      }
      else {
        drive_straight();
        delay(400);
        update_position(dir,2);
      }
      Serial.print("Next position is:");
      Serial.print(pos);
      Serial.println();
      line_follow_until_intersection();
    }
    return; 
}

//info about treasure shape and color sent to arduino
//this information is transmitted as four bits in the form shown below:
// |X|X|X|X|shp|shp|col|col|
//where shp is a 2 bit value corresponding with shape info
//and col is a 2 bit value corresponding with color info
byte get_FPGA_data(){
  byte treasure = 0b00000000;
  digitalWrite(8, HIGH);
  delay(5); //wait for FPGA to compute treasure data
  bitWrite(treasure, 3, digitalRead(4)); //store MSB (shape bit 1)
  digitalWrite(8, LOW);
  digitalWrite(8, HIGH);
  bitWrite(treasure, 2, digitalRead(4)); //store shape bit 2
  digitalWrite(8, LOW);
  digitalWrite(8, HIGH);
  bitWrite(treasure, 1, digitalRead(4)); //store color bit 1
  digitalWrite(8, LOW);
  digitalWrite(8, HIGH);
  bitWrite(treasure, 0, digitalRead(4)); //store color bit 2
  digitalWrite(8, LOW);
  //Serial.println(treasure);
  //decode_treasure_info(treasure);
  return treasure;
}


//BIN 52 = 660 Hz AUDIO FFT
//BIN 43 =
void audio_begin() {
  servoL.detach();
  servoR.detach();
  digitalWrite(7, LOW);
  while (start == 0) {
    cli();
    for (int i = 0 ; i < 128 ; i += 2) {
      fft_input[i] = analogRead(A5); // <-- NOTE THIS LINE
      fft_input[i + 1] = 0;
      delayMicroseconds(500);
    }
    fft_window();
    fft_reorder();
    fft_run();
    fft_mag_log();
    sei();
    /*for (int k = 0; k<64; k++) {
      Serial.print(fft_log_out[k]);
      Serial.print(" ");
    }
    Serial.println();
    count = count + 1;*/
    //Serial.println(analogRead(A4));
    //Serial.println(analogRead(A5));

    Serial.print(fft_log_out[43]);
    Serial.println();
    
    
    if (fft_log_out[43] > 75) {
      Serial.println("660 Hz Detected");
      servoL.attach(3);
      servoR.attach(5);
      servoL.write(90);
      servoR.write(90);
      start =1; 
      return;
    }
    else {
    }
  }
}

//BIN 33 = 6.6 KHz IR signal
//BIN 12,13,14??
//returns true if a 6.6 KHz signal is detected, false otherwise.
bool IR_detect() {
  servoL.detach();
  servoR.detach();
  byte sum0 = 0;
  byte sum1 = 0;
  byte sum2 = 0;
  byte sum3 = 0;
  digitalWrite(7, HIGH);
  for (int k = 0; k <= 50; k++) {
    for (int i = 0 ; i < 128 ; i += 2) {
      fft_input[i] = analogRead(A5); // <-- NOTE THIS LINE
      fft_input[i + 1] = 0;
    }
    fft_window();
    fft_reorder();
    fft_run();
    fft_mag_log();
    sei();
    /*for (int k = 0; k<64; k++) {
      Serial.print(fft_log_out[k]);
      Serial.print(" ");
    }
    Serial.println();*/
    
    Serial.println();
    Serial.println(fft_log_out[14]);
    if (fft_log_out[12] > 60){
      sum0 = sum0 + 1;
    }
    if(fft_log_out[13] > 60) {
      sum1 = sum1 + 1;
    }
    if(fft_log_out[14] > 60) { 
      sum2 = sum2 + 1;    
    }
    if(fft_log_out[15] > 60) { 
      sum3 = sum3 + 1;    
    }
  }
  if (sum0 > 25 || sum1 > 25 || sum2 > 25|| sum3 > 25) {
    delay(300);
    return true;
  }
  else {
    Serial.println();
    Serial.print("Sum0 is: ");
    Serial.print(sum0);
    Serial.print(" ");
    Serial.print("Sum1 is: ");
    Serial.print(sum1);
    Serial.print(" ");
    Serial.print("Sum2 is: ");
    Serial.print(sum2);
    Serial.print(" ");
    Serial.print("Sum3 is: ");
    Serial.print(sum3);
    Serial.print(" ");
    Serial.println();
    servoL.attach(3);
    servoR.attach(5);
    servoL.write(90);
    servoR.write(90);
    return false;
  }
}
