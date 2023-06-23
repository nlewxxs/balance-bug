module mppt_signals (

	input logic clk,
	input logic [7:0] V_panel, I_panel, V_cap,
	input logic [2:0] state_number,
	output logic cap_charged, cap_over5, PV_power_high, pwr_low1, pwr_low2,
	output logic [9:0] val
	
);

	logic [15:0] p_in;
	logic cap_over7, up_flag, down_flag, cap_over13;

//-------------increases/decreases control value depending on comb logic outputs---------//
	initial val = 10'd500;

	always_ff @(posedge clk) begin
			if((state_number == 0)||(state_number == 4)) begin //val does nothing in these states
				val <= val;
			end
			else if (val == 10'd1023) begin // prevents going past 1023
				val <= val - 10'd1;
			end
			else if (val == 10'd0) // prevents going past 0
				val <= val + 10'd1;
			else if (up_flag) begin //we want val to increase 
				val <= val + 10'd1;
			end
			else if (down_flag) begin //we want val to decrease
				val <= val - 10'd1;
			end
			else begin //default if none of the conditions are met
				val <= val;
			end	
	end

//---------------------------------------------------------------------------------------------//

	always_comb begin

   //control of encoded variable
	
		case (state_number)

//--------------------------------------don't care---------------------------------------------//

				0 :   begin 		
							up_flag = 1'b0;
							down_flag = 1'b0;
						end
						
//------------------------val will be mapped to Buck duty cycle for state 1 ---------------------//

				1 :	begin
							if (V_panel > 8'd190) begin  // increase current
								up_flag = 1'b1;
								down_flag = 1'b0;
							end
							else if (V_panel < 10'd175) begin // decrease current
								up_flag = 1'b0;
								down_flag = 1'b1;
							end
							else begin // within range so do nothing
								up_flag = 1'b0;
								down_flag = 1'b0;
							end
						end
//-----------------------val will be mapped to Boost duty cycle for state 2 -----------------//	
		
				2 : 	begin
							if (V_panel > 8'd190) begin // increase current
								up_flag = 1'b1;
								down_flag = 1'b0;
							end
							else if (V_panel < 8'd175) begin // decrease current
								up_flag = 1'b0;
								down_flag = 1'b1;
							end
							else begin// within range so do nothing
								up_flag = 1'b0;
								down_flag = 1'b0;
							end
						end

					
//--------------val will be mapped to LED driver SMPS duty cycle for state 3------------------//
			
				3 :   begin
							if (V_panel > 8'd190) begin //increase current to drag voltage down
								up_flag = 1'b1;
								down_flag = 1'b0;
							end
							else if (V_panel < 8'd175) begin//decrease current to pull voltage up
								up_flag = 1'b0;
								down_flag = 1'b1;
							end
							else begin// within range so do nothing
								up_flag = 1'b0;
								down_flag = 1'b0;
							end
						end
//-------------------------------------------------------------------------------------------//
				4 :   begin // need to implement tracking - will this be required at all?
							up_flag = 1'b0;
							down_flag = 1'b0;
						end
			
		default : 	begin
							up_flag = 1'b0;
							down_flag = 1'b0;
						end
					
//------------------------------- look at this further-----------------------------------------//	
		endcase
	end
	
//---------------------------------capcitor charged condition--------------------------------//
	always_comb begin

		begin
			if(V_cap > 8'd227) begin //tune this value
				cap_charged = 1'b1;
			end
			else begin
				cap_charged = 1'b0;
			end
		end
		
	
//---------------------------------capacitor over 5V, and 7V check-----------------------------//
//note: 5V is not needed without the Buck state, also need to add a capacitor 
//threshold, will need to be as high as 12-13V otherwise the capacitor will go below 7V 
//quickly and the beacons will not be on for long enough 
		begin
			if(V_cap > 8'd71) begin //tune this value (5V)
				cap_over5 = 1'b1;
			end
			else begin
				cap_over5 = 1'b0;
			end
		end
		
		begin
			if(V_cap > 8'd110) begin //tune this value (7V)
				cap_over7 = 1'b1;
			end
			else begin
				cap_over7 = 1'b0;
			end
		end
		
		begin
			if(V_cap > 8'd184) begin //tune this value (13V)
				cap_over13 = 1'b1;
			end
			else begin
				cap_over13 = 1'b0;
			end
		end
			
	
//------------------------------checking input power of PV panels----------------------------//
	
		p_in = I_panel * V_panel; 
		
		begin
			if(p_in > 16'd6000) begin // tune this
				PV_power_high = 1'b1;
			end	
			else begin
				PV_power_high = 1'b0;
			end
		end
	
//-------------------------------checking overall power availability--------------------------//
	
		begin
			if((~PV_power_high)&&(~cap_over7)) begin //if the panels are low power and the cap voltage is below the threshold
				pwr_low1 = 1'b1;
			end
			else begin 
				pwr_low1 = 1'b0;
			end
		end
		
		begin
			if((~PV_power_high)&&(~cap_over13)) begin 
				pwr_low2 = 1'b1;
			end
			else begin 
				pwr_low2 = 1'b0;
			end
		end
		
	end
endmodule
 