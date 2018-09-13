# Milestone 1
[Home](./index.md)

The purpose of this milestone was to design a motion control system for our robot.

The tasks we set out to complete included:
  * Getting our robot to successfully follow a line
  * Getting our robot to successfully traverse a grid in a figure eight.
  
Additionally, we focused on: 
  * Developing a robust control system, and
  * Getting our robot to complete these tasks at a reasonably high speed
    
The only materials used for this milestone that weren't used in lab one were 3 QTR-1A reflectance sensors ([datasheet here](https://www.pololu.com/product/958)). 

These sensors have three pins, VIN, GND, and OUT. Each sensor outputs a voltage at its OUT pin that corresponds with the lightness or darkness of the surface below it. 

The OUT pin of each sensor is tied to a separate analog input of the Arduino Uno controlling our robot. Each of these analog inputs is capable of reading out values between 0 and 1023 corresponding to the lightness or darkness of the area under it.
## Line Following

Our robot is equipped with two line sensors in its front, designed to be far enough apart to straddle a line of electrical tape. Our line-following code is written under the assumption that our robot will follow a grid of white tape on a black background. so that the robot will drive forward with both wheels at the same speed if it detects that both line sensors 

![Front of Robot](/media/FRONT\ OF\ ROBOT.jpg)

![Potentiometer Setup](/media/PotentiometerSetup.png)  

## Figure Eight
