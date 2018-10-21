
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

## Encoding the Maze and Updating the GUI:

The maze we chose to simulate for the first part of this lab is a 4x3 maze shown below:

SHOW DIAGRAM OF MAZE

Since the maximum payload for the transceivers is 32 bytes, we decided to choose a 3 byte encoding scheme that is efficient but not too restrictive such that the base station would have to infer a lot of information. A diagram of the encoding scheme is shown below:

SHOW DIAGRAM OF ENCODING SCHEME

The data structure which we chose to encode our maze information is a 3 byte array.


A video of the maze simulation and GUI updating is shown below:

SHOW VIDEO OF GUI UPDATING

## Robot Detection

The second part of this lab is to integrate the microphone circuit in order to detect the starting 660 Hz tone that was built in Lab 2. This circuit was implemented using a multiplexer to switch between the optical and audio circuits on order to save on analog pins beng used. The select bit starts low until the tone is heard, and then the select bit is switched high for the rest of the time to accomplish robot detection. The modifications to the FFT detection code are shown below and the code to start on the 660 Hz tone is shown below:

SHOW FFT CODE 

A video of the robot starting on the 660 Hz tone is shown below:

SHOW VIDEO OF ROBOT STARTING ON 660 HZ TONE

In order to improve upon wall detection from Milestone 2, we decided to add a third wall sensor to the left side of the robot. Not only would this improve turning and decision making at intersections, but it reduces the number of turns needed in a square in order to map all sides. We also used a multiplexer to switch between the analog outputs of the three wall sensors, accounting for the fact that this switching between select bits occurs on the scale of nanoseconds, and shouldn't slow down our processing speed too much. This modified code is shown below:

SHOW WALL SENSOR CODE

A video of our robot navigating the test maze is shown below. The robot successfully stops upon detecting another robot and ignores decoys:

SHOW VID

A video of the robot wirelessly communicating the maze information to the base station and updating the GUI is shown below: 

SHOW VID


