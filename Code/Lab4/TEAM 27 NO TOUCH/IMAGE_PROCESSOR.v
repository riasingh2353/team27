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

output reg [1:0] RESULT;

reg REDCOUNT =16'd0;
reg BLUECOUNT = 16'd0;
reg lastSYNC = 1'b0;

always @ (posedge CLK) begin 
	
	if (VGA_VSYNC_NEG == 1 && lastSYNC == 0) begin
		RESULT = 2'b11;
		//if (BLUECOUNT >REDCOUNT && BLUECOUNT>16'd10) RESULT = 2'b10;
		//if (REDCOUNT > BLUECOUNT && REDCOUNT > 16'd10) RESULT = 2'b01;
		//else RESULT = 2'b00;
		//BLUECOUNT = 0;
		//REDCOUNT = 0;
	end
	else begin 
		if (PIXEL_IN[7:6] > PIXEL_IN[1:0] && PIXEL_IN[7:6] > PIXEL_IN[4:3]) REDCOUNT = REDCOUNT +1'b1;
		if (PIXEL_IN[7:6] < PIXEL_IN[1:0] && PIXEL_IN[1:0] > PIXEL_IN[4:3]) BLUECOUNT = BLUECOUNT +1'b1;
	end
	lastSYNC = VGA_VSYNC_NEG;
end

endmodule
