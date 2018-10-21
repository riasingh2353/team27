#include <Servo.h>

#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft
#include <FFT.h> // include the library

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

Servo servoL;                     //instantiate servos
Servo servoR;
unsigned int sensor_values[3];   //store sensor values
                                 //sensor_values[0] -> left sensor; sensor_values[1] -> right sensor;
                                 //sensor_values[2] -> rear sensor.
int          front_wall_value;   //store front wall sensor value
int          right_wall_value;   //store right wall sensor value
int          left_wall_value;    //store left wall sensor value

int line_threshold = 300;        //cutoff value b/w white and not white
int wall_threshold = 150;        //A wall exists if a wall sensor reads a value greater than this threshold

int countdown = 3000;
bool wall_before = false;
bool start = 0;                  //0 if 660Hz has not been detected. 1 o/w
int dir = 1;                     //direction the robot is traveling in.
                                 //0 -> N; 1 -> E; 2 -> S; 3 -> W;
                                 //We assume the robot starts in the northwest corner
                                 // traveling east by default
        
int l = 0;                       //counter used in fft_detect()

RF24 radio(9, 10);
//pipe addresses
const uint64_t pipes[2] = { 0x0000000048LL, 0x0000000049LL };
byte transmit[3] = {10, 20, 30};//wall_encodings[x][3];

int count = 0;
typedef enum { role_ping_out = 1, role_pong_back } role_e;
role_e role = role_ping_out;

void setup() {
  Serial.begin(57600);
  servoL.attach(3);
  servoR.attach(5);

  //Digital Output Pins
  pinMode(6, OUTPUT); //Wall sensor mux select bit
                      //LOW reads right sensor, HIGH reads left sensor
  pinMode(7, OUTPUT); //FFT mux select bit
                      //LOW reads microphone, HIGH reads IR circuit
    digitalWrite(7, LOW);

  ADCSRA &= ~(bit (ADPS0) | bit (ADPS1) | bit (ADPS2)); // clear prescaler bits
  ADCSRA |= bit (ADPS2); // set ADC prescalar to be eight times faster than default

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
  // optionally, reduce the payload size.  seems to
  // improve reliability
  //radio.setPayloadSize(8);

  // Open pipes to other nodes for communication

  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1, pipes[1]);

  // Dump the configuration of the rf unit for debugging
  radio.printDetails();
}


void loop() {

  // Read line sensor values continuously
  get_line_values();

  while (start == 0) {
    fft_detect();
  }

  if (sensor_values[0] < line_threshold && sensor_values[1] < line_threshold ) { //INTERSECTION
    Serial.println("Intersection!");
    intersection();
    radio_transmit_sim();
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
      get_wall_values();
      while (countdown > 0) {
        servoL.write(88);
        servoR.write(80);
        countdown = countdown - 1;
      }
    
      while (sensor_values[2] > line_threshold && sensor_values[0] < line_threshold && sensor_values[1] < line_threshold ) {
        servoL.write(88);
        servoR.write(80);
        get_line_values();
      }
      countdown = 3000;
      update_direction(dir, 1);
      fft_detect();
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
        get_line_values();
      }
      countdown = 3000;
      update_direction(dir, 0);
      fft_detect();
      check_wall();
      
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
    
    //Called at the end of turn_left() and turn_right() to determine if the robot
    //is going to run into a wall.
    void check_wall() {
      delay(25);
      get_wall_values();
      if (right_wall_value >= 150) { // *|
        //digitalWrite(5, HIGH);
        if (front_wall_value > 150) { //-|
          //digitalWrite(4, HIGH);
          move_back();
          turn_left();
        }
        else { // |
          //digitalWrite(4, LOW);
          fft_detect(); //check if need to turn left
          drive_straight();
        }
      }
    
      if (right_wall_value < 150) { //no right wall
        //digitalWrite(5, LOW);
        if (wall_before) {
          move_back();
          turn_right();
        }
        else if (front_wall_value > 150) { // front wall
          //digitalWrite(4, HIGH);
          move_back();
          turn_left();
        }
        else {
          fft_detect();
          drive_straight();
        }             //no front wall, check if need to turn right
        //digitalWrite(4, LOW);
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
          get_line_values();
        }
      }
    }


  ///////////
  //GETTERS//
  ///////////
    
    //obtains front_wall_value, right_wall_value and left_wall_value
    //also writes to wall detection indicator LEDs
    void get_wall_values() {
      front_wall_value  = analogRead(A3);
      digitalWrite(6, LOW);//set wall sensor select bit to read right wall sensor
      right_wall_value  = analogRead(A4);
      digitalWrite(6, HIGH);//set wall sensor select bit to read left wall sensor
      left_wall_value   = analogRead(A4);
      
      /*
      //set wall sensor indicator LEDs
      if(front_wall_value > wall_threshold) digitalWrite(4, HIGH);
      else digitalWrite(4, LOW);
      if(right_wall_value > wall_threshold) digitalWrite(7, HIGH);
      else digitalWrite(7, LOW);
      if(left_wall_value > wall_threshold) digitalWrite(8, HIGH);
      else digitalWrite(8, LOW);
      */
    }
    
    //obtains left, right, and rear wall sensor values
    void get_line_values() {
      sensor_values[0] = analogRead(A0); //left line sensor
      sensor_values[1] = analogRead(A1); //right line sensor
      sensor_values[2] = analogRead(A2); //rear line sensor
    }

  /////////////////////////////////////////
  //MAZE TRAVERSAL LOGIC HELPER FUNCTIONS//
  /////////////////////////////////////////
    
    //call at the end of turn_left() and turn_right() turn to update global dir (direction) variable.
    //takes as input the direction the robot is facing (pass global dir var),
    //and the direction the robot is turning (0 -> Right turn, 1-> left turn)
    void update_direction(int facing, int turn_dir) {
      switch (facing) {
        case 0: //ROBOT IS FACING NORTH
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
    
    //Called when the robot reaches an intersection. reads wall sensor values and determines
    //whether to turn or drive straight. Also sets wall_before to appropriate values
    void intersection() {
      get_wall_values();
      
      //determine whether to turn
      if(front_wall_value > wall_threshold) {
        if (right_wall_value > wall_threshold && left_wall_value > wall_threshold) {
          //NEED A TURN 180• function
          turn_right();
          turn_right();
          wall_before = true;
        }
        else if (right_wall_value > wall_threshold) {//WALLS ON FRONT AND RIGHT
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
      else if(right_wall_value < wall_threshold && wall_before) {//SPECIAL CASE
        turn_right();
        wall_before = false;
      }
      else {//default case -- occurs when there is no wall, or wall only on right/left side
        drive_straight();
        if(right_wall_value > wall_threshold) {
          wall_before = true;
        }
        else {
          wall_before = false;
        }
      }
    }

  ////////////////////////////////////////
  //RADIO TRANSMISSION HELPER FUNCTIONS://
  ////////////////////////////////////////

    //packs information about a given intersection (presence of walls,
    //treasures at walls, direction, etc) into a 24-bit array
    //then sends this array to the base station
    // refer to github for encoding of this array
    //NOTE: get_wall_values() must be called beforehand

    void radio_transmit_sim() {
      // First, stop listening so we can talk.
      byte info[3] = {0, 0, 0};   //stores maze info.
      info[0] = pack_bit_one(dir);
      radio.stopListening();
      // NOTE: the maze array is defined here
      // Send the maze in a single payload
      printf(count);
      if (role == role_ping_out) {
        // First, stop listening so we can talk.
        radio.stopListening();

        // NOTE: the maze array is defined here

        // Send the maze in a single payload
        printf("Sending\n");
        bool ok = radio.write( info, sizeof(info) );

        if (ok) {
          printf("ok, sending. \n");
          for (int i = 0; i < 3; i++) {
          //  Serial.println(transmit[i],BIN);
          }
          printf(count);
        }
        else {
          printf("failed.\n\r");
          for (int i = 0; i < 3; i++) {
          //  Serial.println(transmit[i]);
          }
        }
        delay(1000); //give time for other end to receive
      }
    }

    void copy(byte* src, byte* dst, int len) {
      memcpy(dst, src, sizeof(src[0])*len);
    }

    //helper for send_intersection_info()
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
        case 0: //ROBOT IS FACING NORTH
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


  ////////
  //MISC//
  ////////
    
    
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
        if (!start) {
          if (fft_log_out[3] > 70){
            l = l + 1;
          }
          else {
            l = 0;
          }
         if (l >= 10) {
           start = 1;
           drive_straight();
           digitalWrite(7, HIGH);  //flip select bit
           //Serial.println("660 HURTS !!!!!");
          }
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
