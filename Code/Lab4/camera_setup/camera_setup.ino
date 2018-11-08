#include <Wire.h>

#define OV7670_I2C_ADDRESS 0x21 

// Define register addresses
#define COM3 0x0C        // ENABLE SCALING
#define COM7 0x12        // RESET REGISTERS, SET OUTPUT FORMAT
#define COM9 0x14        // SET AUTOMATIC GAIN CEILING
#define COM15 0xC0       // SET RGB OPTION
#define COM17 0x0C       // ENABLE COLOR BAR TEST
#define SCALING_XSC 0x70 // 8-BAR COLOR
#define SCALING_YSC 0x71 // 8-BAR COLOR
#define MVFP 0x1E        // FLIP/MIRROR IMAGE
#define CLKRC 0x11       // ENABLE DOUBLE CLOCK OPTION

/*0c
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
//
//  pinMode(4, INPUT); //input from FPGA
//  pinMode(8, OUTPUT); //output to FPGA saying it wants image info
//
//  digitalWrite(8, LOW);
//    
//  delay(100);

  // Write to bit 7 to reset registers
  OV7670_write_register(COM7, 0x80); 
//
//  delay(100); 
//  
  set_color_matrix(); 
  
  //Write to bit 3 to enable scaling
  OV7670_write_register(COM3, 0x08); 
  
  //Write to bit 2 for color bar selection and enable color bar
   OV7670_write_register(COM7, 0x0E);
   
  //Enable external clock
  OV7670_write_register(CLKRC, 0xC0);
  
  // Enable color bar test
  OV7670_write_register(COM17, 0x0C);
  
  // Mirror/flip image
  OV7670_write_register(MVFP, 0x30);

  // x scaling
 // y scaling

  read_key_registers();
//  read_color_registers();

 // get_FPGA_data();
 // get_FPGA_data();
}

void loop(){
  
 }


///////// Function Definition //////////////
void read_key_registers(){
  byte regval;
  
  Serial.println("Value at COM3:");
  regval = read_register_value(COM3);
  Serial.println(regval,HEX);
  
  Serial.println("Value at COM7:");
  regval = read_register_value(COM7);
  Serial.println(regval,HEX);

  //READ COM14
  Serial.println("Value at COM15:");
  regval = read_register_value(COM15);
  Serial.println(regval,HEX);
  
  //READ COM17
  Serial.println("Value at COM17:");
  regval = read_register_value(COM17);
  Serial.println(regval,HEX);

  //READ CLKRC
  Serial.println("Value at CLKRC:");
  regval = read_register_value(CLKRC);
  Serial.println(regval,HEX);
  
  //READ MVFP
  Serial.println("Value at MVFP:");
  regval = read_register_value(MVFP);
  Serial.println(regval,HEX);


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
    OV7670_write_register(0x4f, 0x80); //1
    OV7670_write_register(0x50, 0x80); //2
    OV7670_write_register(0x51, 0x00);
    OV7670_write_register(0x52, 0x22);
    OV7670_write_register(0x53, 0x5e); //5
    OV7670_write_register(0x54, 0x80);
    OV7670_write_register(0x56, 0x40);
    OV7670_write_register(0x58, 0x9e); //8
    OV7670_write_register(0x59, 0x88);
    OV7670_write_register(0x5a, 0x88);
    OV7670_write_register(0x5b, 0x44);
    OV7670_write_register(0x5c, 0x67); //12
    OV7670_write_register(0x5d, 0x49);
    OV7670_write_register(0x5e, 0x0e);
    OV7670_write_register(0x69, 0x00);
    OV7670_write_register(0x6a, 0x40);
    OV7670_write_register(0x6b, 0x0a);
    OV7670_write_register(0x6c, 0x0a);
    OV7670_write_register(0x6d, 0x55);
    OV7670_write_register(0x6e, 0x11); //20
    OV7670_write_register(0x6f, 0x9f);
    OV7670_write_register(0xb0, 0x84); 
}

//info about treasure shape and color sent to arduino
//this information is transmitted as four bits in the form shown below:
// |X|X|X|X|shp|shp|col|col|
//where shp is a 2 bit value corresponding with shape info
//and col is a 2 bit value corresponding with color info
byte get_FPGA_data(){
  byte treasure = 0b00000000;
  digitalWrite(8, HIGH);
  delay(5); //wait for FPGA to compute treasure data
  bitWrite(treasure, 3, digitalRead(4)); //store MSB (shape bit 1)
  digitalWrite(8, LOW);
  digitalWrite(8, HIGH);
  bitWrite(treasure, 2, digitalRead(4)); //store shape bit 2
  digitalWrite(8, LOW);
  digitalWrite(8, HIGH);
  bitWrite(treasure, 1, digitalRead(4)); //store color bit 1
  digitalWrite(8, LOW);
  digitalWrite(8, HIGH);
  bitWrite(treasure, 0, digitalRead(4)); //store color bit 2
  digitalWrite(8, LOW);
  Serial.println(treasure);
  decode_treasure_info(treasure);
  return treasure;
}

void decode_treasure_info(byte t){
  if (bitRead(t, 3)){
    if (bitRead(t, 2)){
      Serial.println("triangle");
    }
    else Serial.println("square");
  }
  else {
    if (bitRead(t, 2)){
      Serial.println("circle");
    }
    else Serial.println("None");
  }

  if (bitRead(t, 1)){
    if (bitRead(t, 0)){
      Serial.println("blue");
    }
    else Serial.println("green");
  }
  else {
    if (bitRead(t, 0)){
      Serial.println("red");
    }
    else Serial.println("None");
  } 
 }

void read_color_registers() {
  //READ COM7
  Serial.println("Value at Register 1:");
  byte regval = read_register_value(0x4f);
  for (int k = 7; k >= 0; k--) {
    Serial.print(bitRead(regval, k));
  }
  Serial.println();
  //READ COM14
  Serial.println("Value at Register 2:");
  regval = read_register_value(0x50);
  for (int k = 7; k >= 0; k--) {
    Serial.print(bitRead(regval, k));
  }
  Serial.println();
  //READ COM17
  Serial.println("Register 5:");
  regval = read_register_value(0x53);
  for (int k = 7; k >= 0; k--) {
    Serial.print(bitRead(regval, k));
  }
  Serial.println();

  //READ CLKRC
  Serial.println("Value at 8:");
  regval = read_register_value(0x58);
  for (int k = 7; k >= 0; k--) {
    Serial.print(bitRead(regval, k));
  }
  Serial.println();
  //READ MVFP
  Serial.println("Register 12:");
  regval = read_register_value(0x5c);
  for (int k = 7; k >= 0; k--) {
    Serial.print(bitRead(regval, k));
  }
  Serial.println();
  //READ GFIX
  Serial.println("Value at 20:");
  regval = read_register_value(0x6e);
  for (int k = 7; k >= 0; k--) {
    Serial.print(bitRead(regval, k));
  }
  Serial.println();
}



