module spi_slave(clk, SCK, MISO, SS);
input clk;
input SCK, SS;
output MISO;

reg count = 1'b0;
// no MOSI bc i can't think of anything we would need to send to the FPGA

//sync the SCK to the FPGA clock using 3-bits shift register
reg[2:0] SCKr; always @(posedge clk) SCKr <= {SCKr[1:0], SCK};
wire SCK_risingedge = (SCKr[2:1] == 2'b01);
wire SCK_fallingedge = (SCKr[2:1] == 2'b10);

//same for SS
reg[2:0] SSr; always @(posedge clk) SSr <= {SSr[1:0], SS};
wire SS_active = ~SSr[1];
wire SS_risingedge = (SSr[2:1] == 2'b01);
wire SS_fallingedge = (SSr[2:1] == 2'b10);

// for MOSI the code would look like
//reg[2:0] MOSIr; always @(posedge clk) MOSIr <= {MOSIr[0], MOSI};
//wire MOSI_data = MOSIr[1];

// not doing the whole receiving code for now

// TRANSMISSION: 
//reg[7:0] byte_data_sent;
//reg[7:0] count;
//always @(posedge clk) if(SS_fallingedge) count <= count + 8'h1;

//always @(posedge clk)
//if(SS_active)
//begin
//	if(SCK_risingedge)
//	begin
//		count = count + 1'b1;
//		byte_data_sent <= count;
//	end
//end

always @(posedge clk)
if (SCK_risingedge)
begin
count = ~count;
end

assign MISO = count;

endmodule
