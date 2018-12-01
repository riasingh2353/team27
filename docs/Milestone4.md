# Milestone 4

[Home](./index.md)

## Objective
The purpose of milestone was to complete implementation of computer vision on our robot, and thus allow it to detect treasures and identify their shapes and colors, as well as determine whether or not treasures are present. 

### Materials
No additional materials were needed for this milestone

### Shape Detection
Previously, we had already implemented color detection with our camera. This milestone adds shape detection to functionality, allowing us to distinguish between triangles, diamonds, and square. As our image processor reads pixels in rows from top to bottom, we implemented a simple algorithm that would count the number of red or blue pixels in a row every couple of rows, and compare with a previous row from higher up in the frame. If the shape is a triangle, each next row read would have more counted pixels than the previous row read. If the shape is a diamond, at a certain point a read row will have less counted pixels than a previous row. If the shape is a square, the amount of pixels counted will remain relatively constant. To prevent comparison to white space where the treasure image is not present, we set a threshold for the number of pixels that must be counted in a row before it is allowed for comparison. 
~~~c
		if (VGA_PIXEL_X == 10'b0  && rowCount > 15'd50 && VGA_PIXEL_Y < 144*2/3) begin //new row
			count = count - 3'b001;
			if (count == 3'b0) begin
				if (lastRowCount+16'd7 < rowCount) begin
					TEMP_SHAPE = 2'b11; // triangle
				end
				else if (lastRowCount-16'd15 > rowCount) begin
					TEMP_SHAPE = 2'b01; // diamond
				end
				else begin
					TEMP_SHAPE = 2'b10; // square
				end
				count = 3'b101;
			lastRowCount = rowCount;
			end
			rowCount  = 16'd0;
~~~


The video below demonstrates our shape detection capability. The last 3 LEDs on the right are assigned to shapes as follows:

| Shape            | LED Color    |
|------------------|--------------|
| Square           | Red          |
| Triangle         | Green        |
| Diamond          | Yellow       |

The first 2 LEDs of the 5 total are assigned to colors as follows:

| Color        | LED Color    |
|--------------|--------------|
| Red          | Red          |
| Blue         | Green        |


<iframe width="560" height="315" src="https://www.youtube.com/embed/F9ZXnC7BIEg" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

We found that, on average, our shape detection works well but is highly dependent on the quality of the image, which varies at different times that we test our code. On the Arduino side, we will likely take the shape value that occurs the most often and take it as the value to reduce this effect.

In Lab 4, we demonstrated the functionality of the communication between the Arduino and the FPGA, and we extended this to communicate treasure shape information as well as color. A video of this is shown below:


<iframe width="560" height="315" src="https://www.youtube.com/embed/RRkVq5P6AAY" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

The values fluctuate slightly before a steady input is shown, but we believe that the issue will be solved by a stationary treasure and a fixed camera.
