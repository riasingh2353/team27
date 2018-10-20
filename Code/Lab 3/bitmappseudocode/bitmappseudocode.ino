void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
//BYTE 0:
  //bit 0:    iamhere -- 1 if our robot is in this position
  
  //bit 1:    other robot -- 1 if another robot is here
  
  //bit 2:    explored -- 1 if our robot has been to this square
  
  //bits 3-7: North wall
      //bit 3: wall exists: 1 if north wall exists, 0 o/w
      //bits 4,5: treasure color
      //bits 6,7: treasure shape
//BYTE 1:
  //bits 0-4: South wall
      //bit 8: wall exists: 1 if south wall exists, 0 o/w
      //bits 9,10: treasure color
      //bits 11,12: treasure shape
  
  //bits 5-7: East wall
      //bit 13: wall exists: 1 if east wall exists, 0 o/w
      //bits 14,15: treasure color

//BYTE 2:
  //bits 0,1: East wall(continues)
      //bits 16,17: treasure shape
  
  //bits 2-6: West wall
      //bit 18: wall exists: 1 if west wall exists, 0 o/w
      //bits 19,20: treasure color
      //bits 21,22: treasure shape
byte maze[81][3]; //81 entry array, with each entry containing 3 8-bit values
//initialize 
for (int i = 0; i<=80; i++) {
  maze[i][0] = 0;
  maze[i][1] = 0;
  maze[i][2] = 0;
}
