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

module tiger_ff(
	input [4:0]	regnum,			// register number that we are writing to
				writereg1,		// register number the execute unit wishes to write to
				writereg2,		// register number the MWB unit wishes to write to
	input		writeregen1,	// enable WB for the execute unit
				writeregen2,	// enable WB for the MWB unit
	input [31:0]regdata,		// current contents of the register
				writeregdata1,	// data the execute unit wishes to write to the register
				writeregdata2,	// data the MWB unit wishes to write to the register
	output [31:0]out
);
	//////////////////////////////////////////////////////////////////////////
	// The following code performs the same operation as the line commented 
	// below but synthesises to a better circuit
	//////////////////////////////////////////////////////////////////////////
	
/*  assign out = (regnum == 5'b0) ? 32'b0 : 
			 (regnum == writereg1) && writeregen1 ? writeregdata1 :
			 (regnum == writereg2) && writeregen2 ? writeregdata2 : regdata;
*/

	// check to see if regnum == writereg2 and writeregen2 is enabled
	wire en2;
	tiger_ff_compare c2(regnum,writereg2,writeregen2,en2);
	// if it is enabled, then write the data to the register, otherwise write the current contents of the register
	// back to it (i.e. do not change the contents of the register
	wire [31:0] muxedin2=en2 ? writeregdata2 : regdata;
	
	// check to see if regnum == writereg1 and writeregen1 is enabled
	wire en1;
	tiger_ff_compare c1(regnum,writereg1,writeregen1,en1);
	// if it is enabled, then write the data to the register, otherwise use the result of the MWB checker above
	wire [31:0] muxedin1=en1 ? writeregdata1 : muxedin2;
	
	// check to see if the register number is zero
	// we do not have a 5-input or gate, so use a 4 input and feed
	// the result into a 2-input gate
	wire notzero1;
	or nz1(notzero1,regnum[0],regnum[1],regnum[2],regnum[3]);
	wire notzero;
	or nz(notzero,notzero1,regnum[4]);
	
	// if the register number is zero, the 'and' ensures we return the constant zero, as per the MIPS specification
	// otherwise we return the data determined by the multiplexor
	and a[31:0](out,muxedin1,notzero); 
endmodule

module tiger_ff_compare(
	input [4:0] regnum,		// the register number we are writing to
	input [4:0] writereg,	// the register number we wish to write to
	input		writeregen,	// write-enable signal
	output		en			// output indicating if the write is enabled, and the register that is being
							// written to is the same as the one we would like to write to
);
	///////////////////////////////////////////////////////////////
	// The following code performs the operation below
	// but synthesises to a better circuit on the FPGA
	///////////////////////////////////////////////////////////////
	
	// assign en = (regnum == writereg) && writeregen ? 1'b1 : 1'b0; 
	
	// wires indidicating equality 
	wire eq[5:0];
	wire anded[2:0];
	
	// check to see if bits [1:0] of the register numbers are equal
	xnor e0(eq[0],regnum[0],writereg[0]); 	and a0(anded[0],eq[0],eq[1]);
	xnor e1(eq[1],regnum[1],writereg[1]);

	// check to see if bits [3:2] of the register numbers are equal
	xnor e2(eq[2],regnum[2],writereg[2]);	and a1(anded[1],eq[2],eq[3]);
	xnor e3(eq[3],regnum[3],writereg[3]);

	// check to see if bit [4] of the register numbers is equal
	xnor e4(eq[4],regnum[4],writereg[4]);	and a2(anded[2],eq[4],eq[5]);
	
	// we assign the 6th bit of eq the write-enable signal
	assign eq[5]=writeregen;

	// the write is allowed only if all bits of the register numbers are equal and 
	// the write has been enabled
	and(en,anded[0],anded[1],anded[2]);
endmodule 