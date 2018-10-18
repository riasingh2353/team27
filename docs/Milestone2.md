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

These sensors have three pins; VCC, GND, and VO (respectively Supply Voltage, Ground, and Output Voltage). Each sensor outputs a voltage at its VO pin that corresponds with the reflectiveness of the sensor's target.  Using this, our robot is able to measure the distance between itself and surrounding walls.

## Sensor Implementation and Wall Detection

We used an IR sensor for our robot’s front wall detection, with output voltage vs. distance characteristic as shown below.

![Distance vs. Voltage Chart](./media/milestone2/sensorcharacteristics.png)

In addition to the front wall detection, we added another Sharp GP2Y0A41SK0F IR sensor on the right side of our robot to accomplish right hand wall following (2 short range IR sensors, one on the front and one on the right of the robot). 

After experimenting with different values, we decided to determine that a right wall has been detected by setting a threshold reading of 150 (corresponding to 150*.0049 = .735 Volts) for the sensor: if the sensor output reads greater than 150, a wall has been detected.

The logic for wall following is as follows: 

1) If there is nothing detected by the front or right wall sensors, the robot reverts to its default line following state.

2) If a wall is detected on both the front and right side of the robot, this means the robot has reached a corner on its right side and it should turn left.

3) If a wall is detected only in the front, the robot will turn right to accomplish right-hand wall following. 

4) If a wall is detected on the right side of the robot, the robot will drive straight along the side of this wall. 

Special case: If the robot was right-hand wall following and reaches an intersection where it no longer detects a wall, it is likely that the robot has reached a corner in the maze and should continue wall following, so the robot will turn right. This case is demonstrated in the image below:

![Corner Case](./media/milestone2/cornercase.png/)

We extended our line following code to include wall detection and right hand wall following, and chose to save processing time by only checking the wall sensors at intersections. If our line sensors read that the robot is at an intersection, the wall detection code will take precedence over the line following code. The cases outlined above are shown in code below:

~~~c

if (right_wall_value >= 150) { 
      if (front_wall_value > 150) { 
        turn_left();
      }
      else { 
        drive_straight();
      }
      wall_before = true;
    }

    if (right_wall_value < 150) { //no right wall
      if (wall_before) {
        turn_right();
      }
      else if (front_wall_value > 150) { // front wall
        turn_left();
      }
      else {  //no front wall, check if need to turn right
        drive_straight();
      }
      wall_before = false;
    }
   
~~~

The code will first check the right wall sensor such that right wall following takes precedence, and a right wall is indicated by a sensor reading below 150 based on experimental values. If both a front wall and right wall is detected, the robot will turn left, otherwise it will drive straight. If no right wall is detected, we must take into account the special case from above in which the robot was previously right hand following a wall but reaches an intersection and needs to turn to continue following that wall. This is accomplished in the code by using the boolean wall_before indicator, and setting it true when the robot was wall-following before the intersection. Otherwise, if only a front wall is detected, the robot will simply turn right, and if nothing is detected, the robot will drive straight.


## Avoiding Other Robots

In order to detect other robots, we integrated our IR detection circuit from Lab 2 into our line following/wall detection system. We used one phototransistor, which will check if another robot is in front of our robot at intersections. If another robot is detected in front, then our robot will stop driving and wait for a set time until the robot has passed. Since our turning functions check for robots at the end of each turn, this process of checking for robots and turning will continue until no robot is detected, in which case the robot will drive straight. We decided to implement a single phototransistor circuit and will place blinders on either side of the phototransistor such that it sits at the vertex of a 45 degree angle. This will ensure that our robot is only capable of detecting robots in front of it.

The code used to implement this robot avoidance is shown below in the function fft_detect, called during intersections as well:

~~~c
void fft_detect( int x ) {
  cli();

  for (int i = 0 ; i < 512 ; i += 2) {
    fft_input[i] = analogRead(A5); // 
    fft_input[i + 1] = 0;
  }

  fft_window();
  fft_reorder();
  fft_run();
  fft_mag_log();
  sei();

// if 6kHz signal is detected

  if (fft_log_out[26] > 60 || fft_log_out[25] > 60 || fft_log_out[27] > 60) {
    stop_drive();
    delay(2500);
  }
  else {
    drive_straight();
  }
}
~~~

## Final System

In order to demonstrate what our robot is "thinking," we added three indicator LEDs on top of our robot according to the following system:

- Green: Front wall detected
- Yellow: Right wall detected
- Red: Other robot deteced

A demonstration of the complete wall detection/robot avoidance system is shown in the following videos (two separate runs):

<iframe width="560" height="315" src="https://www.youtube.com/embed/O-Vg5KvUeTA" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>


<iframe width="560" height="315" src="https://www.youtube.com/embed/6OwpiuwGSlo" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>


