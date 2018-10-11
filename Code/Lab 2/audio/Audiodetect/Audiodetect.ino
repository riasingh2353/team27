  //DETECT 660Hz SIGNAL
  //HOW TO SET SAMPLING FREQUENCY??
  //from: https://arduino.stackexchange.com/questions/24868/what-is-the-sampling-rate-of-the-analogread
  //analogRead() should be able to run every 110.4 uS (DOUBLE CHECK W OSCILLOSCOPE)
  //time: 110.4 uS relevant freuency: 9.06 kHz
  
  #define LOG_OUT 1 // use the log output function
  #define FFT_N 256 // set to 256 point fft
   
  #include <FFT.h> // include the library

void setup() {
  Serial.begin(9600);
}

int j = 0;
void loop() {
  for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
    fft_input[i] = analogRead(A3); //
    fft_input[i + 1] = 0;
  }
  fft_window();
  fft_reorder();
  fft_run();
  fft_mag_log();

  for (byte i = 0; i < FFT_N; i++) {
    Serial.println(fft_log_out[i]);
  }
  j++;
  if (j == 3) delay (1000000000);
}
