
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
 * Successfully avoiding other robots while ignoring decoys

## Testing the Radios:

We began ensuring the functionality of the RF24 transceivers by running the provided Getting Started sketch.  This sketch ensures that the transceivers are able to pass a timestamp to each other and print it to the Serial Monitor. These sample printouts are shown below.

SHOW SCREENSHOTS OF SERIAL PRINTOUTS FOR GETTING STARTED SKETCH

## Encoding the Maze and Updating the GUI:

We created the below 4X3 maze to demonstrate our robot's new capabilities:

SHOW DIAGRAM OF MAZE/PHOTO OF MAZE

Since the maximum payload for the transceivers is 32 bytes, we decided that a 3 byte encoding scheme would be efficient without restricting the base station's ability to interpret and process accurate information. We decided to package our information payload in a byte array, since the first byte is related specifically to wall information, while the last two bytes will be related to treasure information. A diagram of the encoding scheme of the first byte is shown below:

SHOW DIAGRAM OF ENCODING SCHEME


## ADD STUFF ABOUT TRANSMIT AND RECEIVE CODE

A video of the maze simulation and GUI updating is shown below:

SHOW VIDEO OF GUI UPDATING

## Robot Detection

The second part of this lab is to integrate the microphone circuit in order to detect the starting 660 Hz tone that was built in Lab 2. This circuit was implemented using a multiplexer to switch between the optical and audio circuits on order to save on analog pins beng used. The select bit starts low until the tone is heard, and then the select bit is switched high for the rest of the time to accomplish robot detection. The modifications to the FFT detection code are shown below and the code to start on the 660 Hz tone is shown below:

SHOW FFT CODE 

A video of the robot starting on the 660 Hz tone is shown below:

SHOW VIDEO OF ROBOT STARTING ON 660 HZ TONE

Moreover, in order to encode the maze information using the encoding scheme detailed above, information is recorded by reading the wall sensors and updating the byte array using the built-in Arduino functions relating to the byte type. The transmitting code from the robot is shown below:

SHOW CODE FOR TRANSMITTING INFO FROM ROBOT TO BASE STATION

In order to improve upon wall detection from Milestone 2, we decided to add a third wall sensor to the left side of the robot. Not only would this improve turning and decision making at intersections, but it reduces the number of turns needed in a square in order to map all sides. We also used a multiplexer to switch between the analog outputs of the three wall sensors, accounting for the fact that this switching between select bits occurs on the scale of nanoseconds, and shouldn't slow down our processing speed too much. This modified code is shown below:

SHOW WALL SENSOR CODE

A video of our robot navigating the test maze is shown below. The robot successfully stops upon detecting another robot and ignores decoys:

SHOW VID

A video of the robot wirelessly communicating the maze information to the base station and updating the GUI is shown below: 

SHOW VID


