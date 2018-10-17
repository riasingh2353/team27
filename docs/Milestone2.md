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

![Distance vs. Voltage Chart](./media/IRChart.png)

In addition to the front wall detection, we added another Sharp GP2Y0A41SK0F IR sensor on the right side of our robot to accomplish right hand wall following. 

## Subject to Change:

After experimenting with different values, we decided to determine that a wall has been detected by setting a threshold reading of 100 (corresponding to 100*.0049 = .49 Volts) for the sensor: if the sensor output reads greater than 100, a wall has been detected.  

![Front of Robot](./media/FRONTOFROBOT.jpg)

Our line following code is written with three cases in mind: 
* If the robot detects that neither of its sensors are on the line, it will drive straight
* If the robot detects that its left sensor is on the line, it will veer left to correct course
* If the robot detects that its right sensor is on the line, it will veer right to correct courses 

A snippet of this code is shown below
(Please note: Our line-following code is written under the assumption that our robot will follow a grid of white tape on a black background. The threshold sensor values (300 in this case) will change under different circumstances.)

```c
//Case: traveling along line --> drive straight
if (sensor_values[0] > 300 && sensor_values[1] > 300) { drive_straight();}

//Case: drifting off to the right --> correct left
else if (sensor_values[0] < 300) { veer_left(); }

//Case: drifting off to the left --> correct right
else if (sensor_values[1] < 300) { veer_right(); }
```

Some code from the helper functions ```drive_straight()```, and ```veer_left()``` is shown below. 


```drive_straight()``` sets both wheels to rotate in the forward direction at a moderate speed.
```c
void drive_straight(){
  servoL.write(95);
  servoR.write(85);     
}
```

```veer_left()``` increases the speed of the right wheelrelative to its speed set by ```drive_straight()```, and rotates the left wheel backward. ```veer_right()``` is defined nearly identically to ```veer_left()```.
```c
void veer_left(){
  servoL.write(80);
  servoR.write(55);
}
```

Check out our robot following this jagged line:

<iframe width="560" height="315" src="https://www.youtube.com/embed/HUTCxMMCKz0" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>

## Figure Eight
This part of the milestone builds upon our line-following implementation to recognize intersections and perform 90 degree turns, and perform them at the appropriate junctions. 

The intersection-detection was accomplished by simply noting when both the left front and right front sensors saw white. Under this condition, our code enters a series of switch/case statements to determine whether to turn left or right. Each turn is numbered, and depending on the intersection number, the robot will execute a specific case statement. After completing a turn, the number of interesections detected is incremented so it will advance to the next case statement on the next turn it sees. 

At each intersection, the robot makes the following turns:

    1) Left 
    2) Right
    3) Right
    4) Right
    5) Right
    6) Left
    7) Left
        
The switch/case statements in our setup and loop are shown below:
```c
//Case: reaches intersection
if (sensor_values[0] < 300 && sensor_values[1] < 300){
    Serial.println("Intersection!");
    switch(turn_num){
      case 0:turn_left(); break;
      case 1:turn_right(); break;
      case 2:turn_right(); break;
      case 3:turn_right(); break;
      case 4:turn_right(); break;
      case 5:turn_left(); break;
      case 6:turn_left(); break;
      case 7:turn_left(); break;
      default: stop_drive(); break;
    }

    turn_num += 1;    // increment number of turns

    if (turn_num == 8){ turn_num = 0;}   // reset number of turns after figure 8 completed
```

In order to complete a 90 degree turn, we use the third line sensor to tell the robot when to stop turning-- the idea is that the robot will stop turning once the back sensor sees white again. However, to deal with the fact that for a short time before the robot completely leaves the first white line, the sensor will still see white, a delay is put in by waiting until a change from black to white is detected. Then, the turn is executed until the change from black to white is detected again. 

This delay is implemented using a countdown variable that executes at the processing speed of the Arduino and is calibrated to the speed of the wheels that we chose. We can later optimize this countdown variable to increase the speed of our robot.

The sensor values are also read during each action-- turning and line following--to ensure that the robot knows when to stop.

We implemented these turns using helper functions, which are shown below:

```c
//HELPER FUNCTIONS

void turn_left(){     
  while (countdown > 0) {
    servoL.write(87);
    servoR.write(35);
    countdown = countdown - 1;
  }
    
   while (sensor_values[2] > 300){
       servoL.write(87);
       servoR.write(35);
       sensor_values[0] = analogRead(A0);
       sensor_values[1] = analogRead(A1);
       sensor_values[2] = analogRead(A2);//delay(500);
       Serial.println("Turning left!"); 
    } 
   countdown = 8000;
}

void turn_right(){
  while(countdown > 0){
         servoL.write(105);
        servoR.write(93);
        countdown = countdown -1;
  }
  while (sensor_values[2] > 300){
        servoL.write(105);
        servoR.write(93);
        //delay(500); 
            sensor_values[0] = analogRead(A0);
          sensor_values[1] = analogRead(A1);
         sensor_values[2] = analogRead(A2);
        Serial.println("Turning right!");
        Serial.println(sensor_values[2]);
    }

  countdown = 8000;
 }
}
```

Check out our robot trace a figure 8!!! (including some tough love to demonstrate course correction abilities)

<iframe width="560" height="315" src="https://www.youtube.com/embed/wixIh0njYuw" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>
