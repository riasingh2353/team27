
# Lab 3
[Home](./index.md)

Materials:  
  * Arduino Uno (2)
  * Nordic nRF24L01+ transceiver (2)
  * 2 Breakout Boards w/ Headers
  * Electret microphone
  * Phototransistor
  
The purpose of this lab is to integrate the individual components completed in the previous labs and milestones. The objectives of this milestone are as follows:

 * Have the robot navigate a small maze and map the maze by wirelessly communicating to a base station
 * Integrate the microphone circuit from Lab 2 to start the robot on a 660 Hz tone
 * Successfully avoiding other robots and ignoring decoys


## Testing the Radios:

The first task of this lab was to ensure the functionality of the RF24 transceivers- this was done by running the provided Getting Started sketch to check that the transceivers would be able to communicate a timestamp between each other and print it to the Serial. The Serial printouts demonstrating the functionality of these transceivers are shown below:

SHOW SCREENSHOTS OF SERIAL PRINTOUTS FOR GETTING STARTED SKETCH

## Encoding the Maze:

The maze we chose to simulate for the first part of this lab is a 4x3 maze shown below:

SHOW DIAGRAM OF MAZE

Since the maximum payload for the transceivers is 32 bytes, we decided to choose a 3 byte encoding scheme that is efficient but not too restrictive such that the base station would have to infer a lot of information. A diagram of the encoding scheme is shown below:

SHOW DIAGRAM OF ENCODING SCHEME

The data structure which we chose to encode our maze information is a 3 byte array. 

## Robot Detection

The second part of this lab is to integrate the microphone circuit in order to detect the starting 660 Hz tone that was built in Lab 2. This circuit was implemented using a multiplexer to switch between the 
