# Final Robot Design

[Home](./index.md)

##Introduction:
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
