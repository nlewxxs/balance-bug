module spi_slave(clk, SCK, MISO, SS);
input clk;
input SCK, SS;
output MISO;
// no MOSI bc i can't think of anything we would need to send to the FPGA

reg [7:0] data [0:15]; // 16 different bytes that we want to send
reg [7:0] byte_to_send;
reg [4:0] index; // index of byte_to_send
reg [3:0] count; // bit index

// populate array
initial begin
	data[0] = 8'b00000001;
	data[1] = 8'b00000010;
	data[2] = 8'b00000011;
	data[3] = 8'b00000100;
	data[4] = 8'b00000101;
	data[5] = 8'b00000110; // 6
	data[6] = 8'b00000111;
	data[7] = 8'b00001000;
	data[8] = 8'b00010001;
	data[9] = 8'b00010010;
	data[10] = 8'b00010011;
	data[11] = 8'b00010100;
	data[12] = 8'b00010101;
	data[13] = 8'b00010110; // 22
	data[14] = 8'b00010111;
	data[15] = 8'b00011000;
	index = 1'b0;
	count = 0;
	byte_to_send = data[0];
end

//sync the SCK to the FPGA clock using 3-bits shift register
reg[2:0] SCKr; always @(posedge clk) SCKr <= {SCKr[1:0], SCK};
wire SCK_risingedge = (SCKr[2:1] == 2'b01);
wire SCK_fallingedge = (SCKr[2:1] == 2'b10);

//same for SS
reg[2:0] SSr; always @(posedge clk) SSr <= {SSr[1:0], SS};
wire SS_active = ~SSr[1];
wire SS_risingedge = (SSr[2:1] == 2'b01);
wire SS_fallingedge = (SSr[2:1] == 2'b10);


// TRANSMISSION: 

always @(posedge clk)
if (SS_active) begin
	if (SCK_fallingedge) begin 
		byte_to_send = {byte_to_send[6:0], 1'b0}; 
		count = count + 1; 
	end
	if (SCK_risingedge && (count == 8)) begin
		index = index + 1;
		byte_to_send = data[index];
		count = 0;
	end
end

assign MISO = byte_to_send[7];

endmodule
