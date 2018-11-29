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
input 		    [40:0]		GPIO_1_D;
input 		     [1:0]		KEY;

///// PIXEL DATA /////
wire[1:0] RESULT; 
wire [1:0] SHAPE;

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

assign PCLK = GPIO_1_D[7];
assign VSYNC = GPIO_1_D[3];
assign HREF = GPIO_1_D[5];
assign D7 = GPIO_1_D[23];
assign D6 = GPIO_1_D[21];
assign D5 = GPIO_1_D[19];
assign D4 = GPIO_1_D[17];
assign D3 = GPIO_1_D[15];
assign D2 = GPIO_1_D[13];
assign D1 = GPIO_1_D[11];
assign D0 = GPIO_1_D[9];

reg lastHREF;
reg lastVSYNC;

///////* INSTANTIATE YOUR PLL HERE *///////
jankiestPLL	jankiestPLL_inst (
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
	.RESULT(RESULT),
	.SHAPE(SHAPE)
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

reg [7:0] temp;
//assign GPIO_0_D[31] = RESULT[2];
assign GPIO_0_D[32] = RESULT[1];
assign GPIO_0_D[33] = RESULT[0];

always @ (posedge PCLK) begin
	temp = {D7, D6, D5, D4, D3, D2, D1, D0};
	if (VSYNC == 1 && lastVSYNC == 0) begin //posedge VSYNC
		X_ADDR = 0;
		Y_ADDR = 0; 
		//lastVSYNC <= 1;
	end
	else if (HREF == 0 && lastHREF ==1) begin
			X_ADDR = 0;
			Y_ADDR = Y_ADDR + 1;
	end
	else begin
		if(HREF && PCLK) begin
			if (~byte_num) begin
				//pixel_data_RGB332[7:3] = {temp[6:4],1'b0, 1'b0};
				//pixel_data_RGB332[2:0] = pixel_data_RGB332[2:0];
				//pixel_data_RGB332[7:3] = pixel_data_RGB332[7:3]; //flip
				//pixel_data_RGB332[2:0] = {1'b0, temp[4:3]};
				pixel_data_RGB332[7:5] = temp[3:1]; //RGB444
				pixel_data_RGB332[4:0] = pixel_data_RGB332[4:0];
				X_ADDR = X_ADDR;	
			Y_ADDR = Y_ADDR;
				W_EN = 0;      
			end
			else begin 
				//pixel_data_RGB332[7:3] = pixel_data_RGB332[7:3];
				//pixel_data_RGB332[2:0] = {1'b0, temp[4:3]};
				//pixel_data_RGB332[7:3] = {temp[6:4],1'b0, 1'b0}; //flip
				//pixel_data_RGB332[2:0] = pixel_data_RGB332[2:0];
				pixel_data_RGB332[7:5] = pixel_data_RGB332[7:5]; //RGB444
				pixel_data_RGB332[4:0] =  {temp[7:5], temp[3:2]};
				X_ADDR = X_ADDR + 1;	
				Y_ADDR = Y_ADDR;	
				W_EN = 1;	  
			end
			byte_num = ~byte_num;
		end
		else begin
			X_ADDR=0;
			Y_ADDR = Y_ADDR;
		end
	end
		lastVSYNC = VSYNC;
		lastHREF = HREF;
	/*else if (VSYNC == 0) begin
		lastVSYNC <= 0;
		if (HREF == 0 && lastHREF==1) begin //negedge HREF
			X_ADDR <= 0;
			Y_ADDR <= Y_ADDR + 1;
			lastHREF <= 0;
		end
		else if (HREF == 1) begin
			lastHREF <= 1;
			if (~byte_num) begin
				pixel_data_RGB332[7:3] <= {D7,D6,D5,D2,D1};
				//pixel_data_RGB332[1:0] <= pixel_data_RGB332[1:0];
				X_ADDR <= X_ADDR;	
				W_EN <= 0;      
			end
			else begin 
				//pixel_data_RGB332[7:2] <= pixel_data_RGB332[7:2];
				pixel_data_RGB332[2:0] <= {D4,D3,D2};
				X_ADDR <= X_ADDR + 1;		
				W_EN <= 1;	  
			end
			byte_num <= ~byte_num;
		end
	end
	*/
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

//TRIANGLE (green)
assign GPIO_0_D[31] = (SHAPE == 2'b11) ? 1 : 0;
//SQUARE (red)
assign GPIO_0_D[30] = (SHAPE == 2'b10) ? 1 : 0;
//DIAMOND (yellow)
assign GPIO_0_D[29] = (SHAPE == 2'b01) ? 1 : 0; 

//SEND IMAGE INFO TO ARDUINO
reg write;
reg [1:0] state = 3;
wire [3:0] treasure;
//wire [1:0] SHAPE;
//assign SHAPE[1] = 1;
//assign SHAPE[0] = 0;
assign treasure = {RESULT[1:0], SHAPE[1:0]};
wire arduino_signal;
reg arduino_signal_prev;
assign arduino_signal = GPIO_1_D[32];
always @(posedge c1_sig) begin
		if (arduino_signal == 1 && arduino_signal_prev == 0) begin
			state = state+1;
			write = treasure[state];
		end
		arduino_signal_prev = arduino_signal;
end
//assign GPIO_0_D[30] = write;

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