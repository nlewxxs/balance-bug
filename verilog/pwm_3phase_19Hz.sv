module pwm_3phase_3Hz (

	input logic clk_50MHz, enable,
	output logic pwm_phase1, pwm_phase2, pwm_phase3
	
);

	logic clk_300;


	clktick #(.WIDTH(18)) tick 	 (.clk(clk_50MHz),
											  .rst(1'b0),
											  .en(1'b1),
											  .N(18'd167500),
											  .tick(clk_300)
											 );
					 
					 
	pwm_3_phase pwm					 (.clk(clk_300),
											  .en(enable),
											  .pwm1(pwm_phase1),
											  .pwm2(pwm_phase2),
											  .pwm3(pwm_phase3)
											 );

endmodule
//tested and works