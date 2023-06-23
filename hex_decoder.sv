`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: Jacob Larcombe
// 
// Create Date: 14.11.2022
// Design Name: Lab 3
// Module Name: Task 1 - Hex decoder
// Project Name: Lab 3
// Target Devices: 10M50DAF484C7G
// Tool Versions: 
// Description: 
// 	Inital task - translate lower 4 switches into hex digit displayed on lowest 7 seg display
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module hexTo7Seg (
	output 	logic 	[6:0] out,
	input		logic		[3:0] in
	);
	
	always_comb begin
		case(in) 
			4'h0:		out = 7'b1000000;
			4'h1:		out = 7'b1111001;
			4'h2:		out = 7'b0100100;
			4'h3:		out = 7'b0110000;
			4'h4:		out = 7'b0011001;
			4'h5:		out = 7'b0010010;
			4'h6:		out = 7'b0000010;
			4'h7:		out = 7'b1111000;
			4'h8:		out = 7'b0000000;
			4'h9:		out = 7'b0011000;
			4'ha:		out = 7'b0001000;
			4'hb:		out = 7'b0000011;
			4'hc:		out = 7'b1000110;
			4'hd:		out = 7'b0100001;
			4'he:		out = 7'b0000110;
			4'hf:		out = 7'b0001110;
			default: out = 7'b0000000;
		endcase
	end
endmodule

	
	