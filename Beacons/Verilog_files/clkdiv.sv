module clkdiv #(
	parameter WIDTH = 16
)(
  
	input  logic clk, rst, en,     
	input  logic [WIDTH-1:0] N,     	 
	output logic tick_phase0, tick_phase180     
	
);

	logic [WIDTH-1:0] count;

	always_ff @ (posedge clk)
		 if (rst) begin
			  tick_phase0 <= 1'b0;
			  tick_phase180 <= 1'b0;
			  count <= N;  
		 end
		 else if (en) begin
			 if (count == 0) begin
				 tick_phase0 <= 1'b1;
				 tick_phase180 <= 1'b0;
				 count <= N;
			 end
			 else if (count < {1'b0, N[WIDTH-1:1]}) begin
				 tick_phase0 <= 1'b1;        
				 tick_phase180 <= 1'b0;
				 count <= count - 1'b1;
			 end
			 else begin
				 tick_phase0 <= 1'b0;
				 tick_phase180 <= 1'b1;
				 count <= count - 1'b1;
			 end
		 end
endmodule
