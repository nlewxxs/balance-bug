module beacon_top (

	input logic MAX10_CLK1_50, lights_on_flag, mosi0, mosi1, SCK0, SCK1, CS0, CS1,
	input logic [9:0] SW,
	output logic PWM1, PWM2, PWM3, miso0, miso1, charge_mode, charge_on, grid_switch, power_out_flag, clk_10_a, clk_10_b,
	output logic [9:0] LED

);

	logic pwm_en, cap_ch, cap_5, PV_pwr, low_pwr, clk_20, sent0, sent1, clk_5M, charge_on1;
	logic [2:0] state_num;
	logic [23:0] data_send0, data_received0;
	logic [15:0] data_send1, data_received1;
	logic [7:0] I_panel, V_panel, V_cap;
	logic [9:0] val;

	clktick 		clk_5Meg 			(.clk(MAX10_CLK1_50),
											 .en(1'b1),
											 .rst(1'b0),
											 .N(5'd24),
											 
											 .tick(clk_5M)
											);
											
//----------------------SPI <-> binary------- encode, decode-------------------------------//

	spi_slave #(.WIDTH(24)) spi0 (.clk(clk_5M), // comms with SMPS
											.SCK(SCK0),
											.MOSI(mosi0),
											.SS(CS0),
											.byte_incoming(data_send0),
											
											.byte_received(data_received0),
											.MISO(miso0),
											.sent(sent0),
										  );
										  
	spi_slave #(.WIDTH(16)) spi1 (.clk(clk_5M), // comms with ESP32
											.SCK(SCK1),
											.MOSI(mosi1),
											.SS(CS1),
											.byte_incoming(data_send1),
											
											.byte_received(data_received1),
											.MISO(miso1),
											.sent(sent1)
										  );
										  
	encode_decode block 			  (.received0(data_received0),
											.received1(data_received1),
											.state(state_num),
											.val(val),
											.charge_on(charge_on1),
											
											.V_panel(V_panel),
											.I_panel(I_panel),
											.V_cap(V_cap),
											.return_data0(data_send0),
											.return_data1(data_send1)
										  );
										  
										  
									  
//---------------------------------set 10Hz clock for state machine-----------------------------//
	clkdiv #(.WIDTH(24)) clk20 (.clk(MAX10_CLK1_50),
										  .rst(1'b0),
										  .en(1'b1),
										  .N(2499999),
										  
										  .tick_phase0(clk_20),
										 );


//---------------------create 19Hz PWM signals out of phase by 120 degrees-----------------//

	pwm_3phase_3Hz pwm 			 (.clk_50MHz(MAX10_CLK1_50),
										  .enable(pwm_en),
								
										  .pwm_phase1(PWM1),
										  .pwm_phase2(PWM2),
										  .pwm_phase3(PWM3)
										 );
						

//---------------------------------implement fsm and mppt-----------------------------------//

	beacon_FSM fsm 			    (.clk(clk_20),
										  .cap_charged(cap_ch),
										  .cap_over5(cap_5),
										  .PV_power_high(PV_pwr),
										  .lights_on_flag(SW[0]),
										  .low_power(1'b0),
						 
										  .charge_mode(charge_mode),
										  .light_power_source(grid_switch),
										  .charge_on(charge_on1),
										  .pwm_enable(pwm_en),
										  .state_number(state_num)
										 );
					
					
	mppt_signals mppt				 (.clk(clk_20),
										  .I_panel(I_panel),
										  .V_panel(V_panel),
										  .V_cap(V_cap),
										  .state_number(state_num),
							 
										  .cap_charged(cap_ch),
										  .cap_over5(cap_5),
										  .PV_power_high(PV_pwr),
										  .pwr_low(low_pwr),
										  .val(val)
										 );

//---------------------------------------assign output pins------------------------------------//

	assign power_out_flag = low_pwr;
	assign charge_on = charge_on1;

	assign LED[0] = state_num[0];
	assign LED[1] = state_num[1];
	assign LED[2] = state_num[2];
	assign LED[3] = cap_ch;
	assign LED[4] = cap_5;
	assign LED[5] = V_cap[3];
	assign LED[6] = V_cap[4];
	assign LED[7] = V_cap[5];
	assign LED[8] = V_cap[6];
	assign LED[9] = V_cap[7];

	assign clk_10_a = clk_20;
	assign clk_10_b = clk_20;
				
endmodule
