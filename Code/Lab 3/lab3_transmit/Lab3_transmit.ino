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
}

void loop() {
  Serial.print("COUNT: ");
  Serial.println(count);
  for (int i = 0; i < 3; i++) {
    Serial.println(i);
    memcpy(transmit, wall_encodings[count], 3);
    for (int i = 0; i<3; i++){
      Serial.print(transmit[i],BIN);
      Serial.print("    ");
      Serial.println(wall_encodings[count][i],BIN);
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

  /* for (int i = 0; i < 5; i++) {
     byte zero[3] = {0, 0, 0};
     bool ok = radio.write( &zero, sizeof(zero) );
     if (ok){
       printf("ok, sending. \n");
       for(int i = 0; i<3; i++){
         Serial.println(zero[i]);
       }
     }
     else{
       printf("failed.\n\r");
       for(int i = 0; i<3; i++){
         Serial.println(zero[i]);
       }
     }
     delay(1000); //give time for other end to receive
    }//send zeros to indicate moving on
  */
  // Now, continue listening
  radio.startListening();
  // Try again 1s later
  //delay(1000);
}

void copy(byte* src, byte* dst, int len) {
  memcpy(dst, src, sizeof(src[0])*len);
}
