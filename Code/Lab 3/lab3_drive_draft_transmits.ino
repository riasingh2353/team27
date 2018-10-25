#include <Servo.h>

#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft
#include <FFT.h> // include the library

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(9, 10);
//pipe addresses
const uint64_t pipes[2] = { 0x0000000048LL, 0x0000000049LL };
byte wall_encodings[12][3] = {{0b00011011, 0, 0},
  {0b10011010, 0, 0}, {0b00101100, 0, 0}, {0b10100101, 0, 0}, {0b00100101, 0, 0}, {0b10110110, 0, 0}, {0b00111010, 0, 0},
  {0b10000011, 0, 0}, {0b00000101, 0, 0}, {0b10011001, 0, 0}, {0b00101100, 0, 0}, {0b10000111, 0, 0}
} ;
byte transmit[3] = {10, 20, 30};//wall_encodings[x][3];

int count = 0;
typedef enum { role_ping_out = 1, role_pong_back } role_e;
role_e role = role_ping_out;

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
//0 -> N; 1 -> E; 2 -> S; 3 -> W;
//We assume the robot starts in the northwest corner
// traveling east by default


void setup() {
  //
  // Print preamble
  //
  ADCSRA &= ~(bit (ADPS0) | bit (ADPS1) | bit (ADPS2)); // clear prescaler bits
  ADCSRA |= bit (ADPS2); // set ADC prescalar to be eight times faster than default
  Serial.begin(57600);
  printf_begin();
  printf("\n\rRF24/examples/GettingStarted/\n\r");
  printf("*** PRESS 'T' to begin transmitting to the other node\n\r");

  //
  // Setup and configure rf radio
  //

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

  //
  // Open pipes to other nodes for communication
  //

  // This simple sketch opens two pipes for these two nodes to communicate
  // back and forth.
  // Open 'our' pipe for writing
  // Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)

  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1, pipes[1]);

  //
  // Start listening
  //

  //radio.startListening();

  //
  // Dump the configuration of the rf unit for debugging
  //

  radio.printDetails();

  //unsigned long transmit;

  servoL.attach(3);
  servoR.attach(5);

  //Digital Output Pins
  pinMode(6, OUTPUT); //Wall sensor mux select bit
  //  //LOW reads right sensor, HIGH reads left sensor
  pinMode(7, OUTPUT); //FFT mux select bit
  //  //LOW reads microphone, HIGH reads IR circuit
  digitalWrite(7, LOW);

  servoL.write(90);
  servoR.write(90);
}

void loop() {
  //
  //  if (start == 0) {
  //    Serial.println("waiting");
  //    fft_detect();
  //  }
  // Read line sensor values continuously
  get_line_values();
  /*while (!start) {
    servoL.write(90);
    servoR.write(90);
    fft_detect();
    Serial.println(start);
    }*/

  if (sensor_values[0] < line_threshold && sensor_values[1] < line_threshold ) { //INTERSECTION
    get_line_values();
    if (sensor_values[0] < line_threshold && sensor_values[1] < line_threshold ) {
      Serial.println("Intersection!");
      intersection();
      Serial.print("COUNT: ");
      Serial.println(count);
      for (int i = 0; i < 3; i++) {
        Serial.println(i);
        memcpy(transmit, wall_encodings[count], 3);
        for (int i = 0; i < 3; i++) {
          Serial.print(transmit[i], BIN);
          Serial.print("    ");
          Serial.println(wall_encodings[count][i], BIN);
        }
        radio_transmit_sim(count);
      }

      if (count == 11) {
        count = 0;
      }
      else {
        count = count + 1;
      }
      printf(count);
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


  //  servoL.write(90);
  //  servoR.write(90);
  //  get_line_values();
  //  get_wall_values();
  //  intersection();
  //  Serial.print("COUNT: ");
  //  Serial.println(count);
  //  for (int i = 0; i < 3; i++) {
  //    Serial.println(i);
  //    memcpy(transmit, wall_encodings[count], 3);
  //    for (int i = 0; i<3; i++){
  //      Serial.print(transmit[i],BIN);
  //      Serial.print("    ");
  //      Serial.println(wall_encodings[count][i],BIN);
  //    }
  //    radio_transmit_sim(count);
  //  }
  //
  //  if (count == 11) {
  //    count = 0;
  //  }
  //  else {
  //    count = count + 1;
  //  }
  //  printf(count);

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
}

void turn_right() {
  get_wall_values();

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

}

void turn_around() {
  get_wall_values();

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
  front_wall_value  = analogRead(A3);
  digitalWrite(6, LOW);//set wall sensor select bit to read right wall sensor
  right_wall_value  = analogRead(A4);
  digitalWrite(6, HIGH);//set wall sensor select bit to read left wall sensor
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

void radio_transmit_sim(int x) {

  printf(count);
  if (role == role_ping_out) {
    // First, stop listening so we can talk.
    radio.stopListening();

    // NOTE: the maze array is defined here

    // Send the maze in a single payload
    printf("Sending\n");
    bool ok = radio.write( transmit, sizeof(transmit) );

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

  // Now, continue listening
  radio.startListening();
  // Try again 1s later
  //delay(1000);
}

///////////////////////////////////////////
////MAZE TRAVERSAL LOGIC HELPER FUNCTIONS//
///////////////////////////////////////////

//Called when the robot reaches an intersection. reads wall sensor values and determines
//whether to turn or drive straight. Also sets wall_before to appropriate values
void intersection() {
  get_wall_values();
  fft_detect();
  //determine whether to turn
  if (front_wall_value > wall_threshold) {
    if (right_wall_value > wall_threshold && left_wall_value > wall_threshold) {
      //NEED A TURN 180• function
      turn_around();
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
  else if (right_wall_value < wall_threshold && wall_before) { //SPECIAL CASE
    turn_right();
    wall_before = false;
  }
  else {//default case -- occurs when there is no wall, or wall only on right/left side
    drive_straight();
    if (right_wall_value > wall_threshold) {
      wall_before = true;
    }
    else {
      wall_before = false;
    }
  }
}

//////////
////MISC//
//////////

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
    if (fft_log_out[3] > 65) {
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
