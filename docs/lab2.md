
# Lab 2
[Home](./index.md)

Acoustic Subteam: 

Optical Subteam: 

Materials:  
  * Arduino Uno
  * Electret Microphone
  * IR Transistor (OP598)
  * IR 'Hat'
  * IR Decoy
  * Various other components, as needed
  
The purpose of this lab is to implement new hardware that allows our robot to ‘see’ and interact with its surroundings to a greater extent.  These hardware additions include a microphone and an IR phototransistor and IR emitter ‘hat,’ which allows our robot to interpret audible and IR signal information and react accordingly.  When fully applied, our robot will have two additional functionalities that will assist us during our final project: the ability to start operation upon perceiving an audio signal at 660 Hz, and the ability to detect other robots outfitted with a similar IR emitter hat.

## Method - Acoustic:

We began this portion of the lab by retrieving the necessary materials and proceeding to power our electret microphone as instructed, detailed in the diagram below.

![Mic Circuit](./media/mic_circuit(1).png)  

This subteam’s goal was to get our robot to identify an audio signal at 660 Hz.  However, upon connecting the output of our circuit to an oscilloscope, we soon discovered that playing loud audio signals into this microphone results in an output that is too low in voltage to be perceived.  As such, we processed the output of this circuit through a simple inverting amplifier (topology shown below), realized using an LM358 op-amp. 

![OpAmp Topology](./media/OpAmpTopology.png)
The output of this amplifier is in the proper voltage range to be detected by our arduino:

### Vout = VR + ((RF/R1)\*(V2-V1))
