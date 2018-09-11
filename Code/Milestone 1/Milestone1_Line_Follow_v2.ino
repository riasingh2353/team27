#include <Servo.h>
#include <QTRSensors.h>


Servo servoL;
Servo servoR;
QTRSensorsAnalog qtr((unsigned char[]){14,15},2);
int lastError = 0;
int Lspeed = 100;
int Rspeed = 80;
int adj = 0;
unsigned int sensor_values[2];
int countdown = 12;

void setup() {
// put your setup code here, to run once:
Serial.begin(9600);
servoL.attach(3);
servoR.attach(9);

}

void loop() {

// put your main code here, to run repeatedly:
  if (countdown > 0) {
    //servoL.write(180);
    //servoR.write(0);
    delay(20);  //Generic calibration delay
    servoL.write(90);
    servoR.write(90);
    delay(20);
    qtr.calibrate();
    delay(20); //60ms * 84 ~ 5s
    Serial.println("CALIBRATING !!!!!");
    countdown = countdown - 1;
  }

  else {

      qtr.read(sensor_values);
      
      if (sensor_values[0] > 900 && sensor_values[1] > 900) { 
        servoL.write(90);
        servoR.write(90);
        Serial.println("Intersection Detected"); //For now the robot just stops, eventually will need FSM thing for fig8 turn tracking
      }
      else if (sensor_values[1] > 900) { //if robot veers left
        //veer left 
        if (Lspeed >10){ //Might change threshold
          Lspeed = Lspeed-10; //speed up left wheel
        }
        else{ //Necessary to make sure we don't keep speeding up one wheel when its at max (and having nothing happen)
          Rspeed = Rspeed-10; //slow down right wheel
        }
        servoL.write(Lspeed);
        servoR.write(Rspeed);
        Serial.println("VEERING LEFT !!!!!");
      }
      else if (sensor_values[0] > 900) { //if robot veers right
        //veer right
        if (Rspeed <170){ //Might change threshold
          Rspeed = Rspeed+10; //speed up right wheel
        }
        else{
          Lspeed = Lspeed+10; //slow down left wheel
        }
        servoL.write(Lspeed);
        servoR.write(Rspeed);
        Serial.println("VEERING RIGHT !!!!!");
      }
      else{
        servoL.write(Lspeed);
        servoR.write(Rspeed);
        Serial.println("Driving Straight");
      }
      //delay(1);
      Serial.println("LEFT:");
      Serial.println(sensor_values[0]);
      Serial.println("RIGHT:");
      Serial.println(sensor_values[1]);
  }
}
