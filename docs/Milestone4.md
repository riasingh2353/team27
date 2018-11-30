# Milestone 4

[Home](./index.md)

## Objective
The purpose of milestone was to complete implementation of computer vision on our robot, and thus allow it to detect treasures and identify their shapes and colors, as well as determine whether or not treasures are present. 

### Materials
No additional materials were needed for this milestone

### Shape Detection
Previously, we had already implemented color detection with our camera. This milestone adds shape detection to functionality, allowing us to distinguish between triangles, diamonds, and square. 


The video below demonstrates our shape detection capability. The LEDs are assigned to shapes as follows:

| Shape            | Color    |
|------------------|----------|
| Square           | Red      |
| Triangle         | Green    |
| Diamond          | Yellow   |

<iframe width="560" height="315" src="https://www.youtube.com/embed/F9ZXnC7BIEg" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

We found that, on average, our shape detection works well but is highly dependent on the quality of the image, which varies at different times that we test our code. On the Arduino side, we will likely take the shape value that occurs the most often and take it as the value to reduce this effect.
