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
 *  ARM instructions.
 *
 *  Individual functions should keep track of cpu->n_translated_instrs.
 *  (If no instruction was executed, then it should be decreased. If, say, 4
 *  instructions were combined into one function and executed, then it should
 *  be increased by 3.)
 *
 *  Note: cpu->pc is prefered over r[ARM_PC]. r[ARM_PC] is only used in a
 *        few places, and should always be kept in synch with the real
 *        program counter.
 */


/*  #define GATHER_BDT_STATISTICS  */


#ifdef GATHER_BDT_STATISTICS
/*
 *  update_bdt_statistics():
 *
 *  Gathers statistics about load/store multiple instructions.
 *
 *  NOTE/TODO: Perhaps it would be more memory efficient to swap the high
 *  and low parts of the instruction word, so that the lllllll bits become
 *  the high bits; this would cause fewer host pages to be used. Anyway, the
 *  current implementation works on hosts with lots of RAM.
 *
 *  The resulting file, bdt_statistics.txt, should then be processed like
 *  this to give a new cpu_arm_multi.txt:
 *
 *  uniq -c bdt_statistics.txt|sort -nr|head -256|cut -f 2 > cpu_arm_multi.txt
 */
static void update_bdt_statistics(uint32_t iw)
{
	static FILE *f = NULL;
	static long long *counts;
	static char *counts_used;
	static long long n = 0;

	if (f == NULL) {
		size_t s = (1 << 24) * sizeof(long long);
		f = fopen("bdt_statistics.txt", "w");
		if (f == NULL) {
			fprintf(stderr, "update_bdt_statistics(): :-(\n");
			exit(1);
		}
		counts = zeroed_alloc(s);
		counts_used = zeroed_alloc(65536);
	}

	/*  Drop the s-bit: xxxx100P USWLnnnn llllllll llllllll  */
	iw = ((iw & 0x01800000) >> 1) | (iw & 0x003fffff);

	counts_used[iw & 0xffff] = 1;
	counts[iw] ++;

	n ++;
	if ((n % 500000) == 0) {
		int i;
		long long j;
		fatal("[ update_bdt_statistics(): n = %lli ]\n", (long long) n);
		fseek(f, 0, SEEK_SET);
		for (i=0; i<0x1000000; i++)
			if (counts_used[i & 0xffff] && counts[i] != 0) {
				/*  Recreate the opcode:  */
				uint32_t opcode = ((i & 0x00c00000) << 1)
				    | (i & 0x003fffff) | 0x08000000;
				for (j=0; j<counts[i]; j++)
					fprintf(f, "0x%08x\n", opcode);
			}
		fflush(f);
	}
}
#endif


/*****************************************************************************/


/*
 *  Helper definitions:
 *
 *  Each instruction is defined like this:
 *
 *	X(foo)
 *	{
 *		code for foo;
 *	}
 *	Y(foo)
 *
 *  The Y macro defines 14 copies of the instruction, one for each possible
 *  condition code. (The NV condition code is not included, and the AL code
 *  uses the main foo function.)  Y also defines an array with pointers to
 *  all of these functions.
 *
 *  If the compiler is good enough (i.e. allows long enough code sequences
 *  to be inlined), then the Y functions will be compiled as full (inlined)
 *  functions, otherwise they will simply call the X function.
 */

uint8_t condition_hi[16] = { 0,0,1,1, 0,0,0,0, 0,0,1,1, 0,0,0,0 };
uint8_t condition_ge[16] = { 1,0,1,0, 1,0,1,0, 0,1,0,1, 0,1,0,1 };
uint8_t condition_gt[16] = { 1,0,1,0, 0,0,0,0, 0,1,0,1, 0,0,0,0 };

#define Y(n) void arm_instr_ ## n ## __eq(struct cpu *cpu,		\
			struct arm_instr_call *ic)			\
	{  if (cpu->cd.arm.flags & ARM_F_Z)				\
		arm_instr_ ## n (cpu, ic);		}		\
	void arm_instr_ ## n ## __ne(struct cpu *cpu,			\
			struct arm_instr_call *ic)			\
	{  if (!(cpu->cd.arm.flags & ARM_F_Z))				\
		arm_instr_ ## n (cpu, ic);		}		\
	void arm_instr_ ## n ## __cs(struct cpu *cpu,			\
			struct arm_instr_call *ic)			\
	{  if (cpu->cd.arm.flags & ARM_F_C)				\
		arm_instr_ ## n (cpu, ic);		}		\
	void arm_instr_ ## n ## __cc(struct cpu *cpu,			\
			struct arm_instr_call *ic)			\
	{  if (!(cpu->cd.arm.flags & ARM_F_C))				\
		arm_instr_ ## n (cpu, ic);		}		\
	void arm_instr_ ## n ## __mi(struct cpu *cpu,			\
			struct arm_instr_call *ic)			\
	{  if (cpu->cd.arm.flags & ARM_F_N)				\
		arm_instr_ ## n (cpu, ic);		}		\
	void arm_instr_ ## n ## __pl(struct cpu *cpu,			\
			struct arm_instr_call *ic)			\
	{  if (!(cpu->cd.arm.flags & ARM_F_N))				\
		arm_instr_ ## n (cpu, ic);		}		\
	void arm_instr_ ## n ## __vs(struct cpu *cpu,			\
			struct arm_instr_call *ic)			\
	{  if (cpu->cd.arm.flags & ARM_F_V)				\
		arm_instr_ ## n (cpu, ic);		}		\
	void arm_instr_ ## n ## __vc(struct cpu *cpu,			\
			struct arm_instr_call *ic)			\
	{  if (!(cpu->cd.arm.flags & ARM_F_V))				\
		arm_instr_ ## n (cpu, ic);		}		\
	void arm_instr_ ## n ## __hi(struct cpu *cpu,			\
			struct arm_instr_call *ic)			\
	{  if (condition_hi[cpu->cd.arm.flags])				\
		arm_instr_ ## n (cpu, ic);		}		\
	void arm_instr_ ## n ## __ls(struct cpu *cpu,			\
			struct arm_instr_call *ic)			\
	{  if (!condition_hi[cpu->cd.arm.flags])			\
		arm_instr_ ## n (cpu, ic);		}		\
	void arm_instr_ ## n ## __ge(struct cpu *cpu,			\
			struct arm_instr_call *ic)			\
	{  if (condition_ge[cpu->cd.arm.flags])				\
		arm_instr_ ## n (cpu, ic);		}		\
	void arm_instr_ ## n ## __lt(struct cpu *cpu,			\
			struct arm_instr_call *ic)			\
	{  if (!condition_ge[cpu->cd.arm.flags])			\
		arm_instr_ ## n (cpu, ic);		}		\
	void arm_instr_ ## n ## __gt(struct cpu *cpu,			\
			struct arm_instr_call *ic)			\
	{  if (condition_gt[cpu->cd.arm.flags])				\
		arm_instr_ ## n (cpu, ic);		}		\
	void arm_instr_ ## n ## __le(struct cpu *cpu,			\
			struct arm_instr_call *ic)			\
	{  if (!condition_gt[cpu->cd.arm.flags])			\
		arm_instr_ ## n (cpu, ic);		}		\
	void (*arm_cond_instr_ ## n  [16])(struct cpu *,		\
			struct arm_instr_call *) = {			\
		arm_instr_ ## n ## __eq, arm_instr_ ## n ## __ne,	\
		arm_instr_ ## n ## __cs, arm_instr_ ## n ## __cc,	\
		arm_instr_ ## n ## __mi, arm_instr_ ## n ## __pl,	\
		arm_instr_ ## n ## __vs, arm_instr_ ## n ## __vc,	\
		arm_instr_ ## n ## __hi, arm_instr_ ## n ## __ls,	\
		arm_instr_ ## n ## __ge, arm_instr_ ## n ## __lt,	\
		arm_instr_ ## n ## __gt, arm_instr_ ## n ## __le,	\
		arm_instr_ ## n , arm_instr_nop };

#define cond_instr(n)	( arm_cond_instr_ ## n  [condition_code] )


/*****************************************************************************/


/*
 *  invalid:  Invalid instructions end up here.
 */
X(invalid) {
	uint32_t low_pc;
	low_pc = ((size_t)ic - (size_t)
	    cpu->cd.arm.cur_ic_page) / sizeof(struct arm_instr_call);
	cpu->pc &= ~((ARM_IC_ENTRIES_PER_PAGE-1)
	    << ARM_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << ARM_INSTR_ALIGNMENT_SHIFT);

	fatal("FATAL ERROR: An internal error occured in the ARM"
	    " dyntrans code. Please contact the author with detailed"
	    " repro steps on how to trigger this bug. pc = 0x%08"PRIx32"\n",
	    (uint32_t)cpu->pc);

	cpu->cd.arm.next_ic = &nothing_call;
}


/*
 *  nop:  Do nothing.
 */
X(nop)
{
}


/*
 *  b:  Branch (to a different translated page)
 *
 *  arg[0] = relative offset from start of page
 */
X(b)
{
	cpu->pc = (uint32_t)((cpu->pc & 0xfffff000) + (int32_t)ic->arg[0]);

	/*  Find the new physical page and update the translation pointers:  */
	quick_pc_to_pointers(cpu);
}
Y(b)


/*
 *  b_samepage:  Branch (to within the same translated page)
 *
 *  arg[0] = pointer to new arm_instr_call
 *  arg[1] = pointer to the next instruction.
 *
 *  NOTE: This instruction is manually inlined.
 */
X(b_samepage) {
	cpu->cd.arm.next_ic = (struct arm_instr_call *) ic->arg[0];
}
X(b_samepage__eq) {
	cpu->cd.arm.next_ic = (struct arm_instr_call *)
	    ic->arg[cpu->cd.arm.flags & ARM_F_Z? 0 : 1];
}
X(b_samepage__ne) {
	cpu->cd.arm.next_ic = (struct arm_instr_call *)
	    ic->arg[cpu->cd.arm.flags & ARM_F_Z? 1 : 0];
}
X(b_samepage__cs) {
	cpu->cd.arm.next_ic = (struct arm_instr_call *)
	    ic->arg[cpu->cd.arm.flags & ARM_F_C? 0 : 1];
}
X(b_samepage__cc) {
	cpu->cd.arm.next_ic = (struct arm_instr_call *)
	    ic->arg[cpu->cd.arm.flags & ARM_F_C? 1 : 0];
}
X(b_samepage__mi) {
	cpu->cd.arm.next_ic = (struct arm_instr_call *)
	    ic->arg[cpu->cd.arm.flags & ARM_F_N? 0 : 1];
}
X(b_samepage__pl) {
	cpu->cd.arm.next_ic = (struct arm_instr_call *)
	    ic->arg[cpu->cd.arm.flags & ARM_F_N? 1 : 0];
}
X(b_samepage__vs) {
	cpu->cd.arm.next_ic = (struct arm_instr_call *)
	    ic->arg[cpu->cd.arm.flags & ARM_F_V? 0 : 1];
}
X(b_samepage__vc) {
	cpu->cd.arm.next_ic = (struct arm_instr_call *)
	    ic->arg[cpu->cd.arm.flags & ARM_F_V? 1 : 0];
}
X(b_samepage__hi) {
	cpu->cd.arm.next_ic = (condition_hi[cpu->cd.arm.flags])?
	    (struct arm_instr_call *) ic->arg[0] :
	    (struct arm_instr_call *) ic->arg[1];
}
X(b_samepage__ls) {
	cpu->cd.arm.next_ic = (struct arm_instr_call *)
	    ic->arg[condition_hi[cpu->cd.arm.flags]];
}
X(b_samepage__ge) {
	cpu->cd.arm.next_ic = (condition_ge[cpu->cd.arm.flags])?
	    (struct arm_instr_call *) ic->arg[0] :
	    (struct arm_instr_call *) ic->arg[1];
}
X(b_samepage__lt) {
	cpu->cd.arm.next_ic = (struct arm_instr_call *)
	    ic->arg[condition_ge[cpu->cd.arm.flags]];
}
X(b_samepage__gt) {
	cpu->cd.arm.next_ic = (condition_gt[cpu->cd.arm.flags])?
	    (struct arm_instr_call *) ic->arg[0] :
	    (struct arm_instr_call *) ic->arg[1];
}
X(b_samepage__le) {
	cpu->cd.arm.next_ic = (struct arm_instr_call *)
	    ic->arg[condition_gt[cpu->cd.arm.flags]];
}
void (*arm_cond_instr_b_samepage[16])(struct cpu *,
	struct arm_instr_call *) = {
	arm_instr_b_samepage__eq, arm_instr_b_samepage__ne,
	arm_instr_b_samepage__cs, arm_instr_b_samepage__cc,
	arm_instr_b_samepage__mi, arm_instr_b_samepage__pl,
	arm_instr_b_samepage__vs, arm_instr_b_samepage__vc,
	arm_instr_b_samepage__hi, arm_instr_b_samepage__ls,
	arm_instr_b_samepage__ge, arm_instr_b_samepage__lt,
	arm_instr_b_samepage__gt, arm_instr_b_samepage__le,
	arm_instr_b_samepage, arm_instr_nop };


/*
 *  bx:  Branch, potentially exchanging Thumb/ARM encoding
 *
 *  arg[0] = ptr to rm
 */
X(bx)
{
	cpu->pc = reg(ic->arg[0]);
	if (cpu->pc & 1) {
		fatal("thumb: TODO\n");
		exit(1);
	}
	cpu->pc &= ~3;

	/*  Find the new physical page and update the translation pointers:  */
	quick_pc_to_pointers(cpu);
}
Y(bx)


/*
 *  bx_trace:  As bx, but with trace enabled, arg[0] = the link register.
 *
 *  arg[0] = ignored
 */
X(bx_trace)
{
	cpu->pc = cpu->cd.arm.r[ARM_LR];
	if (cpu->pc & 1) {
		fatal("thumb: TODO\n");
		exit(1);
	}
	cpu->pc &= ~3;

	cpu_functioncall_trace_return(cpu);

	/*  Find the new physical page and update the translation pointers:  */
	quick_pc_to_pointers(cpu);
}
Y(bx_trace)


/*
 *  bl:  Branch and Link (to a different translated page)
 *
 *  arg[0] = relative address
 */
X(bl)
{
	uint32_t pc = ((uint32_t)cpu->pc & 0xfffff000) + (int32_t)ic->arg[1];
	cpu->cd.arm.r[ARM_LR] = pc + 4;

	/*  Calculate new PC from this instruction + arg[0]  */
	cpu->pc = pc + (int32_t)ic->arg[0];

	/*  Find the new physical page and update the translation pointers:  */
	quick_pc_to_pointers(cpu);
}
Y(bl)


/*
 *  blx:  Branch and Link, potentially exchanging Thumb/ARM encoding
 *
 *  arg[0] = ptr to rm
 */
X(blx)
{
	uint32_t lr = ((uint32_t)cpu->pc & 0xfffff000) + (int32_t)ic->arg[2];
	cpu->cd.arm.r[ARM_LR] = lr;
	cpu->pc = reg(ic->arg[0]);
	if (cpu->pc & 1) {
		fatal("thumb: TODO\n");
		exit(1);
	}
	cpu->pc &= ~3;

	/*  Find the new physical page and update the translation pointers:  */
	quick_pc_to_pointers(cpu);
}
Y(blx)


/*
 *  bl_trace:  Branch and Link (to a different translated page), with trace
 *
 *  Same as for bl.
 */
X(bl_trace)
{
	uint32_t pc = ((uint32_t)cpu->pc & 0xfffff000) + (int32_t)ic->arg[1];
	cpu->cd.arm.r[ARM_LR] = pc + 4;

	/*  Calculate new PC from this instruction + arg[0]  */
	cpu->pc = pc + (int32_t)ic->arg[0];

	cpu_functioncall_trace(cpu, cpu->pc);

	/*  Find the new physical page and update the translation pointers:  */
	quick_pc_to_pointers(cpu);
}
Y(bl_trace)


/*
 *  bl_samepage:  A branch + link within the same page
 *
 *  arg[0] = pointer to new arm_instr_call
 */
X(bl_samepage)
{
	cpu->cd.arm.r[ARM_LR] =
	    ((uint32_t)cpu->pc & 0xfffff000) + (int32_t)ic->arg[2];
	cpu->cd.arm.next_ic = (struct arm_instr_call *) ic->arg[0];
}
Y(bl_samepage)


/*
 *  bl_samepage_trace:  Branch and Link (to the same page), with trace
 *
 *  Same as for bl_samepage.
 */
X(bl_samepage_trace)
{
	uint32_t low_pc, lr = (cpu->pc & 0xfffff000) + ic->arg[2];

	/*  Link and branch:  */
	cpu->cd.arm.r[ARM_LR] = lr;
	cpu->cd.arm.next_ic = (struct arm_instr_call *) ic->arg[0];

	/*  Synchronize the program counter:  */
	low_pc = ((size_t)cpu->cd.arm.next_ic - (size_t)
	    cpu->cd.arm.cur_ic_page) / sizeof(struct arm_instr_call);
	cpu->pc &= ~((ARM_IC_ENTRIES_PER_PAGE-1)
	    << ARM_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << ARM_INSTR_ALIGNMENT_SHIFT);

	/*  ... and show trace:  */
	cpu_functioncall_trace(cpu, cpu->pc);
}
Y(bl_samepage_trace)


/*
 *  clz: Count leading zeroes.
 *
 *  arg[0] = ptr to rm
 *  arg[1] = ptr to rd
 */
X(clz)
{
	uint32_t rm = reg(ic->arg[0]);
	int i = 32, n = 0, j;
	while (i>0) {
		if (rm & 0xff000000) {
			for (j=0; j<8; j++) {
				if (rm & 0x80000000)
					break;
				n ++;
				rm <<= 1;
			}
			break;
		} else {
			rm <<= 8;
			i -= 8;
			n += 8;
		}
	}
	reg(ic->arg[1]) = n;
}
Y(clz)


/*
 *  mul: Multiplication
 *
 *  arg[0] = ptr to rd
 *  arg[1] = ptr to rm
 *  arg[2] = ptr to rs
 */
X(mul)
{
	reg(ic->arg[0]) = reg(ic->arg[1]) * reg(ic->arg[2]);
}
Y(mul)
X(muls)
{
	uint32_t result;
	result = reg(ic->arg[1]) * reg(ic->arg[2]);
	cpu->cd.arm.flags &= ~(ARM_F_Z | ARM_F_N);
	if (result == 0)
		cpu->cd.arm.flags |= ARM_F_Z;
	if (result & 0x80000000)
		cpu->cd.arm.flags |= ARM_F_N;
	reg(ic->arg[0]) = result;
}
Y(muls)


/*
 *  mla: Multiplication with addition
 *
 *  arg[0] = copy of instruction word
 */
X(mla)
{
	/*  xxxx0000 00ASdddd nnnnssss 1001mmmm (Rd,Rm,Rs[,Rn])  */
	uint32_t iw = ic->arg[0];
	int rd, rs, rn, rm;
	rd = (iw >> 16) & 15; rn = (iw >> 12) & 15,
	rs = (iw >> 8) & 15;  rm = iw & 15;
	cpu->cd.arm.r[rd] = cpu->cd.arm.r[rm] * cpu->cd.arm.r[rs]
	    + cpu->cd.arm.r[rn];
}
Y(mla)
X(mlas)
{
	/*  xxxx0000 00ASdddd nnnnssss 1001mmmm (Rd,Rm,Rs[,Rn])  */
	uint32_t iw = ic->arg[0];
	int rd, rs, rn, rm;
	rd = (iw >> 16) & 15; rn = (iw >> 12) & 15,
	rs = (iw >> 8) & 15;  rm = iw & 15;
	cpu->cd.arm.r[rd] = cpu->cd.arm.r[rm] * cpu->cd.arm.r[rs]
	    + cpu->cd.arm.r[rn];
	cpu->cd.arm.flags &= ~(ARM_F_Z | ARM_F_N);
	if (cpu->cd.arm.r[rd] == 0)
		cpu->cd.arm.flags |= ARM_F_Z;
	if (cpu->cd.arm.r[rd] & 0x80000000)
		cpu->cd.arm.flags |= ARM_F_N;
}
Y(mlas)


/*
 *  mull: Long multiplication
 *
 *  arg[0] = copy of instruction word
 */
X(mull)
{
	/*  xxxx0000 1UAShhhh llllssss 1001mmmm  */
	uint32_t iw; uint64_t tmp; int u_bit, a_bit;
	iw = ic->arg[0];
	u_bit = iw & 0x00400000; a_bit = iw & 0x00200000;
	tmp = cpu->cd.arm.r[iw & 15];
	if (u_bit)
		tmp = (int64_t)(int32_t)tmp
		    * (int64_t)(int32_t)cpu->cd.arm.r[(iw >> 8) & 15];
	else
		tmp *= (uint64_t)cpu->cd.arm.r[(iw >> 8) & 15];
	if (a_bit) {
		uint64_t x = ((uint64_t)cpu->cd.arm.r[(iw >> 16) & 15] << 32)
		    | cpu->cd.arm.r[(iw >> 12) & 15];
		x += tmp;
		cpu->cd.arm.r[(iw >> 16) & 15] = (x >> 32);
		cpu->cd.arm.r[(iw >> 12) & 15] = x;
	} else {
		cpu->cd.arm.r[(iw >> 16) & 15] = (tmp >> 32);
		cpu->cd.arm.r[(iw >> 12) & 15] = tmp;
	}
}
Y(mull)


/*
 *  smulXY:  16-bit * 16-bit multiplication (32-bit result)
 *
 *  arg[0] = ptr to rm
 *  arg[1] = ptr to rs
 *  arg[2] = ptr to rd
 */
X(smulbb)
{
	reg(ic->arg[2]) = (int32_t)(int16_t)reg(ic->arg[0]) *
	    (int32_t)(int16_t)reg(ic->arg[1]);
}
Y(smulbb)
X(smultb)
{
	reg(ic->arg[2]) = (int32_t)(int16_t)(reg(ic->arg[0]) >> 16) *
	    (int32_t)(int16_t)reg(ic->arg[1]);
}
Y(smultb)
X(smulbt)
{
	reg(ic->arg[2]) = (int32_t)(int16_t)reg(ic->arg[0]) *
	    (int32_t)(int16_t)(reg(ic->arg[1]) >> 16);
}
Y(smulbt)
X(smultt)
{
	reg(ic->arg[2]) = (int32_t)(int16_t)(reg(ic->arg[0]) >> 16) *
	    (int32_t)(int16_t)(reg(ic->arg[1]) >> 16);
}
Y(smultt)


/*
 *  mov_reg_reg:  Move a register to another.
 *
 *  arg[0] = ptr to source register
 *  arg[1] = ptr to destination register
 */
X(mov_reg_reg)
{
	reg(ic->arg[1]) = reg(ic->arg[0]);
}
Y(mov_reg_reg)


/*
 *  mov_reg_pc:  Move the PC register to a normal register.
 *
 *  arg[0] = offset compared to start of current page + 8
 *  arg[1] = ptr to destination register
 */
X(mov_reg_pc)
{
	reg(ic->arg[1]) = ((uint32_t)cpu->pc&0xfffff000) + (int32_t)ic->arg[0];
}
Y(mov_reg_pc)


/*
 *  ret_trace:  "mov pc,lr" with trace enabled
 *  ret:  "mov pc,lr" without trace enabled
 *
 *  arg[0] = ignored
 */
X(ret_trace)
{
	uint32_t old_pc, mask_within_page;
	old_pc = cpu->pc;
	mask_within_page = ((ARM_IC_ENTRIES_PER_PAGE-1)
	    << ARM_INSTR_ALIGNMENT_SHIFT) |
	    ((1 << ARM_INSTR_ALIGNMENT_SHIFT) - 1);

	/*  Update the PC register:  */
	cpu->pc = cpu->cd.arm.r[ARM_LR];

	cpu_functioncall_trace_return(cpu);

	/*
	 *  Is this a return to code within the same page? Then there is no
	 *  need to update all pointers, just next_ic.
	 */
	if ((old_pc & ~mask_within_page) == (cpu->pc & ~mask_within_page)) {
		cpu->cd.arm.next_ic = cpu->cd.arm.cur_ic_page +
		    ((cpu->pc & mask_within_page) >> ARM_INSTR_ALIGNMENT_SHIFT);
	} else {
		/*  Find the new physical page and update pointers:  */
		quick_pc_to_pointers(cpu);
	}
}
Y(ret_trace)
X(ret)
{
	cpu->pc = cpu->cd.arm.r[ARM_LR];
	quick_pc_to_pointers(cpu);
}
Y(ret)


/*
 *  msr: Move to status register from a normal register or immediate value.
 *
 *  arg[0] = immediate value
 *  arg[1] = mask
 *  arg[2] = pointer to rm
 *
 *  msr_imm and msr_imm_spsr use arg[1] and arg[0].
 *  msr and msr_spsr use arg[1] and arg[2].
 */
X(msr_imm)
{
	uint32_t mask = ic->arg[1];
	int switch_register_banks = (mask & ARM_FLAG_MODE) &&
	    ((cpu->cd.arm.cpsr & ARM_FLAG_MODE) !=
	    (ic->arg[0] & ARM_FLAG_MODE));
	uint32_t new_value = ic->arg[0];

	cpu->cd.arm.cpsr &= 0x0fffffff;
	cpu->cd.arm.cpsr |= (cpu->cd.arm.flags << 28);

	if (switch_register_banks)
		arm_save_register_bank(cpu);

	cpu->cd.arm.cpsr &= ~mask;
	cpu->cd.arm.cpsr |= (new_value & mask);

	cpu->cd.arm.flags = cpu->cd.arm.cpsr >> 28;

	if (switch_register_banks)
		arm_load_register_bank(cpu);
}
Y(msr_imm)
X(msr)
{
	ic->arg[0] = reg(ic->arg[2]);
	instr(msr_imm)(cpu, ic);
}
Y(msr)
X(msr_imm_spsr)
{
	uint32_t mask = ic->arg[1];
	uint32_t new_value = ic->arg[0];
	switch (cpu->cd.arm.cpsr & ARM_FLAG_MODE) {
	case ARM_MODE_FIQ32:
		cpu->cd.arm.spsr_fiq &= ~mask;
		cpu->cd.arm.spsr_fiq |= (new_value & mask);
		break;
	case ARM_MODE_ABT32:
		cpu->cd.arm.spsr_abt &= ~mask;
		cpu->cd.arm.spsr_abt |= (new_value & mask);
		break;
	case ARM_MODE_UND32:
		cpu->cd.arm.spsr_und &= ~mask;
		cpu->cd.arm.spsr_und |= (new_value & mask);
		break;
	case ARM_MODE_IRQ32:
		cpu->cd.arm.spsr_irq &= ~mask;
		cpu->cd.arm.spsr_irq |= (new_value & mask);
		break;
	case ARM_MODE_SVC32:
		cpu->cd.arm.spsr_svc &= ~mask;
		cpu->cd.arm.spsr_svc |= (new_value & mask);
		break;
	default:fatal("msr_spsr: unimplemented mode %i\n",
		    cpu->cd.arm.cpsr & ARM_FLAG_MODE);
{
	/*  Synchronize the program counter:  */
	uint32_t old_pc, low_pc = ((size_t)ic - (size_t)
	    cpu->cd.arm.cur_ic_page) / sizeof(struct arm_instr_call);
	cpu->pc &= ~((ARM_IC_ENTRIES_PER_PAGE-1) << ARM_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << ARM_INSTR_ALIGNMENT_SHIFT);
	old_pc = cpu->pc;
	printf("msr_spsr: old pc = 0x%08"PRIx32"\n", old_pc);
}
		exit(1);
	}
}
Y(msr_imm_spsr)
X(msr_spsr)
{
	ic->arg[0] = reg(ic->arg[2]);
	instr(msr_imm_spsr)(cpu, ic);
}
Y(msr_spsr)


/*
 *  mrs: Move from status/flag register to a normal register.
 *
 *  arg[0] = pointer to rd
 */
X(mrs)
{
	cpu->cd.arm.cpsr &= 0x0fffffff;
	cpu->cd.arm.cpsr |= (cpu->cd.arm.flags << 28);
	reg(ic->arg[0]) = cpu->cd.arm.cpsr;
}
Y(mrs)


/*
 *  mrs: Move from saved status/flag register to a normal register.
 *
 *  arg[0] = pointer to rd
 */
X(mrs_spsr)
{
	switch (cpu->cd.arm.cpsr & ARM_FLAG_MODE) {
	case ARM_MODE_FIQ32: reg(ic->arg[0]) = cpu->cd.arm.spsr_fiq; break;
	case ARM_MODE_ABT32: reg(ic->arg[0]) = cpu->cd.arm.spsr_abt; break;
	case ARM_MODE_UND32: reg(ic->arg[0]) = cpu->cd.arm.spsr_und; break;
	case ARM_MODE_IRQ32: reg(ic->arg[0]) = cpu->cd.arm.spsr_irq; break;
	case ARM_MODE_SVC32: reg(ic->arg[0]) = cpu->cd.arm.spsr_svc; break;
	case ARM_MODE_USR32:
	case ARM_MODE_SYS32: reg(ic->arg[0]) = 0; break;
	default:fatal("mrs_spsr: unimplemented mode %i\n",
		    cpu->cd.arm.cpsr & ARM_FLAG_MODE);
		exit(1);
	}
}
Y(mrs_spsr)


/*
 *  mcr_mrc:  Coprocessor move
 *  cdp:      Coprocessor operation
 *
 *  arg[0] = copy of the instruction word
 */
X(mcr_mrc) {
	uint32_t low_pc = ((size_t)ic - (size_t)
	    cpu->cd.arm.cur_ic_page) / sizeof(struct arm_instr_call);
	cpu->pc &= ~((ARM_IC_ENTRIES_PER_PAGE-1) << ARM_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << ARM_INSTR_ALIGNMENT_SHIFT);
	arm_mcr_mrc(cpu, ic->arg[0]);
}
Y(mcr_mrc)
X(cdp) {
	uint32_t low_pc = ((size_t)ic - (size_t)
	    cpu->cd.arm.cur_ic_page) / sizeof(struct arm_instr_call);
	cpu->pc &= ~((ARM_IC_ENTRIES_PER_PAGE-1) << ARM_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << ARM_INSTR_ALIGNMENT_SHIFT);
	arm_cdp(cpu, ic->arg[0]);
}
Y(cdp)


/*
 *  openfirmware:
 */
X(openfirmware)
{
	/*  TODO: sync pc?  */
	of_emul(cpu);
	cpu->pc = cpu->cd.arm.r[ARM_LR];
	if (cpu->machine->show_trace_tree)
		cpu_functioncall_trace_return(cpu);
	quick_pc_to_pointers(cpu);
}


/*
 *  reboot:
 */
X(reboot)
{
	cpu->running = 0;
	cpu->n_translated_instrs --;
	cpu->cd.arm.next_ic = &nothing_call;
}


/*
 *  swi:  Software interrupt.
 */
X(swi)
{
	/*  Synchronize the program counter first:  */
	cpu->pc &= 0xfffff000;
	cpu->pc += ic->arg[0];
	arm_exception(cpu, ARM_EXCEPTION_SWI);
}
Y(swi)


/*
 * bkpt:  Breakpoint instruction.
 */
X(bkpt)
{
	/*  Synchronize the program counter first:  */
	cpu->pc &= 0xfffff000;
	cpu->pc += ic->arg[0];
	arm_exception(cpu, ARM_EXCEPTION_PREF_ABT);
}
Y(bkpt)


/*
 *  und:  Undefined instruction.
 */
X(und)
{
	/*  Synchronize the program counter first:  */
	cpu->pc &= 0xfffff000;
	cpu->pc += ic->arg[0];
	arm_exception(cpu, ARM_EXCEPTION_UND);
}
Y(und)


/*
 *  swp, swpb:  Swap (word or byte).
 *
 *  arg[0] = ptr to rd
 *  arg[1] = ptr to rm
 *  arg[2] = ptr to rn
 */
X(swp)
{
	uint32_t addr = reg(ic->arg[2]), data, data2;
	unsigned char d[4];

	/*  Synchronize the program counter:  */
	uint32_t low_pc = ((size_t)ic - (size_t)
	    cpu->cd.arm.cur_ic_page) / sizeof(struct arm_instr_call);
	cpu->pc &= ~((ARM_IC_ENTRIES_PER_PAGE-1) << ARM_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << ARM_INSTR_ALIGNMENT_SHIFT);

	if (!cpu->memory_rw(cpu, cpu->mem, addr, d, sizeof(d), MEM_READ,
	    CACHE_DATA)) {
		fatal("swp: load failed\n");
		return;
	}
	data = d[0] + (d[1] << 8) + (d[2] << 16) + (d[3] << 24);
	data2 = reg(ic->arg[1]);
	d[0] = data2; d[1] = data2 >> 8; d[2] = data2 >> 16; d[3] = data2 >> 24;
	if (!cpu->memory_rw(cpu, cpu->mem, addr, d, sizeof(d), MEM_WRITE,
	    CACHE_DATA)) {
		fatal("swp: store failed\n");
		return;
	}
	reg(ic->arg[0]) = data;
}
Y(swp)
X(swpb)
{
	uint32_t addr = reg(ic->arg[2]), data;
	unsigned char d[1];

	/*  Synchronize the program counter:  */
	uint32_t low_pc = ((size_t)ic - (size_t)
	    cpu->cd.arm.cur_ic_page) / sizeof(struct arm_instr_call);
	cpu->pc &= ~((ARM_IC_ENTRIES_PER_PAGE-1) << ARM_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << ARM_INSTR_ALIGNMENT_SHIFT);

	if (!cpu->memory_rw(cpu, cpu->mem, addr, d, sizeof(d), MEM_READ,
	    CACHE_DATA)) {
		fatal("swp: load failed\n");
		return;
	}
	data = d[0];
	d[0] = reg(ic->arg[1]);
	if (!cpu->memory_rw(cpu, cpu->mem, addr, d, sizeof(d), MEM_WRITE,
	    CACHE_DATA)) {
		fatal("swp: store failed\n");
		return;
	}
	reg(ic->arg[0]) = data;
}
Y(swpb)


extern void (*arm_load_store_instr[1024])(struct cpu *,
	struct arm_instr_call *);
X(store_w1_word_u1_p0_imm);
X(store_w0_byte_u1_p0_imm);
X(store_w0_word_u1_p0_imm);
X(store_w0_word_u1_p1_imm);
X(load_w0_word_u1_p0_imm);
X(load_w0_word_u1_p1_imm);
X(load_w1_word_u1_p0_imm);
X(load_w0_byte_u1_p1_imm);
X(load_w0_byte_u1_p1_reg);
X(load_w1_byte_u1_p1_imm);

extern void (*arm_load_store_instr_pc[1024])(struct cpu *,
	struct arm_instr_call *);

extern void (*arm_load_store_instr_3[2048])(struct cpu *,
	struct arm_instr_call *);

extern void (*arm_load_store_instr_3_pc[2048])(struct cpu *,
	struct arm_instr_call *);

extern uint32_t (*arm_r[8192])(struct cpu *, struct arm_instr_call *);
extern uint32_t arm_r_r3_t0_c0(struct cpu *cpu, struct arm_instr_call *ic);

extern void (*arm_dpi_instr[2 * 2 * 2 * 16 * 16])(struct cpu *,
	struct arm_instr_call *);
extern void (*arm_dpi_instr_regshort[2 * 16 * 16])(struct cpu *,
	struct arm_instr_call *);
X(cmps);
X(teqs);
X(tsts);
X(sub);
X(add);
X(subs);
X(eor_regshort);
X(cmps_regshort);


#include "cpu_arm_instr_misc.cc"


/*
 *  bdt_load:  Block Data Transfer, Load
 *
 *  arg[0] = pointer to uint32_t in host memory, pointing to the base register
 *  arg[1] = 32-bit instruction word. Most bits are read from this.
 */
X(bdt_load)
{
	unsigned char data[4];
	uint32_t *np = (uint32_t *)ic->arg[0];
	uint32_t addr = *np, low_pc;
	unsigned char *page;
	uint32_t iw = ic->arg[1];  /*  xxxx100P USWLnnnn llllllll llllllll  */
	int p_bit = iw & 0x01000000;
	int u_bit = iw & 0x00800000;
	int s_bit = iw & 0x00400000;
	int w_bit = iw & 0x00200000;
	int i, return_flag = 0;
	uint32_t new_values[16];

#ifdef GATHER_BDT_STATISTICS
	if (!s_bit)
		update_bdt_statistics(iw);
#endif

	/*  Synchronize the program counter:  */
	low_pc = ((size_t)ic - (size_t)
	    cpu->cd.arm.cur_ic_page) / sizeof(struct arm_instr_call);
	cpu->pc &= ~((ARM_IC_ENTRIES_PER_PAGE-1) << ARM_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << ARM_INSTR_ALIGNMENT_SHIFT);

	if (s_bit) {
		/*  Load to USR registers:  */
		if ((cpu->cd.arm.cpsr & ARM_FLAG_MODE) == ARM_MODE_USR32) {
			fatal("[ bdt_load: s-bit: in usermode? ]\n");
			s_bit = 0;
		}
		if (iw & 0x8000) {
			s_bit = 0;
			return_flag = 1;
		}
	}

	for (i=(u_bit? 0 : 15); i>=0 && i<=15; i+=(u_bit? 1 : -1)) {
		uint32_t value;

		if (!((iw >> i) & 1)) {
			/*  Skip register i:  */
			continue;
		}

		if (p_bit) {
			if (u_bit)
				addr += sizeof(uint32_t);
			else
				addr -= sizeof(uint32_t);
		}

		page = cpu->cd.arm.host_load[addr >> 12];
		if (page != NULL) {
			uint32_t *p32 = (uint32_t *) page;
			value = p32[(addr & 0xfff) >> 2];
			/*  Change byte order of value if
			    host and emulated endianness differ:  */
#ifdef HOST_LITTLE_ENDIAN
			if (cpu->byte_order == EMUL_BIG_ENDIAN)
#else
			if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
#endif
				value = ((value & 0xff) << 24) |
				    ((value & 0xff00) << 8) |
				    ((value & 0xff0000) >> 8) |
				    ((value & 0xff000000) >> 24);
		} else {
			if (!cpu->memory_rw(cpu, cpu->mem, addr, data,
			    sizeof(data), MEM_READ, CACHE_DATA)) {
				/*  load failed  */
				return;
			}
			if (cpu->byte_order == EMUL_LITTLE_ENDIAN) {
				value = data[0] +
				    (data[1] << 8) + (data[2] << 16)
				    + (data[3] << 24);
			} else {
				value = data[3] +
				    (data[2] << 8) + (data[1] << 16)
				    + (data[0] << 24);
			}
		}

		new_values[i] = value;

		if (!p_bit) {
			if (u_bit)
				addr += sizeof(uint32_t);
			else
				addr -= sizeof(uint32_t);
		}
	}

	for (i=(u_bit? 0 : 15); i>=0 && i<=15; i+=(u_bit? 1 : -1)) {
		if (!((iw >> i) & 1)) {
			/*  Skip register i:  */
			continue;
		}

		if (!s_bit) {
			cpu->cd.arm.r[i] = new_values[i];
		} else {
			switch (cpu->cd.arm.cpsr & ARM_FLAG_MODE) {
			case ARM_MODE_USR32:
			case ARM_MODE_SYS32:
				cpu->cd.arm.r[i] = new_values[i];
				break;
			case ARM_MODE_FIQ32:
				if (i >= 8 && i <= 14)
					cpu->cd.arm.default_r8_r14[i-8] =
					    new_values[i];
				else
					cpu->cd.arm.r[i] = new_values[i];
				break;
			case ARM_MODE_SVC32:
			case ARM_MODE_ABT32:
			case ARM_MODE_UND32:
			case ARM_MODE_IRQ32:
				if (i >= 13 && i <= 14)
					cpu->cd.arm.default_r8_r14[i-8] =
					    new_values[i];
				else
					cpu->cd.arm.r[i] = new_values[i];
				break;
			}
		}
	}

	if (w_bit)
		*np = addr;

	if (return_flag) {
		uint32_t new_cpsr;
		int switch_register_banks;

		switch (cpu->cd.arm.cpsr & ARM_FLAG_MODE) {
		case ARM_MODE_FIQ32:
			new_cpsr = cpu->cd.arm.spsr_fiq; break;
		case ARM_MODE_ABT32:
			new_cpsr = cpu->cd.arm.spsr_abt; break;
		case ARM_MODE_UND32:
			new_cpsr = cpu->cd.arm.spsr_und; break;
		case ARM_MODE_IRQ32:
			new_cpsr = cpu->cd.arm.spsr_irq; break;
		case ARM_MODE_SVC32:
			new_cpsr = cpu->cd.arm.spsr_svc; break;
		default:fatal("bdt_load: unimplemented mode %i\n",
			    cpu->cd.arm.cpsr & ARM_FLAG_MODE);
			exit(1);
		}

		switch_register_banks = (cpu->cd.arm.cpsr & ARM_FLAG_MODE) !=
		    (new_cpsr & ARM_FLAG_MODE);

		if (switch_register_banks)
			arm_save_register_bank(cpu);

		cpu->cd.arm.cpsr = new_cpsr;
		cpu->cd.arm.flags = cpu->cd.arm.cpsr >> 28;

		if (switch_register_banks)
			arm_load_register_bank(cpu);
	}

	/*  NOTE: Special case: Loading the PC  */
	if (iw & 0x8000) {
		cpu->pc = cpu->cd.arm.r[ARM_PC] & 0xfffffffc;
		if (cpu->machine->show_trace_tree)
			cpu_functioncall_trace_return(cpu);
		/*  TODO: There is no need to update the
		    pointers if this is a return to the
		    same page!  */
		/*  Find the new physical page and update the
		    translation pointers:  */
		quick_pc_to_pointers(cpu);
	}
}
Y(bdt_load)


/*
 *  bdt_store:  Block Data Transfer, Store
 *
 *  arg[0] = pointer to uint32_t in host memory, pointing to the base register
 *  arg[1] = 32-bit instruction word. Most bits are read from this.
 */
X(bdt_store)
{
	unsigned char data[4];
	uint32_t *np = (uint32_t *)ic->arg[0];
	uint32_t low_pc, value, addr = *np;
	uint32_t iw = ic->arg[1];  /*  xxxx100P USWLnnnn llllllll llllllll  */
	unsigned char *page;
	int p_bit = iw & 0x01000000;
	int u_bit = iw & 0x00800000;
	int s_bit = iw & 0x00400000;
	int w_bit = iw & 0x00200000;
	int i;

#ifdef GATHER_BDT_STATISTICS
	if (!s_bit)
		update_bdt_statistics(iw);
#endif

	/*  Synchronize the program counter:  */
	low_pc = ((size_t)ic - (size_t)
	    cpu->cd.arm.cur_ic_page) / sizeof(struct arm_instr_call);
	cpu->pc &= ~((ARM_IC_ENTRIES_PER_PAGE-1) << ARM_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (low_pc << ARM_INSTR_ALIGNMENT_SHIFT);

	for (i=(u_bit? 0 : 15); i>=0 && i<=15; i+=(u_bit? 1 : -1)) {
		if (!((iw >> i) & 1)) {
			/*  Skip register i:  */
			continue;
		}

		value = cpu->cd.arm.r[i];

		if (s_bit) {
			switch (cpu->cd.arm.cpsr & ARM_FLAG_MODE) {
			case ARM_MODE_FIQ32:
				if (i >= 8 && i <= 14)
					value = cpu->cd.arm.default_r8_r14[i-8];
				break;
			case ARM_MODE_ABT32:
			case ARM_MODE_UND32:
			case ARM_MODE_IRQ32:
			case ARM_MODE_SVC32:
				if (i >= 13 && i <= 14)
					value = cpu->cd.arm.default_r8_r14[i-8];
				break;
			case ARM_MODE_USR32:
			case ARM_MODE_SYS32:
				break;
			}
		}

		/*  NOTE/TODO: 8 vs 12 on some ARMs  */
		if (i == ARM_PC)
			value = cpu->pc + 12;

		if (p_bit) {
			if (u_bit)
				addr += sizeof(uint32_t);
			else
				addr -= sizeof(uint32_t);
		}

		page = cpu->cd.arm.host_store[addr >> 12];
		if (page != NULL) {
			uint32_t *p32 = (uint32_t *) page;
			/*  Change byte order of value if
			    host and emulated endianness differ:  */
#ifdef HOST_LITTLE_ENDIAN
			if (cpu->byte_order == EMUL_BIG_ENDIAN)
#else
			if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
#endif
				value = ((value & 0xff) << 24) |
				    ((value & 0xff00) << 8) |
				    ((value & 0xff0000) >> 8) |
				    ((value & 0xff000000) >> 24);
			p32[(addr & 0xfff) >> 2] = value;
		} else {
			if (cpu->byte_order == EMUL_LITTLE_ENDIAN) {
				data[0] = value;
				data[1] = value >> 8;
				data[2] = value >> 16;
				data[3] = value >> 24;
			} else {
				data[0] = value >> 24;
				data[1] = value >> 16;
				data[2] = value >> 8;
				data[3] = value;
			}
			if (!cpu->memory_rw(cpu, cpu->mem, addr, data,
			    sizeof(data), MEM_WRITE, CACHE_DATA)) {
				/*  store failed  */
				return;
			}
		}

		if (!p_bit) {
			if (u_bit)
				addr += sizeof(uint32_t);
			else
				addr -= sizeof(uint32_t);
		}
	}

	if (w_bit)
		*np = addr;
}
Y(bdt_store)


/*  Various load/store multiple instructions:  */
extern uint32_t *multi_opcode[256];
extern void (**multi_opcode_f[256])(struct cpu *, struct arm_instr_call *);
X(multi_0x08b15018);
X(multi_0x08ac000c__ge);
X(multi_0x08a05018);


/*****************************************************************************/


/*
 *  netbsd_memset:
 *
 *  The core of a NetBSD/arm memset.
 *
 *  f01bc420:  e25XX080     subs    rX,rX,#0x80
 *  f01bc424:  a8ac000c     stmgeia ip!,{r2,r3}   (16 of these)
 *  ..
 *  f01bc464:  caffffed     bgt     0xf01bc420      <memset+0x38>
 */
X(netbsd_memset)
{
	unsigned char *page;
	uint32_t addr;

	do {
		addr = cpu->cd.arm.r[ARM_IP];

		instr(subs)(cpu, ic);

		if (((cpu->cd.arm.flags & ARM_F_N)?1:0) !=
		    ((cpu->cd.arm.flags & ARM_F_V)?1:0)) {
			cpu->n_translated_instrs += 16;
			/*  Skip the store multiples:  */
			cpu->cd.arm.next_ic = &ic[17];
			return;
		}

		/*  Crossing a page boundary? Then continue non-combined.  */
		if ((addr & 0xfff) + 128 > 0x1000)
			return;

		/*  R2/R3 non-zero? Not allowed here.  */
		if (cpu->cd.arm.r[2] != 0 || cpu->cd.arm.r[3] != 0)
			return;

		/*  printf("addr = 0x%08x\n", addr);  */

		page = cpu->cd.arm.host_store[addr >> 12];
		/*  No page translation? Continue non-combined.  */
		if (page == NULL)
			return;

		/*  Clear:  */
		memset(page + (addr & 0xfff), 0, 128);
		cpu->cd.arm.r[ARM_IP] = addr + 128;
		cpu->n_translated_instrs += 16;

		/*  Branch back if greater:  */
		cpu->n_translated_instrs += 1;
	} while (((cpu->cd.arm.flags & ARM_F_N)?1:0) ==
	    ((cpu->cd.arm.flags & ARM_F_V)?1:0) &&
	    !(cpu->cd.arm.flags & ARM_F_Z));

	/*  Continue at the instruction after the bgt:  */
	cpu->cd.arm.next_ic = &ic[18];
}


/*
 *  netbsd_memcpy:
 *
 *  The core of a NetBSD/arm memcpy.
 *
 *  f01bc530:  e8b15018     ldmia   r1!,{r3,r4,ip,lr}
 *  f01bc534:  e8a05018     stmia   r0!,{r3,r4,ip,lr}
 *  f01bc538:  e8b15018     ldmia   r1!,{r3,r4,ip,lr}
 *  f01bc53c:  e8a05018     stmia   r0!,{r3,r4,ip,lr}
 *  f01bc540:  e2522020     subs    r2,r2,#0x20
 *  f01bc544:  aafffff9     bge     0xf01bc530
 */
X(netbsd_memcpy)
{
	unsigned char *page_0, *page_1;
	uint32_t addr_r0, addr_r1;

	do {
		addr_r0 = cpu->cd.arm.r[0];
		addr_r1 = cpu->cd.arm.r[1];

		/*  printf("addr_r0 = %08x  r1 = %08x\n", addr_r0, addr_r1);  */

		/*  Crossing a page boundary? Then continue non-combined.  */
		if ((addr_r0 & 0xfff) + 32 > 0x1000 ||
		    (addr_r1 & 0xfff) + 32 > 0x1000) {
			instr(multi_0x08b15018)(cpu, ic);
			return;
		}

		page_0 = cpu->cd.arm.host_store[addr_r0 >> 12];
		page_1 = cpu->cd.arm.host_store[addr_r1 >> 12];

		/*  No page translations? Continue non-combined.  */
		if (page_0 == NULL || page_1 == NULL) {
			instr(multi_0x08b15018)(cpu, ic);
			return;
		}

		memcpy(page_0 + (addr_r0 & 0xfff),
		    page_1 + (addr_r1 & 0xfff), 32);
		cpu->cd.arm.r[0] = addr_r0 + 32;
		cpu->cd.arm.r[1] = addr_r1 + 32;

		cpu->n_translated_instrs += 4;

		instr(subs)(cpu, ic + 4);
		cpu->n_translated_instrs ++;

		/*  Loop while greater or equal:  */
		cpu->n_translated_instrs ++;
	} while (((cpu->cd.arm.flags & ARM_F_N)?1:0) ==
	    ((cpu->cd.arm.flags & ARM_F_V)?1:0));

	/*  Continue at the instruction after the bge:  */
	cpu->cd.arm.next_ic = &ic[6];
	cpu->n_translated_instrs --;
}


/*
 *  netbsd_cacheclean:
 *
 *  The core of a NetBSD/arm cache clean routine, variant 1:
 *
 *  f015f88c:  e4902020     ldr     r2,[r0],#32
 *  f015f890:  e2511020     subs    r1,r1,#0x20
 *  f015f894:  1afffffc     bne     0xf015f88c
 *  f015f898:  ee070f9a     mcr     15,0,r0,cr7,cr10,4
 */
X(netbsd_cacheclean)
{
	uint32_t r1 = cpu->cd.arm.r[1];
	cpu->n_translated_instrs += ((r1 >> 5) * 3);
	cpu->cd.arm.r[0] += r1;
	cpu->cd.arm.r[1] = 0;
	cpu->cd.arm.next_ic = &ic[4];
}


/*
 *  netbsd_cacheclean2:
 *
 *  The core of a NetBSD/arm cache clean routine, variant 2:
 *
 *  f015f93c:  ee070f3a     mcr     15,0,r0,cr7,cr10,1
 *  f015f940:  ee070f36     mcr     15,0,r0,cr7,cr6,1
 *  f015f944:  e2800020     add     r0,r0,#0x20
 *  f015f948:  e2511020     subs    r1,r1,#0x20
 *  f015f94c:  8afffffa     bhi     0xf015f93c
 */
X(netbsd_cacheclean2)
{
	cpu->n_translated_instrs += ((cpu->cd.arm.r[1] >> 5) * 5) - 1;
	cpu->cd.arm.next_ic = &ic[5];
}


/*
 *  netbsd_scanc:
 *
 *  f01bccbc:  e5d13000     ldrb    r3,[r1]
 *  f01bccc0:  e7d23003     ldrb    r3,[r2,r3]
 *  f01bccc4:  e113000c     tsts    r3,ip
 */
X(netbsd_scanc)
{
	unsigned char *page = cpu->cd.arm.host_load[cpu->cd.arm.r[1] >> 12];
	uint32_t t;

	if (page == NULL) {
		instr(load_w0_byte_u1_p1_imm)(cpu, ic);
		return;
	}

	t = page[cpu->cd.arm.r[1] & 0xfff];
	t += cpu->cd.arm.r[2];
	page = cpu->cd.arm.host_load[t >> 12];

	if (page == NULL) {
		instr(load_w0_byte_u1_p1_imm)(cpu, ic);
		return;
	}

	cpu->cd.arm.r[3] = page[t & 0xfff];

	t = cpu->cd.arm.r[3] & cpu->cd.arm.r[ARM_IP];
	cpu->cd.arm.flags &= ~(ARM_F_Z | ARM_F_N);
	if (t == 0)
		cpu->cd.arm.flags |= ARM_F_Z;

	cpu->n_translated_instrs += 2;
	cpu->cd.arm.next_ic = &ic[3];
}


/*
 *  netbsd_idle:
 *
 *  L:	ldr     rX,[rY]
 *	teqs    rX,#0
 *	bne     X (samepage)
 *	teqs    rZ,#0
 *	beq     L (samepage)
 *	....
 *	X:  somewhere else on the same page
 */
X(netbsd_idle)
{
	uint32_t rY = reg(ic[0].arg[0]);
	uint32_t rZ = reg(ic[3].arg[0]);
	uint32_t *p;
	uint32_t rX;

	p = (uint32_t *) cpu->cd.arm.host_load[rY >> 12];
	if (p == NULL) {
		instr(load_w0_word_u1_p1_imm)(cpu, ic);
		return;
	}

	rX = p[(rY & 0xfff) >> 2];
	/*  No need to convert endianness, since it's only a 0-test.  */

	/*  This makes execution continue on the first teqs instruction,
	    which is fine.  */
	if (rX != 0) {
		instr(load_w0_word_u1_p1_imm)(cpu, ic);
		return;
	}

	if (rZ == 0) {
		static int x = 0;

		/*  Synch the program counter.  */
		uint32_t low_pc = ((size_t)ic - (size_t)
		    cpu->cd.arm.cur_ic_page) / sizeof(struct arm_instr_call);
		cpu->pc &= ~((ARM_IC_ENTRIES_PER_PAGE-1)
		    << ARM_INSTR_ALIGNMENT_SHIFT);
		cpu->pc += (low_pc << ARM_INSTR_ALIGNMENT_SHIFT);

		/*  Quasi-idle for a while:  */
		cpu->has_been_idling = 1;
	        if (cpu->machine->ncpus == 1 && (++x) == 100) {
			usleep(50);
			x = 0;
		}

		cpu->n_translated_instrs += N_SAFE_DYNTRANS_LIMIT / 6;
		cpu->cd.arm.next_ic = &nothing_call;
		return;
	}

	cpu->cd.arm.next_ic = &ic[5];
}


/*
 *  strlen:
 *
 *  S: e5f03001   ldrb  rY,[rX,#1]!
 *     e3530000   cmps  rY,#0
 *     1afffffc   bne   S
 */
X(strlen)
{
	unsigned int n_loops = 0;
	uint32_t rY, rX = reg(ic[0].arg[0]);
	unsigned char *p;

	do {
		rX ++;
		p = cpu->cd.arm.host_load[rX >> 12];
		if (p == NULL) {
			cpu->n_translated_instrs += (n_loops * 3);
			instr(load_w1_byte_u1_p1_imm)(cpu, ic);
			return;
		}

		rY = reg(ic[0].arg[2]) = p[rX & 0xfff];	/*  load  */
		reg(ic[0].arg[0]) = rX;			/*  writeback  */
		n_loops ++;

		/*  Compare rY to zero:  */
		cpu->cd.arm.flags = ARM_F_C;
		if (rY == 0)
			cpu->cd.arm.flags |= ARM_F_Z;
	} while (rY != 0);

	cpu->n_translated_instrs += (n_loops * 3) - 1;
	cpu->cd.arm.next_ic = &ic[3];
}


/*
 *  xchg:
 *
 *  e02YX00X     eor     rX,rY,rX
 *  e02XY00Y     eor     rY,rX,rY
 *  e02YX00X     eor     rX,rY,rX
 */
X(xchg)
{
	uint32_t tmp = reg(ic[0].arg[0]);
	cpu->n_translated_instrs += 2;
	cpu->cd.arm.next_ic = &ic[3];
	reg(ic[0].arg[0]) = reg(ic[1].arg[0]);
	reg(ic[1].arg[0]) = tmp;
}


/*
 *  netbsd_copyin:
 *
 *  e4b0a004     ldrt    sl,[r0],#4
 *  e4b0b004     ldrt    fp,[r0],#4
 *  e4b06004     ldrt    r6,[r0],#4
 *  e4b07004     ldrt    r7,[r0],#4
 *  e4b08004     ldrt    r8,[r0],#4
 *  e4b09004     ldrt    r9,[r0],#4
 */
X(netbsd_copyin)
{
	uint32_t r0 = cpu->cd.arm.r[0], ofs = (r0 & 0xffc), index = r0 >> 12;
	unsigned char *p = cpu->cd.arm.host_load[index];
	uint32_t *p32 = (uint32_t *) p, *q32;
	int ok = cpu->cd.arm.is_userpage[index >> 5] & (1 << (index & 31));

	if (ofs > 0x1000 - 6*4 || !ok || p == NULL) {
		instr(load_w1_word_u1_p0_imm)(cpu, ic);
		return;
	}
	q32 = &cpu->cd.arm.r[6];
	ofs >>= 2;
	q32[0] = p32[ofs+2];
	q32[1] = p32[ofs+3];
	q32[2] = p32[ofs+4];
	q32[3] = p32[ofs+5];
	q32[4] = p32[ofs+0];
	q32[5] = p32[ofs+1];
	cpu->cd.arm.r[0] = r0 + 24;
	cpu->n_translated_instrs += 5;
	cpu->cd.arm.next_ic = &ic[6];
}


/*
 *  netbsd_copyout:
 *
 *  e4a18004     strt    r8,[r1],#4
 *  e4a19004     strt    r9,[r1],#4
 *  e4a1a004     strt    sl,[r1],#4
 *  e4a1b004     strt    fp,[r1],#4
 *  e4a16004     strt    r6,[r1],#4
 *  e4a17004     strt    r7,[r1],#4
 */
X(netbsd_copyout)
{
	uint32_t r1 = cpu->cd.arm.r[1], ofs = (r1 & 0xffc), index = r1 >> 12;
	unsigned char *p = cpu->cd.arm.host_store[index];
	uint32_t *p32 = (uint32_t *) p, *q32;
	int ok = cpu->cd.arm.is_userpage[index >> 5] & (1 << (index & 31));

	if (ofs > 0x1000 - 6*4 || !ok || p == NULL) {
		instr(store_w1_word_u1_p0_imm)(cpu, ic);
		return;
	}
	q32 = &cpu->cd.arm.r[6];
	ofs >>= 2;
	p32[ofs  ] = q32[2];
	p32[ofs+1] = q32[3];
	p32[ofs+2] = q32[4];
	p32[ofs+3] = q32[5];
	p32[ofs+4] = q32[0];
	p32[ofs+5] = q32[1];
	cpu->cd.arm.r[1] = r1 + 24;
	cpu->n_translated_instrs += 5;
	cpu->cd.arm.next_ic = &ic[6];
}


/*
 *  cmps by 0, followed by beq (inside the same page):
 */
X(cmps0_beq_samepage)
{
	uint32_t a = reg(ic->arg[0]);
	cpu->n_translated_instrs ++;
	if (a == 0) {
		cpu->cd.arm.flags = ARM_F_Z | ARM_F_C;
	} else {
		/*  Semi-ugly hack which sets the negative-bit if a < 0:  */
		cpu->cd.arm.flags = ARM_F_C | ((a >> 28) & 8);
	}
	if (a == 0)
		cpu->cd.arm.next_ic = (struct arm_instr_call *) ic[1].arg[0];
	else
		cpu->cd.arm.next_ic = &ic[2];
}


/*
 *  cmps followed by beq (inside the same page):
 */
X(cmps_beq_samepage)
{
	uint32_t a = reg(ic->arg[0]), b = ic->arg[1], c = a - b;
	cpu->n_translated_instrs ++;
	cpu->cd.arm.flags = ((uint32_t)a >= (uint32_t)b)? ARM_F_C : 0;
	if (((int32_t)a >= 0 && (int32_t)b < 0 && (int32_t)c < 0) ||
	    ((int32_t)a < 0 && (int32_t)b >= 0 && (int32_t)c >= 0))
		cpu->cd.arm.flags |= ARM_F_V;
	if (c == 0) {
		cpu->cd.arm.flags |= ARM_F_Z;
		cpu->cd.arm.next_ic = (struct arm_instr_call *) ic[1].arg[0];
	} else {
		cpu->cd.arm.next_ic = &ic[2];
		if (c & 0x80000000)
			cpu->cd.arm.flags |= ARM_F_N;
	}
}


/*
 *  cmps followed by beq (not the same page):
 */
X(cmps_0_beq)
{
	uint32_t a = reg(ic->arg[0]);
	cpu->n_translated_instrs ++;
	if (a == 0) {
		cpu->cd.arm.flags = ARM_F_Z | ARM_F_C;
		cpu->pc = (uint32_t)(((uint32_t)cpu->pc & 0xfffff000)
		    + (int32_t)ic[1].arg[0]);
		quick_pc_to_pointers(cpu);
	} else {
		/*  Semi-ugly hack which sets the negative-bit if a < 0:  */
		cpu->cd.arm.flags = ARM_F_C | ((a >> 28) & 8);
		cpu->cd.arm.next_ic = &ic[2];
	}
}
X(cmps_pos_beq)
{
	uint32_t a = reg(ic->arg[0]), b = ic->arg[1], c = a - b;
	cpu->n_translated_instrs ++;
	cpu->cd.arm.flags = ((uint32_t)a >= (uint32_t)b)? ARM_F_C : 0;
	if ((int32_t)a < 0 && (int32_t)c >= 0)
		cpu->cd.arm.flags |= ARM_F_V;
	if (c == 0) {
		cpu->cd.arm.flags |= ARM_F_Z;
		cpu->pc = (uint32_t)(((uint32_t)cpu->pc & 0xfffff000)
		    + (int32_t)ic[1].arg[0]);
		quick_pc_to_pointers(cpu);
	} else {
		cpu->cd.arm.next_ic = &ic[2];
		if (c & 0x80000000)
			cpu->cd.arm.flags |= ARM_F_N;
	}
}
X(cmps_neg_beq)
{
	uint32_t a = reg(ic->arg[0]), b = ic->arg[1], c = a - b;
	cpu->n_translated_instrs ++;
	cpu->cd.arm.flags = ((uint32_t)a >= (uint32_t)b)? ARM_F_C : 0;
	if ((int32_t)a >= 0 && (int32_t)c < 0)
		cpu->cd.arm.flags |= ARM_F_V;
	if (c == 0) {
		cpu->cd.arm.flags |= ARM_F_Z;
		cpu->pc = (uint32_t)(((uint32_t)cpu->pc & 0xfffff000)
		    + (int32_t)ic[1].arg[0]);
		quick_pc_to_pointers(cpu);
	} else {
		cpu->cd.arm.next_ic = &ic[2];
		if (c & 0x80000000)
			cpu->cd.arm.flags |= ARM_F_N;
	}
}


/*
 *  cmps by 0, followed by bne (inside the same page):
 */
X(cmps0_bne_samepage)
{
	uint32_t a = reg(ic->arg[0]);
	cpu->n_translated_instrs ++;
	if (a == 0) {
		cpu->cd.arm.flags = ARM_F_Z | ARM_F_C;
	} else {
		/*  Semi-ugly hack which sets the negative-bit if a < 0:  */
		cpu->cd.arm.flags = ARM_F_C | ((a >> 28) & 8);
	}
	if (a == 0)
		cpu->cd.arm.next_ic = &ic[2];
	else
		cpu->cd.arm.next_ic = (struct arm_instr_call *) ic[1].arg[0];
}


/*
 *  cmps followed by bne (inside the same page):
 */
X(cmps_bne_samepage)
{
	uint32_t a = reg(ic->arg[0]), b = ic->arg[1], c = a - b;
	cpu->n_translated_instrs ++;
	cpu->cd.arm.flags = ((uint32_t)a >= (uint32_t)b)? ARM_F_C : 0;
	if (((int32_t)a >= 0 && (int32_t)b < 0 && (int32_t)c < 0) ||
	    ((int32_t)a < 0 && (int32_t)b >= 0 && (int32_t)c >= 0))
		cpu->cd.arm.flags |= ARM_F_V;
	if (c == 0) {
		cpu->cd.arm.flags |= ARM_F_Z;
		cpu->cd.arm.next_ic = &ic[2];
	} else {
		if (c & 0x80000000)
			cpu->cd.arm.flags |= ARM_F_N;
		cpu->cd.arm.next_ic = (struct arm_instr_call *) ic[1].arg[0];
	}
}


/*
 *  cmps followed by bcc (inside the same page):
 */
X(cmps_bcc_samepage)
{
	uint32_t a = reg(ic->arg[0]), b = ic->arg[1], c = a - b;
	cpu->n_translated_instrs ++;
	cpu->cd.arm.flags = ((uint32_t)a >= (uint32_t)b)? ARM_F_C : 0;
	if (c & 0x80000000)
		cpu->cd.arm.flags |= ARM_F_N;
	else if (c == 0)
		cpu->cd.arm.flags |= ARM_F_Z;
	if (((int32_t)a >= 0 && (int32_t)b < 0 && (int32_t)c < 0) ||
	    ((int32_t)a < 0 && (int32_t)b >= 0 && (int32_t)c >= 0))
		cpu->cd.arm.flags |= ARM_F_V;
	if (a >= b)
		cpu->cd.arm.next_ic = &ic[2];
	else
		cpu->cd.arm.next_ic = (struct arm_instr_call *) ic[1].arg[0];
}


/*
 *  cmps (reg) followed by bcc (inside the same page):
 */
X(cmps_reg_bcc_samepage)
{
	uint32_t a = reg(ic->arg[0]), b = reg(ic->arg[1]), c = a - b;
	cpu->n_translated_instrs ++;
	cpu->cd.arm.flags = ((uint32_t)a >= (uint32_t)b)? ARM_F_C : 0;
	if (c & 0x80000000)
		cpu->cd.arm.flags |= ARM_F_N;
	else if (c == 0)
		cpu->cd.arm.flags |= ARM_F_Z;
	if (((int32_t)a >= 0 && (int32_t)b < 0 && (int32_t)c < 0) ||
	    ((int32_t)a < 0 && (int32_t)b >= 0 && (int32_t)c >= 0))
		cpu->cd.arm.flags |= ARM_F_V;
	if (a >= b)
		cpu->cd.arm.next_ic = &ic[2];
	else
		cpu->cd.arm.next_ic = (struct arm_instr_call *) ic[1].arg[0];
}


/*
 *  cmps followed by bhi (inside the same page):
 */
X(cmps_bhi_samepage)
{
	uint32_t a = reg(ic->arg[0]), b = ic->arg[1], c = a - b;
	cpu->n_translated_instrs ++;
	cpu->cd.arm.flags = ((uint32_t)a >= (uint32_t)b)? ARM_F_C : 0;
	if (c & 0x80000000)
		cpu->cd.arm.flags |= ARM_F_N;
	else if (c == 0)
		cpu->cd.arm.flags |= ARM_F_Z;
	if (((int32_t)a >= 0 && (int32_t)b < 0 && (int32_t)c < 0) ||
	    ((int32_t)a < 0 && (int32_t)b >= 0 && (int32_t)c >= 0))
		cpu->cd.arm.flags |= ARM_F_V;
	if (a > b)
		cpu->cd.arm.next_ic = (struct arm_instr_call *) ic[1].arg[0];
	else
		cpu->cd.arm.next_ic = &ic[2];
}


/*
 *  cmps (reg) followed by bhi (inside the same page):
 */
X(cmps_reg_bhi_samepage)
{
	uint32_t a = reg(ic->arg[0]), b = reg(ic->arg[1]), c = a - b;
	cpu->n_translated_instrs ++;
	cpu->cd.arm.flags = ((uint32_t)a >= (uint32_t)b)? ARM_F_C : 0;
	if (c & 0x80000000)
		cpu->cd.arm.flags |= ARM_F_N;
	else if (c == 0)
		cpu->cd.arm.flags |= ARM_F_Z;
	if (((int32_t)a >= 0 && (int32_t)b < 0 && (int32_t)c < 0) ||
	    ((int32_t)a < 0 && (int32_t)b >= 0 && (int32_t)c >= 0))
		cpu->cd.arm.flags |= ARM_F_V;
	if (a > b)
		cpu->cd.arm.next_ic = (struct arm_instr_call *) ic[1].arg[0];
	else
		cpu->cd.arm.next_ic = &ic[2];
}


/*
 *  cmps followed by bgt (inside the same page):
 */
X(cmps_bgt_samepage)
{
	uint32_t a = reg(ic->arg[0]), b = ic->arg[1], c = a - b;
	cpu->n_translated_instrs ++;
	cpu->cd.arm.flags = ((uint32_t)a >= (uint32_t)b)? ARM_F_C : 0;
	if (c & 0x80000000)
		cpu->cd.arm.flags |= ARM_F_N;
	else if (c == 0)
		cpu->cd.arm.flags |= ARM_F_Z;
	if (((int32_t)a >= 0 && (int32_t)b < 0 && (int32_t)c < 0) ||
	    ((int32_t)a < 0 && (int32_t)b >= 0 && (int32_t)c >= 0))
		cpu->cd.arm.flags |= ARM_F_V;
	if ((int32_t)a > (int32_t)b)
		cpu->cd.arm.next_ic = (struct arm_instr_call *) ic[1].arg[0];
	else
		cpu->cd.arm.next_ic = &ic[2];
}


/*
 *  cmps followed by ble (inside the same page):
 */
X(cmps_ble_samepage)
{
	uint32_t a = reg(ic->arg[0]), b = ic->arg[1], c = a - b;
	cpu->n_translated_instrs ++;
	cpu->cd.arm.flags = ((uint32_t)a >= (uint32_t)b)? ARM_F_C : 0;
	if (c & 0x80000000)
		cpu->cd.arm.flags |= ARM_F_N;
	else if (c == 0)
		cpu->cd.arm.flags |= ARM_F_Z;
	if (((int32_t)a >= 0 && (int32_t)b < 0 && (int32_t)c < 0) ||
	    ((int32_t)a < 0 && (int32_t)b >= 0 && (int32_t)c >= 0))
		cpu->cd.arm.flags |= ARM_F_V;
	if ((int32_t)a <= (int32_t)b)
		cpu->cd.arm.next_ic = (struct arm_instr_call *) ic[1].arg[0];
	else
		cpu->cd.arm.next_ic = &ic[2];
}


/*
 *  teqs followed by beq (inside the same page):
 */
X(teqs_beq_samepage)
{
	uint32_t a = reg(ic->arg[0]), b = ic->arg[1], c = a ^ b;
	cpu->n_translated_instrs ++;
	cpu->cd.arm.flags &= ~(ARM_F_Z | ARM_F_N);
	if (c == 0) {
		cpu->cd.arm.flags |= ARM_F_Z;
		cpu->cd.arm.next_ic = (struct arm_instr_call *)
		    ic[1].arg[0];
	} else {
		if (c & 0x80000000)
			cpu->cd.arm.flags |= ARM_F_N;
		cpu->cd.arm.next_ic = &ic[2];
	}
}


/*
 *  tsts followed by beq (inside the same page):
 *  (arg[1] must not have its highest bit set))
 */
X(tsts_lo_beq_samepage)
{
	uint32_t a = reg(ic->arg[0]), b = ic->arg[1], c = a & b;
	cpu->n_translated_instrs ++;
	cpu->cd.arm.flags &= ~(ARM_F_Z | ARM_F_N);
	if (c == 0)
		cpu->cd.arm.flags |= ARM_F_Z;
	if (c == 0)
		cpu->cd.arm.next_ic = (struct arm_instr_call *)
		    ic[1].arg[0];
	else
		cpu->cd.arm.next_ic = &ic[2];
}


/*
 *  teqs followed by bne (inside the same page):
 */
X(teqs_bne_samepage)
{
	uint32_t a = reg(ic->arg[0]), b = ic->arg[1], c = a ^ b;
	cpu->n_translated_instrs ++;
	cpu->cd.arm.flags &= ~(ARM_F_Z | ARM_F_N);
	if (c == 0) {
		cpu->cd.arm.flags |= ARM_F_Z;
	} else {
		if (c & 0x80000000)
			cpu->cd.arm.flags |= ARM_F_N;
	}
	if (c == 0)
		cpu->cd.arm.next_ic = &ic[2];
	else
		cpu->cd.arm.next_ic = (struct arm_instr_call *)
		    ic[1].arg[0];
}


/*
 *  tsts followed by bne (inside the same page):
 *  (arg[1] must not have its highest bit set))
 */
X(tsts_lo_bne_samepage)
{
	uint32_t a = reg(ic->arg[0]), b = ic->arg[1], c = a & b;
	cpu->n_translated_instrs ++;
	cpu->cd.arm.flags &= ~(ARM_F_Z | ARM_F_N);
	if (c == 0)
		cpu->cd.arm.flags |= ARM_F_Z;
	if (c == 0)
		cpu->cd.arm.next_ic = &ic[2];
	else
		cpu->cd.arm.next_ic = (struct arm_instr_call *)
		    ic[1].arg[0];
}


/*****************************************************************************/


X(end_of_page)
{
	/*  Update the PC:  (offset 0, but on the next page)  */
	cpu->pc &= ~((ARM_IC_ENTRIES_PER_PAGE-1) << ARM_INSTR_ALIGNMENT_SHIFT);
	cpu->pc += (ARM_IC_ENTRIES_PER_PAGE << ARM_INSTR_ALIGNMENT_SHIFT);

	/*  Find the new physical page and update the translation pointers:  */
	quick_pc_to_pointers(cpu);

	/*  end_of_page doesn't count as an executed instruction:  */
	cpu->n_translated_instrs --;
}


/*****************************************************************************/


/*
 *  Combine: netbsd_memset():
 *
 *  Check for the core of a NetBSD/arm memset; large memsets use a sequence
 *  of 16 store-multiple instructions, each storing 2 registers at a time.
 */
void COMBINE(netbsd_memset)(struct cpu *cpu,
	struct arm_instr_call *ic, int low_addr)
{
#ifdef HOST_LITTLE_ENDIAN
	int n_back = (low_addr >> ARM_INSTR_ALIGNMENT_SHIFT)
	    & (ARM_IC_ENTRIES_PER_PAGE-1);

	if (n_back >= 17) {
		int i;
		for (i=-16; i<=-1; i++)
			if (ic[i].f != instr(multi_0x08ac000c__ge))
				return;
		if (ic[-17].f == instr(subs) &&
		    ic[-17].arg[0]==ic[-17].arg[2] && ic[-17].arg[1] == 128 &&
		    ic[ 0].f == instr(b_samepage__gt) &&
		    ic[ 0].arg[0] == (size_t)&ic[-17]) {
			ic[-17].f = instr(netbsd_memset);
		}
	}
#endif
}


/*
 *  Combine: netbsd_memcpy():
 *
 *  Check for the core of a NetBSD/arm memcpy; large memcpys use a
 *  sequence of ldmia instructions.
 */
void COMBINE(netbsd_memcpy)(struct cpu *cpu, struct arm_instr_call *ic,
	int low_addr)
{
#ifdef HOST_LITTLE_ENDIAN
	int n_back = (low_addr >> ARM_INSTR_ALIGNMENT_SHIFT)
	    & (ARM_IC_ENTRIES_PER_PAGE-1);

	if (n_back >= 5) {
		if (ic[-5].f==instr(multi_0x08b15018) &&
		    ic[-4].f==instr(multi_0x08a05018) &&
		    ic[-3].f==instr(multi_0x08b15018) &&
		    ic[-2].f==instr(multi_0x08a05018) &&
		    ic[-1].f == instr(subs) &&
		    ic[-1].arg[0]==ic[-1].arg[2] && ic[-1].arg[1] == 0x20 &&
		    ic[ 0].f == instr(b_samepage__ge) &&
		    ic[ 0].arg[0] == (size_t)&ic[-5]) {
			ic[-5].f = instr(netbsd_memcpy);
		}
	}
#endif
}


/*
 *  Combine: netbsd_cacheclean():
 *
 *  Check for the core of a NetBSD/arm cache clean. (There are two variants.)
 */
void COMBINE(netbsd_cacheclean)(struct cpu *cpu,
	struct arm_instr_call *ic, int low_addr)
{
	int n_back = (low_addr >> ARM_INSTR_ALIGNMENT_SHIFT)
	    & (ARM_IC_ENTRIES_PER_PAGE-1);

	if (n_back >= 3) {
		if (ic[-3].f==instr(load_w0_word_u1_p0_imm) &&
		    ic[-2].f == instr(subs) &&
		    ic[-2].arg[0]==ic[-2].arg[2] && ic[-2].arg[1] == 0x20 &&
		    ic[-1].f == instr(b_samepage__ne) &&
		    ic[-1].arg[0] == (size_t)&ic[-3]) {
			ic[-3].f = instr(netbsd_cacheclean);
		}
	}
}


/*
 *  Combine: netbsd_cacheclean2():
 *
 *  Check for the core of a NetBSD/arm cache clean. (Second variant.)
 */
void COMBINE(netbsd_cacheclean2)(struct cpu *cpu,
	struct arm_instr_call *ic, int low_addr)
{
	int n_back = (low_addr >> ARM_INSTR_ALIGNMENT_SHIFT)
	    & (ARM_IC_ENTRIES_PER_PAGE-1);

	if (n_back >= 4) {
		if (ic[-4].f == instr(mcr_mrc) && ic[-4].arg[0] == 0xee070f3a &&
		    ic[-3].f == instr(mcr_mrc) && ic[-3].arg[0] == 0xee070f36 &&
		    ic[-2].f == instr(add) &&
		    ic[-2].arg[0]==ic[-2].arg[2] && ic[-2].arg[1] == 0x20 &&
		    ic[-1].f == instr(subs) &&
		    ic[-1].arg[0]==ic[-1].arg[2] && ic[-1].arg[1] == 0x20) {
			ic[-4].f = instr(netbsd_cacheclean2);
		}
	}
}


/*
 *  Combine: netbsd_scanc():
 */
void COMBINE(netbsd_scanc)(struct cpu *cpu,
	struct arm_instr_call *ic, int low_addr)
{
	int n_back = (low_addr >> ARM_INSTR_ALIGNMENT_SHIFT)
	    & (ARM_IC_ENTRIES_PER_PAGE-1);

	if (n_back < 2)
		return;

	if (ic[-2].f == instr(load_w0_byte_u1_p1_imm) &&
	    ic[-2].arg[0] == (size_t)(&cpu->cd.arm.r[1]) &&
	    ic[-2].arg[1] == 0 &&
	    ic[-2].arg[2] == (size_t)(&cpu->cd.arm.r[3]) &&
	    ic[-1].f == instr(load_w0_byte_u1_p1_reg) &&
	    ic[-1].arg[0] == (size_t)(&cpu->cd.arm.r[2]) &&
	    ic[-1].arg[1] == (size_t)arm_r_r3_t0_c0 &&
	    ic[-1].arg[2] == (size_t)(&cpu->cd.arm.r[3])) {
		ic[-2].f = instr(netbsd_scanc);
	}
}


/*
 *  Combine: strlen():
 */
void COMBINE(strlen)(struct cpu *cpu,
	struct arm_instr_call *ic, int low_addr)
{
	int n_back = (low_addr >> ARM_INSTR_ALIGNMENT_SHIFT)
	    & (ARM_IC_ENTRIES_PER_PAGE-1);

	if (n_back < 2)
		return;

	if (ic[-2].f == instr(load_w1_byte_u1_p1_imm) &&
	    ic[-2].arg[1] == 1 &&
	    ic[-2].arg[2] == (size_t)(&cpu->cd.arm.r[3]) &&
	    ic[-1].f == instr(cmps) &&
	    ic[-1].arg[0] == (size_t)(&cpu->cd.arm.r[3]) &&
	    ic[-1].arg[1] == 0) {
		ic[-2].f = instr(strlen);
	}
}


/*
 *  Combine: xchg():
 */
void COMBINE(xchg)(struct cpu *cpu,
	struct arm_instr_call *ic, int low_addr)
{
	int n_back = (low_addr >> ARM_INSTR_ALIGNMENT_SHIFT)
	    & (ARM_IC_ENTRIES_PER_PAGE-1);
	size_t a, b;

	if (n_back < 2)
		return;

	a = ic[-2].arg[0]; b = ic[-1].arg[0];

	if (ic[-2].f == instr(eor_regshort) &&
	    ic[-1].f == instr(eor_regshort) &&
	    ic[-2].arg[0] == a && ic[-2].arg[1] == b && ic[-2].arg[2] == b &&
	    ic[-1].arg[0] == b && ic[-1].arg[1] == a && ic[-1].arg[2] == a &&
	    ic[ 0].arg[0] == a && ic[ 0].arg[1] == b && ic[ 0].arg[2] == b) {
		ic[-2].f = instr(xchg);
	}
}


/*
 *  Combine: netbsd_copyin():
 */
void COMBINE(netbsd_copyin)(struct cpu *cpu,
	struct arm_instr_call *ic, int low_addr)
{
#ifdef HOST_LITTLE_ENDIAN
	int i, n_back = (low_addr >> ARM_INSTR_ALIGNMENT_SHIFT)
	    & (ARM_IC_ENTRIES_PER_PAGE-1);

	if (n_back < 5)
		return;

	for (i=-5; i<0; i++) {
		if (ic[i].f != instr(load_w1_word_u1_p0_imm) ||
		    ic[i].arg[0] != (size_t)(&cpu->cd.arm.r[0]) ||
		    ic[i].arg[1] != 4)
			return;
	}

	if (ic[-5].arg[2] == (size_t)(&cpu->cd.arm.r[10]) &&
	    ic[-4].arg[2] == (size_t)(&cpu->cd.arm.r[11]) &&
	    ic[-3].arg[2] == (size_t)(&cpu->cd.arm.r[6]) &&
	    ic[-2].arg[2] == (size_t)(&cpu->cd.arm.r[7]) &&
	    ic[-1].arg[2] == (size_t)(&cpu->cd.arm.r[8])) {
		ic[-5].f = instr(netbsd_copyin);
	}
#endif
}


/*
 *  Combine: netbsd_copyout():
 */
void COMBINE(netbsd_copyout)(struct cpu *cpu,
	struct arm_instr_call *ic, int low_addr)
{
#ifdef HOST_LITTLE_ENDIAN
	int i, n_back = (low_addr >> ARM_INSTR_ALIGNMENT_SHIFT)
	    & (ARM_IC_ENTRIES_PER_PAGE-1);

	if (n_back < 5)
		return;

	for (i=-5; i<0; i++) {
		if (ic[i].f != instr(store_w1_word_u1_p0_imm) ||
		    ic[i].arg[0] != (size_t)(&cpu->cd.arm.r[1]) ||
		    ic[i].arg[1] != 4)
			return;
	}

	if (ic[-5].arg[2] == (size_t)(&cpu->cd.arm.r[8]) &&
	    ic[-4].arg[2] == (size_t)(&cpu->cd.arm.r[9]) &&
	    ic[-3].arg[2] == (size_t)(&cpu->cd.arm.r[10]) &&
	    ic[-2].arg[2] == (size_t)(&cpu->cd.arm.r[11]) &&
	    ic[-1].arg[2] == (size_t)(&cpu->cd.arm.r[6])) {
		ic[-5].f = instr(netbsd_copyout);
	}
#endif
}


/*
 *  Combine: cmps + beq, etc:
 */
void COMBINE(beq_etc)(struct cpu *cpu,
	struct arm_instr_call *ic, int low_addr)
{
	int n_back = (low_addr >> ARM_INSTR_ALIGNMENT_SHIFT)
	    & (ARM_IC_ENTRIES_PER_PAGE-1);
	if (n_back < 1)
		return;
	if (ic[0].f == instr(b__eq)) {
		if (ic[-1].f == instr(cmps)) {
			if (ic[-1].arg[1] == 0)
				ic[-1].f = instr(cmps_0_beq);
			else if (ic[-1].arg[1] & 0x80000000)
				ic[-1].f = instr(cmps_neg_beq);
			else
				ic[-1].f = instr(cmps_pos_beq);
		}
		return;
	}
	if (ic[0].f == instr(b_samepage__eq)) {
		if (ic[-1].f == instr(cmps)) {
			if (ic[-1].arg[1] == 0)
				ic[-1].f = instr(cmps0_beq_samepage);
			else
				ic[-1].f = instr(cmps_beq_samepage);
		}
		if (ic[-1].f == instr(tsts) &&
		    !(ic[-1].arg[1] & 0x80000000)) {
			ic[-1].f = instr(tsts_lo_beq_samepage);
		}
		if (n_back >= 4 &&
		    ic[-4].f == instr(load_w0_word_u1_p1_imm) &&
		    ic[-4].arg[0] != ic[-4].arg[2] &&
		    ic[-4].arg[1] == 0 &&
		    ic[-4].arg[2] == ic[-3].arg[0] &&
		    /*  Note: The teqs+bne is already combined!  */
		    ic[-3].f == instr(teqs_bne_samepage) &&
		    ic[-3].arg[1] == 0 &&
		    ic[-2].f == instr(b_samepage__ne) &&
		    ic[-1].f == instr(teqs) &&
		    ic[-1].arg[0] != ic[-4].arg[0] &&
		    ic[-1].arg[1] == 0) {
			ic[-4].f = instr(netbsd_idle);
		}
		if (ic[-1].f == instr(teqs)) {
			ic[-1].f = instr(teqs_beq_samepage);
		}
		return;
	}
	if (ic[0].f == instr(b_samepage__ne)) {
		if (ic[-1].f == instr(cmps)) {
			if (ic[-1].arg[1] == 0)
				ic[-1].f = instr(cmps0_bne_samepage);
			else
				ic[-1].f = instr(cmps_bne_samepage);
		}
		if (ic[-1].f == instr(tsts) &&
		    !(ic[-1].arg[1] & 0x80000000)) {
			ic[-1].f = instr(tsts_lo_bne_samepage);
		}
		if (ic[-1].f == instr(teqs)) {
			ic[-1].f = instr(teqs_bne_samepage);
		}
		return;
	}
	if (ic[0].f == instr(b_samepage__cc)) {
		if (ic[-1].f == instr(cmps)) {
			ic[-1].f = instr(cmps_bcc_samepage);
		}
		if (ic[-1].f == instr(cmps_regshort)) {
			ic[-1].f = instr(cmps_reg_bcc_samepage);
		}
		return;
	}
	if (ic[0].f == instr(b_samepage__hi)) {
		if (ic[-1].f == instr(cmps)) {
			ic[-1].f = instr(cmps_bhi_samepage);
		}
		if (ic[-1].f == instr(cmps_regshort)) {
			ic[-1].f = instr(cmps_reg_bhi_samepage);
		}
		return;
	}
	if (ic[0].f == instr(b_samepage__gt)) {
		if (ic[-1].f == instr(cmps)) {
			ic[-1].f = instr(cmps_bgt_samepage);
		}
		return;
	}
	if (ic[0].f == instr(b_samepage__le)) {
		if (ic[-1].f == instr(cmps)) {
			ic[-1].f = instr(cmps_ble_samepage);
		}
		return;
	}
}


/*****************************************************************************/


static void arm_switch_clear(struct arm_instr_call *ic, int rd,
	int condition_code)
{
	switch (rd) {
	case  0: ic->f = cond_instr(clear_r0); break;
	case  1: ic->f = cond_instr(clear_r1); break;
	case  2: ic->f = cond_instr(clear_r2); break;
	case  3: ic->f = cond_instr(clear_r3); break;
	case  4: ic->f = cond_instr(clear_r4); break;
	case  5: ic->f = cond_instr(clear_r5); break;
	case  6: ic->f = cond_instr(clear_r6); break;
	case  7: ic->f = cond_instr(clear_r7); break;
	case  8: ic->f = cond_instr(clear_r8); break;
	case  9: ic->f = cond_instr(clear_r9); break;
	case 10: ic->f = cond_instr(clear_r10); break;
	case 11: ic->f = cond_instr(clear_r11); break;
	case 12: ic->f = cond_instr(clear_r12); break;
	case 13: ic->f = cond_instr(clear_r13); break;
	case 14: ic->f = cond_instr(clear_r14); break;
	}
}


static void arm_switch_mov1(struct arm_instr_call *ic, int rd,
	int condition_code)
{
	switch (rd) {
	case  0: ic->f = cond_instr(mov1_r0); break;
	case  1: ic->f = cond_instr(mov1_r1); break;
	case  2: ic->f = cond_instr(mov1_r2); break;
	case  3: ic->f = cond_instr(mov1_r3); break;
	case  4: ic->f = cond_instr(mov1_r4); break;
	case  5: ic->f = cond_instr(mov1_r5); break;
	case  6: ic->f = cond_instr(mov1_r6); break;
	case  7: ic->f = cond_instr(mov1_r7); break;
	case  8: ic->f = cond_instr(mov1_r8); break;
	case  9: ic->f = cond_instr(mov1_r9); break;
	case 10: ic->f = cond_instr(mov1_r10); break;
	case 11: ic->f = cond_instr(mov1_r11); break;
	case 12: ic->f = cond_instr(mov1_r12); break;
	case 13: ic->f = cond_instr(mov1_r13); break;
	case 14: ic->f = cond_instr(mov1_r14); break;
	}
}


static void arm_switch_add1(struct arm_instr_call *ic, int rd,
	int condition_code)
{
	switch (rd) {
	case  0: ic->f = cond_instr(add1_r0); break;
	case  1: ic->f = cond_instr(add1_r1); break;
	case  2: ic->f = cond_instr(add1_r2); break;
	case  3: ic->f = cond_instr(add1_r3); break;
	case  4: ic->f = cond_instr(add1_r4); break;
	case  5: ic->f = cond_instr(add1_r5); break;
	case  6: ic->f = cond_instr(add1_r6); break;
	case  7: ic->f = cond_instr(add1_r7); break;
	case  8: ic->f = cond_instr(add1_r8); break;
	case  9: ic->f = cond_instr(add1_r9); break;
	case 10: ic->f = cond_instr(add1_r10); break;
	case 11: ic->f = cond_instr(add1_r11); break;
	case 12: ic->f = cond_instr(add1_r12); break;
	case 13: ic->f = cond_instr(add1_r13); break;
	case 14: ic->f = cond_instr(add1_r14); break;
	}
}


/*****************************************************************************/


/*
 *  arm_instr_to_be_translated():
 *
 *  Translate an instruction word into an arm_instr_call. ic is filled in with
 *  valid data for the translated instruction, or a "nothing" instruction if
 *  there was a translation failure. The newly translated instruction is then
 *  executed.
 */
X(to_be_translated)
{
	uint32_t addr, low_pc, iword, imm = 0;
	unsigned char *page;
	unsigned char ib[4];
	int condition_code, main_opcode, secondary_opcode, s_bit, rn, rd, r8;
	int p_bit, u_bit, w_bit, l_bit, regform, rm, c, t, any_pc_reg;
	void (*samepage_function)(struct cpu *, struct arm_instr_call *);

	/*  Figure out the address of the instruction:  */
	low_pc = ((size_t)ic - (size_t)cpu->cd.arm.cur_ic_page)
	    / sizeof(struct arm_instr_call);
	addr = cpu->pc & ~((ARM_IC_ENTRIES_PER_PAGE-1) <<
	    ARM_INSTR_ALIGNMENT_SHIFT);
	addr += (low_pc << ARM_INSTR_ALIGNMENT_SHIFT);
	cpu->pc = addr;
	addr &= ~((1 << ARM_INSTR_ALIGNMENT_SHIFT) - 1);

	/*  Read the instruction word from memory:  */
	page = cpu->cd.arm.host_load[addr >> 12];

	if (page != NULL) {
		/*  fatal("TRANSLATION HIT! 0x%08x\n", addr);  */
		memcpy(ib, page + (addr & 0xfff), sizeof(ib));
	} else {
		/*  fatal("TRANSLATION MISS! 0x%08x\n", addr);  */
		if (!cpu->memory_rw(cpu, cpu->mem, addr, &ib[0],
		    sizeof(ib), MEM_READ, CACHE_INSTRUCTION)) {
			fatal("to_be_translated(): "
			    "read failed: TODO\n");
			return;
		}
	}

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		iword = ib[0] + (ib[1]<<8) + (ib[2]<<16) + (ib[3]<<24);
	else
		iword = ib[3] + (ib[2]<<8) + (ib[1]<<16) + (ib[0]<<24);


#define DYNTRANS_TO_BE_TRANSLATED_HEAD
#include "cpu_dyntrans.cc"
#undef  DYNTRANS_TO_BE_TRANSLATED_HEAD


	/*  The idea of taking bits 27..24 was found here:
	    http://armphetamine.sourceforge.net/oldinfo.html  */
	condition_code = iword >> 28;
	main_opcode = (iword >> 24) & 15;
	secondary_opcode = (iword >> 21) & 15;
	u_bit = iword & 0x00800000;
	w_bit = iword & 0x00200000;
	s_bit = l_bit = iword & 0x00100000;
	rn    = (iword >> 16) & 15;
	rd    = (iword >> 12) & 15;
	r8    = (iword >> 8) & 15;
	c     = (iword >> 7) & 31;
	t     = (iword >> 4) & 7;
	rm    = iword & 15;

	if (condition_code == 0xf) {
		if ((iword & 0xfc70f000) == 0xf450f000) {
			/*  Preload:  TODO.  Treat as NOP for now.  */
			ic->f = instr(nop);
			goto okay;
		}

		if (!cpu->translation_readahead)
			fatal("TODO: ARM condition code 0x%x\n",
			    condition_code);
		goto bad;
	}


	/*
	 *  Translate the instruction:
	 */

	switch (main_opcode) {

	case 0x0:
	case 0x1:
	case 0x2:
	case 0x3:
		/*  Check special cases first:  */
		if ((iword & 0x0fc000f0) == 0x00000090) {
			/*
			 *  Multiplication:
			 *  xxxx0000 00ASdddd nnnnssss 1001mmmm (Rd,Rm,Rs[,Rn])
			 */
			if (iword & 0x00200000) {
				if (s_bit)
					ic->f = cond_instr(mlas);
				else
					ic->f = cond_instr(mla);
				ic->arg[0] = iword;
			} else {
				if (s_bit)
					ic->f = cond_instr(muls);
				else
					ic->f = cond_instr(mul);
				/*  NOTE: rn means rd in this case:  */
				ic->arg[0] = (size_t)(&cpu->cd.arm.r[rn]);
				ic->arg[1] = (size_t)(&cpu->cd.arm.r[rm]);
				ic->arg[2] = (size_t)(&cpu->cd.arm.r[r8]);
			}
			break;
		}
		if ((iword & 0x0f8000f0) == 0x00800090) {
			/*  Long multiplication:  */
			if (s_bit) {
				if (!cpu->translation_readahead)
					fatal("TODO: sbit mull\n");
				goto bad;
			}
			ic->f = cond_instr(mull);
			ic->arg[0] = iword;
			break;
		}
		if ((iword & 0x0f900ff0) == 0x01000050) {
			if (!cpu->translation_readahead)
				fatal("TODO: q{,d}{add,sub}\n");
			goto bad;
		}
		if ((iword & 0x0ff000d0) == 0x01200010) {
			/*  bx or blx  */
			if (iword & 0x20)
				ic->f = cond_instr(blx);
			else {
				if (cpu->machine->show_trace_tree &&
				    rm == ARM_LR)
					ic->f = cond_instr(bx_trace);
				else
					ic->f = cond_instr(bx);
			}
			ic->arg[0] = (size_t)(&cpu->cd.arm.r[rm]);
                        break;
                }
		if ((iword & 0x0fb00ff0) == 0x1000090) {
			if (iword & 0x00400000)
				ic->f = cond_instr(swpb);
			else
				ic->f = cond_instr(swp);
			ic->arg[0] = (size_t)(&cpu->cd.arm.r[rd]);
			ic->arg[1] = (size_t)(&cpu->cd.arm.r[rm]);
			ic->arg[2] = (size_t)(&cpu->cd.arm.r[rn]);
			break;
		}
		if ((iword & 0x0fff0ff0) == 0x016f0f10) {
			ic->f = cond_instr(clz);
			ic->arg[0] = (size_t)(&cpu->cd.arm.r[rm]);
			ic->arg[1] = (size_t)(&cpu->cd.arm.r[rd]);
			break;
		}
		if ((iword & 0x0ff00090) == 0x01000080) {
			/*  TODO: smlaXX  */
			goto bad;
		}
		if ((iword & 0x0ff00090) == 0x01400080) {
			/*  TODO: smlalY  */
			goto bad;
		}
		if ((iword & 0x0ff000b0) == 0x01200080) {
			/*  TODO: smlawY  */
			goto bad;
		}
		if ((iword & 0x0ff0f090) == 0x01600080) {
			/*  smulXY (16-bit * 16-bit => 32-bit)  */
			switch (iword & 0x60) {
			case 0x00: ic->f = cond_instr(smulbb); break;
			case 0x20: ic->f = cond_instr(smultb); break;
			case 0x40: ic->f = cond_instr(smulbt); break;
			default:   ic->f = cond_instr(smultt); break;
			}
			ic->arg[0] = (size_t)(&cpu->cd.arm.r[rm]);
			ic->arg[1] = (size_t)(&cpu->cd.arm.r[r8]);
			ic->arg[2] = (size_t)(&cpu->cd.arm.r[rn]); /*  Rd  */
			break;
		}
		if ((iword & 0x0ff0f0b0) == 0x012000a0) {
			/*  TODO: smulwY  */
			goto bad;
		}
		if ((iword & 0x0fb0fff0) == 0x0120f000 ||
		    (iword & 0x0fb0f000) == 0x0320f000) {
			/*  msr: move to [S|C]PSR from a register or
			    immediate value  */
			if (iword & 0x02000000) {
				if (iword & 0x00400000)
					ic->f = cond_instr(msr_imm_spsr);
				else
					ic->f = cond_instr(msr_imm);
			} else {
				if (rm == ARM_PC) {
					if (!cpu->translation_readahead)
						fatal("msr PC?\n");
					goto bad;
				}
				if (iword & 0x00400000)
					ic->f = cond_instr(msr_spsr);
				else
					ic->f = cond_instr(msr);
			}
			imm = iword & 0xff;
			while (r8-- > 0)
				imm = (imm >> 2) | ((imm & 3) << 30);
			ic->arg[0] = imm;
			ic->arg[2] = (size_t)(&cpu->cd.arm.r[rm]);
			switch ((iword >> 16) & 15) {
			case 1:	ic->arg[1] = 0x000000ff; break;
			case 8:	ic->arg[1] = 0xff000000; break;
			case 9:	ic->arg[1] = 0xff0000ff; break;
			default:if (!cpu->translation_readahead)
					fatal("unimpl a: msr regform\n");
				goto bad;
			}
			break;
		}
		if ((iword & 0x0fbf0fff) == 0x010f0000) {
			/*  mrs: move from CPSR/SPSR to a register:  */
			if (rd == ARM_PC) {
				if (!cpu->translation_readahead)
					fatal("mrs PC?\n");
				goto bad;
			}
			if (iword & 0x00400000)
				ic->f = cond_instr(mrs_spsr);
			else
				ic->f = cond_instr(mrs);
			ic->arg[0] = (size_t)(&cpu->cd.arm.r[rd]);
			break;
		}
		if ((iword & 0x0e000090) == 0x00000090) {
			int imm = ((iword >> 4) & 0xf0) | (iword & 0xf);
			int regform = !(iword & 0x00400000);
			p_bit = main_opcode & 1;
			ic->arg[0] = (size_t)(&cpu->cd.arm.r[rn]);
			ic->arg[2] = (size_t)(&cpu->cd.arm.r[rd]);
			if (rd == ARM_PC || rn == ARM_PC) {
				ic->f = arm_load_store_instr_3_pc[
				    condition_code + (l_bit? 16 : 0)
				    + (iword & 0x40? 32 : 0)
				    + (w_bit? 64 : 0)
				    + (iword & 0x20? 128 : 0)
				    + (u_bit? 256 : 0) + (p_bit? 512 : 0)
				    + (regform? 1024 : 0)];
				if (rn == ARM_PC)
					ic->arg[0] = (size_t)
					    (&cpu->cd.arm.tmp_pc);
				if (!l_bit && rd == ARM_PC)
					ic->arg[2] = (size_t)
					    (&cpu->cd.arm.tmp_pc);
			} else
				ic->f = arm_load_store_instr_3[
				    condition_code + (l_bit? 16 : 0)
				    + (iword & 0x40? 32 : 0)
				    + (w_bit? 64 : 0)
				    + (iword & 0x20? 128 : 0)
				    + (u_bit? 256 : 0) + (p_bit? 512 : 0)
				    + (regform? 1024 : 0)];
			if (regform)
				ic->arg[1] = (size_t)(void *)arm_r[iword & 0xf];
			else
				ic->arg[1] = imm;
			break;
		}

		if (iword & 0x80 && !(main_opcode & 2) && iword & 0x10) {
			if (!cpu->translation_readahead)
				fatal("reg form blah blah\n");
			goto bad;
		}

		/*  "bkpt", ARMv5 and above  */
		if ((iword & 0x0ff000f0) == 0x01200070) {
			ic->f = cond_instr(bkpt);
			ic->arg[0] = addr & 0xfff;
			break;
		}

		/*  "mov pc,lr":  */
		if ((iword & 0x0fffffff) == 0x01a0f00e) {
			if (cpu->machine->show_trace_tree)
				ic->f = cond_instr(ret_trace);
			else
				ic->f = cond_instr(ret);
			break;
		}

		/*  "mov reg,reg" or "mov reg,pc":  */
		if ((iword & 0x0fff0ff0) == 0x01a00000 && rd != ARM_PC) {
			if (rm != ARM_PC) {
				ic->f = cond_instr(mov_reg_reg);
				ic->arg[0] = (size_t)(&cpu->cd.arm.r[rm]);
			} else {
				ic->f = cond_instr(mov_reg_pc);
				ic->arg[0] = (addr & 0xfff) + 8;
			}
			ic->arg[1] = (size_t)(&cpu->cd.arm.r[rd]);
			break;
		}

		/*  "mov reg,#0":  */
		if ((iword & 0x0fff0fff) == 0x03a00000 && rd != ARM_PC) {
			arm_switch_clear(ic, rd, condition_code);
			break;
		}

		/*  "mov reg,#1":  */
		if ((iword & 0x0fff0fff) == 0x03a00001 && rd != ARM_PC) {
			arm_switch_mov1(ic, rd, condition_code);
			break;
		}

		/*  "add reg,reg,#1":  */
		if ((iword & 0x0ff00fff) == 0x02800001 && rd != ARM_PC
		    && rn == rd) {
			arm_switch_add1(ic, rd, condition_code);
			break;
		}

		/*
		 *  Generic Data Processing Instructions:
		 */
		if ((main_opcode & 2) == 0)
			regform = 1;
		else
			regform = 0;

		if (regform) {
			/*  0x1000 signifies Carry bit update on rotation,
			    which is not necessary for add,adc,sub,sbc,
			    rsb,rsc,cmp, or cmn, because they update the
			    Carry bit manually anyway.  */
			int q = 0x1000;
			if (s_bit == 0)
				q = 0;
			if ((secondary_opcode >= 2 && secondary_opcode <= 7)
			    || secondary_opcode==0xa || secondary_opcode==0xb)
				q = 0;
			ic->arg[1] = (size_t)(void *)arm_r[(iword & 0xfff) + q];
		} else {
			imm = iword & 0xff;
			while (r8-- > 0)
				imm = (imm >> 2) | ((imm & 3) << 30);
			ic->arg[1] = imm;
		}

		/*  mvn #imm ==> mov #~imm  */
		if (secondary_opcode == 0xf && !regform) {
			secondary_opcode = 0xd;
			ic->arg[1] = ~ic->arg[1];
		}

		ic->arg[0] = (size_t)(&cpu->cd.arm.r[rn]);
		ic->arg[2] = (size_t)(&cpu->cd.arm.r[rd]);
		any_pc_reg = 0;
		if (rn == ARM_PC || rd == ARM_PC)
			any_pc_reg = 1;

		if (!any_pc_reg && regform && (iword & 0xfff) < ARM_PC) {
			ic->arg[1] = (size_t)(&cpu->cd.arm.r[rm]);
			ic->f = arm_dpi_instr_regshort[condition_code +
			    16 * secondary_opcode + (s_bit? 256 : 0)];
		} else
			ic->f = arm_dpi_instr[condition_code +
			    16 * secondary_opcode + (s_bit? 256 : 0) +
			    (any_pc_reg? 512 : 0) + (regform? 1024 : 0)];

		if (ic->f == instr(eor_regshort))
			cpu->cd.arm.combination_check = COMBINE(xchg);
		if (iword == 0xe113000c)
			cpu->cd.arm.combination_check = COMBINE(netbsd_scanc);
		break;

	case 0x4:	/*  Load and store...  */
	case 0x5:	/*  xxxx010P UBWLnnnn ddddoooo oooooooo  Immediate  */
	case 0x6:	/*  xxxx011P UBWLnnnn ddddcccc ctt0mmmm  Register  */
	case 0x7:
		ic->arg[0] = (size_t)(&cpu->cd.arm.r[rn]);
		ic->arg[2] = (size_t)(&cpu->cd.arm.r[rd]);
		if (rd == ARM_PC || rn == ARM_PC) {
			ic->f = arm_load_store_instr_pc[((iword >> 16)
			    & 0x3f0) + condition_code];
			if (rn == ARM_PC)
				ic->arg[0] = (size_t)(&cpu->cd.arm.tmp_pc);
			if (!l_bit && rd == ARM_PC)
				ic->arg[2] = (size_t)(&cpu->cd.arm.tmp_pc);
		} else {
			ic->f = arm_load_store_instr[((iword >> 16) &
			    0x3f0) + condition_code];
		}
		imm = iword & 0xfff;
		if (main_opcode < 6)
			ic->arg[1] = imm;
		else
			ic->arg[1] = (size_t)(void *)arm_r[iword & 0xfff];
		if ((iword & 0x0e000010) == 0x06000010) {
			/*  GDB uses this for breakpoints.  */
			ic->f = cond_instr(und);
			ic->arg[0] = addr & 0xfff;
		}
		/*  Special case: pc-relative load within the same page:  */
		if (rn == ARM_PC && rd != ARM_PC && main_opcode < 6 && l_bit) {
			unsigned char *p = page;
			int ofs = (addr & 0xfff) + 8, max = 0xffc;
			int b_bit = iword & 0x00400000;
			if (b_bit)
				max = 0xfff;
			if (u_bit)
				ofs += (iword & 0xfff);
			else
				ofs -= (iword & 0xfff);
			/*  NOTE/TODO: This assumes 4KB pages,
			    it will not work with 1KB pages.  */
			if (ofs >= 0 && ofs <= max && p != NULL) {
				unsigned char c[4];
				int len = b_bit? 1 : 4;
				uint32_t x, a = (addr & 0xfffff000) | ofs;
				/*  ic->f = cond_instr(mov);  */
				ic->f = arm_dpi_instr[condition_code + 16*0xd];
				ic->arg[2] = (size_t)(&cpu->cd.arm.r[rd]);

				memcpy(c, p + (a & 0xfff), len);

				if (b_bit) {
					x = c[0];
				} else {
					if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
						x = c[0] + (c[1]<<8) +
						    (c[2]<<16) + (c[3]<<24);
					else
						x = c[3] + (c[2]<<8) +
						    (c[1]<<16) + (c[0]<<24);
				}
				ic->arg[1] = x;
			}
		}
		if (iword == 0xe4b09004)
			cpu->cd.arm.combination_check = COMBINE(netbsd_copyin);
		if (iword == 0xe4a17004)
			cpu->cd.arm.combination_check = COMBINE(netbsd_copyout);
		break;

	case 0x8:	/*  Multiple load/store...  (Block data transfer)  */
	case 0x9:	/*  xxxx100P USWLnnnn llllllll llllllll  */
		ic->arg[0] = (size_t)(&cpu->cd.arm.r[rn]);
		ic->arg[1] = (size_t)iword;
		/*  Generic case:  */
		if (l_bit)
			ic->f = cond_instr(bdt_load);
		else
			ic->f = cond_instr(bdt_store);
#if defined(HOST_LITTLE_ENDIAN) && !defined(GATHER_BDT_STATISTICS)
		/*
		 *  Check for availability of optimized implementation:
		 *  xxxx100P USWLnnnn llllllll llllllll
		 *           ^  ^ ^ ^        ^  ^ ^ ^   (0x00950154)
		 *  These bits are used to select which list to scan, and then
		 *  the list is scanned linearly.
		 *
		 *  The optimized functions do not support show_trace_tree,
		 *  but it's ok to use the unoptimized version in that case.
		 */
		if (!cpu->machine->show_trace_tree) {
			int i = 0, j = iword;
			j = ((j & 0x00800000) >> 16) | ((j & 0x00100000) >> 14)
			  | ((j & 0x00040000) >> 13) | ((j & 0x00010000) >> 12)
			  | ((j & 0x00000100) >>  5) | ((j & 0x00000040) >>  4)
			  | ((j & 0x00000010) >>  3) | ((j & 0x00000004) >>  2);
			while (multi_opcode[j][i] != 0) {
				if ((iword & 0x0fffffff) ==
				    multi_opcode[j][i]) {
					ic->f = multi_opcode_f[j]
					    [i*16 + condition_code];
					break;
				}
				i ++;
			}
		}
#endif
		if (rn == ARM_PC) {
			if (!cpu->translation_readahead)
				fatal("TODO: bdt with PC as base\n");
			goto bad;
		}
		break;

	case 0xa:					/*  B: branch  */
	case 0xb:					/*  BL: branch+link  */
		if (main_opcode == 0x0a) {
			ic->f = cond_instr(b);
			samepage_function = cond_instr(b_samepage);

			/*  Abort read-ahead on unconditional branches:  */
			if (condition_code == 0xe &&
			    cpu->translation_readahead > 1)
                                cpu->translation_readahead = 1;

			if (iword == 0xcaffffed)
				cpu->cd.arm.combination_check =
				    COMBINE(netbsd_memset);
			if (iword == 0xaafffff9)
				cpu->cd.arm.combination_check =
				    COMBINE(netbsd_memcpy);
		} else {
			if (cpu->machine->show_trace_tree) {
				ic->f = cond_instr(bl_trace);
				samepage_function =
				    cond_instr(bl_samepage_trace);
			} else {
				ic->f = cond_instr(bl);
				samepage_function = cond_instr(bl_samepage);
			}
		}

		/*  arg 1 = offset of current instruction  */
		/*  arg 2 = offset of the following instruction  */
		ic->arg[1] = addr & 0xffc;
		ic->arg[2] = (addr & 0xffc) + 4;

		ic->arg[0] = (iword & 0x00ffffff) << 2;
		/*  Sign-extend:  */
		if (ic->arg[0] & 0x02000000)
			ic->arg[0] |= 0xfc000000;
		/*
		 *  Branches are calculated as PC + 8 + offset.
		 */
		ic->arg[0] = (int32_t)(ic->arg[0] + 8);

		/*
		 *  Special case: branch within the same page:
		 *
		 *  arg[0] = addr of the arm_instr_call of the target
		 *  arg[1] = addr of the next arm_instr_call.
		 */
		{
			uint32_t mask_within_page =
			    ((ARM_IC_ENTRIES_PER_PAGE-1) <<
			    ARM_INSTR_ALIGNMENT_SHIFT) |
			    ((1 << ARM_INSTR_ALIGNMENT_SHIFT) - 1);
			uint32_t old_pc = addr;
			uint32_t new_pc = old_pc + (int32_t)ic->arg[0];
			if ((old_pc & ~mask_within_page) ==
			    (new_pc & ~mask_within_page)) {
				ic->f = samepage_function;
				ic->arg[0] = (size_t) (
				    cpu->cd.arm.cur_ic_page +
				    ((new_pc & mask_within_page) >>
				    ARM_INSTR_ALIGNMENT_SHIFT));
				ic->arg[1] = (size_t) (
				    cpu->cd.arm.cur_ic_page +
				    (((addr & mask_within_page) + 4) >>
				    ARM_INSTR_ALIGNMENT_SHIFT));
			} else if (main_opcode == 0x0a) {
				/*  Special hack for a plain "b":  */
				ic->arg[0] += ic->arg[1];
			}
		}

		if (main_opcode == 0xa && (condition_code <= 1
		    || condition_code == 3 || condition_code == 8
		    || condition_code == 12 || condition_code == 13))
			cpu->cd.arm.combination_check = COMBINE(beq_etc);

		if (iword == 0x1afffffc)
			cpu->cd.arm.combination_check = COMBINE(strlen);

		/*  Hm. Does this really increase performance?  */
		if (iword == 0x8afffffa)
			cpu->cd.arm.combination_check =
			    COMBINE(netbsd_cacheclean2);
		break;

	case 0xc:
	case 0xd:
		/*
		 *  xxxx1100 0100nnnn ddddcccc oooommmm    MCRR c,op,Rd,Rn,CRm
		 *  xxxx1100 0101nnnn ddddcccc oooommmm    MRRC c,op,Rd,Rn,CRm
		 */
		if ((iword & 0x0fe00fff) == 0x0c400000) {
			/*  Special case: mar/mra DSP instructions  */
			if (!cpu->translation_readahead)
				fatal("TODO: mar/mra DSP instructions!\n");
			/*  Perhaps these are actually identical to MCRR/MRRC */
			goto bad;
		}

		if ((iword & 0x0fe00000) == 0x0c400000) {
			if (!cpu->translation_readahead)
				fatal("MCRR/MRRC: TODO\n");
			goto bad;
		}

		/*
		 *  TODO: LDC/STC
		 *
		 *  For now, treat as Undefined instructions. This causes e.g.
		 *  Linux/ARM to emulate these instructions (floating point).
		 */
#if 1
		ic->f = cond_instr(und);
		ic->arg[0] = addr & 0xfff;
#else
		if (!cpu->translation_readahead)
			fatal("LDC/STC: TODO\n");
		goto bad;
#endif
		break;

	case 0xe:
		if ((iword & 0x0ff00ff0) == 0x0e200010) {
			/*  Special case: mia* DSP instructions  */
			/*  See Intel's 27343601.pdf, page 16-20  */
			if (!cpu->translation_readahead)
				fatal("TODO: mia* DSP instructions!\n");
			goto bad;
		}
		if (iword & 0x10) {
			/*  xxxx1110 oooLNNNN ddddpppp qqq1MMMM  MCR/MRC  */
			ic->arg[0] = iword;
			ic->f = cond_instr(mcr_mrc);
		} else {
			/*  xxxx1110 oooonnnn ddddpppp qqq0mmmm  CDP  */
			ic->arg[0] = iword;
			ic->f = cond_instr(cdp);
		}
		if (iword == 0xee070f9a)
			cpu->cd.arm.combination_check =
			    COMBINE(netbsd_cacheclean);
		break;

	case 0xf:
		/*  SWI:  */
		/*  Default handler:  */
		ic->f = cond_instr(swi);
		ic->arg[0] = addr & 0xfff;
		if (iword == 0xef8c64eb) {
			/*  Hack for rebooting a machine:  */
			ic->f = instr(reboot);
		} else if (iword == 0xef8c64be) {
			/*  Hack for openfirmware prom emulation:  */
			ic->f = instr(openfirmware);
		}
		break;

	default:goto bad;
	}

okay:

#define	DYNTRANS_TO_BE_TRANSLATED_TAIL
#include "cpu_dyntrans.cc" 
#undef	DYNTRANS_TO_BE_TRANSLATED_TAIL
}

