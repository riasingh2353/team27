#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(9,10);
//pipe addresses
const uint64_t pipes[2] = { 0x0000000048LL, 0x0000000049LL };
byte wall_encodings[12][3] = {{0b00011011,0,0}, 
  {0b00011011,0,0}, {0b00101100,0,0},{0b00100101,0,0}, {0b00100101,0,0}, {0b00110110,0,0}, {0b00111010,0,0},
  {0b00000011,0,0}, {0b00000101,0,0}, {0b00011001,0,0}, {0b00101100,0,0}, {0b00000111,0,0}} ;
int count = 0;
void setup() {
    //
  // Print preamble
  //
  
  Serial.begin(57600);
  printf_begin();
  printf("\n\rRF24/examples/GettingStarted/\n\r");
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

    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);

  //
  // Start listening
  //

  //radio.startListening();

  //
  // Dump the configuration of the rf unit for debugging
  //

  radio.printDetails();

unsigned long transmit;
}

void loop() {
  radio_transmit_sim(count);
  count = count+1;
  if (count == 11) {
    count = 0;
  }
}

void radio_transmit_sim(int x){
    byte transmit = wall_encodings[x][3];
    // if there is data ready
/*    if ( radio.available() )
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

      }*/

      // First, stop listening so we can talk
      radio.stopListening();

      // Send the final one back.
      for (int i = 0; i<3; i++){
        radio.write( &transmit, sizeof(transmit) );
        printf("Sent response.\n\r");
        delay(20); //give time for other end to receive
      }
      for (int i = 0; i<5; i++){
        byte zero[3] = {0,0,0};
        radio.write( &zero, sizeof(zero) );
        printf("Sent response.\n\r");
        delay(20); //give time for other end to receive
      }//send zeros to indicate moving on 

      // Now, resume listening so we catch the next packets.
     // radio.startListening();
   // }
}
