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
 *  Hitachi SuperH ("SH") CPU emulation.
 *
 *  TODO: It would be nice if this could encompass both 64-bit SH5, and
 *        32-bit SH encodings. Right now, it only really supports 32-bit mode.
 *
 *  TODO: This actually only works or SH4 so far, not SH3.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "cpu.h"
#include "device.h"
#include "float_emul.h"
#include "interrupt.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"
#include "settings.h"
#include "symbol.h"

#include "thirdparty/sh4_exception.h"
#include "thirdparty/sh4_mmu.h"


#define DYNTRANS_32
#define DYNTRANS_DELAYSLOT
#include "tmp_sh_head.cc"


extern int quiet_mode;

void sh_pc_to_pointers(struct cpu *);

void sh3_cpu_interrupt_assert(struct interrupt *interrupt);
void sh3_cpu_interrupt_deassert(struct interrupt *interrupt);


/*
 *  sh_cpu_new():
 *
 *  Create a new SH cpu object.
 *
 *  Returns 1 on success, 0 if there was no matching SH processor with
 *  this cpu_type_name.
 */
int sh_cpu_new(struct cpu *cpu, struct memory *mem, struct machine *machine,
	int cpu_id, char *cpu_type_name)
{
	int i = 0;
	struct sh_cpu_type_def cpu_type_defs[] = SH_CPU_TYPE_DEFS;

	/*  Scan the cpu_type_defs list for this cpu type:  */
	while (cpu_type_defs[i].name != NULL) {
		if (strcasecmp(cpu_type_defs[i].name, cpu_type_name) == 0) {
			break;
		}
		i++;
	}
	if (cpu_type_defs[i].name == NULL)
		return 0;

	cpu->memory_rw = sh_memory_rw;

	cpu->cd.sh.cpu_type = cpu_type_defs[i];
	cpu->byte_order = EMUL_LITTLE_ENDIAN;
	cpu->is_32bit = cpu->cd.sh.cpu_type.bits == 32;

	if (!cpu->is_32bit) {
		fatal("SH64 emulation not implemented. Sorry.\n");
		exit(1);
	}

	cpu->instruction_has_delayslot = sh_cpu_instruction_has_delayslot;

	cpu->translate_v2p = sh_translate_v2p;

	cpu->run_instr = sh_run_instr;
	cpu->update_translation_table = sh_update_translation_table;
	cpu->invalidate_translation_caches =
	    sh_invalidate_translation_caches;
	cpu->invalidate_code_translation =
	    sh_invalidate_code_translation;

	/*  Only show name and caches etc for CPU nr 0 (in SMP machines):  */
	if (cpu_id == 0) {
		debug("%s", cpu->name);
	}

	/*  Initial value of FPSCR (according to the SH4 manual):  */
	cpu->cd.sh.fpscr = 0x00040001;

	/*  (Initial value of the program counter on reboot is 0xA0000000.)  */

	/*  Start in Privileged Mode:  */
	cpu->cd.sh.sr = SH_SR_MD | SH_SR_IMASK;

	/*  Stack pointer at end of physical RAM:  */
	cpu->cd.sh.r[15] = cpu->machine->physical_ram_in_mb * 1048576 - 64;

	CPU_SETTINGS_ADD_REGISTER64("pc", cpu->pc);
	CPU_SETTINGS_ADD_REGISTER32("sr", cpu->cd.sh.sr);
	CPU_SETTINGS_ADD_REGISTER32("pr", cpu->cd.sh.pr);
	CPU_SETTINGS_ADD_REGISTER32("vbr", cpu->cd.sh.vbr);
	CPU_SETTINGS_ADD_REGISTER32("gbr", cpu->cd.sh.gbr);
	CPU_SETTINGS_ADD_REGISTER32("macl", cpu->cd.sh.macl);
	CPU_SETTINGS_ADD_REGISTER32("mach", cpu->cd.sh.mach);
	CPU_SETTINGS_ADD_REGISTER32("expevt", cpu->cd.sh.expevt);
	CPU_SETTINGS_ADD_REGISTER32("intevt", cpu->cd.sh.intevt);
	CPU_SETTINGS_ADD_REGISTER32("tra", cpu->cd.sh.tra);
	CPU_SETTINGS_ADD_REGISTER32("fpscr", cpu->cd.sh.fpscr);
	CPU_SETTINGS_ADD_REGISTER32("fpul", cpu->cd.sh.fpul);
	for (i=0; i<SH_N_GPRS; i++) {
		char tmpstr[5];
		snprintf(tmpstr, sizeof(tmpstr), "r%i", i);
		CPU_SETTINGS_ADD_REGISTER32(tmpstr, cpu->cd.sh.r[i]);
	}
	for (i=0; i<SH_N_GPRS_BANKED; i++) {
		char tmpstr[15];
		snprintf(tmpstr, sizeof(tmpstr), "r%i_bank", i);
		CPU_SETTINGS_ADD_REGISTER32(tmpstr, cpu->cd.sh.r_bank[i]);
	}
	for (i=0; i<SH_N_FPRS; i++) {
		char tmpstr[6];
		snprintf(tmpstr, sizeof(tmpstr), "fr%i", i);
		CPU_SETTINGS_ADD_REGISTER32(tmpstr, cpu->cd.sh.fr[i]);
		snprintf(tmpstr, sizeof(tmpstr), "xf%i", i);
		CPU_SETTINGS_ADD_REGISTER32(tmpstr, cpu->cd.sh.xf[i]);
	}
	for (i=0; i<SH_N_ITLB_ENTRIES; i++) {
		char tmpstr[15];
		snprintf(tmpstr, sizeof(tmpstr), "itlb_hi_%i", i);
		CPU_SETTINGS_ADD_REGISTER32(tmpstr, cpu->cd.sh.itlb_hi[i]);
		snprintf(tmpstr, sizeof(tmpstr), "itlb_lo_%i", i);
		CPU_SETTINGS_ADD_REGISTER32(tmpstr, cpu->cd.sh.itlb_lo[i]);
	}
	for (i=0; i<SH_N_UTLB_ENTRIES; i++) {
		char tmpstr[15];
		snprintf(tmpstr, sizeof(tmpstr), "utlb_hi_%i", i);
		CPU_SETTINGS_ADD_REGISTER32(tmpstr, cpu->cd.sh.utlb_hi[i]);
		snprintf(tmpstr, sizeof(tmpstr), "utlb_lo_%i", i);
		CPU_SETTINGS_ADD_REGISTER32(tmpstr, cpu->cd.sh.utlb_lo[i]);
	}

	/*  Register the CPU's interrupts:  */
	if (cpu->cd.sh.cpu_type.arch == 4) {
		for (i=SH_INTEVT_NMI; i<0x1000; i+=0x20) {
			struct interrupt templ;
			char name[100];
			snprintf(name, sizeof(name), "%s.irq[0x%x]",
			    cpu->path, i);
			memset(&templ, 0, sizeof(templ));
			templ.line = i;
			templ.name = name;
			templ.extra = cpu;
			templ.interrupt_assert = sh_cpu_interrupt_assert;
			templ.interrupt_deassert = sh_cpu_interrupt_deassert;
			interrupt_handler_register(&templ);
		}
	} else {
		struct interrupt templ;
		memset(&templ, 0, sizeof(templ));
		templ.line = i;
		templ.name = cpu->path;
		templ.extra = cpu;
		templ.interrupt_assert = sh3_cpu_interrupt_assert;
		templ.interrupt_deassert = sh3_cpu_interrupt_deassert;
		interrupt_handler_register(&templ);
	}

	/*  SH4-specific memory mapped registers, TLBs, caches, etc:  */
	if (cpu->cd.sh.cpu_type.arch == 4) {
		cpu->cd.sh.pcic_pcibus = (struct pci_data *) device_add(machine, "sh4");

		/*
		 *  Interrupt Controller initial values, according to the
		 *  SH7760 manual:
		 */
		cpu->cd.sh.intc_iprd = 0xda74;
		cpu->cd.sh.intc_intmsk00 = 0xf3ff7fff;
		cpu->cd.sh.intc_intmsk04 = 0x00ffffff;
		/*  All others are zero.  */

		/*  TODO: Initial priorities?  */
		cpu->cd.sh.intc_intpri00 = 0x33333333;
		cpu->cd.sh.intc_intpri04 = 0x33333333;
		cpu->cd.sh.intc_intpri08 = 0x33333333;
		cpu->cd.sh.intc_intpri0c = 0x33333333;
	}

	sh_update_interrupt_priorities(cpu);

	return 1;
}


/*
 *  sh_update_interrupt_priorities():
 *
 *  SH interrupts are a bit complicated; there are several intc registers
 *  controlling priorities for various peripherals:
 *
 *  Register:  Bits 15..12  11..8  7..4      3..0
 *  ---------  -----------  -----  ----      ----
 *  ipra       TMU0         TMU1   TMU2      Reserved
 *  iprb       WDT          REF    Reserved  Reserved
 *  iprc       GPIO         DMAC   Reserved  H-UDI
 *  iprd       IRL0         IRL1   IRL2      IRL3
 *
 *  Register:  31..28  27..24  23..20  19..16  15..12  11..8   7..4   3..0
 *  ---------  ------  ------  ------  ------  ------  -----   ----   ----
 *  intpri00   IRQ4    IRQ5    IRQ6    IRQ7    Rsrvd.  Rsrvd.  Rsrvd. Reserved
 *  intpri04   HCAN2,0 HCAN2,1 SSI(0)  SSI(1)  HAC(0)  HAC(1)  I2C(0) I2C(1)
 *  intpri08   USB     LCDC    DMABRG  SCIF(0) SCIF(1) SCIF(2) SIM    HSPI
 *  intpri0c   Reserv. Reserv. MMCIF   Reserv. MFI     Rsrvd.  ADC    CMT
 */
void sh_update_interrupt_priorities(struct cpu *cpu)
{
	int i;

	/*
	 *  Set priorities of known interrupts, without affecting the
	 *  SH_INT_ASSERTED bit:
	 */

	for (i=SH4_INTEVT_IRQ0; i<=SH4_INTEVT_IRQ14; i+=0x20) {
		cpu->cd.sh.int_prio_and_pending[i/0x20] &= ~SH_INT_PRIO_MASK;
		cpu->cd.sh.int_prio_and_pending[i/0x20] |= (15 - ((i - 
		    SH4_INTEVT_IRQ0) / 0x20));
	}

	cpu->cd.sh.int_prio_and_pending[SH_INTEVT_TMU0_TUNI0 / 0x20] &=
	    ~SH_INT_PRIO_MASK;
	cpu->cd.sh.int_prio_and_pending[SH_INTEVT_TMU0_TUNI0 / 0x20] |=
	    (cpu->cd.sh.intc_ipra >> 12) & 0xf;

	cpu->cd.sh.int_prio_and_pending[SH_INTEVT_TMU1_TUNI1 / 0x20] &=
	    ~SH_INT_PRIO_MASK;
	cpu->cd.sh.int_prio_and_pending[SH_INTEVT_TMU1_TUNI1 / 0x20] |=
	    (cpu->cd.sh.intc_ipra >> 8) & 0xf;

	cpu->cd.sh.int_prio_and_pending[SH_INTEVT_TMU2_TUNI2 / 0x20] &=
	    ~SH_INT_PRIO_MASK;
	cpu->cd.sh.int_prio_and_pending[SH_INTEVT_TMU2_TUNI2 / 0x20] |=
	    (cpu->cd.sh.intc_ipra >> 4) & 0xf;

	for (i=SH4_INTEVT_SCIF_ERI; i<=SH4_INTEVT_SCIF_TXI; i+=0x20) {
		cpu->cd.sh.int_prio_and_pending[i/0x20] &= ~SH_INT_PRIO_MASK;
		cpu->cd.sh.int_prio_and_pending[i/0x20] |=
		    ((cpu->cd.sh.intc_intpri08 >> 16) & 0xf);
	}
}


/*
 *  sh3_cpu_interrupt_assert():
 *  sh3_cpu_interrupt_deassert():
 */
void sh3_cpu_interrupt_assert(struct interrupt *interrupt)
{
	/*  TODO  */
}
void sh3_cpu_interrupt_deassert(struct interrupt *interrupt)
{
	/*  TODO  */
}       


/*
 *  sh_cpu_interrupt_assert():
 */
void sh_cpu_interrupt_assert(struct interrupt *interrupt)
{
	struct cpu *cpu = (struct cpu *) interrupt->extra;
	unsigned int irq_nr = interrupt->line;
	unsigned int index = irq_nr / 0x20;
	unsigned int prio;

	/*  Assert the interrupt, and check its priority level:  */
	cpu->cd.sh.int_prio_and_pending[index] |= SH_INT_ASSERTED;
	prio = cpu->cd.sh.int_prio_and_pending[index] & SH_INT_PRIO_MASK;

	if (prio == 0) {
		/*  Interrupt not implemented? Hm.  */
		fatal("[ SH interrupt 0x%x, prio 0 (?), aborting ]\n", irq_nr);
		exit(1);
	}

	if (cpu->cd.sh.int_to_assert == 0 || prio > cpu->cd.sh.int_level) {
		cpu->cd.sh.int_to_assert = irq_nr;
		cpu->cd.sh.int_level = prio;
	}
}


/*
 *  sh_cpu_interrupt_deassert():
 */
void sh_cpu_interrupt_deassert(struct interrupt *interrupt)
{
	struct cpu *cpu = (struct cpu *) interrupt->extra;
	int irq_nr = interrupt->line;
	int index = irq_nr / 0x20;

	/*  Deassert the interrupt:  */
	if (cpu->cd.sh.int_prio_and_pending[index] & SH_INT_ASSERTED) {
		cpu->cd.sh.int_prio_and_pending[index] &= ~SH_INT_ASSERTED;

		/*  Calculate new interrupt assertion:  */
		cpu->cd.sh.int_to_assert = 0;
		cpu->cd.sh.int_level = 0;

		/*  NOTE/TODO: This is slow, but should hopefully work:  */
		for (index=0; index<0x1000/0x20; index++) {
			uint8_t x = cpu->cd.sh.int_prio_and_pending[index];
			uint8_t prio = x & SH_INT_PRIO_MASK;
			if (x & SH_INT_ASSERTED &&
			    prio > cpu->cd.sh.int_level) {
				cpu->cd.sh.int_to_assert = index * 0x20;
				cpu->cd.sh.int_level = prio;
			}
		}
	}
}


/*
 *  sh_cpu_list_available_types():
 *
 *  Print a list of available SH CPU types.
 */
void sh_cpu_list_available_types(void)
{
	int i = 0, j;
	struct sh_cpu_type_def tdefs[] = SH_CPU_TYPE_DEFS;

	while (tdefs[i].name != NULL) {
		debug("%s", tdefs[i].name);
		for (j=10 - strlen(tdefs[i].name); j>0; j--)
			debug(" ");
		i ++;
		if ((i % 6) == 0 || tdefs[i].name == NULL)
			debug("\n");
	}
}


/*
 *  sh_cpu_dumpinfo():
 */
void sh_cpu_dumpinfo(struct cpu *cpu)
{
	debug(" (%s-endian)\n",
	    cpu->byte_order == EMUL_BIG_ENDIAN? "Big" : "Little");
}


/*
 *  sh_cpu_instruction_has_delayslot():
 *
 *  Return 1 if an opcode is a branch, 0 otherwise.
 */
int sh_cpu_instruction_has_delayslot(struct cpu *cpu, unsigned char *ib)
{
	uint16_t iword = *((uint16_t *)&ib[0]);
	int hi4, lo4, lo8;

	if (!cpu->is_32bit)
		return 0;

	if (cpu->byte_order == EMUL_BIG_ENDIAN)
		iword = BE16_TO_HOST(iword);
	else
		iword = LE16_TO_HOST(iword);

	hi4 = iword >> 12; lo4 = iword & 15; lo8 = iword & 255;

        switch (hi4) {
	case 0x0:
		if (iword == 0x000b)	/*  rts  */
			return 1;
		if (iword == 0x002b)	/*  rte  */
			return 1;
		if (lo8 == 0x03)	/*  bsrf  */
			return 1;
		if (lo8 == 0x23)	/*  braf  */
			return 1;
		break;
	case 0x4:
		switch (lo8) {
		case 0x0b:	/*  jsr  */
		case 0x2b:	/*  jmp  */
			return 1;
		}
		break;
	case 0x8:
		switch ((iword >> 8) & 0xf) {
		case 0xd:	/*  bt/s  */
		case 0xf:	/*  bf/s  */
			return 1;
		}
		break;
	case 0xa:	/*  bra  */
	case 0xb:	/*  bsr  */
		return 1;
	}

	return 0;
}


/*
 *  sh_cpu_register_dump():
 *
 *  Dump cpu registers in a relatively readable format.
 *
 *  gprs: set to non-zero to dump GPRs and some special-purpose registers.
 *  coprocs: set bit 0..3 to dump registers in coproc 0..3.
 */
void sh_cpu_register_dump(struct cpu *cpu, int gprs, int coprocs)
{
	char *symbol;
	uint64_t offset;
	int i, x = cpu->cpu_id;

	if (gprs) {
		/*  Special registers (pc, ...) first:  */
		symbol = get_symbol_name(&cpu->machine->symbol_context,
		    cpu->pc, &offset);

		debug("cpu%i: pc  = 0x%08"PRIx32, x, (uint32_t)cpu->pc);
		debug("  <%s>\n", symbol != NULL? symbol : " no symbol ");

		debug("cpu%i: sr  = 0x%08"PRIx32"  (%s, %s, %s, %s, %s, %s,"
		    " imask=0x%x, %s, %s)\n", x, (int32_t)cpu->cd.sh.sr,
		    (cpu->cd.sh.sr & SH_SR_MD)? "MD" : "!md",
		    (cpu->cd.sh.sr & SH_SR_RB)? "RB" : "!rb",
		    (cpu->cd.sh.sr & SH_SR_BL)? "BL" : "!bl",
		    (cpu->cd.sh.sr & SH_SR_FD)? "FD" : "!fd",
		    (cpu->cd.sh.sr & SH_SR_M)? "M" : "!m",
		    (cpu->cd.sh.sr & SH_SR_Q)? "Q" : "!q",
		    (cpu->cd.sh.sr & SH_SR_IMASK) >> SH_SR_IMASK_SHIFT,
		    (cpu->cd.sh.sr & SH_SR_S)? "S" : "!s",
		    (cpu->cd.sh.sr & SH_SR_T)? "T" : "!t");

		symbol = get_symbol_name(&cpu->machine->symbol_context,
		    cpu->cd.sh.pr, &offset);
		debug("cpu%i: pr  = 0x%08"PRIx32, x, (uint32_t)cpu->cd.sh.pr);
		debug("  <%s>\n", symbol != NULL? symbol : " no symbol ");

		debug("cpu%i: mach = 0x%08"PRIx32"  macl = 0x%08"PRIx32
		    "  gbr = 0x%08"PRIx32"\n", x, (uint32_t)cpu->cd.sh.mach,
		    (uint32_t)cpu->cd.sh.macl, (uint32_t)cpu->cd.sh.gbr);

		for (i=0; i<SH_N_GPRS; i++) {
			if ((i % 4) == 0)
				debug("cpu%i:", x);
			debug(" r%-2i = 0x%08x ", i, (int)cpu->cd.sh.r[i]);
			if ((i % 4) == 3)
				debug("\n");
		}
	}

	if (coprocs & 1) {
		/*  Floating point:  */
		debug("cpu%i: fpscr = 0x%08"PRIx32" (%s,%s,%s)  fpul = 0x%08"
		    PRIx32"\n", x, cpu->cd.sh.fpscr,
		    cpu->cd.sh.fpscr & SH_FPSCR_PR? "PR" : "!pr",
		    cpu->cd.sh.fpscr & SH_FPSCR_SZ? "SZ" : "!sz",
		    cpu->cd.sh.fpscr & SH_FPSCR_FR? "FR" : "!fr",
		    cpu->cd.sh.fpul);

		for (i=0; i<SH_N_FPRS; i++) {
			if ((i % 4) == 0)
				debug("cpu%i:", x);
			debug(" fr%-2i=0x%08x ", i, (int)cpu->cd.sh.fr[i]);
			if ((i % 4) == 3)
				debug("\n");
		}

		for (i=0; i<SH_N_FPRS; i++) {
			if ((i % 4) == 0)
				debug("cpu%i:", x);
			debug(" xf%-2i=0x%08x ", i, (int)cpu->cd.sh.xf[i]);
			if ((i % 4) == 3)
				debug("\n");
		}
	}

	if (coprocs & 2) {
		/*  System registers, etc:  */
		debug("cpu%i: vbr = 0x%08"PRIx32"  sgr = 0x%08"PRIx32
		    "\n", x, cpu->cd.sh.vbr, cpu->cd.sh.sgr);
		debug("cpu%i: spc = 0x%08"PRIx32"  ssr = 0x%08"PRIx32"\n",
		    x, cpu->cd.sh.spc, cpu->cd.sh.ssr);
		debug("cpu%i: expevt = 0x%"PRIx32"  intevt = 0x%"PRIx32
		    "  tra = 0x%"PRIx32"\n", x, cpu->cd.sh.expevt,
		    cpu->cd.sh.intevt, cpu->cd.sh.tra);

		for (i=0; i<SH_N_GPRS_BANKED; i++) {
			if ((i % 2) == 0)
				debug("cpu%i:", x);
			debug(" r%i_bank = 0x%08x ", i,
			    (int)cpu->cd.sh.r_bank[i]);
			if ((i % 2) == 1)
				debug("\n");
		}
	}
}


/*
 *  sh_cpu_tlbdump():
 *
 *  Called from the debugger to dump the TLB in a readable format.
 *  x is the cpu number to dump, or -1 to dump all CPUs.
 *
 *  If rawflag is nonzero, then the TLB contents isn't formated nicely,
 *  just dumped.
 */
void sh_cpu_tlbdump(struct machine *m, int x, int rawflag)
{
	int i, j;

	for (j=0; j<m->ncpus; j++) {
		struct cpu *cpu = m->cpus[j];

		if (x >= 0 && j != x)
			continue;

		for (i=0; i<SH_N_ITLB_ENTRIES; i++)
			printf("cpu%i: itlb_hi_%-2i = 0x%08"PRIx32"  "
			    "itlb_lo_%-2i = 0x%08"PRIx32"\n", j, i,
			    (uint32_t) cpu->cd.sh.itlb_hi[i], i,
			    (uint32_t) cpu->cd.sh.itlb_lo[i]);
		for (i=0; i<SH_N_UTLB_ENTRIES; i++)
			printf("cpu%i: utlb_hi_%-2i = 0x%08"PRIx32"  "
			    "utlb_lo_%-2i = 0x%08"PRIx32"\n", j, i,
			    (uint32_t) cpu->cd.sh.utlb_hi[i], i,
			    (uint32_t) cpu->cd.sh.utlb_lo[i]);
	}
}


/*
 *  sh_update_sr():
 *
 *  Writes a new value to the status register.
 */
void sh_update_sr(struct cpu *cpu, uint32_t new_sr)
{
	uint32_t old_sr = cpu->cd.sh.sr;

	if ((new_sr & SH_SR_RB) != (old_sr & SH_SR_RB)) {
		int i;
		for (i=0; i<SH_N_GPRS_BANKED; i++) {
			uint32_t tmp = cpu->cd.sh.r[i];
			cpu->cd.sh.r[i] = cpu->cd.sh.r_bank[i];
			cpu->cd.sh.r_bank[i] = tmp;
		}
	}

	cpu->cd.sh.sr = new_sr;
}


/*
 *  sh_update_fpscr():
 *
 *  Writes a new value to the floating-point status/control register.
 */
void sh_update_fpscr(struct cpu *cpu, uint32_t new_fpscr)
{
	uint32_t old_fpscr = cpu->cd.sh.fpscr;

	if ((new_fpscr & SH_FPSCR_FR) != (old_fpscr & SH_FPSCR_FR)) {
		int i;
		for (i=0; i<SH_N_FPRS; i++) {
			uint32_t tmp = cpu->cd.sh.fr[i];
			cpu->cd.sh.fr[i] = cpu->cd.sh.xf[i];
			cpu->cd.sh.xf[i] = tmp;
		}
	}

	cpu->cd.sh.fpscr = new_fpscr;
}


/*
 *  sh_exception():
 *
 *  Causes a transfer of control to an exception or interrupt handler.
 *  If intevt > 0, then it is an interrupt, otherwise an exception.
 *
 *  vaddr contains the faulting address, on TLB exceptions.
 */
void sh_exception(struct cpu *cpu, int expevt, int intevt, uint32_t vaddr)
{
	uint32_t vbr = cpu->cd.sh.vbr;

	if (!quiet_mode) {
		if (intevt > 0)
			debug("[ interrupt 0x%03x", intevt);
		else
			debug("[ exception 0x%03x", expevt);

		debug(", pc=0x%08"PRIx32" ", (uint32_t)cpu->pc);
		if (intevt == 0)
			debug("vaddr=0x%08"PRIx32" ", vaddr);

		debug(" ]\n");
	}

	if (cpu->cd.sh.sr & SH_SR_BL) {
		fatal("[ sh_exception(): BL bit already set. ]\n");

		/*  This is actually OK in two cases: a User Break,
		    or on NMI interrupts if a special flag is set?  */
		/*  TODO  */

		expevt = EXPEVT_RESET_POWER;
	}

	if (cpu->is_halted) {
		/*
		 *  If the exception occurred on a 'sleep' instruction, then let
		 *  the instruction following the sleep instruction be the one
		 *  where execution resumes when the interrupt service routine
		 *  returns.
		 */
		cpu->is_halted = 0;
		cpu->pc += sizeof(uint16_t);
	}

	if (cpu->delay_slot) {
		cpu->delay_slot = EXCEPTION_IN_DELAY_SLOT;
		cpu->pc -= sizeof(uint16_t);
	}


	/*
	 *  Stuff common to all exceptions:
	 */

	cpu->cd.sh.spc = cpu->pc;
	cpu->cd.sh.ssr = cpu->cd.sh.sr;
	cpu->cd.sh.sgr = cpu->cd.sh.r[15];

	if (intevt > 0) {
		cpu->cd.sh.intevt = intevt;
		expevt = -1;
	} else {
		cpu->cd.sh.expevt = expevt;
	}

	sh_update_sr(cpu, cpu->cd.sh.sr | SH_SR_MD | SH_SR_RB | SH_SR_BL);

	/*  Most exceptions set PC to VBR + 0x100.  */
	cpu->pc = vbr + 0x100;


	/*  Specific cases:  */
	switch (expevt) {

	case -1:	/*  Interrupt  */
		cpu->pc = vbr + 0x600;
		break;

	case EXPEVT_RESET_POWER:
	case EXPEVT_RESET_MANUAL:
		cpu->pc = 0xa0000000;
		cpu->cd.sh.vbr = 0x00000000;
		sh_update_sr(cpu, (cpu->cd.sh.sr | SH_SR_IMASK) & ~SH_SR_FD);
		break;

	case EXPEVT_TLB_MISS_LD:
	case EXPEVT_TLB_MISS_ST:
		cpu->pc = vbr + 0x400;
	case EXPEVT_TLB_PROT_LD:
	case EXPEVT_TLB_PROT_ST:
	case EXPEVT_TLB_MOD:
		cpu->cd.sh.tea = vaddr;
		cpu->cd.sh.pteh &= ~SH4_PTEH_VPN_MASK;
		cpu->cd.sh.pteh |= (vaddr & SH4_PTEH_VPN_MASK);
		break;

	case EXPEVT_TRAPA:
		/*
		 *  Note: The TRA register is already set by the implementation
		 *  of the trapa instruction. See cpu_sh_instr.c for details.
		 *  Here, spc is incremented, so that a return from the trap
		 *  handler transfers control to the instruction _following_
		 *  the trapa.
		 */
		cpu->cd.sh.spc += sizeof(uint16_t);
		break;

	case EXPEVT_RES_INST:
		/*
		 *  Note: Having this code here makes it possible to catch
		 *  reserved instructions; during normal instruction execution,
		 *  these are not very common.
		 */
#if 1
		printf("\nRESERVED SuperH instruction at spc=%08"PRIx32"\n",
		    cpu->cd.sh.spc);
		exit(1);
#else
		break;
#endif

	case EXPEVT_FPU_DISABLE:
		break;

	default:fatal("sh_exception(): exception 0x%x is not yet "
		    "implemented.\n", expevt);
		exit(1);
	}

	sh_pc_to_pointers(cpu);
}


/*
 *  sh_cpu_disassemble_instr():
 *
 *  SHcompact instruction disassembly. The top 4 bits of each 16-bit
 *  instruction word is used as the main opcode. For most instructions, the
 *  lowest 4 or 8 bits then select sub-opcode.
 *
 *  This function convert an instruction word into human readable format,
 *  for instruction tracing.
 *
 *  If running is 1, cpu->pc should be the address of the instruction.
 *
 *  If running is 0, things that depend on the runtime environment (eg.
 *  register contents) will not be shown, and addr will be used instead of
 *  cpu->pc for relative addresses.
 */
int sh_cpu_disassemble_instr(struct cpu *cpu, unsigned char *instr,
	int running, uint64_t dumpaddr)
{
	char *symbol;
	uint64_t offset, addr;
	uint16_t iword;
	int hi4, lo4, lo8, r8, r4;

	if (running)
		dumpaddr = cpu->pc;

	symbol = get_symbol_name(&cpu->machine->symbol_context,
	    dumpaddr, &offset);
	if (symbol != NULL && offset==0)
		debug("<%s>\n", symbol);

	if (cpu->machine->ncpus > 1 && running)
		debug("cpu%i: ", cpu->cpu_id);

	debug("%08"PRIx32, (uint32_t) dumpaddr);

	if (cpu->byte_order == EMUL_BIG_ENDIAN)
		iword = (instr[0] << 8) + instr[1];
	else
		iword = (instr[1] << 8) + instr[0];

	debug(":  %04x %s\t", iword, cpu->delay_slot? "(d)" : "");
	hi4 = iword >> 12; lo4 = iword & 15; lo8 = iword & 255;
	r8 = (iword >> 8) & 15; r4 = (iword >> 4) & 15;


	/*
	 *  Decode the instruction:
	 */

	switch (hi4) {
	case 0x0:
		if (lo8 == 0x02)
			debug("stc\tsr,r%i\n", r8);
		else if (lo8 == 0x03)
			debug("bsrf\tr%i\n", r8);
		else if (lo4 >= 4 && lo4 <= 6) {
			if (lo4 == 0x4)
				debug("mov.b\tr%i,@(r0,r%i)", r4, r8);
			else if (lo4 == 0x5)
				debug("mov.w\tr%i,@(r0,r%i)", r4, r8);
			else if (lo4 == 0x6)
				debug("mov.l\tr%i,@(r0,r%i)", r4, r8);
			if (running) {
				uint32_t addr = cpu->cd.sh.r[0] +
				    cpu->cd.sh.r[r8];
				debug("\t; r0+r%i = ", r8);
				symbol = get_symbol_name(
				    &cpu->machine->symbol_context,
				    addr, &offset);
				if (symbol != NULL)
					debug("<%s>", symbol);
				else
					debug("0x%08"PRIx32, addr);
			}
			debug("\n");
		} else if (lo4 == 0x7)
			debug("mul.l\tr%i,r%i\n", r4, r8);
		else if (iword == 0x0008)
			debug("clrt\n");
		else if (iword == 0x0009)
			debug("nop\n");
		else if (lo8 == 0x0a)
			debug("sts\tmach,r%i\n", r8);
		else if (iword == 0x000b)
			debug("rts\n");
		else if (lo4 >= 0xc && lo4 <= 0xe) {
			if (lo4 == 0xc)
				debug("mov.b\t@(r0,r%i),r%i", r4, r8);
			else if (lo4 == 0xd)
				debug("mov.w\t@(r0,r%i),r%i", r4, r8);
			else if (lo4 == 0xe)
				debug("mov.l\t@(r0,r%i),r%i", r4, r8);
			if (running) {
				uint32_t addr = cpu->cd.sh.r[0] +
				    cpu->cd.sh.r[r4];
				debug("\t; r0+r%i = ", r4);
				symbol = get_symbol_name(
				    &cpu->machine->symbol_context,
				    addr, &offset);
				if (symbol != NULL)
					debug("<%s>", symbol);
				else
					debug("0x%08"PRIx32, addr);
			}
			debug("\n");
		} else if (lo8 == 0x12)
			debug("stc\tgbr,r%i\n", r8);
		else if (iword == 0x0018)
			debug("sett\n");
		else if (iword == 0x0019)
			debug("div0u\n");
		else if (lo8 == 0x1a)
			debug("sts\tmacl,r%i\n", r8);
		else if (iword == 0x001b)
			debug("sleep\n");
		else if (lo8 == 0x22)
			debug("stc\tvbr,r%i\n", r8);
		else if (lo8 == 0x23)
			debug("braf\tr%i\n", r8);
		else if (iword == 0x0028)
			debug("clrmac\n");
		else if (lo8 == 0x29)
			debug("movt\tr%i\n", r8);
		else if (lo8 == 0x2a)
			debug("sts\tpr,r%i\n", r8);
		else if (iword == 0x002b)
			debug("rte\n");
		else if (lo8 == 0x32)
			debug("stc\tssr,r%i\n", r8);
		else if (iword == 0x0038)
			debug("ldtlb\n");
		else if (iword == 0x003b)
			debug("brk\n");
		else if (lo8 == 0x42)
			debug("stc\tspc,r%i\n", r8);
		else if (iword == 0x0048)
			debug("clrs\n");
		else if (iword == 0x0058)
			debug("sets\n");
		else if (lo8 == 0x5a)
			debug("sts\tfpul,r%i\n", r8);
		else if (lo8 == 0x6a)
			debug("sts\tfpscr,r%i\n", r8);
		else if ((lo8 & 0x8f) == 0x82)
			debug("stc\tr%i_bank,r%i\n", (lo8 >> 4) & 7, r8);
		else if (lo8 == 0x83)
			debug("pref\t@r%i\n", r8);
		else if (lo8 == 0x93)
			debug("ocbi\t@r%i\n", r8);
		else if (lo8 == 0xa3)
			debug("ocbp\t@r%i\n", r8);
		else if (lo8 == 0xb3)
			debug("ocbwb\t@r%i\n", r8);
		else if (lo8 == 0xc3)
			debug("movca.l\tr0,@r%i\n", r8);
		else if (lo8 == 0xfa)
			debug("stc\tdbr,r%i\n", r8);
		else if (iword == SH_INVALID_INSTR)
			debug("gxemul_dreamcast_prom_emul\n");
		else
			debug("UNIMPLEMENTED hi4=0x%x, lo8=0x%02x\n", hi4, lo8);
		break;
	case 0x1:
		debug("mov.l\tr%i,@(%i,r%i)", r4, lo4 * 4, r8);
		if (running) {
			uint32_t addr = cpu->cd.sh.r[r8] + lo4 * 4;
			debug("\t; r%i+%i = ", r8, lo4 * 4);
			symbol = get_symbol_name(&cpu->machine->symbol_context,
			    addr, &offset);
			if (symbol != NULL)
				debug("<%s>", symbol);
			else
				debug("0x%08"PRIx32, addr);
		}
		debug("\n");
		break;
	case 0x2:
		if (lo4 == 0x0)
			debug("mov.b\tr%i,@r%i\n", r4, r8);
		else if (lo4 == 0x1)
			debug("mov.w\tr%i,@r%i\n", r4, r8);
		else if (lo4 == 0x2)
			debug("mov.l\tr%i,@r%i\n", r4, r8);
		else if (lo4 == 0x4)
			debug("mov.b\tr%i,@-r%i\n", r4, r8);
		else if (lo4 == 0x5)
			debug("mov.w\tr%i,@-r%i\n", r4, r8);
		else if (lo4 == 0x6)
			debug("mov.l\tr%i,@-r%i\n", r4, r8);
		else if (lo4 == 0x7)
			debug("div0s\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0x8)
			debug("tst\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0x9)
			debug("and\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0xa)
			debug("xor\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0xb)
			debug("or\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0xc)
			debug("cmp/str\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0xd)
			debug("xtrct\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0xe)
			debug("mulu.w\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0xf)
			debug("muls.w\tr%i,r%i\n", r4, r8);
		else
			debug("UNIMPLEMENTED hi4=0x%x, lo8=0x%02x\n", hi4, lo8);
		break;
	case 0x3:
		if (lo4 == 0x0)
			debug("cmp/eq\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0x2)
			debug("cmp/hs\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0x3)
			debug("cmp/ge\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0x4)
			debug("div1\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0x5)
			debug("dmulu.l\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0x6)
			debug("cmp/hi\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0x7)
			debug("cmp/gt\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0x8)
			debug("sub\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0xa)
			debug("subc\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0xb)
			debug("subv\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0xc)
			debug("add\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0xd)
			debug("dmuls.l\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0xe)
			debug("addc\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0xf)
			debug("addv\tr%i,r%i\n", r4, r8);
		else
			debug("UNIMPLEMENTED hi4=0x%x, lo8=0x%02x\n", hi4, lo8);
		break;
	case 0x4:
		if (lo8 == 0x00)
			debug("shll\tr%i\n", r8);
		else if (lo8 == 0x01)
			debug("shlr\tr%i\n", r8);
		else if (lo8 == 0x02)
			debug("sts.l\tmach,@-r%i\n", r8);
		else if (lo8 == 0x03)
			debug("stc.l\tsr,@-r%i\n", r8);
		else if (lo8 == 0x04)
			debug("rotl\tr%i\n", r8);
		else if (lo8 == 0x05)
			debug("rotr\tr%i\n", r8);
		else if (lo8 == 0x06)
			debug("lds.l\t@r%i+,mach\n", r8);
		else if (lo8 == 0x07)
			debug("ldc.l\t@r%i+,sr\n", r8);
		else if (lo8 == 0x08)
			debug("shll2\tr%i\n", r8);
		else if (lo8 == 0x09)
			debug("shlr2\tr%i\n", r8);
		else if (lo8 == 0x0a)
			debug("lds\tr%i,mach\n", r8);
		else if (lo8 == 0x0b)
			debug("jsr\t@r%i\n", r8);
		else if (lo4 == 0xc)
			debug("shad\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0xd)
			debug("shld\tr%i,r%i\n", r4, r8);
		else if (lo8 == 0x0e)
			debug("ldc\tr%i,sr\n", r8);
		else if (lo8 == 0x10)
			debug("dt\tr%i\n", r8);
		else if (lo8 == 0x11)
			debug("cmp/pz\tr%i\n", r8);
		else if (lo8 == 0x12)
			debug("sts.l\tmacl,@-r%i\n", r8);
		else if (lo8 == 0x13)
			debug("stc.l\tgbr,@-r%i\n", r8);
		else if (lo8 == 0x15)
			debug("cmp/pl\tr%i\n", r8);
		else if (lo8 == 0x16)
			debug("lds.l\t@r%i+,macl\n", r8);
		else if (lo8 == 0x17)
			debug("ldc.l\t@r%i+,gbr\n", r8);
		else if (lo8 == 0x18)
			debug("shll8\tr%i\n", r8);
		else if (lo8 == 0x19)
			debug("shlr8\tr%i\n", r8);
		else if (lo8 == 0x1a)
			debug("lds\tr%i,macl\n", r8);
		else if (lo8 == 0x1b)
			debug("tas.b\t@r%i\n", r8);
		else if (lo8 == 0x1e)
			debug("ldc\tr%i,gbr\n", r8);
		else if (lo8 == 0x20)
			debug("shal\tr%i\n", r8);
		else if (lo8 == 0x21)
			debug("shar\tr%i\n", r8);
		else if (lo8 == 0x22)
			debug("sts.l\tpr,@-r%i\n", r8);
		else if (lo8 == 0x23)
			debug("stc.l\tvbr,@-r%i\n", r8);
		else if (lo8 == 0x24)
			debug("rotcl\tr%i\n", r8);
		else if (lo8 == 0x25)
			debug("rotcr\tr%i\n", r8);
		else if (lo8 == 0x26)
			debug("lds.l\t@r%i+,pr\n", r8);
		else if (lo8 == 0x27)
			debug("ldc.l\t@r%i+,vbr\n", r8);
		else if (lo8 == 0x28)
			debug("shll16\tr%i\n", r8);
		else if (lo8 == 0x29)
			debug("shlr16\tr%i\n", r8);
		else if (lo8 == 0x2a)
			debug("lds\tr%i,pr\n", r8);
		else if (lo8 == 0x2b) {
			debug("jmp\t@r%i", r8);
			if (running) {
				symbol = get_symbol_name(
				    &cpu->machine->symbol_context,
				    cpu->cd.sh.r[r8], &offset);
				if (symbol != NULL)
					debug("\t\t; <%s>", symbol);
			}
			debug("\n");
		} else if (lo8 == 0x2e)
			debug("ldc\tr%i,vbr\n", r8);
		else if (lo8 == 0x33)
			debug("stc.l\tssr,@-r%i\n", r8);
		else if (lo8 == 0x37)
			debug("ldc.l\t@r%i+,ssr\n", r8);
		else if (lo8 == 0x3e)
			debug("ldc\tr%i,ssr\n", r8);
		else if (lo8 == 0x43)
			debug("stc.l\tspc,@-r%i\n", r8);
		else if (lo8 == 0x47)
			debug("ldc.l\t@r%i+,spc\n", r8);
		else if (lo8 == 0x4e)
			debug("ldc\tr%i,spc\n", r8);
		else if (lo8 == 0x52)
			debug("sts.l\tfpul,@-r%i\n", r8);
		else if (lo8 == 0x56)
			debug("lds.l\t@r%i+,fpul\n", r8);
		else if (lo8 == 0x5a)
			debug("lds\tr%i,fpul\n", r8);
		else if (lo8 == 0x62)
			debug("sts.l\tfpscr,@-r%i\n", r8);
		else if (lo8 == 0x66)
			debug("lds.l\t@r%i+,fpscr\n", r8);
		else if (lo8 == 0x6a)
			debug("lds\tr%i,fpscr\n", r8);
		else if ((lo8 & 0x8f) == 0x83)
			debug("stc.l\tr%i_bank,@-r%i\n", (lo8 >> 4) & 7, r8);
		else if ((lo8 & 0x8f) == 0x87)
			debug("ldc.l\t@r%i,r%i_bank\n", r8, (lo8 >> 4) & 7, r8);
		else if ((lo8 & 0x8f) == 0x8e)
			debug("ldc\tr%i,r%i_bank\n", r8, (lo8 >> 4) & 7);
		else if (lo8 == 0xfa)
			debug("ldc\tr%i,dbr\n", r8);
		else
			debug("UNIMPLEMENTED hi4=0x%x, lo8=0x%02x\n", hi4, lo8);
		break;
	case 0x5:
		debug("mov.l\t@(%i,r%i),r%i", lo4 * 4, r4, r8);
		if (running) {
			debug("\t; r%i+%i = 0x%08"PRIx32, r4, lo4 * 4,
			    cpu->cd.sh.r[r4] + lo4 * 4);
		}
		debug("\n");
		break;
	case 0x6:
		if (lo4 == 0x0)
			debug("mov.b\t@r%i,r%i\n", r4, r8);
		else if (lo4 == 0x1)
			debug("mov.w\t@r%i,r%i\n", r4, r8);
		else if (lo4 == 0x2)
			debug("mov.l\t@r%i,r%i\n", r4, r8);
		else if (lo4 == 0x3)
			debug("mov\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0x4)
			debug("mov.b\t@r%i+,r%i\n", r4, r8);
		else if (lo4 == 0x5)
			debug("mov.w\t@r%i+,r%i\n", r4, r8);
		else if (lo4 == 0x6)
			debug("mov.l\t@r%i+,r%i\n", r4, r8);
		else if (lo4 == 0x7)
			debug("not\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0x8)
			debug("swap.b\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0x9)
			debug("swap.w\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0xa)
			debug("negc\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0xb)
			debug("neg\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0xc)
			debug("extu.b\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0xd)
			debug("extu.w\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0xe)
			debug("exts.b\tr%i,r%i\n", r4, r8);
		else if (lo4 == 0xf)
			debug("exts.w\tr%i,r%i\n", r4, r8);
		else
			debug("UNIMPLEMENTED hi4=0x%x, lo8=0x%02x\n", hi4, lo8);
		break;
	case 0x7:
		debug("add\t#%i,r%i\n", (int8_t)lo8, r8);
		break;
	case 0x8:
		if (r8 == 0 || r8 == 4) {
			if (r8 == 0x0)
				debug("mov.b\tr0,@(%i,r%i)", lo4, r4);
			else if (r8 == 0x4)
				debug("mov.b\t@(%i,r%i),r0", lo4, r4);
			if (running) {
				debug("\t; r%i+%i = 0x%08"PRIx32, r4, lo4,
				    cpu->cd.sh.r[r4] + lo4);
			}
			debug("\n");
		} else if (r8 == 1 || r8 == 5) {
			if (r8 == 0x1)
				debug("mov.w\tr0,@(%i,r%i)", lo4 * 2, r4);
			else if (r8 == 0x5)
				debug("mov.w\t@(%i,r%i),r0", lo4 * 2, r4);
			if (running) {
				debug("\t; r%i+%i = 0x%08"PRIx32, r4, lo4 * 2,
				    cpu->cd.sh.r[r4] + lo4 * 2);
			}
			debug("\n");
		} else if (r8 == 0x8) {
			debug("cmp/eq\t#%i,r0\n", (int8_t)lo8);
		} else if (r8 == 0x9 || r8 == 0xb || r8 == 0xd || r8 == 0xf) {
			addr = (int8_t)lo8;
			addr = dumpaddr + 4 + (addr << 1);
			debug("b%s%s\t0x%x",
			    (r8 == 0x9 || r8 == 0xd)? "t" : "f",
			    (r8 == 0x9 || r8 == 0xb)? "" : "/s", (int)addr);
			symbol = get_symbol_name(&cpu->machine->symbol_context,
			    addr, &offset);
			if (symbol != NULL)
				debug("\t; <%s>", symbol);
			debug("\n");
		} else
			debug("UNIMPLEMENTED hi4=0x%x,0x%x\n", hi4, r8);
		break;
	case 0x9:
	case 0xd:
		addr = lo8 * (hi4==9? 2 : 4);
		addr += (dumpaddr & ~(hi4==9? 1 : 3)) + 4;
		debug("mov.%s\t0x%x,r%i", hi4==9? "w":"l", (int)addr, r8);
		symbol = get_symbol_name(&cpu->machine->symbol_context,
		    addr, &offset);
		if (symbol != NULL)
			debug("\t; <%s>", symbol);
		debug("\n");
		break;
	case 0xa:
	case 0xb:
		addr = (int32_t)(int16_t)((iword & 0xfff) << 4);
		addr = ((int32_t)addr >> 3);
		addr += dumpaddr + 4;
		debug("%s\t0x%x", hi4==0xa? "bra":"bsr", (int)addr);

		symbol = get_symbol_name(&cpu->machine->symbol_context,
		    addr, &offset);
		if (symbol != NULL)
			debug("\t; <%s>", symbol);
		debug("\n");
		break;
	case 0xc:
		if (r8 == 0x0)
			debug("mov.b\tr0,@(%i,gbr)\n", lo8);
		else if (r8 == 0x1)
			debug("mov.w\tr0,@(%i,gbr)\n", lo8 * 2);
		else if (r8 == 0x2)
			debug("mov.l\tr0,@(%i,gbr)\n", lo8 * 4);
		else if (r8 == 0x3)
			debug("trapa\t#%i\n", (uint8_t)lo8);
		else if (r8 == 0x4)
			debug("mov.b\t(%i,gbr),r0\n", lo8);
		else if (r8 == 0x5)
			debug("mov.w\t(%i,gbr),r0\n", lo8 * 2);
		else if (r8 == 0x6)
			debug("mov.l\t(%i,gbr),r0\n", lo8 * 4);
		else if (r8 == 0x7) {
			addr = lo8 * 4 + (dumpaddr & ~3) + 4;
			debug("mova\t0x%x,r0\n", (int)addr);
		} else if (r8 == 0x8)
			debug("tst\t#%i,r0\n", (uint8_t)lo8);
		else if (r8 == 0x9)
			debug("and\t#%i,r0\n", (uint8_t)lo8);
		else if (r8 == 0xa)
			debug("xor\t#%i,r0\n", (uint8_t)lo8);
		else if (r8 == 0xb)
			debug("or\t#%i,r0\n", (uint8_t)lo8);
		else if (r8 == 0xc)
			debug("tst.b\t#%i,@(r0,gbr)\n", (uint8_t)lo8);
		else if (r8 == 0xd)
			debug("and.b\t#%i,@(r0,gbr)\n", (uint8_t)lo8);
		else if (r8 == 0xe)
			debug("xor.b\t#%i,@(r0,gbr)\n", (uint8_t)lo8);
		else if (r8 == 0xf)
			debug("or.b\t#%i,@(r0,gbr)\n", (uint8_t)lo8);
		else
			debug("UNIMPLEMENTED hi4=0x%x,0x%x\n", hi4, r8);
		break;
	case 0xe:
		debug("mov\t#%i,r%i\n", (int8_t)lo8, r8);
		break;
	case 0xf:
		if (lo4 == 0x0)
			debug("fadd\t%sr%i,%sr%i\n",
			    cpu->cd.sh.fpscr & SH_FPSCR_PR? "d" : "f", r4,
			    cpu->cd.sh.fpscr & SH_FPSCR_PR? "d" : "f", r8);
		else if (lo4 == 0x1)
			debug("fsub\t%sr%i,%sr%i\n",
			    cpu->cd.sh.fpscr & SH_FPSCR_PR? "d" : "f", r4,
			    cpu->cd.sh.fpscr & SH_FPSCR_PR? "d" : "f", r8);
		else if (lo4 == 0x2)
			debug("fmul\t%sr%i,%sr%i\n",
			    cpu->cd.sh.fpscr & SH_FPSCR_PR? "d" : "f", r4,
			    cpu->cd.sh.fpscr & SH_FPSCR_PR? "d" : "f", r8);
		else if (lo4 == 0x3)
			debug("fdiv\t%sr%i,%sr%i\n",
			    cpu->cd.sh.fpscr & SH_FPSCR_PR? "d" : "f", r4,
			    cpu->cd.sh.fpscr & SH_FPSCR_PR? "d" : "f", r8);
		else if (lo4 == 0x4)
			debug("fcmp/eq\t%sr%i,%sr%i\n",
			    cpu->cd.sh.fpscr & SH_FPSCR_PR? "d" : "f", r4,
			    cpu->cd.sh.fpscr & SH_FPSCR_PR? "d" : "f", r8);
		else if (lo4 == 0x5)
			debug("fcmp/gt\t%sr%i,%sr%i\n",
			    cpu->cd.sh.fpscr & SH_FPSCR_PR? "d" : "f", r4,
			    cpu->cd.sh.fpscr & SH_FPSCR_PR? "d" : "f", r8);
		else if (lo4 == 0x6) {
			const char *n = "fr";
			if (cpu->cd.sh.fpscr & SH_FPSCR_SZ) {
				n = (r8 & 1)? "xd" : "dr";
				r8 &= ~1;
			}
			debug("fmov\t@(r0,r%i),%s%i\n", r4, n, r8);
		} else if (lo4 == 0x7) {
			const char *n = "fr";
			if (cpu->cd.sh.fpscr & SH_FPSCR_SZ) {
				n = (r4 & 1)? "xd" : "dr";
				r4 &= ~1;
			}
			debug("fmov\t%s%i,@(r0,r%i)\n", n, r4, r8);
		} else if (lo4 == 0x8) {
			const char *n = "fr";
			if (cpu->cd.sh.fpscr & SH_FPSCR_SZ) {
				n = (r8 & 1)? "xd" : "dr";
				r8 &= ~1;
			}
			debug("fmov\t@r%i,%s%i\n", r4, n, r8);
		} else if (lo4 == 0x9) {
			const char *n = "fr";
			if (cpu->cd.sh.fpscr & SH_FPSCR_SZ) {
				n = (r8 & 1)? "xd" : "dr";
				r8 &= ~1;
			}
			debug("fmov\t@r%i+,%s%i\n", r4, n, r8);
		} else if (lo4 == 0xa) {
			const char *n = "fr";
			if (cpu->cd.sh.fpscr & SH_FPSCR_SZ) {
				n = (r4 & 1)? "xd" : "dr";
				r4 &= ~1;
			}
			debug("fmov\t%s%i,@r%i\n", n, r4, r8);
		} else if (lo4 == 0xb) {
			const char *n = "fr";
			if (cpu->cd.sh.fpscr & SH_FPSCR_SZ) {
				n = (r4 & 1)? "xd" : "dr";
				r4 &= ~1;
			}
			debug("fmov\t%s%i,@-r%i\n", n, r4, r8);
		} else if (lo4 == 0xc) {
			const char *n1 = "fr", *n2 = "fr";
			if (cpu->cd.sh.fpscr & SH_FPSCR_SZ) {
				n1 = (r4 & 1)? "xd" : "dr";
				n2 = (r8 & 1)? "xd" : "dr";
				r4 &= ~1; r8 &= ~1;
			}
			debug("fmov\t%s%i,%s%i\n", n1, r4, n2, r8);
		} else if (lo8 == 0x0d)
			debug("fsts\tfpul,fr%i\n", r8);
		else if (lo8 == 0x1d)
			debug("flds\tfr%i,fpul\n", r8);
		else if (lo8 == 0x2d)
			debug("float\tfpul,%sr%i\n",
			    cpu->cd.sh.fpscr & SH_FPSCR_PR? "d" : "f", r8);
		else if (lo8 == 0x3d)
			debug("ftrc\t%sr%i,fpul\n",
			    cpu->cd.sh.fpscr & SH_FPSCR_PR? "d" : "f", r8);
		else if (lo8 == 0x4d)
			debug("fneg\t%sr%i\n",
			    cpu->cd.sh.fpscr & SH_FPSCR_PR? "d" : "f", r8);
		else if (lo8 == 0x5d)
			debug("fabs\t%sr%i\n",
			    cpu->cd.sh.fpscr & SH_FPSCR_PR? "d" : "f", r8);
		else if (lo8 == 0x6d)
			debug("fsqrt\t%sr%i\n",
			    cpu->cd.sh.fpscr & SH_FPSCR_PR? "d" : "f", r8);
		else if (lo8 == 0x7d)
			debug("fsrra\tfr%i\n", r8);
		else if (lo8 == 0x8d)
			debug("fldi0\tfr%i\n", r8);
		else if (lo8 == 0x9d)
			debug("fldi1\tfr%i\n", r8);
		else if (lo8 == 0xad)
			debug("fcnvsd\tfpul,dr%i\n", r8);
		else if (lo8 == 0xbd)
			debug("fcnvds\tdr%i,fpul\n", r8);
		else if (lo8 == 0xed)
			debug("fipr\tfv%i,fv%i\n", (r8 & 3) << 2, r8 & 0xc);
		else if ((iword & 0x01ff) == 0x00fd)
			debug("fsca\tfpul,dr%i\n", r8);
		else if (iword == 0xf3fd)
			debug("fschg\n");
		else if (iword == 0xfbfd)
			debug("frchg\n");
		else if ((iword & 0xf3ff) == 0xf1fd)
			debug("ftrv\txmtrx,fv%i\n", r8 & 0xc);
		else if (lo4 == 0xe)
			debug("fmac\tfr0,fr%i,fr%i\n", r4, r8);
		else
			debug("UNIMPLEMENTED hi4=0x%x,0x%x\n", hi4, lo8);
		break;
	default:debug("UNIMPLEMENTED hi4=0x%x\n", hi4);
	}

	return sizeof(iword);
}


#include "tmp_sh_tail.cc"

