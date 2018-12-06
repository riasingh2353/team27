# Final Robot Design

[Home](./index.md)

## Introduction:
Over the course of the past semester, we were tasked with constructing an Intelligent Physical System (IPS) that could autonomously and successfully navigate an arbitrary maze while recording information about its surroundings and reporting this information to an external base station.
<br>
To qualify as an IPS, our robot had to (1) perceive its environment, (2) reason about the information it collected, and (3) act upon its environment. Our system fulfilled these requirements in the following ways:
<br>
1. Using an array of microphone, IR, wall, and line sensors, our robot was able to take in information about its environment.
2. With the gathered information, the robot uses algorithms like the fast Fourier transform (FFT) and depth-first search (DFS) to detect obstacles and accurately traverse its way through the maze.
3. By reasoning about this information, our robot was able determine how to progress through the maze and map additional squares.

Generally, our system aimed to accomplish the following tasks:

* Detect and start on a 660 Hz tone
* Detect a 6 KHz IR transmission from another robot and avoid it
* Detect and avoid walls
* Follow and traverse the gridlines of the maze
* Record and transmit information about walls and positionality to a base station


## Physical Design:
We reconstructed our FFT-amplifier circuit, adding additional pin headers to make testing and verification of functionality easier. The extra pin headers also allowed us to replace broken components easily. Our FFT-amplifier circuitry contains our IR and microphone circuits, as both require an FFT. As the microphone is only used at the beginning of the competition, we added a multiplexer to reduce the number of analog inputs we used on our Arduino, which deselects the microphone circuit and selects the IR circuit after the 660 Hz tone initiates operation. The layouts for both these circuits is shown below:
<br>
![Mic Circuit](./media/final_design/mic.png =250x) ![IR Circuit](./media/final_design/phototransistor_schem.png =250x) 
<br>
The output of each of these circuits was fed into an LM358 op-amp in an inverting configuration (as shown below). A resistive voltage divider was used to bias the non-inverting terminal of each such op-amp to 2.5V (half of the supply voltage provided by the Arduino) in order to preserve the signal’s shape.
<br>
![Inverting Amp](./media/final_design/inverting_op_amp.png)
<br>
The above circuits were wired to one of the analog inputs on our Arduino. Below is the entire pinout of our Arduino:
<br>
| Pin | Use |
|:---:|:---:|
| A0 | Left line sensor |
| A1 | Right line sensor |
| A2 | Center line sensor |
| A3 | Front wall sensor |
| A4 | Left/right wall sensor |
| A5 | Audio/IR output |
| 0, 1, 2 | Unused |
| 3 | Left servo control signal |
| 4 | Serial communication with camera |
| 5 | Right servo control signal |
| 6 | Wall sensor mux select bit |
| 7 | FFT mux select bit |
| 8 | Serial communication with camera |
| 9-13 | Nordic Radio |
| SDA, SCL | I2C protocol for camera |

<br>
As shown in the table above, we also have a multiplexed input for our left and right wall sensors. This circuit, in addition to the audio/IR circuit and a breadboard is located on our robot’s bottom plate. On its top plate are three wall sensors and an IR transmission hat.
<br>
![Top](./media/final_design/top_plate.png)
<br>
On the bottom of our robot, we attached servos and line sensors. Additionally, we have our two battery packs - one to power the arduino and one to power the servos - velcroed to the bottom plate, so that they are secure and easy to swap out between runs. In the back, we added a mounted ball bearing that is designed to roll on the ground while our robot moves.
<br>
![Inside](./media/final_design/inside_diagram.png)
<br>
Had we elected to use our camera and FPGA, we would have attached the camera to the front of the robot next to our front wall sensor. Our breadboard would have had enough space to wire all of the camera connections. The FPGA would have been attached to the top plate of our robot in the back. We chose not to include these two components in our final design however, since we were concerned about the reliability of our camera connection and image quality for determining treasure shape and color. Since we did not want to lose points for incorrectly identified treasures, we strategically chose to not integrate the treasure detection aspect.
<br>
![Outside](./media/final_design/outside_diagram.png)
<br>
Our final design is shown below:
![Final](./media/final_design/final_design.png)

