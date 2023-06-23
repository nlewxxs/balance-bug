//creator: Luca Mazenod
//Version 1.5
//FSM for solar beacon system
//notes:
//*may be missing a low power state to differentiate between 'idle - charged' and 'idle - low power' implement next
//*unfinished implementation of extra variable "low_power2"
module beacon_FSM (

	input logic clk,
	input logic cap_charged, // not charged 0, charged 1
	input logic cap_over5, // below 5V 0, above 5V 1
	input logic PV_power_high, // 0 low power, 1 high power
	input logic lights_on_flag, // off 0, on 1 
	input logic low_power1, //0 - do not go to state 4
	input logic low_power2, //0 - go to idle and stay
	
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
		
	initial current_state = IDLE; //change this for testing, 
	
	always_comb begin
		case(current_state)
	
//--------------------------------IDLE, everything disconnected---------------------------------//	

			IDLE : 			begin
									charge_mode = 1'b0; // not important here
									light_power_source = 1'b0; //not important here
									charge_on = 1'b0; // disconnect the cap
									pwm_enable = 1'b0; // deactivate the beacons
									state_number = 3'd0;
									 
									if(low_power2) begin //low power so can't charge or power beacons
										next_state = IDLE;
									end
									else if (lights_on_flag) begin //we have power and want the lights to turn on
										next_state = beacons_panel;
									end
									else if ((~cap_charged)&&(~cap_over5)) begin // both statements lead to "if cap is not charged, charge it"
										next_state = charge_boost; //add buck
									end
									else if ((~cap_charged)&&(cap_over5)) begin
										next_state = charge_boost;
									end
									else begin //if we have power, but the cap doesn't need charging nor the beacons need to be on, stay idle
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
								   charge_mode = 1'b1; //not important without buck state
									light_power_source = 1'b0; // not important here
									charge_on = 1'b1; //connect cap
									pwm_enable = 1'b0;// deactivate beacons
									state_number = 3'd2;
									
									
									if (lights_on_flag) begin // if we want the beacons on, go to state 3
										next_state = beacons_panel;
									end
									else if (cap_charged) begin
										next_state = IDLE; //when charged, return to idle
									end
									else begin
										next_state = charge_boost; //if not charged and lights off, keep charging
									end
								end
								
//---------------------beacons running from panel power only--------------------------------//								
								
			beacons_panel : begin
								    charge_mode = 1'b0; //not important 
									 light_power_source = 1'b0; // powered from the boost
									 charge_on = 1'b0; //disconnect the cap
									 pwm_enable = 1'b1; // activate beacons
									 state_number = 3'd3;
									 
									 if (~lights_on_flag) begin // if we want to turn the lights off, go to idle
									    next_state = IDLE;
									 end
									 else if(low_power1) begin // if not enough power, go to idle (protect the drivers)
										 next_state = IDLE;
									 end
									 else if (~PV_power_high) begin // if the panels are not providing enough, use cap if possible
										 if (~low_power2)
											next_state = beacons_cap;
										 else
											next_state = IDLE;
									 end
									 else begin
										 next_state = beacons_panel; // if panel is high power and the lights need to be on, stay here
									 end	 
								 end
								 
//---------------------------beacons running from capacitor power only------------------------//
								 
			beacons_cap : begin
							     charge_mode = 1'b0; //not important
								  light_power_source = 1'b1; // beacons running from cap
								  charge_on = 1'b0; // disconnect boost from cap
								  pwm_enable = 1'b1; //activate beacons
								  state_number = 3'd4;
								  
								  if (~lights_on_flag) begin // if the flag goes off, turn the lights off by going to idle
								     next_state = IDLE;
								  end
								  else if(low_power1) begin // if not enough power to continue, go to idle
									  next_state = IDLE;
								  end
								  else if (PV_power_high) begin //if the panels increase power again, use these to save capacitoer energy
									  next_state = beacons_panel; //edit when implementing extra states
								  end
								  else begin // if pv power is low, cap is still charged and lights are still on, stay here
									  next_state = beacons_cap;
								  end
							  end 
//---------------------------------add extra states here-----------------------------------//

//---------------------------beacons taking power from both sources------------------------//	
//add in future prototypes
			
		endcase			
	end
endmodule				
									  
							
							
						
					
	