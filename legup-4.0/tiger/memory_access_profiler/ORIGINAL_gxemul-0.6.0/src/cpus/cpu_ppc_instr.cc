/*
 *  Copyright (C) 2005-2009  Anders Gavare.  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright  
 *     notice, this list of conditions and the following disclaimer in the 
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE   
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 *
 *
 *  POWER/PowerPC instructions.
 *
 *  Individual functions should keep track of cpu->n_translated_instrs.
 *  (If no instruction was executed, then it should be decreased. If, say, 4
 *  instructions were combined into one function and executed, then it should
 *  be increased by 3.)
 */


#include "float_emul.h"


#define DOT0(n) X(n ## _dot) { instr(n)(cpu,ic); \
	update_cr0(cpu, reg(ic->arg[0])); }
#define DOT1(n) X(n ## _dot) { instr(n)(cpu,ic); \
	update_cr0(cpu, reg(ic->arg[1])); }
#define DOT2(n) X(n ## _dot) { instr(n)(cpu,ic); \
	update_cr0(cpu, reg(ic->arg[2])); }

#ifndef CHECK_FOR_FPU_EXCEPTION
#define CHECK_FOR_FPU_EXCEPTION { if (!(cpu->cd.ppc.msr & PPC_MSR_FP)) { \
		/*  Synchronize the PC, and cause an FPU exception:  */  \
		uint64_t low_pc = ((size_t)ic -				 \
		    (size_t)cpu->cd.ppc.cur_ic_page)			 \
		    / sizeof(struct ppc_instr_call);			 \
		cpu->pc = (cpu->pc & ~((PPC_IC_ENTRIES_PER_PAGE-1) <<	 \
		    PPC_INSTR_ALIGNMENT_SHIFT)) + (low_pc <<		 \
		    PPC_INSTR_ALIGNMENT_SHIFT);				 \
		ppc_exception(cpu, PPC_EXCEPTION_FPU);			 \
		return; } }
#endif



/*
 *  nop:  Do nothing.
 */
X(nop)
{
}


/*
 *  invalid:  To catch bugs.
 */
X(invalid)
{
	fatal("PPC: invalid(): INTERNAL ERROR\n");
	exit(1);
}


/*
 *  addi:  Add immediate.
 *
 *  arg[0] = pointer to source uint64_t
 *  arg[1] = immediate value (int32_t or larger)
 *  arg[2] = pointer to destination uint64_t
 */
X(addi)
{
	reg(ic->arg[2]) = reg(ic->arg[0]) + (int32_t)ic->arg[1];
}
X(li)
{
	reg(ic->arg[2]) = (int32_t)ic->arg[1];
}
X(li_0)
{
	reg(ic->arg[2]) = 0;
}


/*
 *  andi_dot:  AND immediate, update CR.
 *
 *  arg[0] = pointer to source uint64_t
 *  arg[1] = immediate value (uint32_t)
 *  arg[2] = pointer to destination uint64_t
 */
X(andi_dot)
{
	MODE_uint_t tmp = reg(ic->arg[0]) & (uint32_t)ic->arg[1];
	reg(ic->arg[2]) = tmp;
	update_cr0(cpu, tmp);
}


/*
 *  addic:  Add immediate, Carry.
 *
 *  arg[0] = pointer to source register
 *  arg[1] = immediate value (int32_t or larger)
 *  arg[2] = pointer to destination register
 */
X(addic)
{
	/*  TODO/NOTE: Only for 32-bit mode, so far!  */
	uint64_t tmp = (uint32_t)reg(ic->arg[0]);
	uint64_t tmp2 = tmp;
	cpu->cd.ppc.spr[SPR_XER] &= ~PPC_XER_CA;
	tmp2 += (uint32_t)ic->arg[1];
	if ((tmp2 >> 32) != (tmp >> 32))
		cpu->cd.ppc.spr[SPR_XER] |= PPC_XER_CA;
	reg(ic->arg[2]) = (uint32_t)tmp2;
}


/*
 *  subfic:  Subtract from immediate, Carry.
 *
 *  arg[0] = pointer to source uint64_t
 *  arg[1] = immediate value (int32_t or larger)
 *  arg[2] = pointer to destination uint64_t
 */
X(subfic)
{
	MODE_uint_t tmp = (int64_t)(int32_t)ic->arg[1];
	cpu->cd.ppc.spr[SPR_XER] &= ~PPC_XER_CA;
	if (tmp >= reg(ic->arg[0]))
		cpu->cd.ppc.spr[SPR_XER] |= PPC_XER_CA;
	reg(ic->arg[2]) = tmp - reg(ic->arg[0]);
}


/*
 *  addic_dot:  Add immediate, Carry.
 *
 *  arg[0] = pointer to source uint64_t
 *  arg[1] = immediate value (int32_t or larger)
 *  arg[2] = pointer to destination uint64_t
 */
X(addic_dot)
{
	/*  TODO/NOTE: Only for 32-bit mode, so far!  */
	uint64_t tmp = (uint32_t)reg(ic->arg[0]);
	uint64_t tmp2 = tmp;
	cpu->cd.ppc.spr[SPR_XER] &= ~PPC_XER_CA;
	tmp2 += (uint32_t)ic->arg[1];
	if ((tmp2 >> 32) != (tmp >> 32))
		cpu->cd.ppc.spr[SPR_XER] |= PPC_XER_CA;
	reg(ic->arg[2]) = (uint32_t)tmp2;
	update_cr0(cpu, (uint32_t)tmp2);
}


/*
 *  bclr:  Branch Conditional to Link Register
 *
 *  arg[0] = bo
 *  arg[1] = 31 - bi
 *  arg[2] = bh
 */
X(bclr)
{
	unsigned int bo = ic->arg[0], bi31m = ic->arg[1];
	int ctr_ok, cond_ok;
	uint64_t old_pc = cpu->pc;
	MODE_uint_t tmp, addr = cpu->cd.ppc.spr[SPR_LR];
	if (!(bo & 4))
		cpu->cd.ppc.spr[SPR_CTR] --;
	ctr_ok = (bo >> 2) & 1;
	tmp = cpu->cd.ppc.spr[SPR_CTR];
	ctr_ok |= ( (tmp == 0) == ((bo >> 1) & 1) );
	cond_ok = (bo >> 4) & 1;
	cond_ok |= ( ((bo >> 3) & 1) == ((cpu->cd.ppc.cr >> bi31m) & 1) );
	if (ctr_ok && cond_ok) {
		uint64_t mask_within_page =
		    ((PPC_IC_ENTRIES_PER_PAGE-1) << PPC_INSTR_ALIGNMENT_SHIFT)
		    | ((1 << PPC_INSTR_ALIGNMENT_SHIFT) - 1);
		cpu->pc = addr & ~((1 << PPC_INSTR_ALIGNMENT_SHIFT) - 1);
		/*  TODO: trace in separate (duplicate) function?  */
		if (cpu->machine->show_trace_tree)
			cpu_functioncall_trace_return(cpu);
		if ((old_pc  & ~mask_within_page) ==
		    (cpu->pc & ~mask_within_page)) {
			cpu->cd.ppc.next_ic =
			    cpu->cd.ppc.cur_ic_page +
			    ((cpu->pc & mask_within_page) >>
			    PPC_INSTR_ALIGNMENT_SHIFT);
		} else {
			/*  Find the new physical page and update pointers:  */
			quick_pc_to_pointers(cpu);
		}
	}
}
X(bclr_20)
{
	cpu->pc = cpu->cd.ppc.spr[SPR_LR];
	quick_pc_to_pointers(cpu);
}
X(bclr_l)
{
	uint64_t low_pc, old_pc = cpu->pc;
	unsigned int bo = ic->arg[0], bi31m = ic->arg[1]  /* ,bh = ic->arg[2]*/;
	int ctr_ok, cond_ok;
	MODE_uint_t tmp, addr = cpu->cd.ppc.spr[SPR_LR];
	if (!(bo & 4))
		cpu->cd.ppc.spr[SPR_CTR] --;
	ctr_ok = (bo >> 2) & 1;
	tmp = cpu->cd.ppc.spr[SPR_CTR];
	ctr_ok |= ( (tmp == 0) == ((bo >> 1) & 1) );
	cond_ok = (bo >> 4) & 1;
	cond_ok |= ( ((bo >> 3) & 1) == ((cpu->cd.ppc.cr >> bi31m) & 1) );

	/*  Calculate return PC:  */
	low_pc = ((size_t)ic - (size_t)
	    cpu->cd.ppc.cur_ic_page) / sizeof(struct ppc_instr_call) + 1;
	cpu->cd.ppc.spr[SPR_LR] = cpu->pc & ~((PPC_IC_ENTRIES_PER_PAGE-1)
	    << PPC_INSTR_ALIGNMENT_SHIFT);
	cpu->cd.ppc.spr[SPR_LR] += (low_pc << PPC_INSTR_ALIGNMENT_SHIFT);

	if (ctr_ok && cond_ok) {
		uint64_t mask_within_page =
		    ((PPC_IC_ENTRIES_PER_PAGE-1) << PPC_INSTR_ALIGNMENT_SHIFT)
		    | ((1 << PPC_INSTR_ALIGNMENT_SHIFT) - 1);
		cpu->pc = addr & ~((1 << PPC_INSTR_ALIGNMENT_SHIFT) - 1);
		/*  TODO: trace in separate (duplicate) function?  */
		if (cpu->machine->show_trace_tree)
			cpu_functioncall_trace_return(cpu);
		if (cpu->machine->show_trace_tree)
			cpu_functioncall_trace(cpu, cpu->pc);
		if ((old_pc  & ~mask_within_page) ==
		    (cpu->pc & ~mask_within_page)) {
			cpu->cd.ppc.next_ic =
			    cpu->cd.ppc.cur_ic_page +
			    ((cpu->pc & mask_within_page) >>
			    PPC_INSTR_ALIGNMENT_SHIFT);
		} else {
			/*  Find the new physical page and update pointers:  */
			quick_pc_to_pointers(cpu);
		}
	}
}


/*
 *  bcctr:  Branch Conditional to Count register
 *
 *  arg[0] = bo
 *  arg[1] = 31 - bi
 *  arg[2] = bh
 */
X(bcctr)
{
	unsigned int bo = ic->arg[0], bi31m = ic->arg[1];
	uint64_t old_pc = cpu->pc;
	MODE_uint_t addr = cpu->cd.ppc.spr[SPR_CTR];
	int cond_ok = (bo >> 4) & 1;
	cond_ok |= ( ((bo >> 3) & 1) == ((cpu->cd.ppc.cr >> bi31m) & 1) );
	if (cond_ok) {
		uint64_t mask_within_page =
		    ((PPC_IC_ENTRIES_PER_PAGE-1) << PPC_INSTR_ALIGNMENT_SHIFT)
		    | ((1 << PPC_INSTR_ALIGNMENT_SHIFT) - 1);
		cpu->pc = addr & ~((1 << PPC_INSTR_ALIGNMENT_SHIFT) - 1);
		/*  TODO: trace in separate (duplicate) function?  */
		if (cpu->machine->show_trace_tree)
			cpu_functioncall_trace_return(cpu);
		if ((old_pc  & ~mask_within_page) ==
		    (cpu->pc & ~mask_within_page)) {
			cpu->cd.ppc.next_ic =
			    cpu->cd.ppc.cur_ic_page +
			    ((cpu->pc & mask_within_page) >>
			    PPC_INSTR_ALIGNMENT_SHIFT);
		} else {
			/*  Find the new physical page and update pointers:  */
			quick_pc_to_pointers(cpu);
		}
	}
}
X(bcctr_l)
{
	uint64_t low_pc, old_pc = cpu->pc;
	unsigned int bo = ic->arg[0], bi31m = ic->arg[1]  /*,bh = ic->arg[2] */;
	MODE_uint_t addr = cpu->cd.ppc.spr[SPR_CTR];
	int cond_ok = (bo >> 4) & 1;
	cond_ok |= ( ((bo >> 3) & 1) == ((cpu->cd.ppc.cr >> bi31m) & 1) );

	/*  Calculate return PC:  */
	low_pc = ((size_t)ic - (size_t)
	    cpu->cd.ppc.cur_ic_page) / sizeof(struct ppc_instr_call) + 1;
	cpu->cd.ppc.spr[SPR_LR] = cpu->pc & ~((PPC_IC_ENTRIES_PER_PAGE-1)
	    << PPC_INSTR_ALIGNMENT_SHIFT);
	cpu->cd.ppc.spr[SPR_LR] += (low_pc << PPC_INSTR_ALIGNMENT_SHIFT);

	if (cond_ok) {
		uint64_t mask_within_page =
		    ((PPC_IC_ENTRIES_PER_PAGE-1) << PPC_INSTR_ALIGNMENT_SHIFT)
		    | ((1 << PPC_INSTR_ALIGNMENT_SHIFT) - 1);
		cpu->pc = addr & ~((1 << PPC_INSTR_ALIGNMENT_SHIFT) - 1);
		/*  TODO: trace in separate (duplicate) function?  */
		if (cpu->machine->show_trace_tree)
			cpu_functioncall_trace(cpu, cpu->pc);
		if ((old_pc  & ~mask_within_page) ==
		    (cpu->pc & ~mask_within_page)) {
			cpu->cd.ppc.next_ic =
			    cpu->cd.ppc.cur_ic_page +
			    ((cpu->pc & mask_within_page) >>
			    PPC_INSTR_ALIGNMENT_SHIFT);
		} else {
			/*  Find the new physical page and update pointers:  */
			quick_pc_to_pointers(cpu);
		}
	}
}


/*
 *  b:  Branch (to a different translated page)
 *
 *  arg[0] = relative offset (as an int32_t) from start of page
 */
X(b)
{
	cpu->pc &= ~((PPC_IC_ENTRIES_PER_PAGE-1) << PPC_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (int32_t)ic->arg[0];

	/*  Find the new physical page and update the translation pointers:  */
	quick_pc_to_pointers(cpu);
}
X(ba)
{
	cpu->pc = (int32_t)ic->arg[0];
	quick_pc_to_pointers(cpu);
}


/*
 *  bc:  Branch Conditional (to a different translated page)
 *
 *  arg[0] = relative offset (as an int32_t) from start of page
 *  arg[1] = bo
 *  arg[2] = 31-bi
 */
X(bc)
{
	MODE_uint_t tmp;
	unsigned int ctr_ok, cond_ok, bi31m = ic->arg[2], bo = ic->arg[1];
	if (!(bo & 4))
		cpu->cd.ppc.spr[SPR_CTR] --;
	ctr_ok = (bo >> 2) & 1;
	tmp = cpu->cd.ppc.spr[SPR_CTR];
	ctr_ok |= ( (tmp == 0) == ((bo >> 1) & 1) );
	cond_ok = (bo >> 4) & 1;
	cond_ok |= ( ((bo >> 3) & 1) ==
	    ((cpu->cd.ppc.cr >> (bi31m)) & 1)  );
	if (ctr_ok && cond_ok)
		instr(b)(cpu,ic);
}
X(bcl)
{
	MODE_uint_t tmp;
	unsigned int ctr_ok, cond_ok, bi31m = ic->arg[2], bo = ic->arg[1];
	int low_pc;

	/*  Calculate LR:  */
	low_pc = ((size_t)ic - (size_t)
	    cpu->cd.ppc.cur_ic_page) / sizeof(struct ppc_instr_call) + 1;
	cpu->cd.ppc.spr[SPR_LR] = cpu->pc & ~((PPC_IC_ENTRIES_PER_PAGE-1)
	    << PPC_INSTR_ALIGNMENT_SHIFT);
	cpu->cd.ppc.spr[SPR_LR] += (low_pc << PPC_INSTR_ALIGNMENT_SHIFT);

	if (!(bo & 4))
		cpu->cd.ppc.spr[SPR_CTR] --;
	ctr_ok = (bo >> 2) & 1;
	tmp = cpu->cd.ppc.spr[SPR_CTR];
	ctr_ok |= ( (tmp == 0) == ((bo >> 1) & 1) );
	cond_ok = (bo >> 4) & 1;
	cond_ok |= ( ((bo >> 3) & 1) ==
	    ((cpu->cd.ppc.cr >> bi31m) & 1)  );
	if (ctr_ok && cond_ok)
		instr(b)(cpu,ic);
}


/*
 *  b_samepage:  Branch (to within the same translated page)
 *
 *  arg[0] = pointer to new ppc_instr_call
 */
X(b_samepage)
{
	cpu->cd.ppc.next_ic = (struct ppc_instr_call *) ic->arg[0];
}


/*
 *  bc_samepage:  Branch Conditional (to within the same page)
 *
 *  arg[0] = new ic ptr
 *  arg[1] = bo
 *  arg[2] = 31-bi
 */
X(bc_samepage)
{
	MODE_uint_t tmp;
	unsigned int ctr_ok, cond_ok, bi31m = ic->arg[2], bo = ic->arg[1];
	if (!(bo & 4))
		cpu->cd.ppc.spr[SPR_CTR] --;
	ctr_ok = (bo >> 2) & 1;
	tmp = cpu->cd.ppc.spr[SPR_CTR];
	ctr_ok |= ( (tmp == 0) == ((bo >> 1) & 1) );
	cond_ok = (bo >> 4) & 1;
	cond_ok |= ( ((bo >> 3) & 1) ==
	    ((cpu->cd.ppc.cr >> bi31m) & 1)  );
	if (ctr_ok && cond_ok)
		cpu->cd.ppc.next_ic = (struct ppc_instr_call *) ic->arg[0];
}
X(bc_samepage_simple0)
{
	int bi31m = ic->arg[2];
	if (!((cpu->cd.ppc.cr >> bi31m) & 1))
		cpu->cd.ppc.next_ic = (struct ppc_instr_call *) ic->arg[0];
}
X(bc_samepage_simple1)
{
	int bi31m = ic->arg[2];
	if ((cpu->cd.ppc.cr >> bi31m) & 1)
		cpu->cd.ppc.next_ic = (struct ppc_instr_call *) ic->arg[0];
}
X(bcl_samepage)
{
	MODE_uint_t tmp;
	unsigned int ctr_ok, cond_ok, bi31m = ic->arg[2], bo = ic->arg[1];
	int low_pc;

	/*  Calculate LR:  */
	low_pc = ((size_t)ic - (size_t)
	    cpu->cd.ppc.cur_ic_page) / sizeof(struct ppc_instr_call) + 1;
	cpu->cd.ppc.spr[SPR_LR] = cpu->pc & ~((PPC_IC_ENTRIES_PER_PAGE-1)
	    << PPC_INSTR_ALIGNMENT_SHIFT);
	cpu->cd.ppc.spr[SPR_LR] += (low_pc << PPC_INSTR_ALIGNMENT_SHIFT);

	if (!(bo & 4))
		cpu->cd.ppc.spr[SPR_CTR] --;
	ctr_ok = (bo >> 2) & 1;
	tmp = cpu->cd.ppc.spr[SPR_CTR];
	ctr_ok |= ( (tmp == 0) == ((bo >> 1) & 1) );
	cond_ok = (bo >> 4) & 1;
	cond_ok |= ( ((bo >> 3) & 1) ==
	    ((cpu->cd.ppc.cr >> bi31m) & 1)  );
	if (ctr_ok && cond_ok)
		cpu->cd.ppc.next_ic = (struct ppc_instr_call *) ic->arg[0];
}


/*
 *  bl:  Branch and Link (to a different translated page)
 *
 *  arg[0] = relative offset (as an int32_t) from start of page
 *  arg[1] = lr offset (relative to start of current page)
 */
X(bl)
{
	/*  Calculate LR and new PC:  */
	cpu->pc &= ~((PPC_IC_ENTRIES_PER_PAGE-1) << PPC_INSTR_ALIGNMENT_SHIFT);
	cpu->cd.ppc.spr[SPR_LR] = cpu->pc + ic->arg[1];
	cpu->pc += (int32_t)ic->arg[0];

	/*  Find the new physical page and update the translation pointers:  */
	quick_pc_to_pointers(cpu);
}
X(bla)
{
	/*  Calculate LR:  */
	cpu->cd.ppc.spr[SPR_LR] = (cpu->pc & ~((PPC_IC_ENTRIES_PER_PAGE-1) 
	    << PPC_INSTR_ALIGNMENT_SHIFT)) + ic->arg[1];

	cpu->pc = (int32_t)ic->arg[0];
	quick_pc_to_pointers(cpu);
}


/*
 *  bl_trace:  Branch and Link (to a different translated page)  (with trace)
 *
 *  arg[0] = relative offset (as an int32_t) from start of page
 *  arg[1] = lr offset (relative to start of current page)
 */
X(bl_trace)
{
	/*  Calculate LR:  */
	cpu->cd.ppc.spr[SPR_LR] = (cpu->pc & ~((PPC_IC_ENTRIES_PER_PAGE-1) 
	    << PPC_INSTR_ALIGNMENT_SHIFT)) + ic->arg[1];

	/*  Calculate new PC from start of page + arg[0]  */
	cpu->pc &= ~((PPC_IC_ENTRIES_PER_PAGE-1) << PPC_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (int32_t)ic->arg[0];

	cpu_functioncall_trace(cpu, cpu->pc);

	/*  Find the new physical page and update the translation pointers:  */
	quick_pc_to_pointers(cpu);
}
X(bla_trace)
{
	/*  Calculate LR:  */
	cpu->cd.ppc.spr[SPR_LR] = (cpu->pc & ~((PPC_IC_ENTRIES_PER_PAGE-1) 
	    << PPC_INSTR_ALIGNMENT_SHIFT)) + ic->arg[1];

	cpu->pc = (int32_t)ic->arg[0];
	cpu_functioncall_trace(cpu, cpu->pc);
	quick_pc_to_pointers(cpu);
}


/*
 *  bl_samepage:  Branch and Link (to within the same translated page)
 *
 *  arg[0] = pointer to new ppc_instr_call
 *  arg[1] = lr offset (relative to start of current page)
 */
X(bl_samepage)
{
	/*  Calculate LR:  */
	cpu->cd.ppc.spr[SPR_LR] = (cpu->pc & ~((PPC_IC_ENTRIES_PER_PAGE-1) 
	    << PPC_INSTR_ALIGNMENT_SHIFT)) + ic->arg[1];

	cpu->cd.ppc.next_ic = (struct ppc_instr_call *) ic->arg[0];
}


/*
 *  bl_samepage_trace:  Branch and Link (to within the same translated page)
 *
 *  arg[0] = pointer to new ppc_instr_call
 *  arg[1] = lr offset (relative to start of current page)
 */
X(bl_samepage_trace)
{
	uint32_t low_pc;

	/*  Calculate LR:  */
	cpu->cd.ppc.spr[SPR_LR] = (cpu->pc & ~((PPC_IC_ENTRIES_PER_PAGE-1) 
	    << PPC_INSTR_ALIGNMENT_SHIFT)) + ic->arg[1];

	cpu->cd.ppc.next_ic = (struct ppc_instr_call *) ic->arg[0];

	/*  Calculate new PC (for the trace)  */
	low_pc = ((size_t)cpu->cd.ppc.next_ic - (size_t)
	    cpu->cd.ppc.cur_ic_page) / sizeof(struct ppc_instr_call);
	cpu->pc &= ~((PPC_IC_ENTRIES_PER_PAGE-1) << PPC_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << PPC_INSTR_ALIGNMENT_SHIFT);
	cpu_functioncall_trace(cpu, cpu->pc);
}


/*
 *  cntlzw:  Count leading zeroes (32-bit word).
 *
 *  arg[0] = ptr to rs
 *  arg[1] = ptr to ra
 */
X(cntlzw)
{
	uint32_t tmp = reg(ic->arg[0]);
	int i;
	for (i=0; i<32; i++) {
		if (tmp & 0x80000000)
			break;
		tmp <<= 1;
	}
	reg(ic->arg[1]) = i;
}


/*
 *  cmpd:  Compare Doubleword
 *
 *  arg[0] = ptr to ra
 *  arg[1] = ptr to rb
 *  arg[2] = 28 - 4*bf
 */
X(cmpd)
{
	int64_t tmp = reg(ic->arg[0]), tmp2 = reg(ic->arg[1]);
	int bf_shift = ic->arg[2], c;
	if (tmp < tmp2)
		c = 8;
	else if (tmp > tmp2)
		c = 4;
	else
		c = 2;
	/*  SO bit, copied from XER  */
	c |= ((cpu->cd.ppc.spr[SPR_XER] >> 31) & 1);
	cpu->cd.ppc.cr &= ~(0xf << bf_shift);
	cpu->cd.ppc.cr |= (c << bf_shift);
}


/*
 *  cmpld:  Compare Doubleword, unsigned
 *
 *  arg[0] = ptr to ra
 *  arg[1] = ptr to rb
 *  arg[2] = 28 - 4*bf
 */
X(cmpld)
{
	uint64_t tmp = reg(ic->arg[0]), tmp2 = reg(ic->arg[1]);
	int bf_shift = ic->arg[2], c;
	if (tmp < tmp2)
		c = 8;
	else if (tmp > tmp2)
		c = 4;
	else
		c = 2;
	/*  SO bit, copied from XER  */
	c |= ((cpu->cd.ppc.spr[SPR_XER] >> 31) & 1);
	cpu->cd.ppc.cr &= ~(0xf << bf_shift);
	cpu->cd.ppc.cr |= (c << bf_shift);
}


/*
 *  cmpdi:  Compare Doubleword immediate
 *
 *  arg[0] = ptr to ra
 *  arg[1] = int32_t imm
 *  arg[2] = 28 - 4*bf
 */
X(cmpdi)
{
	int64_t tmp = reg(ic->arg[0]), imm = (int32_t)ic->arg[1];
	int bf_shift = ic->arg[2], c;
	if (tmp < imm)
		c = 8;
	else if (tmp > imm)
		c = 4;
	else
		c = 2;
	/*  SO bit, copied from XER  */
	c |= ((cpu->cd.ppc.spr[SPR_XER] >> 31) & 1);
	cpu->cd.ppc.cr &= ~(0xf << bf_shift);
	cpu->cd.ppc.cr |= (c << bf_shift);
}


/*
 *  cmpldi:  Compare Doubleword immediate, logical
 *
 *  arg[0] = ptr to ra
 *  arg[1] = int32_t imm
 *  arg[2] = 28 - 4*bf
 */
X(cmpldi)
{
	uint64_t tmp = reg(ic->arg[0]), imm = (uint32_t)ic->arg[1];
	int bf_shift = ic->arg[2], c;
	if (tmp < imm)
		c = 8;
	else if (tmp > imm)
		c = 4;
	else
		c = 2;
	/*  SO bit, copied from XER  */
	c |= ((cpu->cd.ppc.spr[SPR_XER] >> 31) & 1);
	cpu->cd.ppc.cr &= ~(0xf << bf_shift);
	cpu->cd.ppc.cr |= (c << bf_shift);
}


/*
 *  cmpw:  Compare Word
 *
 *  arg[0] = ptr to ra
 *  arg[1] = ptr to rb
 *  arg[2] = 28 - 4*bf
 */
X(cmpw)
{
	int32_t tmp = reg(ic->arg[0]), tmp2 = reg(ic->arg[1]);
	int bf_shift = ic->arg[2], c;
	if (tmp < tmp2)
		c = 8;
	else if (tmp > tmp2)
		c = 4;
	else
		c = 2;
	/*  SO bit, copied from XER  */
	c |= ((cpu->cd.ppc.spr[SPR_XER] >> 31) & 1);
	cpu->cd.ppc.cr &= ~(0xf << bf_shift);
	cpu->cd.ppc.cr |= (c << bf_shift);
}
X(cmpw_cr0)
{
	/*  arg[2] is assumed to be 28  */
	int32_t tmp = reg(ic->arg[0]), tmp2 = reg(ic->arg[1]);
	cpu->cd.ppc.cr &= ~(0xf0000000);
	if (tmp < tmp2)
		cpu->cd.ppc.cr |= 0x80000000;
	else if (tmp > tmp2)
		cpu->cd.ppc.cr |= 0x40000000;
	else
		cpu->cd.ppc.cr |= 0x20000000;
	cpu->cd.ppc.cr |= ((cpu->cd.ppc.spr[SPR_XER] >> 3) & 0x10000000);
}


/*
 *  cmplw:  Compare Word, unsigned
 *
 *  arg[0] = ptr to ra
 *  arg[1] = ptr to rb
 *  arg[2] = 28 - 4*bf
 */
X(cmplw)
{
	uint32_t tmp = reg(ic->arg[0]), tmp2 = reg(ic->arg[1]);
	int bf_shift = ic->arg[2], c;
	if (tmp < tmp2)
		c = 8;
	else if (tmp > tmp2)
		c = 4;
	else
		c = 2;
	/*  SO bit, copied from XER  */
	c |= ((cpu->cd.ppc.spr[SPR_XER] >> 31) & 1);
	cpu->cd.ppc.cr &= ~(0xf << bf_shift);
	cpu->cd.ppc.cr |= (c << bf_shift);
}


/*
 *  cmpwi:  Compare Word immediate
 *
 *  arg[0] = ptr to ra
 *  arg[1] = int32_t imm
 *  arg[2] = 28 - 4*bf
 */
X(cmpwi)
{
	int32_t tmp = reg(ic->arg[0]), imm = ic->arg[1];
	int bf_shift = ic->arg[2], c;
	if (tmp < imm)
		c = 8;
	else if (tmp > imm)
		c = 4;
	else
		c = 2;
	/*  SO bit, copied from XER  */
	c |= ((cpu->cd.ppc.spr[SPR_XER] >> 31) & 1);
	cpu->cd.ppc.cr &= ~(0xf << bf_shift);
	cpu->cd.ppc.cr |= (c << bf_shift);
}
X(cmpwi_cr0)
{
	/*  arg[2] is assumed to be 28  */
	int32_t tmp = reg(ic->arg[0]), imm = ic->arg[1];
	cpu->cd.ppc.cr &= ~(0xf0000000);
	if (tmp < imm)
		cpu->cd.ppc.cr |= 0x80000000;
	else if (tmp > imm)
		cpu->cd.ppc.cr |= 0x40000000;
	else
		cpu->cd.ppc.cr |= 0x20000000;
	cpu->cd.ppc.cr |= ((cpu->cd.ppc.spr[SPR_XER] >> 3) & 0x10000000);
}


/*
 *  cmplwi:  Compare Word immediate, logical
 *
 *  arg[0] = ptr to ra
 *  arg[1] = int32_t imm
 *  arg[2] = 28 - 4*bf
 */
X(cmplwi)
{
	uint32_t tmp = reg(ic->arg[0]), imm = ic->arg[1];
	int bf_shift = ic->arg[2], c;
	if (tmp < imm)
		c = 8;
	else if (tmp > imm)
		c = 4;
	else
		c = 2;
	/*  SO bit, copied from XER  */
	c |= ((cpu->cd.ppc.spr[SPR_XER] >> 31) & 1);
	cpu->cd.ppc.cr &= ~(0xf << bf_shift);
	cpu->cd.ppc.cr |= (c << bf_shift);
}


/*
 *  dcbz:  Data-Cache Block Zero
 *
 *  arg[0] = ptr to ra (or zero)
 *  arg[1] = ptr to rb
 */
X(dcbz)
{
	MODE_uint_t addr = reg(ic->arg[0]) + reg(ic->arg[1]);
	unsigned char cacheline[128];
	size_t cacheline_size = 1 << cpu->cd.ppc.cpu_type.dlinesize;
	size_t cleared = 0;

	/*  Synchronize the PC first:  */
	cpu->pc = (cpu->pc & ~0xfff) + ic->arg[2];

	addr &= ~(cacheline_size - 1);
	memset(cacheline, 0, sizeof(cacheline));

	while (cleared < cacheline_size) {
		int to_clear = cacheline_size < sizeof(cacheline)?
		    cacheline_size : sizeof(cacheline);
#ifdef MODE32
		unsigned char *page = cpu->cd.ppc.host_store[addr >> 12];
		if (page != NULL) {
			memset(page + (addr & 0xfff), 0, to_clear);
		} else
#endif
		if (cpu->memory_rw(cpu, cpu->mem, addr, cacheline,
		    to_clear, MEM_WRITE, CACHE_DATA) != MEMORY_ACCESS_OK) {
			/*  exception  */
			return;
		}

		cleared += to_clear;
		addr += to_clear;
	}
}


/*
 *  mtfsf:  Copy FPR into the FPSCR.
 *
 *  arg[0] = ptr to frb
 *  arg[1] = mask
 */
X(mtfsf)
{
	CHECK_FOR_FPU_EXCEPTION;
	cpu->cd.ppc.fpscr &= ~ic->arg[1];
	cpu->cd.ppc.fpscr |= (ic->arg[1] & (*(uint64_t *)ic->arg[0]));
}


/*
 *  mffs:  Copy FPSCR into a FPR.
 *
 *  arg[0] = ptr to frt
 */
X(mffs)
{
	CHECK_FOR_FPU_EXCEPTION;
	(*(uint64_t *)ic->arg[0]) = cpu->cd.ppc.fpscr;
}


/*
 *  fmr:  Floating-point Move
 *
 *  arg[0] = ptr to frb
 *  arg[1] = ptr to frt
 */
X(fmr)
{
	/*
	 *  This works like a normal register to register copy, but
	 *  a) it can cause an FPU exception, and b) the move is always
	 *  64-bit, even when running in 32-bit mode.
	 */
	CHECK_FOR_FPU_EXCEPTION;
	*(uint64_t *)ic->arg[1] = *(uint64_t *)ic->arg[0];
}


/*
 *  fabs:  Floating-point Absulute Value
 *
 *  arg[0] = ptr to frb
 *  arg[1] = ptr to frt
 */
X(fabs)
{
	uint64_t v;
	CHECK_FOR_FPU_EXCEPTION;
	v = *(uint64_t *)ic->arg[0];
	*(uint64_t *)ic->arg[1] = v & 0x7fffffffffffffffULL;
}


/*
 *  fneg:  Floating-point Negate
 *
 *  arg[0] = ptr to frb
 *  arg[1] = ptr to frt
 */
X(fneg)
{
	uint64_t v;
	CHECK_FOR_FPU_EXCEPTION;
	v = *(uint64_t *)ic->arg[0];
	*(uint64_t *)ic->arg[1] = v ^ 0x8000000000000000ULL;
}


/*
 *  fcmpu:  Floating-point Compare Unordered
 *
 *  arg[0] = 28 - 4*bf  (bitfield shift)
 *  arg[1] = ptr to fra
 *  arg[2] = ptr to frb
 */
X(fcmpu)
{
	struct ieee_float_value fra, frb;
	int bf_shift = ic->arg[0], c = 0;

	CHECK_FOR_FPU_EXCEPTION;

	ieee_interpret_float_value(*(uint64_t *)ic->arg[1], &fra, IEEE_FMT_D);
	ieee_interpret_float_value(*(uint64_t *)ic->arg[2], &frb, IEEE_FMT_D);
	if (fra.nan | frb.nan) {
		c = 1;
	} else {
		if (fra.f < frb.f)
			c = 8;
		else if (fra.f > frb.f)
			c = 4;
		else
			c = 2;
	}
	/*  TODO: Signaling vs Quiet NaN  */
	cpu->cd.ppc.cr &= ~(0xf << bf_shift);
	cpu->cd.ppc.cr |= ((c&0xe) << bf_shift);
	cpu->cd.ppc.fpscr &= ~(PPC_FPSCR_FPCC | PPC_FPSCR_VXNAN);
	cpu->cd.ppc.fpscr |= (c << PPC_FPSCR_FPCC_SHIFT);
}


/*
 *  frsp:  Floating-point Round to Single Precision
 *
 *  arg[0] = ptr to frb
 *  arg[1] = ptr to frt
 */
X(frsp)
{
	struct ieee_float_value frb;
	float fl = 0.0;
	int c = 0;

	CHECK_FOR_FPU_EXCEPTION;

	ieee_interpret_float_value(*(uint64_t *)ic->arg[0], &frb, IEEE_FMT_D);
	if (frb.nan) {
		c = 1;
	} else {
		fl = frb.f;
		if (fl < 0.0)
			c = 8;
		else if (fl > 0.0)
			c = 4;
		else
			c = 2;
	}
	/*  TODO: Signaling vs Quiet NaN  */
	cpu->cd.ppc.fpscr &= ~(PPC_FPSCR_FPCC | PPC_FPSCR_VXNAN);
	cpu->cd.ppc.fpscr |= (c << PPC_FPSCR_FPCC_SHIFT);
	(*(uint64_t *)ic->arg[1]) =
	    ieee_store_float_value(fl, IEEE_FMT_D, frb.nan);
}


/*
 *  fctiwz:  Floating-point Convert to Integer Word, Round to Zero
 *
 *  arg[0] = ptr to frb
 *  arg[1] = ptr to frt
 */
X(fctiwz)
{
	struct ieee_float_value frb;
	uint32_t res = 0;

	CHECK_FOR_FPU_EXCEPTION;

	ieee_interpret_float_value(*(uint64_t *)ic->arg[0], &frb, IEEE_FMT_D);
	if (!frb.nan) {
		if (frb.f >= 2147483647.0)
			res = 0x7fffffff;
		else if (frb.f <= -2147483648.0)
			res = 0x80000000;
		else
			res = (int32_t) frb.f;
	}

	*(uint64_t *)ic->arg[1] = (uint32_t)res;
}


/*
 *  fmul:  Floating-point Multiply
 *
 *  arg[0] = ptr to frt
 *  arg[1] = ptr to fra
 *  arg[2] = ptr to frc
 */
X(fmul)
{
	struct ieee_float_value fra;
	struct ieee_float_value frc;
	double result = 0.0;
	int c, nan = 0;

	CHECK_FOR_FPU_EXCEPTION;

	ieee_interpret_float_value(*(uint64_t *)ic->arg[1], &fra, IEEE_FMT_D);
	ieee_interpret_float_value(*(uint64_t *)ic->arg[2], &frc, IEEE_FMT_D);
	if (fra.nan || frc.nan)
		nan = 1;
	else
		result = fra.f * frc.f;
	if (nan)
		c = 1;
	else {
		if (result < 0.0)
			c = 8;
		else if (result > 0.0)
			c = 4;
		else
			c = 2;
	}
	/*  TODO: Signaling vs Quiet NaN  */
	cpu->cd.ppc.fpscr &= ~(PPC_FPSCR_FPCC | PPC_FPSCR_VXNAN);
	cpu->cd.ppc.fpscr |= (c << PPC_FPSCR_FPCC_SHIFT);

	(*(uint64_t *)ic->arg[0]) =
	    ieee_store_float_value(result, IEEE_FMT_D, nan);
}
X(fmuls)
{
	/*  TODO  */
	instr(fmul)(cpu, ic);
}


/*
 *  fmadd:  Floating-point Multiply and Add
 *
 *  arg[0] = ptr to frt
 *  arg[1] = ptr to fra
 *  arg[2] = copy of the instruction word
 */
X(fmadd)
{
	uint32_t iw = ic->arg[2];
	int b = (iw >> 11) & 31, c = (iw >> 6) & 31;
	struct ieee_float_value fra;
	struct ieee_float_value frb;
	struct ieee_float_value frc;
	double result = 0.0;
	int nan = 0, cc;

	CHECK_FOR_FPU_EXCEPTION;

	ieee_interpret_float_value(*(uint64_t *)ic->arg[1], &fra, IEEE_FMT_D);
	ieee_interpret_float_value(cpu->cd.ppc.fpr[b], &frb, IEEE_FMT_D);
	ieee_interpret_float_value(cpu->cd.ppc.fpr[c], &frc, IEEE_FMT_D);
	if (fra.nan || frb.nan || frc.nan)
		nan = 1;
	else
		result = fra.f * frc.f + frb.f;
	if (nan)
		cc = 1;
	else {
		if (result < 0.0)
			cc = 8;
		else if (result > 0.0)
			cc = 4;
		else
			cc = 2;
	}
	/*  TODO: Signaling vs Quiet NaN  */
	cpu->cd.ppc.fpscr &= ~(PPC_FPSCR_FPCC | PPC_FPSCR_VXNAN);
	cpu->cd.ppc.fpscr |= (cc << PPC_FPSCR_FPCC_SHIFT);

	(*(uint64_t *)ic->arg[0]) =
	    ieee_store_float_value(result, IEEE_FMT_D, nan);
}


/*
 *  fmsub:  Floating-point Multiply and Sub
 *
 *  arg[0] = ptr to frt
 *  arg[1] = ptr to fra
 *  arg[2] = copy of the instruction word
 */
X(fmsub)
{
	uint32_t iw = ic->arg[2];
	int b = (iw >> 11) & 31, c = (iw >> 6) & 31;
	struct ieee_float_value fra;
	struct ieee_float_value frb;
	struct ieee_float_value frc;
	double result = 0.0;
	int nan = 0, cc;

	CHECK_FOR_FPU_EXCEPTION;

	ieee_interpret_float_value(*(uint64_t *)ic->arg[1], &fra, IEEE_FMT_D);
	ieee_interpret_float_value(cpu->cd.ppc.fpr[b], &frb, IEEE_FMT_D);
	ieee_interpret_float_value(cpu->cd.ppc.fpr[c], &frc, IEEE_FMT_D);
	if (fra.nan || frb.nan || frc.nan)
		nan = 1;
	else
		result = fra.f * frc.f - frb.f;
	if (nan)
		cc = 1;
	else {
		if (result < 0.0)
			cc = 8;
		else if (result > 0.0)
			cc = 4;
		else
			cc = 2;
	}
	/*  TODO: Signaling vs Quiet NaN  */
	cpu->cd.ppc.fpscr &= ~(PPC_FPSCR_FPCC | PPC_FPSCR_VXNAN);
	cpu->cd.ppc.fpscr |= (cc << PPC_FPSCR_FPCC_SHIFT);

	(*(uint64_t *)ic->arg[0]) =
	    ieee_store_float_value(result, IEEE_FMT_D, nan);
}


/*
 *  fadd, fsub, fdiv:  Various Floating-point operationgs
 *
 *  arg[0] = ptr to fra
 *  arg[1] = ptr to frb
 *  arg[2] = ptr to frt
 */
X(fadd)
{
	struct ieee_float_value fra;
	struct ieee_float_value frb;
	double result = 0.0;
	int nan = 0, c;

	CHECK_FOR_FPU_EXCEPTION;

	ieee_interpret_float_value(*(uint64_t *)ic->arg[0], &fra, IEEE_FMT_D);
	ieee_interpret_float_value(*(uint64_t *)ic->arg[1], &frb, IEEE_FMT_D);
	if (fra.nan || frb.nan)
		nan = 1;
	else
		result = fra.f + frb.f;
	if (nan)
		c = 1;
	else {
		if (result < 0.0)
			c = 8;
		else if (result > 0.0)
			c = 4;
		else
			c = 2;
	}
	/*  TODO: Signaling vs Quiet NaN  */
	cpu->cd.ppc.fpscr &= ~(PPC_FPSCR_FPCC | PPC_FPSCR_VXNAN);
	cpu->cd.ppc.fpscr |= (c << PPC_FPSCR_FPCC_SHIFT);

	(*(uint64_t *)ic->arg[2]) =
	    ieee_store_float_value(result, IEEE_FMT_D, nan);
}
X(fadds)
{
	/*  TODO  */
	instr(fadd)(cpu, ic);
}
X(fsub)
{
	struct ieee_float_value fra;
	struct ieee_float_value frb;
	double result = 0.0;
	int nan = 0, c;

	CHECK_FOR_FPU_EXCEPTION;

	ieee_interpret_float_value(*(uint64_t *)ic->arg[0], &fra, IEEE_FMT_D);
	ieee_interpret_float_value(*(uint64_t *)ic->arg[1], &frb, IEEE_FMT_D);
	if (fra.nan || frb.nan)
		nan = 1;
	else
		result = fra.f - frb.f;
	if (nan)
		c = 1;
	else {
		if (result < 0.0)
			c = 8;
		else if (result > 0.0)
			c = 4;
		else
			c = 2;
	}
	/*  TODO: Signaling vs Quiet NaN  */
	cpu->cd.ppc.fpscr &= ~(PPC_FPSCR_FPCC | PPC_FPSCR_VXNAN);
	cpu->cd.ppc.fpscr |= (c << PPC_FPSCR_FPCC_SHIFT);

	(*(uint64_t *)ic->arg[2]) =
	    ieee_store_float_value(result, IEEE_FMT_D, nan);
}
X(fsubs)
{
	/*  TODO  */
	instr(fsub)(cpu, ic);
}
X(fdiv)
{
	struct ieee_float_value fra;
	struct ieee_float_value frb;
	double result = 0.0;
	int nan = 0, c;

	CHECK_FOR_FPU_EXCEPTION;

	ieee_interpret_float_value(*(uint64_t *)ic->arg[0], &fra, IEEE_FMT_D);
	ieee_interpret_float_value(*(uint64_t *)ic->arg[1], &frb, IEEE_FMT_D);
	if (fra.nan || frb.nan || frb.f == 0)
		nan = 1;
	else
		result = fra.f / frb.f;
	if (nan)
		c = 1;
	else {
		if (result < 0.0)
			c = 8;
		else if (result > 0.0)
			c = 4;
		else
			c = 2;
	}
	/*  TODO: Signaling vs Quiet NaN  */
	cpu->cd.ppc.fpscr &= ~(PPC_FPSCR_FPCC | PPC_FPSCR_VXNAN);
	cpu->cd.ppc.fpscr |= (c << PPC_FPSCR_FPCC_SHIFT);

	(*(uint64_t *)ic->arg[2]) =
	    ieee_store_float_value(result, IEEE_FMT_D, nan);
}
X(fdivs)
{
	/*  TODO  */
	instr(fdiv)(cpu, ic);
}


/*
 *  llsc: Load-linked and store conditional
 *
 *  arg[0] = copy of the instruction word.
 */
X(llsc)
{
	int iw = ic->arg[0], len = 4, load = 0, xo = (iw >> 1) & 1023;
	int i, rc = iw & 1, rt, ra, rb;
	uint64_t addr = 0, value;
	unsigned char d[8];

	switch (xo) {
	case PPC_31_LDARX:
		len = 8;
	case PPC_31_LWARX:
		load = 1;
		break;
	case PPC_31_STDCX_DOT:
		len = 8;
	case PPC_31_STWCX_DOT:
		break;
	}

	rt = (iw >> 21) & 31;
	ra = (iw >> 16) & 31;
	rb = (iw >> 11) & 31;

	if (ra != 0)
		addr = cpu->cd.ppc.gpr[ra];
	addr += cpu->cd.ppc.gpr[rb];

	if (load) {
		if (rc) {
			fatal("ll: rc-bit set?\n");
			exit(1);
		}
		if (cpu->memory_rw(cpu, cpu->mem, addr, d, len,
		    MEM_READ, CACHE_DATA) != MEMORY_ACCESS_OK) {
			fatal("ll: error: TODO\n");
			exit(1);
		}

		value = 0;
		for (i=0; i<len; i++) {
			value <<= 8;
			if (cpu->byte_order == EMUL_BIG_ENDIAN)
				value |= d[i];
			else
				value |= d[len - 1 - i];
		}

		cpu->cd.ppc.gpr[rt] = value;
		cpu->cd.ppc.ll_addr = addr;
		cpu->cd.ppc.ll_bit = 1;
	} else {
		uint32_t old_so = cpu->cd.ppc.spr[SPR_XER] & PPC_XER_SO;
		if (!rc) {
			fatal("sc: rc-bit not set?\n");
			exit(1);
		}

		value = cpu->cd.ppc.gpr[rt];

		/*  "If the store is performed, bits 0-2 of Condition
		    Register Field 0 are set to 0b001, otherwise, they are
		    set to 0b000. The SO bit of the XER is copied to to bit
		    4 of Condition Register Field 0.  */
		if (!cpu->cd.ppc.ll_bit || cpu->cd.ppc.ll_addr != addr) {
			cpu->cd.ppc.cr &= 0x0fffffff;
			if (old_so)
				cpu->cd.ppc.cr |= 0x10000000;
			cpu->cd.ppc.ll_bit = 0;
			return;
		}

		for (i=0; i<len; i++) {
			if (cpu->byte_order == EMUL_BIG_ENDIAN)
				d[len - 1 - i] = value >> (8*i);
			else
				d[i] = value >> (8*i);
		}

		if (cpu->memory_rw(cpu, cpu->mem, addr, d, len,
		    MEM_WRITE, CACHE_DATA) != MEMORY_ACCESS_OK) {
			fatal("sc: error: TODO\n");
			exit(1);
		}

		cpu->cd.ppc.cr &= 0x0fffffff;
		cpu->cd.ppc.cr |= 0x20000000;	/*  success!  */
		if (old_so)
			cpu->cd.ppc.cr |= 0x10000000;

		/*  Clear _all_ CPUs' ll_bits:  */
		for (i=0; i<cpu->machine->ncpus; i++)
			cpu->machine->cpus[i]->cd.ppc.ll_bit = 0;
	}
}


/*
 *  mtsr, mtsrin:  Move To Segment Register [Indirect]
 *
 *  arg[0] = sr number, or for indirect mode: ptr to rb
 *  arg[1] = ptr to rt
 *
 *  TODO: These only work for 32-bit mode!
 */
X(mtsr)
{
	int sr_num = ic->arg[0];
	uint32_t old = cpu->cd.ppc.sr[sr_num];
	cpu->cd.ppc.sr[sr_num] = reg(ic->arg[1]);

	if (cpu->cd.ppc.sr[sr_num] != old)
		cpu->invalidate_translation_caches(cpu, ic->arg[0] << 28,
		    INVALIDATE_ALL | INVALIDATE_VADDR_UPPER4);
}
X(mtsrin)
{
	int sr_num = reg(ic->arg[0]) >> 28;
	uint32_t old = cpu->cd.ppc.sr[sr_num];
	cpu->cd.ppc.sr[sr_num] = reg(ic->arg[1]);

	if (cpu->cd.ppc.sr[sr_num] != old)
		cpu->invalidate_translation_caches(cpu, sr_num << 28,
		    INVALIDATE_ALL | INVALIDATE_VADDR_UPPER4);
}


/*
 *  mfsrin, mtsrin:  Move From/To Segment Register Indirect
 *
 *  arg[0] = sr number, or for indirect mode: ptr to rb
 *  arg[1] = ptr to rt
 */
X(mfsr)
{
	/*  TODO: This only works for 32-bit mode  */
	reg(ic->arg[1]) = cpu->cd.ppc.sr[ic->arg[0]];
}
X(mfsrin)
{
	/*  TODO: This only works for 32-bit mode  */
	uint32_t sr_num = reg(ic->arg[0]) >> 28;
	reg(ic->arg[1]) = cpu->cd.ppc.sr[sr_num];
}


/*
 *  rldicl:
 *
 *  arg[0] = copy of the instruction word
 */
X(rldicl)
{
	int rs = (ic->arg[0] >> 21) & 31;
	int ra = (ic->arg[0] >> 16) & 31;
	int sh = ((ic->arg[0] >> 11) & 31) | ((ic->arg[0] & 2) << 4);
	int mb = ((ic->arg[0] >> 6) & 31) | (ic->arg[0] & 0x20);
	int rc = ic->arg[0] & 1;
	uint64_t tmp = cpu->cd.ppc.gpr[rs], tmp2;
	/*  TODO: Fix this, its performance is awful:  */
	while (sh-- != 0) {
		int b = (tmp >> 63) & 1;
		tmp = (tmp << 1) | b;
	}
	tmp2 = 0;
	while (mb <= 63) {
		tmp |= ((uint64_t)1 << (63-mb));
		mb ++;
	}
	cpu->cd.ppc.gpr[ra] = tmp & tmp2;
	if (rc)
		update_cr0(cpu, cpu->cd.ppc.gpr[ra]);
}


/*
 *  rldicr:
 *
 *  arg[0] = copy of the instruction word
 */
X(rldicr)
{
	int rs = (ic->arg[0] >> 21) & 31;
	int ra = (ic->arg[0] >> 16) & 31;
	int sh = ((ic->arg[0] >> 11) & 31) | ((ic->arg[0] & 2) << 4);
	int me = ((ic->arg[0] >> 6) & 31) | (ic->arg[0] & 0x20);
	int rc = ic->arg[0] & 1;
	uint64_t tmp = cpu->cd.ppc.gpr[rs];
	/*  TODO: Fix this, its performance is awful:  */
	while (sh-- != 0) {
		int b = (tmp >> 63) & 1;
		tmp = (tmp << 1) | b;
	}
	while (me++ < 63)
		tmp &= ~((uint64_t)1 << (63-me));
	cpu->cd.ppc.gpr[ra] = tmp;
	if (rc)
		update_cr0(cpu, tmp);
}


/*
 *  rldimi:
 *
 *  arg[0] = copy of the instruction word
 */
X(rldimi)
{
	uint32_t iw = ic->arg[0];
	int rs = (iw >> 21) & 31, ra = (iw >> 16) & 31;
	int sh = ((iw >> 11) & 31) | ((iw & 2) << 4);
	int mb = ((iw >> 6) & 31) | (iw & 0x20);
	int rc = ic->arg[0] & 1;
	int m;
	uint64_t tmp, s = cpu->cd.ppc.gpr[rs];
	/*  TODO: Fix this, its performance is awful:  */
	while (sh-- != 0) {
		int b = (s >> 63) & 1;
		s = (s << 1) | b;
	}
	m = mb; tmp = 0;
	do {
		tmp |= ((uint64_t)1 << (63-m));
		m ++;
	} while (m != 63 - sh);
	cpu->cd.ppc.gpr[ra] &= ~tmp;
	cpu->cd.ppc.gpr[ra] |= (tmp & s);
	if (rc)
		update_cr0(cpu, cpu->cd.ppc.gpr[ra]);
}


/*
 *  rlwnm:
 *
 *  arg[0] = ptr to ra
 *  arg[1] = mask
 *  arg[2] = copy of the instruction word
 */
X(rlwnm)
{
	uint32_t tmp, iword = ic->arg[2];
	int rs = (iword >> 21) & 31;
	int rb = (iword >> 11) & 31;
	int sh = cpu->cd.ppc.gpr[rb] & 0x1f;
	tmp = (uint32_t)cpu->cd.ppc.gpr[rs];
	tmp = (tmp << sh) | (tmp >> (32-sh));
	tmp &= (uint32_t)ic->arg[1];
	reg(ic->arg[0]) = tmp;
}
DOT0(rlwnm)


/*
 *  rlwinm:
 *
 *  arg[0] = ptr to ra
 *  arg[1] = mask
 *  arg[2] = copy of the instruction word
 */
X(rlwinm)
{
	uint32_t tmp, iword = ic->arg[2];
	int rs = (iword >> 21) & 31;
	int sh = (iword >> 11) & 31;
	tmp = (uint32_t)cpu->cd.ppc.gpr[rs];
	tmp = (tmp << sh) | (tmp >> (32-sh));
	tmp &= (uint32_t)ic->arg[1];
	reg(ic->arg[0]) = tmp;
}
DOT0(rlwinm)


/*
 *  rlwimi:
 *
 *  arg[0] = ptr to rs
 *  arg[1] = ptr to ra
 *  arg[2] = copy of the instruction word
 */
X(rlwimi)
{
	MODE_uint_t tmp = reg(ic->arg[0]), ra = reg(ic->arg[1]);
	uint32_t iword = ic->arg[2];
	int sh = (iword >> 11) & 31;
	int mb = (iword >> 6) & 31;
	int me = (iword >> 1) & 31;   
	int rc = iword & 1;

	tmp = (tmp << sh) | (tmp >> (32-sh));

	for (;;) {
		uint64_t mask;
		mask = (uint64_t)1 << (31-mb);
		ra &= ~mask;
		ra |= (tmp & mask);
		if (mb == me)
			break;
		mb ++;
		if (mb == 32)
			mb = 0;
	}
	reg(ic->arg[1]) = ra;
	if (rc)
		update_cr0(cpu, ra);
}


/*
 *  srawi:
 *
 *  arg[0] = ptr to rs
 *  arg[1] = ptr to ra
 *  arg[2] = sh (shift amount)
 */
X(srawi)
{
	uint32_t tmp = reg(ic->arg[0]);
	int i = 0, j = 0, sh = ic->arg[2];

	cpu->cd.ppc.spr[SPR_XER] &= ~PPC_XER_CA;
	if (tmp & 0x80000000)
		i = 1;
	while (sh-- > 0) {
		if (tmp & 1)
			j ++;
		tmp >>= 1;
		if (tmp & 0x40000000)
			tmp |= 0x80000000;
	}
	if (i && j>0)
		cpu->cd.ppc.spr[SPR_XER] |= PPC_XER_CA;
	reg(ic->arg[1]) = (int64_t)(int32_t)tmp;
}
DOT1(srawi)


/*
 *  mcrf:  Move inside condition register
 *
 *  arg[0] = 28-4*bf,  arg[1] = 28-4*bfa
 */
X(mcrf)
{
	int bf_shift = ic->arg[0], bfa_shift = ic->arg[1];
	uint32_t tmp = (cpu->cd.ppc.cr >> bfa_shift) & 0xf;
	cpu->cd.ppc.cr &= ~(0xf << bf_shift);
	cpu->cd.ppc.cr |= (tmp << bf_shift);
}


/*
 *  crand, crxor etc:  Condition Register operations
 *
 *  arg[0] = copy of the instruction word
 */
X(crand) {
	uint32_t iword = ic->arg[0]; int bt = (iword >> 21) & 31;
	int ba = (iword >> 16) & 31, bb = (iword >> 11) & 31;
	ba = (cpu->cd.ppc.cr >> (31-ba)) & 1;
	bb = (cpu->cd.ppc.cr >> (31-bb)) & 1;
	cpu->cd.ppc.cr &= ~(1 << (31-bt));
	if (ba & bb)
		cpu->cd.ppc.cr |= (1 << (31-bt));
}
X(crandc) {
	uint32_t iword = ic->arg[0]; int bt = (iword >> 21) & 31;
	int ba = (iword >> 16) & 31, bb = (iword >> 11) & 31;
	ba = (cpu->cd.ppc.cr >> (31-ba)) & 1;
	bb = (cpu->cd.ppc.cr >> (31-bb)) & 1;
	cpu->cd.ppc.cr &= ~(1 << (31-bt));
	if (!(ba & bb))
		cpu->cd.ppc.cr |= (1 << (31-bt));
}
X(creqv) {
	uint32_t iword = ic->arg[0]; int bt = (iword >> 21) & 31;
	int ba = (iword >> 16) & 31, bb = (iword >> 11) & 31;
	ba = (cpu->cd.ppc.cr >> (31-ba)) & 1;
	bb = (cpu->cd.ppc.cr >> (31-bb)) & 1;
	cpu->cd.ppc.cr &= ~(1 << (31-bt));
	if (!(ba ^ bb))
		cpu->cd.ppc.cr |= (1 << (31-bt));
}
X(cror) {
	uint32_t iword = ic->arg[0]; int bt = (iword >> 21) & 31;
	int ba = (iword >> 16) & 31, bb = (iword >> 11) & 31;
	ba = (cpu->cd.ppc.cr >> (31-ba)) & 1;
	bb = (cpu->cd.ppc.cr >> (31-bb)) & 1;
	cpu->cd.ppc.cr &= ~(1 << (31-bt));
	if (ba | bb)
		cpu->cd.ppc.cr |= (1 << (31-bt));
}
X(crorc) {
	uint32_t iword = ic->arg[0]; int bt = (iword >> 21) & 31;
	int ba = (iword >> 16) & 31, bb = (iword >> 11) & 31;
	ba = (cpu->cd.ppc.cr >> (31-ba)) & 1;
	bb = (cpu->cd.ppc.cr >> (31-bb)) & 1;
	cpu->cd.ppc.cr &= ~(1 << (31-bt));
	if (!(ba | bb))
		cpu->cd.ppc.cr |= (1 << (31-bt));
}
X(crnor) {
	uint32_t iword = ic->arg[0]; int bt = (iword >> 21) & 31;
	int ba = (iword >> 16) & 31, bb = (iword >> 11) & 31;
	ba = (cpu->cd.ppc.cr >> (31-ba)) & 1;
	bb = (cpu->cd.ppc.cr >> (31-bb)) & 1;
	cpu->cd.ppc.cr &= ~(1 << (31-bt));
	if (!(ba | bb))
		cpu->cd.ppc.cr |= (1 << (31-bt));
}
X(crxor) {
	uint32_t iword = ic->arg[0]; int bt = (iword >> 21) & 31;
	int ba = (iword >> 16) & 31, bb = (iword >> 11) & 31;
	ba = (cpu->cd.ppc.cr >> (31-ba)) & 1;
	bb = (cpu->cd.ppc.cr >> (31-bb)) & 1;
	cpu->cd.ppc.cr &= ~(1 << (31-bt));
	if (ba ^ bb)
		cpu->cd.ppc.cr |= (1 << (31-bt));
}


/*
 *  mfspr: Move from SPR
 *
 *  arg[0] = pointer to destination register
 *  arg[1] = pointer to source SPR
 */
X(mfspr) {
	/*  TODO: Check permission  */
	reg(ic->arg[0]) = reg(ic->arg[1]);
}
X(mfspr_pmc1) {
	/*
	 *  TODO: This is a temporary hack to make NetBSD/ppc detect
	 *  a CPU of the correct (emulated) speed.
	 */
	reg(ic->arg[0]) = cpu->machine->emulated_hz / 10;
}
X(mftb) {
	/*  NOTE/TODO: This increments the time base (slowly) if it
	    is being polled.  */
	if (++cpu->cd.ppc.spr[SPR_TBL] == 0)
		cpu->cd.ppc.spr[SPR_TBU] ++;
	reg(ic->arg[0]) = cpu->cd.ppc.spr[SPR_TBL];
}
X(mftbu) {
	reg(ic->arg[0]) = cpu->cd.ppc.spr[SPR_TBU];
}


/*
 *  mtspr: Move to SPR.
 *
 *  arg[0] = pointer to source register
 *  arg[1] = pointer to the SPR
 */
X(mtspr) {
	/*  TODO: Check permission  */
	reg(ic->arg[1]) = reg(ic->arg[0]);
}
X(mtlr) {
	cpu->cd.ppc.spr[SPR_LR] = reg(ic->arg[0]);
}
X(mtctr) {
	cpu->cd.ppc.spr[SPR_CTR] = reg(ic->arg[0]);
}


/*
 *  rfi[d]:  Return from Interrupt
 */
X(rfi)
{
	uint64_t tmp;

	reg_access_msr(cpu, &tmp, 0, 0);
	tmp &= ~0xffff;
	tmp |= (cpu->cd.ppc.spr[SPR_SRR1] & 0xffff);
	reg_access_msr(cpu, &tmp, 1, 0);

	cpu->pc = cpu->cd.ppc.spr[SPR_SRR0];
	quick_pc_to_pointers(cpu);
}
X(rfid)
{
	uint64_t tmp, mask = 0x800000000000ff73ULL;

	reg_access_msr(cpu, &tmp, 0, 0);
	tmp &= ~mask;
	tmp |= (cpu->cd.ppc.spr[SPR_SRR1] & mask);
	reg_access_msr(cpu, &tmp, 1, 0);

	cpu->pc = cpu->cd.ppc.spr[SPR_SRR0];
	if (!(tmp & PPC_MSR_SF))
		cpu->pc = (uint32_t)cpu->pc;
	quick_pc_to_pointers(cpu);
}


/*
 *  mfcr:  Move From Condition Register
 *
 *  arg[0] = pointer to destination register
 */
X(mfcr)
{
	reg(ic->arg[0]) = cpu->cd.ppc.cr;
}


/*
 *  mfmsr:  Move From MSR
 *
 *  arg[0] = pointer to destination register
 */
X(mfmsr)
{
	reg_access_msr(cpu, (uint64_t*)ic->arg[0], 0, 0);
}


/*
 *  mtmsr:  Move To MSR
 *
 *  arg[0] = pointer to source register
 *  arg[1] = page offset of the next instruction
 *  arg[2] = 0 for 32-bit (mtmsr), 1 for 64-bit (mtmsrd)
 */
X(mtmsr)
{
	MODE_uint_t old_pc;
	uint64_t x = reg(ic->arg[0]);

	/*  TODO: check permission!  */

	/*  Synchronize the PC (pointing to _after_ this instruction)  */
	cpu->pc = (cpu->pc & ~0xfff) + ic->arg[1];
	old_pc = cpu->pc;

	if (!ic->arg[2]) {
		uint64_t y;
		reg_access_msr(cpu, &y, 0, 0);
		x = (y & 0xffffffff00000000ULL) | (x & 0xffffffffULL);
	}

	reg_access_msr(cpu, &x, 1, 1);

	/*
	 *  Super-ugly hack:  If the pc wasn't changed (i.e. if there was no
	 *  exception while accessing the msr), then we _decrease_ the PC by 4
	 *  again. This is because the next ic could be an end_of_page.
	 */
	if ((MODE_uint_t)cpu->pc == old_pc)
		cpu->pc -= 4;
}


/*
 *  wrteei:  Write EE immediate  (on PPC405GP)
 *
 *  arg[0] = either 0 or 0x8000
 */
X(wrteei)
{
	/*  TODO: check permission!  */
	uint64_t x;

	/*  Synchronize the PC (pointing to _after_ this instruction)  */
	cpu->pc = (cpu->pc & ~0xfff) + ic->arg[1];

	reg_access_msr(cpu, &x, 0, 0);
	x = (x & ~0x8000) | ic->arg[0];
	reg_access_msr(cpu, &x, 1, 1);
}


/*
 *  mtcrf:  Move To Condition Register Fields
 *
 *  arg[0] = pointer to source register
 */
X(mtcrf)
{
	cpu->cd.ppc.cr &= ~ic->arg[1];
	cpu->cd.ppc.cr |= (reg(ic->arg[0]) & ic->arg[1]);
}


/*
 *  mulli:  Multiply Low Immediate.
 *
 *  arg[0] = pointer to source register ra
 *  arg[1] = int32_t immediate
 *  arg[2] = pointer to destination register rt
 */
X(mulli)
{
	reg(ic->arg[2]) = (uint32_t)(reg(ic->arg[0]) * (int32_t)ic->arg[1]);
}


/*
 *  Load/Store Multiple:
 *
 *  arg[0] = rs  (or rt for loads)  NOTE: not a pointer
 *  arg[1] = ptr to ra
 *  arg[2] = int32_t immediate offset
 */
X(lmw) {
	MODE_uint_t addr = reg(ic->arg[1]) + (int32_t)ic->arg[2];
	unsigned char d[4];
	int rs = ic->arg[0];

	int low_pc = ((size_t)ic - (size_t)cpu->cd.ppc.cur_ic_page)
	    / sizeof(struct ppc_instr_call);
	cpu->pc &= ~((PPC_IC_ENTRIES_PER_PAGE-1)
	    << PPC_INSTR_ALIGNMENT_SHIFT);
	cpu->pc |= (low_pc << PPC_INSTR_ALIGNMENT_SHIFT);

	while (rs <= 31) {
		if (cpu->memory_rw(cpu, cpu->mem, addr, d, sizeof(d),
		    MEM_READ, CACHE_DATA) != MEMORY_ACCESS_OK) {
			/*  exception  */
			return;
		}

		if (cpu->byte_order == EMUL_BIG_ENDIAN)
			cpu->cd.ppc.gpr[rs] = (d[0] << 24) + (d[1] << 16)
			    + (d[2] << 8) + d[3];
		else
			cpu->cd.ppc.gpr[rs] = (d[3] << 24) + (d[2] << 16)
			    + (d[1] << 8) + d[0];

		rs ++;
		addr += sizeof(uint32_t);
	}
}
X(stmw) {
	MODE_uint_t addr = reg(ic->arg[1]) + (int32_t)ic->arg[2];
	unsigned char d[4];
	int rs = ic->arg[0];

	int low_pc = ((size_t)ic - (size_t)cpu->cd.ppc.cur_ic_page)
	    / sizeof(struct ppc_instr_call);
	cpu->pc &= ~((PPC_IC_ENTRIES_PER_PAGE-1)
	    << PPC_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << PPC_INSTR_ALIGNMENT_SHIFT);

	while (rs <= 31) {
		uint32_t tmp = cpu->cd.ppc.gpr[rs];
		if (cpu->byte_order == EMUL_BIG_ENDIAN) {
			d[3] = tmp; d[2] = tmp >> 8;
			d[1] = tmp >> 16; d[0] = tmp >> 24;
		} else {
			d[0] = tmp; d[1] = tmp >> 8;
			d[2] = tmp >> 16; d[3] = tmp >> 24;
		}
		if (cpu->memory_rw(cpu, cpu->mem, addr, d, sizeof(d),
		    MEM_WRITE, CACHE_DATA) != MEMORY_ACCESS_OK) {
			/*  exception  */
			return;
		}

		rs ++;
		addr += sizeof(uint32_t);
	}
}


/*
 *  Load/store string:
 *
 *  arg[0] = rs   (well, rt for lswi)
 *  arg[1] = ptr to ra (or ptr to zero)
 *  arg[2] = nb
 */
X(lswi)
{
	MODE_uint_t addr = reg(ic->arg[1]);
	int rt = ic->arg[0], nb = ic->arg[2];
	int sub = 0;

	int low_pc = ((size_t)ic - (size_t)cpu->cd.ppc.cur_ic_page)
	    / sizeof(struct ppc_instr_call);
	cpu->pc &= ~((PPC_IC_ENTRIES_PER_PAGE-1)
	    << PPC_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << PPC_INSTR_ALIGNMENT_SHIFT);

	while (nb > 0) {
		unsigned char d;
		if (cpu->memory_rw(cpu, cpu->mem, addr, &d, 1,
		    MEM_READ, CACHE_DATA) != MEMORY_ACCESS_OK) {
			/*  exception  */
			return;
		}

		if (cpu->cd.ppc.mode == MODE_POWER && sub == 0)
			cpu->cd.ppc.gpr[rt] = 0;
		cpu->cd.ppc.gpr[rt] &= ~(0xff << (24-8*sub));
		cpu->cd.ppc.gpr[rt] |= (d << (24-8*sub));
		sub ++;
		if (sub == 4) {
			rt = (rt + 1) & 31;
			sub = 0;
		}
		addr ++;
		nb --;
	}
}
X(stswi)
{
	MODE_uint_t addr = reg(ic->arg[1]);
	int rs = ic->arg[0], nb = ic->arg[2];
	uint32_t cur = cpu->cd.ppc.gpr[rs];
	int sub = 0;

	int low_pc = ((size_t)ic - (size_t)cpu->cd.ppc.cur_ic_page)
	    / sizeof(struct ppc_instr_call);
	cpu->pc &= ~((PPC_IC_ENTRIES_PER_PAGE-1)
	    << PPC_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << PPC_INSTR_ALIGNMENT_SHIFT);

	while (nb > 0) {
		unsigned char d = cur >> 24;
		if (cpu->memory_rw(cpu, cpu->mem, addr, &d, 1,
		    MEM_WRITE, CACHE_DATA) != MEMORY_ACCESS_OK) {
			/*  exception  */
			return;
		}
		cur <<= 8;
		sub ++;
		if (sub == 4) {
			rs = (rs + 1) & 31;
			sub = 0;
			cur = cpu->cd.ppc.gpr[rs];
		}
		addr ++;
		nb --;
	}
}


/*
 *  Shifts, and, or, xor, etc.
 *
 *  arg[0] = pointer to source register rs
 *  arg[1] = pointer to source register rb
 *  arg[2] = pointer to destination register ra
 */
X(extsb) {
#ifdef MODE32
	reg(ic->arg[2]) = (int32_t)(int8_t)reg(ic->arg[0]);
#else
	reg(ic->arg[2]) = (int64_t)(int8_t)reg(ic->arg[0]);
#endif
}
DOT2(extsb)
X(extsh) {
#ifdef MODE32
	reg(ic->arg[2]) = (int32_t)(int16_t)reg(ic->arg[0]);
#else
	reg(ic->arg[2]) = (int64_t)(int16_t)reg(ic->arg[0]);
#endif
}
DOT2(extsh)
X(extsw) {
#ifdef MODE32
	fatal("TODO: extsw: invalid instruction\n");
#else
	reg(ic->arg[2]) = (int64_t)(int32_t)reg(ic->arg[0]);
#endif
}
DOT2(extsw)
X(slw) {	reg(ic->arg[2]) = (uint64_t)reg(ic->arg[0])
		    << (reg(ic->arg[1]) & 31); }
DOT2(slw)
X(sld) {int sa = reg(ic->arg[1]) & 127;
	if (sa >= 64)	reg(ic->arg[2]) = 0;
	else reg(ic->arg[2]) = (uint64_t)reg(ic->arg[0]) << (sa & 63); }
DOT2(sld)
X(sraw)
{
	uint32_t tmp = reg(ic->arg[0]);
	int i = 0, j = 0, sh = reg(ic->arg[1]) & 31;

	cpu->cd.ppc.spr[SPR_XER] &= ~PPC_XER_CA;
	if (tmp & 0x80000000)
		i = 1;
	while (sh-- > 0) {
		if (tmp & 1)
			j ++;
		tmp >>= 1;
		if (tmp & 0x40000000)
			tmp |= 0x80000000;
	}
	if (i && j>0)
		cpu->cd.ppc.spr[SPR_XER] |= PPC_XER_CA;
	reg(ic->arg[2]) = (int64_t)(int32_t)tmp;
}
DOT2(sraw)
X(srw) {	reg(ic->arg[2]) = (uint64_t)reg(ic->arg[0])
		    >> (reg(ic->arg[1]) & 31); }
DOT2(srw)
X(and) {	reg(ic->arg[2]) = reg(ic->arg[0]) & reg(ic->arg[1]); }
DOT2(and)
X(nand) {	reg(ic->arg[2]) = ~(reg(ic->arg[0]) & reg(ic->arg[1])); }
DOT2(nand)
X(andc) {	reg(ic->arg[2]) = reg(ic->arg[0]) & (~reg(ic->arg[1])); }
DOT2(andc)
X(nor) {	reg(ic->arg[2]) = ~(reg(ic->arg[0]) | reg(ic->arg[1])); }
DOT2(nor)
X(mr) {		reg(ic->arg[2]) = reg(ic->arg[1]); }
X(or) {		reg(ic->arg[2]) = reg(ic->arg[0]) | reg(ic->arg[1]); }
DOT2(or)
X(orc) {	reg(ic->arg[2]) = reg(ic->arg[0]) | (~reg(ic->arg[1])); }
DOT2(orc)
X(xor) {	reg(ic->arg[2]) = reg(ic->arg[0]) ^ reg(ic->arg[1]); }
DOT2(xor)
X(eqv) {	reg(ic->arg[2]) = ~(reg(ic->arg[0]) ^ reg(ic->arg[1])); }
DOT2(eqv)


/*
 *  neg:
 *
 *  arg[0] = pointer to source register ra
 *  arg[1] = pointer to destination register rt
 */
X(neg) {	reg(ic->arg[1]) = -reg(ic->arg[0]); }
DOT1(neg)


/*
 *  mullw, mulhw[u], divw[u]:
 *
 *  arg[0] = pointer to source register ra
 *  arg[1] = pointer to source register rb
 *  arg[2] = pointer to destination register rt
 */
X(mullw)
{
	int32_t sum = (int32_t)reg(ic->arg[0]) * (int32_t)reg(ic->arg[1]);
	reg(ic->arg[2]) = (int32_t)sum;
}
DOT2(mullw)
X(mulhw)
{
	int64_t sum;
	sum = (int64_t)(int32_t)reg(ic->arg[0])
	    * (int64_t)(int32_t)reg(ic->arg[1]);
	reg(ic->arg[2]) = sum >> 32;
}
DOT2(mulhw)
X(mulhwu)
{
	uint64_t sum;
	sum = (uint64_t)(uint32_t)reg(ic->arg[0])
	    * (uint64_t)(uint32_t)reg(ic->arg[1]);
	reg(ic->arg[2]) = sum >> 32;
}
DOT2(mulhwu)
X(divw)
{
	int32_t a = reg(ic->arg[0]), b = reg(ic->arg[1]);
	int32_t sum;
	if (b == 0)
		sum = 0;
	else
		sum = a / b;
	reg(ic->arg[2]) = (uint32_t)sum;
}
DOT2(divw)
X(divwu)
{
	uint32_t a = reg(ic->arg[0]), b = reg(ic->arg[1]);
	uint32_t sum;
	if (b == 0)
		sum = 0;
	else
		sum = a / b;
	reg(ic->arg[2]) = sum;
}
DOT2(divwu)


/*
 *  add:  Add.
 *
 *  arg[0] = pointer to source register ra
 *  arg[1] = pointer to source register rb
 *  arg[2] = pointer to destination register rt
 */
X(add)     { reg(ic->arg[2]) = reg(ic->arg[0]) + reg(ic->arg[1]); }
DOT2(add)


/*
 *  addc:  Add carrying.
 *
 *  arg[0] = pointer to source register ra
 *  arg[1] = pointer to source register rb
 *  arg[2] = pointer to destination register rt
 */
X(addc)
{
	/*  TODO: this only works in 32-bit mode  */
	uint64_t tmp = (uint32_t)reg(ic->arg[0]);
	uint64_t tmp2 = tmp;
	cpu->cd.ppc.spr[SPR_XER] &= ~PPC_XER_CA;
	tmp += (uint32_t)reg(ic->arg[1]);
	if ((tmp >> 32) != (tmp2 >> 32))
		cpu->cd.ppc.spr[SPR_XER] |= PPC_XER_CA;
	reg(ic->arg[2]) = (uint32_t)tmp;
}


/*
 *  adde:  Add extended, etc.
 *
 *  arg[0] = pointer to source register ra
 *  arg[1] = pointer to source register rb
 *  arg[2] = pointer to destination register rt
 */
X(adde)
{
	/*  TODO: this only works in 32-bit mode  */
	int old_ca = cpu->cd.ppc.spr[SPR_XER] & PPC_XER_CA;
	uint64_t tmp = (uint32_t)reg(ic->arg[0]);
	uint64_t tmp2 = tmp;
	cpu->cd.ppc.spr[SPR_XER] &= ~PPC_XER_CA;
	tmp += (uint32_t)reg(ic->arg[1]);
	if (old_ca)
		tmp ++;
	if ((tmp >> 32) != (tmp2 >> 32))
		cpu->cd.ppc.spr[SPR_XER] |= PPC_XER_CA;
	reg(ic->arg[2]) = (uint32_t)tmp;
}
DOT2(adde)
X(addme)
{
	/*  TODO: this only works in 32-bit mode  */
	int old_ca = cpu->cd.ppc.spr[SPR_XER] & PPC_XER_CA;
	uint64_t tmp = (uint32_t)reg(ic->arg[0]);
	uint64_t tmp2 = tmp;
	cpu->cd.ppc.spr[SPR_XER] &= ~PPC_XER_CA;
	if (old_ca)
		tmp ++;
	tmp += 0xffffffffULL;
	if ((tmp >> 32) != (tmp2 >> 32))
		cpu->cd.ppc.spr[SPR_XER] |= PPC_XER_CA;
	reg(ic->arg[2]) = (uint32_t)tmp;
}
DOT2(addme)
X(addze)
{
	/*  TODO: this only works in 32-bit mode  */
	int old_ca = cpu->cd.ppc.spr[SPR_XER] & PPC_XER_CA;
	uint64_t tmp = (uint32_t)reg(ic->arg[0]);
	uint64_t tmp2 = tmp;
	cpu->cd.ppc.spr[SPR_XER] &= ~PPC_XER_CA;
	if (old_ca)
		tmp ++;
	if ((tmp >> 32) != (tmp2 >> 32))
		cpu->cd.ppc.spr[SPR_XER] |= PPC_XER_CA;
	reg(ic->arg[2]) = (uint32_t)tmp;
}
DOT2(addze)


/*
 *  subf:  Subf, etc.
 *
 *  arg[0] = pointer to source register ra
 *  arg[1] = pointer to source register rb
 *  arg[2] = pointer to destination register rt
 */
X(subf)
{
	reg(ic->arg[2]) = reg(ic->arg[1]) - reg(ic->arg[0]);
}
DOT2(subf)
X(subfc)
{
	cpu->cd.ppc.spr[SPR_XER] &= ~PPC_XER_CA;
	if (reg(ic->arg[1]) >= reg(ic->arg[0]))
		cpu->cd.ppc.spr[SPR_XER] |= PPC_XER_CA;
	reg(ic->arg[2]) = reg(ic->arg[1]) - reg(ic->arg[0]);
}
DOT2(subfc)
X(subfe)
{
	int old_ca = (cpu->cd.ppc.spr[SPR_XER] & PPC_XER_CA)? 1 : 0;
	cpu->cd.ppc.spr[SPR_XER] &= ~PPC_XER_CA;
	if (reg(ic->arg[1]) == reg(ic->arg[0])) {
		if (old_ca)
			cpu->cd.ppc.spr[SPR_XER] |= PPC_XER_CA;
	} else if (reg(ic->arg[1]) >= reg(ic->arg[0]))
		cpu->cd.ppc.spr[SPR_XER] |= PPC_XER_CA;

	/*
	 *  TODO: The register value calculation should be correct,
	 *  but the CA bit calculation above is probably not.
	 */

	reg(ic->arg[2]) = reg(ic->arg[1]) - reg(ic->arg[0]) - (old_ca? 0 : 1);
}
DOT2(subfe)
X(subfme)
{
	int old_ca = cpu->cd.ppc.spr[SPR_XER] & PPC_XER_CA;
	uint64_t tmp = (uint32_t)(~reg(ic->arg[0]));
	tmp += 0xffffffffULL;
	cpu->cd.ppc.spr[SPR_XER] &= ~PPC_XER_CA;
	if (old_ca)
		tmp ++;
	if ((tmp >> 32) != 0)
		cpu->cd.ppc.spr[SPR_XER] |= PPC_XER_CA;
	reg(ic->arg[2]) = (uint32_t)tmp;
}
DOT2(subfme)
X(subfze)
{
	int old_ca = cpu->cd.ppc.spr[SPR_XER] & PPC_XER_CA;
	uint64_t tmp = (uint32_t)(~reg(ic->arg[0]));
	uint64_t tmp2 = tmp;
	cpu->cd.ppc.spr[SPR_XER] &= ~PPC_XER_CA;
	if (old_ca)
		tmp ++;
	if ((tmp >> 32) != (tmp2 >> 32))
		cpu->cd.ppc.spr[SPR_XER] |= PPC_XER_CA;
	reg(ic->arg[2]) = (uint32_t)tmp;
}
DOT2(subfze)


/*
 *  ori, xori etc.:
 *
 *  arg[0] = pointer to source uint64_t
 *  arg[1] = immediate value (uint32_t or larger)
 *  arg[2] = pointer to destination uint64_t
 */
X(ori)  { reg(ic->arg[2]) = reg(ic->arg[0]) | (uint32_t)ic->arg[1]; }
X(xori) { reg(ic->arg[2]) = reg(ic->arg[0]) ^ (uint32_t)ic->arg[1]; }


#include "tmp_ppc_loadstore.cc"


/*
 *  lfs, stfs: Load/Store Floating-point Single precision
 */
X(lfs)
{
	/*  Sync. PC in case of an exception, and remember it:  */
	uint64_t old_pc, low_pc = ((size_t)ic - (size_t)
	    cpu->cd.ppc.cur_ic_page) / sizeof(struct ppc_instr_call);
	old_pc = cpu->pc = (cpu->pc & ~((PPC_IC_ENTRIES_PER_PAGE-1) <<
	    PPC_INSTR_ALIGNMENT_SHIFT)) + (low_pc << PPC_INSTR_ALIGNMENT_SHIFT);
	if (!(cpu->cd.ppc.msr & PPC_MSR_FP)) {
		ppc_exception(cpu, PPC_EXCEPTION_FPU);
		return;
	}

	/*  Perform a 32-bit load:  */
#ifdef MODE32
	ppc32_loadstore
#else
	ppc_loadstore
#endif
	    [2 + 4 + 8](cpu, ic);

	if (old_pc == cpu->pc) {
		/*  The load succeeded. Let's convert the value:  */
		struct ieee_float_value val;
		(*(uint64_t *)ic->arg[0]) &= 0xffffffff;
		ieee_interpret_float_value(*(uint64_t *)ic->arg[0],
		    &val, IEEE_FMT_S);
		(*(uint64_t *)ic->arg[0]) =
		    ieee_store_float_value(val.f, IEEE_FMT_D, val.nan);
	}
}
X(lfsx)
{
	/*  Sync. PC in case of an exception, and remember it:  */
	uint64_t old_pc, low_pc = ((size_t)ic - (size_t)
	    cpu->cd.ppc.cur_ic_page) / sizeof(struct ppc_instr_call);
	old_pc = cpu->pc = (cpu->pc & ~((PPC_IC_ENTRIES_PER_PAGE-1) <<
	    PPC_INSTR_ALIGNMENT_SHIFT)) + (low_pc << PPC_INSTR_ALIGNMENT_SHIFT);
	if (!(cpu->cd.ppc.msr & PPC_MSR_FP)) {
		ppc_exception(cpu, PPC_EXCEPTION_FPU);
		return;
	}

	/*  Perform a 32-bit load:  */
#ifdef MODE32
	ppc32_loadstore_indexed
#else
	ppc_loadstore_indexed
#endif
	    [2 + 4 + 8](cpu, ic);

	if (old_pc == cpu->pc) {
		/*  The load succeeded. Let's convert the value:  */
		struct ieee_float_value val;
		(*(uint64_t *)ic->arg[0]) &= 0xffffffff;
		ieee_interpret_float_value(*(uint64_t *)ic->arg[0],
		    &val, IEEE_FMT_S);
		(*(uint64_t *)ic->arg[0]) =
		    ieee_store_float_value(val.f, IEEE_FMT_D, val.nan);
	}
}
X(lfd)
{
	CHECK_FOR_FPU_EXCEPTION;

	/*  Perform a 64-bit load:  */
#ifdef MODE32
	ppc32_loadstore
#else
	ppc_loadstore
#endif
	    [3 + 4 + 8](cpu, ic);
}
X(lfdx)
{
	CHECK_FOR_FPU_EXCEPTION;

	/*  Perform a 64-bit load:  */
#ifdef MODE32
	ppc32_loadstore_indexed
#else
	ppc_loadstore_indexed
#endif
	    [3 + 4 + 8](cpu, ic);
}
X(stfs)
{
	uint64_t *old_arg0 = (uint64_t *) ic->arg[0];
	struct ieee_float_value val;
	uint64_t tmp_val;

	CHECK_FOR_FPU_EXCEPTION;

	ieee_interpret_float_value(*old_arg0, &val, IEEE_FMT_D);
	tmp_val = ieee_store_float_value(val.f, IEEE_FMT_S, val.nan);

	ic->arg[0] = (size_t)&tmp_val;

	/*  Perform a 32-bit store:  */
#ifdef MODE32
	ppc32_loadstore
#else
	ppc_loadstore
#endif
	    [2 + 4](cpu, ic);

	ic->arg[0] = (size_t)old_arg0;
}
X(stfsx)
{
	uint64_t *old_arg0 = (uint64_t *)ic->arg[0];
	struct ieee_float_value val;
	uint64_t tmp_val;

	CHECK_FOR_FPU_EXCEPTION;

	ieee_interpret_float_value(*old_arg0, &val, IEEE_FMT_D);
	tmp_val = ieee_store_float_value(val.f, IEEE_FMT_S, val.nan);

	ic->arg[0] = (size_t)&tmp_val;

	/*  Perform a 32-bit store:  */
#ifdef MODE32
	ppc32_loadstore_indexed
#else
	ppc_loadstore_indexed
#endif
	    [2 + 4](cpu, ic);

	ic->arg[0] = (size_t)old_arg0;
}
X(stfd)
{
	CHECK_FOR_FPU_EXCEPTION;

	/*  Perform a 64-bit store:  */
#ifdef MODE32
	ppc32_loadstore
#else
	ppc_loadstore
#endif
	    [3 + 4](cpu, ic);
}
X(stfdx)
{
	CHECK_FOR_FPU_EXCEPTION;

	/*  Perform a 64-bit store:  */
#ifdef MODE32
	ppc32_loadstore_indexed
#else
	ppc_loadstore_indexed
#endif
	    [3 + 4](cpu, ic);
}


/*
 *  lvx, stvx:  Vector (16-byte) load/store  (slow implementation)
 *
 *  arg[0] = v-register nr of rs
 *  arg[1] = pointer to ra
 *  arg[2] = pointer to rb
 */
X(lvx)
{
	MODE_uint_t addr = reg(ic->arg[1]) + reg(ic->arg[2]);
	uint8_t data[16];
	uint64_t hi, lo;
	int rs = ic->arg[0];

	if (cpu->memory_rw(cpu, cpu->mem, addr, data, sizeof(data),
	    MEM_READ, CACHE_DATA) != MEMORY_ACCESS_OK) {
		/*  exception  */
		return;
	}

	hi = ((uint64_t)data[0] << 56) +
	     ((uint64_t)data[1] << 48) +
	     ((uint64_t)data[2] << 40) +
	     ((uint64_t)data[3] << 32) +
	     ((uint64_t)data[4] << 24) +
	     ((uint64_t)data[5] << 16) +
	     ((uint64_t)data[6] << 8) +
	     ((uint64_t)data[7]);
	lo = ((uint64_t)data[8] << 56) +
	     ((uint64_t)data[9] << 48) +
	     ((uint64_t)data[10] << 40) +
	     ((uint64_t)data[11] << 32) +
	     ((uint64_t)data[12] << 24) +
	     ((uint64_t)data[13] << 16) +
	     ((uint64_t)data[14] << 8) +
	     ((uint64_t)data[15]);

	cpu->cd.ppc.vr_hi[rs] = hi; cpu->cd.ppc.vr_lo[rs] = lo;
}
X(stvx)
{
	uint8_t data[16];
	MODE_uint_t addr = reg(ic->arg[1]) + reg(ic->arg[2]);
	int rs = ic->arg[0];
	uint64_t hi = cpu->cd.ppc.vr_hi[rs], lo = cpu->cd.ppc.vr_lo[rs];

	data[0] = hi >> 56;
	data[1] = hi >> 48;
	data[2] = hi >> 40;
	data[3] = hi >> 32;
	data[4] = hi >> 24;
	data[5] = hi >> 16;
	data[6] = hi >> 8;
	data[7] = hi;
	data[8] = lo >> 56;
	data[9] = lo >> 48;
	data[10] = lo >> 40;
	data[11] = lo >> 32;
	data[12] = lo >> 24;
	data[13] = lo >> 16;
	data[14] = lo >> 8;
	data[15] = lo;

	cpu->memory_rw(cpu, cpu->mem, addr, data,
	    sizeof(data), MEM_WRITE, CACHE_DATA);
}


/*
 *  vxor:  Vector (16-byte) XOR
 *
 *  arg[0] = v-register nr of source 1
 *  arg[1] = v-register nr of source 2
 *  arg[2] = v-register nr of destination
 */
X(vxor)
{
	cpu->cd.ppc.vr_hi[ic->arg[2]] =
	    cpu->cd.ppc.vr_hi[ic->arg[0]] ^ cpu->cd.ppc.vr_hi[ic->arg[1]];
	cpu->cd.ppc.vr_lo[ic->arg[2]] =
	    cpu->cd.ppc.vr_lo[ic->arg[0]] ^ cpu->cd.ppc.vr_lo[ic->arg[1]];
}


/*
 *  tlbia:  TLB invalidate all
 */
X(tlbia)
{
	fatal("[ tlbia ]\n");
	cpu->invalidate_translation_caches(cpu, 0, INVALIDATE_ALL);
}


/*
 *  tlbie:  TLB invalidate
 */
X(tlbie)
{
	/*  fatal("[ tlbie ]\n");  */
	cpu->invalidate_translation_caches(cpu, reg(ic->arg[0]),
	    INVALIDATE_VADDR);
}


/*
 *  sc: Syscall.
 */
X(sc)
{
	/*  Synchronize the PC (pointing to _after_ this instruction)  */
	cpu->pc = (cpu->pc & ~0xfff) + ic->arg[1];

	ppc_exception(cpu, PPC_EXCEPTION_SC);

	/*  This caused an update to the PC register, so there is no need
	    to worry about the next instruction being an end_of_page.  */
}


/*
 *  openfirmware:
 */
X(openfirmware)
{
	of_emul(cpu);
	if (cpu->running == 0) {
		cpu->n_translated_instrs --;
		cpu->cd.ppc.next_ic = &nothing_call;
	}
	cpu->pc = cpu->cd.ppc.spr[SPR_LR];
	if (cpu->machine->show_trace_tree)
		cpu_functioncall_trace_return(cpu);
	quick_pc_to_pointers(cpu);
}


/*
 *  tlbsx_dot: TLB scan
 */
X(tlbsx_dot)
{
	/*  TODO  */
	cpu->cd.ppc.cr &= ~(0xf0000000);
	cpu->cd.ppc.cr |= 0x20000000;
	cpu->cd.ppc.cr |= ((cpu->cd.ppc.spr[SPR_XER] >> 3) & 0x10000000);
}


/*
 *  tlbli:
 */
X(tlbli)
{
	fatal("tlbli\n");
	cpu->invalidate_translation_caches(cpu, 0, INVALIDATE_ALL);
}


/*
 *  tlbld:
 */
X(tlbld)
{
	/*  MODE_uint_t vaddr = reg(ic->arg[0]);
	    MODE_uint_t paddr = cpu->cd.ppc.spr[SPR_RPA];  */

	fatal("tlbld\n");
	cpu->invalidate_translation_caches(cpu, 0, INVALIDATE_ALL);
}


/*****************************************************************************/


X(end_of_page)
{
	/*  Update the PC:  (offset 0, but on the next page)  */
	cpu->pc &= ~((PPC_IC_ENTRIES_PER_PAGE-1) << PPC_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (PPC_IC_ENTRIES_PER_PAGE << PPC_INSTR_ALIGNMENT_SHIFT);

	/*  Find the new physical page and update the translation pointers:  */
	quick_pc_to_pointers(cpu);

	/*  end_of_page doesn't count as an executed instruction:  */
	cpu->n_translated_instrs --;
}


/*****************************************************************************/


/*
 *  ppc_instr_to_be_translated():
 *
 *  Translate an instruction word into a ppc_instr_call. ic is filled in with
 *  valid data for the translated instruction, or a "nothing" instruction if
 *  there was a translation failure. The newly translated instruction is then
 *  executed.
 */
X(to_be_translated)
{
	uint64_t addr, low_pc, tmp_addr;
	uint32_t iword, mask;
	unsigned char *page;
	unsigned char ib[4];
	int main_opcode, rt, rs, ra, rb, rc, aa_bit, l_bit, lk_bit, spr, sh,
	    xo, imm, load, size, update, zero, bf, bo, bi, bh, oe_bit, n64=0,
	    bfa, fp, byterev, nb, mb, me;
	void (*samepage_function)(struct cpu *, struct ppc_instr_call *);
	void (*rc_f)(struct cpu *, struct ppc_instr_call *);

	/*  Figure out the (virtual) address of the instruction:  */
	low_pc = ((size_t)ic - (size_t)cpu->cd.ppc.cur_ic_page)
	    / sizeof(struct ppc_instr_call);
	addr = cpu->pc & ~((PPC_IC_ENTRIES_PER_PAGE-1)
	    << PPC_INSTR_ALIGNMENT_SHIFT);
	addr += (low_pc << PPC_INSTR_ALIGNMENT_SHIFT);
	cpu->pc = addr;
	addr &= ~((1 << PPC_INSTR_ALIGNMENT_SHIFT) - 1);

	/*  Read the instruction word from memory:  */
#ifdef MODE32
	page = cpu->cd.ppc.host_load[((uint32_t)addr) >> 12];
#else
	{
		const uint32_t mask1 = (1 << DYNTRANS_L1N) - 1;
		const uint32_t mask2 = (1 << DYNTRANS_L2N) - 1;
		const uint32_t mask3 = (1 << DYNTRANS_L3N) - 1;
		uint32_t x1 = (addr >> (64-DYNTRANS_L1N)) & mask1;
		uint32_t x2 = (addr >> (64-DYNTRANS_L1N-DYNTRANS_L2N)) & mask2;
		uint32_t x3 = (addr >> (64-DYNTRANS_L1N-DYNTRANS_L2N-
		    DYNTRANS_L3N)) & mask3;
		struct DYNTRANS_L2_64_TABLE *l2 = cpu->cd.ppc.l1_64[x1];
		struct DYNTRANS_L3_64_TABLE *l3 = l2->l3[x2];
		page = l3->host_load[x3];
	}
#endif

	if (page != NULL) {
		/*  fatal("TRANSLATION HIT!\n");  */
		memcpy(ib, page + (addr & 0xfff), sizeof(ib));
	} else {
		/*  fatal("TRANSLATION MISS!\n");  */
		if (!cpu->memory_rw(cpu, cpu->mem, addr, ib,
		    sizeof(ib), MEM_READ, CACHE_INSTRUCTION)) {
			fatal("PPC to_be_translated(): "
			    "read failed: TODO\n");
			exit(1);
			/*  goto bad;  */
		}
	}

	{
		uint32_t *p = (uint32_t *) ib;
		iword = *p;
		iword = BE32_TO_HOST(iword);
	}

#define DYNTRANS_TO_BE_TRANSLATED_HEAD
#include "cpu_dyntrans.cc"
#undef  DYNTRANS_TO_BE_TRANSLATED_HEAD


	/*
	 *  Translate the instruction:
	 */

	main_opcode = iword >> 26;

	switch (main_opcode) {

	case 0x04:
		if (iword == 0x12739cc4) {
			/*  vxor v19,v19,v19  */
			ic->f = instr(vxor);
			ic->arg[0] = 19;
			ic->arg[1] = 19;
			ic->arg[2] = 19;
		} else {
			if (!cpu->translation_readahead)
				fatal("[ TODO: Unimplemented ALTIVEC, iword"
				    " = 0x%08"PRIx32"x ]\n", iword);
			goto bad;
		}
		break;

	case PPC_HI6_MULLI:
		rt = (iword >> 21) & 31;
		ra = (iword >> 16) & 31;
		imm = (int16_t)(iword & 0xffff);
		ic->f = instr(mulli);
		ic->arg[0] = (size_t)(&cpu->cd.ppc.gpr[ra]);
		ic->arg[1] = (ssize_t)imm;
		ic->arg[2] = (size_t)(&cpu->cd.ppc.gpr[rt]);
		break;

	case PPC_HI6_SUBFIC:
		rt = (iword >> 21) & 31;
		ra = (iword >> 16) & 31;
		imm = (int16_t)(iword & 0xffff);
		ic->f = instr(subfic);
		ic->arg[0] = (size_t)(&cpu->cd.ppc.gpr[ra]);
		ic->arg[1] = (ssize_t)imm;
		ic->arg[2] = (size_t)(&cpu->cd.ppc.gpr[rt]);
		break;

	case PPC_HI6_CMPLI:
	case PPC_HI6_CMPI:
		bf = (iword >> 23) & 7;
		l_bit = (iword >> 21) & 1;
		ra = (iword >> 16) & 31;
		if (main_opcode == PPC_HI6_CMPLI) {
			imm = iword & 0xffff;
			if (l_bit)
				ic->f = instr(cmpldi);
			else
				ic->f = instr(cmplwi);
		} else {
			imm = (int16_t)(iword & 0xffff);
			if (l_bit)
				ic->f = instr(cmpdi);
			else {
				if (bf == 0)
					ic->f = instr(cmpwi_cr0);
				else
					ic->f = instr(cmpwi);
			}
		}
		ic->arg[0] = (size_t)(&cpu->cd.ppc.gpr[ra]);
		ic->arg[1] = (ssize_t)imm;
		ic->arg[2] = 28 - 4 * bf;
		break;

	case PPC_HI6_ADDIC:
	case PPC_HI6_ADDIC_DOT:
		if (cpu->cd.ppc.bits == 64) {
			if (!cpu->translation_readahead)
				fatal("addic for 64-bit: TODO\n");
			goto bad;
		}
		rt = (iword >> 21) & 31;
		ra = (iword >> 16) & 31;
		imm = (int16_t)(iword & 0xffff);
		if (main_opcode == PPC_HI6_ADDIC)
			ic->f = instr(addic);
		else
			ic->f = instr(addic_dot);
		ic->arg[0] = (size_t)(&cpu->cd.ppc.gpr[ra]);
		ic->arg[1] = imm;
		ic->arg[2] = (size_t)(&cpu->cd.ppc.gpr[rt]);
		break;

	case PPC_HI6_ADDI:
	case PPC_HI6_ADDIS:
		rt = (iword >> 21) & 31; ra = (iword >> 16) & 31;
		ic->f = instr(addi);
		if (ra == 0)
			ic->f = instr(li);
		else
			ic->arg[0] = (size_t)(&cpu->cd.ppc.gpr[ra]);
		ic->arg[1] = (int16_t)(iword & 0xffff);
		if (main_opcode == PPC_HI6_ADDIS)
			ic->arg[1] <<= 16;
		if (ra == 0 && ic->arg[1] == 0)
			ic->f = instr(li_0);
		ic->arg[2] = (size_t)(&cpu->cd.ppc.gpr[rt]);
		break;

	case PPC_HI6_ANDI_DOT:
	case PPC_HI6_ANDIS_DOT:
		rs = (iword >> 21) & 31; ra = (iword >> 16) & 31;
		ic->f = instr(andi_dot);
		ic->arg[0] = (size_t)(&cpu->cd.ppc.gpr[rs]);
		ic->arg[1] = iword & 0xffff;
		if (main_opcode == PPC_HI6_ANDIS_DOT)
			ic->arg[1] <<= 16;
		ic->arg[2] = (size_t)(&cpu->cd.ppc.gpr[ra]);
		break;

	case PPC_HI6_ORI:
	case PPC_HI6_ORIS:
	case PPC_HI6_XORI:
	case PPC_HI6_XORIS:
		rs = (iword >> 21) & 31; ra = (iword >> 16) & 31;
		if (main_opcode == PPC_HI6_ORI ||
		    main_opcode == PPC_HI6_ORIS)
			ic->f = instr(ori);
		else
			ic->f = instr(xori);
		ic->arg[0] = (size_t)(&cpu->cd.ppc.gpr[rs]);
		ic->arg[1] = iword & 0xffff;
		if (main_opcode == PPC_HI6_ORIS ||
		    main_opcode == PPC_HI6_XORIS)
			ic->arg[1] <<= 16;
		ic->arg[2] = (size_t)(&cpu->cd.ppc.gpr[ra]);
		break;

	case PPC_HI6_LBZ:
	case PPC_HI6_LBZU:
	case PPC_HI6_LHZ:
	case PPC_HI6_LHZU:
	case PPC_HI6_LHA:
	case PPC_HI6_LHAU:
	case PPC_HI6_LWZ:
	case PPC_HI6_LWZU:
	case PPC_HI6_LD:
	case PPC_HI6_LFD:
	case PPC_HI6_LFS:
	case PPC_HI6_STB:
	case PPC_HI6_STBU:
	case PPC_HI6_STH:
	case PPC_HI6_STHU:
	case PPC_HI6_STW:
	case PPC_HI6_STWU:
	case PPC_HI6_STD:
	case PPC_HI6_STFD:
	case PPC_HI6_STFS:
		rs = (iword >> 21) & 31;
		ra = (iword >> 16) & 31;
		imm = (int16_t)iword;
		load = 0; zero = 1; size = 0; update = 0; fp = 0;
		ic->f = NULL;
		switch (main_opcode) {
		case PPC_HI6_LBZ:  load=1; break;
		case PPC_HI6_LBZU: load=1; update=1; break;
		case PPC_HI6_LHA:  load=1; size=1; zero=0; break;
		case PPC_HI6_LHAU: load=1; size=1; zero=0; update=1; break;
		case PPC_HI6_LHZ:  load=1; size=1; break;
		case PPC_HI6_LHZU: load=1; size=1; update=1; break;
		case PPC_HI6_LWZ:  load=1; size=2; break;
		case PPC_HI6_LWZU: load=1; size=2; update=1; break;
		case PPC_HI6_LD:   load=1; size=3; break;
		case PPC_HI6_LFD:  load=1; size=3; fp=1;ic->f=instr(lfd);break;
		case PPC_HI6_LFS:  load=1; size=2; fp=1;ic->f=instr(lfs);break;
		case PPC_HI6_STB:  break;
		case PPC_HI6_STBU: update=1; break;
		case PPC_HI6_STH:  size=1; break;
		case PPC_HI6_STHU: size=1; update=1; break;
		case PPC_HI6_STW:  size=2; break;
		case PPC_HI6_STWU: size=2; update=1; break;
		case PPC_HI6_STD:  size=3; break;
		case PPC_HI6_STFD: size=3; fp=1; ic->f = instr(stfd); break;
		case PPC_HI6_STFS: size=2; fp=1; ic->f = instr(stfs); break;
		}
		if (ic->f == NULL) {
			ic->f =
#ifdef MODE32
			    ppc32_loadstore
#else
			    ppc_loadstore
#endif
			    [size + 4*zero + 8*load + (imm==0? 16 : 0)
			    + 32*update];
		}
		if (ra == 0 && update) {
			if (!cpu->translation_readahead)
				fatal("TODO: ra=0 && update?\n");
			goto bad;
		}
		if (fp)
			ic->arg[0] = (size_t)(&cpu->cd.ppc.fpr[rs]);
		else
			ic->arg[0] = (size_t)(&cpu->cd.ppc.gpr[rs]);
		if (ra == 0)
			ic->arg[1] = (size_t)(&cpu->cd.ppc.zero);
		else
			ic->arg[1] = (size_t)(&cpu->cd.ppc.gpr[ra]);
		ic->arg[2] = (ssize_t)imm;
		break;

	case PPC_HI6_BC:
		aa_bit = (iword >> 1) & 1;
		lk_bit = iword & 1;
		bo = (iword >> 21) & 31;
		bi = (iword >> 16) & 31;
		tmp_addr = (int64_t)(int16_t)(iword & 0xfffc);
		if (aa_bit) {
			if (!cpu->translation_readahead)
				fatal("aa_bit: NOT YET\n");
			goto bad;
		}
		if (lk_bit) {
			ic->f = instr(bcl);
			samepage_function = instr(bcl_samepage);
		} else {
			ic->f = instr(bc);
			if ((bo & 0x14) == 0x04) {
				samepage_function = bo & 8?
				    instr(bc_samepage_simple1) :
				    instr(bc_samepage_simple0);
			} else
				samepage_function = instr(bc_samepage);
		}
		ic->arg[0] = (ssize_t)(tmp_addr + (addr & 0xffc));
		ic->arg[1] = bo;
		ic->arg[2] = 31-bi;
		/*  Branches are calculated as cur PC + offset.  */
		/*  Special case: branch within the same page:  */
		{
			uint64_t mask_within_page =
			    ((PPC_IC_ENTRIES_PER_PAGE-1) << 2) | 3;
			uint64_t old_pc = addr;
			uint64_t new_pc = old_pc + (int32_t)tmp_addr;
			if ((old_pc & ~mask_within_page) ==
			    (new_pc & ~mask_within_page)) {
				ic->f = samepage_function;
				ic->arg[0] = (size_t) (
				    cpu->cd.ppc.cur_ic_page +
				    ((new_pc & mask_within_page) >> 2));
			}
		}
		break;

	case PPC_HI6_SC:
		ic->arg[0] = (iword >> 5) & 0x7f;
		ic->arg[1] = (addr & 0xfff) + 4;
		if (iword == 0x44ee0002) {
			/*  Special case/magic hack for OpenFirmware emul:  */
			ic->f = instr(openfirmware);
		} else
			ic->f = instr(sc);
		break;

	case PPC_HI6_B:
		aa_bit = (iword & 2) >> 1;
		lk_bit = iword & 1;
		tmp_addr = (int64_t)(int32_t)((iword & 0x03fffffc) << 6);
		tmp_addr = (int64_t)tmp_addr >> 6;
		if (lk_bit) {
			if (cpu->machine->show_trace_tree) {
				ic->f = instr(bl_trace);
				samepage_function = instr(bl_samepage_trace);
			} else {
				ic->f = instr(bl);
				samepage_function = instr(bl_samepage);
			}
		} else {
			ic->f = instr(b);
			samepage_function = instr(b_samepage);
		}
		ic->arg[0] = (ssize_t)(tmp_addr + (addr & 0xffc));
		ic->arg[1] = (addr & 0xffc) + 4;
		/*  Branches are calculated as cur PC + offset.  */
		/*  Special case: branch within the same page:  */
		{
			uint64_t mask_within_page =
			    ((PPC_IC_ENTRIES_PER_PAGE-1) << 2) | 3;
			uint64_t old_pc = addr;
			uint64_t new_pc = old_pc + (int32_t)tmp_addr;
			if ((old_pc & ~mask_within_page) ==
			    (new_pc & ~mask_within_page)) {
				ic->f = samepage_function;
				ic->arg[0] = (size_t) (
				    cpu->cd.ppc.cur_ic_page +
				    ((new_pc & mask_within_page) >> 2));
			}
		}
		if (aa_bit) {
			if (lk_bit) {
				if (cpu->machine->show_trace_tree) {
					ic->f = instr(bla_trace);
				} else {
					ic->f = instr(bla);
				}
			} else {
				ic->f = instr(ba);
			}
			ic->arg[0] = (ssize_t)tmp_addr;
		}
		break;

	case PPC_HI6_19:
		xo = (iword >> 1) & 1023;
		switch (xo) {

		case PPC_19_BCLR:
		case PPC_19_BCCTR:
			bo = (iword >> 21) & 31;
			bi = (iword >> 16) & 31;
			bh = (iword >> 11) & 3;
			lk_bit = iword & 1;
			if (xo == PPC_19_BCLR) {
				if (lk_bit)
					ic->f = instr(bclr_l);
				else {
					ic->f = instr(bclr);
					if (!cpu->machine->show_trace_tree &&
					    (bo & 0x14) == 0x14)
						ic->f = instr(bclr_20);
				}
			} else {
				if (!(bo & 4)) {
					if (!cpu->translation_readahead)
						fatal("TODO: bclr/bcctr "
						    "bo bit 2 clear!\n");
					goto bad;
				}
				if (lk_bit)
					ic->f = instr(bcctr_l);
				else
					ic->f = instr(bcctr);
			}
			ic->arg[0] = bo;
			ic->arg[1] = 31 - bi;
			ic->arg[2] = bh;
			break;

		case PPC_19_ISYNC:
			/*  TODO  */
			ic->f = instr(nop);
			break;

		case PPC_19_RFI:
			ic->f = instr(rfi);
			break;

		case PPC_19_RFID:
			ic->f = instr(rfid);
			break;

		case PPC_19_MCRF:
			bf = (iword >> 23) & 7;
			bfa = (iword >> 18) & 7;
			ic->arg[0] = 28 - 4*bf;
			ic->arg[1] = 28 - 4*bfa;
			ic->f = instr(mcrf);
			break;

		case PPC_19_CRAND:
		case PPC_19_CRANDC:
		case PPC_19_CREQV:
		case PPC_19_CROR:
		case PPC_19_CRORC:
		case PPC_19_CRNOR:
		case PPC_19_CRXOR:
			switch (xo) {
			case PPC_19_CRAND:  ic->f = instr(crand); break;
			case PPC_19_CRANDC: ic->f = instr(crandc); break;
			case PPC_19_CREQV:  ic->f = instr(creqv); break;
			case PPC_19_CROR:   ic->f = instr(cror); break;
			case PPC_19_CRORC:  ic->f = instr(crorc); break;
			case PPC_19_CRNOR:  ic->f = instr(crnor); break;
			case PPC_19_CRXOR:  ic->f = instr(crxor); break;
			}
			ic->arg[0] = iword;
			break;

		default:goto bad;
		}
		break;

	case PPC_HI6_RLWNM:
	case PPC_HI6_RLWINM:
		ra = (iword >> 16) & 31;
		mb = (iword >> 6) & 31;
		me = (iword >> 1) & 31;   
		rc = iword & 1;
		mask = 0;
		for (;;) {
			mask |= ((uint32_t)0x80000000 >> mb);
			if (mb == me)
				break;
			mb ++; mb &= 31;
		}
		switch (main_opcode) {
		case PPC_HI6_RLWNM:
			ic->f = rc? instr(rlwnm_dot) : instr(rlwnm); break;
		case PPC_HI6_RLWINM:
			ic->f = rc? instr(rlwinm_dot) : instr(rlwinm); break;
		}
		ic->arg[0] = (size_t)(&cpu->cd.ppc.gpr[ra]);
		ic->arg[1] = mask;
		ic->arg[2] = (uint32_t)iword;
		break;

	case PPC_HI6_RLWIMI:
		rs = (iword >> 21) & 31;
		ra = (iword >> 16) & 31;
		ic->f = instr(rlwimi);
		ic->arg[0] = (size_t)(&cpu->cd.ppc.gpr[rs]);
		ic->arg[1] = (size_t)(&cpu->cd.ppc.gpr[ra]);
		ic->arg[2] = (uint32_t)iword;
		break;

	case PPC_HI6_LMW:
	case PPC_HI6_STMW:
		/*  NOTE: Loads use rt, not rs.  */
		rs = (iword >> 21) & 31;
		ra = (iword >> 16) & 31;
		ic->arg[0] = rs;
		if (ra == 0)
			ic->arg[1] = (size_t)(&cpu->cd.ppc.zero);
		else
			ic->arg[1] = (size_t)(&cpu->cd.ppc.gpr[ra]);
		ic->arg[2] = (int32_t)(int16_t)iword;
		switch (main_opcode) {
		case PPC_HI6_LMW:
			ic->f = instr(lmw);
			break;
		case PPC_HI6_STMW:
			ic->f = instr(stmw);
			break;
		}
		break;

	case PPC_HI6_30:
		xo = (iword >> 2) & 7;
		switch (xo) {

		case PPC_30_RLDICL:
		case PPC_30_RLDICR:
		case PPC_30_RLDIMI:
			switch (xo) {
			case PPC_30_RLDICL: ic->f = instr(rldicl); break;
			case PPC_30_RLDICR: ic->f = instr(rldicr); break;
			case PPC_30_RLDIMI: ic->f = instr(rldimi); break;
			}
			ic->arg[0] = iword;
			if (cpu->cd.ppc.bits == 32) {
				if (!cpu->translation_readahead)
					fatal("TODO: rld* in 32-bit mode?\n");
				goto bad;
			}
			break;

		default:goto bad;
		}
		break;

	case PPC_HI6_31:
		xo = (iword >> 1) & 1023;
		switch (xo) {

		case PPC_31_CMPL:
		case PPC_31_CMP:
			bf = (iword >> 23) & 7;
			l_bit = (iword >> 21) & 1;
			ra = (iword >> 16) & 31;
			rb = (iword >> 11) & 31;
			if (xo == PPC_31_CMPL) {
				if (l_bit)
					ic->f = instr(cmpld);
				else
					ic->f = instr(cmplw);
			} else {
				if (l_bit)
					ic->f = instr(cmpd);
				else {
					if (bf == 0)
						ic->f = instr(cmpw_cr0);
					else
						ic->f = instr(cmpw);
				}
			}
			ic->arg[0] = (size_t)(&cpu->cd.ppc.gpr[ra]);
			ic->arg[1] = (size_t)(&cpu->cd.ppc.gpr[rb]);
			ic->arg[2] = 28 - 4*bf;
			break;

		case PPC_31_CNTLZW:
			rs = (iword >> 21) & 31;
			ra = (iword >> 16) & 31;
			rc = iword & 1;
			if (rc) {
				if (!cpu->translation_readahead)
					fatal("TODO: rc\n");
				goto bad;
			}
			ic->arg[0] = (size_t)(&cpu->cd.ppc.gpr[rs]);
			ic->arg[1] = (size_t)(&cpu->cd.ppc.gpr[ra]);
			ic->f = instr(cntlzw);
			break;

		case PPC_31_MFSPR:
			rt = (iword >> 21) & 31;
			spr = ((iword >> 6) & 0x3e0) + ((iword >> 16) & 31);
			debug_spr_usage(cpu->pc, spr);
			ic->arg[0] = (size_t)(&cpu->cd.ppc.gpr[rt]);
			ic->arg[1] = (size_t)(&cpu->cd.ppc.spr[spr]);
			switch (spr) {
			case SPR_PMC1:	ic->f = instr(mfspr_pmc1); break;
			default:	ic->f = instr(mfspr);
			}
			break;

		case PPC_31_MTSPR:
			rs = (iword >> 21) & 31;
			spr = ((iword >> 6) & 0x3e0) + ((iword >> 16) & 31);
			debug_spr_usage(cpu->pc, spr);
			ic->arg[0] = (size_t)(&cpu->cd.ppc.gpr[rs]);
			ic->arg[1] = (size_t)(&cpu->cd.ppc.spr[spr]);
			switch (spr) {
			case SPR_LR:
				ic->f = instr(mtlr);
				break;
			case SPR_CTR:
				ic->f = instr(mtctr);
				break;
			default:ic->f = instr(mtspr);
			}
			break;

		case PPC_31_MFCR:
			rt = (iword >> 21) & 31;
			ic->arg[0] = (size_t)(&cpu->cd.ppc.gpr[rt]);
			ic->f = instr(mfcr);
			break;

		case PPC_31_MFMSR:
			rt = (iword >> 21) & 31;
			ic->arg[0] = (size_t)(&cpu->cd.ppc.gpr[rt]);
			ic->f = instr(mfmsr);
			break;

		case PPC_31_MTMSR:
		case PPC_31_MTMSRD:
			rs = (iword >> 21) & 31;
			l_bit = (iword >> 16) & 1;
			if (l_bit) {
				if (!cpu->translation_readahead)
					fatal("TODO: mtmsr l-bit\n");
				goto bad;
			}
			ic->arg[0] = (size_t)(&cpu->cd.ppc.gpr[rs]);
			ic->arg[1] = (addr & 0xfff) + 4;
			ic->arg[2] = xo == PPC_31_MTMSRD;
			ic->f = instr(mtmsr);
			break;

		case PPC_31_MTCRF:
			rs = (iword >> 21) & 31;
			{
				int i, fxm = (iword >> 12) & 255;
				uint32_t tmp = 0;
				for (i=0; i<8; i++, fxm <<= 1) {
					tmp <<= 4;
					if (fxm & 128)
						tmp |= 0xf;
				}
				ic->arg[1] = (uint32_t)tmp;
			}
			ic->arg[0] = (size_t)(&cpu->cd.ppc.gpr[rs]);
			ic->f = instr(mtcrf);
			break;

		case PPC_31_MFSRIN:
		case PPC_31_MTSRIN:
			rt = (iword >> 21) & 31;
			rb = (iword >> 11) & 31;
			ic->arg[0] = (size_t)(&cpu->cd.ppc.gpr[rb]);
			ic->arg[1] = (size_t)(&cpu->cd.ppc.gpr[rt]);
			switch (xo) {
			case PPC_31_MFSRIN: ic->f = instr(mfsrin); break;
			case PPC_31_MTSRIN: ic->f = instr(mtsrin); break;
			}
			if (cpu->cd.ppc.bits == 64) {
				if (!cpu->translation_readahead)
					fatal("Not yet for 64-bit mode\n");
				goto bad;
			}
			break;

		case PPC_31_MFSR:
		case PPC_31_MTSR:
			rt = (iword >> 21) & 31;
			ic->arg[0] = (iword >> 16) & 15;
			ic->arg[1] = (size_t)(&cpu->cd.ppc.gpr[rt]);
			switch (xo) {
			case PPC_31_MFSR:   ic->f = instr(mfsr); break;
			case PPC_31_MTSR:   ic->f = instr(mtsr); break;
			}
			if (cpu->cd.ppc.bits == 64) {
				if (!cpu->translation_readahead)
					fatal("Not yet for 64-bit mode\n");
				goto bad;
			}
			break;

		case PPC_31_SRAWI:
			rs = (iword >> 21) & 31;
			ra = (iword >> 16) & 31;
			sh = (iword >> 11) & 31;
			rc = iword & 1;
			ic->arg[0] = (size_t)(&cpu->cd.ppc.gpr[rs]);
			ic->arg[1] = (size_t)(&cpu->cd.ppc.gpr[ra]);
			ic->arg[2] = sh;
			if (rc)
				ic->f = instr(srawi_dot);
			else
				ic->f = instr(srawi);
			break;

		case PPC_31_SYNC:
		case PPC_31_DSSALL:
		case PPC_31_EIEIO:
		case PPC_31_DCBST:
		case PPC_31_DCBTST:
		case PPC_31_DCBF:
		case PPC_31_DCBT:
		case PPC_31_ICBI:
			ic->f = instr(nop);
			break;

		case PPC_31_DCBZ:
			ra = (iword >> 16) & 31;
			rb = (iword >> 11) & 31;
			if (ra == 0)
				ic->arg[0] = (size_t)(&cpu->cd.ppc.zero);
			else
				ic->arg[0] = (size_t)(&cpu->cd.ppc.gpr[ra]);
			ic->arg[1] = (size_t)(&cpu->cd.ppc.gpr[rb]);
			ic->arg[2] = addr & 0xfff;
			ic->f = instr(dcbz);
			break;

		case PPC_31_TLBIA:
			ic->f = instr(tlbia);
			break;

		case PPC_31_TLBSYNC:
			/*  According to IBM, "Ensures that a tlbie and
			    tlbia instruction executed by one processor has
			    completed on all other processors.", which in
			    GXemul means a nop :-)  */
			ic->f = instr(nop);
			break;

		case PPC_31_TLBIE:
			/*  TODO: POWER also uses ra?  */
			rb = (iword >> 11) & 31;
			ic->arg[0] = (size_t)(&cpu->cd.ppc.gpr[rb]);
			ic->f = instr(tlbie);
			break;

		case PPC_31_TLBLD:	/*  takes an arg  */
			rb = (iword >> 11) & 31;
			ic->arg[0] = (size_t)(&cpu->cd.ppc.gpr[rb]);
			ic->f = instr(tlbld);
			break;

		case PPC_31_TLBLI:	/*  takes an arg  */
			rb = (iword >> 11) & 31;
			ic->arg[0] = (size_t)(&cpu->cd.ppc.gpr[rb]);
			ic->f = instr(tlbli);
			break;

		case PPC_31_TLBSX_DOT:
			/*  TODO  */
			ic->f = instr(tlbsx_dot);
			break;

		case PPC_31_MFTB:
			rt = (iword >> 21) & 31;
			spr = ((iword >> 6) & 0x3e0) + ((iword >> 16) & 31);
			ic->arg[0] = (size_t)(&cpu->cd.ppc.gpr[rt]);
			switch (spr) {
			case 268: ic->f = instr(mftb); break;
			case 269: ic->f = instr(mftbu); break;
			default:if (!cpu->translation_readahead)
					fatal("mftb spr=%i?\n", spr);
				goto bad;
			}
			break;

		case PPC_31_NEG:
			rt = (iword >> 21) & 31;
			ra = (iword >> 16) & 31;
			rc = iword & 1;
			ic->arg[0] = (size_t)(&cpu->cd.ppc.gpr[ra]);
			ic->arg[1] = (size_t)(&cpu->cd.ppc.gpr[rt]);
			if (rc)
				ic->f = instr(neg_dot);
			else
				ic->f = instr(neg);
			break;

		case PPC_31_LWARX:
		case PPC_31_LDARX:
		case PPC_31_STWCX_DOT:
		case PPC_31_STDCX_DOT:
			ic->arg[0] = iword;
			ic->f = instr(llsc);
			break;

		case PPC_31_LSWI:
		case PPC_31_STSWI:
			rs = (iword >> 21) & 31;
			ra = (iword >> 16) & 31;
			nb = (iword >> 11) & 31;
			ic->arg[0] = rs;
			if (ra == 0)
				ic->arg[1] = (size_t)(&cpu->cd.ppc.zero);
			else
				ic->arg[1] = (size_t)(&cpu->cd.ppc.gpr[ra]);
			ic->arg[2] = nb == 0? 32 : nb;
			switch (xo) {
			case PPC_31_LSWI:  ic->f = instr(lswi); break;
			case PPC_31_STSWI: ic->f = instr(stswi); break;
			}
			break;

		case PPC_31_WRTEEI:
			ic->arg[0] = iword & 0x8000;
			ic->f = instr(wrteei);
			break;

		case 0x1c3:
			fatal("[ mtdcr: TODO ]\n");
			ic->f = instr(nop);
			break;

		case PPC_31_LBZX:
		case PPC_31_LBZUX:
		case PPC_31_LHAX:
		case PPC_31_LHAUX:
		case PPC_31_LHZX:
		case PPC_31_LHZUX:
		case PPC_31_LWZX:
		case PPC_31_LWZUX:
		case PPC_31_LHBRX:
		case PPC_31_LWBRX:
		case PPC_31_LFDX:
		case PPC_31_LFSX:
		case PPC_31_STBX:
		case PPC_31_STBUX:
		case PPC_31_STHX:
		case PPC_31_STHUX:
		case PPC_31_STWX:
		case PPC_31_STWUX:
		case PPC_31_STDX:
		case PPC_31_STDUX:
		case PPC_31_STHBRX:
		case PPC_31_STWBRX:
		case PPC_31_STFDX:
		case PPC_31_STFSX:
			rs = (iword >> 21) & 31;
			ra = (iword >> 16) & 31;
			rb = (iword >> 11) & 31;
			if (ra == 0)
				ic->arg[1] = (size_t)(&cpu->cd.ppc.zero);
			else
				ic->arg[1] = (size_t)(&cpu->cd.ppc.gpr[ra]);
			ic->arg[2] = (size_t)(&cpu->cd.ppc.gpr[rb]);
			load = 0; zero = 1; size = 0; update = 0;
			byterev = 0; fp = 0;
			ic->f = NULL;
			switch (xo) {
			case PPC_31_LBZX:  load = 1; break;
			case PPC_31_LBZUX: load=update=1; break;
			case PPC_31_LHAX:  size=1; load=1; zero=0; break;
			case PPC_31_LHAUX: size=1; load=update=1; zero=0; break;
			case PPC_31_LHZX:  size=1; load=1; break;
			case PPC_31_LHZUX: size=1; load=update = 1; break;
			case PPC_31_LWZX:  size=2; load=1; break;
			case PPC_31_LWZUX: size=2; load=update = 1; break;
			case PPC_31_LHBRX: size=1; load=1; byterev=1;
					   ic->f = instr(lhbrx); break;
			case PPC_31_LWBRX: size=2; load=1; byterev=1;
					   ic->f = instr(lwbrx); break;
			case PPC_31_LFDX:  size=3; load=1; fp=1;
					   ic->f = instr(lfdx); break;
			case PPC_31_LFSX:  size=2; load=1; fp=1;
					   ic->f = instr(lfsx); break;
			case PPC_31_STBX:  break;
			case PPC_31_STBUX: update = 1; break;
			case PPC_31_STHX:  size=1; break;
			case PPC_31_STHUX: size=1; update = 1; break;
			case PPC_31_STWX:  size=2; break;
			case PPC_31_STWUX: size=2; update = 1; break;
			case PPC_31_STDX:  size=3; break;
			case PPC_31_STDUX: size=3; update = 1; break;
			case PPC_31_STHBRX:size=1; byterev = 1;
					   ic->f = instr(sthbrx); break;
			case PPC_31_STWBRX:size=2; byterev = 1;
					   ic->f = instr(stwbrx); break;
			case PPC_31_STFDX: size=3; fp=1;
					   ic->f = instr(stfdx); break;
			case PPC_31_STFSX: size=2; fp=1;
					   ic->f = instr(stfsx); break;
			}
			if (fp)
				ic->arg[0] = (size_t)(&cpu->cd.ppc.fpr[rs]);
			else
				ic->arg[0] = (size_t)(&cpu->cd.ppc.gpr[rs]);
			if (!byterev && ic->f == NULL) {
				ic->f =
#ifdef MODE32
				    ppc32_loadstore_indexed
#else
				    ppc_loadstore_indexed
#endif
				    [size + 4*zero + 8*load + 16*update];
			}
			if (ra == 0 && update) {
				if (!cpu->translation_readahead)
					fatal("TODO: ra=0 && update?\n");
				goto bad;
			}
			break;

		case PPC_31_EXTSB:
		case PPC_31_EXTSH:
		case PPC_31_EXTSW:
		case PPC_31_SLW:
		case PPC_31_SLD:
		case PPC_31_SRAW:
		case PPC_31_SRW:
		case PPC_31_AND:
		case PPC_31_NAND:
		case PPC_31_ANDC:
		case PPC_31_NOR:
		case PPC_31_OR:
		case PPC_31_ORC:
		case PPC_31_XOR:
		case PPC_31_EQV:
			rs = (iword >> 21) & 31;
			ra = (iword >> 16) & 31;
			rb = (iword >> 11) & 31;
			rc = iword & 1;
			rc_f = NULL;
			switch (xo) {
			case PPC_31_EXTSB:ic->f = instr(extsb);
					  rc_f  = instr(extsb_dot); break;
			case PPC_31_EXTSH:ic->f = instr(extsh);
					  rc_f  = instr(extsh_dot); break;
			case PPC_31_EXTSW:ic->f = instr(extsw);
					  rc_f  = instr(extsw_dot); break;
			case PPC_31_SLW:  ic->f = instr(slw);
					  rc_f  = instr(slw_dot); break;
			case PPC_31_SLD:  ic->f = instr(sld);
					  rc_f  = instr(sld_dot); break;
			case PPC_31_SRAW: ic->f = instr(sraw);
					  rc_f  = instr(sraw_dot); break;
			case PPC_31_SRW:  ic->f = instr(srw);
					  rc_f  = instr(srw_dot); break;
			case PPC_31_AND:  ic->f = instr(and);
					  rc_f  = instr(and_dot); break;
			case PPC_31_NAND: ic->f = instr(nand);
					  rc_f  = instr(nand_dot); break;
			case PPC_31_ANDC: ic->f = instr(andc);
					  rc_f  = instr(andc_dot); break;
			case PPC_31_NOR:  ic->f = instr(nor);
					  rc_f  = instr(nor_dot); break;
			case PPC_31_OR:   ic->f = rs == rb? instr(mr)
						: instr(or);
					  rc_f  = instr(or_dot); break;
			case PPC_31_ORC:  ic->f = instr(orc);
					  rc_f  = instr(orc_dot); break;
			case PPC_31_XOR:  ic->f = instr(xor);
					  rc_f  = instr(xor_dot); break;
			case PPC_31_EQV:  ic->f = instr(eqv);
					  rc_f  = instr(eqv_dot); break;
			}
			ic->arg[0] = (size_t)(&cpu->cd.ppc.gpr[rs]);
			ic->arg[1] = (size_t)(&cpu->cd.ppc.gpr[rb]);
			ic->arg[2] = (size_t)(&cpu->cd.ppc.gpr[ra]);
			if (rc)
				ic->f = rc_f;
			break;

		case PPC_31_MULLW:
		case PPC_31_MULHW:
		case PPC_31_MULHWU:
		case PPC_31_DIVW:
		case PPC_31_DIVWU:
		case PPC_31_ADD:
		case PPC_31_ADDC:
		case PPC_31_ADDE:
		case PPC_31_ADDME:
		case PPC_31_ADDZE:
		case PPC_31_SUBF:
		case PPC_31_SUBFC:
		case PPC_31_SUBFE:
		case PPC_31_SUBFME:
		case PPC_31_SUBFZE:
			rt = (iword >> 21) & 31;
			ra = (iword >> 16) & 31;
			rb = (iword >> 11) & 31;
			oe_bit = (iword >> 10) & 1;
			rc = iword & 1;
			if (oe_bit) {
				if (!cpu->translation_readahead)
					fatal("oe_bit not yet implemented\n");
				goto bad;
			}
			switch (xo) {
			case PPC_31_MULLW:  ic->f = instr(mullw); break;
			case PPC_31_MULHW:  ic->f = instr(mulhw); break;
			case PPC_31_MULHWU: ic->f = instr(mulhwu); break;
			case PPC_31_DIVW:   ic->f = instr(divw); n64=1; break;
			case PPC_31_DIVWU:  ic->f = instr(divwu); n64=1; break;
			case PPC_31_ADD:    ic->f = instr(add); break;
			case PPC_31_ADDC:   ic->f = instr(addc); n64=1; break;
			case PPC_31_ADDE:   ic->f = instr(adde); n64=1; break;
			case PPC_31_ADDME:  ic->f = instr(addme); n64=1; break;
			case PPC_31_ADDZE:  ic->f = instr(addze); n64=1; break;
			case PPC_31_SUBF:   ic->f = instr(subf); break;
			case PPC_31_SUBFC:  ic->f = instr(subfc); break;
			case PPC_31_SUBFE:  ic->f = instr(subfe); n64=1; break;
			case PPC_31_SUBFME: ic->f = instr(subfme); n64=1; break;
			case PPC_31_SUBFZE: ic->f = instr(subfze); n64=1;break;
			}
			if (rc) {
				switch (xo) {
				case PPC_31_ADD:
					ic->f = instr(add_dot); break;
				case PPC_31_ADDE:
					ic->f = instr(adde_dot); break;
				case PPC_31_ADDME:
					ic->f = instr(addme_dot); break;
				case PPC_31_ADDZE:
					ic->f = instr(addze_dot); break;
				case PPC_31_DIVW:
					ic->f = instr(divw_dot); break;
				case PPC_31_DIVWU:
					ic->f = instr(divwu_dot); break;
				case PPC_31_MULLW:
					ic->f = instr(mullw_dot); break;
				case PPC_31_MULHW:
					ic->f = instr(mulhw_dot); break;
				case PPC_31_MULHWU:
					ic->f = instr(mulhwu_dot); break;
				case PPC_31_SUBF:
					ic->f = instr(subf_dot); break;
				case PPC_31_SUBFC:
					ic->f = instr(subfc_dot); break;
				case PPC_31_SUBFE:
					ic->f = instr(subfe_dot); break;
				case PPC_31_SUBFME:
					ic->f = instr(subfme_dot); break;
				case PPC_31_SUBFZE:
					ic->f = instr(subfze_dot); break;
				default:if (!cpu->translation_readahead)
						fatal("RC bit not yet "
						    "implemented\n");
					goto bad;
				}
			}
			ic->arg[0] = (size_t)(&cpu->cd.ppc.gpr[ra]);
			ic->arg[1] = (size_t)(&cpu->cd.ppc.gpr[rb]);
			ic->arg[2] = (size_t)(&cpu->cd.ppc.gpr[rt]);
			if (cpu->cd.ppc.bits == 64 && n64) {
				if (!cpu->translation_readahead)
					fatal("Not yet for 64-bit mode\n");
				goto bad;
			}
			break;

		case PPC_31_LVX:
		case PPC_31_LVXL:
		case PPC_31_STVX:
		case PPC_31_STVXL:
			load = 0;
			switch (xo) {
			case PPC_31_LVX:
			case PPC_31_LVXL:
				load = 1; break;
			}
			rs = (iword >> 21) & 31;
			ra = (iword >> 16) & 31;
			rb = (iword >> 11) & 31;
			ic->arg[0] = rs;
			if (ra == 0)
				ic->arg[1] = (size_t)(&cpu->cd.ppc.zero);
			else
				ic->arg[1] = (size_t)(&cpu->cd.ppc.gpr[ra]);
			ic->arg[2] = (size_t)(&cpu->cd.ppc.gpr[rb]);
			ic->f = load? instr(lvx) : instr(stvx);
			break;

		default:goto bad;
		}
		break;

	case PPC_HI6_59:
		xo = (iword >>  1) & 1023;
		rt = (iword >> 21) & 31;
		ra = (iword >> 16) & 31;
		rb = (iword >> 11) & 31;
		rs = (iword >>  6) & 31;	/*  actually frc  */
		rc = iword & 1;

		if (rc) {
			if (!cpu->translation_readahead)
				fatal("Floating point (59) "
				    "with rc bit! TODO\n");
			goto bad;
		}

		/*  NOTE: Some floating-point instructions are selected
		    using only the lowest 5 bits, not all 10!  */
		switch (xo & 31) {
		case PPC_59_FDIVS:
		case PPC_59_FSUBS:
		case PPC_59_FADDS:
			switch (xo & 31) {
			case PPC_59_FDIVS: ic->f = instr(fdivs); break;
			case PPC_59_FSUBS: ic->f = instr(fsubs); break;
			case PPC_59_FADDS: ic->f = instr(fadds); break;
			}
			ic->arg[0] = (size_t)(&cpu->cd.ppc.fpr[ra]);
			ic->arg[1] = (size_t)(&cpu->cd.ppc.fpr[rb]);
			ic->arg[2] = (size_t)(&cpu->cd.ppc.fpr[rt]);
			break;
		case PPC_59_FMULS:
			ic->f = instr(fmuls);
			ic->arg[0] = (size_t)(&cpu->cd.ppc.fpr[rt]);
			ic->arg[1] = (size_t)(&cpu->cd.ppc.fpr[ra]);
			ic->arg[2] = (size_t)(&cpu->cd.ppc.fpr[rs]); /* frc */
			break;
		default:/*  Use all 10 bits of xo:  */
			switch (xo) {
			default:goto bad;
			}
		}
		break;

	case PPC_HI6_63:
		xo = (iword >>  1) & 1023;
		rt = (iword >> 21) & 31;
		ra = (iword >> 16) & 31;
		rb = (iword >> 11) & 31;
		rs = (iword >>  6) & 31;	/*  actually frc  */
		rc = iword & 1;

		if (rc) {
			if (!cpu->translation_readahead)
				fatal("Floating point (63) "
				    "with rc bit! TODO\n");
			goto bad;
		}

		/*  NOTE: Some floating-point instructions are selected
		    using only the lowest 5 bits, not all 10!  */
		switch (xo & 31) {
		case PPC_63_FDIV:
		case PPC_63_FSUB:
		case PPC_63_FADD:
			switch (xo & 31) {
			case PPC_63_FDIV: ic->f = instr(fdiv); break;
			case PPC_63_FSUB: ic->f = instr(fsub); break;
			case PPC_63_FADD: ic->f = instr(fadd); break;
			}
			ic->arg[0] = (size_t)(&cpu->cd.ppc.fpr[ra]);
			ic->arg[1] = (size_t)(&cpu->cd.ppc.fpr[rb]);
			ic->arg[2] = (size_t)(&cpu->cd.ppc.fpr[rt]);
			break;
		case PPC_63_FMUL:
			ic->f = instr(fmul);
			ic->arg[0] = (size_t)(&cpu->cd.ppc.fpr[rt]);
			ic->arg[1] = (size_t)(&cpu->cd.ppc.fpr[ra]);
			ic->arg[2] = (size_t)(&cpu->cd.ppc.fpr[rs]); /* frc */
			break;
		case PPC_63_FMSUB:
		case PPC_63_FMADD:
			switch (xo & 31) {
			case PPC_63_FMSUB: ic->f = instr(fmsub); break;
			case PPC_63_FMADD: ic->f = instr(fmadd); break;
			}
			ic->arg[0] = (size_t)(&cpu->cd.ppc.fpr[rt]);
			ic->arg[1] = (size_t)(&cpu->cd.ppc.fpr[ra]);
			ic->arg[2] = iword;
			break;
		default:/*  Use all 10 bits of xo:  */
			switch (xo) {
			case PPC_63_FCMPU:
				ic->f = instr(fcmpu);
				ic->arg[0] = 28 - 4*(rt >> 2);
				ic->arg[1] = (size_t)(&cpu->cd.ppc.fpr[ra]);
				ic->arg[2] = (size_t)(&cpu->cd.ppc.fpr[rb]);
				break;
			case PPC_63_FRSP:
			case PPC_63_FCTIWZ:
			case PPC_63_FNEG:
			case PPC_63_FABS:
			case PPC_63_FMR:
				switch (xo) {
				case PPC_63_FRSP:   ic->f = instr(frsp); break;
				case PPC_63_FCTIWZ: ic->f = instr(fctiwz);break;
				case PPC_63_FNEG:   ic->f = instr(fneg); break;
				case PPC_63_FABS:   ic->f = instr(fabs); break;
				case PPC_63_FMR:    ic->f = instr(fmr); break;
				}
				ic->arg[0] = (size_t)(&cpu->cd.ppc.fpr[rb]);
				ic->arg[1] = (size_t)(&cpu->cd.ppc.fpr[rt]);
				break;
			case PPC_63_MFFS:
				ic->f = instr(mffs);
				ic->arg[0] = (size_t)(&cpu->cd.ppc.fpr[rt]);
				break;
			case PPC_63_MTFSF:
				ic->f = instr(mtfsf);
				ic->arg[0] = (size_t)(&cpu->cd.ppc.fpr[rb]);
				ic->arg[1] = 0;
				for (bi=7; bi>=0; bi--) {
					ic->arg[1] <<= 8;
					if (iword & (1 << (17+bi)))
						ic->arg[1] |= 0xf;
				}
				break;
			default:goto bad;
			}
		}
		break;

	default:goto bad;
	}


#define	DYNTRANS_TO_BE_TRANSLATED_TAIL
#include "cpu_dyntrans.cc"
#undef	DYNTRANS_TO_BE_TRANSLATED_TAIL
}

