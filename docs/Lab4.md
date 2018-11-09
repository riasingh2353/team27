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
The FPGA team began with the provided _Lab4_FPGA_Template.zip_


## Final Integration:
