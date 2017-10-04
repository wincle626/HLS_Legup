/*
  This work by Simon Moore and Gregory Chadwick is licenced under the
  Creative Commons Attribution-Non-Commercial-Share Alike 2.0
  UK: England & Wales License.

  To view a copy of this licence, visit:
     http://creativecommons.org/licenses/by-nc-sa/2.0/uk/
  or send a letter to:
     Creative Commons,
     171 Second Street,
     Suite 300,
     San Francisco,
     California 94105,
     USA.
*/
module tiger_debug (
	input clk,
	input reset_n,
	input avs_debugSlave_write,
	input avs_debugSlave_writedata,
	output reg avs_debugSlave_irq
);
		
	wire tck, tdi;
	wire sdr;
	
	VJT VJTInst (
		.tdo(1'b0),
		.tdi(tdi),
		.tck(tck),
		.virtual_state_sdr(sdr),
		.ir_out(),
		.ir_in(),
		.virtual_state_cdr(),
		.virtual_state_cir(),
		.virtual_state_e1dr(),
		.virtual_state_e2dr(),
		.virtual_state_pdr(),
		.virtual_state_udr(),
		.virtual_state_uir()
	);
			
	reg lastAssertIrq;
	reg assertIrq;
	reg assertIrqSync;
	reg assertIrqSync2;
	
	reg irqAssertAck;
	reg irqAssertAckSync;
	reg irqAssertAckSync2;
	
				   
	//Avalon bus side clk clock domain
	always @(posedge clk, negedge reset_n) begin
		if(!reset_n) begin
			avs_debugSlave_irq <= 0;
			irqAssertAck <= 0;
			assertIrqSync <= 0;
			assertIrqSync2 <= 0;
		end else begin
			assertIrqSync <= assertIrq;
			assertIrqSync2 <= assertIrqSync;
			lastAssertIrq <= assertIrqSync2;
			
				
			if({assertIrqSync2, lastAssertIrq} == 2'b10) begin
				irqAssertAck <= 1;
				avs_debugSlave_irq <= 1;
			end else begin
				if(avs_debugSlave_write && avs_debugSlave_writedata)
					avs_debugSlave_irq <= 0;	
					
				if(!assertIrqSync2)
					irqAssertAck <= 0;	
			end 
		end
	end
	
	//JTAG Side tck clock domain
	always @(posedge tck, negedge reset_n) begin
		if(!reset_n) begin
			assertIrq <= 0;
			irqAssertAckSync <= 0;
			irqAssertAckSync2 <= 0;
		end else begin
			irqAssertAckSync <= irqAssertAck;
			irqAssertAckSync2 <= irqAssertAckSync;
		
			if(sdr && tdi) begin
				assertIrq <= 1;	
			end else if(irqAssertAckSync2) begin
				assertIrq <= 0;
			end
		end
	end	
				   		  
endmodule
