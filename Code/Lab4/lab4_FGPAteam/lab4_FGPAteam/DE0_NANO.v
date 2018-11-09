`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144

///////* DON'T CHANGE THIS PART *///////
module DE0_NANO(
	CLOCK_50,
	GPIO_0_D,
	GPIO_1_D,
	KEY
);

//=======================================================
//  PARAMETER declarations
//=======================================================
localparam RED = 8'b111_000_00;
localparam GREEN = 8'b000_111_00;
localparam BLUE = 8'b000_000_11;
localparam WHITE = 8'b111_111_11;


//=======================================================
//  PORT declarations
//=======================================================

//////////// CLOCK - DON'T NEED TO CHANGE THIS //////////
input 		          		CLOCK_50;

//////////// GPIO_0, GPIO_0 connect to GPIO Default //////////
output 		    [33:0]		GPIO_0_D;
//////////// GPIO_0, GPIO_1 connect to GPIO Default //////////
input 		    [33:20]		GPIO_1_D;
input 		     [1:0]		KEY;

///// PIXEL DATA /////
wire[8:0] RESULT;

reg [7:0]	pixel_data_RGB332 = 8'd0;

reg [1:0] blue;
reg [2:0] green; 
reg [2:0] red;

///// READ/WRITE ADDRESS /////
reg [14:0] X_ADDR = 15'b0;
reg [14:0] Y_ADDR = 15'b0;
wire [14:0] WRITE_ADDRESS;
reg [14:0] READ_ADDRESS; 

assign WRITE_ADDRESS = X_ADDR + Y_ADDR*(`SCREEN_WIDTH);

///// VGA INPUTS/OUTPUTS /////
wire 			VGA_RESET;
wire [7:0]	VGA_COLOR_IN;
wire [9:0]	VGA_PIXEL_X;
wire [9:0]	VGA_PIXEL_Y;
wire [7:0]	MEM_OUTPUT;
wire			VGA_VSYNC_NEG;
wire			VGA_HSYNC_NEG;
reg			VGA_READ_MEM_EN;

assign GPIO_0_D[5] = VGA_VSYNC_NEG;
assign VGA_RESET = ~KEY[0];

/* WRITE ENABLE */
reg W_EN;

///////* CREATE ANY LOCAL WIRES YOU NEED FOR YOUR PLL *///////
wire c0_sig;
wire c1_sig;
wire c2_sig;
//reg state = 0;

// INDICATE WHICH BYTE IS BEING READ IN DOWNSAMPLER
reg byte_num;

//ASSIGN CLOCK SIGNALS TO PINS
assign GPIO_0_D[0] = c0_sig;
assign GPIO_0_D[1] = c1_sig;
assign GPIO_0_D[2] = c2_sig;


//// ASSIGN CAMERA INPUTS TO PINS
wire PCLK, VSYNC, HREF;
wire D7, D6, D5, D4, D3, D2, D1, D0;

assign PCLK = GPIO_1_D[31];
assign VSYNC = GPIO_1_D[32];
assign HREF = GPIO_1_D[33];
assign D7 = GPIO_1_D[22];
assign D6 = GPIO_1_D[21];
assign D5 = GPIO_1_D[24];
assign D4 = GPIO_1_D[23];
assign D3 = GPIO_1_D[26];
assign D2 = GPIO_1_D[25];
assign D1 = GPIO_1_D[28];
assign D0 = GPIO_1_D[27];

reg lastHREF;
reg lastVSYNC;

///////* INSTANTIATE YOUR PLL HERE *///////
jankPLL	jankPLL_inst (
	.inclk0 ( CLOCK_50 ),
	.c0 ( c0_sig ),
	.c1 ( c1_sig ),
	.c2 ( c2_sig )
	);



///////* M9K Module *///////
Dual_Port_RAM_M9K mem(
	.input_data(pixel_data_RGB332),
	.w_addr(WRITE_ADDRESS),
	.r_addr(READ_ADDRESS),
	.w_en(W_EN),
	.clk_W(c2_sig),
	.clk_R(c1_sig),
	.output_data(MEM_OUTPUT)
);

///////* VGA Module *///////
VGA_DRIVER driver (
	.RESET(VGA_RESET),
	.CLOCK(c1_sig),
	.PIXEL_COLOR_IN(VGA_READ_MEM_EN ? MEM_OUTPUT : BLUE),
	.PIXEL_X(VGA_PIXEL_X),
	.PIXEL_Y(VGA_PIXEL_Y),
	.PIXEL_COLOR_OUT({GPIO_0_D[9],GPIO_0_D[11],GPIO_0_D[13],GPIO_0_D[15],GPIO_0_D[17],GPIO_0_D[19],GPIO_0_D[21],GPIO_0_D[23]}),
   .H_SYNC_NEG(GPIO_0_D[7]),
   .V_SYNC_NEG(VGA_VSYNC_NEG)
);

///////* Image Processor *///////
IMAGE_PROCESSOR proc(
	.PIXEL_IN(MEM_OUTPUT),
	.CLK(c1_sig),
	.VGA_PIXEL_X(VGA_PIXEL_X),
	.VGA_PIXEL_Y(VGA_PIXEL_Y),
	.VGA_VSYNC_NEG(VGA_VSYNC_NEG),
	.RESULT(RESULT)
);


///////* Update Read Address *///////
always @ (VGA_PIXEL_X, VGA_PIXEL_Y) begin
		READ_ADDRESS = (VGA_PIXEL_X + VGA_PIXEL_Y*`SCREEN_WIDTH);
		if(VGA_PIXEL_X>(`SCREEN_WIDTH-1) || VGA_PIXEL_Y>(`SCREEN_HEIGHT-1)) begin
				VGA_READ_MEM_EN = 1'b0;
		end
		else begin
				VGA_READ_MEM_EN = 1'b1;
		end
end



//////// *DOWNSAMPLER* ////////

always @ (posedge PCLK) begin
	if (HREF == 0 && lastHREF==1) begin //negedge HREF
		X_ADDR <= 0;
		Y_ADDR <= Y_ADDR + 1;
	end
	
	else if (VSYNC == 1 && lastVSYNC == 0) begin //posedge VSYNC
		X_ADDR <= 0;
		Y_ADDR <= 0; 
	end
	
	else begin 
		Y_ADDR <= Y_ADDR;
		if (HREF) begin
			if (~byte_num) begin
				pixel_data_RGB332[7:2] <= {D7,D6,D5,D2,D1,D0};
				pixel_data_RGB332[1:0] <= pixel_data_RGB332[1:0];
				byte_num <= W_EN;
				X_ADDR <= X_ADDR;	
				W_EN <= 0;      
			end
			
			else begin
				pixel_data_RGB332[7:2] <=pixel_data_RGB332[7:2];
				pixel_data_RGB332[1:0] <= {D4,D3}; 
				X_ADDR <= X_ADDR + 1;		
				byte_num <= W_EN;
				W_EN <= 1;	  
			end
			
		end else begin
			X_ADDR <= 0;
		end
		
	end

	lastHREF = HREF;
	lastVSYNC = VSYNC;

end


/*always @ (negedge HREF or posedge VSYNC) begin
	if (VSYNC) begin
		Y_ADDR <= 0;        // if new frame reset y address
	end
	else begin
		Y_ADDR <= Y_ADDR + 1;  // otherwise increment y address with each new row
	end
end
*/

////////* CREATE FLAG PATTERN IN MEMORY* //////
/*
always @ (posedge PCLK) begin
	pixel_data_RGB332[1:0] <= blue;
	pixel_data_RGB332[4:2] <= green;
	pixel_data_RGB332[7:5] <= red;
		W_EN <= 1;
		
		if (((X_ADDR + 10 > `SCREEN_WIDTH / 2) && (X_ADDR - 10 < `SCREEN_WIDTH / 2)) || 
		((Y_ADDR + 10 > `SCREEN_HEIGHT / 2) && (Y_ADDR - 10 < `SCREEN_HEIGHT / 2)) ) begin
			pixel_data_RGB332 <= RED;
		end
		else begin
			pixel_data_RGB332 <= WHITE;
		end
	
		if (X_ADDR == (`SCREEN_WIDTH-1)) begin
			X_ADDR <= 0;
			if (Y_ADDR == (`SCREEN_HEIGHT-1)) begin
				Y_ADDR <= 0;
			end 
			else begin
				Y_ADDR <= Y_ADDR + 1;
			end
		end 
		else begin
			X_ADDR <= X_ADDR + 1;
			Y_ADDR <= Y_ADDR;
		end
end
*/

	
endmodule 