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
 *  Common dyntrans routines. Included from cpu_*.c.
 *
 *  Note: This might be a bit hard to follow, if you are reading this source
 *  code for the first time. It is basically a hack to implement "templates"
 *  with normal C code, by using suitable defines/macros, and then including
 *  this file.
 */


#ifndef STATIC_STUFF
#define	STATIC_STUFF
/*
 *  gather_statistics():
 */
static void gather_statistics(struct cpu *cpu)
{
	char ch, buf[60];
	struct DYNTRANS_IC *ic = cpu->cd.DYNTRANS_ARCH.next_ic;
	int i = 0;
	uint64_t a;
	int low_pc = ((size_t)cpu->cd.DYNTRANS_ARCH.next_ic - (size_t)
	    cpu->cd.DYNTRANS_ARCH.cur_ic_page) / sizeof(struct DYNTRANS_IC);

	if (cpu->machine->statistics.file == NULL) {
		fatal("statistics gathering with no filename set is"
		    " meaningless\n");
		return;
	}

	/*  low_pc must be within the page!  */
	if (low_pc < 0 || low_pc > DYNTRANS_IC_ENTRIES_PER_PAGE)
		return;

	buf[0] = '\0';

	while ((ch = cpu->machine->statistics.fields[i]) != '\0') {
		if (i != 0)
			strlcat(buf, " ", sizeof(buf));

		switch (ch) {
		case 'i':
			snprintf(buf + strlen(buf), sizeof(buf),
			    "%p", (void *)ic->f);
			break;
		case 'p':
			/*  Physical program counter address:  */
			cpu->cd.DYNTRANS_ARCH.cur_physpage = (struct DYNTRANS_TC_PHYSPAGE *)
			    cpu->cd.DYNTRANS_ARCH.cur_ic_page;
			a = cpu->cd.DYNTRANS_ARCH.cur_physpage->physaddr;
			a &= ~((DYNTRANS_IC_ENTRIES_PER_PAGE-1) <<
			    DYNTRANS_INSTR_ALIGNMENT_SHIFT);
			a += low_pc << DYNTRANS_INSTR_ALIGNMENT_SHIFT;
			if (cpu->is_32bit)
				snprintf(buf + strlen(buf), sizeof(buf),
				    "0x%08"PRIx32, (uint32_t)a);
			else
				snprintf(buf + strlen(buf), sizeof(buf),
				    "0x%016"PRIx64, (uint64_t)a);
			break;
		case 'v':
			/*  Virtual program counter address:  */
			a = cpu->pc;
			a &= ~((DYNTRANS_IC_ENTRIES_PER_PAGE-1) <<
			    DYNTRANS_INSTR_ALIGNMENT_SHIFT);
			a += low_pc << DYNTRANS_INSTR_ALIGNMENT_SHIFT;
			if (cpu->is_32bit)
				snprintf(buf + strlen(buf), sizeof(buf),
				    "0x%08"PRIx32, (uint32_t)a);
			else
				snprintf(buf + strlen(buf), sizeof(buf),
				    "0x%016"PRIx64, (uint64_t)a);
			break;
		}
		i++;
	}

	fprintf(cpu->machine->statistics.file, "%s\n", buf);
}


#define S		gather_statistics(cpu)


#if 1

/*  The normal instruction execution core:  */
#define I	ic = cpu->cd.DYNTRANS_ARCH.next_ic ++; ic->f(cpu, ic);

#else

/*  For heavy debugging:  */
#define I	ic = cpu->cd.DYNTRANS_ARCH.next_ic ++;	\
		{	\
			int low_pc = ((size_t)cpu->cd.DYNTRANS_ARCH.next_ic - \
			    (size_t)cpu->cd.DYNTRANS_ARCH.cur_ic_page) / \
			    sizeof(struct DYNTRANS_IC);			\
			printf("cur_ic_page=%p ic=%p (low_pc=0x%x)\n",	\
			    cpu->cd.DYNTRANS_ARCH.cur_ic_page,		\
			    ic, low_pc << DYNTRANS_INSTR_ALIGNMENT_SHIFT); \
		} \
		ic->f(cpu, ic);

#endif

/*  static long long nr_of_I_calls = 0;  */

/*  Temporary hack for finding NULL bugs:  */
/*  #define I	ic = cpu->cd.DYNTRANS_ARCH.next_ic ++; 			\
		nr_of_I_calls ++;					\
		if (ic->f == NULL) {					\
			int low_pc = ((size_t)cpu->cd.DYNTRANS_ARCH.next_ic - \
			    (size_t)cpu->cd.DYNTRANS_ARCH.cur_ic_page) / \
			    sizeof(struct DYNTRANS_IC);			\
			cpu->pc &= ~((DYNTRANS_IC_ENTRIES_PER_PAGE-1) << \
			    DYNTRANS_INSTR_ALIGNMENT_SHIFT);		\
			cpu->pc += (low_pc << DYNTRANS_INSTR_ALIGNMENT_SHIFT);\
			printf("Crash at %016"PRIx64"\n", cpu->pc);	\
			printf("nr of I calls: %lli\n", nr_of_I_calls);	\
			printf("Next ic = %p\n", cpu->cd.		\
				DYNTRANS_ARCH.next_ic);			\
			printf("cur ic page = %p\n", cpu->cd.		\
				DYNTRANS_ARCH.cur_ic_page);		\
			cpu->running = 0;				\
			return 0;					\
		}							\
		ic->f(cpu, ic);  */

/*  Temporary hack for MIPS, to hunt for 32-bit/64-bit sign-extension bugs:  */
/*  #define I		{ int k; for (k=1; k<=31; k++)	\
	cpu->cd.mips.gpr[k] = (int32_t)cpu->cd.mips.gpr[k];\
	if (cpu->cd.mips.gpr[0] != 0) {			\
		fatal("NOOOOOO\n"); exit(1);		\
	}						\
	ic = cpu->cd.DYNTRANS_ARCH.next_ic ++; ic->f(cpu, ic); }
*/
#endif	/*  STATIC STUFF  */



#ifdef	DYNTRANS_RUN_INSTR_DEF
/*
 *  XXX_run_instr():
 *
 *  Execute one or more instructions on a specific CPU, using dyntrans.
 *  (For dualmode archs, this function is included twice.)
 *
 *  Return value is the number of instructions executed during this call,
 *  0 if no instructions were executed.
 */
int DYNTRANS_RUN_INSTR_DEF(struct cpu *cpu)
{
	MODE_uint_t cached_pc;
	int low_pc, n_instrs;

	/*  Ugly... fix this some day.  */
#ifdef DYNTRANS_DUALMODE_32
#ifdef MODE32
	DYNTRANS_PC_TO_POINTERS32(cpu);
#else
	DYNTRANS_PC_TO_POINTERS(cpu);
#endif
#else
	DYNTRANS_PC_TO_POINTERS(cpu);
#endif

	/*
	 *  Interrupt assertion?  (This is _below_ the initial PC to pointer
	 *  conversion; if the conversion caused an exception of some kind
	 *  then interrupts are probably disabled, and the exception will get
	 *  priority over device interrupts.)
	 *
	 *  TODO: Turn this into a family-specific function somewhere...
 	 */

	/*  Note: Do not cause interrupts while single-stepping. It is
	    so horribly annoying.  */
	if (!single_step) {
#ifdef DYNTRANS_ARM
		if (cpu->cd.arm.irq_asserted && !(cpu->cd.arm.cpsr & ARM_FLAG_I))
			arm_exception(cpu, ARM_EXCEPTION_IRQ);
#endif
#ifdef DYNTRANS_M88K
		if (cpu->cd.m88k.irq_asserted &&
		    !(cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_IND))
			m88k_exception(cpu, M88K_EXCEPTION_INTERRUPT, 0);
#endif
#ifdef DYNTRANS_MIPS
		int enabled, mask;
		int status = cpu->cd.mips.coproc[0]->reg[COP0_STATUS];
		if (cpu->cd.mips.cpu_type.exc_model == EXC3K) {
			/*  R3000:  */
			enabled = status & MIPS_SR_INT_IE;
		} else {
			/*  R4000 and others:  */
			enabled = (status & STATUS_IE)
			    && !(status & STATUS_EXL) && !(status & STATUS_ERL);
			/*  Special case for R5900/C790/TX79:  */
			if (cpu->cd.mips.cpu_type.rev == MIPS_R5900 &&
			    !(status & R5900_STATUS_EIE))
				enabled = 0;
		}
		mask = status & cpu->cd.mips.coproc[0]->reg[COP0_CAUSE]
		    & STATUS_IM_MASK;

		if (enabled && mask)
			mips_cpu_exception(cpu, EXCEPTION_INT, 0, 0, 0, 0, 0,0);
#endif
#ifdef DYNTRANS_PPC
		if (cpu->cd.ppc.dec_intr_pending && cpu->cd.ppc.msr & PPC_MSR_EE) {
			if (!(cpu->cd.ppc.cpu_type.flags & PPC_NO_DEC))
				ppc_exception(cpu, PPC_EXCEPTION_DEC);
			cpu->cd.ppc.dec_intr_pending = 0;
		}
		if (cpu->cd.ppc.irq_asserted && cpu->cd.ppc.msr & PPC_MSR_EE)
			ppc_exception(cpu, PPC_EXCEPTION_EI);
#endif
#ifdef DYNTRANS_SH
		if (cpu->cd.sh.int_to_assert > 0 && !(cpu->cd.sh.sr & SH_SR_BL)
		    && ((cpu->cd.sh.sr & SH_SR_IMASK) >> SH_SR_IMASK_SHIFT)
		    < cpu->cd.sh.int_level)
			sh_exception(cpu, 0, cpu->cd.sh.int_to_assert, 0);
#endif
	}

	cached_pc = cpu->pc;

	cpu->n_translated_instrs = 0;

	cpu->cd.DYNTRANS_ARCH.cur_physpage = (struct DYNTRANS_TC_PHYSPAGE *)
	    cpu->cd.DYNTRANS_ARCH.cur_ic_page;

	if (single_step || cpu->machine->instruction_trace
	    || cpu->machine->register_dump) {
		/*
		 *  Single-step:
		 */
		struct DYNTRANS_IC *ic = cpu->cd.DYNTRANS_ARCH.next_ic;
		if (cpu->machine->register_dump) {
			debug("\n");
			cpu_register_dump(cpu->machine, cpu, 1, 0x1);
		}
		if (cpu->machine->instruction_trace) {
			/*  TODO/Note: This must be large enough to hold
			    any instruction for any ISA:  */
			unsigned char instr[1 <<
			    DYNTRANS_INSTR_ALIGNMENT_SHIFT];
			if (!cpu->memory_rw(cpu, cpu->mem, cached_pc, &instr[0],
			    sizeof(instr), MEM_READ, CACHE_INSTRUCTION)) {
				fatal("XXX_run_instr(): could not read "
				    "the instruction\n");
			} else {
#ifdef DYNTRANS_DELAYSLOT
				int len =
#endif
				    cpu_disassemble_instr(
				    cpu->machine, cpu, instr, 1, 0);
#ifdef DYNTRANS_DELAYSLOT
				/*  Show the instruction in the delay slot,
				    if any:  */
				if (cpu->instruction_has_delayslot == NULL)
					fatal("WARNING: ihd func not yet"
					    " implemented?\n");
				else if (cpu->instruction_has_delayslot(cpu,
				    instr)) {
					int saved_delayslot = cpu->delay_slot;
					cpu->memory_rw(cpu, cpu->mem, cached_pc
					    + len, &instr[0],
					    sizeof(instr), MEM_READ,
					    CACHE_INSTRUCTION);
					cpu->delay_slot = DELAYED;
					cpu->pc += len;
					cpu_disassemble_instr(cpu->machine,
					    cpu, instr, 1, 0);
					cpu->delay_slot = saved_delayslot;
					cpu->pc -= len;
				}
#endif
			}
		}

		if (cpu->machine->statistics.enabled)
			S;

		/*  Execute just one instruction:  */
		I;

		n_instrs = 1;
	} else if (cpu->machine->statistics.enabled) {
		/*  Gather statistics while executing multiple instructions:  */
		n_instrs = 0;
		for (;;) {
			struct DYNTRANS_IC *ic;

			S; I; S; I; S; I; S; I; S; I; S; I;
			S; I; S; I; S; I; S; I; S; I; S; I;
			S; I; S; I; S; I; S; I; S; I; S; I;
			S; I; S; I; S; I; S; I; S; I; S; I;

			n_instrs += 24;

			if (n_instrs + cpu->n_translated_instrs >=
			    N_SAFE_DYNTRANS_LIMIT)
				break;
		}
	} else {
		/*
		 *  Execute multiple instructions:
		 *
		 *  (This is the core dyntrans loop.)
		 */
		n_instrs = 0;

		for (;;) {
			struct DYNTRANS_IC *ic;

			I; I; I; I; I;   I; I; I; I; I;
			I; I; I; I; I;   I; I; I; I; I;
			I; I; I; I; I;   I; I; I; I; I;
			I; I; I; I; I;   I; I; I; I; I;
			I; I; I; I; I;   I; I; I; I; I;

			I; I; I; I; I;   I; I; I; I; I;

			I; I; I; I; I;   I; I; I; I; I;
			I; I; I; I; I;   I; I; I; I; I;
			I; I; I; I; I;   I; I; I; I; I;
			I; I; I; I; I;   I; I; I; I; I;
			I; I; I; I; I;   I; I; I; I; I;

			I; I; I; I; I;   I; I; I; I; I;

			cpu->n_translated_instrs += 120;
			if (cpu->n_translated_instrs >= N_SAFE_DYNTRANS_LIMIT)
				break;
		}
	}

	n_instrs += cpu->n_translated_instrs;

	/*  Synchronize the program counter:  */
	low_pc = ((size_t)cpu->cd.DYNTRANS_ARCH.next_ic - (size_t)
	    cpu->cd.DYNTRANS_ARCH.cur_ic_page) / sizeof(struct DYNTRANS_IC);
	if (low_pc >= 0 && low_pc < DYNTRANS_IC_ENTRIES_PER_PAGE) {
		cpu->pc &= ~((DYNTRANS_IC_ENTRIES_PER_PAGE-1) <<
		    DYNTRANS_INSTR_ALIGNMENT_SHIFT);
		cpu->pc += (low_pc << DYNTRANS_INSTR_ALIGNMENT_SHIFT);
	} else if (low_pc == DYNTRANS_IC_ENTRIES_PER_PAGE) {
		/*  Switch to next page:  */
		cpu->pc &= ~((DYNTRANS_IC_ENTRIES_PER_PAGE-1) <<
		    DYNTRANS_INSTR_ALIGNMENT_SHIFT);
		cpu->pc += (DYNTRANS_IC_ENTRIES_PER_PAGE <<
		    DYNTRANS_INSTR_ALIGNMENT_SHIFT);
	} else if (low_pc == DYNTRANS_IC_ENTRIES_PER_PAGE + 1) {
		/*  Switch to next page and skip an instruction which was
		    already executed (in a delay slot):  */
		cpu->pc &= ~((DYNTRANS_IC_ENTRIES_PER_PAGE-1) <<
		    DYNTRANS_INSTR_ALIGNMENT_SHIFT);
		cpu->pc += ((DYNTRANS_IC_ENTRIES_PER_PAGE + 1) <<
		    DYNTRANS_INSTR_ALIGNMENT_SHIFT);
	}

#ifdef DYNTRANS_MIPS
	/*  Update the count register (on everything except EXC3K):  */
	if (cpu->cd.mips.cpu_type.exc_model != EXC3K) {
		uint32_t old = cpu->cd.mips.coproc[0]->reg[COP0_COUNT];
		int32_t diff1 = cpu->cd.mips.coproc[0]->reg[COP0_COMPARE] - old;
		int32_t diff2;
		cpu->cd.mips.coproc[0]->reg[COP0_COUNT] =
		    (int32_t) (old + n_instrs);
		diff2 = cpu->cd.mips.coproc[0]->reg[COP0_COMPARE] -
		    cpu->cd.mips.coproc[0]->reg[COP0_COUNT];

		if (cpu->cd.mips.compare_register_set) {
#if 1
/*  Not yet.  TODO  */
			if (cpu->machine->emulated_hz > 0) {
				if (cpu->cd.mips.compare_interrupts_pending > 0)
					INTERRUPT_ASSERT(
					    cpu->cd.mips.irq_compare);
			} else
#endif
			{
				if (diff1 > 0 && diff2 <= 0)
					INTERRUPT_ASSERT(
					    cpu->cd.mips.irq_compare);
			}
		}
	}
#endif
#ifdef DYNTRANS_PPC
	/*  Update the Decrementer and Time base registers:  */
	{
		uint32_t old = cpu->cd.ppc.spr[SPR_DEC];
		cpu->cd.ppc.spr[SPR_DEC] = (uint32_t) (old - n_instrs);
		if ((old >> 31) == 0 && (cpu->cd.ppc.spr[SPR_DEC] >> 31) == 1
		    && !(cpu->cd.ppc.cpu_type.flags & PPC_NO_DEC))
			cpu->cd.ppc.dec_intr_pending = 1;
		old = cpu->cd.ppc.spr[SPR_TBL];
		cpu->cd.ppc.spr[SPR_TBL] += n_instrs;
		if ((old >> 31) == 1 && (cpu->cd.ppc.spr[SPR_TBL] >> 31) == 0)
			cpu->cd.ppc.spr[SPR_TBU] ++;
	}
#endif

	cpu->ninstrs += n_instrs;

	/*  Return the nr of instructions executed:  */
	return n_instrs;
}
#endif	/*  DYNTRANS_RUN_INSTR  */



#ifdef DYNTRANS_FUNCTION_TRACE_DEF
/*
 *  XXX_cpu_functioncall_trace():
 *
 *  Without this function, the main trace tree function prints something
 *  like    <f()>  or  <0x1234()>   on a function call. It is up to this
 *  function to print the arguments passed.
 */
void DYNTRANS_FUNCTION_TRACE_DEF(struct cpu *cpu, int n_args)
{
	int show_symbolic_function_name = 1;
        char strbuf[50];
	char *symbol;
	uint64_t ot;
	int x, print_dots = 1, n_args_to_print =
#if defined(DYNTRANS_SH) || defined(DYNTRANS_M88K)
	    8	/*  Both for 32-bit and 64-bit SuperH, and M88K  */
#else
	    4	/*  Default value for most archs  */
#endif
	    ;

	if (n_args >= 0 && n_args <= n_args_to_print) {
		print_dots = 0;
		n_args_to_print = n_args;
	}

#ifdef DYNTRANS_M88K
	/*  Special hack for M88K userspace:  */
	if (!(cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_MODE))
		show_symbolic_function_name = 0;
#endif


	/*
	 *  TODO: The type of each argument should be taken from the symbol
	 *  table, in some way.
	 *
	 *  The code here does a kind of "heuristic guess" regarding what the
	 *  argument values might mean. Sometimes the output looks weird, but
	 *  usually it looks good enough.
	 *
	 *  Print ".." afterwards to show that there might be more arguments
	 *  than were passed in register.
	 */
	for (x=0; x<n_args_to_print; x++) {
		int64_t d = cpu->cd.DYNTRANS_ARCH.
#ifdef DYNTRANS_ARM
		    r[0
#endif
#ifdef DYNTRANS_MIPS
		    gpr[MIPS_GPR_A0
#endif
#ifdef DYNTRANS_M88K
		    r[2		/*  r2..r9  */
#endif
#ifdef DYNTRANS_PPC
		    gpr[3
#endif
#ifdef DYNTRANS_SH
		    r[4		/*  NetBSD seems to use 4? But 2 seems
					to be used by other code? TODO  */
#endif
		    + x];

		symbol = get_symbol_name(&cpu->machine->symbol_context, d, &ot);

		if (d > -256 && d < 256)
			fatal("%i", (int)d);
		else if (memory_points_to_string(cpu, cpu->mem, d, 1))
			fatal("\"%s\"", memory_conv_to_string(cpu,
			    cpu->mem, d, strbuf, sizeof(strbuf)));
		else if (symbol != NULL && ot == 0 &&
		    show_symbolic_function_name)
			fatal("&%s", symbol);
		else {
			if (cpu->is_32bit)
				fatal("0x%"PRIx32, (uint32_t)d);
			else
				fatal("0x%"PRIx64, (uint64_t)d);
		}

		if (x < n_args_to_print - 1)
			fatal(",");
	}

	if (print_dots)
		fatal(",..");
}
#endif	/*  DYNTRANS_FUNCTION_TRACE_DEF  */



#ifdef DYNTRANS_TC_ALLOCATE_DEFAULT_PAGE_DEF
/*
 *  XXX_tc_allocate_default_page():
 *
 *  Create a default page (with just pointers to instr(to_be_translated)
 *  at cpu->translation_cache_cur_ofs.
 */
static void DYNTRANS_TC_ALLOCATE_DEFAULT_PAGE_DEF(struct cpu *cpu,
	uint64_t physaddr)
{ 
	struct DYNTRANS_TC_PHYSPAGE *ppp;

	ppp = (struct DYNTRANS_TC_PHYSPAGE *)(cpu->translation_cache
	    + cpu->translation_cache_cur_ofs);

	/*  Copy the entire template page first:  */
	memcpy(ppp, cpu->cd.DYNTRANS_ARCH.physpage_template, sizeof(
	    struct DYNTRANS_TC_PHYSPAGE));

	ppp->physaddr = physaddr & ~(DYNTRANS_PAGESIZE - 1);

	cpu->translation_cache_cur_ofs += sizeof(struct DYNTRANS_TC_PHYSPAGE);

	cpu->translation_cache_cur_ofs --;
	cpu->translation_cache_cur_ofs |= 63;
	cpu->translation_cache_cur_ofs ++;
}
#endif	/*  DYNTRANS_TC_ALLOCATE_DEFAULT_PAGE_DEF  */



#ifdef DYNTRANS_PC_TO_POINTERS_FUNC
/*
 *  XXX_pc_to_pointers_generic():
 *
 *  Generic case. See DYNTRANS_PC_TO_POINTERS_FUNC below.
 */
void DYNTRANS_PC_TO_POINTERS_GENERIC(struct cpu *cpu)
{
#ifdef MODE32
	uint32_t
#else
	uint64_t
#endif
	    cached_pc = cpu->pc, physaddr = 0;
	uint32_t physpage_ofs;
	int ok, pagenr, table_index;
	uint32_t *physpage_entryp;
	struct DYNTRANS_TC_PHYSPAGE *ppp;

#ifdef MODE32
	int index = DYNTRANS_ADDR_TO_PAGENR(cached_pc);
#else
	const uint32_t mask1 = (1 << DYNTRANS_L1N) - 1;
	const uint32_t mask2 = (1 << DYNTRANS_L2N) - 1;
	const uint32_t mask3 = (1 << DYNTRANS_L3N) - 1;
	uint32_t x1, x2, x3;
	struct DYNTRANS_L2_64_TABLE *l2;
	struct DYNTRANS_L3_64_TABLE *l3;

	x1 = (cached_pc >> (64-DYNTRANS_L1N)) & mask1;
	x2 = (cached_pc >> (64-DYNTRANS_L1N-DYNTRANS_L2N)) & mask2;
	x3 = (cached_pc >> (64-DYNTRANS_L1N-DYNTRANS_L2N-DYNTRANS_L3N)) & mask3;
	/*  fatal("X3: cached_pc=%016"PRIx64" x1=%x x2=%x x3=%x\n",
	    (uint64_t)cached_pc, (int)x1, (int)x2, (int)x3);  */
	l2 = cpu->cd.DYNTRANS_ARCH.l1_64[x1];
	/*  fatal("  l2 = %p\n", l2);  */
	l3 = l2->l3[x2];
	/*  fatal("  l3 = %p\n", l3);  */
#endif

	/*  Virtual to physical address translation:  */
	ok = 0;
#ifdef MODE32
	if (cpu->cd.DYNTRANS_ARCH.host_load[index] != NULL) {
		physaddr = cpu->cd.DYNTRANS_ARCH.phys_addr[index];
		ok = 1;
	}
#else
	if (l3->host_load[x3] != NULL) {
		physaddr = l3->phys_addr[x3];
		ok = 1;
	}
#endif

	if (!ok) {
		uint64_t paddr;
		if (cpu->translate_v2p != NULL) {
			uint64_t vaddr =
#if defined(MODE32) && defined(DYNTRANS_MIPS)
			/*  32-bit MIPS is _sign_ extend, not zero.  */
			    (int32_t)
#endif
			    cached_pc;
			ok = cpu->translate_v2p(
			    cpu, vaddr, &paddr, FLAG_INSTR);
		} else {
			paddr = cached_pc;
			ok = 1;
		}
		if (!ok) {
			/*
			 *  The PC is now set to the exception handler.
			 *  Try to find the paddr in the translation arrays,
			 *  or if that fails, call translate_v2p for the
			 *  exception handler.
			 */
			/*  fatal("TODO: instruction vaddr=>paddr translation "
			    "failed. vaddr=0x%"PRIx64"\n", (uint64_t)cached_pc);
			fatal("!! cpu->pc=0x%"PRIx64"\n", (uint64_t)cpu->pc); */

			/*  If there was an exception, the PC has changed.
			    Update cached_pc:  */
			cached_pc = cpu->pc;

#ifdef MODE32
			index = DYNTRANS_ADDR_TO_PAGENR(cached_pc);
			if (cpu->cd.DYNTRANS_ARCH.host_load[index] != NULL) {
				paddr = cpu->cd.DYNTRANS_ARCH.phys_addr[index];
				ok = 1;
			}
#else
			x1 = (cached_pc >> (64-DYNTRANS_L1N)) & mask1;
			x2 = (cached_pc >> (64-DYNTRANS_L1N-DYNTRANS_L2N))
			    & mask2;
			x3 = (cached_pc >> (64-DYNTRANS_L1N-DYNTRANS_L2N
			    - DYNTRANS_L3N)) & mask3;
			l2 = cpu->cd.DYNTRANS_ARCH.l1_64[x1];
			l3 = l2->l3[x2];
			if (l3->host_load[x3] != NULL) {
				paddr = l3->phys_addr[x3];
				ok = 1;
			}
#endif

			if (!ok) {
				ok = cpu->translate_v2p(cpu, cpu->pc, &paddr,
				    FLAG_INSTR);
			}

			/*  printf("EXCEPTION HANDLER: vaddr = 0x%x ==> "
			    "paddr = 0x%x\n", (int)cpu->pc, (int)paddr);
			fatal("!? cpu->pc=0x%"PRIx64"\n", (uint64_t)cpu->pc); */

			if (!ok) {
				fatal("FATAL: could not find physical"
				    " address of the exception handler?");
				exit(1);
			}
		}

		physaddr = paddr;
	}

	physaddr &= ~(DYNTRANS_PAGESIZE - 1);

#ifdef MODE32
	if (cpu->cd.DYNTRANS_ARCH.host_load[index] == NULL) {
#else
	if (l3->host_load[x3] == NULL) {
#endif
		int q = DYNTRANS_PAGESIZE - 1;
		unsigned char *host_page = memory_paddr_to_hostaddr(cpu->mem,
		    physaddr, MEM_READ);
		if (host_page != NULL) {
			cpu->update_translation_table(cpu, cached_pc & ~q,
			    host_page, 0, physaddr);
		}
	}

	if (cpu->translation_cache_cur_ofs >= dyntrans_cache_size) {
#ifdef UNSTABLE_DEVEL
		fatal("[ dyntrans: resetting the translation cache ]\n");
#endif
		cpu_create_or_reset_tc(cpu);
	}

	pagenr = DYNTRANS_ADDR_TO_PAGENR(physaddr);
	table_index = PAGENR_TO_TABLE_INDEX(pagenr);

	physpage_entryp = &(((uint32_t *)cpu->translation_cache)[table_index]);
	physpage_ofs = *physpage_entryp;
	ppp = NULL;

	/*  Traverse the physical page chain:  */
	while (physpage_ofs != 0) {
		ppp = (struct DYNTRANS_TC_PHYSPAGE *)(cpu->translation_cache
		    + physpage_ofs);

		/*  If we found the page in the cache, then we're done:  */
		if (ppp->physaddr == physaddr)
			break;

		/*  Try the next page in the chain:  */
		physpage_ofs = ppp->next_ofs;
	}

	/*
	 *  If the offset is 0, then no translation exists yet for this
	 *  physical address. Let's create a new page, and add it first in
	 *  the chain.
	 */
	if (physpage_ofs == 0) {
		uint32_t previous_first_page_in_chain;

		/*  fatal("CREATING page %lli (physaddr 0x%"PRIx64"), table "
		    "index %i\n", (long long)pagenr, (uint64_t)physaddr,
		    (int)table_index);  */

		previous_first_page_in_chain = *physpage_entryp;

		/*  Insert the new page first in the chain:  */
		*physpage_entryp = physpage_ofs =
		    cpu->translation_cache_cur_ofs;

		/*  Allocate a default page, with to_be_translated entries:  */
		DYNTRANS_TC_ALLOCATE(cpu, physaddr);

		ppp = (struct DYNTRANS_TC_PHYSPAGE *)(cpu->translation_cache
		    + physpage_ofs);

		/*  Point to the other pages in the same chain:  */
		ppp->next_ofs = previous_first_page_in_chain;
	}

	/*  Here, ppp points to a valid physical page struct.  */

#ifdef MODE32
	if (cpu->cd.DYNTRANS_ARCH.host_load[index] != NULL)
		cpu->cd.DYNTRANS_ARCH.phys_page[index] = ppp;
#else
	if (l3->host_load[x3] != NULL)
		l3->phys_page[x3] = ppp;
#endif

	/*
	 *  If there are no translations yet on this page, then mark it
	 *  as non-writable. If there are already translations, then it
	 *  should already have been marked as non-writable.
	 */
	if (ppp->translations_bitmap == 0) {
		cpu->invalidate_translation_caches(cpu, physaddr,
		    JUST_MARK_AS_NON_WRITABLE | INVALIDATE_PADDR);
	}

	cpu->cd.DYNTRANS_ARCH.cur_ic_page = &ppp->ics[0];

	cpu->cd.DYNTRANS_ARCH.next_ic = cpu->cd.DYNTRANS_ARCH.cur_ic_page +
	    DYNTRANS_PC_TO_IC_ENTRY(cached_pc);

	/*  printf("cached_pc=0x%016"PRIx64"  pagenr=%lli  table_index=%lli, "
	    "physpage_ofs=0x%016"PRIx64"\n", (uint64_t)cached_pc, (long long)
	    pagenr, (long long)table_index, (uint64_t)physpage_ofs);  */
}


/*
 *  XXX_pc_to_pointers():
 *
 *  This function uses the current program counter (a virtual address) to
 *  find out which physical translation page to use, and then sets the current
 *  translation page pointers to that page.
 *
 *  If there was no translation page for that physical page, then an empty
 *  one is created.
 *
 *  NOTE: This is the quick lookup version. See
 *  DYNTRANS_PC_TO_POINTERS_GENERIC above for the generic case.
 */
void DYNTRANS_PC_TO_POINTERS_FUNC(struct cpu *cpu)
{
#ifdef MODE32
	uint32_t
#else
	uint64_t
#endif
	    cached_pc = cpu->pc;
	struct DYNTRANS_TC_PHYSPAGE *ppp;

#ifdef MODE32
	int index;
	index = DYNTRANS_ADDR_TO_PAGENR(cached_pc);
	ppp = cpu->cd.DYNTRANS_ARCH.phys_page[index];
	if (ppp != NULL)
		goto have_it;
#else
	const uint32_t mask1 = (1 << DYNTRANS_L1N) - 1;
	const uint32_t mask2 = (1 << DYNTRANS_L2N) - 1;
	const uint32_t mask3 = (1 << DYNTRANS_L3N) - 1;
	uint32_t x1, x2, x3;
	struct DYNTRANS_L2_64_TABLE *l2;
	struct DYNTRANS_L3_64_TABLE *l3;

	x1 = (cached_pc >> (64-DYNTRANS_L1N)) & mask1;
	x2 = (cached_pc >> (64-DYNTRANS_L1N-DYNTRANS_L2N)) & mask2;
	x3 = (cached_pc >> (64-DYNTRANS_L1N-DYNTRANS_L2N-DYNTRANS_L3N)) & mask3;
	l2 = cpu->cd.DYNTRANS_ARCH.l1_64[x1];
	l3 = l2->l3[x2];
	ppp = l3->phys_page[x3];
	if (ppp != NULL)
		goto have_it;
#endif

	DYNTRANS_PC_TO_POINTERS_GENERIC(cpu);
	return;

	/*  Quick return path:  */
have_it:
	cpu->cd.DYNTRANS_ARCH.cur_ic_page = &ppp->ics[0];
	cpu->cd.DYNTRANS_ARCH.next_ic = cpu->cd.DYNTRANS_ARCH.cur_ic_page +
	    DYNTRANS_PC_TO_IC_ENTRY(cached_pc);

	/*  printf("cached_pc=0x%016"PRIx64"  pagenr=%lli  table_index=%lli, "
	    "physpage_ofs=0x%016"PRIx64"\n", (uint64_t)cached_pc, (long long)
	    pagenr, (long long)table_index, (uint64_t)physpage_ofs);  */
}
#endif	/*  DYNTRANS_PC_TO_POINTERS_FUNC  */



#ifdef DYNTRANS_INIT_TABLES

/*  forward declaration of to_be_translated and end_of_page:  */
static void instr(to_be_translated)(struct cpu *, struct DYNTRANS_IC *);
static void instr(end_of_page)(struct cpu *,struct DYNTRANS_IC *);
#ifdef DYNTRANS_DUALMODE_32
static void instr32(to_be_translated)(struct cpu *, struct DYNTRANS_IC *);
static void instr32(end_of_page)(struct cpu *,struct DYNTRANS_IC *);
#endif

#ifdef DYNTRANS_DUALMODE_32
#define TO_BE_TRANSLATED    ( cpu->is_32bit? instr32(to_be_translated) : \
			      instr(to_be_translated) )
#else
#define TO_BE_TRANSLATED    ( instr(to_be_translated) )
#endif

#ifdef DYNTRANS_DELAYSLOT
static void instr(end_of_page2)(struct cpu *,struct DYNTRANS_IC *);
#ifdef DYNTRANS_DUALMODE_32
static void instr32(end_of_page2)(struct cpu *,struct DYNTRANS_IC *);
#endif
#endif

/*
 *  XXX_init_tables():
 *
 *  Initializes the default translation page (for newly allocated pages), and
 *  for 64-bit emulation it also initializes 64-bit dummy tables and pointers.
 */
void DYNTRANS_INIT_TABLES(struct cpu *cpu)
{
#ifndef MODE32
	struct DYNTRANS_L2_64_TABLE *dummy_l2;
	struct DYNTRANS_L3_64_TABLE *dummy_l3;
	int x1, x2;
#endif
	int i;
	struct DYNTRANS_TC_PHYSPAGE *ppp;

	CHECK_ALLOCATION(ppp =
	    (struct DYNTRANS_TC_PHYSPAGE *) malloc(sizeof(struct DYNTRANS_TC_PHYSPAGE)));

	ppp->next_ofs = 0;
	ppp->translations_bitmap = 0;
	ppp->translation_ranges_ofs = 0;
	/*  ppp->physaddr is filled in by the page allocator  */

	for (i=0; i<DYNTRANS_IC_ENTRIES_PER_PAGE; i++)
		ppp->ics[i].f = TO_BE_TRANSLATED;

	/*  End-of-page:  */
	ppp->ics[DYNTRANS_IC_ENTRIES_PER_PAGE + 0].f =
#ifdef DYNTRANS_DUALMODE_32
	    cpu->is_32bit? instr32(end_of_page) :
#endif
	    instr(end_of_page);

	/*  End-of-page-2, for delay-slot architectures:  */
#ifdef DYNTRANS_DELAYSLOT
	ppp->ics[DYNTRANS_IC_ENTRIES_PER_PAGE + 1].f =
#ifdef DYNTRANS_DUALMODE_32
	    cpu->is_32bit? instr32(end_of_page2) :
#endif
	    instr(end_of_page2);
#endif

	cpu->cd.DYNTRANS_ARCH.physpage_template = ppp;


	/*  Prepare 64-bit virtual address translation tables:  */
#ifndef MODE32
	if (cpu->is_32bit)
		return;

	dummy_l2 = (struct DYNTRANS_L2_64_TABLE *) zeroed_alloc(sizeof(struct DYNTRANS_L2_64_TABLE));
	dummy_l3 = (struct DYNTRANS_L3_64_TABLE *) zeroed_alloc(sizeof(struct DYNTRANS_L3_64_TABLE));

	cpu->cd.DYNTRANS_ARCH.l2_64_dummy = dummy_l2;
	cpu->cd.DYNTRANS_ARCH.l3_64_dummy = dummy_l3;

	for (x1 = 0; x1 < (1 << DYNTRANS_L1N); x1 ++)
		cpu->cd.DYNTRANS_ARCH.l1_64[x1] = dummy_l2;

	for (x2 = 0; x2 < (1 << DYNTRANS_L2N); x2 ++)
		dummy_l2->l3[x2] = dummy_l3;
#endif
}
#endif	/*  DYNTRANS_INIT_TABLES  */



#ifdef DYNTRANS_INVAL_ENTRY
/*
 *  XXX_invalidate_tlb_entry():
 *
 *  Invalidate one translation entry (based on virtual address).
 *
 *  If the JUST_MARK_AS_NON_WRITABLE flag is set, then the translation entry
 *  is just downgraded to non-writable (ie the host store page is set to
 *  NULL). Otherwise, the entire translation is removed.
 */
static void DYNTRANS_INVALIDATE_TLB_ENTRY(struct cpu *cpu,
#ifdef MODE32
	uint32_t
#else
	uint64_t
#endif
	vaddr_page, int flags)
{
#ifdef MODE32
	uint32_t index = DYNTRANS_ADDR_TO_PAGENR(vaddr_page);

#ifdef DYNTRANS_ARM
	cpu->cd.DYNTRANS_ARCH.is_userpage[index >> 5] &= ~(1 << (index & 31));
#endif

	if (flags & JUST_MARK_AS_NON_WRITABLE) {
		/*  printf("JUST MARKING NON-W: vaddr 0x%08x\n",
		    (int)vaddr_page);  */
		cpu->cd.DYNTRANS_ARCH.host_store[index] = NULL;
	} else {
		int tlbi = cpu->cd.DYNTRANS_ARCH.vaddr_to_tlbindex[index];
		cpu->cd.DYNTRANS_ARCH.host_load[index] = NULL;
		cpu->cd.DYNTRANS_ARCH.host_store[index] = NULL;
		cpu->cd.DYNTRANS_ARCH.phys_addr[index] = 0;
		cpu->cd.DYNTRANS_ARCH.phys_page[index] = NULL;
		if (tlbi > 0)
			cpu->cd.DYNTRANS_ARCH.vph_tlb_entry[tlbi-1].valid = 0;
		cpu->cd.DYNTRANS_ARCH.vaddr_to_tlbindex[index] = 0;
	}
#else
	const uint32_t mask1 = (1 << DYNTRANS_L1N) - 1;
	const uint32_t mask2 = (1 << DYNTRANS_L2N) - 1;
	const uint32_t mask3 = (1 << DYNTRANS_L3N) - 1;
	uint32_t x1, x2, x3;
	struct DYNTRANS_L2_64_TABLE *l2;
	struct DYNTRANS_L3_64_TABLE *l3;

	x1 = (vaddr_page >> (64-DYNTRANS_L1N)) & mask1;
	x2 = (vaddr_page >> (64-DYNTRANS_L1N-DYNTRANS_L2N)) & mask2;
	x3 = (vaddr_page >> (64-DYNTRANS_L1N-DYNTRANS_L2N-DYNTRANS_L3N))& mask3;

	l2 = cpu->cd.DYNTRANS_ARCH.l1_64[x1];
	if (l2 == cpu->cd.DYNTRANS_ARCH.l2_64_dummy)
		return;

	l3 = l2->l3[x2];
	if (l3 == cpu->cd.DYNTRANS_ARCH.l3_64_dummy)
		return;

	if (flags & JUST_MARK_AS_NON_WRITABLE) {
		l3->host_store[x3] = NULL;
		return;
	}

#ifdef BUGHUNT

{
	/*  Consistency check, for debugging:  */
	int x1, x1b; // x2, x3;
	struct DYNTRANS_L2_64_TABLE *l2;
	//struct DYNTRANS_L3_64_TABLE *l3;

	for (x1 = 0; x1 <= mask1; x1 ++) {
		l2 = cpu->cd.DYNTRANS_ARCH.l1_64[x1];
		if (l2 == cpu->cd.DYNTRANS_ARCH.l2_64_dummy)
			continue;
		/*  Make sure that this l2 isn't used more than 1 time!  */
		for (x1b = 0; x1b <= mask1; x1b ++)
			if (x1 != x1b &&
			    l2 == cpu->cd.DYNTRANS_ARCH.l1_64[x1b]) {
				fatal("L2 reuse: %p\n", l2);
				exit(1);
			}
	}
}

/*  Count how many pages are actually in use:  */
{
	int n=0, i;
	for (i=0; i<=mask3; i++)
		if (l3->vaddr_to_tlbindex[i])
			n++;
	if (n != l3->refcount) {
		printf("Z: %i in use, but refcount = %i!\n", n, l3->refcount);
		exit(1);
	}

	n = 0;
	for (i=0; i<=mask3; i++)
		if (l3->host_load[i] != NULL)
			n++;
	if (n != l3->refcount) {
		printf("ZHL: %i in use, but refcount = %i!\n", n, l3->refcount);
		exit(1);
	}
}
#endif

	l3->host_load[x3] = NULL;
	l3->host_store[x3] = NULL;
	l3->phys_addr[x3] = 0;
	l3->phys_page[x3] = NULL;
	if (l3->vaddr_to_tlbindex[x3] != 0) {
		cpu->cd.DYNTRANS_ARCH.vph_tlb_entry[
		    l3->vaddr_to_tlbindex[x3] - 1].valid = 0;
		l3->refcount --;
	}
	l3->vaddr_to_tlbindex[x3] = 0;

	if (l3->refcount < 0) {
		fatal("xxx_invalidate_tlb_entry(): huh? Refcount bug.\n");
		exit(1);
	}

	if (l3->refcount == 0) {
		l3->next = cpu->cd.DYNTRANS_ARCH.next_free_l3;
		cpu->cd.DYNTRANS_ARCH.next_free_l3 = l3;
		l2->l3[x2] = cpu->cd.DYNTRANS_ARCH.l3_64_dummy;

#ifdef BUGHUNT
/*  Make sure that we're placing a CLEAN page on the
    freelist:  */
{
	int i;
	for (i=0; i<=mask3; i++)
		if (l3->host_load[i] != NULL) {
			fatal("TRYING TO RETURN A NON-CLEAN L3 PAGE!\n");
			exit(1);
		}
}
#endif
		l2->refcount --;
		if (l2->refcount < 0) {
			fatal("xxx_invalidate_tlb_entry(): Refcount bug L2.\n");
			exit(1);
		}
		if (l2->refcount == 0) {
			l2->next = cpu->cd.DYNTRANS_ARCH.next_free_l2;
			cpu->cd.DYNTRANS_ARCH.next_free_l2 = l2;
			cpu->cd.DYNTRANS_ARCH.l1_64[x1] =
			    cpu->cd.DYNTRANS_ARCH.l2_64_dummy;
		}
	}
#endif
}
#endif


#ifdef DYNTRANS_INVALIDATE_TC
/*
 *  XXX_invalidate_translation_caches():
 *
 *  Invalidate all entries matching a specific physical address, a specific
 *  virtual address, or ALL entries.
 *
 *  flags should be one of
 *	INVALIDATE_PADDR  INVALIDATE_VADDR  or  INVALIDATE_ALL
 *
 *  In addition, for INVALIDATE_ALL, INVALIDATE_VADDR_UPPER4 may be set and
 *  bit 31..28 of addr are used to select the virtual addresses to invalidate.
 *  (This is useful for PowerPC emulation, when segment registers are updated.)
 *
 *  In the case when all translations are invalidated, paddr doesn't need
 *  to be supplied.
 *
 *  NOTE/TODO: When invalidating a virtual address, it is only cleared from
 *             the quick translation array, not from the linear
 *             vph_tlb_entry[] array.  Hopefully this is enough anyway.
 */
void DYNTRANS_INVALIDATE_TC(struct cpu *cpu, uint64_t addr, int flags)
{
	int r;
#ifdef MODE32
	uint32_t
#else
	uint64_t
#endif
	    addr_page = addr & ~(DYNTRANS_PAGESIZE - 1);

	/*  fatal("invalidate(): ");  */

	/*  Quick case for _one_ virtual addresses: see note above.  */
	if (flags & INVALIDATE_VADDR) {
		/*  fatal("vaddr 0x%08x\n", (int)addr_page);  */
		DYNTRANS_INVALIDATE_TLB_ENTRY(cpu, addr_page, flags);
		return;
	}

	/*  Invalidate everything:  */
#ifdef DYNTRANS_PPC
	if (flags & INVALIDATE_ALL && flags & INVALIDATE_VADDR_UPPER4) {
		/*  fatal("all, upper4 (PowerPC segment)\n");  */
		for (r=0; r<DYNTRANS_MAX_VPH_TLB_ENTRIES; r++) {
			if (cpu->cd.DYNTRANS_ARCH.vph_tlb_entry[r].valid &&
			    (cpu->cd.DYNTRANS_ARCH.vph_tlb_entry[r].vaddr_page
			    & 0xf0000000) == addr_page) {
				DYNTRANS_INVALIDATE_TLB_ENTRY(cpu, cpu->cd.
				    DYNTRANS_ARCH.vph_tlb_entry[r].vaddr_page,
				    0);
				cpu->cd.DYNTRANS_ARCH.vph_tlb_entry[r].valid=0;
			}
		}
		return;
	}
#endif
	if (flags & INVALIDATE_ALL) {
		/*  fatal("all\n");  */
		for (r=0; r<DYNTRANS_MAX_VPH_TLB_ENTRIES; r++) {
			if (cpu->cd.DYNTRANS_ARCH.vph_tlb_entry[r].valid) {
				DYNTRANS_INVALIDATE_TLB_ENTRY(cpu, cpu->cd.
				    DYNTRANS_ARCH.vph_tlb_entry[r].vaddr_page,
				    0);
				cpu->cd.DYNTRANS_ARCH.vph_tlb_entry[r].valid=0;
			}
		}
		return;
	}

	/*  Invalidate a physical page:  */

	if (!(flags & INVALIDATE_PADDR))
		fatal("HUH? Invalidate: Not vaddr, all, or paddr?\n");

	/*  fatal("addr 0x%08x\n", (int)addr_page);  */

	for (r=0; r<DYNTRANS_MAX_VPH_TLB_ENTRIES; r++) {
		if (cpu->cd.DYNTRANS_ARCH.vph_tlb_entry[r].valid && addr_page
		    == cpu->cd.DYNTRANS_ARCH.vph_tlb_entry[r].paddr_page) {
			DYNTRANS_INVALIDATE_TLB_ENTRY(cpu,
			    cpu->cd.DYNTRANS_ARCH.vph_tlb_entry[r].vaddr_page,
			    flags);
			if (flags & JUST_MARK_AS_NON_WRITABLE)
				cpu->cd.DYNTRANS_ARCH.vph_tlb_entry[r]
				    .writeflag = 0;
			else
				cpu->cd.DYNTRANS_ARCH.vph_tlb_entry[r]
				    .valid = 0;
		}
	}
}
#endif	/*  DYNTRANS_INVALIDATE_TC  */



#ifdef DYNTRANS_INVALIDATE_TC_CODE
/*
 *  XXX_invalidate_code_translation():
 *
 *  Invalidate code translations for a specific physical address, a specific
 *  virtual address, or for all entries in the cache.
 */
void DYNTRANS_INVALIDATE_TC_CODE(struct cpu *cpu, uint64_t addr, int flags)
{
	int r;
#ifdef MODE32
	uint32_t
#else
	uint64_t
#endif
	    vaddr_page, paddr_page;

	addr &= ~(DYNTRANS_PAGESIZE-1);

	/*  printf("DYNTRANS_INVALIDATE_TC_CODE addr=0x%08x flags=%i\n",
	    (int)addr, flags);  */

	if (flags & INVALIDATE_PADDR) {
		int pagenr, table_index;
		uint32_t physpage_ofs, *physpage_entryp;
		struct DYNTRANS_TC_PHYSPAGE *ppp, *prev_ppp;

		pagenr = DYNTRANS_ADDR_TO_PAGENR(addr);
		table_index = PAGENR_TO_TABLE_INDEX(pagenr);

		physpage_entryp = &(((uint32_t *)cpu->
		    translation_cache)[table_index]);
		physpage_ofs = *physpage_entryp;

		/*  Return immediately if there is no code translation
		    for this page.  */
		if (physpage_ofs == 0)
			return;

		prev_ppp = ppp = NULL;

		/*  Traverse the physical page chain:  */
		while (physpage_ofs != 0) {
			prev_ppp = ppp;
			ppp = (struct DYNTRANS_TC_PHYSPAGE *)
			    (cpu->translation_cache + physpage_ofs);

			/*  If we found the page in the cache,
			    then we're done:  */
			if (ppp->physaddr == addr)
				break;

			/*  Try the next page in the chain:  */
			physpage_ofs = ppp->next_ofs;
		}

		/*  If there is no translation, there is no need to go
		    on and try to remove it from the vph_tlb_entry array:  */
		if (physpage_ofs == 0)
			return;

#if 0
		/*
		 *  "Bypass" the page, removing it from the code cache.
		 *
		 *  NOTE/TODO: This gives _TERRIBLE_ performance with self-
		 *  modifying code, or when a single page is used for both
		 *  code and (writable) data.
		 */
		if (ppp != NULL) {
			if (prev_ppp != NULL)
				prev_ppp->next_ofs = ppp->next_ofs;
			else
				*physpage_entryp = ppp->next_ofs;
		}
#else
		/*
		 *  Instead of removing the page from the code cache, each
		 *  entry can be set to "to_be_translated". This is slow in
		 *  the general case, but in the case of self-modifying code,
		 *  it might be faster since we don't risk wasting cache
		 *  memory as quickly (which would force unnecessary Restarts).
		 */
		if (ppp != NULL && ppp->translations_bitmap != 0) {
			uint32_t x = ppp->translations_bitmap;	/*  TODO:
				urk Should be same type as the bitmap */
			int i, j, n, m;

#ifdef DYNTRANS_ARM
			/*
			 *  Note: On ARM, PC-relative load instructions are
			 *  implemented as immediate mov instructions. When
			 *  setting parts of the page to "to be translated",
			 *  we cannot keep track of which of the immediate
			 *  movs that were affected, so we need to clear
			 *  the entire page. (ARM only; not for the general
			 *  case.)
			 */
			x = 0xffffffff;
#endif
			n = 8 * sizeof(x);
			m = DYNTRANS_IC_ENTRIES_PER_PAGE / n;

			for (i=0; i<n; i++) {
				if (x & 1) {
					for (j=0; j<m; j++)
						ppp->ics[i*m + j].f =
						    TO_BE_TRANSLATED;
				}

				x >>= 1;
			}

			ppp->translations_bitmap = 0;

			/*  Clear the list of translatable ranges:  */
			if (ppp->translation_ranges_ofs != 0) {
				struct physpage_ranges *physpage_ranges =
				    (struct physpage_ranges *)
				    (cpu->translation_cache +
				    ppp->translation_ranges_ofs);
				physpage_ranges->next_ofs = 0;
				physpage_ranges->n_entries_used = 0;
			}
		}
#endif
	}

	/*  Invalidate entries in the VPH table:  */
	for (r = 0; r < DYNTRANS_MAX_VPH_TLB_ENTRIES; r ++) {
		if (cpu->cd.DYNTRANS_ARCH.vph_tlb_entry[r].valid) {
			vaddr_page = cpu->cd.DYNTRANS_ARCH.vph_tlb_entry[r]
			    .vaddr_page & ~(DYNTRANS_PAGESIZE-1);
			paddr_page = cpu->cd.DYNTRANS_ARCH.vph_tlb_entry[r]
			    .paddr_page & ~(DYNTRANS_PAGESIZE-1);

			if (flags & INVALIDATE_ALL ||
			    (flags & INVALIDATE_PADDR && paddr_page == addr) ||
			    (flags & INVALIDATE_VADDR && vaddr_page == addr)) {
#ifdef MODE32
				uint32_t index =
				    DYNTRANS_ADDR_TO_PAGENR(vaddr_page);
				cpu->cd.DYNTRANS_ARCH.phys_page[index] = NULL;
#else
				const uint32_t mask1 = (1 << DYNTRANS_L1N) - 1;
				const uint32_t mask2 = (1 << DYNTRANS_L2N) - 1;
				const uint32_t mask3 = (1 << DYNTRANS_L3N) - 1;
				uint32_t x1, x2, x3;
				struct DYNTRANS_L2_64_TABLE *l2;
				struct DYNTRANS_L3_64_TABLE *l3;

				x1 = (vaddr_page >> (64-DYNTRANS_L1N)) & mask1;
				x2 = (vaddr_page >> (64-DYNTRANS_L1N -
				    DYNTRANS_L2N)) & mask2;
				x3 = (vaddr_page >> (64-DYNTRANS_L1N -
				    DYNTRANS_L2N - DYNTRANS_L3N)) & mask3;
				l2 = cpu->cd.DYNTRANS_ARCH.l1_64[x1];
				l3 = l2->l3[x2];
				l3->phys_page[x3] = NULL;
#endif
			}
		}
	}
}
#endif	/*  DYNTRANS_INVALIDATE_TC_CODE  */



#ifdef DYNTRANS_UPDATE_TRANSLATION_TABLE
/*
 *  XXX_update_translation_table():
 *
 *  Update the virtual memory translation tables.
 */
void DYNTRANS_UPDATE_TRANSLATION_TABLE(struct cpu *cpu, uint64_t vaddr_page,
	unsigned char *host_page, int writeflag, uint64_t paddr_page)
{
	int found, r, useraccess = 0;

#ifdef MODE32
	uint32_t index;
	vaddr_page &= 0xffffffffULL;

	if (paddr_page > 0xffffffffULL) {
		fatal("update_translation_table(): v=0x%016"PRIx64", h=%p w=%i"
		    " p=0x%016"PRIx64"\n", vaddr_page, host_page, writeflag,
		    paddr_page);
		exit(1);
	}

	/*  fatal("update_translation_table(): v=0x%x, h=%p w=%i"
	    " p=0x%x\n", (int)vaddr_page, host_page, writeflag,
	    (int)paddr_page);  */
#else	/*  !MODE32  */
	const uint32_t mask1 = (1 << DYNTRANS_L1N) - 1;
	const uint32_t mask2 = (1 << DYNTRANS_L2N) - 1;
	const uint32_t mask3 = (1 << DYNTRANS_L3N) - 1;
	uint32_t x1, x2, x3;
	struct DYNTRANS_L2_64_TABLE *l2;
	struct DYNTRANS_L3_64_TABLE *l3;

	/*  fatal("update_translation_table(): v=0x%016"PRIx64", h=%p w=%i"
	    " p=0x%016"PRIx64"\n", (uint64_t)vaddr_page, host_page, writeflag,
	    (uint64_t)paddr_page);  */
#endif

	assert((vaddr_page & (DYNTRANS_PAGESIZE-1)) == 0);
	assert((paddr_page & (DYNTRANS_PAGESIZE-1)) == 0);

	if (writeflag & MEMORY_USER_ACCESS) {
		writeflag &= ~MEMORY_USER_ACCESS;
		useraccess = 1;
	}

#ifdef DYNTRANS_M88K
	/*  TODO  */
	if (useraccess)
		return;
#endif

	/*  Scan the current TLB entries:  */

#ifdef MODE32
	/*
	 *  NOTE 1: vaddr_to_tlbindex is one more than the index, so that
	 *          0 becomes -1, which means a miss.
	 *
	 *  NOTE 2: When a miss occurs, instead of scanning the entire tlb
	 *          for the entry with the lowest time stamp, just choosing
	 *          one at random will work as well.
	 */
	found = (int)cpu->cd.DYNTRANS_ARCH.vaddr_to_tlbindex[
	    DYNTRANS_ADDR_TO_PAGENR(vaddr_page)] - 1;
#else
	x1 = (vaddr_page >> (64-DYNTRANS_L1N)) & mask1;
	x2 = (vaddr_page >> (64-DYNTRANS_L1N-DYNTRANS_L2N)) & mask2;
	x3 = (vaddr_page >> (64-DYNTRANS_L1N-DYNTRANS_L2N-DYNTRANS_L3N))
	    & mask3;

	l2 = cpu->cd.DYNTRANS_ARCH.l1_64[x1];
	if (l2 == cpu->cd.DYNTRANS_ARCH.l2_64_dummy)
		found = -1;
	else {
		l3 = l2->l3[x2];
		if (l3 == cpu->cd.DYNTRANS_ARCH.l3_64_dummy)
			found = -1;
		else
			found = (int)l3->vaddr_to_tlbindex[x3] - 1;
	}
#endif

	if (found < 0) {
		/*  Create the new TLB entry, overwriting a "random" entry:  */
		static unsigned int x = 0;
		r = (x++) % DYNTRANS_MAX_VPH_TLB_ENTRIES;

		if (cpu->cd.DYNTRANS_ARCH.vph_tlb_entry[r].valid) {
			/*  This one has to be invalidated first:  */
			DYNTRANS_INVALIDATE_TLB_ENTRY(cpu,
			    cpu->cd.DYNTRANS_ARCH.vph_tlb_entry[r].vaddr_page,
			    0);
		}

		cpu->cd.DYNTRANS_ARCH.vph_tlb_entry[r].valid = 1;
		cpu->cd.DYNTRANS_ARCH.vph_tlb_entry[r].host_page = host_page;
		cpu->cd.DYNTRANS_ARCH.vph_tlb_entry[r].paddr_page = paddr_page;
		cpu->cd.DYNTRANS_ARCH.vph_tlb_entry[r].vaddr_page = vaddr_page;
		cpu->cd.DYNTRANS_ARCH.vph_tlb_entry[r].writeflag =
		    writeflag & MEM_WRITE;

		/*  Add the new translation to the table:  */
#ifdef MODE32
		index = DYNTRANS_ADDR_TO_PAGENR(vaddr_page);
		cpu->cd.DYNTRANS_ARCH.host_load[index] = host_page;
		cpu->cd.DYNTRANS_ARCH.host_store[index] =
		    writeflag? host_page : NULL;
		cpu->cd.DYNTRANS_ARCH.phys_addr[index] = paddr_page;
		cpu->cd.DYNTRANS_ARCH.phys_page[index] = NULL;
		cpu->cd.DYNTRANS_ARCH.vaddr_to_tlbindex[index] = r + 1;
#ifdef DYNTRANS_ARM
		if (useraccess)
			cpu->cd.DYNTRANS_ARCH.is_userpage[index >> 5]
			    |= 1 << (index & 31);
#endif
#else	/* !MODE32  */
		l2 = cpu->cd.DYNTRANS_ARCH.l1_64[x1];
		if (l2 == cpu->cd.DYNTRANS_ARCH.l2_64_dummy) {
			if (cpu->cd.DYNTRANS_ARCH.next_free_l2 != NULL) {
				l2 = cpu->cd.DYNTRANS_ARCH.l1_64[x1] =
				    cpu->cd.DYNTRANS_ARCH.next_free_l2;
				cpu->cd.DYNTRANS_ARCH.next_free_l2 = l2->next;
			} else {
				int i;
				CHECK_ALLOCATION(l2 =
				    cpu->cd.DYNTRANS_ARCH.l1_64[x1] =
				    (struct DYNTRANS_L2_64_TABLE *) malloc(
				    sizeof(struct DYNTRANS_L2_64_TABLE)));
				l2->refcount = 0;
				for (i=0; i<(1 << DYNTRANS_L2N); i++)
					l2->l3[i] = cpu->cd.DYNTRANS_ARCH.
					    l3_64_dummy;
			}
			if (l2->refcount != 0) {
				fatal("Huh? l2 Refcount problem.\n");
				exit(1);
			}
		}
		if (l2 == cpu->cd.DYNTRANS_ARCH.l2_64_dummy) {
			fatal("INTERNAL ERROR L2 reuse\n");
			exit(1);
		}
		l3 = l2->l3[x2];
		if (l3 == cpu->cd.DYNTRANS_ARCH.l3_64_dummy) {
			if (cpu->cd.DYNTRANS_ARCH.next_free_l3 != NULL) {
				l3 = l2->l3[x2] =
				    cpu->cd.DYNTRANS_ARCH.next_free_l3;
				cpu->cd.DYNTRANS_ARCH.next_free_l3 = l3->next;
			} else {
				l3 = l2->l3[x2] = (struct DYNTRANS_L3_64_TABLE *)
				    zeroed_alloc(sizeof(
				    struct DYNTRANS_L3_64_TABLE));
			}
			if (l3->refcount != 0) {
				fatal("Huh? l3 Refcount problem.\n");
				exit(1);
			}
			l2->refcount ++;
		}
		if (l3 == cpu->cd.DYNTRANS_ARCH.l3_64_dummy) {
			fatal("INTERNAL ERROR L3 reuse\n");
			exit(1);
		}

		l3->host_load[x3] = host_page;
		l3->host_store[x3] = writeflag? host_page : NULL;
		l3->phys_addr[x3] = paddr_page;
		l3->phys_page[x3] = NULL;
		l3->vaddr_to_tlbindex[x3] = r + 1;
		l3->refcount ++;

#ifdef BUGHUNT
/*  Count how many pages are actually in use:  */
{
	int n=0, i;
	for (i=0; i<=mask3; i++)
		if (l3->vaddr_to_tlbindex[i])
			n++;
	if (n != l3->refcount) {
		printf("X: %i in use, but refcount = %i!\n", n, l3->refcount);
		exit(1);
	}

	n = 0;
	for (i=0; i<=mask3; i++)
		if (l3->host_load[i] != NULL)
			n++;
	if (n != l3->refcount) {
		printf("XHL: %i in use, but refcount = %i!\n", n, l3->refcount);
		exit(1);
	}
}
#endif

#endif	/* !MODE32  */
	} else {
		/*
		 *  The translation was already in the TLB.
		 *	Writeflag = 0:  Do nothing.
		 *	Writeflag = 1:  Make sure the page is writable.
		 *	Writeflag = MEM_DOWNGRADE: Downgrade to readonly.
		 */
		r = found;
		if (writeflag & MEM_WRITE)
			cpu->cd.DYNTRANS_ARCH.vph_tlb_entry[r].writeflag = 1;
		if (writeflag & MEM_DOWNGRADE)
			cpu->cd.DYNTRANS_ARCH.vph_tlb_entry[r].writeflag = 0;
#ifdef MODE32
		index = DYNTRANS_ADDR_TO_PAGENR(vaddr_page);
		cpu->cd.DYNTRANS_ARCH.phys_page[index] = NULL;
#ifdef DYNTRANS_ARM
		cpu->cd.DYNTRANS_ARCH.is_userpage[index>>5] &= ~(1<<(index&31));
		if (useraccess)
			cpu->cd.DYNTRANS_ARCH.is_userpage[index >> 5]
			    |= 1 << (index & 31);
#endif
		if (cpu->cd.DYNTRANS_ARCH.phys_addr[index] == paddr_page) {
			if (writeflag & MEM_WRITE)
				cpu->cd.DYNTRANS_ARCH.host_store[index] =
				    host_page;
			if (writeflag & MEM_DOWNGRADE)
				cpu->cd.DYNTRANS_ARCH.host_store[index] = NULL;
		} else {
			/*  Change the entire physical/host mapping:  */
			cpu->cd.DYNTRANS_ARCH.host_load[index] = host_page;
			cpu->cd.DYNTRANS_ARCH.host_store[index] =
			    writeflag? host_page : NULL;
			cpu->cd.DYNTRANS_ARCH.phys_addr[index] = paddr_page;
		}
#else	/*  !MODE32  */
		x1 = (vaddr_page >> (64-DYNTRANS_L1N)) & mask1;
		x2 = (vaddr_page >> (64-DYNTRANS_L1N-DYNTRANS_L2N)) & mask2;
		x3 = (vaddr_page >> (64-DYNTRANS_L1N-DYNTRANS_L2N-DYNTRANS_L3N))
		    & mask3;
		l2 = cpu->cd.DYNTRANS_ARCH.l1_64[x1];
		l3 = l2->l3[x2];
		if (l3->phys_addr[x3] == paddr_page) {
			if (writeflag & MEM_WRITE)
				l3->host_store[x3] = host_page;
			if (writeflag & MEM_DOWNGRADE)
				l3->host_store[x3] = NULL;
		} else {
			/*  Change the entire physical/host mapping:  */
			l3->host_load[x3] = host_page;
			l3->host_store[x3] = writeflag? host_page : NULL;
			l3->phys_addr[x3] = paddr_page;
		}

#ifdef BUGHUNT
/*  Count how many pages are actually in use:  */
{
	int n=0, i;
	for (i=0; i<=mask3; i++)
		if (l3->vaddr_to_tlbindex[i])
			n++;
	if (n != l3->refcount) {
		printf("Y: %i in use, but refcount = %i!\n", n, l3->refcount);
		exit(1);
	}

	n = 0;
	for (i=0; i<=mask3; i++)
		if (l3->host_load[i] != NULL)
			n++;
	if (n != l3->refcount) {
		printf("YHL: %i in use, but refcount = %i!\n", n, l3->refcount);
		printf("Entry r = %i\n", r);
		printf("Valid = %i\n",
cpu->cd.DYNTRANS_ARCH.vph_tlb_entry[r].valid);
		exit(1);
	}
}
#endif

#endif	/*  !MODE32  */
	}
}
#endif	/*  DYNTRANS_UPDATE_TRANSLATION_TABLE  */


/*****************************************************************************/


#ifdef DYNTRANS_TO_BE_TRANSLATED_HEAD
	/*
	 *  Check for breakpoints.
	 */
	if (!single_step_breakpoint && !cpu->translation_readahead) {
		MODE_uint_t curpc = cpu->pc;
		int i;
		for (i=0; i<cpu->machine->breakpoints.n; i++)
			if (curpc == (MODE_uint_t)
			    cpu->machine->breakpoints.addr[i]) {
				if (!cpu->machine->instruction_trace) {
					int old_quiet_mode = quiet_mode;
					quiet_mode = 0;
					DISASSEMBLE(cpu, ib, 1, 0);
					quiet_mode = old_quiet_mode;
				}
#ifdef MODE32
				fatal("BREAKPOINT: pc = 0x%"PRIx32"\n(The "
				    "instruction has not yet executed.)\n",
				    (uint32_t)cpu->pc);
#else
				fatal("BREAKPOINT: pc = 0x%"PRIx64"\n(The "
				    "instruction has not yet executed.)\n",
				    (uint64_t)cpu->pc);
#endif
#ifdef DYNTRANS_DELAYSLOT
				if (cpu->delay_slot != NOT_DELAYED)
					fatal("ERROR! Breakpoint in a delay"
					    " slot! Not yet supported.\n");
#endif
				single_step_breakpoint = 1;
				single_step = ENTER_SINGLE_STEPPING;
				goto stop_running_translated;
			}
	}
#endif	/*  DYNTRANS_TO_BE_TRANSLATED_HEAD  */


/*****************************************************************************/


#ifdef DYNTRANS_TO_BE_TRANSLATED_TAIL
	/*
	 *  If we end up here, then an instruction was translated. Let's mark
	 *  the page as containing a translation at this part of the page.
	 */

	/*  Make sure cur_physpage is in synch:  */
	cpu->cd.DYNTRANS_ARCH.cur_physpage = (struct DYNTRANS_TC_PHYSPAGE *)
	    cpu->cd.DYNTRANS_ARCH.cur_ic_page;

	{
		int x = addr & (DYNTRANS_PAGESIZE - 1);
		int addr_per_translation_range = DYNTRANS_PAGESIZE / (8 *
		    sizeof(cpu->cd.DYNTRANS_ARCH.cur_physpage->
		    translations_bitmap));
		x /= addr_per_translation_range;

		cpu->cd.DYNTRANS_ARCH.cur_physpage->
		    translations_bitmap |= (1 << x);
	}


	/*
	 *  Now it is time to check for combinations of instructions that can
	 *  be converted into a single function call.
	 *
	 *  Note: Single-stepping or instruction tracing doesn't work with
	 *  instruction combinations. For architectures with delay slots,
	 *  we also ignore combinations if the delay slot is across a page
	 *  boundary.
	 */
	if (!single_step && !cpu->machine->instruction_trace
#ifdef DYNTRANS_DELAYSLOT
	    && !in_crosspage_delayslot
#endif
	    && cpu->cd.DYNTRANS_ARCH.combination_check != NULL
	    && cpu->machine->allow_instruction_combinations) {
		cpu->cd.DYNTRANS_ARCH.combination_check(cpu, ic,
		    addr & (DYNTRANS_PAGESIZE - 1));
	}

	cpu->cd.DYNTRANS_ARCH.combination_check = NULL;

	/*  An additional check, to catch some bugs:  */
	if (ic->f == TO_BE_TRANSLATED) {
		fatal("INTERNAL ERROR: ic->f not set!\n");
		goto bad;
	}
	if (ic->f == NULL) {
		fatal("INTERNAL ERROR: ic->f == NULL!\n");
		goto bad;
	}


	/*
	 *  ... and finally execute the translated instruction:
	 */

	/*  (Except when doing read-ahead!)  */
	if (cpu->translation_readahead)
		return;

	/*
	 *  Special case when single-stepping: Execute the translated
	 *  instruction, but then replace it with a "to be translated"
	 *  directly afterwards.
	 */
	if ((single_step_breakpoint && cpu->delay_slot == NOT_DELAYED)
#ifdef DYNTRANS_DELAYSLOT
	    || in_crosspage_delayslot
#endif
	    ) {
		single_step_breakpoint = 0;
		ic->f(cpu, ic);
		ic->f = TO_BE_TRANSLATED;
		return;
	}


	/*  Translation read-ahead:  */
	if (!single_step && !cpu->machine->instruction_trace &&
	    cpu->machine->breakpoints.n == 0) {
		uint64_t baseaddr = cpu->pc;
		uint64_t pagenr = DYNTRANS_ADDR_TO_PAGENR(baseaddr);
		int i = 1;

		cpu->translation_readahead = MAX_DYNTRANS_READAHEAD;

		while (DYNTRANS_ADDR_TO_PAGENR(baseaddr +
		    (i << DYNTRANS_INSTR_ALIGNMENT_SHIFT)) == pagenr &&
		    cpu->translation_readahead > 0) {
			void (*old_f)(struct cpu *,
			    struct DYNTRANS_IC *) = ic[i].f;

			/*  Already translated? Then abort:  */
			if (old_f != TO_BE_TRANSLATED)
				break;

			/*  Translate the instruction:  */
			ic[i].f(cpu, ic+i);

			/*  Translation failed? Then abort.  */
			if (ic[i].f == old_f)
				break;

			cpu->translation_readahead --;
			++i;
		}

		cpu->translation_readahead = 0;
	}


	/*
	 *  Finally finally :-), execute the instruction.
	 *
	 *  Note: The instruction might have changed during read-ahead, if
	 *  instruction combinations are used.
	 */

	ic->f(cpu, ic);

	return;


bad:	/*
	 *  Nothing was translated. (Unimplemented or illegal instruction.)
	 */

	/*  Clear the translation, in case it was "half-way" done:  */
	ic->f = TO_BE_TRANSLATED;

	if (cpu->translation_readahead)
		return;

	quiet_mode = 0;
	fatal("to_be_translated(): TODO: unimplemented instruction");

	if (cpu->machine->instruction_trace) {
		if (cpu->is_32bit)
			fatal(" at 0x%"PRIx32"\n", (uint32_t)cpu->pc);
		else
			fatal(" at 0x%"PRIx64"\n", (uint64_t)cpu->pc);
	} else {
		fatal(":\n");
		DISASSEMBLE(cpu, ib, 1, 0);
	}

	cpu->running = 0;

	/*  Note: Single-stepping can jump here.  */
stop_running_translated:

	debugger_n_steps_left_before_interaction = 0;

	ic = cpu->cd.DYNTRANS_ARCH.next_ic = &nothing_call;
	cpu->cd.DYNTRANS_ARCH.next_ic ++;

#ifdef DYNTRANS_DELAYSLOT
	/*  Special hack: If the bad instruction was in a delay slot,
	    make sure that execution does not continue anyway:  */
	if (cpu->delay_slot)
		cpu->delay_slot |= EXCEPTION_IN_DELAY_SLOT;
#endif

	/*  Execute the "nothing" instruction:  */
	ic->f(cpu, ic);

#endif	/*  DYNTRANS_TO_BE_TRANSLATED_TAIL  */

