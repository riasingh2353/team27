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

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

int data_array[32];
unsigned long data;

int x = 0;
int y = 0;
bool first = true;
int facing;

int width;
int height;

String xstring;
String ystring;
String west;
String east;
String north;
String south;
String robot;
String tshape;
String tcolor;

//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10

RF24 radio(9,10);

//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0x0000000048LL, 0x0000000049LL };

//
// Role management
//
// Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.
//

// The various roles supported by this sketch
typedef enum { role_ping_out = 1, role_pong_back } role_e;

// The debug-friendly names of those roles
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};

// The role of the current running sketch
role_e role = role_pong_back;

unsigned long time;
unsigned long vals[4] = {1000, 9000, 2222, 192903};
int count = 0;
void setup(void){
  //
  // Print preamble
  //

  Serial.begin(9600);
 // printf_begin();


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

  //radio.printDetails();
}

void loop() {
  role = role_pong_back;
  
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
        done = radio.read( &data, sizeof(data) );
        decipher();
        
        // Spew it

        // Delay just a little bit to let the other unit
        // make the transition to receiver
        delay(20);

      }

      // First, stop listening so we can talk
      radio.stopListening();

      // Send the final one back.
      radio.write( &got_time, sizeof(unsigned long) );


      // Now, resume listening so we catch the next packets.
      radio.startListening();
    }
  }

  //
  // Change roles
  //

  if ( Serial.available() )
  {
    char c = toupper(Serial.read());
    if ( c == 'T' && role == role_pong_back )
    {
      printf("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK\n\r");

      // Become the primary transmitter (ping out)
      role = role_ping_out;
      radio.openWritingPipe(pipes[0]);
      radio.openReadingPipe(1,pipes[1]);
    }
    else if ( c == 'R' && role == role_ping_out )
    {
      printf("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK\n\r");

      // Become the primary receiver (pong back)
      role = role_pong_back;
      radio.openWritingPipe(pipes[1]);
      radio.openReadingPipe(1,pipes[0]);
    }
  }
}


void decipher() {
  // Parse byte array into 1D int array
  height = 3;
  width = 2
  
  ;
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 8; j++) {
      int k = 8 * i + j;
      data_array[k] = int(bitRead(data,k));
    //  Serial.print(data_array[k]);
    }
  }
  //Serial.println("");
  // Other Robot Information
  //i don't think we actually transmit this info
  for (int k = 0; k<8;k++) {
    //Serial.print(data_array[7-k]);
  }
  /*Serial.println();
  for (int k = 0; k<8;k++) {
    Serial.print(data_array[15-k]);
  }
  Serial.println();
  for (int k = 0; k<8;k++) {
    Serial.print(data_array[23-k]);
  }
  Serial.println();
  for (int k =0 ; k<8;k++) {
    Serial.print(data_array[31-k]);
  }
  Serial.println();
*/
  // Wall information
  if (data_array[3] == 1) {
    north = "true";
  }
  else {
    north = "false";
  }

  if (data_array[2] == 1) {
    east = "true";
  }
  else {
    east = "false";
  }

  if (data_array[1] == 1) {
    south = "true";
  }
  else {
    south = "false";
  }

  if (data_array[0] == 1) {
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
  //Serial.println("Position is: ");
  //Serial.println(pos);
  xstring = pos % width;
  ystring = pos/width;

  //updated this line to print treasure info.
  //Serial.println();
  Serial.println(ystring + "," + xstring + "," + "north=" + north + "," + "east=" + east + "," + "south=" + south + "," + "west=" + west + "," + "robot=" + robot + "," + "tshape=" + tshape + "," + "tcolor=" + tcolor);
//Serial.println("2" + "," + "3" + "," + "north=" + north + "," + "east=" + east + "," + "south=" + south + "," + "west=" + west)
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
