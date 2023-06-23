module spi_slave #(
	parameter WIDTH = 8 
)(

	input logic clk, SCK, MOSI, SS,
	input logic [WIDTH-1:0] byte_incoming,
	output logic [WIDTH-1:0] byte_received,
	output logic MISO, sent
	
);
// ---------------------------------------------------------- //

// For demo purposes, I will receive a byte on MOSI, add 2, 
// and send the result back through MISO

// ---------------------------------------------------------- //
	logic [WIDTH-1:0] byte_to_send;
	reg [5:0] count; 				// counts up to WIDTH

// initialising registers
	initial begin
		count = 1'b0;
		byte_to_send = 0;
		byte_received = 0;
	end

// ---------- Detecting rising / falling edges -------------- //


//sync the SCK to the FPGA clock using 3-bits shift register. 
	reg[2:0] SCKr; 
	always @(posedge clk) SCKr <= {SCKr[1:0], SCK};
	wire SCK_risingedge = (SCKr[2:1] == 2'b01);
	wire SCK_fallingedge = (SCKr[2:1] == 2'b10);

//same for SS
	reg[2:0] SSr; 
	always @(posedge clk) SSr <= {SSr[1:0], SS};
	wire SS_active = ~SSr[1];  // note SPI is active-low
	wire SS_risingedge = (SSr[2:1] == 2'b01);
	wire SS_fallingedge = (SSr[2:1] == 2'b10);


// ------------- TRANSMISSION / RECEIVING ------------------- //

	always @(posedge clk)
	if(SS_risingedge)
		count <= 0;
	else if (SS_active) begin
		// receive on rising edge
		if (SCK_risingedge) begin 
			byte_received = {byte_received[WIDTH-2:0], MOSI};
			count = count + 1'b1;
		end
		// prepare data on falling edge
		if (SCK_fallingedge) begin 
			if (count == WIDTH) begin // when we have sent a full byte
				count <= 0;
				sent <= 1;
				
				// >>>>>>>>>>>> EDIT HERE >>>>>>>>>>>>>>
		
				byte_to_send <= byte_incoming;

				// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
			end
			else begin
				byte_to_send <= {byte_to_send[WIDTH-2:0], 1'b0};
				sent <= 0 ;
			end
		end
	end

	assign MISO = byte_to_send[WIDTH-1];

endmodule