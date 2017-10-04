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
 *  ARM Data Processing Instructions
 *  --------------------------------
 *
 *  xxxx000a aaaSnnnn ddddcccc ctttmmmm  Register form
 *  xxxx001a aaaSnnnn ddddrrrr bbbbbbbb  Immediate form
 *
 *  4 bits to select which instruction, one of the following:
 *
 *	0000	and		1000	tst
 *	0001	eor		1001	teq
 *	0010	sub		1010	cmp
 *	0011	rsb		1011	cmn
 *	0100	add		1100	orr
 *	0101	adc		1101	mov
 *	0110	sbc		1110	bic
 *	0111	rsc		1111	mvn
 *
 *  1 bit to select Status flag update.
 *
 *  1 bit to select Register form or Immediate form.
 *
 *  1 bit to select if the PC register is used.
 *
 *  Each function must also (as always) be repeated for each possible ARM
 *  condition code (15 in total).  Total: 1920 functions.
 *
 *  NOTE: This does not include any special common cases, which might be
 *        nice to have. Examples: comparing against zero, moving common
 *        constants.
 *
 *  See src/tools/generate_arm_dpi.c for more details.
 */


/*
 *  arg[0] = pointer to rn
 *  arg[1] = int32_t immediate value   OR  ptr to a reg_func() function
 *  arg[2] = pointer to rd
 */
void A__NAME(struct cpu *cpu, struct arm_instr_call *ic)
{
#if defined(A__RSB) || defined(A__RSC)
#define VAR_A  b
#define VAR_B  a
#else
#define VAR_A  a
#define VAR_B  b
#endif

#ifdef A__REG
	uint32_t (*reg_func)(struct cpu *, struct arm_instr_call *)
	    = (uint32_t (*)(struct cpu *, struct arm_instr_call *))
	      (void *)(size_t)ic->arg[1];
#endif

#ifdef A__S
	uint32_t c32;
#endif
#if defined(A__CMP) || defined(A__CMN) || defined(A__ADC) || defined(A__ADD) \
 || defined(A__RSC) || defined(A__RSC) || defined(A__SBC) || defined(A__SUB)
#ifdef A__S
	uint64_t
#else
	uint32_t
#endif
#else
	uint32_t
#endif
	    VAR_B =
#ifdef A__REG
	    reg_func(cpu, ic)
#else
#ifdef A__REGSHORT
	    reg(ic->arg[1])
#else
	    ic->arg[1]
#endif
#endif
	    , c64
#if !defined(A__MOV) && !defined(A__MVN)
	    , VAR_A = reg(ic->arg[0])
#endif
	    ;

#if defined(A__MOV) || defined(A__MVN) || defined(A__TST) || defined(A__TEQ) \
 || defined(A__AND) || defined(A__BIC) || defined(A__EOR) || defined(A__ORR)
#if !defined(A__REG) && defined(A__S)
	/*
	 *  TODO: This is not 100% correct, but should work with "recommended"
	 *  ARM code: Immediate values larger than 255 are encoded with
	 *  rotation. If the S-bit is set, then the carry bit is set to the
	 *  highest bit of the operand.
	 *
	 *  TODO 2: Perhaps this check should be moved out from here, and into
	 *  cpu_arm_instr.c. (More correct, and higher performance.)
	 */
	if (VAR_B > 255) {
		if (VAR_B & 0x80000000)
			cpu->cd.arm.flags |= ARM_F_C;
		else
			cpu->cd.arm.flags &= ~ARM_F_C;
	}
#endif
#endif


#if !defined(A__MOV) && !defined(A__MVN)
#ifdef A__PC
	if (ic->arg[0] == (size_t)&cpu->cd.arm.r[ARM_PC]) {
		uint32_t low_pc = ((size_t)ic - (size_t)
		    cpu->cd.arm.cur_ic_page) / sizeof(struct arm_instr_call);
		VAR_A = cpu->pc & ~((ARM_IC_ENTRIES_PER_PAGE-1)
		    << ARM_INSTR_ALIGNMENT_SHIFT);
		VAR_A += (low_pc << ARM_INSTR_ALIGNMENT_SHIFT) + 8;
	}
#endif
#endif

	/*
	 *  Perform the operation:
	 */
#if defined(A__AND) || defined(A__TST)
	c64 = a & b;
#endif
#if defined(A__EOR) || defined(A__TEQ)
	c64 = a ^ b;
#endif
#if defined(A__SUB) || defined(A__CMP) || defined(A__RSB)
	c64 = a - b;
#endif
#if defined(A__ADD) || defined(A__CMN)
	c64 = a + b;
#endif
#if defined(A__ADC)
	c64 = a + b + (cpu->cd.arm.flags & ARM_F_C? 1 : 0);
#endif
#if defined(A__SBC) || defined(A__RSC)
	b += (cpu->cd.arm.flags & ARM_F_C? 0 : 1);
	c64 = a - b;
#endif
#if defined(A__ORR)
	c64 = a | b;
#endif
#if defined(A__MOV)
	c64 = b;
#endif
#if defined(A__BIC)
	c64 = a & ~b;
#endif
#if defined(A__MVN)
	c64 = ~b;
#endif


#if defined(A__CMP) || defined(A__CMN) || defined(A__TST) || defined(A__TEQ)
	/*  No write to rd for compare/test.  */
#else
#ifdef A__PC
	if (ic->arg[2] == (size_t)&cpu->cd.arm.r[ARM_PC]) {
#ifndef A__S
		uint32_t old_pc = cpu->pc;
		uint32_t mask_within_page = ((ARM_IC_ENTRIES_PER_PAGE-1)
		    << ARM_INSTR_ALIGNMENT_SHIFT) |
		    ((1 << ARM_INSTR_ALIGNMENT_SHIFT) - 1);
#endif
		cpu->pc = (uint32_t)c64;
#ifdef A__S
		/*  Copy the right SPSR into CPSR:  */
		arm_save_register_bank(cpu);
		switch (cpu->cd.arm.cpsr & ARM_FLAG_MODE) {
		case ARM_MODE_FIQ32:
			cpu->cd.arm.cpsr = cpu->cd.arm.spsr_fiq; break;
		case ARM_MODE_IRQ32:
			cpu->cd.arm.cpsr = cpu->cd.arm.spsr_irq; break;
		case ARM_MODE_SVC32:
			cpu->cd.arm.cpsr = cpu->cd.arm.spsr_svc; break;
		case ARM_MODE_ABT32:
			cpu->cd.arm.cpsr = cpu->cd.arm.spsr_abt; break;
		case ARM_MODE_UND32:
			cpu->cd.arm.cpsr = cpu->cd.arm.spsr_und; break;
		}
		cpu->cd.arm.flags = cpu->cd.arm.cpsr >> 28;
		arm_load_register_bank(cpu);
#else
		if ((old_pc & ~mask_within_page) ==
		    ((uint32_t)cpu->pc & ~mask_within_page)) {
			cpu->cd.arm.next_ic = cpu->cd.arm.cur_ic_page +
			    ((cpu->pc & mask_within_page) >>
			    ARM_INSTR_ALIGNMENT_SHIFT);
		} else
#endif
			quick_pc_to_pointers(cpu);
		return;
	} else
		reg(ic->arg[2]) = c64;
#else
	reg(ic->arg[2]) = c64;
#endif
#endif


	/*
	 *  Status flag update (if the S-bit is set):
	 */
#ifdef A__S
	c32 = c64;
	cpu->cd.arm.flags
#if defined(A__CMP) || defined(A__CMN) || defined(A__ADC) || defined(A__ADD) \
 || defined(A__RSB) || defined(A__RSC) || defined(A__SBC) || defined(A__SUB)
	    = 0;
#else
	    &= ~(ARM_F_Z | ARM_F_N);
#endif

#if defined(A__CMP) || defined(A__RSB) || defined(A__SUB) || \
    defined(A__RSC) || defined(A__SBC)
	if ((uint32_t)a >= (uint32_t)b)
		cpu->cd.arm.flags |= ARM_F_C;
#else
#if defined(A__ADC) || defined(A__ADD) || defined(A__CMN)
	if (c32 != c64)
		cpu->cd.arm.flags |= ARM_F_C;
#endif
#endif

	if (c32 == 0)
		cpu->cd.arm.flags |= ARM_F_Z;

	if ((int32_t)c32 < 0)
		cpu->cd.arm.flags |= ARM_F_N;

	/*  Calculate the Overflow bit:  */
#if defined(A__CMP) || defined(A__CMN) || defined(A__ADC) || defined(A__ADD) \
 || defined(A__RSB) || defined(A__RSC) || defined(A__SBC) || defined(A__SUB)
	{
		int v = 0;
#if defined(A__ADD) || defined(A__CMN)
		if (((int32_t)a >= 0 && (int32_t)b >= 0 &&
		    (int32_t)c32 < 0) ||
		    ((int32_t)a < 0 && (int32_t)b < 0 &&
		    (int32_t)c32 >= 0))
			v = 1;
#else
#if defined(A__SUB) || defined(A__RSB) || defined(A__CMP) || \
    defined(A__RSC) || defined(A__SBC)
		if (((int32_t)a >= 0 && (int32_t)b < 0 &&
		    (int32_t)c32 < 0) ||
		    ((int32_t)a < 0 && (int32_t)b >= 0 &&
		    (int32_t)c32 >= 0))
			v = 1;
#endif
#endif
		if (v)
			cpu->cd.arm.flags |= ARM_F_V;
	}
#endif
#endif	/*  A__S  */

#undef VAR_A
#undef VAR_B
}


void A__NAME__eq(struct cpu *cpu, struct arm_instr_call *ic)
{ if (cpu->cd.arm.flags & ARM_F_Z) A__NAME(cpu, ic); }
void A__NAME__ne(struct cpu *cpu, struct arm_instr_call *ic)
{ if (!(cpu->cd.arm.flags & ARM_F_Z)) A__NAME(cpu, ic); }
void A__NAME__cs(struct cpu *cpu, struct arm_instr_call *ic)
{ if (cpu->cd.arm.flags & ARM_F_C) A__NAME(cpu, ic); }
void A__NAME__cc(struct cpu *cpu, struct arm_instr_call *ic)
{ if (!(cpu->cd.arm.flags & ARM_F_C)) A__NAME(cpu, ic); }
void A__NAME__mi(struct cpu *cpu, struct arm_instr_call *ic)
{ if (cpu->cd.arm.flags & ARM_F_N) A__NAME(cpu, ic); }
void A__NAME__pl(struct cpu *cpu, struct arm_instr_call *ic)
{ if (!(cpu->cd.arm.flags & ARM_F_N)) A__NAME(cpu, ic); }
void A__NAME__vs(struct cpu *cpu, struct arm_instr_call *ic)
{ if (cpu->cd.arm.flags & ARM_F_V) A__NAME(cpu, ic); }
void A__NAME__vc(struct cpu *cpu, struct arm_instr_call *ic)
{ if (!(cpu->cd.arm.flags & ARM_F_V)) A__NAME(cpu, ic); }

#ifndef BLAHURG
#define BLAHURG
extern uint8_t condition_hi[16];
extern uint8_t condition_ge[16];
extern uint8_t condition_gt[16];
#endif

void A__NAME__hi(struct cpu *cpu, struct arm_instr_call *ic)
{ if (condition_hi[cpu->cd.arm.flags]) A__NAME(cpu, ic); }
void A__NAME__ls(struct cpu *cpu, struct arm_instr_call *ic)
{ if (!condition_hi[cpu->cd.arm.flags]) A__NAME(cpu, ic); }
void A__NAME__ge(struct cpu *cpu, struct arm_instr_call *ic)
{ if (condition_ge[cpu->cd.arm.flags]) A__NAME(cpu, ic); }
void A__NAME__lt(struct cpu *cpu, struct arm_instr_call *ic)
{ if (!condition_ge[cpu->cd.arm.flags]) A__NAME(cpu, ic); }
void A__NAME__gt(struct cpu *cpu, struct arm_instr_call *ic)
{ if (condition_gt[cpu->cd.arm.flags]) A__NAME(cpu, ic); }
void A__NAME__le(struct cpu *cpu, struct arm_instr_call *ic)
{ if (!condition_gt[cpu->cd.arm.flags]) A__NAME(cpu, ic); }

