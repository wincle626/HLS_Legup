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
 *  MIPS instructions.
 *
 *  Individual functions should keep track of cpu->n_translated_instrs.
 *  (If no instruction was executed, then it should be decreased. If, say, 4
 *  instructions were combined into one function and executed, then it should
 *  be increased by 3.)
 */


/*
 *  COPROC_AVAILABILITY_CHECK(n) checks for the coprocessor available bit for
 *  coprocessor number n, and causes a CoProcessor Unusable exception if it
 *  is not set.  (Note: For coprocessor 0 checks, use cop0_availability_check!)
 */
#ifndef	COPROC_AVAILABILITY_CHECK
#define	COPROC_AVAILABILITY_CHECK(x)		{		\
		const int cpnr = (x);					\
		int low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page) \
		    / sizeof(struct mips_instr_call);			\
		cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)		\
		    << MIPS_INSTR_ALIGNMENT_SHIFT);			\
		cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);	\
		if (!(cpu->cd.mips.coproc[0]->reg[COP0_STATUS] &	\
		    ((1 << cpnr) << STATUS_CU_SHIFT)) ) {		\
			mips_cpu_exception(cpu, EXCEPTION_CPU,		\
			    0, 0, cpnr, 0, 0, 0);			\
			return;						\
		}							\
	}
#endif


#ifndef	COP0_AVAILABILITY_CHECK_INCLUDED
#define	COP0_AVAILABILITY_CHECK_INCLUDED
/*
 *  cop0_availability_check() causes a CoProcessor Unusable exception if
 *  we are currently running in usermode, and the coprocessor available bit
 *  for coprocessor 0 is not set.
 *
 *  Returns 1 if ok (i.e. if the coprocessor was usable), 0 on exceptions.
 */
int cop0_availability_check(struct cpu *cpu, struct mips_instr_call *ic)
{
	int in_usermode = 0;
	struct mips_coproc *cp0 = cpu->cd.mips.coproc[0];

	switch (cpu->cd.mips.cpu_type.exc_model) {
	case EXC3K:
		/*
		 *  NOTE: If the KU bit is checked, Linux crashes.
		 *  It is the PC that counts.
		 *
		 *  TODO: Check whether this is true or not for R4000 as well.
		 */
		/*  TODO: if (cp0->reg[COP0_STATUS] & MIPS1_SR_KU_CUR)  */
		if (cpu->pc <= 0x7fffffff)
			in_usermode = 1;
		break;
	default:
		/*  R4000 etc:  (TODO: How about supervisor mode?)  */
		if (((cp0->reg[COP0_STATUS] &
		    STATUS_KSU_MASK) >> STATUS_KSU_SHIFT) != KSU_KERNEL)
			in_usermode = 1;
		if (cp0->reg[COP0_STATUS] & (STATUS_ERL | STATUS_EXL))
			in_usermode = 0;
		break;
	}

	if (in_usermode) {
		int low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
		    / sizeof(struct mips_instr_call);
		cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)
		    << MIPS_INSTR_ALIGNMENT_SHIFT);
		cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);
		if (!(cpu->cd.mips.coproc[0]->reg[COP0_STATUS] &
		    (1 << STATUS_CU_SHIFT)) ) {
			mips_cpu_exception(cpu, EXCEPTION_CPU,
			    0, 0, /* cpnr */ 0, 0, 0, 0);
			return 0;
		}
	}

	return 1;
}
#endif


/*
 *  invalid:  For catching bugs.
 */
X(invalid)
{
	fatal("FATAL ERROR: An internal error occured in the MIPS"
	    " dyntrans code. Please contact the author with detailed"
	    " repro steps on how to trigger this bug.\n");
	exit(1);
}


/*
 *  reserved:  Attempt to execute a reserved instruction (e.g. a 64-bit
 *             instruction on an emulated 32-bit processor).
 */
X(reserved)
{
	/*  Synchronize the PC and cause an exception:  */
	int low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
	    / sizeof(struct mips_instr_call);
	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)
	    << MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);
	mips_cpu_exception(cpu, EXCEPTION_RI, 0, 0, 0, 0, 0, 0);
}


/*
 *  cpu:  Cause a CoProcessor Unusable exception.
 *
 *  arg[0] = the number of the coprocessor
 */
X(cpu)
{
	/*  Synchronize the PC and cause an exception:  */
	int low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
	    / sizeof(struct mips_instr_call);
	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)
	    << MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);
	mips_cpu_exception(cpu, EXCEPTION_CPU, 0, 0, ic->arg[0], 0, 0, 0);
}


/*
 *  nop:  Do nothing.
 */
X(nop)
{
}


/*
 *  beq:  Branch if equal
 *  bne:  Branch if not equal
 *  b:  Branch (comparing a register to itself, always true)
 *
 *  arg[0] = pointer to rs
 *  arg[1] = pointer to rt
 *  arg[2] = (int32_t) relative offset from the next instruction
 */
X(beq)
{
	MODE_int_t old_pc = cpu->pc;
	MODE_uint_t rs = reg(ic->arg[0]), rt = reg(ic->arg[1]);
	int x = rs == rt;
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (x) {
			old_pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1) <<
			    MIPS_INSTR_ALIGNMENT_SHIFT);
			cpu->pc = old_pc + (int32_t)ic->arg[2];
			quick_pc_to_pointers(cpu);
		} else
			cpu->cd.mips.next_ic ++;
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(beq_samepage)
{
	MODE_uint_t rs = reg(ic->arg[0]), rt = reg(ic->arg[1]);
	int x = rs == rt;
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		if (x)
			cpu->cd.mips.next_ic = (struct mips_instr_call *)
			    ic->arg[2];
		else
			cpu->cd.mips.next_ic ++;
	}
	cpu->delay_slot = NOT_DELAYED;
}
X(beq_samepage_addiu)
{
	MODE_uint_t rs = reg(ic->arg[0]), rt = reg(ic->arg[1]);
	cpu->n_translated_instrs ++;
	reg(ic[1].arg[1]) = (int32_t)
	    ((int32_t)reg(ic[1].arg[0]) + (int32_t)ic[1].arg[2]);
	if (rs == rt)
		cpu->cd.mips.next_ic = (struct mips_instr_call *) ic->arg[2];
	else
		cpu->cd.mips.next_ic ++;
}
X(beq_samepage_nop)
{
	MODE_uint_t rs = reg(ic->arg[0]), rt = reg(ic->arg[1]);
	cpu->n_translated_instrs ++;
	if (rs == rt)
		cpu->cd.mips.next_ic = (struct mips_instr_call *) ic->arg[2];
	else
		cpu->cd.mips.next_ic ++;
}
X(bne)
{
	MODE_int_t old_pc = cpu->pc;
	MODE_uint_t rs = reg(ic->arg[0]), rt = reg(ic->arg[1]);
	int x = rs != rt;
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (x) {
			old_pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1) <<
			    MIPS_INSTR_ALIGNMENT_SHIFT);
			cpu->pc = old_pc + (int32_t)ic->arg[2];
			quick_pc_to_pointers(cpu);
		} else
			cpu->cd.mips.next_ic ++;
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(bne_samepage)
{
	MODE_uint_t rs = reg(ic->arg[0]), rt = reg(ic->arg[1]);
	int x = rs != rt;
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		if (x)
			cpu->cd.mips.next_ic = (struct mips_instr_call *)
			    ic->arg[2];
		else
			cpu->cd.mips.next_ic ++;
	}
	cpu->delay_slot = NOT_DELAYED;
}
X(bne_samepage_addiu)
{
	MODE_uint_t rs = reg(ic->arg[0]), rt = reg(ic->arg[1]);
	cpu->n_translated_instrs ++;
	reg(ic[1].arg[1]) = (int32_t)
	    ((int32_t)reg(ic[1].arg[0]) + (int32_t)ic[1].arg[2]);
	if (rs != rt)
		cpu->cd.mips.next_ic = (struct mips_instr_call *) ic->arg[2];
	else
		cpu->cd.mips.next_ic ++;
}
X(bne_samepage_nop)
{
	MODE_uint_t rs = reg(ic->arg[0]), rt = reg(ic->arg[1]);
	cpu->n_translated_instrs ++;
	if (rs != rt)
		cpu->cd.mips.next_ic = (struct mips_instr_call *) ic->arg[2];
	else
		cpu->cd.mips.next_ic ++;
}
X(b)
{
	MODE_int_t old_pc = cpu->pc;
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		old_pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1) <<
		    MIPS_INSTR_ALIGNMENT_SHIFT);
		cpu->pc = old_pc + (int32_t)ic->arg[2];
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(b_samepage)
{
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT))
		cpu->cd.mips.next_ic = (struct mips_instr_call *) ic->arg[2];
	cpu->delay_slot = NOT_DELAYED;
}


/*
 *  beql:  Branch if equal likely
 *  bnel:  Branch if not equal likely
 *
 *  arg[0] = pointer to rs
 *  arg[1] = pointer to rt
 *  arg[2] = (int32_t) relative offset from the next instruction
 */
X(beql)
{
	MODE_int_t old_pc = cpu->pc;
	MODE_uint_t rs = reg(ic->arg[0]), rt = reg(ic->arg[1]);
	int x = rs == rt;
	cpu->delay_slot = TO_BE_DELAYED;
	if (x)
		ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (x) {
			old_pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1) <<
			    MIPS_INSTR_ALIGNMENT_SHIFT);
			cpu->pc = old_pc + (int32_t)ic->arg[2];
			quick_pc_to_pointers(cpu);
		} else
			cpu->cd.mips.next_ic ++;
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(beql_samepage)
{
	MODE_uint_t rs = reg(ic->arg[0]), rt = reg(ic->arg[1]);
	int x = rs == rt;
	cpu->delay_slot = TO_BE_DELAYED;
	if (x)
		ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		if (x)
			cpu->cd.mips.next_ic = (struct mips_instr_call *)
			    ic->arg[2];
		else
			cpu->cd.mips.next_ic ++;
	}
	cpu->delay_slot = NOT_DELAYED;
}
X(bnel)
{
	MODE_int_t old_pc = cpu->pc;
	MODE_uint_t rs = reg(ic->arg[0]), rt = reg(ic->arg[1]);
	int x = rs != rt;
	cpu->delay_slot = TO_BE_DELAYED;
	if (x)
		ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (x) {
			old_pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1) <<
			    MIPS_INSTR_ALIGNMENT_SHIFT);
			cpu->pc = old_pc + (int32_t)ic->arg[2];
			quick_pc_to_pointers(cpu);
		} else
			cpu->cd.mips.next_ic ++;
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(bnel_samepage)
{
	MODE_uint_t rs = reg(ic->arg[0]), rt = reg(ic->arg[1]);
	int x = rs != rt;
	cpu->delay_slot = TO_BE_DELAYED;
	if (x)
		ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		if (x)
			cpu->cd.mips.next_ic = (struct mips_instr_call *)
			    ic->arg[2];
		else
			cpu->cd.mips.next_ic ++;
	}
	cpu->delay_slot = NOT_DELAYED;
}


/*
 *  blez:   Branch if less than or equal
 *  blezl:  Branch if less than or equal likely
 *
 *  arg[0] = pointer to rs
 *  arg[2] = (int32_t) relative offset from the next instruction
 */
X(blez)
{
	MODE_int_t old_pc = cpu->pc;
	MODE_int_t rs = reg(ic->arg[0]);
	int x = (rs <= 0);
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (x) {
			old_pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1) <<
			    MIPS_INSTR_ALIGNMENT_SHIFT);
			cpu->pc = old_pc + (int32_t)ic->arg[2];
			quick_pc_to_pointers(cpu);
		} else
			cpu->cd.mips.next_ic ++;
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(blez_samepage)
{
	MODE_int_t rs = reg(ic->arg[0]);
	int x = (rs <= 0);
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		if (x)
			cpu->cd.mips.next_ic = (struct mips_instr_call *)
			    ic->arg[2];
		else
			cpu->cd.mips.next_ic ++;
	}
	cpu->delay_slot = NOT_DELAYED;
}
X(blezl)
{
	MODE_int_t old_pc = cpu->pc;
	MODE_int_t rs = reg(ic->arg[0]);
	int x = (rs <= 0);
	cpu->delay_slot = TO_BE_DELAYED;
	if (x)
		ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (x) {
			old_pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1) <<
			    MIPS_INSTR_ALIGNMENT_SHIFT);
			cpu->pc = old_pc + (int32_t)ic->arg[2];
			quick_pc_to_pointers(cpu);
		} else
			cpu->cd.mips.next_ic ++;
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(blezl_samepage)
{
	MODE_int_t rs = reg(ic->arg[0]);
	int x = (rs <= 0);
	cpu->delay_slot = TO_BE_DELAYED;
	if (x)
		ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		if (x)
			cpu->cd.mips.next_ic = (struct mips_instr_call *)
			    ic->arg[2];
		else
			cpu->cd.mips.next_ic ++;
	}
	cpu->delay_slot = NOT_DELAYED;
}


/*
 *  bltz:   Branch if less than
 *  bltzl:  Branch if less than likely
 *
 *  arg[0] = pointer to rs
 *  arg[2] = (int32_t) relative offset from the next instruction
 */
X(bltz)
{
	MODE_int_t old_pc = cpu->pc;
	MODE_int_t rs = reg(ic->arg[0]);
	int x = (rs < 0);
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (x) {
			old_pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1) <<
			    MIPS_INSTR_ALIGNMENT_SHIFT);
			cpu->pc = old_pc + (int32_t)ic->arg[2];
			quick_pc_to_pointers(cpu);
		} else
			cpu->cd.mips.next_ic ++;
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(bltz_samepage)
{
	MODE_int_t rs = reg(ic->arg[0]);
	int x = (rs < 0);
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		if (x)
			cpu->cd.mips.next_ic = (struct mips_instr_call *)
			    ic->arg[2];
		else
			cpu->cd.mips.next_ic ++;
	}
	cpu->delay_slot = NOT_DELAYED;
}
X(bltzl)
{
	MODE_int_t old_pc = cpu->pc;
	MODE_int_t rs = reg(ic->arg[0]);
	int x = (rs < 0);
	cpu->delay_slot = TO_BE_DELAYED;
	if (x)
		ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (x) {
			old_pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1) <<
			    MIPS_INSTR_ALIGNMENT_SHIFT);
			cpu->pc = old_pc + (int32_t)ic->arg[2];
			quick_pc_to_pointers(cpu);
		} else
			cpu->cd.mips.next_ic ++;
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(bltzl_samepage)
{
	MODE_int_t rs = reg(ic->arg[0]);
	int x = (rs < 0);
	cpu->delay_slot = TO_BE_DELAYED;
	if (x)
		ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		if (x)
			cpu->cd.mips.next_ic = (struct mips_instr_call *)
			    ic->arg[2];
		else
			cpu->cd.mips.next_ic ++;
	}
	cpu->delay_slot = NOT_DELAYED;
}


/*
 *  bgez:   Branch if greater than or equal
 *  bgezl:  Branch if greater than or equal likely
 *
 *  arg[0] = pointer to rs
 *  arg[2] = (int32_t) relative offset from the next instruction
 */
X(bgez)
{
	MODE_int_t old_pc = cpu->pc;
	MODE_int_t rs = reg(ic->arg[0]);
	int x = (rs >= 0);
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (x) {
			old_pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1) <<
			    MIPS_INSTR_ALIGNMENT_SHIFT);
			cpu->pc = old_pc + (int32_t)ic->arg[2];
			quick_pc_to_pointers(cpu);
		} else
			cpu->cd.mips.next_ic ++;
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(bgez_samepage)
{
	MODE_int_t rs = reg(ic->arg[0]);
	int x = (rs >= 0);
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		if (x)
			cpu->cd.mips.next_ic = (struct mips_instr_call *)
			    ic->arg[2];
		else
			cpu->cd.mips.next_ic ++;
	}
	cpu->delay_slot = NOT_DELAYED;
}
X(bgezl)
{
	MODE_int_t old_pc = cpu->pc;
	MODE_int_t rs = reg(ic->arg[0]);
	int x = (rs >= 0);
	cpu->delay_slot = TO_BE_DELAYED;
	if (x)
		ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (x) {
			old_pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1) <<
			    MIPS_INSTR_ALIGNMENT_SHIFT);
			cpu->pc = old_pc + (int32_t)ic->arg[2];
			quick_pc_to_pointers(cpu);
		} else
			cpu->cd.mips.next_ic ++;
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(bgezl_samepage)
{
	MODE_int_t rs = reg(ic->arg[0]);
	int x = (rs >= 0);
	cpu->delay_slot = TO_BE_DELAYED;
	if (x)
		ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		if (x)
			cpu->cd.mips.next_ic = (struct mips_instr_call *)
			    ic->arg[2];
		else
			cpu->cd.mips.next_ic ++;
	}
	cpu->delay_slot = NOT_DELAYED;
}


/*
 *  bgezal:   Branch if greater than or equal (and link)
 *  bgezall:  Branch if greater than or equal (and link) likely
 *
 *  arg[0] = pointer to rs
 *  arg[2] = (int32_t) relative offset from the next instruction
 */
X(bgezal)
{
	MODE_int_t old_pc = cpu->pc;
	MODE_int_t rs = reg(ic->arg[0]);
	int x = (rs >= 0), low_pc;

	cpu->delay_slot = TO_BE_DELAYED;
	low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
	    / sizeof(struct mips_instr_call);
	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)
	    << MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->cd.mips.gpr[MIPS_GPR_RA] = cpu->pc + 8;

	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (x) {
			old_pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1) <<
			    MIPS_INSTR_ALIGNMENT_SHIFT);
			cpu->pc = old_pc + (int32_t)ic->arg[2];
			quick_pc_to_pointers(cpu);
		} else
			cpu->cd.mips.next_ic ++;
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(bgezal_samepage)
{
	MODE_int_t rs = reg(ic->arg[0]);
	int x = (rs >= 0), low_pc;

	cpu->delay_slot = TO_BE_DELAYED;
	low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
	    / sizeof(struct mips_instr_call);
	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)
	    << MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->cd.mips.gpr[MIPS_GPR_RA] = cpu->pc + 8;

	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		if (x)
			cpu->cd.mips.next_ic = (struct mips_instr_call *)
			    ic->arg[2];
		else
			cpu->cd.mips.next_ic ++;
	}
	cpu->delay_slot = NOT_DELAYED;
}
X(bgezall)
{
	MODE_int_t old_pc = cpu->pc;
	MODE_int_t rs = reg(ic->arg[0]);
	int x = (rs >= 0), low_pc;

	cpu->delay_slot = TO_BE_DELAYED;
	low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
	    / sizeof(struct mips_instr_call);
	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)
	    << MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->cd.mips.gpr[MIPS_GPR_RA] = cpu->pc + 8;

	if (x)
		ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (x) {
			old_pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1) <<
			    MIPS_INSTR_ALIGNMENT_SHIFT);
			cpu->pc = old_pc + (int32_t)ic->arg[2];
			quick_pc_to_pointers(cpu);
		} else
			cpu->cd.mips.next_ic ++;
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(bgezall_samepage)
{
	MODE_int_t rs = reg(ic->arg[0]);
	int x = (rs >= 0), low_pc;

	cpu->delay_slot = TO_BE_DELAYED;
	low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
	    / sizeof(struct mips_instr_call);
	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)
	    << MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->cd.mips.gpr[MIPS_GPR_RA] = cpu->pc + 8;

	if (x)
		ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		if (x)
			cpu->cd.mips.next_ic = (struct mips_instr_call *)
			    ic->arg[2];
		else
			cpu->cd.mips.next_ic ++;
	}
	cpu->delay_slot = NOT_DELAYED;
}


/*
 *  bltzal:   Branch if less than zero (and link)
 *  bltzall:  Branch if less than zero (and link) likely
 *
 *  arg[0] = pointer to rs
 *  arg[2] = (int32_t) relative offset from the next instruction
 */
X(bltzal)
{
	MODE_int_t old_pc = cpu->pc;
	MODE_int_t rs = reg(ic->arg[0]);
	int x = (rs < 0), low_pc;

	cpu->delay_slot = TO_BE_DELAYED;
	low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
	    / sizeof(struct mips_instr_call);
	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)
	    << MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->cd.mips.gpr[MIPS_GPR_RA] = cpu->pc + 8;

	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (x) {
			old_pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1) <<
			    MIPS_INSTR_ALIGNMENT_SHIFT);
			cpu->pc = old_pc + (int32_t)ic->arg[2];
			quick_pc_to_pointers(cpu);
		} else
			cpu->cd.mips.next_ic ++;
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(bltzal_samepage)
{
	MODE_int_t rs = reg(ic->arg[0]);
	int x = (rs < 0), low_pc;

	cpu->delay_slot = TO_BE_DELAYED;
	low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
	    / sizeof(struct mips_instr_call);
	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)
	    << MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->cd.mips.gpr[MIPS_GPR_RA] = cpu->pc + 8;

	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		if (x)
			cpu->cd.mips.next_ic = (struct mips_instr_call *)
			    ic->arg[2];
		else
			cpu->cd.mips.next_ic ++;
	}
	cpu->delay_slot = NOT_DELAYED;
}
X(bltzall)
{
	MODE_int_t old_pc = cpu->pc;
	MODE_int_t rs = reg(ic->arg[0]);
	int x = (rs < 0), low_pc;

	cpu->delay_slot = TO_BE_DELAYED;
	low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
	    / sizeof(struct mips_instr_call);
	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)
	    << MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->cd.mips.gpr[MIPS_GPR_RA] = cpu->pc + 8;

	if (x)
		ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (x) {
			old_pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1) <<
			    MIPS_INSTR_ALIGNMENT_SHIFT);
			cpu->pc = old_pc + (int32_t)ic->arg[2];
			quick_pc_to_pointers(cpu);
		} else
			cpu->cd.mips.next_ic ++;
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(bltzall_samepage)
{
	MODE_int_t rs = reg(ic->arg[0]);
	int x = (rs < 0), low_pc;

	cpu->delay_slot = TO_BE_DELAYED;
	low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
	    / sizeof(struct mips_instr_call);
	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)
	    << MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->cd.mips.gpr[MIPS_GPR_RA] = cpu->pc + 8;

	if (x)
		ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		if (x)
			cpu->cd.mips.next_ic = (struct mips_instr_call *)
			    ic->arg[2];
		else
			cpu->cd.mips.next_ic ++;
	}
	cpu->delay_slot = NOT_DELAYED;
}


/*
 *  bgtz:   Branch if greater than zero
 *  bgtzl:  Branch if greater than zero likely
 *
 *  arg[0] = pointer to rs
 *  arg[2] = (int32_t) relative offset from the next instruction
 */
X(bgtz)
{
	MODE_int_t old_pc = cpu->pc;
	MODE_int_t rs = reg(ic->arg[0]);
	int x = (rs > 0);
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (x) {
			old_pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1) <<
			    MIPS_INSTR_ALIGNMENT_SHIFT);
			cpu->pc = old_pc + (int32_t)ic->arg[2];
			quick_pc_to_pointers(cpu);
		} else
			cpu->cd.mips.next_ic ++;
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(bgtz_samepage)
{
	MODE_int_t rs = reg(ic->arg[0]);
	int x = (rs > 0);
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		if (x)
			cpu->cd.mips.next_ic = (struct mips_instr_call *)
			    ic->arg[2];
		else
			cpu->cd.mips.next_ic ++;
	}
	cpu->delay_slot = NOT_DELAYED;
}
X(bgtzl)
{
	MODE_int_t old_pc = cpu->pc;
	MODE_int_t rs = reg(ic->arg[0]);
	int x = (rs > 0);
	cpu->delay_slot = TO_BE_DELAYED;
	if (x)
		ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (x) {
			old_pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1) <<
			    MIPS_INSTR_ALIGNMENT_SHIFT);
			cpu->pc = old_pc + (int32_t)ic->arg[2];
			quick_pc_to_pointers(cpu);
		} else
			cpu->cd.mips.next_ic ++;
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(bgtzl_samepage)
{
	MODE_int_t rs = reg(ic->arg[0]);
	int x = (rs > 0);
	cpu->delay_slot = TO_BE_DELAYED;
	if (x)
		ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		if (x)
			cpu->cd.mips.next_ic = (struct mips_instr_call *)
			    ic->arg[2];
		else
			cpu->cd.mips.next_ic ++;
	}
	cpu->delay_slot = NOT_DELAYED;
}


/*
 *  jr, jalr: Jump to a register [and link].
 *
 *  arg[0] = ptr to rs
 *  arg[1] = ptr to rd (for jalr)
 *  arg[2] = (int32_t) relative offset of the next instruction
 */
X(jr)
{
	MODE_int_t rs = reg(ic->arg[0]);
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		cpu->pc = rs;
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(jr_ra)
{
	MODE_int_t rs = cpu->cd.mips.gpr[MIPS_GPR_RA];
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		cpu->pc = rs;
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(jr_ra_addiu)
{
	/*  jr ra, followed by an addiu  */
	MODE_int_t rs = cpu->cd.mips.gpr[MIPS_GPR_RA];
	reg(ic[1].arg[1]) = (int32_t)
	    ((int32_t)reg(ic[1].arg[0]) + (int32_t)ic[1].arg[2]);
	cpu->pc = rs;
	quick_pc_to_pointers(cpu);
	cpu->n_translated_instrs ++;
}
X(jr_ra_trace)
{
	MODE_int_t rs = cpu->cd.mips.gpr[MIPS_GPR_RA];
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		cpu->pc = rs;
		cpu_functioncall_trace_return(cpu);
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(jalr)
{
	MODE_int_t rs = reg(ic->arg[0]), rd;
	cpu->delay_slot = TO_BE_DELAYED;
	rd = cpu->pc & ~((MIPS_IC_ENTRIES_PER_PAGE-1) <<
	    MIPS_INSTR_ALIGNMENT_SHIFT);
	rd += (int32_t)ic->arg[2];
	reg(ic->arg[1]) = rd;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		cpu->pc = rs;
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(jalr_trace)
{
	MODE_int_t rs = reg(ic->arg[0]), rd;
	cpu->delay_slot = TO_BE_DELAYED;
	rd = cpu->pc & ~((MIPS_IC_ENTRIES_PER_PAGE-1) <<
	    MIPS_INSTR_ALIGNMENT_SHIFT);
	rd += (int32_t)ic->arg[2];
	reg(ic->arg[1]) = rd;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		cpu->pc = rs;
		cpu_functioncall_trace(cpu, cpu->pc);
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}


/*
 *  j, jal:  Jump [and link].
 *
 *  arg[0] = lowest 28 bits of new pc.
 *  arg[1] = offset from start of page to the jal instruction + 8
 */
X(j)
{
	MODE_int_t old_pc = cpu->pc;
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		old_pc &= ~0x03ffffff;
		cpu->pc = old_pc | (uint32_t)ic->arg[0];
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(jal)
{
	MODE_int_t old_pc = cpu->pc;
	cpu->delay_slot = TO_BE_DELAYED;
	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)<<MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->cd.mips.gpr[31] = (MODE_int_t)cpu->pc + (int32_t)ic->arg[1];
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		old_pc &= ~0x03ffffff;
		cpu->pc = old_pc | (int32_t)ic->arg[0];
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(jal_trace)
{
	MODE_int_t old_pc = cpu->pc;
	cpu->delay_slot = TO_BE_DELAYED;
	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)<<MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->cd.mips.gpr[31] = (MODE_int_t)cpu->pc + (int32_t)ic->arg[1];
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		old_pc &= ~0x03ffffff;
		cpu->pc = old_pc | (int32_t)ic->arg[0];
		cpu_functioncall_trace(cpu, cpu->pc);
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}


/*
 *  cache:  Cache operation.
 */
X(cache)
{
	/*  TODO: Implement cache operations.  */

	/*  Make sure the rmw bit is cleared:  */
	cpu->cd.mips.rmw = 0;
}


/*
 *  ins: Insert bitfield.
 *
 *  arg[0] = pointer to rt
 *  arg[1] = pointer to rs
 *  arg[2] = (msb << 5) + lsb
 */
X(ins)
{
	int msb = ic->arg[2] >> 5, pos = ic->arg[2] & 0x1f;
	int size = msb + 1 - pos;
	uint32_t rt = reg(ic->arg[0]);
	uint32_t rs = reg(ic->arg[1]);
	uint32_t mask = (-1) << pos;

	mask <<= (32 - pos - size);
	mask >>= (32 - pos - size);

	reg(ic->arg[0]) = (int32_t) ((rt & ~mask) | ((rs << pos) & mask));
}


/*
 *  ext:  Extract bitfield.
 *
 *  arg[0] = pointer to rt
 *  arg[1] = pointer to rs
 *  arg[2] = (msbd << 5) + lsb
 */
X(ext)
{
	int msbd = ic->arg[2] >> 5, lsb = ic->arg[2] & 0x1f;
	int size = msbd + 1;
	uint32_t rs = reg(ic->arg[1]);
	uint32_t x = (rs << (32-lsb-size)) >> (32-lsb-size);
	reg(ic->arg[0]) = (int32_t) (x >> lsb);
}


/*
 *  dext:  Extract bitfield (64-bit).
 *
 *  arg[0] = pointer to rt
 *  arg[1] = pointer to rs
 *  arg[2] = (msbd << 6) + lsb
 */
X(dext)
{
	int msbd = ic->arg[2] >> 6, lsb = ic->arg[2] & 0x3f;
	int size = msbd + 1;
	uint64_t rs = reg(ic->arg[1]);
	uint64_t x = (rs << (uint64_t)(64-lsb-size)) >> (uint64_t)(64-lsb-size);
	reg(ic->arg[0]) = x >> lsb;
}


/*
 *  dsbh:  Doubleword swap bytes within half-word
 *  dshd:  Doubleword swap half-words within double-word
 *  wsbh:  Word swap bytes within half-word
 *  seb:   Sign-extend byte
 *  seh:   Sign-extend half-word
 *
 *  arg[0] = pointer to rt
 *  arg[1] = pointer to rd
 */
X(dsbh)
{
	uint64_t x = reg(ic->arg[0]);
	x = ((x & 0x00ff00ff00ff00ffULL) << 8)
	  | ((x & 0xff00ff00ff00ff00ULL) >> 8);
	reg(ic->arg[1]) = x;
}
X(dshd)
{
	uint64_t x = reg(ic->arg[0]);
	x = ((x & 0x000000000000ffffULL) << 48)
	  | ((x & 0x00000000ffff0000ULL) << 16)
	  | ((x & 0x0000ffff00000000ULL) >> 16)
	  | ((x & 0xffff000000000000ULL) >> 48);
	reg(ic->arg[1]) = x;
}
X(wsbh)
{
	uint32_t x = reg(ic->arg[0]);
	x = ((x & 0x00ff00ff) << 8) | ((x & 0xff00ff00) >> 8);
	reg(ic->arg[1]) = (int32_t) x;
}
X(seb) { reg(ic->arg[1]) = (int8_t)reg(ic->arg[0]); }
X(seh) { reg(ic->arg[1]) = (int16_t)reg(ic->arg[0]); }


/*
 *  2-register + immediate:
 *
 *  arg[0] = pointer to rs
 *  arg[1] = pointer to rt
 *  arg[2] = uint32_t immediate value
 */
X(andi) { reg(ic->arg[1]) = reg(ic->arg[0]) & (uint32_t)ic->arg[2]; }
X(ori)  { reg(ic->arg[1]) = reg(ic->arg[0]) | (uint32_t)ic->arg[2]; }
X(xori) { reg(ic->arg[1]) = reg(ic->arg[0]) ^ (uint32_t)ic->arg[2]; }


/*
 *  2-register:
 *
 *  arg[0] = ptr to rs
 *  arg[1] = ptr to rt
 */
X(div)
{
	int32_t a = reg(ic->arg[0]), b = reg(ic->arg[1]);
	int32_t res, rem;
	if (b == 0)
		res = 0, rem = a;
	else
		res = a / b, rem = a - b*res;
	cpu->cd.mips.lo = (int32_t)res;
	cpu->cd.mips.hi = (int32_t)rem;
}
X(divu)
{
	uint32_t a = reg(ic->arg[0]), b = reg(ic->arg[1]);
	uint32_t res, rem;
	if (b == 0)
		res = 0, rem = a;
	else
		res = a / b, rem = a - b*res;
	cpu->cd.mips.lo = (int32_t)res;
	cpu->cd.mips.hi = (int32_t)rem;
}
X(ddiv)
{
	int64_t a = reg(ic->arg[0]), b = reg(ic->arg[1]);
	int64_t res, rem;
	if (b == 0)
		res = 0;
	else
		res = a / b;
	rem = a - b*res;
	cpu->cd.mips.lo = res;
	cpu->cd.mips.hi = rem;
}
X(ddivu)
{
	uint64_t a = reg(ic->arg[0]), b = reg(ic->arg[1]);
	uint64_t res, rem;
	if (b == 0)
		res = 0;
	else
		res = a / b;
	rem = a - b*res;
	cpu->cd.mips.lo = res;
	cpu->cd.mips.hi = rem;
}
X(mult)
{
	int32_t a = reg(ic->arg[0]), b = reg(ic->arg[1]);
	int64_t res = (int64_t)a * (int64_t)b;
	cpu->cd.mips.lo = (int32_t)res;
	cpu->cd.mips.hi = (int32_t)(res >> 32);
}
X(mult_r5900)
{
	/*  C790/TX79/R5900 multiplication, stores result in
	    hi, lo, and a third register  */
	int32_t a = reg(ic->arg[0]), b = reg(ic->arg[1]);
	int64_t res = (int64_t)a * (int64_t)b;
	cpu->cd.mips.lo = (int32_t)res;
	cpu->cd.mips.hi = (int32_t)(res >> 32);
	reg(ic->arg[2]) = (int32_t)res;
}
X(multu)
{
	uint32_t a = reg(ic->arg[0]), b = reg(ic->arg[1]);
	uint64_t res = (uint64_t)a * (uint64_t)b;
	cpu->cd.mips.lo = (int32_t)res;
	cpu->cd.mips.hi = (int32_t)(res >> 32);
}
X(multu_r5900)
{
	/*  C790/TX79/R5900 multiplication, stores result in
	    hi, lo, and a third register  */
	uint32_t a = reg(ic->arg[0]), b = reg(ic->arg[1]);
	uint64_t res = (uint64_t)a * (uint64_t)b;
	cpu->cd.mips.lo = (int32_t)res;
	cpu->cd.mips.hi = (int32_t)(res >> 32);
	reg(ic->arg[2]) = (int32_t)res;
}
X(dmult)
{
	uint64_t a = reg(ic->arg[0]), b = reg(ic->arg[1]), c = 0;
	uint64_t hi = 0, lo = 0;
	int neg = 0;
	if (a >> 63)
		neg = !neg, a = -a;
	if (b >> 63)
		neg = !neg, b = -b;
	for (; a; a >>= 1) {
		if (a & 1) {
			uint64_t old_lo = lo;
			hi += c;
			lo += b;
			if (lo < old_lo)
				hi ++;
		}
		c = (c << 1) | (b >> 63); b <<= 1;
	}
	if (neg) {
		if (lo == 0)
			hi --;
		lo --;
		hi ^= (int64_t) -1;
		lo ^= (int64_t) -1;
	}
	cpu->cd.mips.lo = lo;
	cpu->cd.mips.hi = hi;
}
X(dmultu)
{
	uint64_t a = reg(ic->arg[0]), b = reg(ic->arg[1]), c = 0;
	uint64_t hi = 0, lo = 0;
	for (; a; a >>= 1) {
		if (a & 1) {
			uint64_t old_lo = lo;
			hi += c;
			lo += b;
			if (lo < old_lo)
				hi ++;
		}
		c = (c << 1) | (b >> 63); b <<= 1;
	}
	cpu->cd.mips.lo = lo;
	cpu->cd.mips.hi = hi;
}
X(tge)
{
	MODE_int_t a = reg(ic->arg[0]), b = reg(ic->arg[1]);
	if (a >= b) {
		/*  Synch. PC and cause an exception:  */
		int low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
		    / sizeof(struct mips_instr_call);
		cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)
		    << MIPS_INSTR_ALIGNMENT_SHIFT);
		cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);
		mips_cpu_exception(cpu, EXCEPTION_TR, 0, 0, 0, 0, 0, 0);
	}
}
X(tgeu)
{
	MODE_uint_t a = reg(ic->arg[0]), b = reg(ic->arg[1]);
	if (a >= b) {
		/*  Synch. PC and cause an exception:  */
		int low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
		    / sizeof(struct mips_instr_call);
		cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)
		    << MIPS_INSTR_ALIGNMENT_SHIFT);
		cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);
		mips_cpu_exception(cpu, EXCEPTION_TR, 0, 0, 0, 0, 0, 0);
	}
}
X(tlt)
{
	MODE_int_t a = reg(ic->arg[0]), b = reg(ic->arg[1]);
	if (a < b) {
		/*  Synch. PC and cause an exception:  */
		int low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
		    / sizeof(struct mips_instr_call);
		cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)
		    << MIPS_INSTR_ALIGNMENT_SHIFT);
		cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);
		mips_cpu_exception(cpu, EXCEPTION_TR, 0, 0, 0, 0, 0, 0);
	}
}
X(tltu)
{
	MODE_uint_t a = reg(ic->arg[0]), b = reg(ic->arg[1]);
	if (a < b) {
		/*  Synch. PC and cause an exception:  */
		int low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
		    / sizeof(struct mips_instr_call);
		cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)
		    << MIPS_INSTR_ALIGNMENT_SHIFT);
		cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);
		mips_cpu_exception(cpu, EXCEPTION_TR, 0, 0, 0, 0, 0, 0);
	}
}
X(teq)
{
	MODE_uint_t a = reg(ic->arg[0]), b = reg(ic->arg[1]);
	if (a == b) {
		/*  Synch. PC and cause an exception:  */
		int low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
		    / sizeof(struct mips_instr_call);
		cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)
		    << MIPS_INSTR_ALIGNMENT_SHIFT);
		cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);
		mips_cpu_exception(cpu, EXCEPTION_TR, 0, 0, 0, 0, 0, 0);
	}
}
X(tne)
{
	MODE_uint_t a = reg(ic->arg[0]), b = reg(ic->arg[1]);
	if (a != b) {
		/*  Synch. PC and cause an exception:  */
		int low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
		    / sizeof(struct mips_instr_call);
		cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)
		    << MIPS_INSTR_ALIGNMENT_SHIFT);
		cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);
		mips_cpu_exception(cpu, EXCEPTION_TR, 0, 0, 0, 0, 0, 0);
	}
}


/*
 *  3-register arithmetic instructions:
 *
 *  arg[0] = ptr to rs
 *  arg[1] = ptr to rt
 *  arg[2] = ptr to rd
 */
X(addu) { reg(ic->arg[2]) = (int32_t)(reg(ic->arg[0]) + reg(ic->arg[1])); }
X(add)
{
	int32_t rs = reg(ic->arg[0]), rt = reg(ic->arg[1]);
	int32_t rd = rs + rt;

	if ((rs >= 0 && rt >= 0 && rd < 0) || (rs < 0 && rt < 0 && rd >= 0)) {
		/*  Synch. PC and cause an exception:  */
		int low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
		    / sizeof(struct mips_instr_call);
		cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)
		    << MIPS_INSTR_ALIGNMENT_SHIFT);
		cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);
		mips_cpu_exception(cpu, EXCEPTION_OV, 0, 0, 0, 0, 0, 0);
	} else
		reg(ic->arg[2]) = rd;
}
X(daddu){ reg(ic->arg[2]) = reg(ic->arg[0]) + reg(ic->arg[1]); }
X(dadd)
{
	int64_t rs = reg(ic->arg[0]), rt = reg(ic->arg[1]);
	int64_t rd = rs + rt;

	if ((rs >= 0 && rt >= 0 && rd < 0) || (rs < 0 && rt < 0 && rd >= 0)) {
		/*  Synch. PC and cause an exception:  */
		int low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
		    / sizeof(struct mips_instr_call);
		cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)
		    << MIPS_INSTR_ALIGNMENT_SHIFT);
		cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);
		mips_cpu_exception(cpu, EXCEPTION_OV, 0, 0, 0, 0, 0, 0);
	} else
		reg(ic->arg[2]) = rd;
}
X(subu) { reg(ic->arg[2]) = (int32_t)(reg(ic->arg[0]) - reg(ic->arg[1])); }
X(sub)
{
	/*  NOTE: Negating rt and using addition. TODO: Is this correct?  */
	int32_t rs = reg(ic->arg[0]), rt = - reg(ic->arg[1]);
	int32_t rd = rs + rt;

	if ((rs >= 0 && rt >= 0 && rd < 0) || (rs < 0 && rt < 0 && rd >= 0)) {
		/*  Synch. PC and cause an exception:  */
		int low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
		    / sizeof(struct mips_instr_call);
		cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)
		    << MIPS_INSTR_ALIGNMENT_SHIFT);
		cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);
		mips_cpu_exception(cpu, EXCEPTION_OV, 0, 0, 0, 0, 0, 0);
	} else
		reg(ic->arg[2]) = rd;
}
X(dsubu){ reg(ic->arg[2]) = reg(ic->arg[0]) - reg(ic->arg[1]); }
X(dsub)
{
	/*  NOTE: Negating rt and using addition. TODO: Is this correct?  */
	int64_t rs = reg(ic->arg[0]), rt = - reg(ic->arg[1]);
	int64_t rd = rs + rt;

	if ((rs >= 0 && rt >= 0 && rd < 0) || (rs < 0 && rt < 0 && rd >= 0)) {
		/*  Synch. PC and cause an exception:  */
		int low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
		    / sizeof(struct mips_instr_call);
		cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)
		    << MIPS_INSTR_ALIGNMENT_SHIFT);
		cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);
		mips_cpu_exception(cpu, EXCEPTION_OV, 0, 0, 0, 0, 0, 0);
	} else
		reg(ic->arg[2]) = rd;
}
X(slt) {
	reg(ic->arg[2]) =
	    (MODE_int_t)reg(ic->arg[0]) < (MODE_int_t)reg(ic->arg[1]);
}
X(sltu) {
	reg(ic->arg[2]) =
	    (MODE_uint_t)reg(ic->arg[0]) < (MODE_uint_t)reg(ic->arg[1]);
}
X(and) { reg(ic->arg[2]) = reg(ic->arg[0]) & reg(ic->arg[1]); }
X(or)  { reg(ic->arg[2]) = reg(ic->arg[0]) | reg(ic->arg[1]); }
X(xor) { reg(ic->arg[2]) = reg(ic->arg[0]) ^ reg(ic->arg[1]); }
X(nor) { reg(ic->arg[2]) = ~(reg(ic->arg[0]) | reg(ic->arg[1])); }
X(sll) { reg(ic->arg[2]) = (int32_t)(reg(ic->arg[0]) << (int32_t)ic->arg[1]); }
X(sllv){ int32_t sa = reg(ic->arg[1]) & 31;
	 reg(ic->arg[2]) = (int32_t)(reg(ic->arg[0]) << sa); }
X(srl) { reg(ic->arg[2]) = (int32_t)((uint32_t)reg(ic->arg[0]) >> ic->arg[1]); }
X(srlv){ int32_t sa = reg(ic->arg[1]) & 31;
	 reg(ic->arg[2]) = (int32_t)((uint32_t)reg(ic->arg[0]) >> sa); }
X(sra) { reg(ic->arg[2]) = (int32_t)((int32_t)reg(ic->arg[0]) >> ic->arg[1]); }
X(srav){ int32_t sa = reg(ic->arg[1]) & 31;
	 reg(ic->arg[2]) = (int32_t)((int32_t)reg(ic->arg[0]) >> sa); }
X(dsll) { reg(ic->arg[2]) = (int64_t)reg(ic->arg[0]) << (int64_t)ic->arg[1]; }
X(dsllv){ int64_t sa = reg(ic->arg[1]) & 63;
	 reg(ic->arg[2]) = reg(ic->arg[0]) << sa; }
X(dsrl) { reg(ic->arg[2]) = (int64_t)((uint64_t)reg(ic->arg[0]) >>
	(uint64_t) ic->arg[1]);}
X(dsrlv){ int64_t sa = reg(ic->arg[1]) & 63;
	 reg(ic->arg[2]) = (uint64_t)reg(ic->arg[0]) >> sa; }
X(dsra) { reg(ic->arg[2]) = (int64_t)reg(ic->arg[0]) >> (int64_t)ic->arg[1]; }
X(dsrav){ int64_t sa = reg(ic->arg[1]) & 63;
	 reg(ic->arg[2]) = (int64_t)reg(ic->arg[0]) >> sa; }
X(mul) { reg(ic->arg[2]) = (int32_t)
	( (int32_t)reg(ic->arg[0]) * (int32_t)reg(ic->arg[1]) ); }
X(movn) { if (reg(ic->arg[1])) reg(ic->arg[2]) = reg(ic->arg[0]); }
X(movz) { if (!reg(ic->arg[1])) reg(ic->arg[2]) = reg(ic->arg[0]); }

X(ror)
{
	uint32_t result = reg(ic->arg[0]);
	int sa = ic->arg[1];

	result = (result >> sa) | (result << (32-sa));

	reg(ic->arg[2]) = (int32_t) result;
}


/*
 *  p*:  128-bit C790/TX79/R5900 stuff
 *
 *  arg[0] = rs (note: not a pointer)
 *  arg[1] = rt (note: not a pointer)
 *  arg[2] = rd (note: not a pointer)
 */
X(por)
{
	cpu->cd.mips.gpr[ic->arg[2]] = cpu->cd.mips.gpr[ic->arg[0]] |
	    cpu->cd.mips.gpr[ic->arg[1]];
	cpu->cd.mips.gpr_quadhi[ic->arg[2]] =
	    cpu->cd.mips.gpr_quadhi[ic->arg[0]] |
	    cpu->cd.mips.gpr_quadhi[ic->arg[1]];
}
X(pextlw)
{
	uint64_t lo, hi;

	lo = (uint32_t)cpu->cd.mips.gpr[ic->arg[1]] |
	    (uint64_t)((uint64_t)cpu->cd.mips.gpr[ic->arg[0]] << 32);
	hi = (cpu->cd.mips.gpr[ic->arg[0]] & 0xffffffff00000000ULL) |
	    (uint32_t)((uint64_t)cpu->cd.mips.gpr[ic->arg[1]] >> 32);

	cpu->cd.mips.gpr[ic->arg[2]] = lo;
	cpu->cd.mips.gpr_quadhi[ic->arg[2]] = hi;
}


/*
 *  madd, maddu, msub, msubu: Multiply-and-add/subtract
 *
 *  arg[0] = ptr to rs
 *  arg[1] = ptr to rt
 *  arg[2] = ptr to rd (only used on R5900/TX79)
 */
X(madd)
{
	int64_t rs = (int32_t)reg(ic->arg[0]), rt = (int32_t)reg(ic->arg[1]);
	int64_t sum = rs * rt,
	    hilo = (cpu->cd.mips.hi << 32) | (uint32_t)(cpu->cd.mips.lo);
	hilo += sum;
	cpu->cd.mips.hi = (int32_t)(hilo>>32); cpu->cd.mips.lo = (int32_t)hilo;
}
X(madd_rd)
{
	int64_t rs = (int32_t)reg(ic->arg[0]), rt = (int32_t)reg(ic->arg[1]);
	int64_t sum = rs * rt,
	    hilo = (cpu->cd.mips.hi << 32) | (uint32_t)(cpu->cd.mips.lo);
	hilo += sum;
	cpu->cd.mips.hi = (int32_t)(hilo>>32); cpu->cd.mips.lo = (int32_t)hilo;
	reg(ic->arg[2]) = (int32_t)hilo;
}
X(msub)
{
	int64_t rs = (int32_t)reg(ic->arg[0]), rt = (int32_t)reg(ic->arg[1]);
	int64_t sum = rs * rt,
	    hilo = (cpu->cd.mips.hi << 32) | (uint32_t)(cpu->cd.mips.lo);
	hilo -= sum;
	cpu->cd.mips.hi = (int32_t)(hilo>>32); cpu->cd.mips.lo = (int32_t)hilo;
}
X(maddu)
{
	int64_t rs = (uint32_t)reg(ic->arg[0]), rt = (uint32_t)reg(ic->arg[1]);
	int64_t sum = rs * rt,
	    hilo = (cpu->cd.mips.hi << 32) | (uint32_t)(cpu->cd.mips.lo);
	hilo += sum;
	cpu->cd.mips.hi = (int32_t)(hilo>>32); cpu->cd.mips.lo = (int32_t)hilo;
}
X(maddu_rd)
{
	int64_t rs = (uint32_t)reg(ic->arg[0]), rt = (uint32_t)reg(ic->arg[1]);
	int64_t sum = rs * rt,
	    hilo = (cpu->cd.mips.hi << 32) | (uint32_t)(cpu->cd.mips.lo);
	hilo += sum;
	cpu->cd.mips.hi = (int32_t)(hilo>>32); cpu->cd.mips.lo = (int32_t)hilo;
	reg(ic->arg[2]) = (int32_t)hilo;
}
X(msubu)
{
	int64_t rs = (uint32_t)reg(ic->arg[0]), rt = (uint32_t)reg(ic->arg[1]);
	int64_t sum = rs * rt,
	    hilo = (cpu->cd.mips.hi << 32) | (uint32_t)(cpu->cd.mips.lo);
	hilo -= sum;
	cpu->cd.mips.hi = (int32_t)(hilo>>32); cpu->cd.mips.lo = (int32_t)hilo;
}


/*
 *  mov:  Move one register into another.
 *
 *  arg[0] = pointer to source
 *  arg[2] = pointer to destination
 */
X(mov)  { reg(ic->arg[2]) = reg(ic->arg[0]); }


/*
 *  clz, clo, dclz, dclo: Count leading zeroes/ones.
 *
 *  arg[0] = pointer to rs
 *  arg[1] = pointer to rd
 */
X(clz)
{
	uint32_t x = reg(ic->arg[0]);
	int count;
	for (count=0; count<32; count++) {
		if (x & 0x80000000UL)
			break;
		x <<= 1;
	}
	reg(ic->arg[1]) = count;
}
X(clo)
{
	uint32_t x = reg(ic->arg[0]);
	int count;
	for (count=0; count<32; count++) {
		if (!(x & 0x80000000UL))
			break;
		x <<= 1;
	}
	reg(ic->arg[1]) = count;
}
X(dclz)
{
	uint64_t x = reg(ic->arg[0]);
	int count;
	for (count=0; count<64; count++) {
		if (x & 0x8000000000000000ULL)
			break;
		x <<= 1;
	}
	reg(ic->arg[1]) = count;
}
X(dclo)
{
	uint64_t x = reg(ic->arg[0]);
	int count;
	for (count=0; count<64; count++) {
		if (!(x & 0x8000000000000000ULL))
			break;
		x <<= 1;
	}
	reg(ic->arg[1]) = count;
}


/*
 *  addi, daddi: Add immediate, overflow detection.
 *  addiu, daddiu: Add immediate.
 *  slti:   Set if less than immediate (signed 32-bit)
 *  sltiu:  Set if less than immediate (signed 32-bit, but unsigned compare)
 *
 *  arg[0] = pointer to rs
 *  arg[1] = pointer to rt
 *  arg[2] = (int32_t) immediate value
 */
X(addi)
{
	int32_t rs = reg(ic->arg[0]), imm = (int32_t)ic->arg[2];
	int32_t rt = rs + imm;

	if ((rs >= 0 && imm >= 0 && rt < 0) || (rs < 0 && imm < 0 && rt >= 0)) {
		/*  Synch. PC and cause an exception:  */
		int low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
		    / sizeof(struct mips_instr_call);
		cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)
		    << MIPS_INSTR_ALIGNMENT_SHIFT);
		cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);
		mips_cpu_exception(cpu, EXCEPTION_OV, 0, 0, 0, 0, 0, 0);
	} else
		reg(ic->arg[1]) = rt;
}
X(addiu)
{
	reg(ic->arg[1]) = (int32_t)
	    ((int32_t)reg(ic->arg[0]) + (int32_t)ic->arg[2]);
}
X(daddi)
{
	int64_t rs = reg(ic->arg[0]), imm = (int32_t)ic->arg[2];
	int64_t rt = rs + imm;

	if ((rs >= 0 && imm >= 0 && rt < 0) || (rs < 0 && imm < 0 && rt >= 0)) {
		/*  Synch. PC and cause an exception:  */
		int low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
		    / sizeof(struct mips_instr_call);
		cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)
		    << MIPS_INSTR_ALIGNMENT_SHIFT);
		cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);
		mips_cpu_exception(cpu, EXCEPTION_OV, 0, 0, 0, 0, 0, 0);
	} else
		reg(ic->arg[1]) = rt;
}
X(daddiu)
{
	reg(ic->arg[1]) = reg(ic->arg[0]) + (int32_t)ic->arg[2];
}
X(slti)
{
	reg(ic->arg[1]) = (MODE_int_t)reg(ic->arg[0]) < (int32_t)ic->arg[2];
}
X(sltiu)
{
	reg(ic->arg[1]) = (MODE_uint_t)reg(ic->arg[0]) <
	   ((MODE_uint_t)(int32_t)ic->arg[2]);
}


/*
 *  set:  Set a register to an immediate (signed) 32-bit value.
 *        (This is the actual implementation of the lui instruction.)
 *
 *  arg[0] = pointer to the register
 *  arg[1] = (int32_t) immediate value
 */
X(set)
{
	reg(ic->arg[0]) = (int32_t)ic->arg[1];
}


/*
 *  cfc0:         Copy from Coprocessor 0.
 *  mfc0, dmfc0:  Move from Coprocessor 0.
 *  mtc0, dmtc0:  Move to Coprocessor 0.
 *
 *  arg[0] = pointer to GPR (rt)
 *  arg[1] = coprocessor 0 register number | (select << 5)   (or for the
 *           cfc0 instruction, the coprocessor control register number)
 *  arg[2] = relative addr of this instruction within the page
 */
X(cfc0)
{
	int fs = ic->arg[1] & 31;
	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)<<MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc |= ic->arg[2];
	/*  TODO: cause exception if necessary  */
	reg(ic->arg[0]) = (int32_t)cpu->cd.mips.coproc[0]->fcr[fs];
}
X(mfc0)
{
	int rd = ic->arg[1] & 31, select = ic->arg[1] >> 5;
	uint64_t tmp;
	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)<<MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc |= ic->arg[2];
	/*  TODO: cause exception if necessary  */
	coproc_register_read(cpu, cpu->cd.mips.coproc[0], rd, &tmp, select);
	reg(ic->arg[0]) = (int32_t)tmp;
}
X(mfc0_select0)
{
	/*  Fast int32_t read, with no side effects:  */
	int rd = ic->arg[1] & 31;
#if 0
	uint64_t tmp;
	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)<<MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc |= ic->arg[2];
	/*  TODO: cause exception if necessary  */
#endif
	reg(ic->arg[0]) = (int32_t)cpu->cd.mips.coproc[0]->reg[rd];
}
X(mtc0)
{
	int rd = ic->arg[1] & 31, select = ic->arg[1] >> 5;
	uint64_t tmp = (int32_t) reg(ic->arg[0]);

	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)<<MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc |= ic->arg[2];

	/*  TODO: cause exception if necessary  */
	coproc_register_write(cpu, cpu->cd.mips.coproc[0], rd, &tmp, 0, select);

	/*
	 *  Interrupts enabled, and any interrupt pending? (Note/TODO: This
	 *  code is duplicated in cpu_dyntrans.c. Fix this?)
	 */
	if (rd == COP0_STATUS && !cpu->delay_slot) {
		uint32_t status = cpu->cd.mips.coproc[0]->reg[COP0_STATUS];
		uint32_t cause = cpu->cd.mips.coproc[0]->reg[COP0_CAUSE];
		/*  NOTE: STATUS_IE happens to match the enable bit also
		    on R2000/R3000, so this is ok.  */
		if (cpu->cd.mips.cpu_type.exc_model != EXC3K) {
			if (status & (STATUS_EXL | STATUS_ERL))
				status &= ~STATUS_IE;
		}
		/*  Ugly R5900 special case:  (TODO: move this?)  */
		if (cpu->cd.mips.cpu_type.rev == MIPS_R5900 &&
		    !(status & R5900_STATUS_EIE))
			status &= ~STATUS_IE;
		if (status & STATUS_IE && (status & cause & STATUS_IM_MASK)) {
			cpu->pc += sizeof(uint32_t);
			mips_cpu_exception(cpu, EXCEPTION_INT, 0, 0,0,0,0,0);
		}
	}
}
X(dmfc0)
{
	int rd = ic->arg[1] & 31, select = ic->arg[1] >> 5;
	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)<<MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc |= ic->arg[2];
	/*  TODO: cause exception if necessary  */
	coproc_register_read(cpu, cpu->cd.mips.coproc[0], rd,
	    (uint64_t *)ic->arg[0], select);
}
X(dmfc0_select0)
{
	/*  Fast int64_t read, with no side effects:  */
	int rd = ic->arg[1] & 31;
#if 0
	uint64_t tmp;
	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)<<MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc |= ic->arg[2];
	/*  TODO: cause exception if necessary  */
#endif
	reg(ic->arg[0]) = cpu->cd.mips.coproc[0]->reg[rd];
}
X(dmtc0)
{
	int rd = ic->arg[1] & 31, select = ic->arg[1] >> 5;
	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)<<MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc |= ic->arg[2];
	/*  TODO: cause exception if necessary  */
	coproc_register_write(cpu, cpu->cd.mips.coproc[0], rd,
	    (uint64_t *)ic->arg[0], 1, select);
}


/*
 *  cop1_bc:  Floating point conditional branch.
 *
 *  arg[0] = cc
 *  arg[1] = nd (=2) and tf (=1) bits
 *  arg[2] = offset (relative to start of this page)
 */
X(cop1_bc)
{
	MODE_int_t old_pc = cpu->pc;
	int x, cc = ic->arg[0];

	COPROC_AVAILABILITY_CHECK(1);

	/*  Get the correct condition code bit:  */
	if (cc == 0)
		x = (cpu->cd.mips.coproc[1]->fcr[MIPS_FPU_FCSR]
		    >> MIPS_FCSR_FCC0_SHIFT) & 1;
	else
		x = (cpu->cd.mips.coproc[1]->fcr[MIPS_FPU_FCSR]
		    >> (MIPS_FCSR_FCC1_SHIFT + cc-1)) & 1;

	/*  Branch on false? Then invert the truth value.  */
	if (!(ic->arg[1] & 1))
		x ^= 1;

	/*  Execute the delay slot (except if it is nullified):  */
	cpu->delay_slot = TO_BE_DELAYED;
	if (x || !(ic->arg[1] & 2))
		ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;

	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (x) {
			old_pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1) <<
			    MIPS_INSTR_ALIGNMENT_SHIFT);
			cpu->pc = old_pc + (int32_t)ic->arg[2];
			quick_pc_to_pointers(cpu);
		} else
			cpu->cd.mips.next_ic ++;
	} else
		cpu->delay_slot = NOT_DELAYED;
}


/*
 *  cop1_slow:  Fallback to legacy cop1 code. (Slow, but it should work.)
 */
X(cop1_slow)
{
	COPROC_AVAILABILITY_CHECK(1);

	coproc_function(cpu, cpu->cd.mips.coproc[1], 1, ic->arg[0], 0, 1);
}


/*
 *  syscall, break:  Synchronize the PC and cause an exception.
 */
X(syscall)
{
	int low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
	    / sizeof(struct mips_instr_call);
	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)<< MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);
	mips_cpu_exception(cpu, EXCEPTION_SYS, 0, 0, 0, 0, 0, 0);
}
X(break)
{
	int low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
	    / sizeof(struct mips_instr_call);
	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)<< MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);
	mips_cpu_exception(cpu, EXCEPTION_BP, 0, 0, 0, 0, 0, 0);
}
X(reboot)
{
	if (!cop0_availability_check(cpu, ic))
		return;

	cpu->running = 0;
	debugger_n_steps_left_before_interaction = 0;
	cpu->cd.mips.next_ic = &nothing_call;
}


/*
 *  promemul:  PROM software emulation.
 */
X(promemul)
{
	/*  Synchronize the PC and call the correct emulation layer:  */
	MODE_int_t old_pc;
	int res, low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
	    / sizeof(struct mips_instr_call);
	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)<< MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);
	old_pc = cpu->pc;

	switch (cpu->machine->machine_type) {
	case MACHINE_PMAX:
		res = decstation_prom_emul(cpu);
		break;
	case MACHINE_PS2:
		res = playstation2_sifbios_emul(cpu);
		break;
	case MACHINE_ARC:
	case MACHINE_SGI:
		res = arcbios_emul(cpu);
		break;
	case MACHINE_EVBMIPS:
		res = yamon_emul(cpu);
		break;
	default:fatal("TODO: Unimplemented machine type for PROM magic trap\n");
		exit(1);
	}

	if (res) {
		/*  Return from the PROM call:  */
		cpu->pc = (MODE_int_t)cpu->cd.mips.gpr[MIPS_GPR_RA];
		cpu->delay_slot = NOT_DELAYED;

		if (cpu->machine->show_trace_tree)
			cpu_functioncall_trace_return(cpu);
	} else {
		/*  The PROM call blocks.  */
		cpu->n_translated_instrs += 10;
		cpu->pc = old_pc;
	}

	quick_pc_to_pointers(cpu);
}


/*
 *  tlbw: TLB write indexed and random
 *
 *  arg[0] = 1 for random, 0 for indexed
 *  arg[2] = relative addr of this instruction within the page
 */
X(tlbw)
{
	if (!cop0_availability_check(cpu, ic))
		return;

	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)<<MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc |= ic->arg[2];
	coproc_tlbwri(cpu, ic->arg[0]);
}


/*
 *  tlbp: TLB probe
 *  tlbr: TLB read
 *
 *  arg[2] = relative addr of this instruction within the page
 */
X(tlbp)
{
	if (!cop0_availability_check(cpu, ic))
		return;

	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)<<MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc |= ic->arg[2];
	coproc_tlbpr(cpu, 0);
}
X(tlbr)
{
	if (!cop0_availability_check(cpu, ic))
		return;

	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)<<MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc |= ic->arg[2];
	coproc_tlbpr(cpu, 1);
}


/*
 *  ei_or_di:  MIPS32/64 rev 2, Enable or disable interrupts
 *
 *  arg[0] = ptr to rt
 *  arg[1] = non-zero to enable interrupts
 */
X(ei_or_di)
{
	reg(ic->arg[0]) = cpu->cd.mips.coproc[0]->reg[COP0_STATUS];
	if (ic->arg[1])
		cpu->cd.mips.coproc[0]->reg[COP0_STATUS] |= STATUS_IE;
	else
		cpu->cd.mips.coproc[0]->reg[COP0_STATUS] &= ~STATUS_IE;
}


/*
 *  rfe: Return from exception handler (R2000/R3000)
 */
X(rfe)
{
	if (!cop0_availability_check(cpu, ic))
		return;

	/*  Just rotate the interrupt/user bits:  */
	cpu->cd.mips.coproc[0]->reg[COP0_STATUS] =
	    (cpu->cd.mips.coproc[0]->reg[COP0_STATUS] & ~0x3f) |
	    ((cpu->cd.mips.coproc[0]->reg[COP0_STATUS] & 0x3c) >> 2);

	/*
	 *  Note: no pc to pointers conversion is necessary here. Usually the
	 *  rfe instruction resides in the delay slot of a jr k0/k1, and
	 *  it is up to that instruction to do the pointer conversion.
	 */
}


/*
 *  eret: Return from exception handler (non-R3000 style)
 */
X(eret)
{
	if (!cop0_availability_check(cpu, ic))
		return;

	if (cpu->cd.mips.coproc[0]->reg[COP0_STATUS] & STATUS_ERL) {
		cpu->pc = cpu->cd.mips.coproc[0]->reg[COP0_ERROREPC];
		cpu->cd.mips.coproc[0]->reg[COP0_STATUS] &= ~STATUS_ERL;
	} else {
		cpu->pc = cpu->cd.mips.coproc[0]->reg[COP0_EPC];
		cpu->delay_slot = 0;             
		cpu->cd.mips.coproc[0]->reg[COP0_STATUS] &= ~STATUS_EXL;
	}

	quick_pc_to_pointers(cpu);

	cpu->cd.mips.rmw = 0;   /*  the "LL bit"  */
}


/*
 *  deret: Return from debug (EJTAG) handler
 */
X(deret)
{
	if (!cop0_availability_check(cpu, ic))
		return;

	/*
	 *  According to the MIPS64 manual, deret loads PC from the DEPC cop0
	 *  register, and jumps there immediately. No delay slot.
	 *
	 *  TODO: This instruction is only available if the processor is in
	 *  debug mode. (What does that mean?)
	 *
	 *  TODO: This instruction is undefined in a delay slot.
	 */

	cpu->pc = cpu->cd.mips.coproc[0]->reg[COP0_DEPC];
	cpu->delay_slot = 0;
	cpu->cd.mips.coproc[0]->reg[COP0_STATUS] &= ~STATUS_EXL;
	quick_pc_to_pointers(cpu);
}


/*
 *  idle:  Called from the implementation of wait, or netbsd_pmax_idle.
 */
X(idle)
{
	/*
	 *  If there is an interrupt, then just return. Otherwise
	 *  re-run the wait instruction (after a delay).
	 */
	uint32_t status = cpu->cd.mips.coproc[0]->reg[COP0_STATUS];
	uint32_t cause = cpu->cd.mips.coproc[0]->reg[COP0_CAUSE];

	if (cpu->cd.mips.cpu_type.exc_model != EXC3K) {
		if (status & (STATUS_EXL | STATUS_ERL))
			status &= ~STATUS_IE;
	}

	/*  Ugly R5900 special case:  (TODO: move this?)  */
	if (cpu->cd.mips.cpu_type.rev == MIPS_R5900 &&
	    !(status & R5900_STATUS_EIE))
		status &= ~STATUS_IE;
	if (status & STATUS_IE && (status & cause & STATUS_IM_MASK))
		return;

	cpu->cd.mips.next_ic = ic;
	cpu->is_halted = 1;
	cpu->has_been_idling = 1;

	/*
	 *  There was no interrupt. Go to sleep.
	 *
	 *  TODO:
	 *
	 *  Think about how to actually implement this usleep stuff,
	 *  in an SMP and/or timing accurate environment.
	 */

	if (cpu->machine->ncpus == 1) {
		static int x = 0;

		if ((++x) == 300) {
			usleep(20);
			x = 0;
		}

		cpu->n_translated_instrs += N_SAFE_DYNTRANS_LIMIT / 6;
	}
}


/*
 *  wait: Wait for external interrupt.
 */
X(wait)
{
	if (!cop0_availability_check(cpu, ic))
		return;

	instr(idle)(cpu, ic);
}


/*
 *  rdhwr: Read hardware register into gpr (MIPS32/64 rev 2).
 *
 *  arg[0] = ptr to rt (destination register)
 */
X(rdhwr_cpunum)
{
	reg(ic->arg[0]) = cpu->cpu_id;
}


#include "tmp_mips_loadstore.cc"


/*
 *  Load linked / store conditional:
 *
 *  A Load-linked instruction initiates a RMW (read-modify-write) sequence.
 *  COP0_LLADDR is updated for diagnostic purposes, except for CPUs in the
 *  R10000 family.
 *
 *  A Store-conditional instruction ends the sequence.
 *
 *  arg[0] = ptr to rt
 *  arg[1] = ptr to rs
 *  arg[2] = int32_t imm
 */
X(ll)
{
	MODE_int_t addr = reg(ic->arg[1]) + (int32_t)ic->arg[2];
	int low_pc;
	uint8_t word[sizeof(uint32_t)];

	/*  Synch. PC and load using slow memory_rw():  */
	low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
	    / sizeof(struct mips_instr_call);
	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)
	    << MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);

	if (addr & (sizeof(word)-1)) {
		fatal("TODO: load linked unaligned access: exception\n");
		exit(1);
	}

	if (!cpu->memory_rw(cpu, cpu->mem, addr, word,
	    sizeof(word), MEM_READ, CACHE_DATA)) {
		/*  An exception occurred.  */
		return;
	}

	cpu->cd.mips.rmw = 1;
	cpu->cd.mips.rmw_addr = addr;
	cpu->cd.mips.rmw_len = sizeof(word);
	if (cpu->cd.mips.cpu_type.exc_model != MMU10K)
		cpu->cd.mips.coproc[0]->reg[COP0_LLADDR] =
		    (addr >> 4) & 0xffffffffULL;

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		reg(ic->arg[0]) = (int32_t) (word[0] + (word[1] << 8)
		    + (word[2] << 16) + (word[3] << 24));
	else
		reg(ic->arg[0]) = (int32_t) (word[3] + (word[2] << 8)
		    + (word[1] << 16) + (word[0] << 24));
}
X(lld)
{
	MODE_int_t addr = reg(ic->arg[1]) + (int32_t)ic->arg[2];
	int low_pc;
	uint8_t word[sizeof(uint64_t)];

	/*  Synch. PC and load using slow memory_rw():  */
	low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
	    / sizeof(struct mips_instr_call);
	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)
	    << MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);

	if (addr & (sizeof(word)-1)) {
		fatal("TODO: load linked unaligned access: exception\n");
		exit(1);
	}

	if (!cpu->memory_rw(cpu, cpu->mem, addr, word,
	    sizeof(word), MEM_READ, CACHE_DATA)) {
		/*  An exception occurred.  */
		return;
	}

	cpu->cd.mips.rmw = 1;
	cpu->cd.mips.rmw_addr = addr;
	cpu->cd.mips.rmw_len = sizeof(word);
	if (cpu->cd.mips.cpu_type.exc_model != MMU10K)
		cpu->cd.mips.coproc[0]->reg[COP0_LLADDR] =
		    (addr >> 4) & 0xffffffffULL;

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		reg(ic->arg[0]) = word[0] + (word[1] << 8)
		    + (word[2] << 16) + ((uint64_t)word[3] << 24) +
		    + ((uint64_t)word[4] << 32) + ((uint64_t)word[5] << 40)
		    + ((uint64_t)word[6] << 48) + ((uint64_t)word[7] << 56);
	else
		reg(ic->arg[0]) = word[7] + (word[6] << 8)
		    + (word[5] << 16) + ((uint64_t)word[4] << 24) +
		    + ((uint64_t)word[3] << 32) + ((uint64_t)word[2] << 40)
		    + ((uint64_t)word[1] << 48) + ((uint64_t)word[0] << 56);
}
X(sc)
{
	MODE_int_t addr = reg(ic->arg[1]) + (int32_t)ic->arg[2];
	uint64_t r = reg(ic->arg[0]);
	int low_pc, i;
	uint8_t word[sizeof(uint32_t)];

	/*  Synch. PC and store using slow memory_rw():  */
	low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
	    / sizeof(struct mips_instr_call);
	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)
	    << MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);

	if (addr & (sizeof(word)-1)) {
		fatal("TODO: sc unaligned access: exception\n");
		exit(1);
	}

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN) {
		word[0]=r; word[1]=r>>8; word[2]=r>>16; word[3]=r>>24;
	} else {
		word[3]=r; word[2]=r>>8; word[1]=r>>16; word[0]=r>>24;
	}

	/*  If rmw is 0, then the store failed.  (This cache-line was written
	    to by someone else.)  */
	if (cpu->cd.mips.rmw == 0 || (MODE_int_t)cpu->cd.mips.rmw_addr != addr
	    || cpu->cd.mips.rmw_len != sizeof(word)) {
		reg(ic->arg[0]) = 0;
		cpu->cd.mips.rmw = 0;
		return;
	}

	if (!cpu->memory_rw(cpu, cpu->mem, addr, word,
	    sizeof(word), MEM_WRITE, CACHE_DATA)) {
		/*  An exception occurred.  */
		return;
	}

	/*  We succeeded. Let's invalidate everybody else's store to this
	    cache line:  */
	for (i=0; i<cpu->machine->ncpus; i++) {
		if (cpu->machine->cpus[i]->cd.mips.rmw) {
			uint64_t yaddr = addr, xaddr = cpu->machine->cpus[i]->
			    cd.mips.rmw_addr;
			uint64_t mask = ~(cpu->machine->cpus[i]->
			    cd.mips.cache_linesize[CACHE_DATA] - 1);
			xaddr &= mask;
			yaddr &= mask;
			if (xaddr == yaddr)
				cpu->machine->cpus[i]->cd.mips.rmw = 0;
		}
	}

	reg(ic->arg[0]) = 1;
	cpu->cd.mips.rmw = 0;
}
X(scd)
{
	MODE_int_t addr = reg(ic->arg[1]) + (int32_t)ic->arg[2];
	uint64_t r = reg(ic->arg[0]);
	int low_pc, i;
	uint8_t word[sizeof(uint64_t)];

	/*  Synch. PC and store using slow memory_rw():  */
	low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
	    / sizeof(struct mips_instr_call);
	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)
	    << MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);

	if (addr & (sizeof(word)-1)) {
		fatal("TODO: sc unaligned access: exception\n");
		exit(1);
	}

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN) {
		word[0]=r;     word[1]=r>>8; word[2]=r>>16; word[3]=r>>24;
		word[4]=r>>32; word[5]=r>>40; word[6]=r>>48; word[7]=r>>56;
	} else {
		word[7]=r;     word[6]=r>>8; word[5]=r>>16; word[4]=r>>24;
		word[3]=r>>32; word[2]=r>>40; word[1]=r>>48; word[0]=r>>56;
	}

	/*  If rmw is 0, then the store failed.  (This cache-line was written
	    to by someone else.)  */
	if (cpu->cd.mips.rmw == 0 || (MODE_int_t)cpu->cd.mips.rmw_addr != addr
	    || cpu->cd.mips.rmw_len != sizeof(word)) {
		reg(ic->arg[0]) = 0;
		cpu->cd.mips.rmw = 0;
		return;
	}

	if (!cpu->memory_rw(cpu, cpu->mem, addr, word,
	    sizeof(word), MEM_WRITE, CACHE_DATA)) {
		/*  An exception occurred.  */
		return;
	}

	/*  We succeeded. Let's invalidate everybody else's store to this
	    cache line:  */
	for (i=0; i<cpu->machine->ncpus; i++) {
		if (cpu->machine->cpus[i]->cd.mips.rmw) {
			uint64_t yaddr = addr, xaddr = cpu->machine->cpus[i]->
			    cd.mips.rmw_addr;
			uint64_t mask = ~(cpu->machine->cpus[i]->
			    cd.mips.cache_linesize[CACHE_DATA] - 1);
			xaddr &= mask;
			yaddr &= mask;
			if (xaddr == yaddr)
				cpu->machine->cpus[i]->cd.mips.rmw = 0;
		}
	}

	reg(ic->arg[0]) = 1;
	cpu->cd.mips.rmw = 0;
}


/*
 *  lwc1, swc1:  Coprocessor 1 load/store (32-bit)
 *  ldc1, sdc1:  Coprocessor 1 load/store (64-bit)
 *
 *  arg[0] = ptr to coprocessor register
 *  arg[1] = ptr to rs (base pointer register)
 *  arg[2] = int32_t imm
 */
X(lwc1)
{
	COPROC_AVAILABILITY_CHECK(1);

#ifdef MODE32
	mips32_loadstore
#else
	mips_loadstore
#endif
	    [ (cpu->byte_order == EMUL_LITTLE_ENDIAN? 0 : 16) + 2 * 2 + 1]
	    (cpu, ic);
}
X(swc1)
{
	COPROC_AVAILABILITY_CHECK(1);

#ifdef MODE32
	mips32_loadstore
#else
	mips_loadstore
#endif
	    [ (cpu->byte_order == EMUL_LITTLE_ENDIAN? 0 : 16) + 8 + 2 * 2]
	    (cpu, ic);
}
X(ldc1)
{
	int use_fp_pairs =
	    !(cpu->cd.mips.coproc[0]->reg[COP0_STATUS] & STATUS_FR);
	uint64_t fpr, *backup_ptr;

	COPROC_AVAILABILITY_CHECK(1);

	backup_ptr = (uint64_t *) ic->arg[0];
	ic->arg[0] = (size_t) &fpr;

#ifdef MODE32
	mips32_loadstore
#else
	mips_loadstore
#endif
	    [ (cpu->byte_order == EMUL_LITTLE_ENDIAN? 0 : 16) + 3 * 2 + 1]
	    (cpu, ic);

	if (use_fp_pairs) {
		backup_ptr[0] = (int64_t)(int32_t) fpr;
		backup_ptr[1] = (int64_t)(int32_t) (fpr >> 32);
	} else {
		*backup_ptr = fpr;
	}

	ic->arg[0] = (size_t) backup_ptr;
}
X(sdc1)
{
	int use_fp_pairs =
	    !(cpu->cd.mips.coproc[0]->reg[COP0_STATUS] & STATUS_FR);
	uint64_t fpr, *backup_ptr;

	COPROC_AVAILABILITY_CHECK(1);

	backup_ptr = (uint64_t *) ic->arg[0];
	ic->arg[0] = (size_t) &fpr;

	if (use_fp_pairs) {
		uint32_t lo = backup_ptr[0];
		uint32_t hi = backup_ptr[1];
		fpr = (((uint64_t)hi) << 32) | lo;
	} else {
		fpr = *backup_ptr;
	}

#ifdef MODE32
	mips32_loadstore
#else
	mips_loadstore
#endif
	    [ (cpu->byte_order == EMUL_LITTLE_ENDIAN? 0 : 16) + 8 + 3 * 2]
	    (cpu, ic);

	ic->arg[0] = (size_t) backup_ptr;
}


/*
 *  Unaligned loads/stores:
 *
 *  arg[0] = ptr to rt
 *  arg[1] = ptr to rs
 *  arg[2] = int32_t imm
 */
X(lwl) { mips_unaligned_loadstore(cpu, ic, 1, sizeof(uint32_t), 0); }
X(lwr) { mips_unaligned_loadstore(cpu, ic, 0, sizeof(uint32_t), 0); }
X(ldl) { mips_unaligned_loadstore(cpu, ic, 1, sizeof(uint64_t), 0); }
X(ldr) { mips_unaligned_loadstore(cpu, ic, 0, sizeof(uint64_t), 0); }
X(swl) { mips_unaligned_loadstore(cpu, ic, 1, sizeof(uint32_t), 1); }
X(swr) { mips_unaligned_loadstore(cpu, ic, 0, sizeof(uint32_t), 1); }
X(sdl) { mips_unaligned_loadstore(cpu, ic, 1, sizeof(uint64_t), 1); }
X(sdr) { mips_unaligned_loadstore(cpu, ic, 0, sizeof(uint64_t), 1); }


/*
 *  di, ei: R5900 interrupt enable/disable.
 *
 *  TODO: check the R5900_STATUS_EDI bit in the status register. If it is
 *  cleared, and we are not running in kernel mode, then both the EI and DI
 *  instructions should be treated as NOPs!
 */
X(di_r5900)
{
	if (!cop0_availability_check(cpu, ic))
		return;

	cpu->cd.mips.coproc[0]->reg[COP0_STATUS] &= ~R5900_STATUS_EIE;
}
X(ei_r5900)
{
	if (!cop0_availability_check(cpu, ic))
		return;

	cpu->cd.mips.coproc[0]->reg[COP0_STATUS] |= R5900_STATUS_EIE;
}


/*****************************************************************************/


/*
 *  sw_loop:
 *
 *  s:	addiu	rX,rX,4			rX = arg[0] and arg[1]
 *	bne	rY,rX,s  (or rX,rY,s)	rt=arg[1], rs=arg[0]
 *	sw	rZ,-4(rX)		rt=arg[0], rs=arg[1]
 */
X(sw_loop)
{
	MODE_uint_t rX = reg(ic->arg[0]), rZ = reg(ic[2].arg[0]);
	uint64_t *rYp = (uint64_t *) ic[1].arg[0];
	MODE_uint_t rY, bytes_to_write;
	unsigned char *page;
	int partial = 0;

	page = cpu->cd.mips.host_store[rX >> 12];

	/*  Fallback:  */
	if (cpu->delay_slot || page == NULL || (rX & 3) != 0 || rZ != 0) {
		instr(addiu)(cpu, ic);
		return;
	}

	if (rYp == (uint64_t *) ic->arg[0])
		rYp = (uint64_t *) ic[1].arg[1];

	rY = reg(rYp);

	bytes_to_write = rY - rX;
	if ((rX & 0xfff) + bytes_to_write > 0x1000) {
		bytes_to_write = 0x1000 - (rX & 0xfff);
		partial = 1;
	}

	/*  printf("rX = %08x\n", (int)rX);
	    printf("rY = %08x\n", (int)rY);
	    printf("rZ = %08x\n", (int)rZ);
	    printf("%i bytes\n", (int)bytes_to_write);  */

	memset(page + (rX & 0xfff), 0, bytes_to_write);

	reg(ic->arg[0]) = rX + bytes_to_write;

	cpu->n_translated_instrs += bytes_to_write / 4 * 3 - 1;
	cpu->cd.mips.next_ic = partial?
	    (struct mips_instr_call *) &ic[0] :
	    (struct mips_instr_call *) &ic[3];
}


#ifdef MODE32
/*  multi_{l,s}w_2, _3, etc.  */
#include "tmp_mips_loadstore_multi.cc"
#endif


/*
 *  multi_addu_3:
 */
X(multi_addu_3)
{
	/*  Fallback:  */
	if (cpu->delay_slot) {
		instr(addu)(cpu, ic);
		return;
	}

	reg(ic[0].arg[2]) = (int32_t)(reg(ic[0].arg[0]) + reg(ic[0].arg[1]));
	reg(ic[1].arg[2]) = (int32_t)(reg(ic[1].arg[0]) + reg(ic[1].arg[1]));
	reg(ic[2].arg[2]) = (int32_t)(reg(ic[2].arg[0]) + reg(ic[2].arg[1]));
	cpu->n_translated_instrs += 2;
	cpu->cd.mips.next_ic = ic + 3;
}


/*
 *  netbsd_r3k_picache_do_inv:
 *
 *  ic[0]	mtc0	rV,status
 *     1	nop
 *     2	nop
 *     3  s:	addiu	rX,rX,4
 *     4	bne	rY,rX,s
 *     5	sb	zr,-4(rX)
 *     6	nop
 *     7	nop
 *     8	mtc0	rT,status
 */
X(netbsd_r3k_picache_do_inv)
{
	MODE_uint_t rx = reg(ic[3].arg[0]), ry = reg(ic[4].arg[1]);

	/*  Fallback if the environment isn't exactly right:  */
	if (!(reg(ic[0].arg[0]) & MIPS1_ISOL_CACHES) ||
	    (rx & 3) || (ry & 3) || cpu->delay_slot) {
		instr(mtc0)(cpu, ic);
		return;
        }

	reg(ic[3].arg[0]) = ry;
	cpu->n_translated_instrs += (ry - rx + 4) / 4 * 3 + 4;

	/*  Run the last mtc0 instruction:  */
	cpu->cd.mips.next_ic = ic + 8;
}


#ifdef MODE32
/*
 *  netbsd_pmax_idle():
 *
 *  s:  lui     rX, hi
 *      lw      rY, lo(rX)
 *      nop
 *      beq     zr, rY, s
 *      nop
 */
X(netbsd_pmax_idle)
{
	uint32_t addr, pageindex, i;
	int32_t *page;

	reg(ic[0].arg[0]) = (int32_t)ic[0].arg[1];

	addr = reg(ic[0].arg[0]) + (int32_t)ic[1].arg[2];
	pageindex = addr >> 12;
	i = (addr & 0xfff) >> 2;
	page = (int32_t *) cpu->cd.mips.host_load[pageindex];

	/*  Fallback:  */
	if (cpu->delay_slot || page == NULL || page[i] != 0)
		return;

	instr(idle)(cpu, ic);
}


/*
 *  linux_pmax_idle():
 *
 *  s:  lui     rX, hi
 *      lw      rX, lo(rX)
 *      nop
 *      bne     zr, rX, ...
 *      nop
 *      lw      rX, ofs(gp)
 *      nop
 *      beq     zr, rX, s
 *      nop
 */
X(linux_pmax_idle)
{
	uint32_t addr, addr2, pageindex, pageindex2, i, i2;
	int32_t *page, *page2;

	reg(ic[0].arg[0]) = (int32_t)ic[0].arg[1];

	addr = reg(ic[0].arg[0]) + (int32_t)ic[1].arg[2];
	pageindex = addr >> 12;
	i = (addr & 0xfff) >> 2;
	page = (int32_t *) cpu->cd.mips.host_load[pageindex];

	addr2 = reg(ic[5].arg[1]) + (int32_t)ic[5].arg[2];
	pageindex2 = addr2 >> 12;
	i2 = (addr2 & 0xfff) >> 2;
	page2 = (int32_t *) cpu->cd.mips.host_load[pageindex2];

	/*  Fallback:  */
	if (cpu->delay_slot || page == NULL || page[i] != 0 || page2[i2] != 0)
		return;

	instr(idle)(cpu, ic);
}


/*
 *  netbsd_strlen():
 *
 *	lb      rV,0(rX)
 *   s:	addiu   rX,rX,1
 *	bne	zr,rV,s
 *	nop
 */
X(netbsd_strlen)
{
	MODE_uint_t rx = reg(ic[0].arg[1]);
	MODE_int_t rv;
	signed char *page;
	uint32_t pageindex = rx >> 12;
	int i;

	page = (signed char *) cpu->cd.mips.host_load[pageindex];

	/*  Fallback:  */
	if (cpu->delay_slot || page == NULL) {
		/*
		 *  Normal lb:  NOTE: It doesn't matter whether [1] or
		 *  [16+1] is called here, because endianness for 8-bit
		 *  loads is irrelevant. :-)
		 */
		mips32_loadstore[1](cpu, ic);
		return;
	}

	i = rx & 0xfff;

	/*
	 *  TODO: This loop can be optimized further for optimal
	 *  performance on the host, e.g. by reading full words...
	 */
	do {
		rv = page[i ++];
	} while (i < 0x1000 && rv != 0);

	cpu->n_translated_instrs += (i - (rx & 0xfff)) * 4 - 1;

	reg(ic[0].arg[1]) = (rx & ~0xfff) + i;
	reg(ic[2].arg[0]) = rv;

	/*  Done with the loop? Or continue on the next rx page?  */
	if (rv == 0)
		cpu->cd.mips.next_ic = ic + 4;
	else
		cpu->cd.mips.next_ic = ic;
}
#endif


/*
 *  addiu_bne_samepage_addiu:
 */
X(addiu_bne_samepage_addiu)
{
	MODE_uint_t rs, rt;

	if (cpu->delay_slot) {
		instr(addiu)(cpu, ic);
		return;
	}

	cpu->n_translated_instrs += 2;
	reg(ic[0].arg[1]) = (int32_t)
	    ((int32_t)reg(ic[0].arg[0]) + (int32_t)ic[0].arg[2]);
	rs = reg(ic[1].arg[0]);
	rt = reg(ic[1].arg[1]);
	reg(ic[2].arg[1]) = (int32_t)
	    ((int32_t)reg(ic[2].arg[0]) + (int32_t)ic[2].arg[2]);
	if (rs != rt)
		cpu->cd.mips.next_ic = (struct mips_instr_call *) ic[1].arg[2];
	else
		cpu->cd.mips.next_ic = ic + 3;
}


/*
 *  xor_andi_sll:
 */
X(xor_andi_sll)
{
	/*  Fallback:  */
	if (cpu->delay_slot) {
		instr(xor)(cpu, ic);
		return;
	}

	reg(ic[0].arg[2]) = reg(ic[0].arg[0]) ^ reg(ic[0].arg[1]);
	reg(ic[1].arg[1]) = reg(ic[1].arg[0]) & (uint32_t)ic[1].arg[2];
	reg(ic[2].arg[2]) = (int32_t)(reg(ic[2].arg[0])<<(int32_t)ic[2].arg[1]);

	cpu->n_translated_instrs += 2;
	cpu->cd.mips.next_ic = ic + 3;
}


/*
 *  andi_sll:
 */
X(andi_sll)
{
	/*  Fallback:  */
	if (cpu->delay_slot) {
		instr(andi)(cpu, ic);
		return;
	}

	reg(ic[0].arg[1]) = reg(ic[0].arg[0]) & (uint32_t)ic[0].arg[2];
	reg(ic[1].arg[2]) = (int32_t)(reg(ic[1].arg[0])<<(int32_t)ic[1].arg[1]);

	cpu->n_translated_instrs ++;
	cpu->cd.mips.next_ic = ic + 2;
}


/*
 *  lui_ori:
 */
X(lui_ori)
{
	/*  Fallback:  */
	if (cpu->delay_slot) {
		instr(set)(cpu, ic);
		return;
	}

	reg(ic[0].arg[0]) = (int32_t)ic[0].arg[1];
	reg(ic[1].arg[1]) = reg(ic[1].arg[0]) | (uint32_t)ic[1].arg[2];

	cpu->n_translated_instrs ++;
	cpu->cd.mips.next_ic = ic + 2;
}


/*
 *  lui_addiu:
 */
X(lui_addiu)
{
	/*  Fallback:  */
	if (cpu->delay_slot) {
		instr(set)(cpu, ic);
		return;
	}

	reg(ic[0].arg[0]) = (int32_t)ic[0].arg[1];
	reg(ic[1].arg[1]) = (int32_t)
	    ((int32_t)reg(ic[1].arg[0]) + (int32_t)ic[1].arg[2]);

	cpu->n_translated_instrs ++;
	cpu->cd.mips.next_ic = ic + 2;
}


/*
 *  b_samepage_addiu:
 *
 *  Combination of branch within the same page, followed by addiu.
 */
X(b_samepage_addiu)
{
	reg(ic[1].arg[1]) = (int32_t)
	    ( (int32_t)reg(ic[1].arg[0]) + (int32_t)ic[1].arg[2] );
	cpu->n_translated_instrs ++;
	cpu->cd.mips.next_ic = (struct mips_instr_call *) ic->arg[2];
}


/*
 *  b_samepage_daddiu:
 *
 *  Combination of branch within the same page, followed by daddiu.
 */
X(b_samepage_daddiu)
{
	*(uint64_t *)ic[1].arg[1] = *(uint64_t *)ic[1].arg[0] +
	    (int32_t)ic[1].arg[2];
	cpu->n_translated_instrs ++;
	cpu->cd.mips.next_ic = (struct mips_instr_call *) ic->arg[2];
}


/*****************************************************************************/


X(end_of_page)
{
	/*  Update the PC:  (offset 0, but on the next page)  */
	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1) <<
	    MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (MIPS_IC_ENTRIES_PER_PAGE << MIPS_INSTR_ALIGNMENT_SHIFT);

	/*  end_of_page doesn't count as an executed instruction:  */
	cpu->n_translated_instrs --;

	/*
	 *  Find the new physpage and update translation pointers.
	 *
	 *  Note: This may cause an exception, if e.g. the new page is
	 *  not accessible.
	 */
	quick_pc_to_pointers(cpu);

	/*  Simple jump to the next page (if we are lucky):  */
	if (cpu->delay_slot == NOT_DELAYED)
		return;

	/*
	 *  If we were in a delay slot, and we got an exception while doing
	 *  quick_pc_to_pointers, then return. The function which called
	 *  end_of_page should handle this case.
	 */
	if (cpu->delay_slot == EXCEPTION_IN_DELAY_SLOT)
		return;

	/*
	 *  Tricky situation; the delay slot is on the next virtual page.
	 *  Calling to_be_translated will translate one instruction manually,
	 *  execute it, and then discard it.
	 */
	/*  fatal("[ end_of_page: delay slot across page boundary! ]\n");  */

	instr(to_be_translated)(cpu, cpu->cd.mips.next_ic);

	/*  The instruction in the delay slot has now executed.  */
	/*  fatal("[ end_of_page: back from executing the delay slot, %i ]\n",
	    cpu->delay_slot);  */

	/*  Find the physpage etc of the instruction in the delay slot
	    (or, if there was an exception, the exception handler):  */
	quick_pc_to_pointers(cpu);
}


X(end_of_page2)
{
	/*  Synchronize PC on the _second_ instruction on the next page:  */
	int low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
	    / sizeof(struct mips_instr_call);
	cpu->pc &= ~((MIPS_IC_ENTRIES_PER_PAGE-1)
	    << MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);

	/*  This doesn't count as an executed instruction.  */
	cpu->n_translated_instrs --;

	quick_pc_to_pointers(cpu);

	if (cpu->delay_slot == NOT_DELAYED)
		return;

	fatal("end_of_page2: fatal error, we're in a delay slot\n");
	exit(1);
}


/*****************************************************************************/


/*
 *  Combine:  Memory fill loop (addiu, bne, sw)
 *
 *  s:	addiu	rX,rX,4
 *	bne	rY,rX,s
 *	sw	rZ,-4(rX)
 */
void COMBINE(sw_loop)(struct cpu *cpu, struct mips_instr_call *ic, int low_addr)
{
	int n_back = (low_addr >> MIPS_INSTR_ALIGNMENT_SHIFT)
	    & (MIPS_IC_ENTRIES_PER_PAGE - 1);

	/*  Only for 32-bit virtual address translation so far.  */
	if (!cpu->is_32bit)
		return;

	if (n_back < 2)
		return;

	if (ic[-2].f == instr(addiu) && ic[-2].arg[0] == ic[-2].arg[1] &&
	    (int32_t)ic[-2].arg[2] == 4 &&
	    ic[-1].f == instr(bne_samepage) &&
	    (ic[-1].arg[0] == ic[-2].arg[0] ||
		ic[-1].arg[1] == ic[-2].arg[0]) &&
	    ic[-1].arg[0] != ic[-1].arg[1] &&
	    ic[-1].arg[2] == (size_t) &ic[-2] &&
	    ic[0].arg[0] != ic[0].arg[1] &&
	    ic[0].arg[1] == ic[-2].arg[0] && (int32_t)ic[0].arg[2] == -4) {
		ic[-2].f = instr(sw_loop);
	}
}


/*  Only for 32-bit virtual address translation so far.  */
#ifdef MODE32
/*
 *  Combine:  Multiple SW in a row using the same base register
 *
 *	sw	r?,???(rX)
 *	sw	r?,???(rX)
 *	sw	r?,???(rX)
 *	...
 */
void COMBINE(multi_sw)(struct cpu *cpu, struct mips_instr_call *ic,
	int low_addr)
{
	int n_back = (low_addr >> MIPS_INSTR_ALIGNMENT_SHIFT)
	    & (MIPS_IC_ENTRIES_PER_PAGE - 1);

	if (n_back < 3)
		return;

	/*  Convert a multi_sw_3 to a multi_sw_4:  */
	if ((ic[-3].f == instr(multi_sw_3_be) ||
	    ic[-3].f == instr(multi_sw_3_le)) &&
	    ic[-3].arg[1] == ic[0].arg[1]) {
		if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
			ic[-3].f = instr(multi_sw_4_le);
		else
			ic[-3].f = instr(multi_sw_4_be);
	}

	/*  Convert a multi_sw_2 to a multi_sw_3:  */
	if ((ic[-2].f == instr(multi_sw_2_be) ||
	    ic[-2].f == instr(multi_sw_2_le)) &&
	    ic[-2].arg[1] == ic[0].arg[1]) {
		if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
			ic[-2].f = instr(multi_sw_3_le);
		else
			ic[-2].f = instr(multi_sw_3_be);
	}

	if (ic[-1].f == ic[0].f && ic[-1].arg[1] == ic[0].arg[1]) {
		if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
			ic[-1].f = instr(multi_sw_2_le);
		else
			ic[-1].f = instr(multi_sw_2_be);
	}
}
#endif


/*  Only for 32-bit virtual address translation so far.  */
#ifdef MODE32
/*
 *  Combine:  Multiple LW in a row using the same base register
 *
 *	lw	r?,???(rX)
 *	lw	r?,???(rX)
 *	lw	r?,???(rX)
 *	...
 */
void COMBINE(multi_lw)(struct cpu *cpu, struct mips_instr_call *ic,
	int low_addr)
{
	int n_back = (low_addr >> MIPS_INSTR_ALIGNMENT_SHIFT)
	    & (MIPS_IC_ENTRIES_PER_PAGE - 1);

	if (n_back < 3)
		return;

	/*  Convert a multi_lw_3 to a multi_lw_4:  */
	if ((ic[-3].f == instr(multi_lw_3_be) ||
	    ic[-3].f == instr(multi_lw_3_le)) &&
	    ic[-3].arg[1] == ic[0].arg[1] &&
	    ic[-1].arg[0] != ic[0].arg[1]) {
		if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
			ic[-3].f = instr(multi_lw_4_le);
		else
			ic[-3].f = instr(multi_lw_4_be);
	}

	/*  Convert a multi_lw_2 to a multi_lw_3:  */
	if ((ic[-2].f == instr(multi_lw_2_be) ||
	    ic[-2].f == instr(multi_lw_2_le)) &&
	    ic[-2].arg[1] == ic[0].arg[1] &&
	    ic[-1].arg[0] != ic[0].arg[1]) {
		if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
			ic[-2].f = instr(multi_lw_3_le);
		else
			ic[-2].f = instr(multi_lw_3_be);
	}

	/*  Note: Loads to the base register are not allowed in slot -1.  */
	if (ic[-1].f == ic[0].f &&
	    ic[-1].arg[1] == ic[0].arg[1] &&
	    ic[-1].arg[0] != ic[0].arg[1]) {
		if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
			ic[-1].f = instr(multi_lw_2_le);
		else
			ic[-1].f = instr(multi_lw_2_be);
	}
}
#endif


/*
 *  Combine:  NetBSD/pmax 3.0 R2000/R3000 physical cache invalidation loop
 *
 *  Instruction cache loop:
 *
 *  ic[-8]	mtc0	rV,status
 *     -7	nop
 *     -6	nop
 *     -5  s:	addiu	rX,rX,4
 *     -4	bne	rY,rX,s
 *     -3	sb	zr,-4(rX)
 *     -2	nop
 *     -1	nop
 *      0	mtc0	rT,status
 */
void COMBINE(netbsd_r3k_cache_inv)(struct cpu *cpu,
	struct mips_instr_call *ic, int low_addr)
{
	int n_back = (low_addr >> MIPS_INSTR_ALIGNMENT_SHIFT)
	    & (MIPS_IC_ENTRIES_PER_PAGE - 1);

	if (n_back < 8)
		return;

	if (ic[-8].f == instr(mtc0) && ic[-8].arg[1] == COP0_STATUS &&
	    ic[-7].f == instr(nop) && ic[-6].f == instr(nop) &&
	    ic[-5].f == instr(addiu) && ic[-5].arg[0] == ic[-5].arg[1] &&
	    (int32_t)ic[-5].arg[2] == 4 && ic[-4].f == instr(bne_samepage) &&
	    ic[-4].arg[0] == ic[-5].arg[0] && ic[-4].arg[0] != ic[-4].arg[1] &&
	    ic[-4].arg[2] == (size_t) &ic[-5] &&
	    ic[-3].arg[1] == ic[-5].arg[0] &&
	    ic[-2].f == instr(nop) && ic[-1].f == instr(nop)) {
		ic[-8].f = instr(netbsd_r3k_picache_do_inv);
	}
}


/*
 *  Combine: something ending with a nop.
 *
 *	NetBSD's strlen core.
 *	[Conditional] branch, followed by nop.
 *	NetBSD/pmax' idle loop (and possibly others as well).
 *	Linux/pmax' idle loop.
 */
void COMBINE(nop)(struct cpu *cpu, struct mips_instr_call *ic, int low_addr)
{
	int n_back = (low_addr >> MIPS_INSTR_ALIGNMENT_SHIFT)
	    & (MIPS_IC_ENTRIES_PER_PAGE - 1);

	if (n_back < 8)
		return;

#ifdef MODE32
	if (ic[-8].f == instr(set) &&
	    ic[-7].f == mips32_loadstore[4 + 1] &&
	    ic[-7].arg[0] == ic[-1].arg[0] &&
	    ic[-7].arg[0] == ic[-3].arg[0] &&
	    ic[-7].arg[0] == ic[-5].arg[0] &&
	    ic[-7].arg[0] == ic[-7].arg[1] &&
	    ic[-7].arg[0] == ic[-8].arg[0] &&
	    ic[-6].f == instr(nop) &&
	    ic[-5].arg[1] == (size_t) &cpu->cd.mips.gpr[MIPS_GPR_ZERO] &&
	    ic[-5].f == instr(bne_samepage_nop) &&
	    ic[-4].f == instr(nop) &&
	    ic[-3].f == mips32_loadstore[4 + 1] &&
	    ic[-2].f == instr(nop) &&
	    ic[-1].arg[1] == (size_t) &cpu->cd.mips.gpr[MIPS_GPR_ZERO] &&
	    ic[-1].arg[2] == (size_t) &ic[-8] &&
	    ic[-1].f == instr(beq_samepage)) {
		ic[-8].f = instr(linux_pmax_idle);
		return;
	}

	if (ic[-4].f == instr(set) &&
	    ic[-3].f == mips32_loadstore[4 + 1] &&
	    ic[-3].arg[0] == ic[-1].arg[0] &&
	    ic[-3].arg[1] == ic[-4].arg[0] &&
	    ic[-2].f == instr(nop) &&
	    ic[-1].arg[1] == (size_t) &cpu->cd.mips.gpr[MIPS_GPR_ZERO] &&
	    ic[-1].arg[2] == (size_t) &ic[-4] &&
	    ic[-1].f == instr(beq_samepage)) {
		ic[-4].f = instr(netbsd_pmax_idle);
		return;
	}

	if ((ic[-3].f == mips32_loadstore[1] ||
	    ic[-3].f == mips32_loadstore[16 + 1]) &&
	    ic[-3].arg[2] == 0 &&
	    ic[-3].arg[0] == ic[-1].arg[0] && ic[-3].arg[1] == ic[-2].arg[0] &&
	    ic[-2].arg[0] == ic[-2].arg[1] && ic[-2].arg[2] == 1 &&
	    ic[-2].f == instr(addiu) && ic[-1].arg[2] == (size_t) &ic[-3] &&
	    ic[-1].arg[1] == (size_t) &cpu->cd.mips.gpr[MIPS_GPR_ZERO] &&
	    ic[-1].f == instr(bne_samepage)) {
		ic[-3].f = instr(netbsd_strlen);
		return;
	}
#endif

	if (ic[-1].f == instr(bne_samepage)) {
		ic[-1].f = instr(bne_samepage_nop);
		return;
	}

	if (ic[-1].f == instr(beq_samepage)) {
		ic[-1].f = instr(beq_samepage_nop);
		return;
	}

	/*  TODO: other branches that are followed by nop should be here  */
}


/*
 *  Combine:
 *
 *	xor + andi + sll
 *	andi + sll
 */
void COMBINE(sll)(struct cpu *cpu, struct mips_instr_call *ic, int low_addr)
{
	int n_back = (low_addr >> MIPS_INSTR_ALIGNMENT_SHIFT)
	    & (MIPS_IC_ENTRIES_PER_PAGE - 1);

	if (n_back < 2)
		return;

	if (ic[-2].f == instr(xor) && ic[-1].f == instr(andi)) {
		ic[-2].f = instr(xor_andi_sll);
		return;
	}

	if (ic[-1].f == instr(andi)) {
		ic[-1].f = instr(andi_sll);
		return;
	}
}


/*
 *  lui + ori
 */
void COMBINE(ori)(struct cpu *cpu, struct mips_instr_call *ic, int low_addr)
{
	int n_back = (low_addr >> MIPS_INSTR_ALIGNMENT_SHIFT)
	    & (MIPS_IC_ENTRIES_PER_PAGE - 1);

	if (n_back < 1)
		return;

	if (ic[-1].f == instr(set)) {
		ic[-1].f = instr(lui_ori);
		return;
	}
}


/*
 *  addu + addu + addu
 */
void COMBINE(addu)(struct cpu *cpu, struct mips_instr_call *ic, int low_addr)
{
	int n_back = (low_addr >> MIPS_INSTR_ALIGNMENT_SHIFT)
	    & (MIPS_IC_ENTRIES_PER_PAGE - 1);

	if (n_back < 4)
		return;

	/*  Avoid "overlapping" instruction combinations:  */
	if (ic[-4].f == instr(multi_addu_3) ||
	    ic[-3].f == instr(multi_addu_3))
		return;

	if (ic[-2].f == instr(addu) && ic[-1].f == instr(addu)) {
		ic[-2].f = instr(multi_addu_3);
		return;
	}
}


/*
 *  Combine:
 *
 *	[Conditional] branch, followed by addiu.
 */
void COMBINE(addiu)(struct cpu *cpu, struct mips_instr_call *ic, int low_addr)
{
	int n_back = (low_addr >> MIPS_INSTR_ALIGNMENT_SHIFT)
	    & (MIPS_IC_ENTRIES_PER_PAGE - 1);

	if (n_back < 2)
		return;

	if (ic[-2].f == instr(addiu) &&
	    ic[-1].f == instr(bne_samepage)) {
		ic[-2].f = instr(addiu_bne_samepage_addiu);
		return;
	}

	if (ic[-1].f == instr(set)) {
		ic[-1].f = instr(lui_addiu);
		return;
	}

	if (ic[-1].f == instr(b_samepage)) {
		ic[-1].f = instr(b_samepage_addiu);
		return;
	}

	if (ic[-1].f == instr(beq_samepage)) {
		ic[-1].f = instr(beq_samepage_addiu);
		return;
	}

	if (ic[-1].f == instr(bne_samepage)) {
		ic[-1].f = instr(bne_samepage_addiu);
		return;
	}

	if (ic[-1].f == instr(jr_ra)) {
		ic[-1].f = instr(jr_ra_addiu);
		return;
	}

	/*  TODO: other branches that are followed by addiu should be here  */
}


/*
 *  Combine: [Conditional] branch, followed by daddiu.
 */
void COMBINE(b_daddiu)(struct cpu *cpu, struct mips_instr_call *ic,
	int low_addr)
{
	int n_back = (low_addr >> MIPS_INSTR_ALIGNMENT_SHIFT)
	    & (MIPS_IC_ENTRIES_PER_PAGE - 1);

	if (n_back < 1)
		return;

	if (ic[-1].f == instr(b_samepage)) {
		ic[-1].f = instr(b_samepage_daddiu);
	}

	/*  TODO: other branches that are followed by daddiu should be here  */
}


/*****************************************************************************/


/*
 *  mips_instr_to_be_translated():
 *
 *  Translate an instruction word into a mips_instr_call. ic is filled in with
 *  valid data for the translated instruction, or a "nothing" instruction if
 *  there was a translation failure. The newly translated instruction is then
 *  executed.
 */
X(to_be_translated)
{
	uint64_t addr, low_pc;
	uint32_t iword, imm;
	unsigned char *page;
	unsigned char ib[4];
	int main_opcode, rt, rs, rd, sa, s6, x64 = 0, s10;
	int in_crosspage_delayslot = 0;
	void (*samepage_function)(struct cpu *, struct mips_instr_call *);
	int store, signedness, size;

	/*  Figure out the (virtual) address of the instruction:  */
	low_pc = ((size_t)ic - (size_t)cpu->cd.mips.cur_ic_page)
	    / sizeof(struct mips_instr_call);

	/*  Special case for branch with delayslot on the next page:  */
	if (cpu->delay_slot == TO_BE_DELAYED && low_pc == 0) {
		/*  fatal("[ delay-slot translation across page "
		    "boundary ]\n");  */
		in_crosspage_delayslot = 1;
	}

	addr = cpu->pc & ~((MIPS_IC_ENTRIES_PER_PAGE-1)
	    << MIPS_INSTR_ALIGNMENT_SHIFT);
	addr += (low_pc << MIPS_INSTR_ALIGNMENT_SHIFT);
	cpu->pc = (MODE_int_t)addr;
	addr &= ~((1 << MIPS_INSTR_ALIGNMENT_SHIFT) - 1);

	/*  Read the instruction word from memory:  */
#ifdef MODE32
	page = cpu->cd.mips.host_load[(uint32_t)addr >> 12];
#else
	{
		const uint32_t mask1 = (1 << DYNTRANS_L1N) - 1;
		const uint32_t mask2 = (1 << DYNTRANS_L2N) - 1;
		const uint32_t mask3 = (1 << DYNTRANS_L3N) - 1;
		uint32_t x1 = (addr >> (64-DYNTRANS_L1N)) & mask1;
		uint32_t x2 = (addr >> (64-DYNTRANS_L1N-DYNTRANS_L2N)) & mask2;
		uint32_t x3 = (addr >> (64-DYNTRANS_L1N-DYNTRANS_L2N-
		    DYNTRANS_L3N)) & mask3;
		struct DYNTRANS_L2_64_TABLE *l2 = cpu->cd.mips.l1_64[x1];
		struct DYNTRANS_L3_64_TABLE *l3 = l2->l3[x2];
		page = l3->host_load[x3];
	}
#endif

	if (page != NULL) {
		/*  fatal("TRANSLATION HIT!\n");  */
		memcpy(ib, page + (addr & 0xffc), sizeof(ib));
	} else {
		/*  fatal("TRANSLATION MISS!\n");  */
		if (!cpu->memory_rw(cpu, cpu->mem, addr, ib,
		    sizeof(ib), MEM_READ, CACHE_INSTRUCTION)) {
			fatal("to_be_translated(): read failed: TODO\n");
			goto bad;
		}
	}

	{
		uint32_t *p = (uint32_t *) ib;
		iword = *p;
	}

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		iword = LE32_TO_HOST(iword);
	else
		iword = BE32_TO_HOST(iword);


#define DYNTRANS_TO_BE_TRANSLATED_HEAD
#include "cpu_dyntrans.cc"
#undef  DYNTRANS_TO_BE_TRANSLATED_HEAD


	/*
	 *  Translate the instruction:
 	 *
	 *  NOTE: _NEVER_ allow writes to the zero register; all instructions
	 *  that use the zero register as their destination should be treated
	 *  as NOPs, except those that access memory (they should use the
	 *  scratch register instead).
	 */

	main_opcode = iword >> 26;
	rs = (iword >> 21) & 31;
	rt = (iword >> 16) & 31;
	rd = (iword >> 11) & 31;
	sa = (iword >>  6) & 31;
	imm = (int16_t)iword;
	s6 = iword & 63;
	s10 = (rs << 5) | sa;

	switch (main_opcode) {

	case HI6_SPECIAL:
		switch (s6) {

		case SPECIAL_SLL:
		case SPECIAL_SLLV:
		case SPECIAL_SRL:
		case SPECIAL_SRLV:
		case SPECIAL_SRA:
		case SPECIAL_SRAV:
		case SPECIAL_DSRL:
		case SPECIAL_DSRLV:
		case SPECIAL_DSRL32:
		case SPECIAL_DSLL:
		case SPECIAL_DSLLV:
		case SPECIAL_DSLL32:
		case SPECIAL_DSRA:
		case SPECIAL_DSRAV:
		case SPECIAL_DSRA32:
			switch (s6) {
			case SPECIAL_SLL:  ic->f = instr(sll); break;
			case SPECIAL_SLLV: ic->f = instr(sllv); sa = -1; break;
			case SPECIAL_SRL:  ic->f = instr(srl); break;
			case SPECIAL_SRLV: ic->f = instr(srlv); sa = -1; break;
			case SPECIAL_SRA:  ic->f = instr(sra); break;
			case SPECIAL_SRAV: ic->f = instr(srav); sa = -1; break;
			case SPECIAL_DSRL: ic->f = instr(dsrl); x64=1; break;
			case SPECIAL_DSRLV:ic->f = instr(dsrlv);
					   x64 = 1; sa = -1; break;
			case SPECIAL_DSRL32:ic->f= instr(dsrl); x64=1;
					   sa += 32; break;
			case SPECIAL_DSLL: ic->f = instr(dsll); x64=1; break;
			case SPECIAL_DSLLV:ic->f = instr(dsllv);
					   x64 = 1; sa = -1; break;
			case SPECIAL_DSLL32:ic->f= instr(dsll); x64=1;
					   sa += 32; break;
			case SPECIAL_DSRA: ic->f = instr(dsra); x64=1; break;
			case SPECIAL_DSRAV:ic->f = instr(dsrav);
					   x64 = 1; sa = -1; break;
			case SPECIAL_DSRA32:ic->f = instr(dsra); x64=1;
					   sa += 32; break;
			}

			ic->arg[0] = (size_t)&cpu->cd.mips.gpr[rt];
			if (sa >= 0)
				ic->arg[1] = sa;
			else
				ic->arg[1] = (size_t)&cpu->cd.mips.gpr[rs];
			ic->arg[2] = (size_t)&cpu->cd.mips.gpr[rd];

			/*  Special checks for MIPS32/64 revision 2 opcodes,
			    such as rotation instructions:  */
			if (sa >= 0 && rs != 0x00) {
				if (cpu->cd.mips.cpu_type.isa_level < 32 ||
				    cpu->cd.mips.cpu_type.isa_revision < 2) {
					static int warning_rotate = 0;
					if (!warning_rotate &&
					    !cpu->translation_readahead) {
						fatal("[ WARNING! MIPS32/64 "
						    "revision 2 rotate opcode"
						    " used, but the %s process"
						    "or does not implement "
						    "such instructions. Only "
						    "printing this "
						    "warning once. ]\n",
						    cpu->cd.mips.cpu_type.name);
						warning_rotate = 1;
					}
					ic->f = instr(reserved);
					break;
				}
				switch (rs) {
				case 0x01:
					switch (s6) {
					case SPECIAL_SRL:	/*  ror  */
						ic->f = instr(ror);
						break;
					default:goto bad;
					}
					break;
				default:goto bad;
				}
			}
			if (sa < 0 && (s10 & 0x1f) != 0) {
				switch (s10 & 0x1f) {
				/*  TODO: [d]rorv, etc.  */
				default:goto bad;
				}
			}

			if (rd == MIPS_GPR_ZERO)
				ic->f = instr(nop);
			if (ic->f == instr(sll))
				cpu->cd.mips.combination_check = COMBINE(sll);
			if (ic->f == instr(nop))
				cpu->cd.mips.combination_check = COMBINE(nop);
			break;

		case SPECIAL_ADD:
		case SPECIAL_ADDU:
		case SPECIAL_SUB:
		case SPECIAL_SUBU:
		case SPECIAL_DADD:
		case SPECIAL_DADDU:
		case SPECIAL_DSUB:
		case SPECIAL_DSUBU:
		case SPECIAL_SLT:
		case SPECIAL_SLTU:
		case SPECIAL_AND:
		case SPECIAL_OR:
		case SPECIAL_XOR:
		case SPECIAL_NOR:
		case SPECIAL_MOVN:
		case SPECIAL_MOVZ:
		case SPECIAL_MFHI:
		case SPECIAL_MFLO:
		case SPECIAL_MTHI:
		case SPECIAL_MTLO:
		case SPECIAL_DIV:
		case SPECIAL_DIVU:
		case SPECIAL_DDIV:
		case SPECIAL_DDIVU:
		case SPECIAL_MULT:
		case SPECIAL_MULTU:
		case SPECIAL_DMULT:
		case SPECIAL_DMULTU:
		case SPECIAL_TGE:
		case SPECIAL_TGEU:
		case SPECIAL_TLT:
		case SPECIAL_TLTU:
		case SPECIAL_TEQ:
		case SPECIAL_TNE:
			switch (s6) {
			case SPECIAL_ADD:   ic->f = instr(add); break;
			case SPECIAL_ADDU:  ic->f = instr(addu); break;
			case SPECIAL_SUB:   ic->f = instr(sub); break;
			case SPECIAL_SUBU:  ic->f = instr(subu); break;
			case SPECIAL_DADD:  ic->f = instr(dadd); x64=1; break;
			case SPECIAL_DADDU: ic->f = instr(daddu); x64=1; break;
			case SPECIAL_DSUB:  ic->f = instr(dsub); x64=1; break;
			case SPECIAL_DSUBU: ic->f = instr(dsubu); x64=1; break;
			case SPECIAL_SLT:   ic->f = instr(slt); break;
			case SPECIAL_SLTU:  ic->f = instr(sltu); break;
			case SPECIAL_AND:   ic->f = instr(and); break;
			case SPECIAL_OR:    ic->f = instr(or); break;
			case SPECIAL_XOR:   ic->f = instr(xor); break;
			case SPECIAL_NOR:   ic->f = instr(nor); break;
			case SPECIAL_MFHI:  ic->f = instr(mov); break;
			case SPECIAL_MFLO:  ic->f = instr(mov); break;
			case SPECIAL_MTHI:  ic->f = instr(mov); break;
			case SPECIAL_MTLO:  ic->f = instr(mov); break;
			case SPECIAL_DIV:   ic->f = instr(div); break;
			case SPECIAL_DIVU:  ic->f = instr(divu); break;
			case SPECIAL_DDIV:  ic->f = instr(ddiv); x64=1; break;
			case SPECIAL_DDIVU: ic->f = instr(ddivu); x64=1; break;
			case SPECIAL_MULT : ic->f = instr(mult); break;
			case SPECIAL_MULTU: ic->f = instr(multu); break;
			case SPECIAL_DMULT: ic->f = instr(dmult); x64=1; break;
			case SPECIAL_DMULTU:ic->f = instr(dmultu); x64=1; break;
			case SPECIAL_TGE:   ic->f = instr(tge); break;
			case SPECIAL_TGEU:  ic->f = instr(tgeu); break;
			case SPECIAL_TLT:   ic->f = instr(tlt); break;
			case SPECIAL_TLTU:  ic->f = instr(tltu); break;
			case SPECIAL_TEQ:   ic->f = instr(teq); break;
			case SPECIAL_TNE:   ic->f = instr(tne); break;
			case SPECIAL_MOVN:  ic->f = instr(movn); break;
			case SPECIAL_MOVZ:  ic->f = instr(movz); break;
			}

			ic->arg[0] = (size_t)&cpu->cd.mips.gpr[rs];
			ic->arg[1] = (size_t)&cpu->cd.mips.gpr[rt];
			ic->arg[2] = (size_t)&cpu->cd.mips.gpr[rd];

			switch (s6) {
			case SPECIAL_MFHI:
				ic->arg[0] = (size_t)&cpu->cd.mips.hi;
				break;
			case SPECIAL_MFLO:
				ic->arg[0] = (size_t)&cpu->cd.mips.lo;
				break;
			case SPECIAL_MTHI:
				ic->arg[2] = (size_t)&cpu->cd.mips.hi;
				break;
			case SPECIAL_MTLO:
				ic->arg[2] = (size_t)&cpu->cd.mips.lo;
				break;
			}
			/*  Special cases for rd:  */
			switch (s6) {
			case SPECIAL_MTHI:
			case SPECIAL_MTLO:
			case SPECIAL_DIV:
			case SPECIAL_DIVU:
			case SPECIAL_DDIV:
			case SPECIAL_DDIVU:
			case SPECIAL_MULT:
			case SPECIAL_MULTU:
			case SPECIAL_DMULT:
			case SPECIAL_DMULTU:
				if (s6 == SPECIAL_MULT && rd != MIPS_GPR_ZERO) {
					if (cpu->cd.mips.cpu_type.rev ==
					    MIPS_R5900) {
						ic->f = instr(mult_r5900);
						break;
					}
					break;
				}
				if (s6 == SPECIAL_MULTU && rd!=MIPS_GPR_ZERO) {
					if (cpu->cd.mips.cpu_type.rev ==
					    MIPS_R5900) {
						ic->f = instr(multu_r5900);
						break;
					}
				}
				if (rd != MIPS_GPR_ZERO) {
					if (!cpu->translation_readahead)
						fatal("TODO: rd NON-zero\n");
					goto bad;
				}
				/*  These instructions don't use rd.  */
				break;
			case SPECIAL_TGE:
			case SPECIAL_TGEU:
			case SPECIAL_TLT:
			case SPECIAL_TLTU:
			case SPECIAL_TEQ:
			case SPECIAL_TNE:
				/*  In these instructions, rd is a 'code',
				    only read by trap handling software.  */
				break;
			default:if (rd == MIPS_GPR_ZERO)
					ic->f = instr(nop);
			}

			if (ic->f == instr(addu))
				cpu->cd.mips.combination_check = COMBINE(addu);

			break;

		case SPECIAL_JR:
		case SPECIAL_JALR:
			ic->arg[0] = (size_t)&cpu->cd.mips.gpr[rs];
			ic->arg[1] = (size_t)&cpu->cd.mips.gpr[rd];
			if (s6 == SPECIAL_JALR && rd == MIPS_GPR_ZERO)
				s6 = SPECIAL_JR;
			ic->arg[2] = (addr & 0xffc) + 8;
			switch (s6) {
			case SPECIAL_JR:
				if (rs == MIPS_GPR_RA) {
					if (cpu->machine->show_trace_tree)
						ic->f = instr(jr_ra_trace);
					else
						ic->f = instr(jr_ra);
				} else {
					ic->f = instr(jr);
				}
				if (cpu->translation_readahead > 2)
					cpu->translation_readahead = 2;
				break;
			case SPECIAL_JALR:
				if (cpu->machine->show_trace_tree)
					ic->f = instr(jalr_trace);
				else
					ic->f = instr(jalr);
				break;
			}
			if (cpu->delay_slot) {
				if (!cpu->translation_readahead)
					fatal("TODO: branch in delay "
					    "slot? (1)\n");
				goto bad;
			}
			break;

		case SPECIAL_SYSCALL:
			if (((iword >> 6) & 0xfffff) == 0x30378) {
				/*  "Magic trap" for PROM emulation:  */
				ic->f = instr(promemul);
			} else {
				ic->f = instr(syscall);
			}
			break;

		case SPECIAL_BREAK:
			if (((iword >> 6) & 0xfffff) == 0x30378) {
				/*  "Magic trap" for REBOOT:  */
				ic->f = instr(reboot);
			} else {
				ic->f = instr(break);
			}
			break;

		case SPECIAL_SYNC:
			ic->f = instr(nop);
			break;

		default:goto bad;
		}
		break;

	case HI6_BEQ:
	case HI6_BNE:
	case HI6_BEQL:
	case HI6_BNEL:
	case HI6_BLEZ:
	case HI6_BLEZL:
	case HI6_BGTZ:
	case HI6_BGTZL:
		samepage_function = NULL;  /*  get rid of a compiler warning  */
		switch (main_opcode) {
		case HI6_BEQ:
			ic->f = instr(beq);
			samepage_function = instr(beq_samepage);
			/*  Special case: comparing a register with itself:  */
			if (rs == rt) {
				ic->f = instr(b);
				samepage_function = instr(b_samepage);
			}
			break;
		case HI6_BNE:
			ic->f = instr(bne);
			samepage_function = instr(bne_samepage);
			break;
		case HI6_BEQL:
			ic->f = instr(beql);
			samepage_function = instr(beql_samepage);
			/*  Special case: comparing a register with itself:  */
			if (rs == rt) {
				ic->f = instr(b);
				samepage_function = instr(b_samepage);
			}
			break;
		case HI6_BNEL:
			ic->f = instr(bnel);
			samepage_function = instr(bnel_samepage);
			break;
		case HI6_BLEZ:
			ic->f = instr(blez);
			samepage_function = instr(blez_samepage);
			break;
		case HI6_BLEZL:
			ic->f = instr(blezl);
			samepage_function = instr(blezl_samepage);
			break;
		case HI6_BGTZ:
			ic->f = instr(bgtz);
			samepage_function = instr(bgtz_samepage);
			break;
		case HI6_BGTZL:
			ic->f = instr(bgtzl);
			samepage_function = instr(bgtzl_samepage);
			break;
		}
		ic->arg[0] = (size_t)&cpu->cd.mips.gpr[rs];
		ic->arg[1] = (size_t)&cpu->cd.mips.gpr[rt];
		ic->arg[2] = (int32_t) ( (imm << MIPS_INSTR_ALIGNMENT_SHIFT)
		    + (addr & 0xffc) + 4 );
		/*  Is the offset from the start of the current page still
		    within the same page? Then use the samepage_function:  */
		if ((uint32_t)ic->arg[2] < ((MIPS_IC_ENTRIES_PER_PAGE - 1)
		    << MIPS_INSTR_ALIGNMENT_SHIFT) && (addr & 0xffc) < 0xffc) {
			ic->arg[2] = (size_t) (cpu->cd.mips.cur_ic_page +
			    ((ic->arg[2] >> MIPS_INSTR_ALIGNMENT_SHIFT)
			    & (MIPS_IC_ENTRIES_PER_PAGE - 1)));
			ic->f = samepage_function;
		}
		if (cpu->delay_slot) {
			if (!cpu->translation_readahead)
				fatal("TODO: branch in delay slot? (2)\n");
			goto bad;
		}
		break;

	case HI6_ADDI:
	case HI6_ADDIU:
	case HI6_SLTI:
	case HI6_SLTIU:
	case HI6_DADDI:
	case HI6_DADDIU:
	case HI6_ANDI:
	case HI6_ORI:
	case HI6_XORI:
		ic->arg[0] = (size_t)&cpu->cd.mips.gpr[rs];
		ic->arg[1] = (size_t)&cpu->cd.mips.gpr[rt];
		if (main_opcode == HI6_ADDI ||
		    main_opcode == HI6_ADDIU ||
		    main_opcode == HI6_SLTI ||
		    main_opcode == HI6_SLTIU ||
		    main_opcode == HI6_DADDI ||
		    main_opcode == HI6_DADDIU)
			ic->arg[2] = (int16_t)iword;
		else
			ic->arg[2] = (uint16_t)iword;

		switch (main_opcode) {
		case HI6_ADDI:    ic->f = instr(addi); break;
		case HI6_ADDIU:   ic->f = instr(addiu); break;
		case HI6_SLTI:    ic->f = instr(slti); break;
		case HI6_SLTIU:   ic->f = instr(sltiu); break;
		case HI6_DADDI:   ic->f = instr(daddi); x64 = 1; break;
		case HI6_DADDIU:  ic->f = instr(daddiu); x64 = 1; break;
		case HI6_ANDI:    ic->f = instr(andi); break;
		case HI6_ORI:     ic->f = instr(ori); break;
		case HI6_XORI:    ic->f = instr(xori); break;
		}

		if (ic->arg[2] == 0) {
			if ((cpu->is_32bit && ic->f == instr(addiu)) ||
			    (!cpu->is_32bit && ic->f == instr(daddiu))) {
				ic->f = instr(mov);
				ic->arg[2] = ic->arg[1];
			}
		}

		if (rt == MIPS_GPR_ZERO)
			ic->f = instr(nop);

		if (ic->f == instr(ori))
			cpu->cd.mips.combination_check = COMBINE(ori);
		if (ic->f == instr(addiu))
			cpu->cd.mips.combination_check = COMBINE(addiu);
		if (ic->f == instr(daddiu))
			cpu->cd.mips.combination_check = COMBINE(b_daddiu);
		break;

	case HI6_LUI:
		ic->f = instr(set);
		ic->arg[0] = (size_t)&cpu->cd.mips.gpr[rt];
		ic->arg[1] = (int32_t) (imm << 16);
		/*  NOTE: Don't use arg[2] here. It can be used with
		    instruction combinations, to do lui + addiu, etc.  */
		if (rt == MIPS_GPR_ZERO)
			ic->f = instr(nop);
		break;

	case HI6_J:
	case HI6_JAL:
		switch (main_opcode) {
		case HI6_J:
			ic->f = instr(j);
			if (cpu->translation_readahead > 2)
				cpu->translation_readahead = 2;
			break;
		case HI6_JAL:
			if (cpu->machine->show_trace_tree)
				ic->f = instr(jal_trace);
			else
				ic->f = instr(jal);
			break;
		}
		ic->arg[0] = (iword & 0x03ffffff) << 2;
		ic->arg[1] = (addr & 0xffc) + 8;
		if (cpu->delay_slot) {
			if (!cpu->translation_readahead)
				fatal("TODO: branch in delay slot (=%i)? (3);"
				    " addr=%016"PRIx64" iword=%08"PRIx32"\n",
				    cpu->delay_slot, (uint64_t)addr, iword);
			goto bad;
		}
		break;

	case HI6_COP0:
		/*  TODO: Is checking bit 25 enough, or perhaps all bits
		    25..21 must be checked?  */
		if ((iword >> 25) & 1) {
			ic->arg[2] = addr & 0xffc;
			switch (iword & 0xff) {
			case COP0_TLBR:
				ic->f = instr(tlbr);
				break;
			case COP0_TLBWI:
			case COP0_TLBWR:
				ic->f = instr(tlbw);
				ic->arg[0] = (iword & 0xff) == COP0_TLBWR;
				break;
			case COP0_TLBP:
				ic->f = instr(tlbp);
				break;
			case COP0_RFE:
				ic->f = instr(rfe);
				break;
			case COP0_ERET:
				ic->f = instr(eret);
				break;
			case COP0_DERET:
				ic->f = instr(deret);
				break;
			case COP0_WAIT:
				ic->f = instr(wait);
				if (cpu->cd.mips.cpu_type.rev != MIPS_RM5200 &&
				    cpu->cd.mips.cpu_type.isa_level < 32) {
					static int warned = 0;
					ic->f = instr(reserved);
					if (!warned &&
					    !cpu->translation_readahead) {
						fatal("{ WARNING: Attempt to "
						    "execute the WAIT instruct"
					            "ion, but the emulated CPU "
						    "is neither RM52xx, nor "
						    "MIPS32/64! }\n");
						warned = 1;
					}
				}
				break;
			case COP0_STANDBY:
				/*  NOTE: Reusing the 'wait' instruction:  */
				ic->f = instr(wait);
				if (cpu->cd.mips.cpu_type.rev != MIPS_R4100) {
					static int warned = 0;
					ic->f = instr(reserved);
					if (!warned &&
					    !cpu->translation_readahead) {
						fatal("{ WARNING: Attempt to "
						    "execute a R41xx instruct"
					            "ion, but the emulated CPU "
						    "doesn't support it! }\n");
						warned = 1;
					}
				}
				break;
			case COP0_HIBERNATE:
				/*  TODO  */
				goto bad;
			case COP0_SUSPEND:
				/*  Used by NetBSD on HPCmips (VR41xx) to
				    halt the machine.  */
				ic->f = instr(reboot);
				break;
			case COP0_EI:
				if (cpu->cd.mips.cpu_type.rev == MIPS_R5900) {
					ic->f = instr(ei_r5900);
				} else
					goto bad;
				break;
			case COP0_DI:
				if (cpu->cd.mips.cpu_type.rev == MIPS_R5900) {
					ic->f = instr(di_r5900);
				} else
					goto bad;
				break;
			default:if (!cpu->translation_readahead)
					fatal("UNIMPLEMENTED cop0 (func "
					    "0x%02x)\n", iword & 0xff);
				goto bad;
			}
			break;
		}

		/*  rs contains the coprocessor opcode!  */
		switch (rs) {
		case COPz_CFCz:
			ic->arg[0] = (size_t)&cpu->cd.mips.gpr[rt];
			ic->arg[1] = rd + ((iword & 7) << 5);
			ic->arg[2] = addr & 0xffc;
			ic->f = instr(cfc0);
			if (rt == MIPS_GPR_ZERO)
				ic->f = instr(nop);
			break;
		case COPz_MFCz:
		case COPz_DMFCz:
			ic->arg[0] = (size_t)&cpu->cd.mips.gpr[rt];
			ic->arg[1] = rd + ((iword & 7) << 5);
			ic->arg[2] = addr & 0xffc;
			ic->f = rs == COPz_MFCz? instr(mfc0) : instr(dmfc0);
			if (rs == COPz_MFCz && (iword & 7) == 0 &&
			    rd != COP0_COUNT)
				ic->f = instr(mfc0_select0);
			if (rs == COPz_DMFCz && (iword & 7) == 0 &&
			    rd != COP0_COUNT)
				ic->f = instr(dmfc0_select0);
			if (rt == MIPS_GPR_ZERO)
				ic->f = instr(nop);
			break;
		case COPz_MTCz:
		case COPz_DMTCz:
			ic->arg[0] = (size_t)&cpu->cd.mips.gpr[rt];
			ic->arg[1] = rd + ((iword & 7) << 5);
			ic->arg[2] = addr & 0xffc;
			ic->f = rs == COPz_MTCz? instr(mtc0) : instr(dmtc0);

			if (cpu->cd.mips.cpu_type.exc_model == EXC3K &&
			    rs == COPz_MTCz && rd == COP0_STATUS)
				cpu->cd.mips.combination_check =
				    COMBINE(netbsd_r3k_cache_inv);

			break;
		case COPz_MFMCz:
			if ((iword & 0xffdf) == 0x6000) {
				/*  MIPS32/64 rev 2 "ei" or "di":   */
				if (cpu->cd.mips.cpu_type.isa_level < 32 ||
				    cpu->cd.mips.cpu_type.isa_revision < 2) {
					static int warning_ei_di = 0;
					if (!warning_ei_di &&
					    !cpu->translation_readahead) {
						fatal("[ WARNING! MIPS32/64 "
						    "revision 2 di or ei opcode"
						    " used, but the %s process"
						    "or does not implement "
						    "such instructions. Only "
						    "printing this "
						    "warning once. ]\n",
						    cpu->cd.mips.cpu_type.name);
						warning_ei_di = 1;
					}
					ic->f = instr(reserved);
					break;
				}
				ic->f = instr(ei_or_di);
				ic->arg[0] = (size_t)&cpu->cd.mips.gpr[rt];
				if (rt == MIPS_GPR_ZERO)
					ic->arg[0] =
					    (size_t)&cpu->cd.mips.scratch;
				ic->arg[1] = iword & 0x20;
			} else {
				if (!cpu->translation_readahead)
					fatal("Unimplemented COP0_MFMCz\n");
				goto bad;
			}
			break;
		case COPz_BCzc:
			if (iword == 0x4100ffff) {
				/*  R2020 DECstation write-loop thingy.  */
				ic->f = instr(nop);
			} else {
				if (!cpu->translation_readahead)
					fatal("Unimplemented COP0_BCzc\n");
				goto bad;
			}
			break;
		
		default:if (!cpu->translation_readahead)
				fatal("UNIMPLEMENTED cop0 (rs = %i)\n", rs);
			goto bad;
		}
		break;

	case HI6_COP1:
		/*  Always cause a coprocessor unusable exception if
		    there is no floating point coprocessor:  */
		if (cpu->cd.mips.cpu_type.flags & NOFPU ||
		    cpu->cd.mips.coproc[1] == NULL) {
			ic->f = instr(cpu);
			ic->arg[0] = 1;
			break;
		}

		/*  Bits 25..21 are floating point main opcode:  */
		switch (rs) {

		case COPz_BCzc:
			/*  Conditional branch:  */
			/*  TODO: Reimplement this in a faster way.  */
			ic->f = instr(cop1_bc);
			ic->arg[0] = (iword >> 18) & 7;	/*  cc  */
			ic->arg[1] = (iword >> 16) & 3;	/*  nd, tf bits  */
			ic->arg[2] = (int32_t) ((imm <<
			    MIPS_INSTR_ALIGNMENT_SHIFT) + (addr & 0xffc) + 4);
			if (cpu->delay_slot) {
				if (!cpu->translation_readahead)
					fatal("TODO: branch in delay slot 4\n");
				goto bad;
			}
			if (cpu->cd.mips.cpu_type.isa_level <= 3 &&
			    ic->arg[0] != 0) {
				if (!cpu->translation_readahead)
					fatal("Attempt to execute a non-cc-0 "
					    "BC* instruction on an isa level "
					    "%i cpu. TODO: How should this be "
					    "handled?\n",
					    cpu->cd.mips.cpu_type.isa_level);
				goto bad;
			}

			break;

		case COPz_DMFCz:
		case COPz_DMTCz:
			x64 = 1;
			/*  FALL-THROUGH  */
		case COP1_FMT_S:
		case COP1_FMT_D:
		case COP1_FMT_W:
		case COP1_FMT_L:
		case COP1_FMT_PS:
		case COPz_CFCz:
		case COPz_CTCz:
		case COPz_MFCz:
		case COPz_MTCz:
			/*  Fallback to slow pre-dyntrans code, for now.  */
			/*  TODO: Fix/optimize/rewrite.  */
			ic->f = instr(cop1_slow);
			ic->arg[0] = (uint32_t)iword & ((1 << 26) - 1);
			break;

		default:if (!cpu->translation_readahead)
			    fatal("COP1 floating point opcode = 0x%02x\n", rs);
			goto bad;
		}
		break;

	case HI6_COP2:
		/*  Always cause a coprocessor unusable exception if
		    there is no coprocessor 2:  */
		if (cpu->cd.mips.coproc[2] == NULL) {
			ic->f = instr(cpu);
			ic->arg[0] = 2;
			break;
		}
		if (!cpu->translation_readahead)
			fatal("COP2 functionality not yet implemented\n");
		goto bad;
		break;

	case HI6_COP3:
		/*  Always cause a coprocessor unusable exception if
		    there is no coprocessor 3:  */
		if (cpu->cd.mips.coproc[3] == NULL) {
			ic->f = instr(cpu);
			ic->arg[0] = 3;
			break;
		}

		if (iword == 0x4d00ffff) {
			/*  R2020 writeback thing, used by e.g. NetBSD/pmax
			    on MIPSMATE.  */
			ic->f = instr(nop);
		} else {
			if (!cpu->translation_readahead)
				fatal("COP3 iword=0x%08x\n", iword);
			goto bad;
		}
		break;

	case HI6_SPECIAL2:
		if (cpu->cd.mips.cpu_type.rev == MIPS_R5900) {
			/*  R5900, TX79/C790, have MMI instead of SPECIAL2:  */
			int mmi_subopcode = (iword >> 6) & 0x1f;

			switch (s6) {

			case MMI_MADD:
				ic->arg[0] = (size_t)&cpu->cd.mips.gpr[rs];
				ic->arg[1] = (size_t)&cpu->cd.mips.gpr[rt];
				ic->arg[2] = (size_t)&cpu->cd.mips.gpr[rd];
				if (rd == MIPS_GPR_ZERO)
					ic->f = instr(madd);
				else
					ic->f = instr(madd_rd);
				break;

			case MMI_MADDU:
				ic->arg[0] = (size_t)&cpu->cd.mips.gpr[rs];
				ic->arg[1] = (size_t)&cpu->cd.mips.gpr[rt];
				ic->arg[2] = (size_t)&cpu->cd.mips.gpr[rd];
				if (rd == MIPS_GPR_ZERO)
					ic->f = instr(maddu);
				else
					ic->f = instr(maddu_rd);
				break;

			case MMI_MMI0:
				switch (mmi_subopcode) {

				case MMI0_PEXTLW:
					ic->arg[0] = rs;
					ic->arg[1] = rt;
					ic->arg[2] = rd;
					if (rd == MIPS_GPR_ZERO)
						ic->f = instr(nop);
					else
						ic->f = instr(pextlw);
					break;

				default:goto bad;
				}
				break;

			case MMI_MMI3:
				switch (mmi_subopcode) {

				case MMI3_POR:
					ic->arg[0] = rs;
					ic->arg[1] = rt;
					ic->arg[2] = rd;
					if (rd == MIPS_GPR_ZERO)
						ic->f = instr(nop);
					else
						ic->f = instr(por);
					break;

				default:goto bad;
				}
				break;

			default:goto bad;
			}
			break;
		}

		/*  TODO: is this correct? Or are there other non-MIPS32/64
		    MIPS processors that have support for SPECIAL2 opcodes?  */
		if (cpu->cd.mips.cpu_type.isa_level < 32) {
			ic->f = instr(reserved);
			break;
		}

		/*  SPECIAL2:  */
		switch (s6) {

		case SPECIAL2_MADD:
		case SPECIAL2_MADDU:
		case SPECIAL2_MSUB:
		case SPECIAL2_MSUBU:
			ic->arg[0] = (size_t)&cpu->cd.mips.gpr[rs];
			ic->arg[1] = (size_t)&cpu->cd.mips.gpr[rt];
			switch (s6) {
			case SPECIAL2_MADD: ic->f = instr(madd); break;
			case SPECIAL2_MADDU:ic->f = instr(maddu); break;
			case SPECIAL2_MSUB: ic->f = instr(msub); break;
			case SPECIAL2_MSUBU:ic->f = instr(msubu); break;
			}
			break;

		case SPECIAL2_MUL:
			ic->f = instr(mul);
			ic->arg[0] = (size_t)&cpu->cd.mips.gpr[rs];
			ic->arg[1] = (size_t)&cpu->cd.mips.gpr[rt];
			ic->arg[2] = (size_t)&cpu->cd.mips.gpr[rd];
			if (rd == MIPS_GPR_ZERO)
				ic->f = instr(nop);
			break;

		case SPECIAL2_CLZ:
		case SPECIAL2_CLO:
		case SPECIAL2_DCLZ:
		case SPECIAL2_DCLO:
			switch (s6) {
			case SPECIAL2_CLZ:  ic->f = instr(clz); break;
			case SPECIAL2_CLO:  ic->f = instr(clo); break;
			case SPECIAL2_DCLZ: ic->f = instr(dclz); break;
			case SPECIAL2_DCLO: ic->f = instr(dclo); break;
			}
			ic->arg[0] = (size_t)&cpu->cd.mips.gpr[rs];
			ic->arg[1] = (size_t)&cpu->cd.mips.gpr[rd];
			if (rd == MIPS_GPR_ZERO)
				ic->f = instr(nop);
			break;

		default:goto bad;
		}
		break;

	case HI6_REGIMM:
		switch (rt) {
		case REGIMM_BGEZ:
		case REGIMM_BGEZL:
		case REGIMM_BLTZ:
		case REGIMM_BLTZL:
		case REGIMM_BGEZAL:
		case REGIMM_BGEZALL:
		case REGIMM_BLTZAL:
		case REGIMM_BLTZALL:
			samepage_function = NULL;
			switch (rt) {
			case REGIMM_BGEZ:
				ic->f = instr(bgez);
				samepage_function = instr(bgez_samepage);
				break;
			case REGIMM_BGEZL:
				ic->f = instr(bgezl);
				samepage_function = instr(bgezl_samepage);
				break;
			case REGIMM_BLTZ:
				ic->f = instr(bltz);
				samepage_function = instr(bltz_samepage);
				break;
			case REGIMM_BLTZL:
				ic->f = instr(bltzl);
				samepage_function = instr(bltzl_samepage);
				break;
			case REGIMM_BGEZAL:
				ic->f = instr(bgezal);
				samepage_function = instr(bgezal_samepage);
				break;
			case REGIMM_BGEZALL:
				ic->f = instr(bgezall);
				samepage_function = instr(bgezall_samepage);
				break;
			case REGIMM_BLTZAL:
				ic->f = instr(bltzal);
				samepage_function = instr(bltzal_samepage);
				break;
			case REGIMM_BLTZALL:
				ic->f = instr(bltzall);
				samepage_function = instr(bltzall_samepage);
				break;
			}
			ic->arg[0] = (size_t)&cpu->cd.mips.gpr[rs];
			ic->arg[2] = (imm << MIPS_INSTR_ALIGNMENT_SHIFT)
			    + (addr & 0xffc) + 4;
			/*  Is the offset from the start of the current page
			    still within the same page? Then use the
			    samepage_function:  */
			if ((uint32_t)ic->arg[2] < ((MIPS_IC_ENTRIES_PER_PAGE-1)
			    << MIPS_INSTR_ALIGNMENT_SHIFT) && (addr & 0xffc)
			    < 0xffc) {
				ic->arg[2] = (size_t) (cpu->cd.mips.cur_ic_page+
				    ((ic->arg[2] >> MIPS_INSTR_ALIGNMENT_SHIFT)
				    & (MIPS_IC_ENTRIES_PER_PAGE - 1)));
				ic->f = samepage_function;
			}
			if (cpu->delay_slot) {
				if (!cpu->translation_readahead)
					fatal("TODO: branch in delay slot:5\n");
				goto bad;
			}
			break;

		default:if (!cpu->translation_readahead)
				fatal("UNIMPLEMENTED regimm rt=%i\n", rt);
			goto bad;
		}
		break;

	case HI6_LB:
	case HI6_LBU:
	case HI6_SB:
	case HI6_LH:
	case HI6_LHU:
	case HI6_SH:
	case HI6_LW:
	case HI6_LWU:
	case HI6_SW:
	case HI6_LD:
	case HI6_SD:
		/*  TODO: LWU should probably also be x64=1?  */
		size = 2; signedness = 0; store = 0;
		switch (main_opcode) {
		case HI6_LB:  size = 0; signedness = 1; break;
		case HI6_LBU: size = 0; break;
		case HI6_LH:  size = 1; signedness = 1; break;
		case HI6_LHU: size = 1; break;
		case HI6_LW:  signedness = 1; break;
		case HI6_LWU: break;
		case HI6_LD:  size = 3; x64 = 1; break;
		case HI6_SB:  store = 1; size = 0; break;
		case HI6_SH:  store = 1; size = 1; break;
		case HI6_SW:  store = 1; break;
		case HI6_SD:  store = 1; size = 3; x64 = 1; break;
		}

		ic->f =
#ifdef MODE32
		    mips32_loadstore
#else
		    mips_loadstore
#endif
		    [ (cpu->byte_order == EMUL_LITTLE_ENDIAN? 0 : 16)
		    + store * 8 + size * 2 + signedness];
		ic->arg[0] = (size_t)&cpu->cd.mips.gpr[rt];
		ic->arg[1] = (size_t)&cpu->cd.mips.gpr[rs];
		ic->arg[2] = (int32_t)imm;

		/*  Load into the dummy scratch register, if rt = zero  */
		if (!store && rt == MIPS_GPR_ZERO)
			ic->arg[0] = (size_t)&cpu->cd.mips.scratch;

		/*  Check for multiple loads or stores in a row using the same
		    base register:  */
#ifdef MODE32
		if (main_opcode == HI6_LW)
			cpu->cd.mips.combination_check = COMBINE(multi_lw);
		if (main_opcode == HI6_SW)
			cpu->cd.mips.combination_check = COMBINE(multi_sw);
#endif
		break;

	case HI6_LL:
	case HI6_LLD:
	case HI6_SC:
	case HI6_SCD:
		/*  32-bit load-linked/store-condition for ISA II and up:  */
		/*  (64-bit load-linked/store-condition for ISA III...)  */
		if (cpu->cd.mips.cpu_type.isa_level < 2) {
			ic->f = instr(reserved);
			break;
		}

		store = 0;
		switch (main_opcode) {
		case HI6_LL:  ic->f = instr(ll); break;
		case HI6_LLD: ic->f = instr(lld); x64 = 1; break;
		case HI6_SC:  ic->f = instr(sc); store = 1; break;
		case HI6_SCD: ic->f = instr(scd); store = 1; x64 = 1; break;
		}
		ic->arg[0] = (size_t)&cpu->cd.mips.gpr[rt];
		ic->arg[1] = (size_t)&cpu->cd.mips.gpr[rs];
		ic->arg[2] = (int32_t)imm;
		if (!store && rt == MIPS_GPR_ZERO) {
			if (!cpu->translation_readahead)
				fatal("HM... unusual load linked\n");
			goto bad;
		}
		break;

	case HI6_LWL:
	case HI6_LWR:
	case HI6_LDL:
	case HI6_LDR:
	case HI6_SWL:
	case HI6_SWR:
	case HI6_SDL:
	case HI6_SDR:
		/*  TODO: replace these with faster versions...  */
		store = 0;
		switch (main_opcode) {
		case HI6_LWL: ic->f = instr(lwl); break;
		case HI6_LWR: ic->f = instr(lwr); break;
		case HI6_LDL: ic->f = instr(ldl); x64 = 1; break;
		case HI6_LDR: ic->f = instr(ldr); x64 = 1; break;
		case HI6_SWL: ic->f = instr(swl); store = 1; break;
		case HI6_SWR: ic->f = instr(swr); store = 1; break;
		case HI6_SDL: ic->f = instr(sdl); store = 1; x64 = 1; break;
		case HI6_SDR: ic->f = instr(sdr); store = 1; x64 = 1; break;
		}
		ic->arg[0] = (size_t)&cpu->cd.mips.gpr[rt];
		ic->arg[1] = (size_t)&cpu->cd.mips.gpr[rs];
		ic->arg[2] = (int32_t)imm;

		/*  Load into the dummy scratch register, if rt = zero  */
		if (!store && rt == MIPS_GPR_ZERO)
			ic->arg[0] = (size_t)&cpu->cd.mips.scratch;
		break;

	case HI6_LWC1:
	case HI6_SWC1:
	case HI6_LDC1:
	case HI6_SDC1:
		/*  64-bit floating-point load/store for ISA II and up...  */
		if ((main_opcode == HI6_LDC1 || main_opcode == HI6_SDC1)
		    && cpu->cd.mips.cpu_type.isa_level < 2) {
			ic->f = instr(reserved);
			break;
		}

		ic->arg[0] = (size_t)&cpu->cd.mips.coproc[1]->reg[rt];
		ic->arg[1] = (size_t)&cpu->cd.mips.gpr[rs];
		ic->arg[2] = (int32_t)imm;
		switch (main_opcode) {
		case HI6_LWC1: ic->f = instr(lwc1); break;
		case HI6_LDC1: ic->f = instr(ldc1); break;
		case HI6_SWC1: ic->f = instr(swc1); break;
		case HI6_SDC1: ic->f = instr(sdc1); break;
		}

		/*  Cause a coprocessor unusable exception if
		    there is no floating point coprocessor:  */
		if (cpu->cd.mips.cpu_type.flags & NOFPU ||
		    cpu->cd.mips.coproc[1] == NULL) {
			ic->f = instr(cpu);
			ic->arg[0] = 1;
		}
		break;

	case HI6_LWC3:
		/*  PREF (prefetch) on ISA IV and MIPS32/64:  */
		if (cpu->cd.mips.cpu_type.isa_level >= 4) {
			/*  Treat as nop for now:  */
			ic->f = instr(nop);
		} else {
			if (!cpu->translation_readahead)
				fatal("TODO: lwc3 not implemented yet\n");
			goto bad;
		}
		break;

	case HI6_LQ_MDMX:
		if (cpu->cd.mips.cpu_type.rev == MIPS_R5900) {
			if (!cpu->translation_readahead)
				fatal("TODO: R5900 128-bit loads\n");
			goto bad;
		}

		if (!cpu->translation_readahead)
			fatal("TODO: MDMX\n");

		goto bad;
		/*  break  */

	case HI6_SQ_SPECIAL3:
		if (cpu->cd.mips.cpu_type.rev == MIPS_R5900) {
			if (!cpu->translation_readahead)
				fatal("TODO: R5900 128-bit stores\n");
			goto bad;
		}

		if (cpu->cd.mips.cpu_type.isa_level < 32 ||
		    cpu->cd.mips.cpu_type.isa_revision < 2) {
			static int warning = 0;
			if (!warning && !cpu->translation_readahead) {
				fatal("[ WARNING! SPECIAL3 opcode used, but"
				    " the %s processor does not implement "
				    "such instructions. Only printing this "
				    "warning once. ]\n",
				    cpu->cd.mips.cpu_type.name);
				warning = 1;
			}
			ic->f = instr(reserved);
			break;
		}

		switch (s6) {

		case SPECIAL3_EXT:
			{
				int msbd = rd, lsb = (iword >> 6) & 0x1f;
				ic->arg[0] = (size_t)&cpu->cd.mips.gpr[rt];
				ic->arg[1] = (size_t)&cpu->cd.mips.gpr[rs];
				ic->arg[2] = (msbd << 5) + lsb;
				ic->f = instr(ext);
				if (rt == MIPS_GPR_ZERO)
					ic->f = instr(nop);
			}
			break;

		case SPECIAL3_DEXT:
		case SPECIAL3_DEXTM:
		case SPECIAL3_DEXTU:
			{
				int msbd = rd, lsb = (iword >> 6) & 0x1f;
				if (s6 == SPECIAL3_DEXTM)
					msbd += 32;
				if (s6 == SPECIAL3_DEXTU)
					lsb += 32;
				ic->arg[0] = (size_t)&cpu->cd.mips.gpr[rt];
				ic->arg[1] = (size_t)&cpu->cd.mips.gpr[rs];
				ic->arg[2] = (msbd << 6) + lsb;
				ic->f = instr(dext);
				if (rt == MIPS_GPR_ZERO)
					ic->f = instr(nop);
			}
			break;

		case SPECIAL3_INS:
			{
				int msb = rd, lsb = (iword >> 6) & 0x1f;
				ic->arg[0] = (size_t)&cpu->cd.mips.gpr[rt];
				ic->arg[1] = (size_t)&cpu->cd.mips.gpr[rs];
				ic->arg[2] = (msb << 5) + lsb;
				ic->f = instr(ins);
				if (rt == MIPS_GPR_ZERO)
					ic->f = instr(nop);
			}
			break;

		case SPECIAL3_BSHFL:
			ic->arg[0] = (size_t)&cpu->cd.mips.gpr[rt];
			ic->arg[1] = (size_t)&cpu->cd.mips.gpr[rd];
			switch (s10) {
			case BSHFL_WSBH:
				ic->f = instr(wsbh);
				break;
			case BSHFL_SEB:
				ic->f = instr(seb);
				break;
			case BSHFL_SEH:
				ic->f = instr(seh);
				break;
			default:goto bad;
			}
			break;

		case SPECIAL3_DBSHFL:
			ic->arg[0] = (size_t)&cpu->cd.mips.gpr[rt];
			ic->arg[1] = (size_t)&cpu->cd.mips.gpr[rd];
			switch (s10) {
			case BSHFL_DSBH:
				ic->f = instr(dsbh);
				break;
			case BSHFL_DSHD:
				ic->f = instr(dshd);
				break;
			default:goto bad;
			}
			break;

		case SPECIAL3_RDHWR:
			ic->arg[0] = (size_t)&cpu->cd.mips.gpr[rt];

			switch (rd) {

			case 0:	ic->f = instr(rdhwr_cpunum);
				if (rt == MIPS_GPR_ZERO)
					ic->f = instr(nop);
				break;

			default:if (!cpu->translation_readahead)
					fatal("unimplemented rdhwr "
					    "register rd=%i\n", rd);
				goto bad;
			}
			break;

		default:goto bad;
		}
		break;

	case HI6_CACHE:
		/*  TODO: rt and op etc...  */
		ic->f = instr(cache);
		break;

	default:goto bad;
	}


#ifdef MODE32
	if (x64) {
		static int has_warned = 0;
		if (!has_warned && !cpu->translation_readahead) {
			fatal("[ WARNING/NOTE: attempt to execute a 64-bit"
			    " instruction on an emulated 32-bit processor; "
			    "pc=0x%08"PRIx32" ]\n", (uint32_t)cpu->pc);
			has_warned = 1;
		}
		if (cpu->translation_readahead)
			goto bad;
		else
			ic->f = instr(reserved);
	}
#endif


#define	DYNTRANS_TO_BE_TRANSLATED_TAIL
#include "cpu_dyntrans.cc" 
#undef	DYNTRANS_TO_BE_TRANSLATED_TAIL
}

