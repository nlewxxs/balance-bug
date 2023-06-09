module EEE_IMGPROC(
	// global clock & reset
	clk,
	reset_n,
	
	// mm slave
	s_chipselect,
	s_read,
	s_write,
	s_readdata,
	s_writedata,
	s_address,

	// stream sink
	sink_data,
	sink_valid,
	sink_ready,
	sink_sop,
	sink_eop,
	
	// streaming source
	source_data,
	source_valid,
	source_ready,
	source_sop,
	source_eop,
	
	// conduit
	mode
);


// global clock & reset
input	clk;
input	reset_n;

// mm slave
input							s_chipselect;
input							s_read;
input							s_write;
output	reg	[31:0]	s_readdata;
input	[31:0]				s_writedata;
input	[2:0]					s_address;


// streaming sink
input	[23:0]            	sink_data;
input								sink_valid;
output							sink_ready;
input								sink_sop;
input								sink_eop;

// streaming source
output	[23:0]			  	   source_data;
output								source_valid;
input									source_ready;
output								source_sop;
output								source_eop;

// conduit export
input                         mode;

////////////////////////////////////////////////////////////////////////
//
parameter IMAGE_W = 11'd640;
parameter IMAGE_H = 11'd480;
parameter MESSAGE_BUF_MAX = 1024;
parameter MSG_INTERVAL = 6;
parameter BB_COL_DEFAULT = 24'h00ff00;


reg [7:0]   red, green, blue;
wire [7:0]   grey;
wire [7:0]   red_sel, green_sel, blue_sel;

wire [7:0]   red_out, green_out, blue_out;

wire         sop, eop, in_valid, out_ready;
////////////////////////////////////////////////////////////////////////

// Detect red areas
wire tl_detect;
assign tl_detect = red[7] & green[7] & blue[7];

// Find boundary of cursor box

wire [1:0] state;
reg [1:0] next_state;

reg [9:0] x_ctr = 10'b0;
reg [9:0] y_ctr = 10'b0;

assign state = next_state;

// Highlight detected areas
wire [23:0] red_high;
assign grey = green[7:1] + red[7:2] + blue[7:2]; //Grey = green/2 + red/4 + blue/4
assign red_high  =  tl_detect ? {8'hff, 8'hff, 8'hff} : {grey, grey, grey};

// Show bounding box
wire [23:0] new_image;
wire bb_active;
assign bb_active = (x == one_left) 		| (x == two_left)  | (x == three_left)| (x == four_left)| (x == five_left)| (x == six_left)| (x == seven_left)| (x == eight_left)| (x == nine_left)| (x == ten_left)| (x == eleven_left)| (x == twelve_left)| (x == thirteen_left)| (x == fourteen_left)| (x == fifteen_left)| (x == sixteen_left)|
				   (x == one_right) 	| (x == two_right)  | (x == three_right)| (x == four_right)| (x == five_right)| (x == six_right)| (x == seven_right)| (x == eight_right)| (x == nine_right)| (x == ten_right)| (x == eleven_right)| (x == twelve_right)| (x == thirteen_right)| (x == fourteen_right)| (x == fifteen_right)| (x == sixteen_right)|
				   (y == one_top) 		| (y == two_top)  | (y == three_top)| (y == four_top)| (y == five_top)| (y == six_top)| (y == seven_top)| (y == eight_top)| (y == nine_top)| (y == ten_top)| (y == eleven_top)| (y == twelve_top)| (y == thirteen_top)| (y == fourteen_top)| (y == fifteen_top)| (y == sixteen_top)|
				   (y == one_bottom)    | (y == two_bottom)  | (y == three_bottom)| (y == four_bottom)| (y == five_bottom)| (y == six_bottom)| (y == seven_bottom)| (y == eight_bottom)| (y == nine_bottom)| (y == ten_bottom)| (y == eleven_bottom)| (y == twelve_bottom)| (y == thirteen_bottom)| (y == fourteen_bottom)| (y == fifteen_bottom)| (y == sixteen_bottom);
assign new_image = bb_active ? bb_col : red_high;

// Switch output pixels depending on mode switch
// Don't modify the start-of-packet word - it's a packet discriptor
// Don't modify data in non-video packets
assign {red_out, green_out, blue_out} = (mode & ~sop & packet_video) ? new_image : {red,green,blue};

//Count valid pixels to tget the image coordinates. Reset and detect packet type on Start of Packet.
reg [10:0] x, y;
reg packet_video;
always@(posedge clk) begin
	if (sop) begin
		x <= 11'h0;
		y <= 11'h0;
		packet_video <= (blue[3:0] == 3'h0);
	end
	else if (in_valid) begin
		if (x == IMAGE_W-1) begin
			x <= 11'h0;
			y <= y + 11'h1;
		end
		else begin
			x <= x + 11'h1;
		end
	end
end

// reg exceeds;

// always begin
// 	if ((x_min == 320) & (x_max == 320) & (y_min ==240) & (y_max == 240)) begin
// 		exceeds = 1;
// 	end
// 	else begin
// 		exceeds = 0;
// 	end
// end
//Find first and last red pixels
reg [10:0] one_x_min, one_y_min, one_x_max, one_y_max;
reg [10:0] two_x_min, two_y_min, two_x_max, two_y_max;
reg [10:0] three_x_min, three_y_min, three_x_max, three_y_max;
reg [10:0] four_x_min, four_y_min, four_x_max, four_y_max;

reg [10:0] five_x_min, five_y_min, five_x_max, five_y_max;
reg [10:0] six_x_min, six_y_min, six_x_max, six_y_max;
reg [10:0] seven_x_min, seven_y_min, seven_x_max, seven_y_max;
reg [10:0] eight_x_min, eight_y_min, eight_x_max, eight_y_max;

reg [10:0] nine_x_min, nine_y_min, nine_x_max, nine_y_max;
reg [10:0] ten_x_min, ten_y_min, ten_x_max, ten_y_max;
reg [10:0] eleven_x_min, eleven_y_min, eleven_x_max, eleven_y_max;
reg [10:0] twelve_x_min, twelve_y_min, twelve_x_max, twelve_y_max;

reg [10:0] thirteen_x_min, thirteen_y_min, thirteen_x_max, thirteen_y_max;
reg [10:0] fourteen_x_min, fourteen_y_min, fourteen_x_max, fourteen_y_max;
reg [10:0] fifteen_x_min, fifteen_y_min, fifteen_x_max, fifteen_y_max;
reg [10:0] sixteen_x_min, sixteen_y_min, sixteen_x_max, sixteen_y_max;



// reg [10:0] bl_x_min, bl_y_min, bl_x_max, bl_y_max;
// reg [10:0] br_x_min, br_y_min, br_x_max, br_y_max;

reg one_activate, two_activate, three_activate, four_activate;
reg five_activate, six_activate, seven_activate, eight_activate;
reg nine_activate, ten_activate, eleven_activate, twelve_activate;
reg thirteen_activate, fourteen_activate, fifteen_activate, sixteen_activate;

always begin
	//row one
	if(x >= 160 || y >= 120) begin
		one_activate = 0;
	end
	else begin
		one_activate = 1;
	end
	if(!((x >= 160) & (x < 320)) || y >= 120) begin
		two_activate = 0;
	end
	else begin
		two_activate = 1;
	end
	if(!((x >= 320) & (x < 480)) || y >= 120) begin
		three_activate = 0;
	end
	else begin
		three_activate = 1;
	end
	if((x < 480) || y >= 120) begin
		four_activate = 0;
	end
	else begin
		four_activate = 1;
	end

	//row two
	if(x >= 160 || !((y >= 120) & (y < 240))) begin
		five_activate = 0;
	end
	else begin
		five_activate = 1;
	end
	if(!((x >= 160) & (x < 320)) || !((y >= 120) & (y < 240))) begin
		six_activate = 0;
	end
	else begin
		six_activate = 1;
	end
	if(!((x >= 320) & (x < 480)) || !((y >= 120) & (y < 240))) begin
		seven_activate = 0;
	end
	else begin
		seven_activate = 1;
	end
	if((x < 480) || !((y >= 120) & (y < 240))) begin
		eight_activate = 0;
	end
	else begin
		eight_activate = 1;
	end

	//row three
	if(x >= 160 || !((y >= 240) & (y < 360))) begin
		nine_activate = 0;
	end
	else begin
		nine_activate = 1;
	end
	if(!((x >= 160) & (x < 320)) || !((y >= 240) & (y < 360))) begin
		ten_activate = 0;
	end
	else begin
		ten_activate = 1;
	end
	if(!((x >= 320) & (x < 480)) || !((y >= 240) & (y < 360))) begin
		eleven_activate = 0;
	end
	else begin
		eleven_activate = 1;
	end
	if((x < 480) || !((y >= 240) & (y < 360))) begin
		twelve_activate = 0;
	end
	else begin
		twelve_activate = 1;
	end

	//row four
	if(x >= 160 || y < 360) begin
		thirteen_activate = 0;
	end
	else begin
		thirteen_activate = 1;
	end
	if(!((x >= 160) & (x < 320)) || y < 360) begin
		fourteen_activate = 0;
	end
	else begin
		fourteen_activate = 1;
	end
	if(!((x >= 320) & (x < 480)) || y < 360) begin
		fifteen_activate = 0;
	end
	else begin
		fifteen_activate = 1;
	end
	if((x < 480) || y < 360) begin
		sixteen_activate = 0;
	end
	else begin
		sixteen_activate = 1;
	end
end

always@(posedge clk) begin
	//row 1
	if (tl_detect & in_valid & one_activate) begin	//Update bounds when the pixel is red  & (exceeds == 1'b0)
		if((x < 160) & (x < one_x_min)) begin
			one_x_min <= x;
		end
		if((x < 160) & (x > one_x_max)) begin
			one_x_max <= x;
		end
		if((y < 120) & (y < one_y_min)) begin
			one_y_min <= y;
		end
		if(y < 120) begin
			one_y_max <= y;
		end
	end
	if (tl_detect & in_valid & two_activate) begin	//Update bounds when the pixel is red  & (exceeds == 1'b0)
		if((x >= 160) & (x < 320) & (x < two_x_min)) begin
			two_x_min <= x;
		end
		if((x >= 160) & (x < 320) & (x > two_x_max)) begin
			two_x_max <= x;
		end
		if((y < 120) & (y < two_y_min)) begin
			two_y_min <= y;
		end
		if(y < 120) begin
			two_y_max <= y;
		end
	end
	if (tl_detect & in_valid & three_activate) begin	//Update bounds when the pixel is red  & (exceeds == 1'b0)
		if((x >= 320) & (x < 480) & (x < three_x_min)) begin
			three_x_min <= x;
		end
		if((x >= 320) & (x < 480) & (x > three_x_max)) begin
			three_x_max <= x;
		end
		if((y < 120) & (y < three_y_min)) begin
			three_y_min <= y;
		end
		if(y < 120) begin
			three_y_max <= y;
		end
	end
	if (tl_detect & in_valid & four_activate) begin	//Update bounds when the pixel is red  & (exceeds == 1'b0)
		if((x >= 480) & (x < four_x_min)) begin
			four_x_min <= x;
		end
		if((x >= 480) & (x > four_x_max)) begin
			four_x_max <= x;
		end
		if((y < 120) & (y < four_y_min)) begin
			four_y_min <= y;
		end
		if(y < 120) begin
			four_y_max <= y;
		end
	end

	//row two
	if (tl_detect & in_valid & five_activate) begin	//Update bounds when the pixel is red  & (exceeds == 1'b0)
		if((x < 160) & (x < five_x_min)) begin
			five_x_min <= x;
		end
		if((x < 160) & (x > five_x_max)) begin
			five_x_max <= x;
		end
		if((y >= 120) & (y < 240) & (y < five_y_min)) begin
			five_y_min <= y;
		end
		if((y >= 120) & (y < 240)) begin
			five_y_max <= y;
		end
	end
	if (tl_detect & in_valid & six_activate) begin	//Update bounds when the pixel is red  & (exceeds == 1'b0)
		if((x >= 160) & (x < 320) & (x < six_x_min)) begin
			six_x_min <= x;
		end
		if((x >= 160) & (x < 320) & (x > six_x_max)) begin
			six_x_max <= x;
		end
		if((y >= 120) & (y < 240) & (y < six_y_min)) begin
			six_y_min <= y;
		end
		if((y >= 120) & (y < 240)) begin
			six_y_max <= y;
		end
	end
	if (tl_detect & in_valid & seven_activate) begin	//Update bounds when the pixel is red  & (exceeds == 1'b0)
		if((x >= 320) & (x < 480) & (x < seven_x_min)) begin
			seven_x_min <= x;
		end
		if((x >= 320) & (x < 480) & (x > seven_x_max)) begin
			seven_x_max <= x;
		end
		if((y >= 120) & (y < 240) & (y < seven_y_min)) begin
			seven_y_min <= y;
		end
		if((y >= 120) & (y < 240)) begin
			seven_y_max <= y;
		end
	end
	if (tl_detect & in_valid & eight_activate) begin	//Update bounds when the pixel is red  & (exceeds == 1'b0)
		if((x >= 480) & (x < eight_x_min)) begin
			eight_x_min <= x;
		end
		if((x >= 480) & (x > eight_x_max)) begin
			eight_x_max <= x;
		end
		if((y >= 120) & (y < 240) & (y < eight_y_min)) begin
			eight_y_min <= y;
		end
		if((y >= 120) & (y < 240)) begin
			eight_y_max <= y;
		end
	end

	//row three
	if (tl_detect & in_valid & nine_activate) begin	//Update bounds when the pixel is red  & (exceeds == 1'b0)
		if((x < 160) & (x < nine_x_min)) begin
			nine_x_min <= x;
		end
		if((x < 160) & (x > nine_x_max)) begin
			nine_x_max <= x;
		end
		if((y >= 240) & (y < 360) & (y < nine_y_min)) begin
			nine_y_min <= y;
		end
		if((y >= 240) & (y < 360)) begin
			nine_y_max <= y;
		end
	end
	if (tl_detect & in_valid & ten_activate) begin	//Update bounds when the pixel is red  & (exceeds == 1'b0)
		if((x >= 160) & (x < 320) & (x < ten_x_min)) begin
			ten_x_min <= x;
		end
		if((x >= 160) & (x < 320) & (x > ten_x_max)) begin
			ten_x_max <= x;
		end
		if((y >= 240) & (y < 360) & (y < ten_y_min)) begin
			ten_y_min <= y;
		end
		if((y >= 240) & (y < 360)) begin
			ten_y_max <= y;
		end
	end
	if (tl_detect & in_valid & eleven_activate) begin	//Update bounds when the pixel is red  & (exceeds == 1'b0)
		if((x >= 320) & (x < 480) & (x < eleven_x_min)) begin
			eleven_x_min <= x;
		end
		if((x >= 320) & (x < 480) & (x > eleven_x_max)) begin
			eleven_x_max <= x;
		end
		if((y >= 240) & (y < 360) & (y < eleven_y_min)) begin
			eleven_y_min <= y;
		end
		if((y >= 240) & (y < 360)) begin
			eleven_y_max <= y;
		end
	end
	if (tl_detect & in_valid & twelve_activate) begin	//Update bounds when the pixel is red  & (exceeds == 1'b0)
		if((x >= 480) & (x < twelve_x_min)) begin
			twelve_x_min <= x;
		end
		if((x >= 480) & (x > twelve_x_max)) begin
			twelve_x_max <= x;
		end
		if((y >= 240) & (y < 360) & (y < twelve_y_min)) begin
			twelve_y_min <= y;
		end
		if((y >= 240) & (y < 360)) begin
			twelve_y_max <= y;
		end
	end

	//row four
	if (tl_detect & in_valid & thirteen_activate) begin	//Update bounds when the pixel is red  & (exceeds == 1'b0)
		if((x < 160) & (x < thirteen_x_min)) begin
			thirteen_x_min <= x;
		end
		if((x < 160) & (x > thirteen_x_max)) begin
			thirteen_x_max <= x;
		end
		if((y >= 360) & (y < thirteen_y_min)) begin
			thirteen_y_min <= y;
		end
		if(y >= 360) begin
			thirteen_y_max <= y;
		end
	end
	if (tl_detect & in_valid & fourteen_activate) begin	//Update bounds when the pixel is red  & (exceeds == 1'b0)
		if((x >= 160) & (x < 320) & (x < fourteen_x_min)) begin
			fourteen_x_min <= x;
		end
		if((x >= 160) & (x < 320) & (x > fourteen_x_max)) begin
			fourteen_x_max <= x;
		end
		if((y >= 360) & (y < fourteen_y_min)) begin
			fourteen_y_min <= y;
		end
		if(y >= 360) begin
			fourteen_y_max <= y;
		end
	end
	if (tl_detect & in_valid & fifteen_activate) begin	//Update bounds when the pixel is red  & (exceeds == 1'b0)
		if((x >= 320) & (x < 480) & (x < fifteen_x_min)) begin
			fifteen_x_min <= x;
		end
		if((x >= 320) & (x < 480) & (x > fifteen_x_max)) begin
			fifteen_x_max <= x;
		end
		if((y >= 360) & (y < fifteen_y_min)) begin
			fifteen_y_min <= y;
		end
		if(y >= 360) begin
			fifteen_y_max <= y;
		end
	end
	if (tl_detect & in_valid & sixteen_activate) begin	//Update bounds when the pixel is red  & (exceeds == 1'b0)
		if((x >= 480) & (x < sixteen_x_min)) begin
			sixteen_x_min <= x;
		end
		if((x >= 480) & (x > sixteen_x_max)) begin
			sixteen_x_max <= x;
		end
		if((y >= 360) & (y < sixteen_y_min)) begin
			sixteen_y_min <= y;
		end
		if(y >= 360) begin
			sixteen_y_max <= y;
		end
	end

	if (sop & in_valid) begin	//Reset bounds on start of packet
		//row 1
		one_x_min <= IMAGE_W-11'h1;
		one_x_max <= 0;
		one_y_min <= IMAGE_H-11'h1;
		one_y_max <= 0;

		two_x_min <= IMAGE_W-11'h1;
		two_x_max <= 0;
		two_y_min <= IMAGE_H-11'h1;
		two_y_max <= 0;

		three_x_min <= IMAGE_W-11'h1;
		three_x_max <= 0;
		three_y_min <= IMAGE_H-11'h1;
		three_y_max <= 0;

		four_x_min <= IMAGE_W-11'h1;
		four_x_max <= 0;
		four_y_min <= IMAGE_H-11'h1;
		four_y_max <= 0;



		//////////////////////////
		//row 2
		five_x_min <= IMAGE_W-11'h1;
		five_x_max <= 0;
		five_y_min <= IMAGE_H-11'h1;
		five_y_max <= 0;

		six_x_min <= IMAGE_W-11'h1;
		six_x_max <= 0;
		six_y_min <= IMAGE_H-11'h1;
		six_y_max <= 0;

		seven_x_min <= IMAGE_W-11'h1;
		seven_x_max <= 0;
		seven_y_min <= IMAGE_H-11'h1;
		seven_y_max <= 0;

		eight_x_min <= IMAGE_W-11'h1;
		eight_x_max <= 0;
		eight_y_min <= IMAGE_H-11'h1;
		eight_y_max <= 0;


		/////////////////////////
		//row 3
		nine_x_min <= IMAGE_W-11'h1;
		nine_x_max <= 0;
		nine_y_min <= IMAGE_H-11'h1;
		nine_y_max <= 0;

		ten_x_min <= IMAGE_W-11'h1;
		ten_x_max <= 0;
		ten_y_min <= IMAGE_H-11'h1;
		ten_y_max <= 0;

		eleven_x_min <= IMAGE_W-11'h1;
		eleven_x_max <= 0;
		eleven_y_min <= IMAGE_H-11'h1;
		eleven_y_max <= 0;

		twelve_x_min <= IMAGE_W-11'h1;
		twelve_x_max <= 0;
		twelve_y_min <= IMAGE_H-11'h1;
		twelve_y_max <= 0;


		/////////////////////////
		//row 4
		thirteen_x_min <= IMAGE_W-11'h1;
		thirteen_x_max <= 0;
		thirteen_y_min <= IMAGE_H-11'h1;
		thirteen_y_max <= 0;

		fourteen_x_min <= IMAGE_W-11'h1;
		fourteen_x_max <= 0;
		fourteen_y_min <= IMAGE_H-11'h1;
		fourteen_y_max <= 0;

		fifteen_x_min <= IMAGE_W-11'h1;
		fifteen_x_max <= 0;
		fifteen_y_min <= IMAGE_H-11'h1;
		fifteen_y_max <= 0;

		sixteen_x_min <= IMAGE_W-11'h1;
		sixteen_x_max <= 0;
		sixteen_y_min <= IMAGE_H-11'h1;
		sixteen_y_max <= 0;
		
	end
end

//Process bounding box at the end of the frame.
reg [4:0] msg_state;

//row 1
reg [10:0] one_left, one_right, one_top, one_bottom;
reg [10:0] two_left, two_right, two_top, two_bottom;
reg [10:0] three_left, three_right, three_top, three_bottom;
reg [10:0] four_left, four_right, four_top, four_bottom;

//row 2
reg [10:0] five_left, five_right, five_top, five_bottom;
reg [10:0] six_left, six_right, six_top, six_bottom;
reg [10:0] seven_left, seven_right, seven_top, seven_bottom;
reg [10:0] eight_left, eight_right, eight_top, eight_bottom;

//row 3
reg [10:0] nine_left, nine_right, nine_top, nine_bottom;
reg [10:0] ten_left, ten_right, ten_top, ten_bottom;
reg [10:0] eleven_left, eleven_right, eleven_top, eleven_bottom;
reg [10:0] twelve_left, twelve_right, twelve_top, twelve_bottom;

//row 4
reg [10:0] thirteen_left, thirteen_right, thirteen_top, thirteen_bottom;
reg [10:0] fourteen_left, fourteen_right, fourteen_top, fourteen_bottom;
reg [10:0] fifteen_left, fifteen_right, fifteen_top, fifteen_bottom;
reg [10:0] sixteen_left, sixteen_right, sixteen_top, sixteen_bottom;

reg [7:0] frame_count;
always@(posedge clk) begin
	if (eop & in_valid & packet_video) begin  //Ignore non-video packets
		
		//row1
		one_left <= one_x_min;
		one_right <= one_x_max;
		one_top <= one_y_min;
		one_bottom <= one_y_max;

		two_left <= two_x_min;
		two_right <= two_x_max;
		two_top <= two_y_min;
		two_bottom <= two_y_max;

		three_left <= 	three_x_min;
		three_right <= 	three_x_max;
		three_top <= 	three_y_min;
		three_bottom <= three_y_max;

		four_left <= 	four_x_min;
		four_right <= 	four_x_max;
		four_top <=		four_y_min;
		four_bottom <= 	four_y_max;

		//row2
		five_left <= 	five_x_min;
		five_right <= 	five_x_max;
		five_top <= 	five_y_min;
		five_bottom <= 	five_y_max;

		six_left <= 	six_x_min;
		six_right <= 	six_x_max;
		six_top <= 		six_y_min;
		six_bottom <= 	six_y_max;

		seven_left <= 	seven_x_min;
		seven_right <= 	seven_x_max;
		seven_top <= 	seven_y_min;
		seven_bottom <= seven_y_max;

		eight_left <= 		eight_x_min;
		eight_right <= 		eight_x_max;
		eight_top <=		eight_y_min;
		eight_bottom <= 	eight_y_max;

		//row3
		nine_left <= 	nine_x_min;
		nine_right <= 	nine_x_max;
		nine_top <= 	nine_y_min;
		nine_bottom <= 	nine_y_max;

		ten_left <= 	ten_x_min;
		ten_right <= 	ten_x_max;
		ten_top <= 		ten_y_min;
		ten_bottom <= 	ten_y_max;

		eleven_left <= 		eleven_x_min;
		eleven_right <= 	eleven_x_max;
		eleven_top <= 		eleven_y_min;
		eleven_bottom <= 	eleven_y_max;

		twelve_left <= 		twelve_x_min;
		twelve_right <= 	twelve_x_max;
		twelve_top <=		twelve_y_min;
		twelve_bottom <= 	twelve_y_max;

		//row4
		thirteen_left <= 	thirteen_x_min;
		thirteen_right <= 	thirteen_x_max;
		thirteen_top <= 	thirteen_y_min;
		thirteen_bottom <= 	thirteen_y_max;

		fourteen_left <= 	fourteen_x_min;
		fourteen_right <= 	fourteen_x_max;
		fourteen_top <= 	fourteen_y_min;
		fourteen_bottom <= 	fourteen_y_max;

		fifteen_left <= 	fifteen_x_min;
		fifteen_right <= 	fifteen_x_max;
		fifteen_top <= 		fifteen_y_min;
		fifteen_bottom <= 	fifteen_y_max;

		sixteen_left <= 	sixteen_x_min;
		sixteen_right <= 	sixteen_x_max;
		sixteen_top <=		sixteen_y_min;
		sixteen_bottom <= 	sixteen_y_max;

		//Latch edges for display overlay on next frame
		
		
		//Start message writer FSM once every MSG_INTERVAL frames, if there is room in the FIFO
		frame_count <= frame_count - 1;
		
		if (frame_count == 0 && msg_buf_size < MESSAGE_BUF_MAX - 3) begin
			msg_state <= 5'b00001;
			frame_count <= MSG_INTERVAL-1;
		end
	end
	
	//Cycle through message writer states once started
	if (msg_state != 5'b0000) begin
		if(msg_state == 5'b10111) begin
			msg_state <= 5'b0000;
		end
		else begin
			msg_state <= msg_state + 5'b00001;
		end
	end
end


//TODO GENERATE OUTPUT MESSAGES
//Generate output messages for CPU
reg [31:0] msg_buf_in; 
wire [31:0] msg_buf_out;
reg msg_buf_wr;
wire msg_buf_rd, msg_buf_flush;
wire [9:0] msg_buf_size;
wire msg_buf_empty;

`define RED_BOX_MSG_ID "NB"

always@(*) begin	//Write words to FIFO as state machine advances TODO:ADD NEW BOX HERE
	case(msg_state)
		5'b00000: begin
			msg_buf_in = 32'b0;
			msg_buf_wr = 1'b0;
		end
		5'b00001: begin
			msg_buf_in = `RED_BOX_MSG_ID;	//Message ID
			msg_buf_wr = 1'b1;
		end
		5'b00010: begin
			msg_buf_in = {one_x_min, one_y_min, two_x_min[10:1]};	//22 full 10 from two
			msg_buf_wr = 1'b1;
		end
		5'b00011: begin
			msg_buf_in = {two_x_min[0], two_y_min, three_x_min, three_y_min[10:2]};	//1 from 2, 23 up until end, 9 from 3 ymin
			msg_buf_wr = 1'b1;
		end
		5'b00100: begin
			msg_buf_in = {three_y_min[1:0], four_x_min, four_y_min, five_x_min[10:3]};	//8 from 5 xmin
			msg_buf_wr = 1'b1;
		end
		5'b00101: begin
			msg_buf_in = {five_x_min[2:0], five_y_min, six_x_min, six_y_min[10:4]};	//Top left coordinate //, two_x_min, two_y_min, bl_x_min, bl_y_min, br_x_min, br_y_min
			msg_buf_wr = 1'b1;
		end
		5'b00110: begin
			msg_buf_in = {six_y_min[3:0], seven_x_min, seven_y_min, eight_x_min[10:5]};	//Top left coordinate //, two_x_min, two_y_min, bl_x_min, bl_y_min, br_x_min, br_y_min
			msg_buf_wr = 1'b1;
		end
		5'b00111: begin
			msg_buf_in = {eight_x_min[4:0], eight_y_min, nine_x_min, nine_y_min[10:6]};	//Top left coordinate //, two_x_min, two_y_min, bl_x_min, bl_y_min, br_x_min, br_y_min
			msg_buf_wr = 1'b1;
		end
		5'b01000: begin
			msg_buf_in = {nine_y_min[5:0], ten_x_min, ten_y_min, eleven_x_min[10:7]};	//Top left coordinate //, two_x_min, two_y_min, bl_x_min, bl_y_min, br_x_min, br_y_min
			msg_buf_wr = 1'b1;
		end
		5'b01001: begin
			msg_buf_in = {eleven_x_min[6:0], eleven_y_min, twelve_x_min, twelve_y_min[10:8]};	//Top left coordinate //, two_x_min, tr_y_min, bl_x_min, bl_y_min, br_x_min, br_y_min
			msg_buf_wr = 1'b1;
		end
		5'b01010: begin
			msg_buf_in = {twelve_y_min[7:0], thirteen_x_min, thirteen_y_min, fourteen_x_min[10:9]};	//Top left coordinate //, two_x_min, tr_y_min, bl_x_min, bl_y_min, br_x_min, br_y_min
			msg_buf_wr = 1'b1;
		end
		5'b01011: begin
			msg_buf_in = {fourteen_x_min[8:0], fourteen_y_min, fifteen_x_min, fifteen_y_min[10]};	//Top left coordinate //, two_x_min, tr_y_min, bl_x_min, bl_y_min, br_x_min, br_y_min
			msg_buf_wr = 1'b1;
		end
		5'b01100: begin
			msg_buf_in = {fifteen_y_min[9:0], sixteen_x_min, sixteen_y_min};	//Top left coordinate //, two_x_min, tr_y_min, bl_x_min, bl_y_min, br_x_min, br_y_min
			msg_buf_wr = 1'b1;
		end

		5'b01101: begin
			msg_buf_in = {one_x_max, one_y_max, two_x_max[10:1]};	//22 full 10 from two
			msg_buf_wr = 1'b1;
		end
		5'b01110: begin
			msg_buf_in = {two_x_max[0], two_y_max, three_x_max, three_y_max[10:2]};	//1 from 2, 23 up until end, 9 from 3 ymax
			msg_buf_wr = 1'b1;
		end
		5'b01111: begin
			msg_buf_in = {three_y_max[1:0], four_x_max, four_y_max, five_x_max[10:3]};	//8 from 5 xmax
			msg_buf_wr = 1'b1;
		end
		5'b10000: begin
			msg_buf_in = {five_x_max[2:0], five_y_max, six_x_max, six_y_max[10:4]};	//Top left coordinate //, two_x_max, two_y_max, bl_x_max, bl_y_max, br_x_max, br_y_max
			msg_buf_wr = 1'b1;
		end
		5'b10001: begin
			msg_buf_in = {six_y_max[3:0], seven_x_max, seven_y_max, eight_x_max[10:5]};	//Top left coordinate //, two_x_max, two_y_max, bl_x_max, bl_y_max, br_x_max, br_y_max
			msg_buf_wr = 1'b1;
		end
		5'b10010: begin
			msg_buf_in = {eight_x_max[4:0], eight_y_max, nine_x_max, nine_y_max[10:6]};	//Top left coordinate //, two_x_max, two_y_max, bl_x_max, bl_y_max, br_x_max, br_y_max
			msg_buf_wr = 1'b1;
		end
		5'b10011: begin
			msg_buf_in = {nine_y_max[5:0], ten_x_max, ten_y_max, eleven_x_max[10:7]};	//Top left coordinate //, two_x_max, two_y_max, bl_x_max, bl_y_max, br_x_max, br_y_max
			msg_buf_wr = 1'b1;
		end
		5'b10100: begin
			msg_buf_in = {eleven_x_max[6:0], eleven_y_max, twelve_x_max, twelve_y_max[10:8]};	//Top left coordinate //, two_x_max, tr_y_max, bl_x_max, bl_y_max, br_x_max, br_y_max
			msg_buf_wr = 1'b1;
		end
		5'b10101: begin
			msg_buf_in = {twelve_y_max[7:0], thirteen_x_max, thirteen_y_max, fourteen_x_max[10:9]};	//Top left coordinate //, two_x_max, tr_y_max, bl_x_max, bl_y_max, br_x_max, br_y_max
			msg_buf_wr = 1'b1;
		end
		5'b10110: begin
			msg_buf_in = {fourteen_x_max[8:0], fourteen_y_max, fifteen_x_max, fifteen_y_max[10]};	//Top left coordinate //, two_x_max, tr_y_max, bl_x_max, bl_y_max, br_x_max, br_y_max
			msg_buf_wr = 1'b1;
		end
		5'b10111: begin
			msg_buf_in = {fifteen_y_max[9:0], sixteen_x_max, sixteen_y_max};	//Top left coordinate //, two_x_max, tr_y_max, bl_x_max, bl_y_max, br_x_max, br_y_max
			msg_buf_wr = 1'b1;
		end
	endcase
end



//Output message FIFO
MSG_FIFO	MSG_FIFO_inst (
	.clock (clk),
	.data (msg_buf_in),
	.rdreq (msg_buf_rd),
	.sclr (~reset_n | msg_buf_flush),
	.wrreq (msg_buf_wr),
	.q (msg_buf_out),
	.usedw (msg_buf_size),
	.empty (msg_buf_empty)
);


//Streaming registers to buffer video signal
STREAM_REG #(.DATA_WIDTH(26)) in_reg (
	.clk(clk),
	.rst_n(reset_n),
	.ready_out(sink_ready),
	.valid_out(in_valid),
	.data_out({red,green,blue,sop,eop}),
	.ready_in(out_ready),
	.valid_in(sink_valid),
	.data_in({sink_data,sink_sop,sink_eop})
);

STREAM_REG #(.DATA_WIDTH(26)) out_reg (
	.clk(clk),
	.rst_n(reset_n),
	.ready_out(out_ready),
	.valid_out(source_valid),
	.data_out({source_data,source_sop,source_eop}),
	.ready_in(source_ready),
	.valid_in(in_valid),
	.data_in({red_out, green_out, blue_out, sop, eop})
);


/////////////////////////////////
/// Memory-mapped port		 /////
/////////////////////////////////

// Addresses
`define REG_STATUS    			0
`define READ_MSG    				1
`define READ_ID    				2
`define REG_BBCOL					3

//Status register bits
// 31:16 - unimplemented
// 15:8 - number of words in message buffer (read only)
// 7:5 - unused
// 4 - flush message buffer (write only - read as 0)
// 3:0 - unused


// Process write

reg  [7:0]   reg_status;
reg	[23:0]	bb_col;

always @ (posedge clk)
begin
	if (~reset_n)
	begin
		reg_status <= 8'b0;
		bb_col <= BB_COL_DEFAULT;
	end
	else begin
		if(s_chipselect & s_write) begin
		   if      (s_address == `REG_STATUS)	reg_status <= s_writedata[7:0];
		   if      (s_address == `REG_BBCOL)	bb_col <= s_writedata[23:0];
		end
	end
end


//Flush the message buffer if 1 is written to status register bit 4
assign msg_buf_flush = (s_chipselect & s_write & (s_address == `REG_STATUS) & s_writedata[4]);

// Process reads
reg read_d; //Store the read signal for correct updating of the message buffer

// Copy the requested word to the output port when there is a read.
always @ (posedge clk)
begin
   if (~reset_n) begin
	   s_readdata <= {32'b0};
		read_d <= 1'b0;
	end
	
	else if (s_chipselect & s_read) begin
		if   (s_address == `REG_STATUS) s_readdata <= {16'b0,msg_buf_size,reg_status};
		if   (s_address == `READ_MSG) s_readdata <= {msg_buf_out};
		if   (s_address == `READ_ID) s_readdata <= 32'h1234EEE2;
		if   (s_address == `REG_BBCOL) s_readdata <= {8'h0, bb_col};
	end
	
	read_d <= s_read;
end

//Fetch next word from message buffer after read from READ_MSG
assign msg_buf_rd = s_chipselect & s_read & ~read_d & ~msg_buf_empty & (s_address == `READ_MSG);
						


endmodule