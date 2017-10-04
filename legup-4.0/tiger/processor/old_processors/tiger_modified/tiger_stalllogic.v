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

`include "tiger_defines.v"

module tiger_stalllogic (
	input [`CONTROL_WIDTH]controlDe,
	input [`CONTROL_WIDTH]controlEx,
	input [31:0]instrDe,
	input [31:0]instrEx,

	input [`REGNUM_WIDTH] writeRegNumMA,
	input writeRegEnMA,
	input writeRegEnCopMA,

	input [`REGNUM_WIDTH] writeRegNumWB,
	input writeRegEnWB,
	input writeRegEnCopWB,

	input stallRqEx,
	input exception,

	input iStall,
	input dStall,
	input stall_cpu,

	output clearDe, 
	output stallDe,
	output clearEx,
	output stallEx,
	output clearMA,
	output stallMA,
	output clearWB,
	output stallWB
);
	
	//needStallX is high for stage X if that stage
	//needs a stall for some reason, so the stage before
	//it must also stall, and every stage after it
	//must be cleared (introduce bubbles)
	//So we clear a stage if the stage before it needs
	//a stall (needStallX where X is the previous stage is 
	//high) and if the stage itself is not stalled
	//We stall a stage if it needs a stall or if a stage
	//following it is stalled.
					
	wire needStallDe;
	wire needStallEx;
	wire needStallMA;
	wire needStallWB;
	
	assign clearDe = exception && !stallDe;
	assign stallDe = needStallDe || stallEx || iStall || stall_cpu;
	
	assign clearEx = (needStallDe || iStall) && !stallEx;
	assign stallEx = needStallEx || stallMA || stall_cpu;
	
	assign clearMA = needStallEx && !stallMA;
	assign stallMA = needStallMA || stallWB || stall_cpu;
	
	assign clearWB = needStallMA && !stallWB;
	assign stallWB = needStallWB || stall_cpu;
	
	//Is the instruction in the decode stage a eq/ne branch?
	wire takeBranchEqNeDe = controlDe[`CONTROL_BRANCH] &&
							 (controlDe[`CONTROL_BRANCHTYPE]==`BR_EQ || controlDe[`CONTROL_BRANCHTYPE]==`BR_NE);
	
	//Is the instruction in the decode stage any kind of branch or a register jump?
	wire takeBranchOrJumpDe = controlDe[`CONTROL_BRANCH] || controlDe[`CONTROL_REGJUMP];
		
	//Does the instruction in the execute stage want to write to
	//either the rs or the rt registers for the instruction in
	//the decode stage.  If writing to $zero, we don't care, so set to false
	wire rsInE = instrDe[25:21] == controlEx[`CONTROL_WRITEREGNUM] && instrDe[25:21] != 0; 
	wire rtInE = instrDe[20:16] == controlEx[`CONTROL_WRITEREGNUM] && instrDe[20:16] != 0;
	
	//Does the instruction in the memory access stage want to write to
	//either the rs or the rt registers for the instruction in
	//the decode stage.  If writing to $zero, we don't care, so set to false
	wire rsInMA = instrDe[25:21] == writeRegNumMA && instrDe[25:21] != 0; 
	wire rtInMA = instrDe[20:16] == writeRegNumMA && instrDe[20:16] != 0;
	
	//We need a stall in the decode stage
	assign needStallDe = 
		//If we're performing an eq/ne branch and the rt register is in the execute stage or
		//memory access stage (so the stall will cause a wait until it is written back so
		//we can then use them for the branch)
		takeBranchEqNeDe && ((rtInE && controlEx[`CONTROL_REGWRITE]) || (rtInMA && writeRegEnMA))
		|| //Or
		//If we're taking any branch or a register jump and the rs register is in the execute stage
		//or memory access stage (so the stall will cause a wait until it is written back so
		//we can then use them for the branch)
		takeBranchOrJumpDe && ((rsInE && controlEx[`CONTROL_REGWRITE]) || (rsInMA && writeRegEnMA))
		|| //Or
		//If there's a read instruction in execute and it's going to write to
		//a register we need, so we must wait for the read to complete (load stall)
		controlEx[`CONTROL_MEMREAD] && ((rsInE && needsRsAndNotBranch(controlDe)) 
										|| (rtInE && needsRtAndNotBranch(controlDe)))
		|| //Or
		//If in decode there's a coprocessor read instruction and we're writing to the coprocessor
		//futher up the pipeline (not always a hazard, and could have been solved by forwarding,
		//however as we don't read or write from the coprocessor that often we use a small amount
		//of stall logic to handle possible hazards, rather than forwarding or more complex stall
		//logic as the performance benefits of doing so are negligable).
		controlDe[`CONTROL_COPREAD] && (controlEx[`CONTROL_COPWRITE]
										|| writeRegEnCopMA
										|| writeRegEnCopWB);
	
	//We need a stall in the execute stage if the execute stage requests one
	assign needStallEx = stallRqEx;
	//We need a stall in the memory access stage if there is a data stall (i.e.
	//we must wait for the data cache to complete its fetch)
	assign needStallMA = dStall;
	
	//If the execute stage is stalled and write back needs to write a register
	//that execute needs we must stall write back as well otherwise when the 
	//execute stage ceases to be stalled the feedforward from the write back
	//will not give the correct value.  So..
	//We need a stall in the write back stage if
	assign needStallWB = 
		//The write back stage will write to a register, that isn't $zero
		(writeRegEnWB && writeRegNumWB != 5'd0 && //And
			(
				(
					//The instruction in the execute stage needs it for rs
					instrEx[25:21] == writeRegNumWB
					&& needsRsAndNotBranch(controlEx)
				)
				|| //Or
				(	
					//The instruction in the execute stage need it for rt
					instrEx[20:16] == writeRegNumWB
					&& needsRtAndNotBranch(controlEx)
				)
			)
			&& //And
			//Either the execute or memory access stage needs a stall
			//(if MA needs a stall then execute will also be stalled,
			//we can't just use stallEx directly as this creates a loop
			//that will cause a stall that never ends)
			(needStallEx || needStallMA))
		||
		(writeRegEnCopWB && writeRegNumWB == 5'd3 &&
			(/*clearDe ||*/  needStallDe || iStall));
	
	//Are we going to need register rs, given the control signals
	//and it's not a branch
	function needsRsAndNotBranch;
		input [`CONTROL_WIDTH] control;
		begin
			needsRsAndNotBranch = !control[`CONTROL_IRQRETURN]
			&& !control[`CONTROL_JUMP]
			&& !control[`CONTROL_REGJUMP]
			&& !control[`CONTROL_BRANCH];
		end
	endfunction
	
	//Are we going to need register rt, given the control signals
	//and it's not a branch
	function needsRtAndNotBranch;
		input [`CONTROL_WIDTH] control;
		begin
			needsRtAndNotBranch = !control[`CONTROL_IRQRETURN] 
				&& !control[`CONTROL_JUMP] 
				&& !control[`CONTROL_REGJUMP]
				&& !control[`CONTROL_BRANCH]
				&& (!control[`CONTROL_USEIMM] || control[`CONTROL_MEMWRITE] 
					|| control[`CONTROL_MEML] || control[`CONTROL_MEMR]);
		end
	endfunction
endmodule
