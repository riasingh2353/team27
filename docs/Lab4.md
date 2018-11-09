# Lab 4

[Home](./index.md)

## Objective:
The purpose of this lab is to begin the process of enabling our robot with computer vision, which will eventually allow it to locate treasures within an arbitrary maze based on sight.  For this week, our team broke off into two groups to tackle the logic and hardware implementation off the robot, so that we could have better control of, and ease of access to, our new system.  This system, at the time of writing, is composed of:

### Materials

* Our Arduino Uno
* Two (2) DE0-Nano - Altera Cyclone IV Field-Programmable Gate Arrays (FPGA)
* CMOS VGA OV7670 Digital Camera
* Pre-built VGA Adapter for FPGA
* Additional Components as Needed

## Subteams:

Team FPGA:  
Team Arduino:

## PLL:
With all the different components to drive for this lab, each of which requiring different clock speeds, we needed to setup Phase-Locked Loops on our FPGAs to create a set of 'virtual' clocks.  While our FPGA can generate a 50 MHz internal clock, we need a 24 MHz clock to drive the camera and a 25 MHz clock for the VGA module to drive the screen output for debugging.  Out onboard memory also needs clocks associated with reading and writing data.  So, we created a phase-locked loop (PLL) to generate three clock lines at 24, 25, and 50 MHz, all in phase with each other.  This was accomplished using the _Altera_ IP that is packaged with Quartus II, the environment in which we programmed the FPGA.

## Team Arduino:



## Team FPGA:
We began by opening the provided _Lab4_FPGA_Template.zip_ file and setting up the project in Quartus II, as well as initializing the PLL as described above.  After reading through the associated project files, we instantiated the PLL within the project's top-level module, _DE0_NANO.v_.

~~~c
jankPLL	jankPLL_inst (
	.inclk0 ( CLOCK_50 ),
	.c0 ( c0_sig ),
	.c1 ( c1_sig ),
	.c2 ( c2_sig )
	);
~~~

Above, c0, c1, and c2 are our 24, 25, and 50 MHz phase-locked clock signals, and c#\_sig are internal wires that can carry our three distinct frequencies.  Note that, while CLOCK_50 can also be used as a 50 MHz line, we use c2\_sig to drive 50 MHz signals instead of CLOCK_50 because our setup ensures that c2\_sig is phase-locked with the other signals.

We attached the VGA adapter to our FPGA at this time.  The pre-made adapter fit neatly over one of the FPGA's GPIO pinouts, occupying pins in the even-numbered sequence from 8 to 28 on GPIO-0, or the odd-numbered sequence of addresses from GPIO_05 to GPIO_023 plus the grounded pin #12 (see below).

![Adapter Pinout Diagram](./media/GPIO-0.PNG)

## Final Integration:
