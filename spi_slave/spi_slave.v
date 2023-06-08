module spi_slave(clk, SCK, MISO, MOSI, SS);

input 	clk;
input 	SCK; 
input 	SS;
input 	MOSI;		// master out slave in
output 	MISO;		// master in slave out

// ---------------------------------------------------------- //

// For demo purposes, I will receive a byte on MOSI, add 2, 
// and send the result back through MISO

// ---------------------------------------------------------- //

reg [7:0] byte_to_send; 	// the current byte being sent
reg [7:0] byte_received;	// MOSI byte currently being processed
reg [3:0] count; 				// counts up to 8

// initialising registers 
initial begin
	count = 1'b0;
	byte_to_send = 8'b0;
	byte_received = 8'b0;
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
if (SS_active) begin
	// receive on rising edge
	if (SCK_risingedge) begin 
		byte_received = {byte_received[6:0], MOSI};
		count = count + 1'b1;
	end
	// prepare data on falling edge
	if (SCK_fallingedge) begin 
		if (count == 8) begin // when we have sent a full byte
			count <= 0;
			
			// >>>>>>>>>>>> EDIT HERE >>>>>>>>>>>>>>
	
			byte_to_send = byte_received + 2'b10;

			// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		end
		else byte_to_send = {byte_to_send[6:0], 1'b0}; 	
	end
end

assign MISO = byte_to_send[7];

endmodule
