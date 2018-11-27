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

int line_threshold = 350;        //cutoff value b/w white and not white
int wall_threshold = 130;        //A wall exists if a wall sensor reads a value greater than this threshold

int countdown = 5000;
int max_countdown_value = 5000;
int dir = 1;                     //direction the robot is traveling in.
//0 -> N; 1 -> E; 2 -> S; 3 -> W;
//We assume the robot starts in the northwest corner
// traveling east by default
int width = 4;
int height = 4;
int maze_size = width * height;
byte visited[16]; //each entry in this array refers to a square
byte pos = 1;   //position in maze in raster coordinates
                  //NOTE: WE START AT POSITION 1 -- I.E. east of the intersection at position 0

int loffset= -1;  //add this value to the left wheel's speed when driving straight to adjust for
                 //veering

byte b1 = 0;  //second bit to be sent to base station
byte b2 = 0;  //third bit to be sent to base station
byte colors[4]; //stores the color/shape info at each wall any any given position

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
  Serial.println("END OF LOOP!");
  servoL.write(90);
  servoR.write(90);
  delay(100000); //stop for 100s
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
    Serial.println("TURNING UNTIL MIDDLE SENSOR REACHES LINE");
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
void radio_transmit(int n, int e, int s, int w) {
  // First, stop listening so we can talk.
  byte info[4] = {0, 0, 0};   //stores maze info.
  info[0] = pack_bit_one(n, e, s, w);
  info[1] = pack_bit_two(colors[0],colors[1]);
  info[2] = pack_bit_three(colors[2],colors[3]);

  //LOOK WE JUST TRANSMIT THE RASTER POSITION NOW!
  info[3] = pos;
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
      for (int i = 0; i < 4; i++) {
        //  Serial.println(transmit[i],BIN);
      };
    }
    else {
      //printf("failed.\n\r");
      for (int i = 0; i < 4; i++) {
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
byte pack_bit_one(int n, int e, int s, int w) {
  byte info = 0;
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
    bitWrite(info, 0, 0);
  }

  //write direction to info. shouldn't be necessary but it can't hurt
  if (dir == 1 || dir == 3){
    bitWrite(info, 4, 1);
  }
  if (dir == 2 || dir == 3){
    bitWrite(info, 5, 1);
  }
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
  Serial.println("Options:");
  for (int j = 0; j<4;j++) {
    Serial.println(options[j]);
  }

  //THIS BLOCK RIGHT HERE NEEDS A LOTTA WORK
  //get colors:
  int tempdir = (dir+2)%4; //direction opposite of where the robot is facing 
  if(!visited[pos]){
    for (int k = 0;k<4;k++){
      if (!options[k] && k != tempdir) {
        if (dir == (k+2)%4) {//if direction k is behind ahead
          spin_right();
          spin_right();
        }
        if (dir == k + 1 || (dir == 0 && k == 3)) {//if direction k is to the left
          spin_left(); //this needs to update dir
        }
        if (dir == k - 1 || (dir == 3 && k == 0)) { //if direction k is to the right
          spin_right(); //this needs to update dir
        }
      colors[k] = get_FPGA_data();
      }
      else {
        colors[k] = 0;
      }
    }
  }

  //if the space has not yet been visited, transmit info about it
  if (!visited[pos]) {
    radio_transmit(options[0], options[1], options[2], options[3]);
  }


  //need to update visited after checking for colors + transmitting
  visited[pos] = 1;
  Serial.print("Visited[");
  Serial.print(pos);
  Serial.print("] is 1");
  Serial.println();

  
  for (int i = 0; i < 4; i++) {
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
      //NEED TO UPDATE POSITION -- PROBABLY SHOULD HAVE HELPER FUNCTION FOR THIS -- MAYBE ALREADY WRITTEN IN BASE STATION CODE
      dfs(dir);
      }
      /*else{
        Serial.print("Not Travelling in direction: ");
        Serial.print(i);
        Serial.println();
      }*/
    }
  }
    //backtrack:
    //move back to space this function was recursively called from:
    //turn so that direction is the opposite of the direction the robot was travelling
    //at the time of the aforementioned recursive call

    //these turns probably shouldn't transmit anything??
    Serial.println("backtracking");
    if (dir == calling_dir) {
      turn_around();
    }
    else if (dir == calling_dir + 1 || (dir == 0 && calling_dir == 3)) {
      turn_right();
    }
    else if (dir == calling_dir - 1 || (dir == 3 && calling_dir == 0)) {
      turn_left();
    }
    Serial.print("Next position is:");
    Serial.print(pos);
    Serial.println();
    line_follow_until_intersection();
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
