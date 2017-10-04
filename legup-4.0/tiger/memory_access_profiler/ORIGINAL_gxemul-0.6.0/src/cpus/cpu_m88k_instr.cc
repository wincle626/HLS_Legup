/*
 *  Copyright (C) 2007-2009  Anders Gavare.  All rights reserved.
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
 *  M88K instructions.
 *
 *  Individual functions should keep track of cpu->n_translated_instrs.
 *  (If no instruction was executed, then it should be decreased. If, say, 4
 *  instructions were combined into one function and executed, then it should
 *  be increased by 3.)
 */


#define SYNCH_PC                {                                       \
                int low_pc = ((size_t)ic - (size_t)cpu->cd.m88k.cur_ic_page) \
                    / sizeof(struct m88k_instr_call);                   \
                cpu->pc &= ~((M88K_IC_ENTRIES_PER_PAGE-1)               \
                    << M88K_INSTR_ALIGNMENT_SHIFT);                     \
                cpu->pc += (low_pc << M88K_INSTR_ALIGNMENT_SHIFT);      \
        }

#define	ABORT_EXECUTION	  {	cpu->cd.m88k.next_ic = &nothing_call;	\
				cpu->running = 0;			\
				debugger_n_steps_left_before_interaction = 0; }


/*
 *  nop:  Do nothing.
 */
X(nop)
{
}


/*
 *  br_samepage:    Branch (to within the same translated page)
 *  bsr_samepage:   Branch to subroutine (to within the same translated page)
 *
 *  arg[0] = pointer to new instr_call
 *  arg[2] = offset to return address, from start of page
 */
X(br_samepage)
{
	cpu->cd.m88k.next_ic = (struct m88k_instr_call *) ic->arg[0];
}
X(bsr_samepage)
{
	cpu->cd.m88k.r[M88K_RETURN_REG] = (cpu->pc &
	    ~((M88K_IC_ENTRIES_PER_PAGE-1) << M88K_INSTR_ALIGNMENT_SHIFT))
	    + ic->arg[2];
	cpu->cd.m88k.next_ic = (struct m88k_instr_call *) ic->arg[0];
}


/*
 *  br:    Branch (to a different translated page)
 *  br.n:  Branch (to a different translated page) with delay slot
 *  bsr:   Branch to subroutine (to a different translated page)
 *  bsr.n: Branch to subroutine (to a different page) with delay slot
 *
 *  arg[1] = relative offset from start of page
 *  arg[2] = offset to return address, from start of page
 */
X(br)
{
	cpu->pc = (uint32_t)((cpu->pc & 0xfffff000) + (int32_t)ic->arg[1]);
	quick_pc_to_pointers(cpu);
}
X(br_n)
{
	cpu->cd.m88k.delay_target = (cpu->pc & ~((M88K_IC_ENTRIES_PER_PAGE-1) <<
	    M88K_INSTR_ALIGNMENT_SHIFT)) + (int32_t)ic->arg[1];
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		cpu->pc = cpu->cd.m88k.delay_target;
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(bsr)
{
	cpu->pc &= ~((M88K_IC_ENTRIES_PER_PAGE-1) <<
	    M88K_INSTR_ALIGNMENT_SHIFT);
	cpu->cd.m88k.r[M88K_RETURN_REG] = cpu->pc + ic->arg[2];
	cpu->pc = (uint32_t) (cpu->pc + ic->arg[1]);
	quick_pc_to_pointers(cpu);
}
X(bsr_n)
{
	cpu->pc &= ~((M88K_IC_ENTRIES_PER_PAGE-1) <<
	    M88K_INSTR_ALIGNMENT_SHIFT);
	cpu->cd.m88k.r[M88K_RETURN_REG] = cpu->pc + ic->arg[2] + 4;
	cpu->cd.m88k.delay_target = cpu->pc + ic->arg[1];
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		cpu->pc = cpu->cd.m88k.delay_target;
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(bsr_trace)
{
	cpu->pc &= ~((M88K_IC_ENTRIES_PER_PAGE-1) <<
	    M88K_INSTR_ALIGNMENT_SHIFT);
	cpu->cd.m88k.r[M88K_RETURN_REG] = cpu->pc + ic->arg[2];
	cpu->pc = (uint32_t) (cpu->pc + ic->arg[1]);
	cpu_functioncall_trace(cpu, cpu->pc);
	quick_pc_to_pointers(cpu);
}
X(bsr_n_trace)
{
	cpu->pc &= ~((M88K_IC_ENTRIES_PER_PAGE-1) <<
	    M88K_INSTR_ALIGNMENT_SHIFT);
	cpu->cd.m88k.r[M88K_RETURN_REG] = cpu->pc + ic->arg[2] + 4;
	cpu->cd.m88k.delay_target = cpu->pc + ic->arg[1];
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		cpu->pc = cpu->cd.m88k.delay_target;
		cpu_functioncall_trace(cpu, cpu->pc);
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}


/*
 *  bb?            Branch if a bit in a register is 0 or 1.
 *  bb?_samepage:  Branch within the same translated page.
 *  bb?_n_*:       With delay slot.
 *
 *  arg[0] = pointer to source register to test (s1).
 *  arg[1] = uint32_t mask to test (e.g. 0x00010000 to test bit 16)
 *  arg[2] = offset from start of current page _OR_ pointer to new instr_call
 */
X(bb0)
{
	if (!(reg(ic->arg[0]) & ic->arg[1])) {
		cpu->pc = (cpu->pc & 0xfffff000) + (int32_t)ic->arg[2];
		quick_pc_to_pointers(cpu);
	}
}
X(bb0_samepage)
{
	if (!(reg(ic->arg[0]) & ic->arg[1]))
		cpu->cd.m88k.next_ic = (struct m88k_instr_call *) ic->arg[2];
}
X(bb0_n)
{
	int cond = !(reg(ic->arg[0]) & (uint32_t)ic->arg[1]);

	SYNCH_PC;

	if (cond)
		cpu->cd.m88k.delay_target =
		    (cpu->pc & ~((M88K_IC_ENTRIES_PER_PAGE-1) <<
		    M88K_INSTR_ALIGNMENT_SHIFT)) + (int32_t)ic->arg[2];
	else
		cpu->cd.m88k.delay_target = cpu->pc + 8;

	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (cond) {
			cpu->pc = cpu->cd.m88k.delay_target;
			quick_pc_to_pointers(cpu);
		} else
			cpu->cd.m88k.next_ic ++;
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(bb1)
{
	if (reg(ic->arg[0]) & ic->arg[1]) {
		cpu->pc = (cpu->pc & 0xfffff000) + (int32_t)ic->arg[2];
		quick_pc_to_pointers(cpu);
	}
}
X(bb1_samepage)
{
	if (reg(ic->arg[0]) & ic->arg[1])
		cpu->cd.m88k.next_ic = (struct m88k_instr_call *) ic->arg[2];
}
X(bb1_n)
{
	int cond = reg(ic->arg[0]) & ic->arg[1];

	SYNCH_PC;

	if (cond)
		cpu->cd.m88k.delay_target =
		    (cpu->pc & ~((M88K_IC_ENTRIES_PER_PAGE-1) <<
		    M88K_INSTR_ALIGNMENT_SHIFT)) + (int32_t)ic->arg[2];
	else
		cpu->cd.m88k.delay_target = cpu->pc + 8;

	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		if (cond) {
			cpu->pc = cpu->cd.m88k.delay_target;
			quick_pc_to_pointers(cpu);
		} else
			cpu->cd.m88k.next_ic ++;
	} else
		cpu->delay_slot = NOT_DELAYED;
}


/*
 *  ff0, ff1: Find first cleared/set bit in a register
 *
 *  arg[0] = pointer to register d
 *  arg[2] = pointer to register s2
 */
X(ff0)
{
	uint32_t mask = 0x80000000, s2 = reg(ic->arg[2]);
	int n = 31;

	for (;;) {
		if (!(s2 & mask)) {
			reg(ic->arg[0]) = n;
			return;
		}
		mask >>= 1; n--;
		if (mask == 0) {
			reg(ic->arg[0]) = 32;
			return;
		}
	}
}
X(ff1)
{
	uint32_t mask = 0x80000000, s2 = reg(ic->arg[2]);
	int n = 31;

	for (;;) {
		if (s2 & mask) {
			reg(ic->arg[0]) = n;
			return;
		}
		mask >>= 1; n--;
		if (mask == 0) {
			reg(ic->arg[0]) = 32;
			return;
		}
	}
}


/*  Include all automatically generated bcnd and bcnd.n instructions:  */
#include "tmp_m88k_bcnd.cc"


/*  Include all automatically generated load/store instructions:  */
#include "tmp_m88k_loadstore.cc"
#define M88K_LOADSTORE_STORE            4
#define M88K_LOADSTORE_SIGNEDNESS       8
#define M88K_LOADSTORE_ENDIANNESS       16
#define M88K_LOADSTORE_SCALEDNESS       32
#define M88K_LOADSTORE_USR              64
#define M88K_LOADSTORE_REGISTEROFFSET   128


/*
 *  jmp:   Jump to register
 *  jmp.n: Jump to register, with delay slot
 *  jsr:   Jump to register, set r1 to return address
 *  jsr.n: Jump to register, set r1 to return address, with delay slot
 *
 *  arg[1] = offset to return address, from start of current page
 *  arg[2] = pointer to register s2
 */
X(jmp)
{
	cpu->pc = reg(ic->arg[2]) & ~3;
	quick_pc_to_pointers(cpu);
}
X(jmp_n)
{
	cpu->cd.m88k.delay_target = reg(ic->arg[2]) & ~3;
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		cpu->pc = cpu->cd.m88k.delay_target;
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(jmp_trace)
{
	cpu->pc = reg(ic->arg[2]) & ~3;
	cpu_functioncall_trace_return(cpu);
	quick_pc_to_pointers(cpu);
}
X(jmp_n_trace)
{
	cpu->cd.m88k.delay_target = reg(ic->arg[2]) & ~3;
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		cpu->pc = cpu->cd.m88k.delay_target;
		cpu_functioncall_trace_return(cpu);
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(jsr)
{
	cpu->cd.m88k.r[M88K_RETURN_REG] = (cpu->pc & 0xfffff000) + ic->arg[1];
	cpu->pc = reg(ic->arg[2]) & ~3;
	quick_pc_to_pointers(cpu);
}
X(jsr_n)
{
	cpu->cd.m88k.delay_target = reg(ic->arg[2]) & ~3;
	cpu->cd.m88k.r[M88K_RETURN_REG] = (cpu->pc & 0xfffff000) + ic->arg[1];
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		cpu->pc = cpu->cd.m88k.delay_target;
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}
X(jsr_trace)
{
	cpu->cd.m88k.r[M88K_RETURN_REG] = (cpu->pc & 0xfffff000) + ic->arg[1];
	cpu->pc = reg(ic->arg[2]) & ~3;
	cpu_functioncall_trace(cpu, cpu->pc);
	quick_pc_to_pointers(cpu);
}
X(jsr_n_trace)
{
	cpu->cd.m88k.delay_target = reg(ic->arg[2]) & ~3;
	cpu->cd.m88k.r[M88K_RETURN_REG] = (cpu->pc & 0xfffff000) + ic->arg[1];
	cpu->delay_slot = TO_BE_DELAYED;
	ic[1].f(cpu, ic+1);
	cpu->n_translated_instrs ++;
	if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
		/*  Note: Must be non-delayed when jumping to the new pc:  */
		cpu->delay_slot = NOT_DELAYED;
		cpu->pc = cpu->cd.m88k.delay_target;
		cpu_functioncall_trace(cpu, cpu->pc);
		quick_pc_to_pointers(cpu);
	} else
		cpu->delay_slot = NOT_DELAYED;
}


/*
 *  cmp_imm:  Compare S1 with immediate value.
 *  cmp:      Compare S1 with S2.
 *
 *  arg[0] = pointer to register d
 *  arg[1] = pointer to register s1
 *  arg[2] = pointer to register s2 or imm
 */
static void m88k_cmp(struct cpu *cpu, struct m88k_instr_call *ic, uint32_t y)
{
	uint32_t x = reg(ic->arg[1]);
	uint32_t r;

	if (x == y) {
		r = M88K_CMP_HS | M88K_CMP_LS | M88K_CMP_GE
		  | M88K_CMP_LE | M88K_CMP_EQ;
	} else {
		if (x > y)
			r = M88K_CMP_NE | M88K_CMP_HS | M88K_CMP_HI;
		else
			r = M88K_CMP_NE | M88K_CMP_LO | M88K_CMP_LS;
		if ((int32_t)x > (int32_t)y)
			r |= M88K_CMP_GE | M88K_CMP_GT;
		else
			r |= M88K_CMP_LT | M88K_CMP_LE;
	}

	reg(ic->arg[0]) = r;
}
X(cmp_imm) { m88k_cmp(cpu, ic, ic->arg[2]); }
X(cmp)     { m88k_cmp(cpu, ic, reg(ic->arg[2])); }


/*
 *  extu_imm:  Extract bits, unsigned, immediate W<O>.
 *  extu:      Extract bits, unsigned, W<O> taken from register s2.
 *  ext_imm:   Extract bits, signed, immediate W<O>.
 *  ext:       Extract bits, signed, W<O> taken from register s2.
 *  mak_imm:   Make bit field, immediate W<O>.
 *  mak:       Make bit field, W<O> taken from register s2.
 *  rot:       Rotate s1 right, nr of steps taken from s2.
 *  clr:       Clear bits, W<O> taken from register s2.
 *  set:       Set bits, W<O> taken from register s2.
 *
 *  arg[0] = pointer to register d
 *  arg[1] = pointer to register s1
 *  arg[2] = pointer to register s2 or 10 bits wwwwwooooo
 */
static void m88k_extu(struct cpu *cpu, struct m88k_instr_call *ic, int w, int o)
{
	uint32_t x = reg(ic->arg[1]) >> o;
	if (w != 0) {
		x <<= (32-w);
		x >>= (32-w);
	}
	reg(ic->arg[0]) = x;
}
static void m88k_ext(struct cpu *cpu, struct m88k_instr_call *ic, int w, int o)
{
	int32_t x = reg(ic->arg[1]);
	x >>= o;	/*  signed (arithmetic) shift  */
	if (w != 0) {
		x <<= (32-w);
		x >>= (32-w);
	}
	reg(ic->arg[0]) = x;
}
static void m88k_mak(struct cpu *cpu, struct m88k_instr_call *ic, int w, int o)
{
	uint32_t x = reg(ic->arg[1]);
	if (w != 0) {
		x <<= (32-w);
		x >>= (32-w);
	}
	reg(ic->arg[0]) = x << o;
}
X(extu_imm)
{
	m88k_extu(cpu, ic, ic->arg[2] >> 5, ic->arg[2] & 0x1f);
}
X(extu)
{
	m88k_extu(cpu, ic, (reg(ic->arg[2]) >> 5) & 0x1f,
	    reg(ic->arg[2]) & 0x1f);
}
X(ext_imm)
{
	m88k_ext(cpu, ic, ic->arg[2] >> 5, ic->arg[2] & 0x1f);
}
X(ext)
{
	m88k_ext(cpu, ic, (reg(ic->arg[2]) >> 5) & 0x1f,
	    reg(ic->arg[2]) & 0x1f);
}
X(mak_imm)
{
	m88k_mak(cpu, ic, ic->arg[2] >> 5, ic->arg[2] & 0x1f);
}
X(mak)
{
	m88k_mak(cpu, ic, (reg(ic->arg[2]) >> 5) & 0x1f,
	    reg(ic->arg[2]) & 0x1f);
}
static void m88k_rot(struct cpu *cpu, struct m88k_instr_call *ic, int n)
{
	uint32_t x = reg(ic->arg[1]);

	if (n != 0) {
		uint32_t mask = (1 << n) - 1;
		uint32_t bits = x & mask;
		x >>= n;
		x |= (bits << (32-n));
	}

	reg(ic->arg[0]) = x;
}
X(rot)
{
	m88k_rot(cpu, ic, reg(ic->arg[2]) & 0x1f);
}
X(clr)
{
	int w = (reg(ic->arg[2]) >> 5) & 0x1f, o = reg(ic->arg[2]) & 0x1f;
	uint32_t x = w == 0? 0xffffffff : ((uint32_t)1 << w) - 1;
	x <<= o;
	reg(ic->arg[0]) = reg(ic->arg[1]) & ~x;
}
X(set)
{
	int w = (reg(ic->arg[2]) >> 5) & 0x1f, o = reg(ic->arg[2]) & 0x1f;
	uint32_t x = w == 0? 0xffffffff : ((uint32_t)1 << w) - 1;
	x <<= o;
	reg(ic->arg[0]) = reg(ic->arg[1]) | x;
}


/*
 *  or_r0_imm0: d = 0		(optimized case when s1 = r0, imm = 0)
 *  or_r0_imm:  d = imm		(optimized case when s1 = r0)
 *  or_imm:     d = s1 | imm
 *  xor_imm:    d = s1 ^ imm
 *  and_imm:    d = (s1 & imm) | (s1 & 0xffff0000)
 *  and_u_imm:  d = (s1 & imm) | (s1 & 0xffff)
 *  mask_imm:   d = s1 & imm
 *  add_imm:    d = s1 - imm	(addition with overflow exception)
 *  addu_imm:   d = s1 + imm
 *  subu_imm:   d = s1 - imm
 *  inc_reg:    d ++;		(addu special case; d = d + 1)
 *  dec_reg:    d --;		(subu special case; d = d - 1)
 *  mulu_imm:   d = s1 * imm
 *  divu_imm:   d = s1 / imm	(unsigned)
 *  div_imm:    d = s1 / imm	(signed)
 *  sub_imm:    d = s1 - imm	(subtraction with overflow exception)
 *
 *  arg[0] = pointer to register d
 *  arg[1] = pointer to register s1
 *  arg[2] = imm
 */
X(or_r0_imm0)	{ reg(ic->arg[0]) = 0; }
X(or_r0_imm)	{ reg(ic->arg[0]) = ic->arg[2]; }
X(or_imm)	{ reg(ic->arg[0]) = reg(ic->arg[1]) | ic->arg[2]; }
X(xor_imm)	{ reg(ic->arg[0]) = reg(ic->arg[1]) ^ ic->arg[2]; }
X(and_imm)	{ reg(ic->arg[0]) = (reg(ic->arg[1]) & ic->arg[2])
		    | (reg(ic->arg[1]) & 0xffff0000); }
X(and_u_imm)	{ reg(ic->arg[0]) = (reg(ic->arg[1]) & ic->arg[2])
		    | (reg(ic->arg[1]) & 0xffff); }
X(mask_imm)	{ reg(ic->arg[0]) = reg(ic->arg[1]) & ic->arg[2]; }
X(add_imm)
{
	uint64_t a = (int32_t) reg(ic->arg[1]);
	uint64_t b = ic->arg[2];
	uint64_t res = a + b;
	uint64_t res2 = (int32_t) res;

	if (res != res2) {
		SYNCH_PC;
		m88k_exception(cpu, M88K_EXCEPTION_INTEGER_OVERFLOW, 0);
		return;
	}

	reg(ic->arg[0]) = res;
}
X(addu_imm)	{ reg(ic->arg[0]) = reg(ic->arg[1]) + ic->arg[2]; }
X(subu_imm)	{ reg(ic->arg[0]) = reg(ic->arg[1]) - ic->arg[2]; }
X(inc_reg)	{ reg(ic->arg[0]) ++; }
X(dec_reg)	{ reg(ic->arg[0]) --; }
X(mulu_imm)
{
	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
	} else
		reg(ic->arg[0]) = reg(ic->arg[1]) * ic->arg[2];
}
X(divu_imm)
{
	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
	} else if (ic->arg[2] == 0) {
		SYNCH_PC;
		m88k_exception(cpu, M88K_EXCEPTION_ILLEGAL_INTEGER_DIVIDE, 0);
	} else
		reg(ic->arg[0]) = (uint32_t) reg(ic->arg[1]) / (uint32_t) ic->arg[2];
}
X(div_imm)
{
	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
	} else if (ic->arg[2] == 0) {
		SYNCH_PC;
		m88k_exception(cpu, M88K_EXCEPTION_ILLEGAL_INTEGER_DIVIDE, 0);
	} else {
		int32_t res = (int32_t) reg(ic->arg[1]) / (int32_t) ic->arg[2];
		reg(ic->arg[0]) = res;
	}
}
X(sub_imm)
{
	uint64_t a = (int32_t) reg(ic->arg[1]);
	uint64_t b = ic->arg[2];
	uint64_t res = a - b;
	uint64_t res2 = (int32_t) res;

	if (res != res2) {
		SYNCH_PC;
		m88k_exception(cpu, M88K_EXCEPTION_INTEGER_OVERFLOW, 0);
		return;
	}

	reg(ic->arg[0]) = res;
}


/*
 *  or:     	d = s1 | s2
 *  or_c:     	d = s1 | ~s2
 *  or_r0:  	d =      s2
 *  xor:    	d = s1 ^ s2
 *  xor_c:    	d = s1 ^ ~s2
 *  and:    	d = s1 & s2
 *  and_c:    	d = s1 & ~s2
 *  add:   	d = s1 + s2		with trap on overflow
 *  addu:   	d = s1 + s2
 *  addu_co:   	d = s1 + s2		carry out
 *  addu_ci:   	d = s1 + s2 + carry	carry in
 *  lda_reg_X:	same as addu, but s2 is scaled by 2, 4, or 8
 *  subu:   	d = s1 - s2
 *  subu_co:   	d = s1 - s2		carry/borrow out
 *  subu_ci:   	d = s1 - s2 - (carry? 0 : 1)	carry in
 *  mul:    	d = s1 * s2
 *  divu:   	d = s1 / s2		(unsigned)
 *  div:    	d = s1 / s2		(signed)
 *
 *  arg[0] = pointer to register d
 *  arg[1] = pointer to register s1
 *  arg[2] = pointer to register s2
 */
X(or)	{ reg(ic->arg[0]) = reg(ic->arg[1]) | reg(ic->arg[2]); }
X(or_c)	{ reg(ic->arg[0]) = reg(ic->arg[1]) | ~(reg(ic->arg[2])); }
X(or_r0){ reg(ic->arg[0]) = reg(ic->arg[2]); }
X(xor)	{ reg(ic->arg[0]) = reg(ic->arg[1]) ^ reg(ic->arg[2]); }
X(xor_c){ reg(ic->arg[0]) = reg(ic->arg[1]) ^ ~(reg(ic->arg[2])); }
X(and)	{ reg(ic->arg[0]) = reg(ic->arg[1]) & reg(ic->arg[2]); }
X(and_c){ reg(ic->arg[0]) = reg(ic->arg[1]) & ~(reg(ic->arg[2])); }
X(addu)	{ reg(ic->arg[0]) = reg(ic->arg[1]) + reg(ic->arg[2]); }
X(addu_s2r0)	{ reg(ic->arg[0]) = reg(ic->arg[1]); }
X(lda_reg_2)	{ reg(ic->arg[0]) = reg(ic->arg[1]) + reg(ic->arg[2]) * 2; }
X(lda_reg_4)	{ reg(ic->arg[0]) = reg(ic->arg[1]) + reg(ic->arg[2]) * 4; }
X(lda_reg_8)	{ reg(ic->arg[0]) = reg(ic->arg[1]) + reg(ic->arg[2]) * 8; }
X(subu)	{ reg(ic->arg[0]) = reg(ic->arg[1]) - reg(ic->arg[2]); }
X(add)
{
	uint64_t s1 = (int32_t) reg(ic->arg[1]);
	uint64_t s2 = (int32_t) reg(ic->arg[2]);
	uint64_t d = s1 + s2;
	uint64_t dx = (int32_t) d;

	/*  "If the result cannot be represented as a signed 32-bit integer"
	    then there should be an exception.  */
	if (d != dx) {
		SYNCH_PC;
		m88k_exception(cpu, M88K_EXCEPTION_INTEGER_OVERFLOW, 0);
	} else {
		reg(ic->arg[0]) = d;
	}
}
X(mul)
{
	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
	} else
		reg(ic->arg[0]) = reg(ic->arg[1]) * reg(ic->arg[2]);
}
X(divu)
{
	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
	} else if (reg(ic->arg[2]) == 0) {
		SYNCH_PC;
		m88k_exception(cpu, M88K_EXCEPTION_ILLEGAL_INTEGER_DIVIDE, 0);
	} else
		reg(ic->arg[0]) = (uint32_t) reg(ic->arg[1]) / (uint32_t) reg(ic->arg[2]);
}
X(div)
{
	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
	} else if (reg(ic->arg[2]) == 0) {
		SYNCH_PC;
		m88k_exception(cpu, M88K_EXCEPTION_ILLEGAL_INTEGER_DIVIDE, 0);
	} else {
		int32_t res = (int32_t) reg(ic->arg[1]) / (int32_t) reg(ic->arg[2]);
		reg(ic->arg[0]) = res;
	}
}
X(addu_co)
{
	uint64_t a = reg(ic->arg[1]), b = reg(ic->arg[2]);
	a += b;
	reg(ic->arg[0]) = a;
	cpu->cd.m88k.cr[M88K_CR_PSR] &= ~M88K_PSR_C;
	if ((a >> 32) & 1)
		cpu->cd.m88k.cr[M88K_CR_PSR] |= M88K_PSR_C;
}
X(addu_ci)
{
	uint32_t result = reg(ic->arg[1]) + reg(ic->arg[2]);
	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_C)
		result ++;
	reg(ic->arg[0]) = result;
}
X(subu_co)
{
	uint64_t a = reg(ic->arg[1]), b = reg(ic->arg[2]);
	a -= b;
	reg(ic->arg[0]) = a;
	cpu->cd.m88k.cr[M88K_CR_PSR] &= ~M88K_PSR_C;
	if ((a >> 32) & 1)
		cpu->cd.m88k.cr[M88K_CR_PSR] |= M88K_PSR_C;
}
X(subu_ci)
{
	uint32_t result = reg(ic->arg[1]) - reg(ic->arg[2]);
	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_C)
		result --;
	reg(ic->arg[0]) = result;
}


/*
 *  ldcr:   Load value from a control register, store in register d.
 *  fldcr:  Load value from a floating point control register, store in reg d.
 *
 *  arg[0] = pointer to register d
 *  arg[1] = 6-bit control register number
 */
X(ldcr)
{
	SYNCH_PC;

	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_MODE)
		m88k_ldcr(cpu, (uint32_t *) (void *) ic->arg[0], ic->arg[1]);
	else
		m88k_exception(cpu, M88K_EXCEPTION_PRIVILEGE_VIOLATION, 0);
}
X(fldcr)
{
	SYNCH_PC;

	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_MODE || ic->arg[1] >= 62)
		reg(ic->arg[0]) = cpu->cd.m88k.fcr[ic->arg[1]];
	else {
		/*  TODO: The manual says "floating point privilege
		    violation", not just "privilege violation"!  */
		m88k_exception(cpu, M88K_EXCEPTION_PRIVILEGE_VIOLATION, 0);
	}
}


/*
 *  stcr:   Store a value into a control register.
 *  fstcr:  Store a value into a floating point control register.
 *
 *  arg[0] = pointer to source register
 *  arg[1] = 6-bit control register number
 */
X(stcr)
{
	SYNCH_PC;

	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_MODE)
		m88k_stcr(cpu, reg(ic->arg[0]), ic->arg[1], 0);
	else
		m88k_exception(cpu, M88K_EXCEPTION_PRIVILEGE_VIOLATION, 0);
}
X(fstcr)
{
	SYNCH_PC;

	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_MODE || ic->arg[1] >= 62)
		m88k_fstcr(cpu, reg(ic->arg[0]), ic->arg[1]);
	else {
		/*  TODO: The manual says "floating point privilege
		    violation", not just "privilege violation"!  */
		m88k_exception(cpu, M88K_EXCEPTION_PRIVILEGE_VIOLATION, 0);
	}
}


/*
 *  fadd.xxx:  Floating point addition
 *  fsub.xxx:  Floating point subtraction
 *  fmul.xxx:  Floating point multiplication
 *  fdiv.xxx:  Floating point multiplication
 *
 *  arg[0] = pointer to destination register
 *  arg[1] = pointer to source register s1
 *  arg[2] = pointer to source register s2
 *
 *  Note: For 'd' variants, arg[x] points to a _pair_ of registers!
 */
X(fadd_sss)
{
	struct ieee_float_value f1;
	struct ieee_float_value f2;
	uint32_t d;
	uint32_t s2 = reg(ic->arg[2]);
	uint32_t s1 = reg(ic->arg[1]);

	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
		return;
	}

	ieee_interpret_float_value(s1, &f1, IEEE_FMT_S);
	ieee_interpret_float_value(s2, &f2, IEEE_FMT_S);

	d = ieee_store_float_value(f1.f + f2.f, IEEE_FMT_S, 0);

	reg(ic->arg[0]) = d;
}
X(fadd_dsd)
{
	struct ieee_float_value f1;
	struct ieee_float_value f2;
	uint64_t d;
	uint64_t s2 = reg(ic->arg[2]);
	uint32_t s1 = reg(ic->arg[1]);
	s2 = (s2 << 32) + reg(ic->arg[2] + 4);

	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
		return;
	}

	ieee_interpret_float_value(s1, &f1, IEEE_FMT_S);
	ieee_interpret_float_value(s2, &f2, IEEE_FMT_D);

	d = ieee_store_float_value(f1.f + f2.f, IEEE_FMT_D, 0);

	reg(ic->arg[0]) = d >> 32;	/*  High 32-bit word,  */
	reg(ic->arg[0] + 4) = d;	/*  and low word.  */
}
X(fadd_dds)
{
	struct ieee_float_value f1;
	struct ieee_float_value f2;
	uint64_t d;
	uint32_t s2 = reg(ic->arg[2]);
	uint64_t s1 = reg(ic->arg[1]);
	s1 = (s1 << 32) + reg(ic->arg[1] + 4);

	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
		return;
	}

	ieee_interpret_float_value(s1, &f1, IEEE_FMT_D);
	ieee_interpret_float_value(s2, &f2, IEEE_FMT_S);

	d = ieee_store_float_value(f1.f + f2.f, IEEE_FMT_D, 0);

	reg(ic->arg[0]) = d >> 32;	/*  High 32-bit word,  */
	reg(ic->arg[0] + 4) = d;	/*  and low word.  */
}
X(fadd_ddd)
{
	struct ieee_float_value f1;
	struct ieee_float_value f2;
	uint64_t d;
	uint64_t s1 = reg(ic->arg[1]);
	uint64_t s2 = reg(ic->arg[2]);
	s1 = (s1 << 32) + reg(ic->arg[1] + 4);
	s2 = (s2 << 32) + reg(ic->arg[2] + 4);

	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
		return;
	}

	ieee_interpret_float_value(s1, &f1, IEEE_FMT_D);
	ieee_interpret_float_value(s2, &f2, IEEE_FMT_D);

	d = ieee_store_float_value(f1.f + f2.f, IEEE_FMT_D, 0);

	reg(ic->arg[0]) = d >> 32;	/*  High 32-bit word,  */
	reg(ic->arg[0] + 4) = d;	/*  and low word.  */
}
X(fsub_sds)
{
	struct ieee_float_value f1;
	struct ieee_float_value f2;
	uint32_t d;
	uint32_t s2 = reg(ic->arg[2]);
	uint64_t s1 = reg(ic->arg[1]);
	s1 = (s1 << 32) + reg(ic->arg[1] + 4);

	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
		return;
	}

	ieee_interpret_float_value(s1, &f1, IEEE_FMT_D);
	ieee_interpret_float_value(s2, &f2, IEEE_FMT_S);

	d = ieee_store_float_value(f1.f - f2.f, IEEE_FMT_S, 0);
	reg(ic->arg[0]) = d;
}
X(fsub_dss)
{
	struct ieee_float_value f1;
	struct ieee_float_value f2;
	uint64_t d;
	uint32_t s2 = reg(ic->arg[2]);
	uint32_t s1 = reg(ic->arg[1]);

	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
		return;
	}

	ieee_interpret_float_value(s1, &f1, IEEE_FMT_S);
	ieee_interpret_float_value(s2, &f2, IEEE_FMT_S);

	d = ieee_store_float_value(f1.f - f2.f, IEEE_FMT_D, 0);

	reg(ic->arg[0]) = d >> 32;	/*  High 32-bit word,  */
	reg(ic->arg[0] + 4) = d;	/*  and low word.  */
}
X(fsub_dsd)
{
	struct ieee_float_value f1;
	struct ieee_float_value f2;
	uint64_t d;
	uint64_t s2 = reg(ic->arg[2]);
	uint32_t s1 = reg(ic->arg[1]);
	s2 = (s2 << 32) + reg(ic->arg[2] + 4);

	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
		return;
	}

	ieee_interpret_float_value(s1, &f1, IEEE_FMT_S);
	ieee_interpret_float_value(s2, &f2, IEEE_FMT_D);

	d = ieee_store_float_value(f1.f - f2.f, IEEE_FMT_D, 0);

	reg(ic->arg[0]) = d >> 32;	/*  High 32-bit word,  */
	reg(ic->arg[0] + 4) = d;	/*  and low word.  */
}
X(fsub_dds)
{
	struct ieee_float_value f1;
	struct ieee_float_value f2;
	uint64_t d;
	uint32_t s2 = reg(ic->arg[2]);
	uint64_t s1 = reg(ic->arg[1]);
	s1 = (s1 << 32) + reg(ic->arg[1] + 4);

	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
		return;
	}

	ieee_interpret_float_value(s1, &f1, IEEE_FMT_D);
	ieee_interpret_float_value(s2, &f2, IEEE_FMT_S);

	d = ieee_store_float_value(f1.f - f2.f, IEEE_FMT_D, 0);

	reg(ic->arg[0]) = d >> 32;	/*  High 32-bit word,  */
	reg(ic->arg[0] + 4) = d;	/*  and low word.  */
}
X(fsub_ddd)
{
	struct ieee_float_value f1;
	struct ieee_float_value f2;
	uint64_t d;
	uint64_t s1 = reg(ic->arg[1]);
	uint64_t s2 = reg(ic->arg[2]);
	s1 = (s1 << 32) + reg(ic->arg[1] + 4);
	s2 = (s2 << 32) + reg(ic->arg[2] + 4);

	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
		return;
	}

	ieee_interpret_float_value(s1, &f1, IEEE_FMT_D);
	ieee_interpret_float_value(s2, &f2, IEEE_FMT_D);

	d = ieee_store_float_value(f1.f - f2.f, IEEE_FMT_D, 0);

	reg(ic->arg[0]) = d >> 32;	/*  High 32-bit word,  */
	reg(ic->arg[0] + 4) = d;	/*  and low word.  */
}
X(fmul_sss)
{
	struct ieee_float_value f1;
	struct ieee_float_value f2;
	uint32_t d;
	uint32_t s2 = reg(ic->arg[2]);
	uint32_t s1 = reg(ic->arg[1]);

	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
		return;
	}

	ieee_interpret_float_value(s1, &f1, IEEE_FMT_S);
	ieee_interpret_float_value(s2, &f2, IEEE_FMT_S);

	d = ieee_store_float_value(f1.f * f2.f, IEEE_FMT_S, 0);
	reg(ic->arg[0]) = d;
}
X(fmul_dss)
{
	struct ieee_float_value f1;
	struct ieee_float_value f2;
	uint64_t d;
	uint32_t s2 = reg(ic->arg[2]);
	uint32_t s1 = reg(ic->arg[1]);

	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
		return;
	}

	ieee_interpret_float_value(s1, &f1, IEEE_FMT_S);
	ieee_interpret_float_value(s2, &f2, IEEE_FMT_S);

	d = ieee_store_float_value(f1.f * f2.f, IEEE_FMT_D, 0);

	reg(ic->arg[0]) = d >> 32;	/*  High 32-bit word,  */
	reg(ic->arg[0] + 4) = d;	/*  and low word.  */
}
X(fmul_dsd)
{
	struct ieee_float_value f1;
	struct ieee_float_value f2;
	uint64_t d;
	uint64_t s2 = reg(ic->arg[2]);
	uint32_t s1 = reg(ic->arg[1]);
	s2 = (s2 << 32) + reg(ic->arg[2] + 4);

	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
		return;
	}

	ieee_interpret_float_value(s1, &f1, IEEE_FMT_S);
	ieee_interpret_float_value(s2, &f2, IEEE_FMT_D);

	d = ieee_store_float_value(f1.f * f2.f, IEEE_FMT_D, 0);

	reg(ic->arg[0]) = d >> 32;	/*  High 32-bit word,  */
	reg(ic->arg[0] + 4) = d;	/*  and low word.  */
}
X(fmul_dds)
{
	struct ieee_float_value f1;
	struct ieee_float_value f2;
	uint64_t d;
	uint32_t s2 = reg(ic->arg[2]);
	uint64_t s1 = reg(ic->arg[1]);
	s1 = (s1 << 32) + reg(ic->arg[1] + 4);

	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
		return;
	}

	ieee_interpret_float_value(s1, &f1, IEEE_FMT_D);
	ieee_interpret_float_value(s2, &f2, IEEE_FMT_S);

	d = ieee_store_float_value(f1.f * f2.f, IEEE_FMT_D, 0);

	reg(ic->arg[0]) = d >> 32;	/*  High 32-bit word,  */
	reg(ic->arg[0] + 4) = d;	/*  and low word.  */
}
X(fmul_ddd)
{
	struct ieee_float_value f1;
	struct ieee_float_value f2;
	uint64_t d;
	uint64_t s1 = reg(ic->arg[1]);
	uint64_t s2 = reg(ic->arg[2]);
	s1 = (s1 << 32) + reg(ic->arg[1] + 4);
	s2 = (s2 << 32) + reg(ic->arg[2] + 4);

	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
		return;
	}

	ieee_interpret_float_value(s1, &f1, IEEE_FMT_D);
	ieee_interpret_float_value(s2, &f2, IEEE_FMT_D);

	d = ieee_store_float_value(f1.f * f2.f, IEEE_FMT_D, 0);

	reg(ic->arg[0]) = d >> 32;	/*  High 32-bit word,  */
	reg(ic->arg[0] + 4) = d;	/*  and low word.  */
}
X(fdiv_sss)
{
	struct ieee_float_value f1;
	struct ieee_float_value f2;
	uint32_t d;
	uint32_t s1 = reg(ic->arg[1]);
	uint32_t s2 = reg(ic->arg[2]);

	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
		return;
	}

	ieee_interpret_float_value(s1, &f1, IEEE_FMT_S);
	ieee_interpret_float_value(s2, &f2, IEEE_FMT_S);

	if (f2.f == 0) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FDVZ;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
		return;
	}

	d = ieee_store_float_value(f1.f / f2.f, IEEE_FMT_D, 0);

	reg(ic->arg[0]) = d;
}
X(fdiv_dsd)
{
	struct ieee_float_value f1;
	struct ieee_float_value f2;
	uint64_t d;
	uint32_t s1 = reg(ic->arg[1]);
	uint64_t s2 = reg(ic->arg[2]);
	s2 = (s2 << 32) + reg(ic->arg[2] + 4);

	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
		return;
	}

	ieee_interpret_float_value(s1, &f1, IEEE_FMT_S);
	ieee_interpret_float_value(s2, &f2, IEEE_FMT_D);

	if (f2.f == 0) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FDVZ;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
		return;
	}

	d = ieee_store_float_value(f1.f / f2.f, IEEE_FMT_D, 0);

	reg(ic->arg[0]) = d >> 32;	/*  High 32-bit word,  */
	reg(ic->arg[0] + 4) = d;	/*  and low word.  */
}
X(fdiv_ddd)
{
	struct ieee_float_value f1;
	struct ieee_float_value f2;
	uint64_t d;
	uint64_t s1 = reg(ic->arg[1]);
	uint64_t s2 = reg(ic->arg[2]);
	s1 = (s1 << 32) + reg(ic->arg[1] + 4);
	s2 = (s2 << 32) + reg(ic->arg[2] + 4);

	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
		return;
	}

	ieee_interpret_float_value(s1, &f1, IEEE_FMT_D);
	ieee_interpret_float_value(s2, &f2, IEEE_FMT_D);

	if (f2.f == 0) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FDVZ;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
		return;
	}

	d = ieee_store_float_value(f1.f / f2.f, IEEE_FMT_D, 0);

	reg(ic->arg[0]) = d >> 32;	/*  High 32-bit word,  */
	reg(ic->arg[0] + 4) = d;	/*  and low word.  */
}


/*
 *  fcmp.sXX:  Floating point comparison
 *
 *  arg[0] = pointer to destination register
 *  arg[1] = pointer to source register s1
 *  arg[2] = pointer to source register s2
 *
 *  Note: For 'd' variants, arg[x] points to a _pair_ of registers!
 */
static uint32_t m88k_fcmp_common(struct ieee_float_value *f1,
	struct ieee_float_value *f2)
{
	uint32_t d;

	/*  TODO: Implement all bits correctly, e.g. "in range" bits.  */
	d = 0;
	if (isnan(f1->f) || isnan(f2->f))
		d |= (1 << 0);
	else {
		d |= (1 << 1);
		if (f1->f == f2->f)
			d |= (1 << 2);
		else
			d |= (1 << 3);
		if (f1->f > f2->f)
			d |= (1 << 4);
		else
			d |= (1 << 5);
		if (f1->f < f2->f)
			d |= (1 << 6);
		else
			d |= (1 << 7);
	}

	return d;
}
X(fcmp_sds)
{
	struct ieee_float_value f1;
	struct ieee_float_value f2;
	uint32_t s2 = reg(ic->arg[2]);
	uint64_t s1 = reg(ic->arg[1]);
	s1 = (s1 << 32) + reg(ic->arg[1] + 4);

	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
		return;
	}

	ieee_interpret_float_value(s1, &f1, IEEE_FMT_D);
	ieee_interpret_float_value(s2, &f2, IEEE_FMT_S);
	reg(ic->arg[0]) = m88k_fcmp_common(&f1, &f2);
}
X(fcmp_sdd)
{
	struct ieee_float_value f1;
	struct ieee_float_value f2;
	uint64_t s1 = reg(ic->arg[1]);
	uint64_t s2 = reg(ic->arg[2]);
	s1 = (s1 << 32) + reg(ic->arg[1] + 4);
	s2 = (s2 << 32) + reg(ic->arg[2] + 4);

	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
		return;
	}

	ieee_interpret_float_value(s1, &f1, IEEE_FMT_D);
	ieee_interpret_float_value(s2, &f2, IEEE_FMT_D);
	reg(ic->arg[0]) = m88k_fcmp_common(&f1, &f2);
}


/*
 *  flt.ss and flt.ds:  Convert integer to floating point.
 *
 *  arg[0] = pointer to destination register
 *  arg[1] = pointer to source register s2
 *
 *  Note: For flt.ds, arg[0] points to a _pair_ of registers!
 */
X(flt_ss)
{
	int32_t x = reg(ic->arg[1]);

	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
		return;
	}

	reg(ic->arg[0]) = ieee_store_float_value((double)x, IEEE_FMT_S, 0);
}
X(flt_ds)
{
	int32_t x = reg(ic->arg[1]);
	uint64_t result;

	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
		return;
	}

	result = ieee_store_float_value((double)x, IEEE_FMT_D, 0);

	reg(ic->arg[0]) = result >> 32;	/*  High 32-bit word,  */
	reg(ic->arg[0] + 4) = result;	/*  and low word.  */
}


/*
 *  trnc.ss and trnc.sd:  Truncate floating point to interger.
 *
 *  arg[0] = pointer to destination register
 *  arg[1] = pointer to source register s2
 *
 *  Note: For trnc.sd, arg[1] points to a _pair_ of registers!
 */
X(trnc_ss)
{
	struct ieee_float_value f1;
	ieee_interpret_float_value(reg(ic->arg[1]), &f1, IEEE_FMT_S);

	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
		return;
	}

	reg(ic->arg[0]) = (int32_t) f1.f;
}
X(trnc_sd)
{
	struct ieee_float_value f1;
	uint64_t x = reg(ic->arg[1]);
	x = (x << 32) + reg(ic->arg[1] + 4);
	ieee_interpret_float_value(x, &f1, IEEE_FMT_D);

	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFD1) {
		SYNCH_PC;
		cpu->cd.m88k.fcr[M88K_FPCR_FPECR] = M88K_FPECR_FUNIMP;
		m88k_exception(cpu, M88K_EXCEPTION_SFU1_PRECISE, 0);
		return;
	}

	reg(ic->arg[0]) = (int32_t) f1.f;
}


/*
 *  xcr:   Exchange (load + store) control register.
 *
 *  arg[0] = pointer to register d
 *  arg[1] = pointer to register s1
 *  arg[2] = 6-bit control register number
 */
X(xcr)
{
	uint32_t tmp, tmp2;
	SYNCH_PC;

	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_MODE) {
		tmp = reg(ic->arg[1]);
		m88k_ldcr(cpu, &tmp2, ic->arg[2]);
		m88k_stcr(cpu, tmp, ic->arg[2], 0);
		reg(ic->arg[0]) = tmp2;
	} else
		m88k_exception(cpu, M88K_EXCEPTION_PRIVILEGE_VIOLATION, 0);
}


/*
 *  rte:  Return from exception
 */
X(rte)
{
	/*  If executed from user mode, then cause an exception:  */
	if (!(cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_MODE)) {
		SYNCH_PC;
		m88k_exception(cpu, M88K_EXCEPTION_PRIVILEGE_VIOLATION, 0);
		return;
	}

	m88k_stcr(cpu, cpu->cd.m88k.cr[M88K_CR_EPSR], M88K_CR_PSR, 1);

	if (cpu->cd.m88k.cr[M88K_CR_SNIP] & M88K_NIP_E) {
		fatal("rte: NIP: TODO: single-step support\n");
		goto abort_dump;
	}

	if (cpu->cd.m88k.cr[M88K_CR_SFIP] & M88K_FIP_E) {
		fatal("rte: TODO: FIP single-step support\n");
		goto abort_dump;
	}

	/*  First try the NIP, if it is Valid:  */
	cpu->pc = cpu->cd.m88k.cr[M88K_CR_SNIP] & M88K_NIP_ADDR;

	/*  If the NIP is not valid, then try the FIP:  */
	if (!(cpu->cd.m88k.cr[M88K_CR_SNIP] & M88K_NIP_V)) {
		/*  Neither the NIP nor the FIP valid?  */
		if (!(cpu->cd.m88k.cr[M88K_CR_SFIP] & M88K_FIP_V)) {
			if ((cpu->cd.m88k.cr[M88K_CR_SFIP] & M88K_FIP_ADDR)
			    != (cpu->cd.m88k.cr[M88K_CR_SNIP] & M88K_NIP_ADDR) + 4) {
				fatal("[ TODO: Neither FIP nor NIP has the "
				    "Valid bit set?! ]\n");
				goto abort_dump;
			}

			/*  For now, continue anyway, using NIP.  */
		} else {
			cpu->pc = cpu->cd.m88k.cr[M88K_CR_SFIP] & M88K_FIP_ADDR;
		}
	} else if (cpu->cd.m88k.cr[M88K_CR_SNIP] & M88K_NIP_V &&
	    cpu->cd.m88k.cr[M88K_CR_SFIP] & M88K_FIP_V &&
	    (cpu->cd.m88k.cr[M88K_CR_SFIP] & M88K_FIP_ADDR)
	    != (cpu->cd.m88k.cr[M88K_CR_SNIP] & M88K_NIP_ADDR) + 4) {
		/*
		 *  The NIP instruction should first be executed (this
		 *  is the one the exception handler choose to return to),
		 *  and then the FIP instruction should run (the target
		 *  of a delayed branch).
		 */

		uint32_t nip = cpu->cd.m88k.cr[M88K_CR_SNIP] & M88K_NIP_ADDR;
		uint32_t fip = cpu->cd.m88k.cr[M88K_CR_SFIP] & M88K_FIP_ADDR;

		cpu->pc = nip;
		cpu->delay_slot = NOT_DELAYED;
		quick_pc_to_pointers(cpu);

		if (cpu->pc != nip) {
			fatal("NIP execution caused exception?! TODO\n");
			goto abort_dump;
		}

		instr(to_be_translated)(cpu, cpu->cd.m88k.next_ic);

		if ((cpu->pc & 0xfffff000) != (nip & 0xfffff000)) {
			fatal("instruction in delay slot when returning via"
			    " rte caused an exception?! nip=0x%08x, but pc "
			    "changed to 0x%08x! TODO\n", nip, (int)cpu->pc);
			goto abort_dump;
		}

		cpu->pc = fip;
		cpu->delay_slot = NOT_DELAYED;
		quick_pc_to_pointers(cpu);
		return;
	}

	/*  fatal("RTE: NIP=0x%08"PRIx32", FIP=0x%08"PRIx32"\n",
	    cpu->cd.m88k.cr[M88K_CR_SNIP], cpu->cd.m88k.cr[M88K_CR_SFIP]);  */

	quick_pc_to_pointers(cpu);
	return;

abort_dump:
	fatal("RTE failed. NIP=0x%08"PRIx32", FIP=0x%08"PRIx32"\n",
	    cpu->cd.m88k.cr[M88K_CR_SNIP], cpu->cd.m88k.cr[M88K_CR_SFIP]);

	ABORT_EXECUTION;
}


/*
 *  xmem_slow:  Unoptimized xmem (exchange register with memory)
 *
 *  arg[0] = copy of the instruction word
 */
X(xmem_slow)
{
	uint32_t iword = ic->arg[0], addr;
	uint8_t tmp[4];
	uint8_t data[4];
	int d      = (iword >> 21) & 0x1f;
	int s1     = (iword >> 16) & 0x1f;
	int s2     =  iword        & 0x1f;
	int imm16  =  iword        & 0xffff;
	int scaled = iword & 0x200;
	int size   = iword & 0x400;
	int user   = iword & 0x80;

	SYNCH_PC;

	if (user) {
		fatal("xmem_slow: user: not yet (TODO)\n");
		exit(1);
	}

	if ((iword & 0xf0000000) == 0) {
		/*  immediate offset:  */
		addr = imm16;
		scaled = 0;
		size = (iword >> 26) & 1;
		user = 0;
	} else {
		/*  register offset:  */
		addr = cpu->cd.m88k.r[s2];
		if (scaled && size)
			addr *= sizeof(uint32_t);
	}

	addr += cpu->cd.m88k.r[s1];

	if (size) {
		uint32_t x = cpu->cd.m88k.r[d];
		if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
			x = LE32_TO_HOST(x);
		else
			x = BE32_TO_HOST(x);

		{
			uint32_t *p = (uint32_t *) tmp;
			*p = x;
		}

		if (addr & 3) {
			m88k_exception(cpu,
			    M88K_EXCEPTION_MISALIGNED_ACCESS, 0);
			return;
		}
	} else
		tmp[0] = cpu->cd.m88k.r[d];

	if (!cpu->memory_rw(cpu, cpu->mem, addr, (uint8_t *) &data,
	    size? 4 : 1, MEM_READ, CACHE_DATA)) {
		/*  Exception.  */

		fatal("XMEM exception: TODO: update the transaction"
		    " registers!\n");
		exit(1);
		/*  return;  */
	}

	if (!cpu->memory_rw(cpu, cpu->mem, addr, (uint8_t *) &tmp,
	    size? 4 : 1, MEM_WRITE, CACHE_DATA)) {
		/*  Exception.  */

		fatal("XMEM exception: TODO: update the transaction"
		    " registers!\n");
		exit(1);
		/*  return;  */
	}

	if (size) {
		uint32_t x;
		uint32_t *p = (uint32_t *) data;
		x = *p;

		if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
			x = LE32_TO_HOST(x);
		else
			x = BE32_TO_HOST(x);
		cpu->cd.m88k.r[d] = x;
	} else
		cpu->cd.m88k.r[d] = data[0];
}


/*
 *  prom-call:
 */
X(prom_call)
{
	/*  If executed from user mode, then cause an exception:  */
	if (!(cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_MODE)) {
		SYNCH_PC;
		m88k_exception(cpu, M88K_EXCEPTION_UNIMPLEMENTED_OPCODE, 0);
		return;
	}

	switch (cpu->machine->machine_type) {
	case MACHINE_MVME88K:
		mvmeprom_emul(cpu);
		break;
	default:fatal("m88k prom_call: unimplemented machine type\n");
		ABORT_EXECUTION;
	}

	if (!cpu->running) {
		cpu->n_translated_instrs --;
		cpu->cd.m88k.next_ic = &nothing_call;
	}
}


/*
 *  tb0, tb1:  Trap on bit Clear/Set
 *
 *  arg[0] = bitmask to check (e.g. 0x00020000 for bit 17)
 *  arg[1] = pointer to register s1
 *  arg[2] = 9-bit vector number
 */
X(tb0)
{
	SYNCH_PC;

	if (!(cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_MODE)
	    && ic->arg[2] < M88K_EXCEPTION_USER_TRAPS_START) {
		m88k_exception(cpu, M88K_EXCEPTION_PRIVILEGE_VIOLATION, 0);
		return;
	}

	if (!(reg(ic->arg[1]) & ic->arg[0]))
		m88k_exception(cpu, ic->arg[2], 1);
}
X(tb1)
{
	SYNCH_PC;

	if (!(cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_MODE)
	    && ic->arg[2] < M88K_EXCEPTION_USER_TRAPS_START) {
		m88k_exception(cpu, M88K_EXCEPTION_PRIVILEGE_VIOLATION, 0);
		return;
	}

	if (reg(ic->arg[1]) & ic->arg[0])
		m88k_exception(cpu, ic->arg[2], 1);
}


/*
 *  idle:
 *
 *  s:	ld    rX,rY,ofs
 *      bcnd  eq0,rX,s
 */
X(idle)
{
	uint32_t rY = reg(ic[0].arg[1]) + ic[0].arg[2];
	uint32_t index = rY >> 12;
	unsigned char *p = cpu->cd.m88k.host_load[index];
	uint32_t *p32 = (uint32_t *) p;
	uint32_t v;

	/*  Fallback:  */
	if (p == NULL || (rY & 3)) {
		instr(ld_u_4_be)(cpu, ic);
		return;
	}

	v = p32[(rY & 0xfff) >> 2];
	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		v = LE32_TO_HOST(v);
	else
		v = BE32_TO_HOST(v);

	reg(ic[0].arg[0]) = v;

	if (v == 0) {
		SYNCH_PC;
		usleep(50);
		cpu->has_been_idling = 1;
		cpu->n_translated_instrs += N_SAFE_DYNTRANS_LIMIT / 2;
		cpu->cd.m88k.next_ic = &nothing_call;
	} else {
		cpu->n_translated_instrs ++;
		cpu->cd.m88k.next_ic = &ic[2];
	}
}


/*
 *  idle_with_tb1:
 *
 *  s:	tb1	bit,r0,vector
 *	ld	rX,rY,ofs
 *	bcnd	eq0,rX,s
 */
X(idle_with_tb1)
{
	uint32_t rY = reg(ic[1].arg[1]) + ic[1].arg[2];
	uint32_t index = rY >> 12;
	unsigned char *p = cpu->cd.m88k.host_load[index];
	uint32_t *p32 = (uint32_t *) p;
	uint32_t v;

	/*  Fallback:  */
	if (p == NULL || (rY & 3)) {
		instr(tb1)(cpu, ic);
		return;
	}

	v = p32[(rY & 0xfff) >> 2];
	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		v = LE32_TO_HOST(v);
	else
		v = BE32_TO_HOST(v);

	reg(ic[1].arg[0]) = v;

	if (v == 0) {
		SYNCH_PC;
		usleep(50);
		cpu->has_been_idling = 1;
		cpu->n_translated_instrs += N_SAFE_DYNTRANS_LIMIT / 2;
		cpu->cd.m88k.next_ic = &nothing_call;
	} else {
		cpu->n_translated_instrs += 2;
		cpu->cd.m88k.next_ic = &ic[3];
	}
}


/*****************************************************************************/


X(end_of_page)
{
	/*  Update the PC:  (offset 0, but on the next page)  */
	cpu->pc &= ~((M88K_IC_ENTRIES_PER_PAGE-1) <<
	    M88K_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (M88K_IC_ENTRIES_PER_PAGE << M88K_INSTR_ALIGNMENT_SHIFT);

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

	instr(to_be_translated)(cpu, cpu->cd.m88k.next_ic);

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
	int low_pc = ((size_t)ic - (size_t)cpu->cd.m88k.cur_ic_page)
	    / sizeof(struct m88k_instr_call);
	cpu->pc &= ~((M88K_IC_ENTRIES_PER_PAGE-1)
	    << M88K_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << M88K_INSTR_ALIGNMENT_SHIFT);

	if (low_pc < 0 || low_pc > ((M88K_IC_ENTRIES_PER_PAGE+1)
	    << M88K_INSTR_ALIGNMENT_SHIFT)) {
		printf("[ end_of_page2: HUH? low_pc=%i, cpu->pc = %08"
		    PRIx32" ]\n", low_pc, (uint32_t) cpu->pc);
	}

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
 *  Idle loop detection, e.g.:
 *
 *  s00028d44: 15b7d650	ld	r13,r23,0xd650	    ; [<_sched_whichqs>]
 *  s00028d48: e84dffff	bcnd	eq0,r13,0x00028d44  ; <_sched_idle+0xc4>
 *
 *  or
 *
 *  s00079d78: f020d8ff	tb1	1,r0,0xff
 *  s00079d7c: 15ac7320	ld	r13,r12,0x7320	; [<_sched_whichqs>]
 *  s00079d80: e84dfffe	bcnd	eq0,r13,0x00079d78	; <_sched_idle+0x158>
 */
void COMBINE(idle)(struct cpu *cpu, struct m88k_instr_call *ic, int low_addr)
{
	int n_back = (low_addr >> M88K_INSTR_ALIGNMENT_SHIFT)
	    & (M88K_IC_ENTRIES_PER_PAGE-1);
	if (n_back < 2)
		return;

	if (ic[0].f == instr(bcnd_samepage_eq0) &&
	    ic[0].arg[2] == (size_t) &ic[-1] &&
	    ic[-1].f == instr(ld_u_4_be) &&
	    ic[0].arg[0] == ic[-1].arg[0] &&
	    ic[0].arg[0] != (size_t) &cpu->cd.m88k.r[M88K_ZERO_REG]) {
		ic[-1].f = instr(idle);
		return;
	}

	if (ic[0].f == instr(bcnd_samepage_eq0) &&
	    ic[0].arg[2] == (size_t) &ic[-2] &&
	    ic[-2].f == instr(tb1) &&
	    ic[-2].arg[1] == (size_t) &cpu->cd.m88k.r[M88K_ZERO_REG] &&
	    ic[-1].f == instr(ld_u_4_be) &&
	    ic[0].arg[0] == ic[-1].arg[0] &&
	    ic[0].arg[0] != (size_t) &cpu->cd.m88k.r[M88K_ZERO_REG]) {
		ic[-2].f = instr(idle_with_tb1);
		return;
	}
}


/*****************************************************************************/


/*
 *  m88k_instr_to_be_translated():
 *
 *  Translate an instruction word into a m88k_instr_call. ic is filled in with
 *  valid data for the translated instruction, or a "nothing" instruction if
 *  there was a translation failure. The newly translated instruction is then
 *  executed.
 */
X(to_be_translated)
{
	uint32_t addr, low_pc, iword;
	unsigned char *page;
	unsigned char ib[4];
	uint32_t op26, op10, op11, d, s1, s2, cr6, imm16;
	int32_t d16, d26, simm16;
	int offset, shift;
	int in_crosspage_delayslot = 0;
	void (*samepage_function)(struct cpu *, struct m88k_instr_call *)=NULL;

	/*  Figure out the (virtual) address of the instruction:  */
	low_pc = ((size_t)ic - (size_t)cpu->cd.m88k.cur_ic_page)
	    / sizeof(struct m88k_instr_call);

	/*  Special case for branch with delayslot on the next page:  */
	if (cpu->delay_slot == TO_BE_DELAYED && low_pc == 0) {
		/*  fatal("[ delay-slot translation across page "
		    "boundary ]\n");  */
		in_crosspage_delayslot = 1;
	}

	addr = cpu->pc & ~((M88K_IC_ENTRIES_PER_PAGE-1)
	    << M88K_INSTR_ALIGNMENT_SHIFT);
	addr += (low_pc << M88K_INSTR_ALIGNMENT_SHIFT);
	cpu->pc = (MODE_int_t)addr;
	addr &= ~((1 << M88K_INSTR_ALIGNMENT_SHIFT) - 1);

	/*  Read the instruction word from memory:  */
	page = cpu->cd.m88k.host_load[(uint32_t)addr >> 12];

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
	if (cpu->cd.m88k.r[M88K_ZERO_REG] != 0) {
		fatal("INTERNAL ERROR! M88K_ZERO_REG != 0?\n");
		exit(1);
	}

	op26   = (iword >> 26) & 0x3f;
	op11   = (iword >> 11) & 0x1f;
	op10   = (iword >> 10) & 0x3f;
	d      = (iword >> 21) & 0x1f;
	s1     = (iword >> 16) & 0x1f;
	s2     =  iword        & 0x1f;
	imm16  =  iword        & 0xffff;
	simm16 = (int16_t) (iword & 0xffff);
	cr6    = (iword >>  5) & 0x3f;
	d16    = ((int16_t) (iword & 0xffff)) * 4;
	d26    = ((int32_t)((iword & 0x03ffffff) << 6)) >> 4;

	switch (op26) {

	case 0x00:
	case 0x01:
		ic->f = instr(xmem_slow);
		ic->arg[0] = iword;
		if (d == M88K_ZERO_REG)
			ic->f = instr(nop);
		if (iword == 0)
			goto bad;
		break;

	case 0x02:	/*  ld.hu  */
	case 0x03:	/*  ld.bu  */
	case 0x04:	/*  ld.d   */
	case 0x05:	/*  ld     */
	case 0x06:	/*  ld.h   */
	case 0x07:	/*  ld.b   */
	case 0x08:	/*  st.d   */
	case 0x09:	/*  st     */
	case 0x0a:	/*  st.h   */
	case 0x0b:	/*  st.b   */
		{
			int store = 0, signedness = 0, opsize = 0;

			ic->arg[0] = (size_t) &cpu->cd.m88k.r[d];
			ic->arg[1] = (size_t) &cpu->cd.m88k.r[s1];
			ic->arg[2] = imm16;

			switch (op26) {
			case 0x02: opsize = 1; break;
			case 0x03: opsize = 0; break;
			case 0x04: opsize = 3; break;
			case 0x05: opsize = 2; break;
			case 0x06: opsize = 1; signedness = 1; break;
			case 0x07: opsize = 0; signedness = 1; break;
			case 0x08: store = 1; opsize = 3; break;
			case 0x09: store = 1; opsize = 2; break;
			case 0x0a: store = 1; opsize = 1; break;
			case 0x0b: store = 1; opsize = 0; break;
			}

			if (opsize == 3 && d == 31) {
				fatal("m88k load/store of register pair r31/r0"
				    " is not yet implemented\n");
				goto bad;
			}

			ic->f = m88k_loadstore[ opsize
			    + (store? M88K_LOADSTORE_STORE : 0)
			    + (signedness? M88K_LOADSTORE_SIGNEDNESS:0)
			    + (cpu->byte_order == EMUL_BIG_ENDIAN?
			       M88K_LOADSTORE_ENDIANNESS : 0) ];

			if (!store && d == 0)
				ic->arg[0] = (size_t) &cpu->cd.m88k.zero_scratch;
		}
		break;

	case 0x10:	/*  and    imm  */
	case 0x11:	/*  and.u  imm  */
	case 0x12:	/*  mask   imm  */
	case 0x13:	/*  mask.u imm  */
	case 0x14:	/*  xor    imm  */
	case 0x15:	/*  xor.u  imm  */
	case 0x16:	/*  or     imm  */
	case 0x17:	/*  or.u   imm  */
	case 0x18:	/*  addu   imm  */
	case 0x19:	/*  subu   imm  */
	case 0x1a:	/*  divu   imm  */
	case 0x1b:	/*  mulu   imm  */
	case 0x1c:	/*  add    imm  */
	case 0x1d:	/*  sub    imm  */
	case 0x1e:	/*  div    imm  */
	case 0x1f:	/*  cmp    imm  */
		shift = 0;
		switch (op26) {
		case 0x10: ic->f = instr(and_imm); break;
		case 0x11: ic->f = instr(and_u_imm); shift = 16; break;
		case 0x12: ic->f = instr(mask_imm); break;
		case 0x13: ic->f = instr(mask_imm); shift = 16; break;
		case 0x14: ic->f = instr(xor_imm); break;
		case 0x15: ic->f = instr(xor_imm); shift = 16; break;
		case 0x16: ic->f = instr(or_imm); break;
		case 0x17: ic->f = instr(or_imm); shift = 16; break;
		case 0x18: ic->f = instr(addu_imm); break;
		case 0x19: ic->f = instr(subu_imm); break;
		case 0x1a: ic->f = instr(divu_imm); break;
		case 0x1b: ic->f = instr(mulu_imm); break;
		case 0x1c: ic->f = instr(add_imm); break;
		case 0x1d: ic->f = instr(sub_imm); break;
		case 0x1e: ic->f = instr(div_imm); break;
		case 0x1f: ic->f = instr(cmp_imm); break;
		}

		ic->arg[0] = (size_t) &cpu->cd.m88k.r[d];
		ic->arg[1] = (size_t) &cpu->cd.m88k.r[s1];
		ic->arg[2] = imm16 << shift;

		/*  Optimization for  or d,r0,imm  and similar  */
		if (s1 == M88K_ZERO_REG && ic->f == instr(or_imm)) {
			if (ic->arg[2] == 0)
				ic->f = instr(or_r0_imm0);
			else
				ic->f = instr(or_r0_imm);
		}
		if (ic->arg[2] == 0 && ic->f == instr(addu_imm))
			ic->f = instr(addu_s2r0);

		if (d == s1 && ic->arg[2] == 1) {
			if (ic->f == instr(addu_imm))
				ic->f = instr(inc_reg);
			if (ic->f == instr(subu_imm))
				ic->f = instr(dec_reg);
		}

		if (d == M88K_ZERO_REG)
			ic->f = instr(nop);
		break;

	case 0x20:
		if ((iword & 0x001ff81f) == 0x00004000) {
			ic->f = instr(ldcr);
			ic->arg[0] = (size_t) &cpu->cd.m88k.r[d];
			ic->arg[1] = cr6;
			if (d == M88K_ZERO_REG)
				ic->arg[0] = (size_t)
				    &cpu->cd.m88k.zero_scratch;
		} else if ((iword & 0x001ff81f) == 0x00004800) {
			ic->f = instr(fldcr);
			ic->arg[0] = (size_t) &cpu->cd.m88k.r[d];
			ic->arg[1] = cr6;
			if (d == M88K_ZERO_REG)
				ic->arg[0] = (size_t)
				    &cpu->cd.m88k.zero_scratch;
		} else if ((iword & 0x03e0f800) == 0x00008000) {
			ic->f = instr(stcr);
			ic->arg[0] = (size_t) &cpu->cd.m88k.r[s1];
			ic->arg[1] = cr6;
			if (s1 != s2)
				goto bad;
		} else if ((iword & 0x03e0f800) == 0x00008800) {
			ic->f = instr(fstcr);
			ic->arg[0] = (size_t) &cpu->cd.m88k.r[s1];
			ic->arg[1] = cr6;
			if (s1 != s2)
				goto bad;
		} else if ((iword & 0x0000f800) == 0x0000c000) {
			ic->f = instr(xcr);
			ic->arg[0] = (size_t) &cpu->cd.m88k.r[d];
			ic->arg[1] = (size_t) &cpu->cd.m88k.r[s1];
			ic->arg[2] = cr6;
			if (s1 != s2)
				goto bad;
		} else
			goto bad;
		break;

	case 0x21:
		switch (op11) {

		case 0x00:	/*  fmul  */
			if (d == 0) {
				/*  d = 0 isn't allowed. for now, let's abort execution.  */
				fatal("TODO: exception for d = 0 in fmul.xxx instruction\n");
				goto bad;
			}
			ic->arg[0] = (size_t) &cpu->cd.m88k.r[d];
			ic->arg[1] = (size_t) &cpu->cd.m88k.r[s1];
			ic->arg[2] = (size_t) &cpu->cd.m88k.r[s2];
			switch ((iword >> 5) & 0x3f) {
			case 0x00:	ic->f = instr(fmul_sss); break;
			case 0x01:	ic->f = instr(fmul_dss); break;
			case 0x05:	ic->f = instr(fmul_dsd); break;
			case 0x11:	ic->f = instr(fmul_dds); break;
			case 0x15:	ic->f = instr(fmul_ddd); break;
			default:if (!cpu->translation_readahead)
					fatal("Unimplemented fmul combination 0x%x.\n",
					    (iword >> 5) & 0x3f);
				goto bad;
			}
			break;

		case 0x04:	/*  flt  */
			if (d == 0) {
				/*  d = 0 isn't allowed. for now, let's abort execution.  */
				fatal("TODO: exception for d = 0 in flt.xx instruction\n");
				goto bad;
			}
			ic->arg[0] = (size_t) &cpu->cd.m88k.r[d];
			ic->arg[1] = (size_t) &cpu->cd.m88k.r[s2];
			if ((iword >> 5) & 1) {
				ic->f = instr(flt_ds);
				if (d & 1) {
					fatal("TODO: double precision load into uneven register r%i?\n", d);
					goto bad;
				}
			} else {
				ic->f = instr(flt_ss);
			}
			break;

		case 0x05:	/*  fadd  */
			if (d == 0) {
				/*  d = 0 isn't allowed. for now, let's abort execution.  */
				fatal("TODO: exception for d = 0 in fadd.xxx instruction\n");
				goto bad;
			}
			ic->arg[0] = (size_t) &cpu->cd.m88k.r[d];
			ic->arg[1] = (size_t) &cpu->cd.m88k.r[s1];
			ic->arg[2] = (size_t) &cpu->cd.m88k.r[s2];
			switch ((iword >> 5) & 0x3f) {
			case 0x00:	ic->f = instr(fadd_sss); break;
			case 0x05:	ic->f = instr(fadd_dsd); break;
			case 0x11:	ic->f = instr(fadd_dds); break;
			case 0x15:	ic->f = instr(fadd_ddd); break;
			default:if (!cpu->translation_readahead)
					fatal("Unimplemented fadd combination 0x%x.\n",
					    (iword >> 5) & 0x3f);
				goto bad;
			}
			break;

		case 0x06:	/*  fsub  */
			if (d == 0) {
				/*  d = 0 isn't allowed. for now, let's abort execution.  */
				fatal("TODO: exception for d = 0 in fsub.xxx instruction\n");
				goto bad;
			}
			ic->arg[0] = (size_t) &cpu->cd.m88k.r[d];
			ic->arg[1] = (size_t) &cpu->cd.m88k.r[s1];
			ic->arg[2] = (size_t) &cpu->cd.m88k.r[s2];
			switch ((iword >> 5) & 0x3f) {
			case 0x01:	ic->f = instr(fsub_dss); break;
			case 0x05:	ic->f = instr(fsub_dsd); break;
			case 0x10:	ic->f = instr(fsub_sds); break;
			case 0x11:	ic->f = instr(fsub_dds); break;
			case 0x15:	ic->f = instr(fsub_ddd); break;
			default:if (!cpu->translation_readahead)
					fatal("Unimplemented fsub combination 0x%x.\n",
					    (iword >> 5) & 0x3f);
				goto bad;
			}
			break;

		case 0x07:	/*  fcmp  */
			if (d == 0) {
				/*  d = 0 isn't allowed. for now, let's abort execution.  */
				fatal("TODO: exception for d = 0 in fcmp.xxx instruction\n");
				goto bad;
			}
			ic->arg[0] = (size_t) &cpu->cd.m88k.r[d];
			ic->arg[1] = (size_t) &cpu->cd.m88k.r[s1];
			ic->arg[2] = (size_t) &cpu->cd.m88k.r[s2];
			switch ((iword >> 5) & 0x3f) {
			case 0x10:	ic->f = instr(fcmp_sds); break;
			case 0x14:	ic->f = instr(fcmp_sdd); break;
			default:if (!cpu->translation_readahead)
					fatal("Unimplemented fcmp combination 0x%x.\n",
					    (iword >> 5) & 0x3f);
				goto bad;
			}
			break;

		case 0x0b:	/*  trnc  */
			if (d == 0) {
				/*  d = 0 isn't allowed. for now, let's abort execution.  */
				fatal("TODO: exception for d = 0 in trnc.xx instruction\n");
				goto bad;
			}
			ic->arg[0] = (size_t) &cpu->cd.m88k.r[d];
			ic->arg[1] = (size_t) &cpu->cd.m88k.r[s2];
			if ((iword >> 7) & 1) {
				ic->f = instr(trnc_sd);
				if (s2 & 1) {
					fatal("TODO: double precision truncation into uneven register r%i?\n", d);
					goto bad;
				}
			} else {
				ic->f = instr(trnc_ss);
			}
			break;

		case 0x0e:	/*  fdiv  */
			if (d == 0) {
				/*  d = 0 isn't allowed. for now, let's abort execution.  */
				fatal("TODO: exception for d = 0 in fdiv.xxx instruction\n");
				goto bad;
			}
			ic->arg[0] = (size_t) &cpu->cd.m88k.r[d];
			ic->arg[1] = (size_t) &cpu->cd.m88k.r[s1];
			ic->arg[2] = (size_t) &cpu->cd.m88k.r[s2];
			switch ((iword >> 5) & 0x3f) {
			case 0x00:	ic->f = instr(fdiv_sss); break;
			case 0x05:	ic->f = instr(fdiv_dsd); break;
			case 0x15:	ic->f = instr(fdiv_ddd); break;
			default:if (!cpu->translation_readahead)
					fatal("Unimplemented fdiv combination 0x%x.\n",
					    (iword >> 5) & 0x3f);
				goto bad;
			}
			break;

		default:goto bad;
		}
		break;

	case 0x30:	/*  br     */
	case 0x31:	/*  br.n   */
	case 0x32:	/*  bsr    */
	case 0x33:	/*  bsr.n  */
		switch (op26) {
		case 0x30:
			ic->f = instr(br);
			samepage_function = instr(br_samepage);
			if (cpu->translation_readahead > 1)
				cpu->translation_readahead = 1;
			break;
		case 0x31:
			ic->f = instr(br_n);
			if (cpu->translation_readahead > 2)
				cpu->translation_readahead = 2;
			break;
		case 0x32:
			ic->f = instr(bsr);
			samepage_function = instr(bsr_samepage);
			break;
		case 0x33:
			ic->f = instr(bsr_n);
			break;
		}

		offset = (addr & 0xffc) + d26;

		/*  Prepare both samepage and offset style args.
		    (Only one will be used in the actual instruction.)  */
		ic->arg[0] = (size_t) ( cpu->cd.m88k.cur_ic_page +
		    (offset >> M88K_INSTR_ALIGNMENT_SHIFT) );
		ic->arg[1] = offset;
		ic->arg[2] = (addr & 0xffc) + 4;    /*  Return offset
							for bsr_samepage  */

		if (offset >= 0 && offset <= 0xffc &&
		    samepage_function != NULL)
			ic->f = samepage_function;

		if (cpu->machine->show_trace_tree) {
			if (op26 == 0x32)
				ic->f = instr(bsr_trace);
			if (op26 == 0x33)
				ic->f = instr(bsr_n_trace);
		}

		break;

	case 0x34:	/*  bb0     */
	case 0x35:	/*  bb0.n   */
	case 0x36:	/*  bb1     */
	case 0x37:	/*  bb1.n   */
		switch (op26) {
		case 0x34:
			ic->f = instr(bb0);
			samepage_function = instr(bb0_samepage);
			break;
		case 0x35:
			ic->f = instr(bb0_n);
			break;
		case 0x36:
			ic->f = instr(bb1);
			samepage_function = instr(bb1_samepage);
			break;
		case 0x37:
			ic->f = instr(bb1_n);
			break;
		}

		ic->arg[0] = (size_t) &cpu->cd.m88k.r[s1];
		ic->arg[1] = (uint32_t) (1 << d);

		offset = (addr & 0xffc) + d16;
		ic->arg[2] = offset;

		if (offset >= 0 && offset <= 0xffc &&
		    samepage_function != NULL) {
			ic->f = samepage_function;
			ic->arg[2] = (size_t) ( cpu->cd.m88k.cur_ic_page +
			    (offset >> M88K_INSTR_ALIGNMENT_SHIFT) );
		}
		break;

	case 0x3a:	/*  bcnd    */
	case 0x3b:	/*  bcnd.n  */
		ic->f = m88k_bcnd[d + 32 * (op26 & 1)];
		samepage_function = m88k_bcnd[64 + d + 32 * (op26 & 1)];

		if (ic->f == NULL)
			goto bad;

		ic->arg[0] = (size_t) &cpu->cd.m88k.r[s1];

		offset = (addr & 0xffc) + d16;
		ic->arg[2] = offset;

		if (offset >= 0 && offset <= 0xffc &&
		    samepage_function != NULL) {
			ic->f = samepage_function;
			ic->arg[2] = (size_t) ( cpu->cd.m88k.cur_ic_page +
			    (offset >> M88K_INSTR_ALIGNMENT_SHIFT) );
		}

		if ((iword & 0xffe0ffff) == 0xe840ffff ||
		    (iword & 0xffe0ffff) == 0xe840fffe)
			cpu->cd.m88k.combination_check = COMBINE(idle);

		break;

	case 0x3c:
		switch (op10) {

		case 0x20:	/*  clr  */
		case 0x22:	/*  set  */
		case 0x24:	/*  ext  */
		case 0x26:	/*  extu  */
		case 0x28:	/*  mak  */
			ic->arg[0] = (size_t) &cpu->cd.m88k.r[d];
			ic->arg[1] = (size_t) &cpu->cd.m88k.r[s1];
			ic->arg[2] = iword & 0x3ff;

			switch (op10) {
			case 0x20: ic->f = instr(mask_imm);
				   {
					int w = ic->arg[2] >> 5;
					int o = ic->arg[2] & 0x1f;
					uint32_t x = w == 0? 0xffffffff
					    : ((uint32_t)1 << w) - 1;
					x <<= o;
					ic->arg[2] = ~x;
				   }
				   break;
			case 0x22: ic->f = instr(or_imm);
				   {
					int w = ic->arg[2] >> 5;
					int o = ic->arg[2] & 0x1f;
					uint32_t x = w == 0? 0xffffffff
					    : ((uint32_t)1 << w) - 1;
					x <<= o;
					ic->arg[2] = x;
				   }
				   break;
			case 0x24: ic->f = instr(ext_imm); break;
			case 0x26: ic->f = instr(extu_imm); break;
			case 0x28: ic->f = instr(mak_imm); break;
			}

			if (d == M88K_ZERO_REG)
				ic->f = instr(nop);
			break;

		case 0x34:	/*  tb0  */
		case 0x36:	/*  tb1  */
			ic->arg[0] = 1 << d;
			ic->arg[1] = (size_t) &cpu->cd.m88k.r[s1];
			ic->arg[2] = iword & 0x1ff;
			switch (op10) {
			case 0x34: ic->f = instr(tb0); break;
			case 0x36: ic->f = instr(tb1); break;
			}
			break;

		default:goto bad;
		}
		break;

	case 0x3d:
		if ((iword & 0xf000) <= 0x3fff ) {
			/*  Load, Store, xmem, and lda:  */
			int op = 0, opsize, user = 0, wt = 0;
			int signedness = 1, scaled = 0;

			switch (iword & 0xf000) {
			case 0x2000: op = 1; /* st */  break;
			case 0x3000: op = 2; /* lda */ break;
			default:     if ((iword & 0xf800) >= 0x0800)
					op = 0; /* ld */
				     else
					op = 3; /* xmem */
			}

			/*  for (most) ld, st, lda:  */
			opsize = (iword >> 10) & 3;

			/*  Turn opsize into x, where size = 1 << x:  */
			opsize = 3 - opsize;

			if (op == 3) {
				/*  xmem:  */
				switch ((iword >> 10) & 3) {
				case 0: opsize = 0; break;
				case 1: opsize = 2; break;
				default:fatal("Weird xmem opsize/type?\n");
					goto bad;
				}
			} else {
				if ((iword & 0xf800) == 0x800) {
					signedness = 0;
					if ((iword & 0xf00) < 0xc00)
						opsize = 1;
					else
						opsize = 0;
				} else {
					if (opsize >= 2 || op == 1)
						signedness = 0;
				}
			}

			if (iword & 0x100)
				user = 1;
			if (iword & 0x80)
				wt = 1;
			if (iword & 0x200)
				scaled = 1;

			if (wt) {
				fatal("wt bit not yet implemented! TODO\n");
				goto bad;
			}

			ic->arg[0] = (size_t) &cpu->cd.m88k.r[d];
			ic->arg[1] = (size_t) &cpu->cd.m88k.r[s1];
			ic->arg[2] = (size_t) &cpu->cd.m88k.r[s2];

			if (op == 0 || op == 1) {
				/*  ld or st:  */
				ic->f = m88k_loadstore[ opsize
				    + (op==1? M88K_LOADSTORE_STORE : 0)
				    + (signedness? M88K_LOADSTORE_SIGNEDNESS:0)
				    + (cpu->byte_order == EMUL_BIG_ENDIAN?
				       M88K_LOADSTORE_ENDIANNESS : 0)
				    + (scaled? M88K_LOADSTORE_SCALEDNESS : 0)
				    + (user? M88K_LOADSTORE_USR : 0)
				    + M88K_LOADSTORE_REGISTEROFFSET ];

				if (d == M88K_ZERO_REG && op == 0)
					ic->arg[0] = (size_t)&cpu->cd.m88k.zero_scratch;

				if (opsize == 3 && d == 31) {
					fatal("m88k load/store of register "
					    "pair r31/r0: TODO\n");
					goto bad;
				}
			} else if (op == 2) {
				/*  lda:  */
				if (scaled) {
					switch (opsize) {
					case 0: ic->f = instr(addu); break;
					case 1: ic->f = instr(lda_reg_2); break;
					case 2: ic->f = instr(lda_reg_4); break;
					case 3: ic->f = instr(lda_reg_8); break;
					}
				} else {
					ic->f = instr(addu);
				}
				if (d == M88K_ZERO_REG)
					ic->f = instr(nop);
			} else {
				/*  xmem:  */
				ic->f = instr(xmem_slow);
				ic->arg[0] = iword;
				if (d == M88K_ZERO_REG)
					ic->f = instr(nop);
			}
		} else switch ((iword >> 8) & 0xff) {
		case 0x40:	/*  and    */
		case 0x44:	/*  and.c  */
		case 0x50:	/*  xor    */
		case 0x54:	/*  xor.c  */
		case 0x58:	/*  or     */
		case 0x5c:	/*  or.c   */
		case 0x60:	/*  addu   */
		case 0x61:	/*  addu.co  */
		case 0x62:	/*  addu.ci  */
		case 0x64:	/*  subu   */
		case 0x65:	/*  subu.co  */
		case 0x66:	/*  subu.ci  */
		case 0x68:	/*  divu   */
		case 0x6c:	/*  mul    */
		case 0x70:	/*  add    */
		case 0x78:	/*  div    */
		case 0x7c:	/*  cmp    */
		case 0x80:	/*  clr    */
		case 0x88:	/*  set    */
		case 0x90:	/*  ext    */
		case 0x98:	/*  extu   */
		case 0xa0:	/*  mak    */
		case 0xa8:	/*  rot    */
			ic->arg[0] = (size_t) &cpu->cd.m88k.r[d];
			ic->arg[1] = (size_t) &cpu->cd.m88k.r[s1];
			ic->arg[2] = (size_t) &cpu->cd.m88k.r[s2];

			switch ((iword >> 8) & 0xff) {
			case 0x40: ic->f = instr(and);   break;
			case 0x44: ic->f = instr(and_c); break;
			case 0x50: ic->f = instr(xor);   break;
			case 0x54: ic->f = instr(xor_c); break;
			case 0x58: ic->f = instr(or);    break;
			case 0x5c: ic->f = instr(or_c);  break;
			case 0x60: ic->f = instr(addu);  break;
			case 0x61: ic->f = instr(addu_co); break;
			case 0x62: ic->f = instr(addu_ci); break;
			case 0x64: ic->f = instr(subu);  break;
			case 0x65: ic->f = instr(subu_co); break;
			case 0x66: ic->f = instr(subu_ci); break;
			case 0x68: ic->f = instr(divu);  break;
			case 0x6c: ic->f = instr(mul);   break;
			case 0x70: ic->f = instr(add);   break;
			case 0x78: ic->f = instr(div);   break;
			case 0x7c: ic->f = instr(cmp);   break;
			case 0x80: ic->f = instr(clr);   break;
			case 0x88: ic->f = instr(set);   break;
			case 0x90: ic->f = instr(ext);   break;
			case 0x98: ic->f = instr(extu);  break;
			case 0xa0: ic->f = instr(mak);   break;
			case 0xa8: ic->f = instr(rot);   break;
			}

			/*  Optimization for  or rX,r0,rY  etc:  */
			if (s1 == M88K_ZERO_REG && ic->f == instr(or))
				ic->f = instr(or_r0);
			if (s2 == M88K_ZERO_REG && ic->f == instr(addu))
				ic->f = instr(addu_s2r0);

			/*
			 *  Handle the case when the destination register is r0:
			 *
			 *  If there is NO SIDE-EFFECT! (i.e. no carry out),
			 *  then replace the instruction with a nop. If there is
			 *  a side-effect, we still have to run the instruction,
			 *  so replace the destination register with a scratch
			 *  register.
			 */
			if (d == M88K_ZERO_REG) {
				int opc = (iword >> 8) & 0xff;
				if (opc != 0x61 /* addu.co */ && opc != 0x63 /* addu.cio */ &&
				    opc != 0x65 /* subu.co */ && opc != 0x67 /* subu.cio */ &&
				    opc != 0x71 /*  add.co */ && opc != 0x73 /*  add.cio */ &&
				    opc != 0x75 /*  sub.co */ && opc != 0x77 /*  sub.cio */ &&
				    opc != 0x68 /*  divu   */ && opc != 0x69 /* divu.d   */ &&
				    opc != 0x6c /*  mul    */ && opc != 0x6d /* mulu.d   */ &&
				    opc != 0x6e /*  muls   */ && opc != 0x78 /*  div     */)
					ic->f = instr(nop);
				else
					ic->arg[0] = (size_t)
					    &cpu->cd.m88k.zero_scratch;
			}
			break;
		case 0xc0:	/*  jmp    */
		case 0xc4:	/*  jmp.n  */
		case 0xc8:	/*  jsr    */
		case 0xcc:	/*  jsr.n  */
			switch ((iword >> 8) & 0xff) {
			case 0xc0: ic->f = instr(jmp);
				   if (cpu->translation_readahead > 1)
					cpu->translation_readahead = 1;
				   break;
			case 0xc4: ic->f = instr(jmp_n);
				   if (cpu->translation_readahead > 2)
					cpu->translation_readahead = 2;
				   break;
			case 0xc8: ic->f = instr(jsr); break;
			case 0xcc: ic->f = instr(jsr_n); break;
			}

			ic->arg[1] = (addr & 0xffc) + 4;
			ic->arg[2] = (size_t) &cpu->cd.m88k.r[s2];

			if (((iword >> 8) & 0x04) == 0x04)
				ic->arg[1] = (addr & 0xffc) + 8;

			if (cpu->machine->show_trace_tree &&
			    s2 == M88K_RETURN_REG) {
				if (ic->f == instr(jmp))
					ic->f = instr(jmp_trace);
				if (ic->f == instr(jmp_n))
					ic->f = instr(jmp_n_trace);
			}
			if (cpu->machine->show_trace_tree) {
				if (ic->f == instr(jsr))
					ic->f = instr(jsr_trace);
				if (ic->f == instr(jsr_n))
					ic->f = instr(jsr_n_trace);
			}
			break;
		case 0xe8:	/*  ff1  */
		case 0xec:      /*  ff0  */
			switch ((iword >> 8) & 0xff) {
			case 0xe8: ic->f = instr(ff1); break;
			case 0xec: ic->f = instr(ff0); break;
			}

			ic->arg[0] = (size_t) &cpu->cd.m88k.r[d];
			ic->arg[2] = (size_t) &cpu->cd.m88k.r[s2];

			if (d == M88K_ZERO_REG)
				ic->f = instr(nop);
			break;
		case 0xfc:
			switch (iword & 0xff) {
			case 0x00:
				if (iword == 0xf400fc00)
					ic->f = instr(rte);
				else {
					fatal("unimplemented rte variant: 0x%08"PRIx32"\n", iword);
					goto bad;
				}
				break;
			case (M88K_PROM_INSTR & 0xff):
				ic->f = instr(prom_call);
				break;
			default:fatal("Unimplemented 3d/fc instruction\n");
				goto bad;
			}
			break;
		default:goto bad;
		}
		break;

	default:goto bad;
	}


#define	DYNTRANS_TO_BE_TRANSLATED_TAIL
#include "cpu_dyntrans.cc" 
#undef	DYNTRANS_TO_BE_TRANSLATED_TAIL
}

