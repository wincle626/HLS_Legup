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
 *  PowerPC/POWER CPU emulation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "cpu.h"
#include "devices.h"
#include "interrupt.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"
#include "of.h"
#include "opcodes_ppc.h"
#include "ppc_spr_strings.h"
#include "settings.h"
#include "symbol.h"

#include "thirdparty/ppc_bat.h"
#include "thirdparty/ppc_pte.h"
#include "thirdparty/ppc_spr.h"

#define	DYNTRANS_DUALMODE_32
#include "tmp_ppc_head.cc"


void ppc_pc_to_pointers(struct cpu *);
void ppc32_pc_to_pointers(struct cpu *);

void ppc_irq_interrupt_assert(struct interrupt *interrupt);
void ppc_irq_interrupt_deassert(struct interrupt *interrupt);


/*
 *  ppc_cpu_new():
 *
 *  Create a new PPC cpu object.
 *
 *  Returns 1 on success, 0 if there was no matching PPC processor with
 *  this cpu_type_name.
 */
int ppc_cpu_new(struct cpu *cpu, struct memory *mem, struct machine *machine,
	int cpu_id, char *cpu_type_name)
{
	int any_cache = 0;
	int i, found;
	struct ppc_cpu_type_def cpu_type_defs[] = PPC_CPU_TYPE_DEFS;

	/*  Scan the cpu_type_defs list for this cpu type:  */
	i = 0;
	found = -1;
	while (i >= 0 && cpu_type_defs[i].name != NULL) {
		if (strcasecmp(cpu_type_defs[i].name, cpu_type_name) == 0) {
			found = i;
			break;
		}
		i++;
	}
	if (found == -1)
		return 0;

	cpu->memory_rw = ppc_memory_rw;

	cpu->cd.ppc.cpu_type = cpu_type_defs[found];
	cpu->name            = strdup(cpu->cd.ppc.cpu_type.name);
	cpu->byte_order      = EMUL_BIG_ENDIAN;
	cpu->cd.ppc.mode     = MODE_PPC;	/*  TODO  */

	/*  Current operating mode:  */
	cpu->cd.ppc.bits = cpu->cd.ppc.cpu_type.bits;
	cpu->cd.ppc.spr[SPR_PVR] = cpu->cd.ppc.cpu_type.pvr;

	/*  cpu->cd.ppc.msr = PPC_MSR_IR | PPC_MSR_DR |
	    PPC_MSR_SF | PPC_MSR_FP;  */

	cpu->cd.ppc.spr[SPR_IBAT0U] = 0x00001ffc | BAT_Vs;
	cpu->cd.ppc.spr[SPR_IBAT0L] = 0x00000000 | BAT_PP_RW;
	cpu->cd.ppc.spr[SPR_IBAT1U] = 0xc0001ffc | BAT_Vs;
	cpu->cd.ppc.spr[SPR_IBAT1L] = 0x00000000 | BAT_PP_RW;
	cpu->cd.ppc.spr[SPR_IBAT3U] = 0xf0001ffc | BAT_Vs;
	cpu->cd.ppc.spr[SPR_IBAT3L] = 0xf0000000 | BAT_PP_RW;
	cpu->cd.ppc.spr[SPR_DBAT0U] = 0x00001ffc | BAT_Vs;
	cpu->cd.ppc.spr[SPR_DBAT0L] = 0x00000000 | BAT_PP_RW;
	cpu->cd.ppc.spr[SPR_DBAT1U] = 0xc0001ffc | BAT_Vs;
	cpu->cd.ppc.spr[SPR_DBAT1L] = 0x00000000 | BAT_PP_RW;
	cpu->cd.ppc.spr[SPR_DBAT2U] = 0xe0001ffc | BAT_Vs;
	cpu->cd.ppc.spr[SPR_DBAT2L] = 0xe0000000 | BAT_PP_RW;
	cpu->cd.ppc.spr[SPR_DBAT3U] = 0xf0001ffc | BAT_Vs;
	cpu->cd.ppc.spr[SPR_DBAT3L] = 0xf0000000 | BAT_PP_RW;

	cpu->is_32bit = (cpu->cd.ppc.bits == 32)? 1 : 0;

	if (cpu->is_32bit) {
		cpu->run_instr = ppc32_run_instr;
		cpu->update_translation_table = ppc32_update_translation_table;
		cpu->invalidate_translation_caches =
		    ppc32_invalidate_translation_caches;
		cpu->invalidate_code_translation =
		    ppc32_invalidate_code_translation;
	} else {
		cpu->run_instr = ppc_run_instr;
		cpu->update_translation_table = ppc_update_translation_table;
		cpu->invalidate_translation_caches =
		    ppc_invalidate_translation_caches;
		cpu->invalidate_code_translation =
		    ppc_invalidate_code_translation;
	}

	cpu->translate_v2p = ppc_translate_v2p;

	/*  Only show name and caches etc for CPU nr 0 (in SMP machines):  */
	if (cpu_id == 0) {
		debug("%s", cpu->cd.ppc.cpu_type.name);

		if (cpu->cd.ppc.cpu_type.icache_shift != 0)
			any_cache = 1;
		if (cpu->cd.ppc.cpu_type.dcache_shift != 0)
			any_cache = 1;
		if (cpu->cd.ppc.cpu_type.l2cache_shift != 0)
			any_cache = 1;

		if (any_cache) {
			debug(" (I+D = %i+%i KB",
			    (int)(1 << (cpu->cd.ppc.cpu_type.icache_shift-10)),
			    (int)(1 << (cpu->cd.ppc.cpu_type.dcache_shift-10)));
			if (cpu->cd.ppc.cpu_type.l2cache_shift != 0) {
				debug(", L2 = %i KB",
				    (int)(1 << (cpu->cd.ppc.cpu_type.
				    l2cache_shift-10)));
			}
			debug(")");
		}
	}

	cpu->cd.ppc.spr[SPR_PIR] = cpu_id;

	/*  Some default stack pointer value.  TODO: move this?  */
	cpu->cd.ppc.gpr[1] = machine->physical_ram_in_mb * 1048576 - 4096;

	/*
	 *  NOTE/TODO: Ugly hack for OpenFirmware emulation:
	 */
	if (cpu->machine->prom_emulation)
		cpu->cd.ppc.of_emul_addr = 0xfff00000;

	/*  Add all register names to the settings:  */
	CPU_SETTINGS_ADD_REGISTER64("pc", cpu->pc);
	CPU_SETTINGS_ADD_REGISTER64("msr", cpu->cd.ppc.msr);
	CPU_SETTINGS_ADD_REGISTER64("ctr", cpu->cd.ppc.spr[SPR_CTR]);
	CPU_SETTINGS_ADD_REGISTER64("xer", cpu->cd.ppc.spr[SPR_XER]);
	CPU_SETTINGS_ADD_REGISTER64("dec", cpu->cd.ppc.spr[SPR_DEC]);
	CPU_SETTINGS_ADD_REGISTER64("hdec", cpu->cd.ppc.spr[SPR_HDEC]);
	CPU_SETTINGS_ADD_REGISTER64("srr0", cpu->cd.ppc.spr[SPR_SRR0]);
	CPU_SETTINGS_ADD_REGISTER64("srr1", cpu->cd.ppc.spr[SPR_SRR1]);
	CPU_SETTINGS_ADD_REGISTER64("sdr1", cpu->cd.ppc.spr[SPR_SDR1]);
	CPU_SETTINGS_ADD_REGISTER64("ibat0u", cpu->cd.ppc.spr[SPR_IBAT0U]);
	CPU_SETTINGS_ADD_REGISTER64("ibat0l", cpu->cd.ppc.spr[SPR_IBAT0L]);
	CPU_SETTINGS_ADD_REGISTER64("ibat1u", cpu->cd.ppc.spr[SPR_IBAT1U]);
	CPU_SETTINGS_ADD_REGISTER64("ibat1l", cpu->cd.ppc.spr[SPR_IBAT1L]);
	CPU_SETTINGS_ADD_REGISTER64("ibat2u", cpu->cd.ppc.spr[SPR_IBAT2U]);
	CPU_SETTINGS_ADD_REGISTER64("ibat2l", cpu->cd.ppc.spr[SPR_IBAT2L]);
	CPU_SETTINGS_ADD_REGISTER64("ibat3u", cpu->cd.ppc.spr[SPR_IBAT3U]);
	CPU_SETTINGS_ADD_REGISTER64("ibat3l", cpu->cd.ppc.spr[SPR_IBAT3L]);
	CPU_SETTINGS_ADD_REGISTER64("dbat0u", cpu->cd.ppc.spr[SPR_DBAT0U]);
	CPU_SETTINGS_ADD_REGISTER64("dbat0l", cpu->cd.ppc.spr[SPR_DBAT0L]);
	CPU_SETTINGS_ADD_REGISTER64("dbat1u", cpu->cd.ppc.spr[SPR_DBAT1U]);
	CPU_SETTINGS_ADD_REGISTER64("dbat1l", cpu->cd.ppc.spr[SPR_DBAT1L]);
	CPU_SETTINGS_ADD_REGISTER64("dbat2u", cpu->cd.ppc.spr[SPR_DBAT2U]);
	CPU_SETTINGS_ADD_REGISTER64("dbat2l", cpu->cd.ppc.spr[SPR_DBAT2L]);
	CPU_SETTINGS_ADD_REGISTER64("dbat3u", cpu->cd.ppc.spr[SPR_DBAT3U]);
	CPU_SETTINGS_ADD_REGISTER64("dbat3l", cpu->cd.ppc.spr[SPR_DBAT3L]);
	CPU_SETTINGS_ADD_REGISTER64("lr", cpu->cd.ppc.spr[SPR_LR]);
	CPU_SETTINGS_ADD_REGISTER32("cr", cpu->cd.ppc.cr);
	CPU_SETTINGS_ADD_REGISTER32("fpscr", cpu->cd.ppc.fpscr);
	/*  Integer GPRs, floating point registers, and segment registers:  */
	for (i=0; i<PPC_NGPRS; i++) {
		char tmpstr[5];
		snprintf(tmpstr, sizeof(tmpstr), "r%i", i);
		CPU_SETTINGS_ADD_REGISTER64(tmpstr, cpu->cd.ppc.gpr[i]);
	}
	for (i=0; i<PPC_NFPRS; i++) {
		char tmpstr[5];
		snprintf(tmpstr, sizeof(tmpstr), "f%i", i);
		CPU_SETTINGS_ADD_REGISTER64(tmpstr, cpu->cd.ppc.fpr[i]);
	}
	for (i=0; i<16; i++) {
		char tmpstr[5];
		snprintf(tmpstr, sizeof(tmpstr), "sr%i", i);
		CPU_SETTINGS_ADD_REGISTER32(tmpstr, cpu->cd.ppc.sr[i]);
	}

	/*  Register the CPU as an interrupt handler:  */
	{
		struct interrupt templ;
		char name[150];
		snprintf(name, sizeof(name), "%s", cpu->path);
		memset(&templ, 0, sizeof(templ));
		templ.line = 0;
		templ.name = name;
		templ.extra = cpu;
		templ.interrupt_assert = ppc_irq_interrupt_assert;
		templ.interrupt_deassert = ppc_irq_interrupt_deassert;
		interrupt_handler_register(&templ);
	}

	return 1;
}


/*
 *  ppc_cpu_list_available_types():
 *
 *  Print a list of available PPC CPU types.
 */
void ppc_cpu_list_available_types(void)
{
	int i, j;
	struct ppc_cpu_type_def tdefs[] = PPC_CPU_TYPE_DEFS;

	i = 0;
	while (tdefs[i].name != NULL) {
		debug("%s", tdefs[i].name);
		for (j=10 - strlen(tdefs[i].name); j>0; j--)
			debug(" ");
		i++;
		if ((i % 6) == 0 || tdefs[i].name == NULL)
			debug("\n");
	}
}


/*
 *  ppc_cpu_dumpinfo():
 */
void ppc_cpu_dumpinfo(struct cpu *cpu)
{
	struct ppc_cpu_type_def *ct = &cpu->cd.ppc.cpu_type;

	debug(" (%i-bit ", cpu->cd.ppc.bits);

	switch (cpu->cd.ppc.mode) {
	case MODE_PPC:
		debug("PPC");
		break;
	case MODE_POWER:
		debug("POWER");
		break;
	default:
		debug("_INTERNAL ERROR_");
	}

	debug(", I+D = %i+%i KB",
	    (1 << ct->icache_shift) / 1024,
	    (1 << ct->dcache_shift) / 1024);

	if (ct->l2cache_shift) {
		int kb = (1 << ct->l2cache_shift) / 1024;
		debug(", L2 = %i %cB",
		    kb >= 1024? kb / 1024 : kb,
		    kb >= 1024? 'M' : 'K');
	}

	debug(")\n");
}


/*
 *  reg_access_msr():
 */
void reg_access_msr(struct cpu *cpu, uint64_t *valuep, int writeflag,
	int check_for_interrupts)
{
	uint64_t old = cpu->cd.ppc.msr;

	if (valuep == NULL) {
		fatal("reg_access_msr(): NULL\n");
		return;
	}

	if (writeflag) {
		cpu->cd.ppc.msr = *valuep;

		/*  Switching between temporary and real gpr 0..3?  */
		if ((old & PPC_MSR_TGPR) != (cpu->cd.ppc.msr & PPC_MSR_TGPR)) {
			int i;
			for (i=0; i<PPC_N_TGPRS; i++) {
				uint64_t t = cpu->cd.ppc.gpr[i];
				cpu->cd.ppc.gpr[i] = cpu->cd.ppc.tgpr[i];
				cpu->cd.ppc.tgpr[i] = t;
			}
		}

		if (cpu->cd.ppc.msr & PPC_MSR_IP) {
			fatal("\n[ Reboot hack for NetBSD/prep. TODO: "
			    "fix this. ]\n");
			cpu->running = 0;
		}
	}

	/*  TODO: Is the little-endian bit writable?  */

	cpu->cd.ppc.msr &= ~PPC_MSR_LE;
	if (cpu->byte_order != EMUL_BIG_ENDIAN)
		cpu->cd.ppc.msr |= PPC_MSR_LE;

	if (!writeflag)
		*valuep = cpu->cd.ppc.msr;

	if (check_for_interrupts && cpu->cd.ppc.msr & PPC_MSR_EE) {
		if (cpu->cd.ppc.dec_intr_pending &&
		    !(cpu->cd.ppc.cpu_type.flags & PPC_NO_DEC)) {
			ppc_exception(cpu, PPC_EXCEPTION_DEC);
			cpu->cd.ppc.dec_intr_pending = 0;
		} else if (cpu->cd.ppc.irq_asserted)
			ppc_exception(cpu, PPC_EXCEPTION_EI);
	}
}


/*
 *  ppc_exception():
 */
void ppc_exception(struct cpu *cpu, int exception_nr)
{
	/*  Save PC and MSR:  */
	cpu->cd.ppc.spr[SPR_SRR0] = cpu->pc;

	if (exception_nr >= 0x10 && exception_nr <= 0x13)
		cpu->cd.ppc.spr[SPR_SRR1] = (cpu->cd.ppc.msr & 0xffff)
		    | (cpu->cd.ppc.cr & 0xf0000000);
	else
		cpu->cd.ppc.spr[SPR_SRR1] = (cpu->cd.ppc.msr & 0x87c0ffff);

	if (!quiet_mode)
		fatal("[ PPC Exception 0x%x; pc=0x%"PRIx64" ]\n",
		    exception_nr, cpu->pc);

	/*  Disable External Interrupts, Recoverable Interrupt Mode,
	    and go to Supervisor mode  */
	cpu->cd.ppc.msr &= ~(PPC_MSR_EE | PPC_MSR_RI | PPC_MSR_PR);

	cpu->pc = exception_nr * 0x100;
	if (cpu->cd.ppc.msr & PPC_MSR_IP)
		cpu->pc += 0xfff00000ULL;

	if (cpu->is_32bit)
		ppc32_pc_to_pointers(cpu);
	else
		ppc_pc_to_pointers(cpu);
}


/*
 *  ppc_cpu_register_dump():
 *
 *  Dump cpu registers in a relatively readable format.
 *
 *  gprs: set to non-zero to dump GPRs and some special-purpose registers.
 *  coprocs: if bit i is set, then we should dump registers from coproc i.
 */
void ppc_cpu_register_dump(struct cpu *cpu, int gprs, int coprocs)
{
	char *symbol;
	uint64_t offset, tmp;
	int i, x = cpu->cpu_id;
	int bits32 = cpu->cd.ppc.bits == 32;

	if (gprs) {
		/*  Special registers (pc, ...) first:  */
		symbol = get_symbol_name(&cpu->machine->symbol_context,
		    cpu->pc, &offset);

		debug("cpu%i: pc  = 0x", x);
		if (bits32)
			debug("%08"PRIx32, (uint32_t)cpu->pc);
		else
			debug("%016"PRIx64, (uint64_t)cpu->pc);
		debug("  <%s>\n", symbol != NULL? symbol : " no symbol ");

		debug("cpu%i: lr  = 0x", x);
		if (bits32)
			debug("%08"PRIx32, (uint32_t)cpu->cd.ppc.spr[SPR_LR]);
		else
			debug("%016"PRIx64, (uint64_t)cpu->cd.ppc.spr[SPR_LR]);
		debug("  cr  = 0x%08"PRIx32, (uint32_t)cpu->cd.ppc.cr);

		if (bits32)
			debug("  ");
		else
			debug("\ncpu%i: ", x);
		debug("ctr = 0x", x);
		if (bits32)
			debug("%08"PRIx32, (uint32_t)cpu->cd.ppc.spr[SPR_CTR]);
		else
			debug("%016"PRIx64, (uint64_t)cpu->cd.ppc.spr[SPR_CTR]);

		debug("  xer = 0x", x);
		if (bits32)
			debug("%08"PRIx32, (uint32_t)cpu->cd.ppc.spr[SPR_XER]);
		else
			debug("%016"PRIx64, (uint64_t)cpu->cd.ppc.spr[SPR_XER]);

		debug("\n");

		if (bits32) {
			/*  32-bit:  */
			for (i=0; i<PPC_NGPRS; i++) {
				if ((i % 4) == 0)
					debug("cpu%i:", x);
				debug(" r%02i = 0x%08"PRIx32" ", i,
				    (uint32_t) cpu->cd.ppc.gpr[i]);
				if ((i % 4) == 3)
					debug("\n");
			}
		} else {
			/*  64-bit:  */
			for (i=0; i<PPC_NGPRS; i++) {
				int r = (i >> 1) + ((i & 1) << 4);
				if ((i % 2) == 0)
					debug("cpu%i:", x);
				debug(" r%02i = 0x%016"PRIx64" ", r,
				    (uint64_t) cpu->cd.ppc.gpr[r]);
				if ((i % 2) == 1)
					debug("\n");
			}
		}

		/*  Other special registers:  */
		if (bits32) {
			debug("cpu%i: srr0 = 0x%08"PRIx32
			    " srr1 = 0x%08"PRIx32"\n", x,
			    (uint32_t) cpu->cd.ppc.spr[SPR_SRR0],
			    (uint32_t) cpu->cd.ppc.spr[SPR_SRR1]);
		} else {
			debug("cpu%i: srr0 = 0x%016"PRIx64
			    "  srr1 = 0x%016"PRIx64"\n", x,
			    (uint64_t) cpu->cd.ppc.spr[SPR_SRR0],
			    (uint64_t) cpu->cd.ppc.spr[SPR_SRR1]);
		}

		debug("cpu%i: msr = ", x);
		reg_access_msr(cpu, &tmp, 0, 0);
		if (bits32)
			debug("0x%08"PRIx32, (uint32_t) tmp);
		else
			debug("0x%016"PRIx64, (uint64_t) tmp);

		debug("  tb  = 0x%08"PRIx32"%08"PRIx32"\n",
		    (uint32_t) cpu->cd.ppc.spr[SPR_TBU],
		    (uint32_t) cpu->cd.ppc.spr[SPR_TBL]);

		debug("cpu%i: dec = 0x%08"PRIx32,
		    x, (uint32_t) cpu->cd.ppc.spr[SPR_DEC]);
		if (!bits32)
			debug("  hdec = 0x%08"PRIx32"\n",
			    (uint32_t) cpu->cd.ppc.spr[SPR_HDEC]);

		debug("\n");
	}

	if (coprocs & 1) {
		debug("cpu%i: fpscr = 0x%08"PRIx32"\n",
		    x, (uint32_t) cpu->cd.ppc.fpscr);

		/*  TODO: show floating-point values :-)  */

		/*  TODO: 32-bit fprs on 32-bit PPC cpus?  */

		for (i=0; i<PPC_NFPRS; i++) {
			if ((i % 2) == 0)
				debug("cpu%i:", x);
			debug(" f%02i = 0x%016"PRIx64" ", i,
			    (uint64_t) cpu->cd.ppc.fpr[i]);
			if ((i % 2) == 1)
				debug("\n");
		}
	}

	if (coprocs & 2) {
		debug("cpu%i:  sdr1 = 0x%"PRIx64"\n", x,
		    (uint64_t) cpu->cd.ppc.spr[SPR_SDR1]);
		if (cpu->cd.ppc.cpu_type.flags & PPC_601)
			debug("cpu%i:  PPC601-style, TODO!\n");
		else {
			for (i=0; i<8; i++) {
				int spr = SPR_IBAT0U + i*2;
				uint32_t upper = cpu->cd.ppc.spr[spr];
				uint32_t lower = cpu->cd.ppc.spr[spr+1];
				uint32_t len = (((upper & BAT_BL) << 15)
				    | 0x1ffff) + 1;
				debug("cpu%i:  %sbat%i: u=0x%08"PRIx32
				    " l=0x%08"PRIx32" ",
				    x, i<4? "i" : "d", i&3, upper, lower);
				if (!(upper & BAT_V)) {
					debug(" (not valid)\n");
					continue;
				}
				if (len < 1048576)
					debug(" (%i KB, ", len >> 10);
				else
					debug(" (%i MB, ", len >> 20);
				if (upper & BAT_Vu)
					debug("user, ");
				if (upper & BAT_Vs)
					debug("supervisor, ");
				if (lower & (BAT_W | BAT_I | BAT_M | BAT_G))
					debug("%s%s%s%s, ",
					    lower & BAT_W? "W" : "",
					    lower & BAT_I? "I" : "",
					    lower & BAT_M? "M" : "",
					    lower & BAT_G? "G" : "");
				switch (lower & BAT_PP) {
				case BAT_PP_NONE: debug("NO access"); break;
				case BAT_PP_RO_S: debug("read-only, soft");
					          break;
				case BAT_PP_RO:   debug("read-only"); break;
				case BAT_PP_RW:   debug("read/write"); break;
				}
				debug(")\n");
			}
		}
	}

	if (coprocs & 4) {
		for (i=0; i<16; i++) {
			uint32_t s = cpu->cd.ppc.sr[i];

			debug("cpu%i:", x);
			debug("  sr%-2i = 0x%08"PRIx32, i, s);

			s &= (SR_TYPE | SR_SUKEY | SR_PRKEY | SR_NOEXEC);
			if (s != 0) {
				debug("  (");
				if (s & SR_TYPE) {
					debug("NON-memory type");
					s &= ~SR_TYPE;
					if (s != 0)
						debug(", ");
				}
				if (s & SR_SUKEY) {
					debug("supervisor-key");
					s &= ~SR_SUKEY;
					if (s != 0)
						debug(", ");
				}
				if (s & SR_PRKEY) {
					debug("user-key");
					s &= ~SR_PRKEY;
					if (s != 0)
						debug(", ");
				}
				if (s & SR_NOEXEC)
					debug("NOEXEC");
				debug(")");
			}
			debug("\n");
		}
	}
}


/*
 *  ppc_cpu_tlbdump():
 *
 *  Not currently used for PPC.
 */
void ppc_cpu_tlbdump(struct machine *m, int x, int rawflag)
{
}


/*
 *  ppc_irq_interrupt_assert():
 */
void ppc_irq_interrupt_assert(struct interrupt *interrupt)
{
	struct cpu *cpu = (struct cpu *) interrupt->extra;
	cpu->cd.ppc.irq_asserted = 1;
}


/*
 *  ppc_irq_interrupt_deassert():
 */
void ppc_irq_interrupt_deassert(struct interrupt *interrupt)
{
	struct cpu *cpu = (struct cpu *) interrupt->extra;
	cpu->cd.ppc.irq_asserted = 0;
}


/*
 *  ppc_cpu_disassemble_instr():
 *
 *  Convert an instruction word into human readable format, for instruction
 *  tracing.
 *
 *  If running is 1, cpu->pc should be the address of the instruction.
 *
 *  If running is 0, things that depend on the runtime environment (eg.
 *  register contents) will not be shown, and addr will be used instead of
 *  cpu->pc for relative addresses.
 */
int ppc_cpu_disassemble_instr(struct cpu *cpu, unsigned char *instr,
	int running, uint64_t dumpaddr)
{
	int hi6, xo, lev, rt, rs, ra, rb, imm, sh, me, rc, l_bit, oe_bit;
	int spr, aa_bit, lk_bit, bf, bh, bi, bo, mb, nb, bt, ba, bb, fpreg;
	int bfa, to, load, wlen, no_rb = 0;
	uint64_t offset, addr;
	uint32_t iword;
	const char *symbol, *mnem = "ERROR";
	int power = cpu->cd.ppc.mode == MODE_POWER;

	if (running)
		dumpaddr = cpu->pc;

	symbol = get_symbol_name(&cpu->machine->symbol_context,
	    dumpaddr, &offset);
	if (symbol != NULL && offset==0)
		debug("<%s>\n", symbol);

	if (cpu->machine->ncpus > 1 && running)
		debug("cpu%i: ", cpu->cpu_id);

	if (cpu->cd.ppc.bits == 32)
		debug("%08"PRIx32, (uint32_t) dumpaddr);
	else
		debug("%016"PRIx64, (uint64_t) dumpaddr);

	/*  NOTE: Fixed to big-endian.  */
	iword = (instr[0] << 24) + (instr[1] << 16) + (instr[2] << 8)
	    + instr[3];

	debug(": %08"PRIx32"\t", iword);

	/*
	 *  Decode the instruction:
	 */

	hi6 = iword >> 26;

	switch (hi6) {
	case 0x4:
		debug("ALTIVEC TODO");
		/*  vxor etc  */
		break;
	case PPC_HI6_MULLI:
	case PPC_HI6_SUBFIC:
		rt = (iword >> 21) & 31;
		ra = (iword >> 16) & 31;
		imm = (int16_t)(iword & 0xffff);
		switch (hi6) {
		case PPC_HI6_MULLI:
			mnem = power? "muli":"mulli";
			break;
		case PPC_HI6_SUBFIC:
			mnem = power? "sfi":"subfic";
			break;
		}
		debug("%s\tr%i,r%i,%i", mnem, rt, ra, imm);
		break;
	case PPC_HI6_CMPLI:
	case PPC_HI6_CMPI:
		bf = (iword >> 23) & 7;
		l_bit = (iword >> 21) & 1;
		ra = (iword >> 16) & 31;
		if (hi6 == PPC_HI6_CMPLI) {
			imm = iword & 0xffff;
			mnem = "cmpl";
		} else {
			imm = (int16_t)(iword & 0xffff);
			mnem = "cmp";
		}
		debug("%s%si\t", mnem, l_bit? "d" : "w");
		if (bf != 0)
			debug("cr%i,", bf);
		debug("r%i,%i", ra, imm);
		break;
	case PPC_HI6_ADDIC:
	case PPC_HI6_ADDIC_DOT:
		rt = (iword >> 21) & 31;
		ra = (iword >> 16) & 31;
		rc = hi6 == PPC_HI6_ADDIC_DOT;
		imm = (int16_t)(iword & 0xffff);
		mnem = power? "ai":"addic";
		if (imm < 0 && !power) {
			mnem = "subic";
			imm = -imm;
		}
		debug("%s%s\tr%i,r%i,%i", mnem, rc?".":"", rt, ra, imm);
		break;
	case PPC_HI6_ADDI:
		rt = (iword >> 21) & 31;
		ra = (iword >> 16) & 31;
		imm = (int16_t)(iword & 0xffff);
		if (ra == 0)
			debug("li\tr%i,%i", rt, imm);
		else {
			mnem = power? "cal":"addi";
			if (imm < 0 && !power) {
				mnem = "subi";
				imm = -imm;
			}
			debug("%s\tr%i,r%i,%i", mnem, rt, ra, imm);
		}
		break;
	case PPC_HI6_ADDIS:
		rt = (iword >> 21) & 31;
		ra = (iword >> 16) & 31;
		imm = (int16_t)(iword & 0xffff);
		if (ra == 0)
			debug("lis\tr%i,%i", rt, imm);
		else
			debug("%s\tr%i,r%i,%i",
			    power? "cau":"addis", rt, ra, imm);
		break;
	case PPC_HI6_BC:
		aa_bit = (iword & 2) >> 1;
		lk_bit = iword & 1;
		bo = (iword >> 21) & 31;
		bi = (iword >> 16) & 31;
		/*  Sign-extend addr:  */
		addr = (int64_t)(int16_t)(iword & 0xfffc);
		debug("bc");
		if (lk_bit)
			debug("l");
		if (aa_bit)
			debug("a");
		else
			addr += dumpaddr;
		debug("\t%i,%i,", bo, bi);
		if (cpu->cd.ppc.bits == 32)
			addr &= 0xffffffff;
		if (cpu->cd.ppc.bits == 32)
			debug("0x%"PRIx32, (uint32_t) addr);
		else
			debug("0x%"PRIx64, (uint64_t) addr);
		symbol = get_symbol_name(&cpu->machine->symbol_context,
		    addr, &offset);
		if (symbol != NULL)
			debug("\t<%s>", symbol);
		break;
	case PPC_HI6_SC:
		lev = (iword >> 5) & 0x7f;
		debug("sc");
		if (lev != 0) {
			debug("\t%i", lev);
			if (lev > 1)
				debug(" (WARNING! reserved value)");
		}
		break;
	case PPC_HI6_B:
		aa_bit = (iword & 2) >> 1;
		lk_bit = iword & 1;
		/*  Sign-extend addr:  */
		addr = (int64_t)(int32_t)((iword & 0x03fffffc) << 6);
		addr = (int64_t)addr >> 6;
		debug("b");
		if (lk_bit)
			debug("l");
		if (aa_bit)
			debug("a");
		else
			addr += dumpaddr;
		if (cpu->cd.ppc.bits == 32)
			addr &= 0xffffffff;
		if (cpu->cd.ppc.bits == 32)
			debug("\t0x%"PRIx32, (uint32_t) addr);
		else
			debug("\t0x%"PRIx64, (uint64_t) addr);
		symbol = get_symbol_name(&cpu->machine->symbol_context,
		    addr, &offset);
		if (symbol != NULL)
			debug("\t<%s>", symbol);
		break;
	case PPC_HI6_19:
		xo = (iword >> 1) & 1023;
		switch (xo) {
		case PPC_19_MCRF:
			bf = (iword >> 23) & 7;
			bfa = (iword >> 18) & 7;
			debug("mcrf\tcr%i,cr%i", bf, bfa);
			break;
		case PPC_19_RFI:
			debug("rfi");
			break;
		case PPC_19_RFID:
			debug("rfid");
			break;
		case PPC_19_RFSVC:
			debug("rfsvc%s", power?"":"\t(INVALID for PowerPC)");
			break;
		case PPC_19_BCLR:
		case PPC_19_BCCTR:
			bo = (iword >> 21) & 31;
			bi = (iword >> 16) & 31;
			bh = (iword >> 11) & 3;
			lk_bit = iword & 1;
			switch (xo) {
			case PPC_19_BCLR:
				mnem = power? "bcr" : "bclr"; break;
			case PPC_19_BCCTR:
				mnem = power? "bcc" : "bcctr"; break;
			}
			debug("%s%s%s\t%i,%i,%i", mnem, lk_bit? "l" : "",
			    bh? (bh==3? "+" : (bh==2? "-" : "?")) : "",
			    bo, bi, bh);
			break;
		case PPC_19_ISYNC:
			debug("%s", power? "ics" : "isync");
			break;
		case PPC_19_CRAND:
		case PPC_19_CRXOR:
		case PPC_19_CROR:
		case PPC_19_CRNAND:
		case PPC_19_CRNOR:
		case PPC_19_CRANDC:
		case PPC_19_CREQV:
		case PPC_19_CRORC:
			bt = (iword >> 21) & 31;
			ba = (iword >> 16) & 31;
			bb = (iword >> 11) & 31;
			switch (xo) {
			case PPC_19_CRAND:	mnem = "crand"; break;
			case PPC_19_CRXOR:	mnem = "crxor"; break;
			case PPC_19_CROR:	mnem = "cror"; break;
			case PPC_19_CRNAND:	mnem = "crnand"; break;
			case PPC_19_CRNOR:	mnem = "crnor"; break;
			case PPC_19_CRANDC:	mnem = "crandc"; break;
			case PPC_19_CREQV:	mnem = "creqv"; break;
			case PPC_19_CRORC:	mnem = "crorc"; break;
			}
			debug("%s\t%i,%i,%i", mnem, bt, ba, bb);
			break;
		default:
			debug("unimplemented hi6_19, xo = 0x%x", xo);
		}
		break;
	case PPC_HI6_RLWNM:
	case PPC_HI6_RLWIMI:
	case PPC_HI6_RLWINM:
		rs = (iword >> 21) & 31;
		ra = (iword >> 16) & 31;
		sh = (iword >> 11) & 31;	/*  actually rb for rlwnm  */
		mb = (iword >> 6) & 31;
		me = (iword >> 1) & 31;
		rc = iword & 1;
		switch (hi6) {
		case PPC_HI6_RLWNM:
			mnem = power? "rlnm" : "rlwnm"; break;
		case PPC_HI6_RLWIMI:
			mnem = power? "rlimi" : "rlwimi"; break;
		case PPC_HI6_RLWINM:
			mnem = power? "rlinm" : "rlwinm"; break;
		}
		debug("%s%s\tr%i,r%i,%s%i,%i,%i",
		    mnem, rc?".":"", ra, rs,
		    hi6 == PPC_HI6_RLWNM? "r" : "",
		    sh, mb, me);
		break;
	case PPC_HI6_ORI:
	case PPC_HI6_ORIS:
	case PPC_HI6_XORI:
	case PPC_HI6_XORIS:
	case PPC_HI6_ANDI_DOT:
	case PPC_HI6_ANDIS_DOT:
		rs = (iword >> 21) & 31;
		ra = (iword >> 16) & 31;
		imm = iword & 0xffff;
		switch (hi6) {
		case PPC_HI6_ORI:
			mnem = power? "oril":"ori";
			break;
		case PPC_HI6_ORIS:
			mnem = power? "oriu":"oris";
			break;
		case PPC_HI6_XORI:
			mnem = power? "xoril":"xori";
			break;
		case PPC_HI6_XORIS:
			mnem = power? "xoriu":"xoris";
			break;
		case PPC_HI6_ANDI_DOT:
			mnem = power? "andil.":"andi.";
			break;
		case PPC_HI6_ANDIS_DOT:
			mnem = power? "andiu.":"andis.";
			break;
		}
		if (hi6 == PPC_HI6_ORI && rs == 0 && ra == 0 && imm == 0)
			debug("nop");
		else
			debug("%s\tr%i,r%i,0x%04x", mnem, ra, rs, imm);
		break;
	case PPC_HI6_30:
		xo = (iword >> 2) & 7;
		switch (xo) {
		case PPC_30_RLDICL:
		case PPC_30_RLDICR:
		case PPC_30_RLDIMI:	/*  mb, not me  */
			mnem = NULL;
			switch (xo) {
			case PPC_30_RLDICL: mnem = "rldicl"; break;
			case PPC_30_RLDICR: mnem = "rldicr"; break;
			case PPC_30_RLDIMI: mnem = "rldimi"; break;
			}
			rs = (iword >> 21) & 31;
			ra = (iword >> 16) & 31;
			sh = ((iword >> 11) & 31) | ((iword & 2) << 4);
			me = ((iword >> 6) & 31) | (iword & 0x20);
			rc = iword & 1;
			debug("%s%s\tr%i,r%i,%i,%i",
			    mnem, rc?".":"", ra, rs, sh, me);
			break;
		default:
			debug("unimplemented hi6_30, xo = 0x%x", xo);
		}
		break;
	case PPC_HI6_31:
		xo = (iword >> 1) & 1023;
		switch (xo) {

		case PPC_31_CMP:
		case PPC_31_CMPL:
			bf = (iword >> 23) & 7;
			l_bit = (iword >> 21) & 1;
			ra = (iword >> 16) & 31;
			rb = (iword >> 11) & 31;
			if (xo == PPC_31_CMPL)
				mnem = "cmpl";
			else
				mnem = "cmp";
			debug("%s%s\t", mnem, l_bit? "d" : "w");
			if (bf != 0)
				debug("cr%i,", bf);
			debug("r%i,r%i", ra, rb);
			break;
		case PPC_31_MFCR:
			rt = (iword >> 21) & 31;
			debug("mfcr\tr%i", rt);
			break;
		case PPC_31_MFMSR:
			rt = (iword >> 21) & 31;
			debug("mfmsr\tr%i", rt);
			break;
		case PPC_31_MTCRF:
			rs = (iword >> 21) & 31;
			mb = (iword >> 12) & 255;  /*  actually fxm, not mb  */
			debug("mtcrf\t%i,r%i", mb, rs);
			break;
		case PPC_31_MTMSR:
			rs = (iword >> 21) & 31;
			l_bit = (iword >> 16) & 1;
			debug("mtmsr\tr%i", rs);
			if (l_bit)
				debug(",%i", l_bit);
			break;
		case PPC_31_TW:
		case PPC_31_TD:
			to = (iword >> 21) & 31;
			ra = (iword >> 16) & 31;
			rb = (iword >> 11) & 31;
			switch (xo) {
			case PPC_31_TW: mnem = power? "t" : "tw"; break;
			case PPC_31_TD: mnem = "td"; break;
			}
			debug("%s\t%i,r%i,r%i", mnem, to, ra, rb);
			break;
		case PPC_31_LWARX:
		case PPC_31_LDARX:
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
		case PPC_31_STWCX_DOT:
		case PPC_31_STDCX_DOT:
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
			/*  rs for stores, rt for loads, actually  */
			load = 0; wlen = 0; fpreg = 0;
			rs = (iword >> 21) & 31;
			ra = (iword >> 16) & 31;
			rb = (iword >> 11) & 31;
			switch (xo) {
			case PPC_31_LWARX: wlen=4;load=1; mnem = "lwarx"; break;
			case PPC_31_LDARX: wlen=8;load=1; mnem = "ldarx"; break;
			case PPC_31_LBZX:  wlen=1;load=1; mnem = "lbzx"; break;
			case PPC_31_LBZUX: wlen=1;load=1; mnem = "lbzux"; break;
			case PPC_31_LHAX:  wlen=2;load=1; mnem = "lhax"; break;
			case PPC_31_LHAUX: wlen=2;load=1; mnem = "lhaux"; break;
			case PPC_31_LHZX:  wlen=2;load=1; mnem = "lhzx"; break;
			case PPC_31_LHZUX: wlen=2;load=1; mnem = "lhzux"; break;
			case PPC_31_LWZX:  wlen = 4; load = 1;
				mnem = power? "lx" : "lwzx";
				break;
			case PPC_31_LWZUX: wlen = 4; load = 1;
				mnem = power? "lux":"lwzux";
				break;
			case PPC_31_LFDX: fpreg = 1; wlen = 8; load = 1;
				mnem = "lfdx"; break;
			case PPC_31_LFSX: fpreg = 1; wlen = 4; load = 1;
				mnem = "lfsx"; break;
			case PPC_31_STWCX_DOT: wlen=4; mnem = "stwcx."; break;
			case PPC_31_STDCX_DOT: wlen=8; mnem = "stdcx."; break;
			case PPC_31_STBX:  wlen=1; mnem = "stbx"; break;
			case PPC_31_STBUX: wlen=1; mnem = "stbux"; break;
			case PPC_31_STHX:  wlen=2; mnem = "sthx"; break;
			case PPC_31_STHUX: wlen=2; mnem = "sthux"; break;
			case PPC_31_STWX:
				wlen = 4; mnem = power? "stx" : "stwx";
				break;
			case PPC_31_STWUX:
				wlen = 4; mnem = power? "stux" : "stwux";
				break;
			case PPC_31_STDX:  wlen = 8; mnem = "stdx"; break;
			case PPC_31_STDUX: wlen = 8; mnem = "stdux"; break;
			case PPC_31_LHBRX:  wlen = 2; mnem = "lhbrx"; break;
			case PPC_31_LWBRX:  wlen = 4; mnem = power?
					    "lbrx" : "lwbrx"; break;
			case PPC_31_STHBRX: wlen = 2; mnem = "sthbrx"; break;
			case PPC_31_STWBRX: wlen = 4; mnem = power?
					    "stbrx" : "stwbrx"; break;
			case PPC_31_STFDX: fpreg = 1; wlen = 8;
				mnem = "stfdx"; break;
			case PPC_31_STFSX: fpreg = 1; wlen = 4;
				mnem = "stfsx"; break;
			}
			debug("%s\t%s%i,r%i,r%i", mnem,
			    fpreg? "f" : "r", rs, ra, rb);
			if (!running)
				break;
			addr = (ra==0? 0 : cpu->cd.ppc.gpr[ra]) +
			    cpu->cd.ppc.gpr[rb];
			if (cpu->cd.ppc.bits == 32)
				addr &= 0xffffffff;
			symbol = get_symbol_name(&cpu->machine->symbol_context,
			    addr, &offset);
			if (symbol != NULL)
				debug(" \t<%s", symbol);
			else
				debug(" \t<0x%"PRIx64, (uint64_t) addr);
			if (wlen > 0 && !fpreg /* && !reverse */) {
				/*  TODO  */
			}
			debug(">");
			break;
		case PPC_31_NEG:
		case PPC_31_NEGO:
			rt = (iword >> 21) & 31;
			ra = (iword >> 16) & 31;
			oe_bit = (iword >> 10) & 1;
			rc = iword & 1;
			switch (xo) {
			case PPC_31_NEG:  mnem = "neg"; break;
			case PPC_31_NEGO: mnem = "nego"; break;
			}
			debug("%s%s\tr%i,r%i", mnem, rc? "." : "", rt, ra);
			break;
		case PPC_31_WRTEEI:
			debug("wrteei\t%i", iword & 0x8000? 1 : 0);
			break;
		case PPC_31_MTMSRD:
			/*  TODO: Just a guess based on MTMSR  */
			rs = (iword >> 21) & 31;
			l_bit = (iword >> 16) & 1;
			debug("mtmsrd\tr%i", rs);
			if (l_bit)
				debug(",%i", l_bit);
			break;
		case PPC_31_ADDZE:
		case PPC_31_ADDZEO:
			rt = (iword >> 21) & 31;
			ra = (iword >> 16) & 31;
			oe_bit = (iword >> 10) & 1;
			rc = iword & 1;
			switch (xo) {
			case PPC_31_ADDZE:
				mnem = power? "aze" : "addze";
				break;
			case PPC_31_ADDZEO:
				mnem = power? "azeo" : "addzeo";
				break;
			}
			debug("%s%s\tr%i,r%i", mnem, rc? "." : "", rt, ra);
			break;
		case PPC_31_MTSR:
		case PPC_31_MFSR:
			/*  Move to/from segment register  */
			rt = (iword >> 21) & 31;
			ra = (iword >> 16) & 15;	/*  actually: sr  */
			switch (xo) {
			case PPC_31_MTSR:  mnem = "mtsr"; break;
			case PPC_31_MFSR:  mnem = "mfsr"; break;
			}
			debug("%s\tr%i,%i", mnem, rt, ra);
			break;
		case PPC_31_MTSRIN:
		case PPC_31_MFSRIN:
			/*  Move to/from segment register indirect  */
			rt = (iword >> 21) & 31;
			rb = (iword >> 11) & 31;
			switch (xo) {
			case PPC_31_MTSRIN:  mnem = "mtsrin"; break;
			case PPC_31_MFSRIN:  mnem = "mfsrin"; break;
			}
			debug("%s\tr%i,r%i", mnem, rt, rb);
			break;
		case PPC_31_ADDC:
		case PPC_31_ADDCO:
		case PPC_31_ADDE:
		case PPC_31_ADDEO:
		case PPC_31_ADDME:
		case PPC_31_ADDMEO:
		case PPC_31_ADD:
		case PPC_31_ADDO:
		case PPC_31_MULHW:
		case PPC_31_MULHWU:
		case PPC_31_MULLW:
		case PPC_31_MULLWO:
		case PPC_31_SUBF:
		case PPC_31_SUBFO:
		case PPC_31_SUBFC:
		case PPC_31_SUBFCO:
		case PPC_31_SUBFE:
		case PPC_31_SUBFEO:
		case PPC_31_SUBFME:
		case PPC_31_SUBFMEO:
		case PPC_31_SUBFZE:
		case PPC_31_SUBFZEO:
			rt = (iword >> 21) & 31;
			ra = (iword >> 16) & 31;
			rb = (iword >> 11) & 31;
			oe_bit = (iword >> 10) & 1;
			rc = iword & 1;
			switch (xo) {
			case PPC_31_ADDC:
				mnem = power? "a" : "addc";
				break;
			case PPC_31_ADDCO:
				mnem = power? "ao" : "addco";
				break;
			case PPC_31_ADDE:
				mnem = power? "ae" : "adde";
				break;
			case PPC_31_ADDEO:
				mnem = power? "aeo" : "addeo";
				break;
			case PPC_31_ADDME:
				mnem = power? "ame" : "addme";
				no_rb = 1;
				break;
			case PPC_31_ADDMEO:
				mnem = power? "ameo" : "addmeo";
				no_rb = 1;
				break;
			case PPC_31_ADD:
				mnem = power? "cax" : "add";
				break;
			case PPC_31_ADDO:
				mnem = power? "caxo" : "addo";
				break;
			case PPC_31_MULHW:  mnem = "mulhw"; break;
			case PPC_31_MULHWU: mnem = "mulhwu"; break;
			case PPC_31_MULLW:
				mnem = power? "muls" : "mullw";
				break;
			case PPC_31_MULLWO:
				mnem = power? "mulso" : "mullwo";
				break;
			case PPC_31_SUBF:   mnem = "subf"; break;
			case PPC_31_SUBFO:  mnem = "subfo"; break;
			case PPC_31_SUBFC:
				mnem = power? "sf" : "subfc"; break;
			case PPC_31_SUBFCO:
				mnem = power? "sfo" : "subfco"; break;
			case PPC_31_SUBFE:
				mnem = power? "sfe" : "subfe"; break;
			case PPC_31_SUBFEO:
				mnem = power? "sfeo" : "subfeo"; break;
			case PPC_31_SUBFME:
				mnem = power? "sfme" : "subfme"; break;
			case PPC_31_SUBFMEO:
				mnem = power? "sfmeo" : "subfmeo"; break;
			case PPC_31_SUBFZE:
				mnem = power? "sfze" : "subfze";
				no_rb = 1;
				break;
			case PPC_31_SUBFZEO:
				mnem = power? "sfzeo" : "subfzeo";
				no_rb = 1;
				break;
			}
			debug("%s%s\tr%i,r%i", mnem, rc? "." : "", rt, ra);
			if (!no_rb)
				debug(",r%i", rb);
			break;
		case PPC_31_MFSPR:
			rt = (iword >> 21) & 31;
			spr = ((iword >> 6) & 0x3e0) + ((iword >> 16) & 31);
			switch (spr) {
			/*  Some very common ones:  */
			case 8:	   debug("mflr\tr%i", rt); break;
			case 9:	   debug("mfctr\tr%i", rt); break;
			default:debug("mfspr\tr%i,spr%i", rt, spr);
			}
			if (spr == 8 || spr == 9)
				debug("\t");
			debug("\t<%s%s", running? "read from " : "",
			    ppc_spr_names[spr]==NULL? "?" : ppc_spr_names[spr]);
			if (running) {
				if (cpu->cd.ppc.bits == 32)
					debug(": 0x%"PRIx32, (uint32_t)
					    cpu->cd.ppc.spr[spr]);
				else
					debug(": 0x%"PRIx64, (uint64_t)
					    cpu->cd.ppc.spr[spr]);
			}
			debug(">");
			break;
		case PPC_31_TLBIA:
			debug("tlbia");
			break;
		case PPC_31_SLBIA:
			debug("slbia");
			break;
		case PPC_31_TLBLD:
		case PPC_31_TLBLI:
			rb = (iword >> 11) & 31;
			debug("tlbl%s\tr%i", xo == PPC_31_TLBLD? "d" : "i", rb);
			break;
		case PPC_31_TLBIE:
			/*  TODO: what is ra? The IBM online docs didn't say  */
			ra = 0;
			rb = (iword >> 11) & 31;
			if (power)
				debug("tlbi\tr%i,r%i", ra, rb);
			else
				debug("tlbie\tr%i", rb);
			break;
		case PPC_31_TLBSX_DOT:
			rs = (iword >> 21) & 31;
			ra = (iword >> 16) & 31;
			rb = (iword >> 11) & 31;
			debug("tlbsx.\tr%i,r%i,r%i", rs, ra, rb);
			break;
		case PPC_31_TLBSYNC:
			debug("tlbsync");
			break;
		case PPC_31_MFTB:
			rt = (iword >> 21) & 31;
			spr = ((iword >> 6) & 0x3e0) + ((iword >> 16) & 31);
			debug("mftb%s\tr%i", spr==268? "" :
			    (spr==269? "u" : "?"), rt);
			break;
		case PPC_31_CNTLZW:
			rs = (iword >> 21) & 31;
			ra = (iword >> 16) & 31;
			rc = iword & 1;
			mnem = power? "cntlz" : "cntlzw";
			debug("%s%s\tr%i,r%i", mnem, rc? "." : "", ra, rs);
			break;
		case PPC_31_CLF:	/*  POWER only  */
		case PPC_31_CLI:	/*  POWER only  */
		case PPC_31_DCLST:	/*  POWER only  */
		case PPC_31_DCBF:	/*  PowerPC only  */
		case PPC_31_DCBI:	/*  PowerPC only  */
		case PPC_31_DCBST:	/*  PowerPC only  */
		case PPC_31_DCBTST:	/*  PowerPC only  */
		case PPC_31_DCBT:	/*  PowerPC only  */
		case PPC_31_ICBI:	/*  PowerPC only  */
		case PPC_31_DCBZ:	/*  POWER/PowerPC  */
			ra = (iword >> 16) & 31;
			rb = (iword >> 11) & 31;
			switch (xo) {
			case PPC_31_CLF:   mnem = "clf"; break;
			case PPC_31_CLI:   mnem = "cli"; break;
			case PPC_31_DCLST: mnem = "dclst"; break;
			case PPC_31_DCBF:  mnem = "dcbf"; break;
			case PPC_31_DCBI:  mnem = "dcbi"; break;
			case PPC_31_DCBST: mnem = "dcbst"; break;
			case PPC_31_DCBTST:mnem = "dcbtst"; break;
			case PPC_31_DCBT:  mnem = "dcbt"; break;
			case PPC_31_ICBI:  mnem = "icbi"; break;
			case PPC_31_DCBZ:  mnem = power ?
					   "dclz" : "dcbz"; break;
			}
			debug("%s\tr%i,r%i", mnem, ra, rb);
			break;
		case PPC_31_SLW:
		case PPC_31_SLD:
		case PPC_31_SRAW:
		case PPC_31_SRW:
		case PPC_31_AND:
		case PPC_31_ANDC:
		case PPC_31_NOR:
		case PPC_31_EQV:
		case PPC_31_OR:
		case PPC_31_ORC:
		case PPC_31_XOR:
		case PPC_31_NAND:
			rs = (iword >> 21) & 31;
			ra = (iword >> 16) & 31;
			rb = (iword >> 11) & 31;
			rc = iword & 1;
			if (rs == rb && xo == PPC_31_OR)
				debug("mr%s\tr%i,r%i", rc? "." : "", ra, rs);
			else {
				switch (xo) {
				case PPC_31_SLW:  mnem =
					power? "sl" : "slw"; break;
				case PPC_31_SLD:  mnem = "sld"; break;
				case PPC_31_SRAW:  mnem =
					power? "sra" : "sraw"; break;
				case PPC_31_SRW:  mnem =
					power? "sr" : "srw"; break;
				case PPC_31_AND:  mnem = "and"; break;
				case PPC_31_NAND: mnem = "nand"; break;
				case PPC_31_ANDC: mnem = "andc"; break;
				case PPC_31_NOR:  mnem = "nor"; break;
				case PPC_31_EQV:  mnem = "eqv"; break;
				case PPC_31_OR:   mnem = "or"; break;
				case PPC_31_ORC:  mnem = "orc"; break;
				case PPC_31_XOR:  mnem = "xor"; break;
				}
				debug("%s%s\tr%i,r%i,r%i", mnem,
				    rc? "." : "", ra, rs, rb);
			}
			break;
		case PPC_31_DCCCI:
			ra = (iword >> 16) & 31;
			rb = (iword >> 11) & 31;
			debug("dccci\tr%i,r%i", ra, rb);
			break;
		case PPC_31_ICCCI:
			ra = (iword >> 16) & 31;
			rb = (iword >> 11) & 31;
			debug("iccci\tr%i,r%i", ra, rb);
			break;
		case PPC_31_DIVW:
		case PPC_31_DIVWO:
		case PPC_31_DIVWU:
		case PPC_31_DIVWUO:
			rt = (iword >> 21) & 31;
			ra = (iword >> 16) & 31;
			rb = (iword >> 11) & 31;
			oe_bit = (iword >> 10) & 1;
			rc = iword & 1;
			switch (xo) {
			case PPC_31_DIVWU:  mnem = "divwu"; break;
			case PPC_31_DIVWUO: mnem = "divwuo"; break;
			case PPC_31_DIVW:   mnem = "divw"; break;
			case PPC_31_DIVWO:  mnem = "divwo"; break;
			}
			debug("%s%s\tr%i,r%i,r%i", mnem, rc? "." : "",
			    rt, ra, rb);
			break;
		case PPC_31_MTSPR:
			rs = (iword >> 21) & 31;
			spr = ((iword >> 6) & 0x3e0) + ((iword >> 16) & 31);
			switch (spr) {
			/*  Some very common ones:  */
			case 8:	   debug("mtlr\tr%i", rs); break;
			case 9:	   debug("mtctr\tr%i", rs); break;
			default:debug("mtspr\tspr%i,r%i", spr, rs);
			}
			if (spr == 8 || spr == 9)
				debug("\t");
			debug("\t<%s%s", running? "write to " : "",
			    ppc_spr_names[spr]==NULL? "?" : ppc_spr_names[spr]);
			if (running) {
				if (cpu->cd.ppc.bits == 32)
					debug(": 0x%"PRIx32, (uint32_t)
					    cpu->cd.ppc.gpr[rs]);
				else
					debug(": 0x%"PRIx64, (uint64_t)
					    cpu->cd.ppc.gpr[rs]);
			}
			debug(">");
			break;
		case PPC_31_SYNC:
			debug("%s", power? "dcs" : "sync");
			break;
		case PPC_31_LSWI:
		case PPC_31_STSWI:
			rs = (iword >> 21) & 31;	/*  lwsi uses rt  */
			ra = (iword >> 16) & 31;
			nb = (iword >> 11) & 31;
			switch (xo) {
			case PPC_31_LSWI:
				mnem = power? "lsi" : "lswi"; break;
			case PPC_31_STSWI:
				mnem = power? "stsi" : "stswi"; break;
			}
			debug("%s\tr%i,r%i,%i", mnem, rs, ra, nb);
			break;
		case PPC_31_SRAWI:
			rs = (iword >> 21) & 31;
			ra = (iword >> 16) & 31;
			sh = (iword >> 11) & 31;
			rc = iword & 1;
			mnem = power? "srai" : "srawi";
			debug("%s%s\tr%i,r%i,%i", mnem,
			    rc? "." : "", ra, rs, sh);
			break;
		case PPC_31_DSSALL:
			debug("dssall");
			break;
		case PPC_31_EIEIO:
			debug("%s", power? "eieio?" : "eieio");
			break;
		case PPC_31_EXTSB:
		case PPC_31_EXTSH:
		case PPC_31_EXTSW:
			rs = (iword >> 21) & 31;
			ra = (iword >> 16) & 31;
			rc = iword & 1;
			switch (xo) {
			case PPC_31_EXTSB:
				mnem = power? "exts" : "extsb";
				break;
			case PPC_31_EXTSH:
				mnem = "extsh";
				break;
			case PPC_31_EXTSW:
				mnem = "extsw";
				break;
			}
			debug("%s%s\tr%i,r%i", mnem, rc? "." : "", ra, rs);
			break;
		case PPC_31_LVX:
		case PPC_31_LVXL:
		case PPC_31_STVX:
		case PPC_31_STVXL:
			rs = (iword >> 21) & 31;	/*  vs for stores,  */
			ra = (iword >> 16) & 31;	/*  rs=vl for loads  */
			rb = (iword >> 11) & 31;
			rc = iword & 1;
			switch (xo) {
			case PPC_31_LVX:   mnem = "lvx";  break;
			case PPC_31_LVXL:  mnem = "lvxl"; break;
			case PPC_31_STVX:  mnem = "stvx";  break;
			case PPC_31_STVXL: mnem = "stvxl"; break;
			}
			debug("%s%s\tv%i,r%i,r%i", mnem, rc? "." : "",
			    rs, ra, rb);
			break;
		default:
			debug("unimplemented hi6_31, xo = 0x%x", xo);
		}
		break;
	case PPC_HI6_LD:
	case PPC_HI6_LWZ:
	case PPC_HI6_LWZU:
	case PPC_HI6_LHZ:
	case PPC_HI6_LHZU:
	case PPC_HI6_LHA:
	case PPC_HI6_LHAU:
	case PPC_HI6_LBZ:
	case PPC_HI6_LBZU:
	case PPC_HI6_LFD:
	case PPC_HI6_LFS:
	case PPC_HI6_LMW:
	case PPC_HI6_STD:
	case PPC_HI6_STW:
	case PPC_HI6_STWU:
	case PPC_HI6_STH:
	case PPC_HI6_STHU:
	case PPC_HI6_STB:
	case PPC_HI6_STBU:
	case PPC_HI6_STMW:
	case PPC_HI6_STFD:
	case PPC_HI6_STFS:
		/*  NOTE: Loads use rt, not rs, but are otherwise similar
		    to stores  */
		load = 0; wlen = 0;
		rs = (iword >> 21) & 31;
		ra = (iword >> 16) & 31;
		imm = (int16_t)(iword & 0xffff);
		fpreg = 0;
		switch (hi6) {
		case PPC_HI6_LD:  load=1; wlen = 8; mnem = "ld"; break;
		case PPC_HI6_LWZ:  load=1; wlen = 4;
			mnem = power? "l" : "lwz"; break;
		case PPC_HI6_LWZU: load=1; wlen = 4;
			mnem = power? "lu" : "lwzu"; break;
		case PPC_HI6_LHZ:  load=1; wlen = 2;
			mnem = "lhz"; break;
		case PPC_HI6_LHZU: load=1; wlen = 2;
			mnem = "lhzu"; break;
		case PPC_HI6_LHA:  load=2; wlen = 2;
			mnem = "lha"; break;
		case PPC_HI6_LHAU: load=2; wlen = 2;
			mnem = "lhau"; break;
		case PPC_HI6_LBZ:  load=1; wlen = 1;
			mnem = "lbz"; break;
		case PPC_HI6_LBZU: load=1; wlen = 1;
			mnem = "lbzu"; break;
		case PPC_HI6_LFD:  load=1; fpreg=1; wlen=8; mnem = "lfd"; break;
		case PPC_HI6_LFS:  load=1; fpreg=1; wlen=4; mnem = "lfs"; break;
		case PPC_HI6_STD:  wlen=8; mnem = "std"; break;
		case PPC_HI6_STW:  wlen=4; mnem = power? "st" : "stw"; break;
		case PPC_HI6_STWU: wlen=4; mnem = power? "stu" : "stwu"; break;
		case PPC_HI6_STH:  wlen=2; mnem = "sth"; break;
		case PPC_HI6_STHU: wlen=2; mnem = "sthu"; break;
		case PPC_HI6_STB:  wlen=1; mnem = "stb"; break;
		case PPC_HI6_STBU: wlen=1; mnem = "stbu"; break;
		case PPC_HI6_LMW:  load=1; mnem = power? "lm" : "lmw"; break;
		case PPC_HI6_STMW: mnem = power? "stm" : "stmw"; break;
		case PPC_HI6_STFD: fpreg=1; wlen=8; mnem = "stfd"; break;
		case PPC_HI6_STFS: fpreg=1; wlen=4; mnem = "stfs"; break;
		}
		debug("%s\t", mnem);
		if (fpreg)
			debug("f");
		else
			debug("r");
		debug("%i,%i(r%i)", rs, imm, ra);
		if (!running)
			break;
		addr = (ra==0? 0 : cpu->cd.ppc.gpr[ra]) + imm;
		if (cpu->cd.ppc.bits == 32)
			addr &= 0xffffffff;
		symbol = get_symbol_name(&cpu->machine->symbol_context,
		    addr, &offset);
		if (symbol != NULL)
			debug(" \t<%s", symbol);
		else
			debug(" \t<0x%"PRIx64, (uint64_t) addr);
		if (wlen > 0 && load && wlen > 0) {
			unsigned char tw[8];
			uint64_t tdata = 0;
			int i, res = cpu->memory_rw(cpu, cpu->mem, addr, tw,
			    wlen, MEM_READ, NO_EXCEPTIONS);
			if (res) {
				if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
					for (i=0; i<wlen; i++) {
						tdata <<= 8;
						tdata |= tw[wlen-1-i];
					}
				else
					for (i=0; i<wlen; i++) {
						tdata <<= 8;
						tdata |= tw[i];
					}
				debug(": ");
				if (wlen >= 4) {
					symbol = get_symbol_name(&cpu->machine->
					    symbol_context, tdata, &offset);
					if (symbol != NULL)
						debug("%s", symbol);
					else
						debug("0x%"PRIx64,
						    (uint64_t) tdata);
				} else {
					/*  TODO: if load==2, then this is
					    a _signed_ load.  */
					debug("0x%"PRIx64, (uint64_t) tdata);
				}
			} else
				debug(": unreadable");
		}
		if (wlen > 0 && !load && wlen > 0) {
			int64_t tdata = 0;
			int i;
			for (i=0; i<wlen; i++)
				tdata |= (cpu->cd.ppc.gpr[rs] &
				    ((uint64_t)0xff << (i*8)));
			debug(": ");
			if (wlen >= 4) {
				symbol = get_symbol_name(&cpu->machine->
				    symbol_context, tdata, &offset);
				if (symbol != NULL)
					debug("%s", symbol);
				else
					debug("0x%"PRIx64, (uint64_t) tdata);
			} else {
				if (tdata > -256 && tdata < 256)
					debug("%i", (int)tdata);
				else
					debug("0x%"PRIx64, (uint64_t) tdata);
			}
		}
		debug(">");
		break;
	case PPC_HI6_59:
		xo = (iword >> 1) & 1023;
		/*  NOTE: Some floating point instructions only use the
		    lowest 5 bits of xo, some use all 10 bits!  */
		switch (xo & 31) {
		case PPC_59_FDIVS:
		case PPC_59_FSUBS:
		case PPC_59_FADDS:
		case PPC_59_FMULS:
		case PPC_59_FMADDS:
			rt = (iword >> 21) & 31;
			ra = (iword >> 16) & 31;
			rb = (iword >> 11) & 31;
			rs = (iword >>  6) & 31;	/*  actually frc  */
			rc = iword & 1;
			switch (xo & 31) {
			case PPC_59_FDIVS:	mnem = "fdivs"; break;
			case PPC_59_FSUBS:	mnem = "fsubs"; break;
			case PPC_59_FADDS:	mnem = "fadds"; break;
			case PPC_59_FMULS:	mnem = "fmuls"; break;
			case PPC_59_FMADDS:	mnem = "fmadds"; break;
			}
			debug("%s%s\t", mnem, rc? "." : "");
			switch (xo & 31) {
			case PPC_59_FMULS:
				debug("f%i,f%i,f%i", rt, ra, rs);
				break;
			case PPC_59_FMADDS:
				debug("f%i,f%i,f%i,f%i", rt, ra, rs, rb);
				break;
			default:debug("f%i,f%i,f%i", rt, ra, rb);
			}
			break;
		default:/*  TODO: similar to hi6_63  */
			debug("unimplemented hi6_59, xo = 0x%x", xo);
		}
		break;
	case PPC_HI6_63:
		xo = (iword >> 1) & 1023;
		/*  NOTE: Some floating point instructions only use the
		    lowest 5 bits of xo, some use all 10 bits!  */
		switch (xo & 31) {
		case PPC_63_FDIV:
		case PPC_63_FSUB:
		case PPC_63_FADD:
		case PPC_63_FMUL:
		case PPC_63_FMSUB:
		case PPC_63_FMADD:
			rt = (iword >> 21) & 31;
			ra = (iword >> 16) & 31;
			rb = (iword >> 11) & 31;
			rs = (iword >>  6) & 31;	/*  actually frc  */
			rc = iword & 1;
			switch (xo & 31) {
			case PPC_63_FDIV:
				mnem = power? "fd" : "fdiv"; break;
			case PPC_63_FSUB:
				mnem = power? "fs" : "fsub"; break;
			case PPC_63_FADD:
				mnem = power? "fa" : "fadd"; break;
			case PPC_63_FMUL:
				mnem = power? "fm" : "fmul"; break;
			case PPC_63_FMSUB:
				mnem = power? "fms" : "fmsub"; break;
			case PPC_63_FMADD:
				mnem = power? "fma" : "fmadd"; break;
			}
			debug("%s%s\t", mnem, rc? "." : "");
			switch (xo & 31) {
			case PPC_63_FMUL:
				debug("f%i,f%i,f%i", rt, ra, rs);
				break;
			case PPC_63_FMADD:
				debug("f%i,f%i,f%i,f%i", rt, ra, rs, rb);
				break;
			default:debug("f%i,f%i,f%i", rt, ra, rb);
			}
			break;
		default:rt = (iword >> 21) & 31;
			ra = (iword >> 16) & 31;
			rb = (iword >> 11) & 31;
			rc = iword & 1;
			switch (xo) {
			case PPC_63_FCMPU:
			case PPC_63_FRSP:
			case PPC_63_FCTIWZ:
			case PPC_63_FNEG:
			case PPC_63_FMR:
			case PPC_63_FNABS:
			case PPC_63_FABS:
				switch (xo) {
				case PPC_63_FCMPU:	mnem = "fcmpu"; break;
				case PPC_63_FCTIWZ:
					mnem = power? "fcirz" : "fctiwz"; break;
				case PPC_63_FRSP:	mnem = "frsp"; break;
				case PPC_63_FNEG:	mnem = "fneg"; break;
				case PPC_63_FMR:	mnem = "fmr"; break;
				case PPC_63_FNABS:	mnem = "fnabs"; break;
				case PPC_63_FABS:	mnem = "fabs"; break;
				}
				debug("%s%s\t", mnem, rc? "." : "");
				switch (xo) {
				case PPC_63_FCMPU:
					debug("%i,f%i,f%i", rt >> 2, ra, rb);
					break;
				case PPC_63_FCTIWZ:
				case PPC_63_FRSP:
				case PPC_63_FNEG:
				case PPC_63_FMR:
				case PPC_63_FNABS:
				case PPC_63_FABS:
					debug("f%i,f%i", rt, rb);
					break;
				default:debug("f%i,f%i,f%i", rt, ra, rb);
				}
				break;
			case PPC_63_MFFS:
				debug("mffs%s\tf%i", rc?".":"", rt);
				break;
			case PPC_63_MTFSF:
				ra = (iword >> 17) & 255;	/*  flm  */
				debug("mtfsf%s\t0x%02x,f%i", rc?".":"", ra, rb);
				break;
			default:debug("unimplemented hi6_63, xo = 0x%x", xo);
			}
		}
		break;
	default:
		/*  TODO  */
		debug("unimplemented hi6 = 0x%02x", hi6);
	}

	debug("\n");
	return sizeof(iword);
}


/*
 *  debug_spr_usage():
 *
 *  Helper function. To speed up overall development speed of the emulator,
 *  all SPR accesses are allowed. This function causes unknown/unimplemented
 *  SPRs to give a warning.
 */
static void debug_spr_usage(uint64_t pc, int spr)
{
	static uint32_t spr_used[1024 / sizeof(uint32_t)];
	static int initialized = 0;

	if (!initialized) {
		memset(spr_used, 0, sizeof(spr_used));
		initialized = 1;
	}

	spr &= 1023;
	if (spr_used[spr >> 2] & (1 << (spr & 3)))
		return;

	switch (spr) {
	/*  Known/implemented SPRs:  */
	case SPR_XER:
	case SPR_LR:
	case SPR_CTR:
	case SPR_DSISR:
	case SPR_DAR:
	case SPR_DEC:
	case SPR_SDR1:
	case SPR_SRR0:
	case SPR_SRR1:
	case SPR_SPRG0:
	case SPR_SPRG1:
	case SPR_SPRG2:
	case SPR_SPRG3:
	case SPR_PVR:
	case SPR_DMISS:
	case SPR_DCMP:
	case SPR_HASH1:
	case SPR_HASH2:
	case SPR_IMISS:
	case SPR_ICMP:
	case SPR_DBSR:
	case SPR_PIR:
		break;
	default:if (spr >= SPR_IBAT0U && spr <= SPR_DBAT3L) {
			break;
		} else
			fatal("[ using UNIMPLEMENTED spr %i (%s), pc = "
			    "0x%"PRIx64" ]\n", spr, ppc_spr_names[spr] == NULL?
			    "UNKNOWN" : ppc_spr_names[spr], (uint64_t) pc);
	}

	spr_used[spr >> 2] |= (1 << (spr & 3));
}


/*
 *  update_cr0():
 *
 *  Sets the top 4 bits of the CR register.
 */
void update_cr0(struct cpu *cpu, uint64_t value)
{
	int c;

	if (cpu->cd.ppc.bits == 64) {
		if ((int64_t)value < 0)
			c = 8;
		else if ((int64_t)value > 0)
			c = 4;
		else
			c = 2;
	} else {
		if ((int32_t)value < 0)
			c = 8;
		else if ((int32_t)value > 0)
			c = 4;
		else
			c = 2;
	}

	/*  SO bit, copied from XER:  */
	c |= ((cpu->cd.ppc.spr[SPR_XER] >> 31) & 1);

	cpu->cd.ppc.cr &= ~((uint32_t)0xf << 28);
	cpu->cd.ppc.cr |= ((uint32_t)c << 28);
}


#include "memory_ppc.cc"


#include "tmp_ppc_tail.cc"


