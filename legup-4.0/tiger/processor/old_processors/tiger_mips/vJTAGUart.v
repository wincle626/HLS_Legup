module vJTAGUart (
	input clk,
	input reset_n,
	input avs_controlSlave_read,
	input avs_controlSlave_write,
	output [31:0]avs_controlSlave_readdata,
	input [31:0]avs_controlSlave_writedata,
	input avs_controlSlave_address
);
	
	reg [7:0]JTAGDataIn;
	wire JTAGDataInFull;
	wire [8:0]JTAGDataInUsed;
	reg [8:0]JTAGDataInAvailable;
	reg DataInWr;
	
	wire [7:0]JTAGDataOut;
	wire JTAGDataOutFull;
	wire [8:0]JTAGDataOutUsed;
	reg [8:0]JTAGDataOutUsedSaved;
	reg DataOutRd;
	
	wire [1:0]ir;
	wire tck;
	wire tdi;
	wire tdo;
	wire cdr;
	wire sdr;
	wire e1dr;
	reg [3:0]bitCounter;
	
	wire [7:0]AvaDataIn;
	wire [8:0]AvaDataInUsed;
	wire AvaDataInEmpty;
	wire AvaDataInFull;
	wire DataInRd;
	
	wire [8:0]AvaDataOutUsed;
	wire AvaDataOutFull;
	wire DataOutWr;
	
	wire [8:0]AvaDataOutAvailable = AvaDataOutFull ? 9'b0 : 9'd256 - AvaDataOutUsed;
	wire [8:0]AvaDataInUsedCorrected = AvaDataInEmpty ? 9'b0 : 
		AvaDataInFull ? 9'd256 : AvaDataInUsed - 9'b1;
	
	assign avs_controlSlave_readdata = avs_controlSlave_address ? {7'b0, AvaDataOutAvailable, 16'b0} :
		{7'b0, AvaDataInUsedCorrected, !AvaDataInEmpty, 7'b0, AvaDataIn};  
	
	assign DataInRd = avs_controlSlave_read && !avs_controlSlave_address;
	assign DataOutWr = avs_controlSlave_write && !avs_controlSlave_address;
	
	assign tdo = ir == 2'b01 ? JTAGDataInAvailable[bitCounter] : 
		ir == 2'b10 ? JTAGDataOut[bitCounter] : 
		ir == 2'b11 ? JTAGDataOutUsedSaved[bitCounter] : 1'bx;
		
	//JTAG side clock domain
	always @(posedge tck, negedge reset_n) begin
		if(!reset_n) begin
			bitCounter <= 0;
		end else begin
			if(sdr) begin
				case (ir)
					2'b00: begin
						bitCounter <= bitCounter + 1;
						
						if(bitCounter == 4'd6)
							DataInWr <= 1;
						else if(bitCounter == 4'd7)
							bitCounter <= 0;
						
						if(bitCounter != 4'd6)
							DataInWr <= 0;
							
						JTAGDataIn <= {tdi, JTAGDataIn[7:1]};
					end
					2'b01: begin
						bitCounter <= bitCounter + 1;
					end
					2'b10: begin
						bitCounter <= bitCounter + 1;
						
						if(bitCounter == 4'd6)
							DataOutRd <= 1;
						else if(bitCounter == 4'd7)
							bitCounter <= 0;
						
						if(bitCounter != 4'd6)
							DataOutRd <= 0;
					end
					2'b11: begin
						bitCounter <= bitCounter + 1;
					end	
				endcase
			end
			else if(cdr) begin
				case (ir)
					2'b00: begin
						bitCounter <= 0;
					end
					2'b01: begin
						bitCounter <= 0;
						JTAGDataInAvailable <= JTAGDataInFull ? 9'd0 : 9'd256 - JTAGDataInUsed;
					end
					2'b10: begin
						bitCounter <= 0;
					end
					2'b11: begin
						bitCounter <= 0;
						JTAGDataOutUsedSaved <= JTAGDataOutFull ? 9'd256 : JTAGDataOutUsed;
					end
				endcase
			end else begin
				if(DataInWr)
					DataInWr <= 0;
				
				if(DataOutRd)
					DataOutRd <= 0;
			end
		end
	end
	
	FIFO DataIn (
		.data({tdi, JTAGDataIn[7:1]}),
		.rdclk(clk),
		.rdreq(DataInRd),
		.wrclk(tck),
		.wrreq(DataInWr),
		.q(AvaDataIn),
		.rdempty(AvaDataInEmpty),
		.rdfull(AvaDataInFull),
		.wrfull(JTAGDataInFull),
		.rdusedw(AvaDataInUsed),
		.wrusedw(JTAGDataInUsed)
	);
		
	FIFO DataOut (
		.data(avs_controlSlave_writedata[7:0]),
		.rdclk(tck),
		.rdreq(DataOutRd),
		.wrclk(clk),
		.wrreq(DataOutWr),
		.q(JTAGDataOut),
		.wrfull(AvaDataOutFull),
		.rdfull(JTAGDataOutFull),
		.wrusedw(AvaDataOutUsed),
		.rdusedw(JTAGDataOutUsed)
	);
	
	VJT2 VJTIns (
		.tdo(tdo),
		.ir_in(ir),
		.tck(tck),
		.tdi(tdi),
		.virtual_state_cdr(cdr),
		.virtual_state_e1dr(e1dr),
		.virtual_state_sdr(sdr),
		.virtual_state_udr(udr)
	);
endmodule
