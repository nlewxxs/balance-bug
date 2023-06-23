module beacon_top (

	input logic MAX10_CLK1_50, lights_on_flag, mosi0, mosi1, SCK0, SCK1, CS0, CS1, KEY,
	input logic [9:0] SW,
	output logic [7:0] HEX0, HEX1, HEX2, HEX3, HEX4, HEX5, 
	output logic PWM1, PWM2, PWM3, miso0, miso1, charge_mode, charge_on, grid_switch, power_out_flag, clk_10_a, clk_10_b,
	output logic [9:0] LED

);

	logic pwm_en, cap_ch, cap_5, PV_pwr, low_pwr1, low_pwr2, clk_20, sent0, sent1, clk_5M, charge_on1;
	logic [2:0] state_num;
	logic [23:0] data_send0, data_received0;
	logic [15:0] data_send1, data_received1;
	logic [7:0] I_panel, V_panel, V_cap;
	logic [9:0] val;

	clktick 		clk_5Meg 			(.clk(MAX10_CLK1_50),
											 .en(1'b1),
											 .rst(1'bf0),
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
											.MISO(grid_switch),
											.sent(sent0)
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
// temporarily 0.1Hz for debugging
// alernative: use button 0 for incrementing counter clock 
// assign clk_20 = KEY;

	clkdiv #(.WIDTH(29)) clk20 (.clk(MAX10_CLK1_50),
										  .rst(1'b0),
										  .en(1'b1),
										  .N(4999999),
										  
										  .tick_phase0(clk_20)
										 );


//---------------------create 19Hz PWM signals out of phase by 120 degrees-----------------//

	pwm_3phase_3Hz pwm 			 (.clk_50MHz(MAX10_CLK1_50),
										  .enable(SW[0]),
								
										  .pwm_phase1(PWM1),
										  .pwm_phase2(PWM2),
										  .pwm_phase3(PWM3)
										 );
						

//---------------------------------implement fsm and mppt-----------------------------------//

	beacon_FSM fsm 			    (.clk(clk_20),
										  .cap_charged(cap_ch),
										  .cap_over5(cap_5),
										  .PV_power_high(1'b1),	// change
										  .lights_on_flag(SW[0]),	// change
										  .low_power1(1'b0),		// change
										  .low_power2(1'b0),			// change
						 
										  .charge_mode(charge_mode),
										  .light_power_source(),		// ad back in grid switch
										  .charge_on(charge_on1),
										  .pwm_enable(pwm_en),
										  .state_number(state_num)
										 );
					
					
	mppt_signals mppt				 (.clk(clk_20),
										  .I_panel(I_panel),
										  .V_panel(V_panel),
										  .V_cap(SW[9:2]),		//change
										  .state_number(state_num),
							 
										  .cap_charged(cap_ch),
										  .cap_over5(cap_5),
										  .PV_power_high(PV_pwr),
										  .pwr_low1(low_pwr1),
										  .pwr_low2(low_pwr2),
										  .val(val)
										 );
										 
//---------------------------------------Debugging Hex Displays--------------------------------//

logic [3:0] bcd0, bcd1, bcd2, bcd3, bcd4, bcd5;

bin2bcd_16 decode8BitBCD1 (.x(I_panel),
									.BCD0(bcd0),
									.BCD1(bcd1),
									.BCD2(bcd2));
bin2bcd_16 deocde8BitBCD2 (.x(data_received0[23:16]),
									.BCD0(bcd3),
									.BCD1(bcd4),
									.BCD2(bcd5));
									
hexTo7Seg hex0 (.in(bcd0),
						.out(HEX0[6:0]));
hexTo7Seg hex1 (.in(bcd1),
						.out(HEX1[6:0]));
hexTo7Seg hex2 (.in(bcd2),
						.out(HEX2[6:0]));
hexTo7Seg hex3 (.in(bcd3),
						.out(HEX3[6:0]));
hexTo7Seg hex4 (.in(bcd4),
						.out(HEX4[6:0]));
hexTo7Seg hex5 (.in(bcd5),
						.out(HEX5[6:0]));

//---------------------------------------assign output pins------------------------------------//

	assign power_out_flag = (((low_pwr2)&&(state_num != 4))||(low_pwr1));
	assign charge_on = charge_on1;
	assign LED[0] = state_num[0];
	assign LED[1] = state_num[1];
	assign LED[2] = state_num[2];
	assign LED[3] = cap_ch;
	assign LED[4] = cap_5;
	assign LED[5] = val[3];
	assign LED[6] = val[4];
	assign LED[7] = val[5];
	assign LED[8] = val[6];
	assign LED[9] = val[7];

	assign clk_10_a = clk_20;
	assign clk_10_b = clk_20;
				
endmodule
