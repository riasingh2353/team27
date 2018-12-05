#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>
#include <stdio.h>
#include <stdlib.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
//#include "printf.h"

RF24 radio(9, 10);
//pipe addresses
const uint64_t pipes[2] = { 0x0000000048LL, 0x0000000049LL };
byte zero[4] = {0, 0, 0, 0};
byte data_before [4];
byte data;
byte full_data[4];
int data_array[32];
int x = 0;
int y = 0;
bool first = true;
int facing;

int width;
int height;

// Intialize strings to print
String xstring;
String ystring;
String west;
String east;
String north;
String south;
String robot;
String tshape;
String tcolor;


void setup() {
  //BE SURE TO CHANGE THESE DEPENDING ON MAZE SIZE!
  width = 9;
  height = 9;
  Serial.begin(9600);
  //printf_begin();
  radio.begin();
  radio.setRetries(15, 15);
  radio.setAutoAck(true);
  radio.setChannel(0x50);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);

  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1, pipes[0]);

  radio.startListening();
  //Serial.println(radio.getPayloadSize());
}
void loop() {
  // put your main code here, to run repeatedly:
  bool avail = radio.available();
  if (avail) {
    radio.startListening();
    for (int k = 0; k < 4; k++) {
      bool done = false;
      while(!done) {
        full_data[k] = 0;
        done = radio.read(&data, 1);
        full_data[k] = data;
        
        Serial.println();
        Serial.print("full_data[");
        Serial.print(k);
        Serial.print("] is: ");
        Serial.print(data);
        delay(200);
      }
      Serial.println("");
      delay(20);    
      Serial.println("done:");
      Serial.println(done);
     // radio.startListening();
    }
  //  radio.stopListening(); 

  }
  //Serial.println("END OF LOOP");
}
void decipher() {
  // Parse byte array into 1D int array
  
  for (int i = 0; i < 1; i++) {
    for (int j = 0; j < 8; j++) {
      int k = 8 * i + j;
      data_array[k] = int(bitRead(full_data[i], j));
      Serial.print(data_array[k]);
    }
  }
  Serial.println("");
  // Other Robot Information
  //i don't think we actually transmit this info
  for (int k = 0; k<8;k++) {
    Serial.print(data_array[k]);
  }
  Serial.println();
  for (int k = 8; k<16;k++) {
    Serial.print(data_array[k]);
  }
  Serial.println();
  for (int k = 16; k<24;k++) {
    Serial.print(data_array[k]);
  }
  Serial.println();
 // for (int k = 24; k<32;k++) {
  //  Serial.print(data_array[k]);
 // }
  Serial.println();
  if (data_array[1] == 1) {
    robot = "true";
  }
  else {
    robot = "false";
  }

  // Wall information
  if (data_array[4] == 1) {
    north = "true";
  }
  else {
    north = "false";
  }

  if (data_array[5] == 1) {
    east = "true";
  }
  else {
    east = "false";
  }

  if (data_array[6] == 1) {
    south = "true";
  }
  else {
    south = "false";
  }

  if (data_array[7] == 1) {
    west = "true";
  }
  else {
    west = "false";
  }
  bool treasure_exists = !(!data_array[8] && !data_array[9] && 
    !data_array[12] && !data_array[13] && !data_array[16] && !data_array[17]
    && !data_array[20] && !data_array[21]);
  int wall = 5;
  if (treasure_exists) {
    for (int k = 0; k < 4; k++) {
      if(data_array[8 + 4*k] || data_array[9 + 4*k] || 
      data_array[10 + 4*k] || data_array[11 + 4*k]) {
          wall = k; // wall stores the wall at which the treasure is placed.
        }
    }
    //determine shape info
    if (data_array[11 + 4*wall]) {
      if (data_array[10 + 4*wall]) {
        tshape = "triangle";
      }
      else tshape = "square";
    }
    else {
      tshape = "circle";
    }

    //determine color info
    if (data_array[9 + 4*wall]) {
      if (data_array[8 + 4*wall]) {
        tcolor = "blue";
      }
      else tcolor = "green";
    }
    else {
      tcolor = "red";
    }
  }
  else {
    tshape = "none";
    tcolor = "none";
  }
  // Start at 0 and mark first false
  if (first) {
    x = 0;
    y = 0;
    first = false;
  }
  byte pos = 0;
  for (int k = 0; k < 8; k++) {
    bitWrite(pos, k, data_array[24+k]);
    //Serial.print(data_array[24+k]);
  }
  //Serial.println(pos);
  xstring = pos % width;
  ystring = pos/width;

  //updated this line to print treasure info.
  Serial.println();
  Serial.println(ystring + "," + xstring + "," + "north=" + north + "," + "east=" + east + "," + "south=" + south + "," + "west=" + west + "," + "robot=" + robot + "," + "tshape=" + tshape + "," + "tcolor=" + tcolor);

  if (data_array[2] == 0) {
    if (data_array[3] == 0) {
      facing = 0;
    }
    else {
      facing = 1;
    }
  }
  else {
    if (data_array[3] == 0) {
      facing = 2;
    }
    else {
      facing = 3;
    }
  }

  switch (facing) {
    case 0: y--; break;
    case 1: x++; break;
    case 2: y++; break;
    case 3: x--; break;
  }
}


void copy(byte* src, byte* dst, int len) {
  memcpy(dst, src, sizeof(src[0])*len);
}

bool check_data() {
  if (first) {
    return true;
  }

  int last = int(bitRead(data_before[0], 7));
  int now = int(bitRead(full_data[0], 7));

  if (last != now) {
    return true;
  }
  else {
    return false;
  }
}

bool check_zeros () {
  // Serial.println("checking if all zeros");
  byte zero = 0;
  for (int i = 0; i < 3; i++) {
    if (full_data[i] != zero) {
      //     Serial.println("not zero");
      return false;
    }
  }
  // Serial.println("all zero");
  return true;
}
