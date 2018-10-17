# Milestone 2
[Home](./index.md)

## Goals

The purpose of this milestone was to enhance our robot's existing motion control system by applying our new hardware additions from Lab 2.  Specifically, the tasks we set out to complete included:
  * Getting our robot to successfully circle an arbitrary set of walls, specifically through right-hand wall following
  * Showing our robot successfully avoiding other robots (i.e. the IR-emitter 'hat' that is installed on each robot)
  
Additionally, we focused on: 
  * Optimizing our initial control algorithms to increase the robustness of the system
  * General map-traversal speed of our robot
    
## Materials    

In addition to the materials listed for Lab 2, we also implemented the Sharp GP2Y0A41SK0F IR sensor into our overall design, in order to accomplish wall detection and, ultimately, wall following.

These sensors have three pins, VCC, GND, and VO (respectively Supply Voltage, Ground, and Output Voltage). Each sensor outputs a voltage at its VO pin that corresponds with the reflectiveness of the sensor's target.  Using this, our robot is able to measure the distance between itself and surrounding walls.

## Sensor Implementation and Wall Detection

We used an IR sensor for our robot’s front wall detection, with output voltage vs. distance characteristic as shown below.

![Distance vs. Voltage Chart](./media/milestone2/sensorcharacteristics.png)

In addition to the front wall detection, we added another Sharp GP2Y0A41SK0F IR sensor on the right side of our robot to accomplish right hand wall following. 

After experimenting with different values, we decided to determine that a wall has been detected by setting a threshold reading of 100 (corresponding to 100*.0049 = .49 Volts) for the sensor: if the sensor output reads greater than 100, a wall has been detected. 

The logic for wall following is as follows: 

1) If there is nothing detected by the front or right wall sensors, the robot reverts to its default line following state.

2) If a wall is detected on both the front and right side of the robot, this means the robot has reached a corner on its right side and it should turn left.

3) If a wall is detected only in the front, the robot will turn right to accomplish right-hand wall following. 

4) If a wall is detected on the right side of the robot, the robot will drive straight along the side of this wall. 

Special case: If the robot was right-hand wall following and reaches an intersection where it no longer detects a wall, it is likely that the robot has reached a corner in the maze and should continue wall following, so the robot will turn right. This case is demonstrated in the image below:

![Corner Case](./media/milestone2/cornercase.png/)




## Final Integration

The next part of milestone 2 required that wall detection be integrated with IR hat detection, such that the robot is able to react to the presence of other robots, which will be signified by the 6 kHz signal emitting from their IR hats. In order to demonstrate this functionality in our robot, we chose to make the robot stop driving for a set amount of time in order to let the other robot pass. We may improve upon this functionality at a later point as we discuss strategy more.

In order to demonstrate what the robot is "thinking"
