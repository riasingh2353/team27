`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144
`define NUM_BARS 3
`define BAR_HEIGHT 48

module IMAGE_PROCESSOR (
	PIXEL_IN,
	CLK,
	VGA_PIXEL_X,
	VGA_PIXEL_Y,
	VGA_VSYNC_NEG,
	RESULT
);


//=======================================================
//  PORT declarations
//=======================================================
input	[7:0]	PIXEL_IN;
input 		CLK;

input [9:0] VGA_PIXEL_X;
input [9:0] VGA_PIXEL_Y;
input			VGA_VSYNC_NEG;

output [8:0] RESULT;

//wire [8:0] RESULT;
reg red;
reg blue;

localparam RED = 8'b111_000_00;
localparam BLUE = 8'b000_000_11;

// directly compare red values of pixel data to blue values of pixel data
assign RESULT = (PIXEL_IN[7:6] > PIXEL_IN[1:0]) ? {RED, 1'b0} : {BLUE, 1'b0};




endmodule
