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
 *  Motorola M881x0 CPU emulation.
 *
 *  M88100: Disassembly of (almost?) everything, and execution of most
 *          instructions. Exception handling and virtual memory has also
 *          been implemented. Exceptions while in delay slots may not work
 *          fully yet, though.
 *
 *  M88110: Not yet. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "cpu.h"
#include "float_emul.h"
#include "interrupt.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"
#include "settings.h"
#include "symbol.h"

#include "thirdparty/m8820x_pte.h"
#include "thirdparty/m88k_dmt.h"
#include "thirdparty/mvmeprom.h"

#define DYNTRANS_32
#define DYNTRANS_DELAYSLOT
#include "tmp_m88k_head.cc"


void m88k_pc_to_pointers(struct cpu *);
void m88k_cpu_functioncall_trace(struct cpu *cpu, int n_args);

static const char *memop[4] = { ".d", "", ".h", ".b" };

void m88k_irq_interrupt_assert(struct interrupt *interrupt);
void m88k_irq_interrupt_deassert(struct interrupt *interrupt);


static const char *m88k_cr_names[] = M88K_CR_NAMES;
static const char *m88k_cr_197_names[] = M88K_CR_NAMES_197;

static const char *m88k_cr_name(struct cpu *cpu, int i)
{
	const char **cr_names = m88k_cr_names;

	/*  Hm. Is this really MVME197 specific? TODO  */
	if (cpu->machine->machine_subtype == MACHINE_MVME88K_197)
		cr_names = m88k_cr_197_names;

	return cr_names[i];
}

static char *m88k_fcr_name(struct cpu *cpu, int fi)
{
	/*  TODO  */
	static char fcr_name[10];
	snprintf(fcr_name, sizeof(fcr_name), "FCR%i", fi);
	return fcr_name;
}



/*
 *  m88k_cpu_new():
 *
 *  Create a new M88K cpu object by filling the CPU struct.
 *  Return 1 on success, 0 if cpu_type_name isn't a valid M88K processor.
 */
int m88k_cpu_new(struct cpu *cpu, struct memory *mem,
	struct machine *machine, int cpu_id, char *cpu_type_name)
{
	int i, found;
	struct m88k_cpu_type_def cpu_type_defs[] = M88K_CPU_TYPE_DEFS;

	/*  Scan the list for this cpu type:  */
	i = 0; found = -1;
	while (i >= 0 && cpu_type_defs[i].name != NULL) {
		if (strcasecmp(cpu_type_defs[i].name, cpu_type_name) == 0) {
			found = i;
			break;
		}
		i++;
	}
	if (found == -1)
		return 0;

	cpu->run_instr = m88k_run_instr;
	cpu->memory_rw = m88k_memory_rw;
	cpu->update_translation_table = m88k_update_translation_table;
	cpu->invalidate_translation_caches =
	    m88k_invalidate_translation_caches;
	cpu->invalidate_code_translation = m88k_invalidate_code_translation;
	cpu->translate_v2p = m88k_translate_v2p;

	cpu->cd.m88k.cpu_type = cpu_type_defs[found];
	cpu->name            = strdup(cpu->cd.m88k.cpu_type.name);
	cpu->is_32bit        = 1;
	cpu->byte_order      = EMUL_BIG_ENDIAN;

	cpu->instruction_has_delayslot = m88k_cpu_instruction_has_delayslot;

	/*  Only show name and caches etc for CPU nr 0:  */
	if (cpu_id == 0) {
		debug("%s", cpu->name);
	}


	/*
	 *  Add register names as settings:
	 */

	CPU_SETTINGS_ADD_REGISTER64("pc", cpu->pc);

	for (i=0; i<N_M88K_REGS; i++) {
		char name[10];
		snprintf(name, sizeof(name), "r%i", i);
		CPU_SETTINGS_ADD_REGISTER32(name, cpu->cd.m88k.r[i]);
	}

	for (i=0; i<N_M88K_CONTROL_REGS; i++) {
		char name[10];
		snprintf(name, sizeof(name), "%s", m88k_cr_name(cpu, i));
		CPU_SETTINGS_ADD_REGISTER32(name, cpu->cd.m88k.cr[i]);
	}

	for (i=0; i<N_M88K_FPU_CONTROL_REGS; i++) {
		char name[10];
		snprintf(name, sizeof(name), "%s", m88k_fcr_name(cpu, i));
		CPU_SETTINGS_ADD_REGISTER32(name, cpu->cd.m88k.fcr[i]);
	}


	/*  Register the CPU interrupt pin:  */
	{
		struct interrupt templ;
		char name[50];
		snprintf(name, sizeof(name), "%s", cpu->path);

		memset(&templ, 0, sizeof(templ));
		templ.line = 0;
		templ.name = name;
		templ.extra = cpu;
		templ.interrupt_assert = m88k_irq_interrupt_assert;
		templ.interrupt_deassert = m88k_irq_interrupt_deassert;
		interrupt_handler_register(&templ);
	}

	/*  Set the Processor ID:  */
	cpu->cd.m88k.cr[M88K_CR_PID] = cpu->cd.m88k.cpu_type.pid | M88K_PID_MC;

	/*  Start in supervisor mode, with interrupts disabled.  */
	cpu->cd.m88k.cr[M88K_CR_PSR] = M88K_PSR_MODE | M88K_PSR_IND;
	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		cpu->cd.m88k.cr[M88K_CR_PSR] |= M88K_PSR_BO;

	/*  Initial stack pointer:  */
	cpu->cd.m88k.r[31] = 1048576 * cpu->machine->physical_ram_in_mb - 1024;

	return 1;
}


/*
 *  m88k_cpu_dumpinfo():
 */
void m88k_cpu_dumpinfo(struct cpu *cpu)
{
	/*  struct m88k_cpu_type_def *ct = &cpu->cd.m88k.cpu_type;  */

	debug(", %s-endian",
	    cpu->byte_order == EMUL_BIG_ENDIAN? "Big" : "Little");

	debug("\n");
}


/*
 *  m88k_cpu_list_available_types():
 *
 *  Print a list of available M88K CPU types.
 */
void m88k_cpu_list_available_types(void)
{
	int i, j;
	struct m88k_cpu_type_def tdefs[] = M88K_CPU_TYPE_DEFS;

	i = 0;
	while (tdefs[i].name != NULL) {
		debug("%s", tdefs[i].name);
		for (j=13 - strlen(tdefs[i].name); j>0; j--)
			debug(" ");
		i++;
		if ((i % 5) == 0 || tdefs[i].name == NULL)
			debug("\n");
	}
}


/*
 *  m88k_cpu_instruction_has_delayslot():
 *
 *  Returns 1 if an opcode has a delay slot after it, 0 otherwise.
 */
int m88k_cpu_instruction_has_delayslot(struct cpu *cpu, unsigned char *ib)
{
	uint32_t iword = *((uint32_t *)&ib[0]);

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		iword = LE32_TO_HOST(iword);
	else
		iword = BE32_TO_HOST(iword);

	switch (iword >> 26) {
	case 0x31:	/*  br.n  */
	case 0x33:	/*  bsr.n  */
	case 0x35:	/*  bb0.n  */
	case 0x37:	/*  bb1.n  */
	case 0x3b:	/*  bcnd.n  */
		return 1;
	case 0x3d:
		switch ((iword >> 8) & 0xff) {
		case 0xc4:	/*  jmp.n  */
		case 0xcc:	/*  jsr.n  */
			return 1;
		}
	}

	return 0;
}


/*
 *  m88k_cpu_register_dump():
 *
 *  Dump cpu registers in a relatively readable format.
 *  
 *  gprs: set to non-zero to dump GPRs and some special-purpose registers.
 *  coprocs: set bit 0..3 to dump registers in coproc 0..3.
 */
void m88k_cpu_register_dump(struct cpu *cpu, int gprs, int coprocs)
{
	char *symbol;
	uint64_t offset;
	int i, x = cpu->cpu_id;

	if (gprs) {
		symbol = get_symbol_name(&cpu->machine->symbol_context,
		    cpu->pc, &offset);
		debug("cpu%i:  pc  = 0x%08"PRIx32, x, (uint32_t)cpu->pc);
		debug("  <%s>\n", symbol != NULL? symbol : " no symbol ");

		for (i=0; i<N_M88K_REGS; i++) {
			if ((i % 4) == 0)
				debug("cpu%i:", x);
			if (i == 0)
				debug("                  ");
			else
				debug("  r%-2i = 0x%08"PRIx32,
				    i, cpu->cd.m88k.r[i]);
			if ((i % 4) == 3)
				debug("\n");
		}
	}

	if (coprocs & 1) {
		int n_control_regs = 32;

		/*  Hm. Is this really MVME197 specific? TODO  */
		if (cpu->machine->machine_subtype == MACHINE_MVME88K_197)
			n_control_regs = 64;

		for (i=0; i<n_control_regs; i++) {
			if ((i % 4) == 0)
				debug("cpu%i:", x);
			debug("  %4s=0x%08"PRIx32,
			    m88k_cr_name(cpu, i), cpu->cd.m88k.cr[i]);
			if ((i % 4) == 3)
				debug("\n");
		}
	}

	if (coprocs & 2) {
		int n_fpu_control_regs = 64;

		for (i=0; i<n_fpu_control_regs; i++) {
			if ((i % 4) == 0)
				debug("cpu%i:", x);
			debug("  %5s=0x%08"PRIx32,
			    m88k_fcr_name(cpu, i), cpu->cd.m88k.fcr[i]);
			if ((i % 4) == 3)
				debug("\n");
		}
	}
}


/*
 *  m88k_cpu_tlbdump():
 *
 *  Called from the debugger to dump the TLB in a readable format.
 *  x is the cpu number to dump, or -1 to dump all CPUs.
 *
 *  If rawflag is nonzero, then the TLB contents isn't formated nicely,
 *  just dumped.
 */
void m88k_cpu_tlbdump(struct machine *m, int x, int rawflag)
{
	int cpu_nr, cmmu_nr, i;

	for (cpu_nr = 0; cpu_nr < m->ncpus; cpu_nr++) {
		struct cpu *cpu = m->cpus[cpu_nr];

		if (x != -1 && cpu_nr != x)
			continue;

		for (cmmu_nr = 0; cmmu_nr < MAX_M8820X_CMMUS; cmmu_nr++) {
			struct m8820x_cmmu *cmmu = cpu->cd.m88k.cmmu[cmmu_nr];
			if (cmmu == NULL)
				continue;

			printf("cpu%i: CMMU %i (%s)\n", cpu_nr, cmmu_nr,
			    cmmu_nr & 1? "data" : "instruction");

			/*  BATC:  */
			for (i = 0; i < N_M88200_BATC_REGS; i++) {
				uint32_t b = cmmu->batc[i];
				printf("cpu%i: BATC[%2i]: ", cpu_nr, i);
				printf("v=0x%08"PRIx32, b & 0xfff80000);
				printf(", p=0x%08"PRIx32,
				    (b << 13) & 0xfff80000);
				printf(", %s %s %s %s %s %s\n",
				    b & BATC_SO? "SP " : "!sp",
				    b & BATC_WT? "WT " : "!wt",
				    b & BATC_GLOBAL? "G  " : "!g ",
				    b & BATC_INH? "CI " : "!ci",
				    b & BATC_PROT? "WP " : "!wp",
				    b & BATC_SO? "V " : "!v");
			}

			/*  PATC:  */
			for (i = 0; i < N_M88200_PATC_ENTRIES; i++) {
				uint32_t v = cmmu->patc_v_and_control[i];
				uint32_t p = cmmu->patc_p_and_supervisorbit[i];

				printf("cpu%i: patc[%2i]: ", cpu_nr, i);
				if (p & M8820X_PATC_SUPERVISOR_BIT)
					printf("superv");
				else
					printf("user  ");
				printf(" v=0x%08"PRIx32, v & 0xfffff000);
				printf(", p=0x%08"PRIx32, p & 0xfffff000);

				printf("  %s %s %s %s %s %s %s",
				    v & PG_U1?   "U1 " : "!u1",
				    v & PG_U0?   "U0 " : "!u0",
				    v & PG_SO?   "SP " : "!sp",
				    v & PG_M?    "M "  : "!m",
				    v & PG_U?    "U "  : "!u",
				    v & PG_PROT? "WP " : "!wp",
				    v & PG_V?    "V "  : "!v");

				if (i == cmmu->patc_update_index)
					printf(" <--");
				printf("\n");
			}
		}
	}
}


/*
 *  m88k_irq_interrupt_assert():
 *  m88k_irq_interrupt_deassert():
 */
void m88k_irq_interrupt_assert(struct interrupt *interrupt)
{
	struct cpu *cpu = (struct cpu *) interrupt->extra;
	cpu->cd.m88k.irq_asserted = 1;
}
void m88k_irq_interrupt_deassert(struct interrupt *interrupt)
{
	struct cpu *cpu = (struct cpu *) interrupt->extra;
	cpu->cd.m88k.irq_asserted = 0;
}


/*
 *  m88k_ldcr():
 *
 *  Read from a control register. Store the resulting value in a register
 *  (pointed to by r32ptr).
 */
void m88k_ldcr(struct cpu *cpu, uint32_t *r32ptr, int cr)
{
	uint32_t retval = cpu->cd.m88k.cr[cr];

	switch (cr) {

	case M88K_CR_PID:
	case M88K_CR_PSR:
	case M88K_CR_EPSR:
	case M88K_CR_SSBR:
	case M88K_CR_SXIP:
	case M88K_CR_SNIP:
	case M88K_CR_SFIP:
	case M88K_CR_VBR:
	case M88K_CR_DMD0:
	case M88K_CR_DMD1:
	case M88K_CR_DMD2:
	case M88K_CR_SR0:
	case M88K_CR_SR1:
	case M88K_CR_SR2:
	case M88K_CR_SR3:
		break;

	case M88K_CR_DMT0:
	case M88K_CR_DMT1:
	case M88K_CR_DMT2:
		/*
		 *  Catch some possible internal errors in the emulator:
		 *
		 *  For valid memory Load transactions, the Destination Register
		 *  should not be zero.
		 *
		 *  The Byte Order bit should be the same as the CPU's.
		 */
		if (retval & DMT_VALID && !(retval & DMT_WRITE)) {
			if (DMT_DREGBITS(retval) == M88K_ZERO_REG) {
				fatal("DMT DREG = zero? Internal error.\n");
				exit(1);
			}
		}
		if (!!(cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_BO)
		    != !!(retval & DMT_BO) && retval & DMT_VALID) {
			fatal("DMT byte order not same as CPUs?\n");
			exit(1);
		}

		break;

	case M88K_CR_DMA0:
	case M88K_CR_DMA1:
	case M88K_CR_DMA2:
		/*
		 *  Catch some possible internal errors in the emulator:
		 *  The lowest 2 bits of the transaction address registers
		 *  should always be zero.
		 */
		if (retval & 3) {
			fatal("DMAx not word-aligned? Internal error.\n");
			exit(1);
		}

		break;

	default:fatal("m88k_ldcr: UNIMPLEMENTED cr = 0x%02x (%s)\n",
		    cr, m88k_cr_name(cpu, cr));
		exit(1);
	}

	*r32ptr = retval;
}


/*
 *  m88k_stcr():
 *
 *  Write to a control register.
 *  (Used by both the stcr and rte instructions.)
 */
void m88k_stcr(struct cpu *cpu, uint32_t value, int cr, int rte)
{
	uint32_t old = cpu->cd.m88k.cr[cr];

	switch (cr) {

	case M88K_CR_PSR:	/*  Processor Status Regoster  */
		if ((cpu->byte_order == EMUL_LITTLE_ENDIAN
		    && !(value & M88K_PSR_BO)) ||
		    (cpu->byte_order == EMUL_BIG_ENDIAN
		    && (value & M88K_PSR_BO))) {
			fatal("TODO: attempt to change endianness by flipping"
			    " the endianness bit in the PSR. How should this"
			    " be handled? Aborting.\n");
			exit(1);
		}

		if (!rte && old & M88K_PSR_MODE && !(value & M88K_PSR_MODE))
			fatal("[ m88k_stcr: WARNING! the PSR_MODE bit is being"
			    " cleared; this should be done using the RTE "
			    "instruction only, according to the M88100 "
			    "manual! Continuing anyway. ]\n");

		if (value & M88K_PSR_MXM) {
			fatal("m88k_stcr: TODO: MXM support\n");
			exit(1);
		}

		if ((old & M88K_PSR_MODE) != (value & M88K_PSR_MODE))
			cpu->invalidate_translation_caches(
			    cpu, 0, INVALIDATE_ALL);

		cpu->cd.m88k.cr[cr] = value;
		break;

	case M88K_CR_EPSR:
		cpu->cd.m88k.cr[cr] = value;
		break;

	case M88K_CR_SXIP:
	case M88K_CR_SNIP:
	case M88K_CR_SFIP:
		cpu->cd.m88k.cr[cr] = value;
		break;

	case M88K_CR_SSBR:	/*  Shadow ScoreBoard Register  */
		if (value & 1)
			fatal("[ m88k_stcr: WARNING! bit 0 non-zero when"
			    " writing to SSBR (?) ]\n");
		cpu->cd.m88k.cr[cr] = value;
		break;

	case M88K_CR_VBR:
		if (value & 0x00000fff)
			fatal("[ m88k_stcr: WARNING! bits 0..11 non-zero when"
			    " writing to VBR (?) ]\n");
		cpu->cd.m88k.cr[cr] = value;
		break;

	case M88K_CR_DMT0:
	case M88K_CR_DMT1:
	case M88K_CR_DMT2:
		cpu->cd.m88k.cr[cr] = value;
		break;

	case M88K_CR_SR0:	/*  Supervisor Storage Registers 0..3  */
	case M88K_CR_SR1:
	case M88K_CR_SR2:
	case M88K_CR_SR3:
		cpu->cd.m88k.cr[cr] = value;
		break;

	default:fatal("m88k_stcr: UNIMPLEMENTED cr = 0x%02x (%s)\n",
		    cr, m88k_cr_name(cpu, cr));
		exit(1);
	}
}


/*
 *  m88k_fstcr():
 *
 *  Write to a floating-point control register.
 */
void m88k_fstcr(struct cpu *cpu, uint32_t value, int fcr)
{
#if 0
	/*  TODO (?)  */
	uint32_t old = cpu->cd.m88k.cr[fcr];

	switch (fcr) {
	default:fatal("m88k_fstcr: UNIMPLEMENTED fcr = 0x%02x (%s)\n",
		    fcr, m88k_fcr_name(cpu, fcr));
		exit(1);
	}
#else
	cpu->cd.m88k.cr[fcr] = value;
#endif
}


/*
 *  m88k_memory_transaction_debug_dump():
 *
 *  Debug dump of the memory transaction registers of a cpu.
 */
static void m88k_memory_transaction_debug_dump(struct cpu *cpu, int n)
{
	uint32_t dmt = cpu->cd.m88k.dmt[n];

	debug("[ DMT%i: ", n);
	if (dmt & DMT_VALID) {
		if (dmt & DMT_BO)
			debug("Little-Endian, ");
		else
			debug("Big-Endian, ");
		if (dmt & DMT_DAS)
			debug("Supervisor, ");
		else
			debug("User, ");
		if (dmt & DMT_DOUB1)
			debug("DOUB1, ");
		if (dmt & DMT_LOCKBAR)
			debug("LOCKBAR, ");
		if (dmt & DMT_WRITE)
			debug("store, ");
		else {
			debug("load.%c(r%i), ",
			    dmt & DMT_SIGNED? 's' : 'u',
			    DMT_DREGBITS(dmt));
		}
		debug("bytebits=0x%x ]\n", DMT_ENBITS(dmt));

		debug("[ DMD%i: 0x%08"PRIx32"; ", n, cpu->cd.m88k.dmd[n]);
		debug("DMA%i: 0x%08"PRIx32" ]\n", n, cpu->cd.m88k.dma[n]);
	} else
		debug("not valid ]\n");
}


/*
 *  m88k_exception():
 *
 *  Cause an exception.
 */
void m88k_exception(struct cpu *cpu, int vector, int is_trap)
{
	int update_shadow_regs = 1;

	debug("[ EXCEPTION 0x%03x: ", vector);
	switch (vector) {
	case M88K_EXCEPTION_RESET:
		debug("RESET"); break;
	case M88K_EXCEPTION_INTERRUPT:
		debug("INTERRUPT"); break;
	case M88K_EXCEPTION_INSTRUCTION_ACCESS:
		debug("INSTRUCTION_ACCESS"); break;
	case M88K_EXCEPTION_DATA_ACCESS:
		debug("DATA_ACCESS"); break;
	case M88K_EXCEPTION_MISALIGNED_ACCESS:
		debug("MISALIGNED_ACCESS"); break;
	case M88K_EXCEPTION_UNIMPLEMENTED_OPCODE:
		debug("UNIMPLEMENTED_OPCODE"); break;
	case M88K_EXCEPTION_PRIVILEGE_VIOLATION:
		debug("PRIVILEGE_VIOLATION"); break;
	case M88K_EXCEPTION_BOUNDS_CHECK_VIOLATION:
		debug("BOUNDS_CHECK_VIOLATION"); break;
	case M88K_EXCEPTION_ILLEGAL_INTEGER_DIVIDE:
		debug("ILLEGAL_INTEGER_DIVIDE"); break;
	case M88K_EXCEPTION_INTEGER_OVERFLOW:
		debug("INTEGER_OVERFLOW"); break;
	case M88K_EXCEPTION_ERROR:
		debug("ERROR"); break;
	case M88K_EXCEPTION_SFU1_PRECISE:
		debug("SFU1_PRECISE"); break;
	case M88K_EXCEPTION_SFU1_IMPRECISE:
		debug("SFU1_IMPRECISE"); break;
	case 0x80:
#if 0
		fatal("[ syscall %i(", cpu->cd.m88k.r[13]);
		m88k_cpu_functioncall_trace(cpu, 8);
		fatal(") ]\n");
#endif
		debug("syscall, r13=%i", cpu->cd.m88k.r[13]); break;
	case MVMEPROM_VECTOR:
		debug("MVMEPROM_VECTOR"); break;
	default:debug("unknown"); break;
	}
	debug(" ]\n");

	/*  Stuff common for all exceptions:  */
	if (cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_SFRZ) {
		/*
		 *  Non-trap exceptions when the shadow freeze bit is already
		 *  set result in an Error exception:
		 */
		if (!is_trap) {
			vector = M88K_EXCEPTION_ERROR;
			fatal("[ SFRZ already set in PSR => ERROR ]\n");
		}

		update_shadow_regs = 0;
	} else {
		/*  Freeze shadow registers, and save the PSR:  */
		cpu->cd.m88k.cr[M88K_CR_EPSR] = cpu->cd.m88k.cr[M88K_CR_PSR];
	}

	m88k_stcr(cpu, cpu->cd.m88k.cr[M88K_CR_PSR]
	    | M88K_PSR_SFRZ	/*  Freeze shadow registers,          */
	    | M88K_PSR_IND	/*  disable interrupts,               */
	    | M88K_PSR_SFD1	/*  disable the floating point unit,  */
	    | M88K_PSR_MODE,	/*  and switch to supervisor mode.    */
	    M88K_CR_PSR, 0);

	if (update_shadow_regs) {
		cpu->cd.m88k.cr[M88K_CR_SSBR] = 0;

		cpu->cd.m88k.cr[M88K_CR_SXIP] = cpu->pc | M88K_XIP_V;

		/*  SNIP is the address to return to, when executing rte:  */
		if (cpu->delay_slot) {
			if (vector == M88K_EXCEPTION_DATA_ACCESS) {
				cpu->cd.m88k.cr[M88K_CR_SNIP] = cpu->cd.m88k.delay_target | M88K_NIP_V;
				cpu->cd.m88k.cr[M88K_CR_SFIP] = cpu->cd.m88k.cr[M88K_CR_SNIP]+4;
			} else if (vector == M88K_EXCEPTION_INSTRUCTION_ACCESS) {
				/*  If we are in a delay slot, then pc is
				    something like 0xc6000 here (not 0xc5ffc).  */
				cpu->cd.m88k.cr[M88K_CR_SNIP] = cpu->cd.m88k.delay_target | M88K_NIP_V;
				cpu->cd.m88k.cr[M88K_CR_SFIP] = 0;
			} else {
				/*  Perhaps something like this could work:  */
				cpu->cd.m88k.cr[M88K_CR_SNIP] = (cpu->pc + 4) | M88K_NIP_V;
				cpu->cd.m88k.cr[M88K_CR_SFIP] = cpu->cd.m88k.delay_target | M88K_NIP_V;
			}
		} else {
			cpu->cd.m88k.cr[M88K_CR_SNIP] = (cpu->pc + 4) | M88K_NIP_V;
			cpu->cd.m88k.cr[M88K_CR_SFIP] = cpu->cd.m88k.cr[M88K_CR_SNIP]+4;
		}

		if (vector == M88K_EXCEPTION_INSTRUCTION_ACCESS)
			cpu->cd.m88k.cr[M88K_CR_SXIP] |= M88K_XIP_E;
	}

	cpu->pc = cpu->cd.m88k.cr[M88K_CR_VBR] + 8 * vector;

	if (cpu->delay_slot)
		cpu->delay_slot = EXCEPTION_IN_DELAY_SLOT;
	else
		cpu->delay_slot = NOT_DELAYED;

	/*  Default to no memory transactions:  */
	cpu->cd.m88k.cr[M88K_CR_DMT0] = 0;
	cpu->cd.m88k.cr[M88K_CR_DMD0] = 0;
	cpu->cd.m88k.cr[M88K_CR_DMA0] = 0;
	cpu->cd.m88k.cr[M88K_CR_DMT1] = 0;
	cpu->cd.m88k.cr[M88K_CR_DMD1] = 0;
	cpu->cd.m88k.cr[M88K_CR_DMA1] = 0;
	cpu->cd.m88k.cr[M88K_CR_DMT2] = 0;
	cpu->cd.m88k.cr[M88K_CR_DMD2] = 0;
	cpu->cd.m88k.cr[M88K_CR_DMA2] = 0;

	/*  Vector-specific handling:  */
	if (vector < M88K_EXCEPTION_USER_TRAPS_START) {
		switch (vector) {

		case M88K_EXCEPTION_RESET:
			fatal("[ m88k_exception: reset ]\n");
			exit(1);

		case M88K_EXCEPTION_INTERRUPT:
			/*  When returning with rte, we want to re-  */
			/*  execute the interrupted instruction:  */
			cpu->cd.m88k.cr[M88K_CR_SNIP] -= 4;
			cpu->cd.m88k.cr[M88K_CR_SFIP] -= 4;
			break;

		case M88K_EXCEPTION_INSTRUCTION_ACCESS:
			/*  When returning with rte, we want to re-  */
			/*  execute the instruction in SXIP, not SNIP/SFIP,  */
			/*  (unless the exception was in a delay-slot):  */
			if (!(cpu->delay_slot & EXCEPTION_IN_DELAY_SLOT)) {
				cpu->cd.m88k.cr[M88K_CR_SNIP] = 0;
				cpu->cd.m88k.cr[M88K_CR_SFIP] = 0;
			}
			break;

		case M88K_EXCEPTION_DATA_ACCESS:
			/*  Update the memory transaction registers:  */
			cpu->cd.m88k.cr[M88K_CR_DMT0] = cpu->cd.m88k.dmt[0];
			cpu->cd.m88k.cr[M88K_CR_DMD0] = cpu->cd.m88k.dmd[0];
			cpu->cd.m88k.cr[M88K_CR_DMA0] = cpu->cd.m88k.dma[0];
			cpu->cd.m88k.cr[M88K_CR_DMT1] = cpu->cd.m88k.dmt[1];
			cpu->cd.m88k.cr[M88K_CR_DMD1] = cpu->cd.m88k.dmd[1];
			cpu->cd.m88k.cr[M88K_CR_DMA1] = cpu->cd.m88k.dma[1];
			cpu->cd.m88k.cr[M88K_CR_DMT2] = 0;
			cpu->cd.m88k.cr[M88K_CR_DMD2] = 0;
			cpu->cd.m88k.cr[M88K_CR_DMA2] = 0;
			m88k_memory_transaction_debug_dump(cpu, 0);
			m88k_memory_transaction_debug_dump(cpu, 1);
			break;

#if 0
		case M88K_EXCEPTION_ILLEGAL_INTEGER_DIVIDE:
			/*  TODO: Is it correct to continue on the instruction
			    _after_ the division by zero? Or should the PC
			    be backed up one step?  */
			break;
#endif

		default:fatal("m88k_exception(): 0x%x: TODO\n", vector);
			fflush(stdout);
			exit(1);
		}
	}

	m88k_pc_to_pointers(cpu);
}


/*
 *  m88k_cpu_disassemble_instr():
 *
 *  Convert an instruction word into human readable format, for instruction
 *  tracing.
 *              
 *  If running is 1, cpu->pc should be the address of the instruction.
 *
 *  If running is 0, things that depend on the runtime environment (eg.
 *  register contents) will not be shown, and dumpaddr will be used instead of
 *  cpu->pc for relative addresses.
 */                     
int m88k_cpu_disassemble_instr(struct cpu *cpu, unsigned char *ib,
        int running, uint64_t dumpaddr)
{
	int supervisor = cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_MODE;
	uint32_t iw;
	const char *symbol, *mnem = NULL;
	uint64_t offset;
	uint32_t op26, op10, op11, d, s1, s2, w5, cr6, imm16;
	int32_t d16, d26;

	if (running)
		dumpaddr = cpu->pc;

	symbol = get_symbol_name(&cpu->machine->symbol_context,
	    dumpaddr, &offset);
	if (symbol != NULL && offset == 0 && supervisor)
		debug("<%s>\n", symbol);

	if (cpu->machine->ncpus > 1 && running)
		debug("cpu%i:\t", cpu->cpu_id);

	debug("%c%08"PRIx32": ",
	    cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_MODE? 's' : 'u',
	    (uint32_t) dumpaddr);

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		iw = ib[0] + (ib[1]<<8) + (ib[2]<<16) + (ib[3]<<24);
	else
		iw = ib[3] + (ib[2]<<8) + (ib[1]<<16) + (ib[0]<<24);

	debug("%08"PRIx32, (uint32_t) iw);

	if (running && cpu->delay_slot)
		debug(" (d)");

	debug("\t");

	op26   = (iw >> 26) & 0x3f;
	op11   = (iw >> 11) & 0x1f;
	op10   = (iw >> 10) & 0x3f;
	d      = (iw >> 21) & 0x1f;
	s1     = (iw >> 16) & 0x1f;
	s2     =  iw        & 0x1f;
	imm16  = iw & 0xffff;
	w5     = (iw >>  5) & 0x1f;
	cr6    = (iw >>  5) & 0x3f;
	d16    = ((int16_t) (iw & 0xffff)) * 4;
	d26    = ((int32_t)((iw & 0x03ffffff) << 6)) >> 4;

	switch (op26) {

	case 0x00:	/*  xmem.bu  */
	case 0x01:	/*  xmem     */
	case 0x02:	/*  ld.hu    */
	case 0x03:	/*  ld.bu    */
	case 0x04:	/*  ld.d     */
	case 0x05:	/*  ld       */
	case 0x06:	/*  ld.h     */
	case 0x07:	/*  ld.b     */
	case 0x08:	/*  st.d     */
	case 0x09:	/*  st       */
	case 0x0a:	/*  st.h     */
	case 0x0b:	/*  st.b     */
		if (iw == 0x00000000) {
			debug("-\n");
			break;
		}
		switch (op26) {
		case 0x00:  debug("xmem.bu"); break;
		case 0x01:  debug("xmem"); break;
		case 0x02:  debug("ld.hu"); break;
		case 0x03:  debug("ld.bu"); break;
		default:    debug("%s%s", op26 >= 0x08? "st" : "ld",
				memop[op26 & 3]);
		}
		debug("\tr%i,r%i,0x%x", d, s1, imm16);
		if (running) {
			uint32_t tmpaddr = cpu->cd.m88k.r[s1] + imm16;
			symbol = get_symbol_name(&cpu->machine->symbol_context,
			    tmpaddr, &offset);
			if (symbol != NULL && supervisor)
				debug("\t; [<%s>]", symbol);
			else
				debug("\t; [0x%08"PRIx32"]", tmpaddr);
			if (op26 >= 0x08) {
				/*  Store:  */
				debug(" = ");
				switch (op26 & 3) {
				case 0:	debug("0x%016"PRIx64, (uint64_t)
					    ((((uint64_t) cpu->cd.m88k.r[d])
					    << 32) + ((uint64_t)
					    cpu->cd.m88k.r[d+1])) );
					break;
				case 1:	debug("0x%08"PRIx32,
					    (uint32_t) cpu->cd.m88k.r[d]);
					break;
				case 2:	debug("0x%04"PRIx16,
					    (uint16_t) cpu->cd.m88k.r[d]);
					break;
				case 3:	debug("0x%02"PRIx8,
					    (uint8_t) cpu->cd.m88k.r[d]);
					break;
				}
			} else {
				/*  Load:  */
				/*  TODO  */
			}
		} else {
			/*
			 *  Not running, but the following instruction
			 *  sequence is quite common:
			 *
			 *  or.u      rX,r0,A
			 *  st_or_ld  rY,rX,B
			 */

			/*  Try loading the instruction before the
			    current one.  */
			uint32_t iw2 = 0;
			cpu->memory_rw(cpu, cpu->mem,
			    dumpaddr - sizeof(uint32_t), (unsigned char *)&iw2,
			    sizeof(iw2), MEM_READ, CACHE_INSTRUCTION
			    | NO_EXCEPTIONS);
			if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
				 iw2 = LE32_TO_HOST(iw2);
			else
				 iw2 = BE32_TO_HOST(iw2);
			if ((iw2 >> 26) == 0x17 &&	/*  or.u  */
			    ((iw2 >> 21) & 0x1f) == s1) {
				uint32_t tmpaddr = (iw2 << 16) + imm16;
				symbol = get_symbol_name(
				    &cpu->machine->symbol_context,
				    tmpaddr, &offset);
				if (symbol != NULL && supervisor)
					debug("\t; [<%s>]", symbol);
				else
					debug("\t; [0x%08"PRIx32"]", tmpaddr);
			}
		}
		debug("\n");
		break;

	case 0x10:	/*  and     */
	case 0x11:	/*  and.u   */
	case 0x12:	/*  mask    */
	case 0x13:	/*  mask.u  */
	case 0x14:	/*  xor     */
	case 0x15:	/*  xor.u   */
	case 0x16:	/*  or      */
	case 0x17:	/*  or.u    */
		switch (op26) {
		case 0x10:
		case 0x11:	mnem = "and"; break;
		case 0x12:
		case 0x13:	mnem = "mask"; break;
		case 0x14:
		case 0x15:	mnem = "xor"; break;
		case 0x16:
		case 0x17:	mnem = "or"; break;
		}
		debug("%s%s\t", mnem, op26 & 1? ".u" : "");
		debug("r%i,r%i,0x%x", d, s1, imm16);

		if (op26 == 0x16 && d != M88K_ZERO_REG) {
			/*
			 *  The following instruction sequence is common:
			 *
			 *  or.u   rX,r0,A
			 *  or     rY,rX,B	; rY = AAAABBBB
			 */

			/*  Try loading the instruction before the
			    current one.  */
			uint32_t iw2 = 0;
			cpu->memory_rw(cpu, cpu->mem,
			    dumpaddr - sizeof(uint32_t), (unsigned char *)&iw2,
			    sizeof(iw2), MEM_READ, CACHE_INSTRUCTION
			    | NO_EXCEPTIONS);
			if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
				 iw2 = LE32_TO_HOST(iw2);
			else
				 iw2 = BE32_TO_HOST(iw2);
			if ((iw2 >> 26) == 0x17 &&	/*  or.u  */
			    ((iw2 >> 21) & 0x1f) == s1) {
				uint32_t tmpaddr = (iw2 << 16) + imm16;
				symbol = get_symbol_name(
				    &cpu->machine->symbol_context,
				    tmpaddr, &offset);
				debug("\t; ");
				if (symbol != NULL && supervisor)
					debug("<%s>", symbol);
				else
					debug("0x%08"PRIx32, tmpaddr);
			}
		}

		debug("\n");
		break;

	case 0x18:	/*  addu    */
	case 0x19:	/*  subu    */
	case 0x1a:	/*  divu    */
	case 0x1b:	/*  mulu    */
	case 0x1c:	/*  add    */
	case 0x1d:	/*  sub    */
	case 0x1e:	/*  div    */
	case 0x1f:	/*  cmp    */
		switch (op26) {
		case 0x18:	mnem = "addu"; break;
		case 0x19:	mnem = "subu"; break;
		case 0x1a:	mnem = "divu"; break;
		case 0x1b:	mnem = "mulu"; break;
		case 0x1c:	mnem = "add"; break;
		case 0x1d:	mnem = "sub"; break;
		case 0x1e:	mnem = "div"; break;
		case 0x1f:	mnem = "cmp"; break;
		}
		debug("%s\tr%i,r%i,%i\n", mnem, d, s1, imm16);
		break;

	case 0x20:
		if ((iw & 0x001ff81f) == 0x00004000) {
			debug("ldcr\tr%i,%s\n", d,
			    m88k_cr_name(cpu, cr6));
		} else if ((iw & 0x001ff81f) == 0x00004800) {
			debug("fldcr\tr%i,%s\n", d,
			    m88k_fcr_name(cpu, cr6));
		} else if ((iw & 0x03e0f800) == 0x00008000) {
			debug("stcr\tr%i,%s", s1,
			    m88k_cr_name(cpu, cr6));
			if (s1 != s2)
				debug("\t\t; NOTE: weird encoding: "
				    "low 5 bits = 0x%02x", s2);
			debug("\n");
		} else if ((iw & 0x03e0f800) == 0x00008800) {
			debug("fstcr\tr%i,%s", s1,
			    m88k_fcr_name(cpu, cr6));
			if (s1 != s2)
				debug("\t\t; NOTE: weird encoding: "
				    "low 5 bits = 0x%02x", s2);
			debug("\n");
		} else if ((iw & 0x0000f800) == 0x0000c000) {
			debug("xcr\tr%i,r%i,%s", d, s1,
			    m88k_cr_name(cpu, cr6));
			if (s1 != s2)
				debug("\t\t; NOTE: weird encoding: "
				    "low 5 bits = 0x%02x", s2);
			debug("\n");
		} else if ((iw & 0x0000f800) == 0x0000c800) {
			debug("fxcr\tr%i,r%i,%s", d, s1,
			    m88k_fcr_name(cpu, cr6));
			if (s1 != s2)
				debug("\t\t; NOTE: weird encoding: "
				    "low 5 bits = 0x%02x", s2);
			debug("\n");
		} else {
			debug("UNIMPLEMENTED 0x20\n");
		}
		break;

	case 0x21:
		switch (op11) {
		case 0x00:	/*  fmul  */
		case 0x05:	/*  fadd  */
		case 0x06:	/*  fsub  */
		case 0x07:	/*  fcmp  */
		case 0x0e:	/*  fdiv  */
			switch (op11) {
			case 0x00: mnem = "fmul"; break;
			case 0x05: mnem = "fadd"; break;
			case 0x06: mnem = "fsub"; break;
			case 0x07: mnem = "fcmp"; break;
			case 0x0e: mnem = "fdiv"; break;
			}
			debug("%s.%c%c%c r%i,r%i,r%i\n",
			    mnem,
			    ((iw >> 5) & 1)? 'd' : 's',
			    ((iw >> 9) & 1)? 'd' : 's',
			    ((iw >> 7) & 1)? 'd' : 's',
			    d, s1, s2);
			break;
		case 0x04:	/*  flt  */
			switch (op11) {
			case 0x04: mnem = "flt"; break;
			}
			debug("%s.%cs\tr%i,r%i\n",
			    mnem,
			    ((iw >> 5) & 1)? 'd' : 's',
			    d, s2);
			break;
		case 0x09:	/*  int  */
		case 0x0a:	/*  nint  */
		case 0x0b:	/*  trnc  */
			switch (op11) {
			case 0x09: mnem = "int"; break;
			case 0x0a: mnem = "nint"; break;
			case 0x0b: mnem = "trnc"; break;
			}
			debug("%s.s%c r%i,r%i\n",
			    mnem,
			    ((iw >> 7) & 1)? 'd' : 's',
			    d, s2);
			break;
		default:debug("UNIMPLEMENTED 0x21, op11=0x%02x\n", op11);
		}
		break;

	case 0x30:
	case 0x31:
	case 0x32:
	case 0x33:
		debug("b%sr%s\t",
		    op26 >= 0x32? "s" : "",
		    op26 & 1? ".n" : "");
		debug("0x%08"PRIx32, (uint32_t) (dumpaddr + d26));
		symbol = get_symbol_name(&cpu->machine->symbol_context,
		    dumpaddr + d26, &offset);
		if (symbol != NULL && supervisor)
			debug("\t; <%s>", symbol);
		debug("\n");
		break;

	case 0x34:	/*  bb0    */
	case 0x35:	/*  bb0.n  */
	case 0x36:	/*  bb1    */
	case 0x37:	/*  bb1.n  */
	case 0x3a:	/*  bcnd    */
	case 0x3b:	/*  bcnd.n  */
		switch (op26) {
		case 0x34:
		case 0x35: mnem = "bb0"; break;
		case 0x36:
		case 0x37: mnem = "bb1"; break;
		case 0x3a:
		case 0x3b: mnem = "bcnd"; break;
		}
		debug("%s%s\t", mnem, op26 & 1? ".n" : "");
		if (op26 == 0x3a || op26 == 0x3b) {
			/*  Attempt to decode bcnd condition:  */
			switch (d) {
			case 0x1: debug("gt0"); break;
			case 0x2: debug("eq0"); break;
			case 0x3: debug("ge0"); break;
			case 0x7: debug("not_maxneg"); break;
			case 0x8: debug("maxneg"); break;
			case 0xc: debug("lt0"); break;
			case 0xd: debug("ne0"); break;
			case 0xe: debug("le0"); break;
			default:  debug("unimplemented_%i", d);
			}
		} else {
			debug("%i", d);
		}
		debug(",r%i,0x%08"PRIx32, s1, (uint32_t) (dumpaddr + d16));
		symbol = get_symbol_name(&cpu->machine->symbol_context,
		    dumpaddr + d16, &offset);
		if (symbol != NULL && supervisor)
			debug("\t; <%s>", symbol);
		debug("\n");
		break;

	case 0x3c:
		if ((iw & 0x0000f000)==0x1000 || (iw & 0x0000f000)==0x2000) {
			int scale = 0;

			/*  Load/store:  */
			debug("%s", (iw & 0x0000f000) == 0x1000? "ld" : "st");
			switch (iw & 0x00000c00) {
			case 0x000: scale = 8; debug(".d"); break;
			case 0x400: scale = 4; break;
			case 0x800: debug(".x"); break;
			default: debug(".UNIMPLEMENTED");
			}
			if (iw & 0x100)
				debug(".usr");
			if (iw & 0x80)
				debug(".wt");
			debug("\tr%i,r%i", d, s1);
			if (iw & 0x200)
				debug("[r%i]", s2);
			else
				debug(",r%i", s2);

			if (running && scale >= 1) {
				uint32_t tmpaddr = cpu->cd.m88k.r[s1];
				if (iw & 0x200)
					tmpaddr += scale * cpu->cd.m88k.r[s2];
				else
					tmpaddr += cpu->cd.m88k.r[s2];
				symbol = get_symbol_name(&cpu->machine->
				    symbol_context, tmpaddr, &offset);
				if (symbol != NULL && supervisor)
					debug("\t; [<%s>]", symbol);
				else
					debug("\t; [0x%08"PRIx32"]", tmpaddr);
			}

			debug("\n");
		} else switch (op10) {
		case 0x20:	/*  clr  */
		case 0x22:	/*  set  */
		case 0x24:	/*  ext  */
		case 0x26:	/*  extu  */
		case 0x28:	/*  mak  */
		case 0x2a:	/*  rot  */
			switch (op10) {
			case 0x20: mnem = "clr"; break;
			case 0x22: mnem = "set"; break;
			case 0x24: mnem = "ext"; break;
			case 0x26: mnem = "extu"; break;
			case 0x28: mnem = "mak"; break;
			case 0x2a: mnem = "rot"; break;
			}
			debug("%s\tr%i,r%i,", mnem, d, s1);
			/*  Don't include w5 for the rot instruction:  */
			if (op10 != 0x2a)
				debug("%i", w5);
			/*  Note: o5 = s2:  */
			debug("<%i>\n", s2);
			break;
		case 0x34:	/*  tb0  */
		case 0x36:	/*  tb1  */
			switch (op10) {
			case 0x34: mnem = "tb0"; break;
			case 0x36: mnem = "tb1"; break;
			}
			debug("%s\t%i,r%i,0x%x\n", mnem, d, s1, iw & 0x1ff);
			break;
		default:debug("UNIMPLEMENTED 0x3c, op10=0x%02x\n", op10);
		}
		break;

	case 0x3d:
		if ((iw & 0xf000) <= 0x3fff) {
			int scale = 0;

			/*  Load, Store, xmem, and lda:  */
			switch (iw & 0xf000) {
			case 0x2000: debug("st"); break;
			case 0x3000: debug("lda"); break;
			default:     if ((iw & 0xf800) >= 0x0800)
					  debug("ld");
				     else
					  debug("xmem");
			}
			if ((iw & 0xf000) >= 0x1000) {
				/*  ld, st, lda  */
				scale = 1 << (3 - ((iw >> 10) & 3));
				debug("%s", memop[(iw >> 10) & 3]);
			} else if ((iw & 0xf800) == 0x0000) {
				/*  xmem  */
				if (iw & 0x400)
					scale = 4;
				else
					debug(".bu"), scale = 1;
			} else {
				/*  ld  */
				if ((iw & 0xf00) < 0xc00)
					debug(".hu"), scale = 2;
				else
					debug(".bu"), scale = 1;
			}
			if (iw & 0x100)
				debug(".usr");
			if (iw & 0x80)
				debug(".wt");
			debug("\tr%i,r%i", d, s1);
			if (iw & 0x200)
				debug("[r%i]", s2);
			else
				debug(",r%i", s2);

			if (running && scale >= 1) {
				uint32_t tmpaddr = cpu->cd.m88k.r[s1];
				if (iw & 0x200)
					tmpaddr += scale * cpu->cd.m88k.r[s2];
				else
					tmpaddr += cpu->cd.m88k.r[s2];
				symbol = get_symbol_name(&cpu->machine->
				    symbol_context, tmpaddr, &offset);
				if (symbol != NULL && supervisor)
					debug("\t; [<%s>]", symbol);
				else
					debug("\t; [0x%08"PRIx32"]", tmpaddr);
			}

			debug("\n");
		} else switch ((iw >> 8) & 0xff) {
		case 0x40:	/*  and  */
		case 0x44:	/*  and.c  */
		case 0x50:	/*  xor  */
		case 0x54:	/*  xor.c  */
		case 0x58:	/*  or  */
		case 0x5c:	/*  or.c  */
		case 0x60:	/*  addu  */
		case 0x61:	/*  addu.co  */
		case 0x62:	/*  addu.ci  */
		case 0x63:	/*  addu.cio  */
		case 0x64:	/*  subu  */
		case 0x65:	/*  subu.co  */
		case 0x66:	/*  subu.ci  */
		case 0x67:	/*  subu.cio  */
		case 0x68:	/*  divu  */
		case 0x69:	/*  divu.d  */
		case 0x6c:	/*  mul  */
		case 0x6d:	/*  mulu.d  */
		case 0x6e:	/*  muls  */
		case 0x70:	/*  add  */
		case 0x71:	/*  add.co  */
		case 0x72:	/*  add.ci  */
		case 0x73:	/*  add.cio  */
		case 0x74:	/*  sub  */
		case 0x75:	/*  sub.co  */
		case 0x76:	/*  sub.ci  */
		case 0x77:	/*  sub.cio  */
		case 0x78:	/*  div  */
		case 0x7c:	/*  cmp  */
		case 0x80:	/*  clr  */
		case 0x88:	/*  set  */
		case 0x90:	/*  ext  */
		case 0x98:	/*  extu  */
		case 0xa0:	/*  mak  */
		case 0xa8:	/*  rot  */
			/*  Three-register opcodes:  */
			switch ((iw >> 8) & 0xff) {
			case 0x40: mnem = "and"; break;
			case 0x44: mnem = "and.c"; break;
			case 0x50: mnem = "xor"; break;
			case 0x54: mnem = "xor.c"; break;
			case 0x58: mnem = "or"; break;
			case 0x5c: mnem = "or.c"; break;
			case 0x60: mnem = "addu"; break;
			case 0x61: mnem = "addu.co"; break;
			case 0x62: mnem = "addu.ci"; break;
			case 0x63: mnem = "addu.cio"; break;
			case 0x64: mnem = "subu"; break;
			case 0x65: mnem = "subu.co"; break;
			case 0x66: mnem = "subu.ci"; break;
			case 0x67: mnem = "subu.cio"; break;
			case 0x68: mnem = "divu"; break;
			case 0x69: mnem = "divu.d"; break;
			case 0x6c: mnem = "mul"; break;
			case 0x6d: mnem = "mulu.d"; break;
			case 0x6e: mnem = "muls"; break;
			case 0x70: mnem = "add"; break;
			case 0x71: mnem = "add.co"; break;
			case 0x72: mnem = "add.ci"; break;
			case 0x73: mnem = "add.cio"; break;
			case 0x74: mnem = "sub"; break;
			case 0x75: mnem = "sub.co"; break;
			case 0x76: mnem = "sub.ci"; break;
			case 0x77: mnem = "sub.cio"; break;
			case 0x78: mnem = "div"; break;
			case 0x7c: mnem = "cmp"; break;
			case 0x80: mnem = "clr"; break;
			case 0x88: mnem = "set"; break;
			case 0x90: mnem = "ext"; break;
			case 0x98: mnem = "extu"; break;
			case 0xa0: mnem = "mak"; break;
			case 0xa8: mnem = "rot"; break;
			}
			debug("%s\tr%i,r%i,r%i\n", mnem, d, s1, s2);
			break;
		case 0xc0:	/*  jmp  */
		case 0xc4:	/*  jmp.n  */
		case 0xc8:	/*  jsr  */
		case 0xcc:	/*  jsr.n  */
			debug("%s%s\t(r%i)",
			    op11 & 1? "jsr" : "jmp",
			    iw & 0x400? ".n" : "",
			    s2);
			if (running) {
				uint32_t tmpaddr = cpu->cd.m88k.r[s2];
				symbol = get_symbol_name(&cpu->machine->
				    symbol_context, tmpaddr, &offset);
				debug("\t\t; ");
				if (symbol != NULL && supervisor)
					debug("<%s>", symbol);
				else
					debug("0x%08"PRIx32, tmpaddr);
			}
			debug("\n");
			break;
		case 0xe8:	/*  ff1  */
		case 0xec:	/*  ff0  */
			debug("%s\tr%i,r%i\n",
			    ((iw >> 8) & 0xff) == 0xe8 ? "ff1" : "ff0", d, s2);
			break;
		case 0xf8:	/*  tbnd  */
			debug("tbnd\tr%i,r%i\n", s1, s2);
			break;
		case 0xfc:
			switch (iw & 0xff) {
			case 0x00:
				debug("rte\n");
				break;
			case 0x01:
			case 0x02:
			case 0x03:
				debug("illop%i\n", iw & 0xff);
				break;
			case (M88K_PROM_INSTR & 0xff):
				debug("gxemul_prom_call\n");
				break;
			default:debug("UNIMPLEMENTED 0x3d,0xfc: 0x%02x\n",
				    iw & 0xff);
			}
			break;
		default:debug("UNIMPLEMENTED 0x3d, opbyte = 0x%02x\n",
			    (iw >> 8) & 0xff);
		}
		break;

	case 0x3e:
		debug("tbnd\tr%i,0x%x\n", s1, imm16);
		break;

	default:debug("UNIMPLEMENTED op26=0x%02x\n", op26);
	}

	return sizeof(uint32_t);
}


#include "tmp_m88k_tail.cc"


