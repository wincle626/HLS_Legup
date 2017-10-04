//----------------------------------------------------
// A four level, round-robin arbiter. This was
// orginally coded by WD Peterson in VHDL.
//----------------------------------------------------
`include "cache_parameters.v"

module arbiter (
  clk,    
  rst,    
  req3,   
  address3,
  stallWrite3,
  fetchWrite3,
  req2,   
  address2,
  stallWrite2,
  fetchWrite2,
  req1,   
  address1,
  stallWrite1,
  fetchWrite1,
  req0,   
  address0,
  stallWrite0,
  fetchWrite0,
  gnt3,   
  gnt2,   
  gnt1,   
  gnt0   
);
// --------------Port Declaration----------------------- 
input           clk;    
input           rst;    
input           req3;   
input `MEM_ADDR address3;
input 			stallWrite3;
input			fetchWrite3;
input           req2;
input `MEM_ADDR	address2;
input 			stallWrite2;
input			fetchWrite2;   
input           req1;   
input `MEM_ADDR	address1;
input 			stallWrite1;
input			fetchWrite1;
input           req0;   
input `MEM_ADDR	address0;
input 			stallWrite0;
input			fetchWrite0;
output          gnt3;   
output          gnt2;   
output          gnt1;   
output          gnt0;   


//--------------Code Starts Here----------------------- 


reg lgnt0;
reg lgnt1;
reg lgnt2;
reg lgnt3;
reg stallWrite0_reg;
reg stallWrite1_reg; 
reg stallWrite2_reg; 
reg stallWrite3_reg; 
reg gnt0_reg, gnt1_reg, gnt2_reg, gnt3_reg;


always @(posedge clk)
begin
	gnt0_reg <= gnt0;
	gnt1_reg <= gnt1;
	gnt2_reg <= gnt2;
	gnt3_reg <= gnt3;
	stallWrite0_reg <= stallWrite0;
	stallWrite1_reg <= stallWrite1;
	stallWrite2_reg <= stallWrite2;
	stallWrite3_reg <= stallWrite3;
end

always @ (posedge clk)
if (rst) begin
  lgnt0 <= 0;
  lgnt1 <= 0;
  lgnt2 <= 0;
  lgnt3 <= 0;
end else begin

  lgnt0 <= (~req3 & ~req2 & ~req1 & req0) & !gnt1 & !gnt2 & !gnt3 & !gnt0_reg;
  lgnt1 <= (~req3 & ~req2 & req1) & !gnt0 & !gnt2 & !gnt3 & !gnt1_reg;
  lgnt2 <= (~req3 & req2) & !gnt0 & !gnt1 & !gnt3 & !gnt2_reg;
  lgnt3 <= (req3) & !gnt0 & !gnt1 & !gnt2 & !gnt3_reg;
 
end 


//----------------------------------------------------
// Drive the outputs
//----------------------------------------------------
//Only arbitrate if it falls under these conditions, else drive in req stright to grant
//Always give writes due to fetch priority

assign gnt3   = fetchWrite3? req3 : stallWrite3 || stallWrite3_reg ? lgnt3 : req3;
assign gnt2   = fetchWrite2? req2 : stallWrite2 || stallWrite2_reg ? lgnt2 : req2;
assign gnt1   = fetchWrite1? req1 : stallWrite1 || stallWrite1_reg ? lgnt1 : req1;
assign gnt0   = fetchWrite0? req0 : stallWrite0 || stallWrite0_reg ? lgnt0 : req0;

endmodule
