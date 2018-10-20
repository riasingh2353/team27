#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft

#include <FFT.h> // include the library

void setup() {
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  ADCSRA &= ~(bit (ADPS0) | bit (ADPS1) | bit (ADPS2)); // clear prescaler bits 
  ADCSRA |= bit (ADPS2);         // set ADC prescalar to be eight times faster than default
  Serial.begin(115200); // use the serial port
}

int l = 0;
boolean start = 0;



void loop() {
  while(1) {
    cli();
    for (int i = 0 ; i < 512 ; i += 2) {
      if(start == 0){
        fft_input[i] = analogRead(A1); // <-- NOTE THIS LINE
      }else{
        fft_input[i] = analogRead(A2);
      }
      fft_input[i+1] = 0;
    }
    fft_window();
    fft_reorder();
    fft_run();
    fft_mag_log();
    sei();
      
      if (start == 0) { 
        if (fft_log_out[3] > 70){
          l = l + 1;
          digitalWrite(2, LOW);
        } else {
          l = 0;
          digitalWrite(2, LOW);
        }
        if (l >= 10) {
          start = 1;
          digitalWrite(2, HIGH);  //flip select bit
          digitalWrite(3, HIGH);  //turn on indicator LED
          Serial.println("660 HURTS !!!!!");
        }
      }
      
      if (start == 1) {
        digitalWrite(2, HIGH);
        //Serial.println("Scanning:");
        Serial.println(fft_log_out[25]);
        Serial.println(fft_log_out[26]);
        Serial.println(fft_log_out[27]);
        if (fft_log_out[26] > 50 || fft_log_out[25] > 50 || fft_log_out[27] > 50){
          Serial.println("6KHz !!!!!");
          digitalWrite(4, HIGH); //turn on indicator LED
        }
        else {
          //digitalWrite(4, LOW); //turn off indicator LED
        }
      }
  }
}
