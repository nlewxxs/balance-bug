module mppt_signals (

	input logic clk,
	input logic [7:0] V_panel, I_panel, V_cap,
	input logic [2:0] state_number,
	output logic cap_charged, cap_over5, PV_power_high, pwr_low,
	output logic [9:0] val
	
);

	logic [15:0] p_in;
	logic cap_over7, up_flag, down_flag;

//-------------increases/decreases control value depending on comb logic outputs---------//
	initial val = 10'd500;

	always_ff @(posedge clk) begin
			if((state_number == 0)||(state_number == 4)) begin
				val <= val;
			end
			else if (val == 10'd1023) begin
				val <= val - 1;
			end
			else if (val == 10'd0)
				val <= val + 1;
			else if (up_flag) begin
				val <= val + 10'd1;
			end
			else if (down_flag) begin
				val <= val - 10'd1;
			end
			else begin
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
			if(V_cap > 8'd128) begin //tune this value
				cap_charged = 1'b1;
			end
			else begin
				cap_charged = 1'b0;
			end
		end
		
	
//---------------------------------capacitor over 5V, and 7V check-----------------------------//
	
		begin
			if(V_cap > 8'd71) begin //tune this value
				cap_over5 = 1'b1;
			end
			else begin
				cap_over5 = 1'b0;
			end
		end
		
		begin
			if(V_cap > 8'd110) begin //tune this value
				cap_over7 = 1'b1;
			end
			else begin
				cap_over7 = 1'b0;
			end
		end
	
//------------------------------checking input power of PV panels----------------------------//
	
		p_in = I_panel * V_panel; //must convert to milliwatts
		
		begin
			if(p_in > 16'd7000) begin
				PV_power_high = 1'b1;
			end	
			else begin
				PV_power_high = 1'b0;
			end
		end
	
//-------------------------------checking overall power availability--------------------------//
	
		begin
			if((~PV_power_high)&&(~cap_over7)) begin
				pwr_low = 1'b1;
			end
			else begin 
				pwr_low = 1'b0;
			end
		end
		
	end
endmodule
 