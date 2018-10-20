#include <nRF24L01.h>
#include <RF24_config.h>
#include <RF24.h>
#include <stdio.h>
#include <stdlib.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(9,10);
//pipe addresses
const uint64_t pipes[2] = { 0x0000000048LL, 0x0000000049LL };
byte zero[3] = {0, 0, 0};
byte data_before [3];
byte data[3];
int data_array[24];
int count_x = -1;
int count_y = 0;
bool first = true;

// Intialize strings to print
String x = "0";
String y = "1";
String west;
String east;
String north;
String south;
String robot;


void setup() {
  Serial.begin(9600);
  printf_begin();
  //printf("\n\rBase Station Receive Code\n\r");
  //printf("*** PRESS 'T' to begin transmitting to the other node\n\r");
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
    radio.openWritingPipe(pipes[1]);
    radio.openReadingPipe(1,pipes[0]);    //
  // Start listening
  //
    radio.startListening();
  //
  // Dump the configuration of the rf unit for debugging
  //
  // radio.printDetails();
}
void loop() {
  // put your main code here, to run repeatedly:
    if ( radio.available() )
      {
        // Dump the payloads until we've gotten everything
        bool done = false;
        while (!done)
        {
          // Fetch the payload, and see if this was the last one.
          done=radio.read(&data,sizeof(data));
          if (check_data()){
             decipher();
          }
          copy(data, data_before, 3);
          delay(20);
        }
      }
}

void decipher(){
  //Serial.println("Deciphering");
  for (int i = 0; i<3; i++){
    for (int j = 0; j < 8; j++){
      int x = 8*i + j;
      data_array[x] = int(bitRead(data_before[i], 7-j));
      //Serial.println(x);
     // Serial.print(data_array[x]);
     }
   }

   count_x++;

   if (count_x > 2) {
    count_y++;
    count_x = 0;
   }
   if (count_y > 3) {
    count_y = 0;
   }
   //Serial.println("");
   if (data_array[1]==1) {robot = "true";}
   else {robot = "false";}  
     
   if (data_array[4]==1) {north = "true";}
   else {north = "false";}
   
   if (data_array[5]==1) {east = "true";}
   else {east = "false";}
   
   if (data_array[6]==1) {south = "true";}  
   else {south = "false";}
   
   if (data_array[7]==1) {west = "true";}
   else {west = "false";}

  x = String(count_x);
  y = String(count_y);

   Serial.println(x+","+y+","+"north="+north+","+"east="+east+","+"south="+south+","+"west="+west+","+"robot="+robot);
 
}

void copy(byte* src, byte* dst, int len) {
    memcpy(dst, src, sizeof(src[0])*len);
}

// True if all zeros
bool check_data(){
  if (first) {
    //Serial.println("First instruction");
    first=false;
    return true;
  }
  
  //Serial.println("checking data");
  //Serial.println(data[0], BIN);
  // Serial.println(data_before[0], BIN);
  int last = int(bitRead(data_before[0],7));
 // Serial.println(last);
  int now = int(bitRead(data[0],7));
  //Serial.println(now);

  if (last != now) {
    //  Serial.println("different parity bits");
      return true;
  }
  else{
    //Serial.println("same parity bits");
    return false; 
  }
 }
/*
// True if all zeros
bool zero_data_before(){
 // Serial.println("Checking data_before is zero");
  byte b = 0;
  for (int i=0; i<3; i++) {
    if (!(data_before[i] == b)){
   //   Serial.println("not zero");
    return false;
    }
  }
  //Serial.println("zero");
  return true; 
 }
*/
