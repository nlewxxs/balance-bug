module pwm_3_phase (

	input logic clk, en,
	output logic pwm1, pwm2, pwm3

);

	logic [6:0] count;
	initial count = 1'b0;

	always_ff @(posedge clk)
		if (count == 7'd99) begin
			count <= 1'b0;
		end
		else begin
			count = count + en;
		end
		
	always_comb

		if (~en) begin
			pwm1 = 1'b0;
			pwm2 = 1'b0;
			pwm3 = 1'b0;
		end
		else if(count < 7'd34) begin
			pwm1 = 1'b1;
			pwm2 = 1'b0;
			pwm3 = 1'b0;
		end	
		else if (count < 7'd67) begin
			pwm1 = 1'b0;
			pwm2 = 1'b1;
			pwm3 = 1'b0;
		end
		else if (count < 7'd100) begin
			pwm1 = 1'b0;
			pwm2 = 1'b0;
			pwm3 = 1'b1;
		end
		else begin
			pwm1 = 1'b0;
			pwm2 = 1'b0;
			pwm3 = 1'b0;
		end
			
endmodule
		