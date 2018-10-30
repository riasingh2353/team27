#include <Wire.h>

#define OV7670_I2C_ADDRESS 0x21 /*) */

//specify correct pixel format
//create a buffer???
/*
    Reset all registers
    Enable scaling
    Use external clock as internal clock
    Set the camera to output the resolution you specified in Q4 and the pixel format you specified in Q2.(Note: The pixel format will take setting an extra register to fully specify)
    Enable a color bar test (takes setting 2 regs, neither of them are the SCALING regs)
    Vertical and mirror flip the output image (optional: it’s not necessarily needed but could be useful)
    As we discussed in class, you may also want to fix, e.g. the gain (Automatic Gain Ceiling) and other parameters of the camera
*/

///////// Main Program //////////////
void setup() {
  Wire.begin();
  Serial.begin(9600);
  Serial.println("HELLO WORLD !");
  // TODO: READ KEY REGISTERS
  
  delay(100);
  
  // TODO: WRITE KEY REGISTERS
  //OV7670_write_register(int reg_address, byte data)

  //WRITE COM7
  OV7670_write_register(0x09, 0b10000110);
  //WRITE COM14
  
  //WRITE COM17
  OV7670_write_register(0x21, 0b00001000);
  //WRITE CLKRC
  OV7670_write_register(0x08, 0b01000000);
  //WRITE MVFP
  OV7670_write_register(0x0F, 0b00110000);
  //WRITE GFIX (WHAT GAIN TO USE???)
  read_key_registers();
}

void loop(){
 }


///////// Function Definition //////////////
void read_key_registers(){
  /*TODO: DEFINE THIS FUNCTION*/
  //READ COM7
  Serial.println("Value at COM7:");
  byte regval = read_register_value(0x09);
  for (int k = 7; k >= 0; k--) {
    Serial.print(bitRead(regval, k));
  }
  Serial.println();
  //READ COM14
  Serial.println("Value at COM14:");
  regval = read_register_value(0x1F);
  for (int k = 7; k >= 0; k--) {
    Serial.print(bitRead(regval, k));
  }
  Serial.println();
  //READ COM17
  Serial.println("Value at COM17:");
  regval = read_register_value(0x21);
  for (int k = 7; k >= 0; k--) {
    Serial.print(bitRead(regval, k));
  }
  Serial.println();

  //READ CLKRC
  Serial.println("Value at CLKRC:");
  regval = read_register_value(0x11);
  for (int k = 7; k >= 0; k--) {
    Serial.print(bitRead(regval, k));
  }
  Serial.println();
  //READ MVFP
  Serial.println("Value at MVFP:");
  regval = read_register_value(0x1E);
  for (int k = 7; k >= 0; k--) {
    Serial.print(bitRead(regval, k));
  }
  Serial.println();
  //READ GFIX
  Serial.println("Value at GFIX:");
  regval = read_register_value(0x34);
  for (int k = 7; k >= 0; k--) {
    Serial.print(bitRead(regval, k));
  }
  Serial.println();
}

byte read_register_value(int register_address){
  byte data = 0;
  Wire.beginTransmission(OV7670_I2C_ADDRESS);
  Wire.write(register_address);
  Wire.endTransmission();
  Wire.requestFrom(OV7670_I2C_ADDRESS,1);
  while(Wire.available()<1);
  data = Wire.read();
  return data;
}

String OV7670_write(int start, const byte *pData, int size){
    int n,error;
    Wire.beginTransmission(OV7670_I2C_ADDRESS);
    n = Wire.write(start);
    if(n != 1){
      return "I2C ERROR WRITING START ADDRESS";   
    }
    n = Wire.write(pData, size);
    if(n != size){
      return "I2C ERROR WRITING DATA";
    }
    error = Wire.endTransmission(true);
    if(error != 0){
      return String(error);
    }
    return "no errors :)";
 }

String OV7670_write_register(int reg_address, byte data){
  return OV7670_write(reg_address, &data, 1);
 }

void set_color_matrix(){
    OV7670_write_register(0x4f, 0x80);
    OV7670_write_register(0x50, 0x80);
    OV7670_write_register(0x51, 0x00);
    OV7670_write_register(0x52, 0x22);
    OV7670_write_register(0x53, 0x5e);
    OV7670_write_register(0x54, 0x80);
    OV7670_write_register(0x56, 0x40);
    OV7670_write_register(0x58, 0x9e);
    OV7670_write_register(0x59, 0x88);
    OV7670_write_register(0x5a, 0x88);
    OV7670_write_register(0x5b, 0x44);
    OV7670_write_register(0x5c, 0x67);
    OV7670_write_register(0x5d, 0x49);
    OV7670_write_register(0x5e, 0x0e);
    OV7670_write_register(0x69, 0x00);
    OV7670_write_register(0x6a, 0x40);
    OV7670_write_register(0x6b, 0x0a);
    OV7670_write_register(0x6c, 0x0a);
    OV7670_write_register(0x6d, 0x55);
    OV7670_write_register(0x6e, 0x11);
    OV7670_write_register(0x6f, 0x9f);
    OV7670_write_register(0xb0, 0x84);
}
