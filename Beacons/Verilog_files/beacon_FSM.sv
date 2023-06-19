module beacon_FSM (

	input logic clk,
	input logic cap_charged, // not charged 0, charged 1
	input logic cap_over5, // below 5V 0, above 5V 1
	input logic PV_power_high, // 0 low power, 1 high power
	input logic lights_on_flag, // off 0, on 1 
	input logic low_power, //0 fine, 1 too low
	
	output logic charge_mode, //buck 0, boost 1
	output logic light_power_source, // panels 0, cap 1
	output logic charge_on, //0 not charging, 1 charging
	output logic pwm_enable, // also acts as a beacon enable
	output [2:0] state_number
	
);

//---------------------------initiating state logic--------------------------------------------//

	typedef enum {IDLE, charge_buck, charge_boost, beacons_panel, beacons_cap} state;
	state current_state, next_state;

	always_ff @ (posedge clk)
		current_state <= next_state;
		
	initial current_state = IDLE; //change this for testing
	
	always_comb begin
		case(current_state)
	
//--------------------------------IDLE, everything disconnected---------------------------------//	
			IDLE : 			begin
									charge_mode = 1'b0;
									light_power_source = 1'b0;
									charge_on = 1'b0;
									pwm_enable = 1'b0;
									state_number = 3'd0;
									 
									if(low_power) begin
										next_state = IDLE;
									end
									else if (lights_on_flag) begin
										next_state = beacons_panel;
									end
									else if ((~cap_charged)&&(~cap_over5)) begin
										next_state = charge_boost; //add buck
									end
									else if ((~cap_charged)&&(cap_over5)) begin
										next_state = charge_boost;
									end
									else begin 
										next_state = IDLE;
									end	
								end

				
//----------------------Capacitor charging through the Buck SMPS (not in use)---------------//
		
			charge_buck : 	begin // this state is currently not being used due to missing SMPS
									  charge_mode = 1'b0;
									  light_power_source = 1'b0;
									  charge_on = 1'b1;
									  pwm_enable = 1'b0;
									  state_number = 3'd1;
							
									  if (lights_on_flag) begin
										  next_state = beacons_panel;
									  end
									  else if (cap_over5) begin
										  next_state = charge_boost;
									  end	
									  else begin
										  next_state = charge_buck;
									  end
								  end
							  
//---------------capacitor charging through Boost SMPS (only charging state)------------------//							  
							  
			charge_boost : begin
								   charge_mode = 1'b1;
									light_power_source = 1'b0;
									charge_on = 1'b1;
									pwm_enable = 1'b0;
									state_number = 3'd2;
									
									
									if (lights_on_flag) begin
										next_state = beacons_panel;
									end
									else if (cap_charged) begin
										next_state = IDLE;
									end
									else begin
										next_state = charge_boost;
									end
								end
								
//---------------------beacons running from panel power only--------------------------------//								
								
			beacons_panel : begin
								    charge_mode = 1'b0;
									 light_power_source = 1'b0;
									 charge_on = 1'b0;
									 pwm_enable = 1'b1;
									 state_number = 3'd3;
									 
									 if (~lights_on_flag) begin
									    next_state = IDLE;
									 end
									 else if(low_power) begin
										 next_state = IDLE;
									 end
									 else if (~PV_power_high) begin
										 next_state = beacons_cap;
									 end
									 else begin
										 next_state = beacons_panel;
									 end	 
								 end
								 
//---------------------------beacons running from capacitor power only------------------------//
								 
			beacons_cap : begin
							     charge_mode = 1'b0;
								  light_power_source = 1'b1;
								  charge_on = 1'b0;
								  pwm_enable = 1'b1;
								  state_number = 3'd4;
								  
								  if (~lights_on_flag) begin
								     next_state = IDLE;
								  end
								  else if(low_power) begin
									  next_state = IDLE;
								  end
								  else if (PV_power_high) begin
									  next_state = beacons_cap; //edit when implementing extra states
								  end
								  else begin
									  next_state = beacons_cap;
								  end
							  end 
//---------------------------------add extra states here-----------------------------------//

//---------------------------beacons taking power from both sources------------------------//	
//add in future prototypes
			
		endcase			
	end
endmodule				
									  
							
							
						
					
	