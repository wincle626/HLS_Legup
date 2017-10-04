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
 *  ARM CPU emulation.
 *
 *  A good source of quick info on ARM instruction encoding:
 *
 *	http://www.pinknoise.demon.co.uk/ARMinstrs/ARMinstrs.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "arm_cpu_types.h"
#include "cpu.h"
#include "interrupt.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"
#include "of.h"
#include "settings.h"
#include "symbol.h"

#define DYNTRANS_32
#include "tmp_arm_head.cc"


/*  ARM symbolic register names and condition strings:  */
static const char *arm_regname[N_ARM_REGS] = ARM_REG_NAMES;
static const char *arm_condition_string[16] = ARM_CONDITION_STRINGS;

/*  Data Processing Instructions:  */
static const char *arm_dpiname[16] = ARM_DPI_NAMES;
static int arm_dpi_uses_d[16] = { 1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1 };
static int arm_dpi_uses_n[16] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0 };

static int arm_exception_to_mode[N_ARM_EXCEPTIONS] = ARM_EXCEPTION_TO_MODE;

/*  For quick_pc_to_pointers():  */
void arm_pc_to_pointers(struct cpu *cpu);
#include "quick_pc_to_pointers.h"

void arm_irq_interrupt_assert(struct interrupt *interrupt);
void arm_irq_interrupt_deassert(struct interrupt *interrupt);


/*
 *  arm_cpu_new():
 *
 *  Create a new ARM cpu object by filling the CPU struct.
 *  Return 1 on success, 0 if cpu_type_name isn't a valid ARM processor.
 */
int arm_cpu_new(struct cpu *cpu, struct memory *mem,
	struct machine *machine, int cpu_id, char *cpu_type_name)
{
	int i, found;
	struct arm_cpu_type_def cpu_type_defs[] = ARM_CPU_TYPE_DEFS;

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

	cpu->run_instr = arm_run_instr;
	cpu->memory_rw = arm_memory_rw;
	cpu->update_translation_table = arm_update_translation_table;
	cpu->invalidate_translation_caches =
	    arm_invalidate_translation_caches;
	cpu->invalidate_code_translation = arm_invalidate_code_translation;
	cpu->translate_v2p = arm_translate_v2p;

	cpu->cd.arm.cpu_type = cpu_type_defs[found];
	cpu->name            = strdup(cpu->cd.arm.cpu_type.name);
	cpu->is_32bit        = 1;
	cpu->byte_order      = EMUL_LITTLE_ENDIAN;

	cpu->cd.arm.cpsr = ARM_FLAG_I | ARM_FLAG_F;
	cpu->cd.arm.control = ARM_CONTROL_PROG32 | ARM_CONTROL_DATA32
	    | ARM_CONTROL_CACHE | ARM_CONTROL_ICACHE | ARM_CONTROL_ALIGN;
	/*  TODO: default auxctrl contents  */

	if (cpu->machine->prom_emulation) {
		cpu->cd.arm.cpsr |= ARM_MODE_SVC32;
		cpu->cd.arm.control |= ARM_CONTROL_S;
	} else {
		cpu->cd.arm.cpsr |= ARM_MODE_SVC32;
		cpu->cd.arm.control |= ARM_CONTROL_R;
	}

	/*  Only show name and caches etc for CPU nr 0:  */
	if (cpu_id == 0) {
		debug("%s", cpu->name);
		if (cpu->cd.arm.cpu_type.icache_shift != 0 ||
		    cpu->cd.arm.cpu_type.dcache_shift != 0) {
			int isize = cpu->cd.arm.cpu_type.icache_shift;
			int dsize = cpu->cd.arm.cpu_type.dcache_shift;
			if (isize != 0)
				isize = 1 << (isize - 10);
			if (dsize != 0)
				dsize = 1 << (dsize - 10);
			debug(" (I+D = %i+%i KB)", isize, dsize);
		}
	}

	/*  TODO: Some of these values (iway and dway) aren't used yet:  */
	cpu->cd.arm.cachetype =
	      (5 << ARM_CACHETYPE_CLASS_SHIFT)
	    | (1 << ARM_CACHETYPE_HARVARD_SHIFT)
	    | ((cpu->cd.arm.cpu_type.dcache_shift - 9) <<
		ARM_CACHETYPE_DSIZE_SHIFT)
	    | (5 << ARM_CACHETYPE_DASSOC_SHIFT)		/*  32-way  */
	    | (2 << ARM_CACHETYPE_DLINE_SHIFT)		/*  8 words/line  */
	    | ((cpu->cd.arm.cpu_type.icache_shift - 9) <<
		ARM_CACHETYPE_ISIZE_SHIFT)
	    | (5 << ARM_CACHETYPE_IASSOC_SHIFT)		/*  32-way  */
	    | (2 << ARM_CACHETYPE_ILINE_SHIFT);		/*  8 words/line  */

	/*  Coprocessor 15 = the system control coprocessor.  */
	cpu->cd.arm.coproc[15] = arm_coproc_15;

	/*  Coprocessor 14 for XScale:  */
	if (cpu->cd.arm.cpu_type.flags & ARM_XSCALE)
		cpu->cd.arm.coproc[14] = arm_coproc_xscale_14;

	/*
	 *  NOTE/TODO: Ugly hack for OpenFirmware emulation:
	 */
	if (cpu->machine->prom_emulation) {
		cpu->cd.arm.of_emul_addr = cpu->machine->physical_ram_in_mb
		    * 1048576 - 8;
		store_32bit_word(cpu, cpu->cd.arm.of_emul_addr, 0xef8c64be);
	}

	cpu->cd.arm.flags = cpu->cd.arm.cpsr >> 28;

	CPU_SETTINGS_ADD_REGISTER64("pc", cpu->pc);
	for (i=0; i<N_ARM_REGS - 1; i++)
		CPU_SETTINGS_ADD_REGISTER32(arm_regname[i], cpu->cd.arm.r[i]);

	/*  Register the CPU's "IRQ" and "FIQ" interrupts:  */
	{
		struct interrupt templ;
		char name[50];
		snprintf(name, sizeof(name), "%s.irq", cpu->path);

                memset(&templ, 0, sizeof(templ));
                templ.line = 0;
                templ.name = name;
                templ.extra = cpu;
                templ.interrupt_assert = arm_irq_interrupt_assert;
                templ.interrupt_deassert = arm_irq_interrupt_deassert;
                interrupt_handler_register(&templ);

		/*  FIQ: TODO  */
        }

	return 1;
}


/*
 *  arm_setup_initial_translation_table():
 *
 *  When booting kernels (such as OpenBSD or NetBSD) directly, it is assumed
 *  that the MMU is already enabled by the boot-loader. This function tries
 *  to emulate that.
 */
void arm_setup_initial_translation_table(struct cpu *cpu, uint32_t ttb_addr)
{
	unsigned char nothing[16384];
	unsigned int i, j;

	cpu->cd.arm.control |= ARM_CONTROL_MMU;
	cpu->translate_v2p = arm_translate_v2p_mmu;
	cpu->cd.arm.dacr |= 0x00000003;
	cpu->cd.arm.ttb = ttb_addr;

	memset(nothing, 0, sizeof(nothing));
	cpu->memory_rw(cpu, cpu->mem, cpu->cd.arm.ttb, nothing,
	    sizeof(nothing), MEM_WRITE, PHYSICAL | NO_EXCEPTIONS);
	for (i=0; i<256; i++)
		for (j=0x0; j<=0xf; j++) {
			unsigned char descr[4];
			uint32_t addr = cpu->cd.arm.ttb +
			    (((j << 28) + (i << 20)) >> 18);
			uint32_t d = (1048576*i) | 0xc02;

			if (cpu->byte_order == EMUL_LITTLE_ENDIAN) {
				descr[0] = d;       descr[1] = d >> 8;
				descr[2] = d >> 16; descr[3] = d >> 24;
			} else {
				descr[3] = d;       descr[2] = d >> 8;
				descr[1] = d >> 16; descr[0] = d >> 24;
			}
			cpu->memory_rw(cpu, cpu->mem, addr, &descr[0],
			    sizeof(descr), MEM_WRITE, PHYSICAL | NO_EXCEPTIONS);
		}
}


/*
 *  arm_translation_table_set_l1():
 */
void arm_translation_table_set_l1(struct cpu *cpu, uint32_t vaddr,
	uint32_t paddr)
{
	unsigned int i, j, vhigh = vaddr >> 28, phigh = paddr >> 28;

	for (i=0; i<256; i++)
		for (j=vhigh; j<=vhigh; j++) {
			unsigned char descr[4];
			uint32_t addr = cpu->cd.arm.ttb +
			    (((j << 28) + (i << 20)) >> 18);
			uint32_t d = ((phigh << 28) + 1048576*i) | 0xc02;

			if (cpu->byte_order == EMUL_LITTLE_ENDIAN) {
				descr[0] = d;       descr[1] = d >> 8;
				descr[2] = d >> 16; descr[3] = d >> 24;
			} else {
				descr[3] = d;       descr[2] = d >> 8;
				descr[1] = d >> 16; descr[0] = d >> 24;
			}
			cpu->memory_rw(cpu, cpu->mem, addr, &descr[0],
			    sizeof(descr), MEM_WRITE, PHYSICAL | NO_EXCEPTIONS);
		}
}


/*
 *  arm_translation_table_set_l1_b():
 */
void arm_translation_table_set_l1_b(struct cpu *cpu, uint32_t vaddr,
	uint32_t paddr)
{
	unsigned int i, j, vhigh = vaddr >> 24, phigh = paddr >> 24;

	for (i=0; i<16; i++)
		for (j=vhigh; j<=vhigh; j++) {
			unsigned char descr[4];
			uint32_t addr = cpu->cd.arm.ttb +
			    (((j << 24) + (i << 20)) >> 18);
			uint32_t d = ((phigh << 24) + 1048576*i) | 0xc02;

			if (cpu->byte_order == EMUL_LITTLE_ENDIAN) {
				descr[0] = d;       descr[1] = d >> 8;
				descr[2] = d >> 16; descr[3] = d >> 24;
			} else {
				descr[3] = d;       descr[2] = d >> 8;
				descr[1] = d >> 16; descr[0] = d >> 24;
			}
			cpu->memory_rw(cpu, cpu->mem, addr, &descr[0],
			    sizeof(descr), MEM_WRITE, PHYSICAL | NO_EXCEPTIONS);
		}
}


/*
 *  arm_cpu_dumpinfo():
 */
void arm_cpu_dumpinfo(struct cpu *cpu)
{
	struct arm_cpu_type_def *ct = &cpu->cd.arm.cpu_type;

	debug(" (I+D = %i+%i KB)\n",
	    (1 << ct->icache_shift) / 1024, (1 << ct->dcache_shift) / 1024);
}


/*
 *  arm_cpu_list_available_types():
 *
 *  Print a list of available ARM CPU types.
 */
void arm_cpu_list_available_types(void)
{
	int i, j;
	struct arm_cpu_type_def tdefs[] = ARM_CPU_TYPE_DEFS;

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
 *  arm_cpu_register_dump():
 *
 *  Dump cpu registers in a relatively readable format.
 *  
 *  gprs: set to non-zero to dump GPRs and some special-purpose registers.
 *  coprocs: set bit 0..3 to dump registers in coproc 0..3.
 */
void arm_cpu_register_dump(struct cpu *cpu, int gprs, int coprocs)
{
	char *symbol;
	uint64_t offset;
	int mode = cpu->cd.arm.cpsr & ARM_FLAG_MODE;
	int i, x = cpu->cpu_id;

	cpu->cd.arm.cpsr &= 0x0fffffff;
	cpu->cd.arm.cpsr |= (cpu->cd.arm.flags << 28);

	if (gprs) {
		symbol = get_symbol_name(&cpu->machine->symbol_context,
		    cpu->pc, &offset);
		debug("cpu%i:  cpsr = ", x);
		debug("%s%s%s%s%s%s",
		    (cpu->cd.arm.cpsr & ARM_FLAG_N)? "N" : "n",
		    (cpu->cd.arm.cpsr & ARM_FLAG_Z)? "Z" : "z",
		    (cpu->cd.arm.cpsr & ARM_FLAG_C)? "C" : "c",
		    (cpu->cd.arm.cpsr & ARM_FLAG_V)? "V" : "v",
		    (cpu->cd.arm.cpsr & ARM_FLAG_I)? "I" : "i",
		    (cpu->cd.arm.cpsr & ARM_FLAG_F)? "F" : "f");
		if (mode < ARM_MODE_USR32)
			debug("   pc =  0x%07x", (int)(cpu->pc & 0x03ffffff));
		else
			debug("   pc = 0x%08x", (int)cpu->pc);

		debug("  <%s>\n", symbol != NULL? symbol : " no symbol ");

		for (i=0; i<N_ARM_REGS; i++) {
			if ((i % 4) == 0)
				debug("cpu%i:", x);
			if (i != ARM_PC)
				debug("  %s = 0x%08x", arm_regname[i],
				    (int)cpu->cd.arm.r[i]);
			if ((i % 4) == 3)
				debug("\n");
		}
	}

	if (coprocs & 1) {
		int m = cpu->cd.arm.cpsr & ARM_FLAG_MODE;
		debug("cpu%i:  cpsr = 0x%08x (", x, cpu->cd.arm.cpsr);
		switch (m) {
		case ARM_MODE_USR32:
			debug("USR32)\n"); break;
		case ARM_MODE_SYS32:
			debug("SYS32)\n"); break;
		case ARM_MODE_FIQ32:
			debug("FIQ32)\n"); break;
		case ARM_MODE_IRQ32:
			debug("IRQ32)\n"); break;
		case ARM_MODE_SVC32:
			debug("SVC32)\n"); break;
		case ARM_MODE_ABT32:
			debug("ABT32)\n"); break;
		case ARM_MODE_UND32:
			debug("UND32)\n"); break;
		default:debug("unimplemented)\n");
		}

		if (m != ARM_MODE_USR32 && m != ARM_MODE_SYS32) {
			debug("cpu%i:  usr r8-14:", x);
			for (i=0; i<7; i++)
				debug(" %08x", cpu->cd.arm.default_r8_r14[i]);
			debug("\n");
		}

		if (m != ARM_MODE_FIQ32) {
			debug("cpu%i:  fiq r8-14:", x);
			for (i=0; i<7; i++)
				debug(" %08x", cpu->cd.arm.fiq_r8_r14[i]);
			debug("\n");
		}

		if (m != ARM_MODE_IRQ32) {
			debug("cpu%i:  irq r13-14:", x);
			for (i=0; i<2; i++)
				debug(" %08x", cpu->cd.arm.irq_r13_r14[i]);
			debug("\n");
		}

		if (m != ARM_MODE_SVC32) {
			debug("cpu%i:  svc r13-14:", x);
			for (i=0; i<2; i++)
				debug(" %08x", cpu->cd.arm.svc_r13_r14[i]);
			debug("\n");
		}

		if (m != ARM_MODE_ABT32) {
			debug("cpu%i:  abt r13-14:", x);
			for (i=0; i<2; i++)
				debug(" %08x", cpu->cd.arm.abt_r13_r14[i]);
			debug("\n");
		}

		if (m != ARM_MODE_UND32) {
			debug("cpu%i:  und r13-14:", x);
			for (i=0; i<2; i++)
				debug(" %08x", cpu->cd.arm.und_r13_r14[i]);
			debug("\n");
		}
	}

	if (coprocs & 2) {
		debug("cpu%i:  control = 0x%08x\n", x, cpu->cd.arm.control);
		debug("cpu%i:      MMU:               %s\n", x,
		    cpu->cd.arm.control &
		    ARM_CONTROL_MMU? "enabled" : "disabled");
		debug("cpu%i:      alignment checks:  %s\n", x,
		    cpu->cd.arm.control &
		    ARM_CONTROL_ALIGN? "enabled" : "disabled");
		debug("cpu%i:      [data] cache:      %s\n", x,
		    cpu->cd.arm.control &
		    ARM_CONTROL_CACHE? "enabled" : "disabled");
		debug("cpu%i:      instruction cache: %s\n", x,
		    cpu->cd.arm.control &
		    ARM_CONTROL_ICACHE? "enabled" : "disabled");
		debug("cpu%i:      write buffer:      %s\n", x,
		    cpu->cd.arm.control &
		    ARM_CONTROL_WBUFFER? "enabled" : "disabled");
		debug("cpu%i:      prog32:            %s\n", x,
		    cpu->cd.arm.control &
		    ARM_CONTROL_PROG32? "yes" : "no (using prog26)");
		debug("cpu%i:      data32:            %s\n", x,
		    cpu->cd.arm.control &
		    ARM_CONTROL_DATA32? "yes" : "no (using data26)");
		debug("cpu%i:      endianness:        %s\n", x,
		    cpu->cd.arm.control &
		    ARM_CONTROL_BIG? "big endian" : "little endian");
		debug("cpu%i:      high vectors:      %s\n", x,
		    cpu->cd.arm.control &
		    ARM_CONTROL_V? "yes (0xffff0000)" : "no");

		/*  TODO: auxctrl on which CPU types?  */
		if (cpu->cd.arm.cpu_type.flags & ARM_XSCALE) {
			debug("cpu%i:  auxctrl = 0x%08x\n", x,
			    cpu->cd.arm.auxctrl);
			debug("cpu%i:      minidata cache attr = 0x%x\n", x,
			    (cpu->cd.arm.auxctrl & ARM_AUXCTRL_MD)
			    >> ARM_AUXCTRL_MD_SHIFT);
			debug("cpu%i:      page table memory attr: %i\n", x,
			    (cpu->cd.arm.auxctrl & ARM_AUXCTRL_P)? 1 : 0);
			debug("cpu%i:      write buffer coalescing: %s\n", x,
			    (cpu->cd.arm.auxctrl & ARM_AUXCTRL_K)?
			    "disabled" : "enabled");
		}

		debug("cpu%i:  ttb = 0x%08x  dacr = 0x%08x\n", x,
		    cpu->cd.arm.ttb, cpu->cd.arm.dacr);
		debug("cpu%i:  fsr = 0x%08x  far = 0x%08x\n", x,
		    cpu->cd.arm.fsr, cpu->cd.arm.far);
	}
}


/*
 *  arm_save_register_bank():
 */
void arm_save_register_bank(struct cpu *cpu)
{
	/*  Save away current registers:  */
	switch (cpu->cd.arm.cpsr & ARM_FLAG_MODE) {
	case ARM_MODE_USR32:
	case ARM_MODE_SYS32:
		memcpy(cpu->cd.arm.default_r8_r14,
		    &cpu->cd.arm.r[8], sizeof(uint32_t) * 7);
		break;
	case ARM_MODE_FIQ32:
		memcpy(cpu->cd.arm.fiq_r8_r14,
		    &cpu->cd.arm.r[8], sizeof(uint32_t) * 7);
		break;
	case ARM_MODE_IRQ32:
		memcpy(cpu->cd.arm.default_r8_r14,
		    &cpu->cd.arm.r[8], sizeof(uint32_t) * 5);
		cpu->cd.arm.irq_r13_r14[0] = cpu->cd.arm.r[13];
		cpu->cd.arm.irq_r13_r14[1] = cpu->cd.arm.r[14];
		break;
	case ARM_MODE_SVC32:
		memcpy(cpu->cd.arm.default_r8_r14,
		    &cpu->cd.arm.r[8], sizeof(uint32_t) * 5);
		cpu->cd.arm.svc_r13_r14[0] = cpu->cd.arm.r[13];
		cpu->cd.arm.svc_r13_r14[1] = cpu->cd.arm.r[14];
		break;
	case ARM_MODE_ABT32:
		memcpy(cpu->cd.arm.default_r8_r14,
		    &cpu->cd.arm.r[8], sizeof(uint32_t) * 5);
		cpu->cd.arm.abt_r13_r14[0] = cpu->cd.arm.r[13];
		cpu->cd.arm.abt_r13_r14[1] = cpu->cd.arm.r[14];
		break;
	case ARM_MODE_UND32:
		memcpy(cpu->cd.arm.default_r8_r14,
		    &cpu->cd.arm.r[8], sizeof(uint32_t) * 5);
		cpu->cd.arm.und_r13_r14[0] = cpu->cd.arm.r[13];
		cpu->cd.arm.und_r13_r14[1] = cpu->cd.arm.r[14];
		break;
	default:fatal("arm_save_register_bank: unimplemented mode %i\n",
		    cpu->cd.arm.cpsr & ARM_FLAG_MODE);
		exit(1);
	}
}


/*
 *  arm_load_register_bank():
 */
void arm_load_register_bank(struct cpu *cpu)
{
	/*  Load new registers:  */
	switch (cpu->cd.arm.cpsr & ARM_FLAG_MODE) {
	case ARM_MODE_USR32:
	case ARM_MODE_SYS32:
		memcpy(&cpu->cd.arm.r[8],
		    cpu->cd.arm.default_r8_r14, sizeof(uint32_t) * 7);
		break;
	case ARM_MODE_FIQ32:
		memcpy(&cpu->cd.arm.r[8], cpu->cd.arm.fiq_r8_r14,
		    sizeof(uint32_t) * 7);
		break;
	case ARM_MODE_IRQ32:
		memcpy(&cpu->cd.arm.r[8],
		    cpu->cd.arm.default_r8_r14, sizeof(uint32_t) * 5);
		cpu->cd.arm.r[13] = cpu->cd.arm.irq_r13_r14[0];
		cpu->cd.arm.r[14] = cpu->cd.arm.irq_r13_r14[1];
		break;
	case ARM_MODE_SVC32:
		memcpy(&cpu->cd.arm.r[8],
		    cpu->cd.arm.default_r8_r14, sizeof(uint32_t) * 5);
		cpu->cd.arm.r[13] = cpu->cd.arm.svc_r13_r14[0];
		cpu->cd.arm.r[14] = cpu->cd.arm.svc_r13_r14[1];
		break;
	case ARM_MODE_ABT32:
		memcpy(&cpu->cd.arm.r[8],
		    cpu->cd.arm.default_r8_r14, sizeof(uint32_t) * 5);
		cpu->cd.arm.r[13] = cpu->cd.arm.abt_r13_r14[0];
		cpu->cd.arm.r[14] = cpu->cd.arm.abt_r13_r14[1];
		break;
	case ARM_MODE_UND32:
		memcpy(&cpu->cd.arm.r[8],
		    cpu->cd.arm.default_r8_r14, sizeof(uint32_t) * 5);
		cpu->cd.arm.r[13] = cpu->cd.arm.und_r13_r14[0];
		cpu->cd.arm.r[14] = cpu->cd.arm.und_r13_r14[1];
		break;
	default:fatal("arm_load_register_bank: unimplemented mode %i\n",
		    cpu->cd.arm.cpsr & ARM_FLAG_MODE);
		exit(1);
	}
}


/*
 *  arm_exception():
 */
void arm_exception(struct cpu *cpu, int exception_nr)
{
	int oldmode, newmode;
	uint32_t retaddr;

	if (exception_nr < 0 || exception_nr >= N_ARM_EXCEPTIONS) {
		fatal("arm_exception(): exception_nr = %i\n", exception_nr);
		exit(1);
	}

	retaddr = cpu->pc;

	if (!quiet_mode) {
		debug("[ arm_exception(): ");
		switch (exception_nr) {
		case ARM_EXCEPTION_RESET:
			fatal("RESET: TODO");
			break;
		case ARM_EXCEPTION_UND:
			debug("UNDEFINED");
			break;
		case ARM_EXCEPTION_SWI:
			debug("SWI");
			break;
		case ARM_EXCEPTION_PREF_ABT:
			debug("PREFETCH ABORT");
			break;
		case ARM_EXCEPTION_IRQ:
			debug("IRQ");
			break;
		case ARM_EXCEPTION_FIQ:
			debug("FIQ");
			break;
		case ARM_EXCEPTION_DATA_ABT:
			debug("DATA ABORT, far=0x%08x fsr=0x%02x",
			    cpu->cd.arm.far, cpu->cd.arm.fsr);
			break;
		}
		debug(" ]\n");
	}

	switch (exception_nr) {
	case ARM_EXCEPTION_RESET:
		cpu->running = 0;
		fatal("ARM RESET: TODO");
		exit(1);
	case ARM_EXCEPTION_DATA_ABT:
		retaddr += 4;
		break;
	}

	retaddr += 4;

	arm_save_register_bank(cpu);

	cpu->cd.arm.cpsr &= 0x0fffffff;
	cpu->cd.arm.cpsr |= (cpu->cd.arm.flags << 28);

	switch (arm_exception_to_mode[exception_nr]) {
	case ARM_MODE_SVC32:
		cpu->cd.arm.spsr_svc = cpu->cd.arm.cpsr; break;
	case ARM_MODE_ABT32:
		cpu->cd.arm.spsr_abt = cpu->cd.arm.cpsr; break;
	case ARM_MODE_UND32:
		cpu->cd.arm.spsr_und = cpu->cd.arm.cpsr; break;
	case ARM_MODE_IRQ32:
		cpu->cd.arm.spsr_irq = cpu->cd.arm.cpsr; break;
	case ARM_MODE_FIQ32:
		cpu->cd.arm.spsr_fiq = cpu->cd.arm.cpsr; break;
	default:fatal("arm_exception(): unimplemented exception nr\n");
		exit(1);
	}

	/*
	 *  Disable Thumb mode (because exception handlers always execute
	 *  in ARM mode), set the exception mode, and disable interrupts:
	 */
	cpu->cd.arm.cpsr &= ~ARM_FLAG_T;

	oldmode = cpu->cd.arm.cpsr & ARM_FLAG_MODE;

	cpu->cd.arm.cpsr &= ~ARM_FLAG_MODE;
	cpu->cd.arm.cpsr |= arm_exception_to_mode[exception_nr];

	/*
	 *  Usually, an exception should change modes (so that saved status
	 *  bits don't get lost). However, Linux on ARM seems to use floating
	 *  point instructions in the kernel (!), and it emulates those using
	 *  its own fp emulation code. This leads to a situation where we
	 *  sometimes change from SVC32 to SVC32.
	 */
	newmode = cpu->cd.arm.cpsr & ARM_FLAG_MODE;
	if (oldmode == newmode && oldmode != ARM_MODE_SVC32) {
		fatal("[ WARNING! Exception caused no mode change? "
		    "mode 0x%02x (pc=0x%x) ]\n", newmode, (int)cpu->pc);
		/*  exit(1);  */
	}

	cpu->cd.arm.cpsr |= ARM_FLAG_I;
	if (exception_nr == ARM_EXCEPTION_RESET ||
	    exception_nr == ARM_EXCEPTION_FIQ)
		cpu->cd.arm.cpsr |= ARM_FLAG_F;

	/*  Load the new register bank, if we switched:  */
	arm_load_register_bank(cpu);

	/*
	 *  Set the return address and new PC.
	 *
	 *  NOTE: r[ARM_PC] is also set; see cpu_arm_instr_loadstore.c for
	 *  details. (If an exception occurs during a load into the pc
	 *  register, the code in that file assumes that the r[ARM_PC]
	 *  was changed to the address of the exception handler.)
	 */
	cpu->cd.arm.r[ARM_LR] = retaddr;
	cpu->pc = cpu->cd.arm.r[ARM_PC] = exception_nr * 4 +
	    ((cpu->cd.arm.control & ARM_CONTROL_V)? 0xffff0000 : 0);
	quick_pc_to_pointers(cpu);
}


/*
 *  arm_cpu_tlbdump():
 *
 *  Called from the debugger to dump the TLB in a readable format.
 *  x is the cpu number to dump, or -1 to dump all CPUs.
 *
 *  If rawflag is nonzero, then the TLB contents isn't formated nicely,
 *  just dumped.
 */
void arm_cpu_tlbdump(struct machine *m, int x, int rawflag)
{
}


/*
 *  arm_irq_interrupt_assert():
 *  arm_irq_interrupt_deassert():
 */
void arm_irq_interrupt_assert(struct interrupt *interrupt)
{
	struct cpu *cpu = (struct cpu *) interrupt->extra;
	cpu->cd.arm.irq_asserted = 1;
}
void arm_irq_interrupt_deassert(struct interrupt *interrupt)
{
	struct cpu *cpu = (struct cpu *) interrupt->extra;
	cpu->cd.arm.irq_asserted = 0;
}


/*
 *  arm_cpu_disassemble_instr():
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
int arm_cpu_disassemble_instr(struct cpu *cpu, unsigned char *ib,
        int running, uint64_t dumpaddr)
{
	uint32_t iw, tmp;
	int main_opcode, secondary_opcode, s_bit, r16, r12, r8;
	int i, n, p_bit, u_bit, b_bit, w_bit, l_bit;
	const char *symbol, *condition;
	uint64_t offset;

	if (running)
		dumpaddr = cpu->pc;

	symbol = get_symbol_name(&cpu->machine->symbol_context,
	    dumpaddr, &offset);
	if (symbol != NULL && offset == 0)
		debug("<%s>\n", symbol);

	if (cpu->machine->ncpus > 1 && running)
		debug("cpu%i:\t", cpu->cpu_id);

	debug("%08x:  ", (int)dumpaddr);

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		iw = ib[0] + (ib[1]<<8) + (ib[2]<<16) + (ib[3]<<24);
	else
		iw = ib[3] + (ib[2]<<8) + (ib[1]<<16) + (ib[0]<<24);
	debug("%08x\t", (int)iw);

	condition = arm_condition_string[iw >> 28];
	main_opcode = (iw >> 24) & 15;
	secondary_opcode = (iw >> 21) & 15;
	u_bit = (iw >> 23) & 1;
	b_bit = (iw >> 22) & 1;
	w_bit = (iw >> 21) & 1;
	s_bit = l_bit = (iw >> 20) & 1;
	r16 = (iw >> 16) & 15;
	r12 = (iw >> 12) & 15;
	r8 = (iw >> 8) & 15;

	switch (main_opcode) {
	case 0x0:
	case 0x1:
	case 0x2:
	case 0x3:
		/*
		 *  Special cases first:
		 */

		/*
		 *  Multiplication:
		 *  xxxx0000 00ASdddd nnnnssss 1001mmmm  (Rd, Rm, Rs [,Rn])
		 */
		if ((iw & 0x0fc000f0) == 0x00000090) {
			int a_bit = (iw >> 21) & 1;
			debug("%s%s%s\t", a_bit? "mla" : "mul",
			    condition, s_bit? "s" : "");
			debug("%s,", arm_regname[r16]);
			debug("%s,", arm_regname[iw & 15]);
			debug("%s", arm_regname[r8]);
			if (a_bit)
				debug(",%s", arm_regname[r12]);
			debug("\n");
			break;
		}

		/*
		 *  Long multiplication:
		 *  xxxx0000 1UAShhhh llllssss 1001mmmm  (Rl,Rh,Rm,Rs)
		 */
		if ((iw & 0x0f8000f0) == 0x00800090) {
			int u_bit = (iw >> 22) & 1;
			int a_bit = (iw >> 21) & 1;
			debug("%s%sl%s%s\t", u_bit? "s" : "u",
			    a_bit? "mla" : "mul", condition, s_bit? "s" : "");
			debug("%s,%s,", arm_regname[r12], arm_regname[r16]);
			debug("%s,%s\n", arm_regname[iw&15], arm_regname[r8]);
			break;
		}

		/*
		 *  xxxx0001 0000nnnn dddd0000 0101mmmm  qadd Rd,Rm,Rn
		 *  xxxx0001 0010nnnn dddd0000 0101mmmm  qsub Rd,Rm,Rn
		 *  xxxx0001 0100nnnn dddd0000 0101mmmm  qdadd Rd,Rm,Rn
		 *  xxxx0001 0110nnnn dddd0000 0101mmmm  qdsub Rd,Rm,Rn
		 */
		if ((iw & 0x0f900ff0) == 0x01000050) {
			debug("q%s%s%s\t", iw & 0x400000? "d" : "",
			    iw & 0x200000? "sub" : "add", condition);
			debug("%s,%s,%s\n", arm_regname[r12],
			    arm_regname[iw&15], arm_regname[r16]);
			break;
		}

		/*
		 *  xxxx0001 0010.... ........ 00L1mmmm  bx/blx rm
		 */
		if ((iw & 0x0ff000d0) == 0x01200010) {
			int l_bit = iw & 0x20;
			debug("b%sx%s\t%s\n", l_bit? "l" : "", condition,
			    arm_regname[iw & 15]);
			break;
		}

		/*
		 *  xxxx0001 0s10aaaa 11110000 0000mmmm  MSR  Regform
		 *  xxxx0011 0s10aaaa 1111rrrr bbbbbbbb  MSR  Immform
		 *  xxxx0001 0s001111 dddd0000 00000000  MRS
		 */
		if ((iw & 0x0fb0fff0) == 0x0120f000 ||
		    (iw & 0x0fb0f000) == 0x0320f000) {
			int a = (iw >> 16) & 15;
			debug("msr%s\t%s", condition, (iw&0x400000)? "S":"C");
			debug("PSR_");
			switch (a) {
			case 1:	debug("ctl"); break;
			case 8:	debug("flg"); break;
			case 9:	debug("all"); break;
			default:debug(" UNIMPLEMENTED (a=%i)", a);
			}
			if (iw & 0x02000000) {
				int r = (iw >> 7) & 30;
				uint32_t b = iw & 0xff;
				while (r-- > 0)
					b = (b >> 1) | ((b & 1) << 31);
				debug(",#0x%x\n", b);
			} else
				debug(",%s\n", arm_regname[iw & 15]);
			break;
		}
		if ((iw & 0x0fbf0fff) == 0x010f0000) {
			debug("mrs%s\t", condition);
			debug("%s,%sPSR\n", arm_regname[r12],
			    (iw&0x400000)? "S":"C");
			break;
		}

		/*
		 *  xxxx0001 0B00nnnn dddd0000 1001mmmm    SWP Rd,Rm,[Rn]
		 */
		if ((iw & 0x0fb00ff0) == 0x01000090) {
			debug("swp%s%s\t", condition, (iw&0x400000)? "b":"");
			debug("%s,%s,[%s]\n", arm_regname[r12],
			    arm_regname[iw & 15], arm_regname[r16]);
			break;
		}

		/*
		 *  xxxx0001 0010iiii iiiiiiii 0111iiii    BKPT immed16
		 */
		if ((iw & 0x0ff000f0) == 0x01200070) {
			debug("bkpt%s\t0x%04x\n", condition,
			    ((iw & 0x000fff00) >> 4) + (iw & 0xf));
			break;
		}

		/*
		 *  xxxx0001 01101111 dddd1111 0001mmmm    CLZ Rd,Rm
		 */
		if ((iw & 0x0fff0ff0) == 0x016f0f10) {
			debug("clz%s\t", condition);
			debug("%s,%s\n", arm_regname[r12], arm_regname[iw&15]);
			break;
		}

		/*
		 *  xxxx0001 0000dddd nnnnssss 1yx0mmmm  SMLAxy Rd,Rm,Rs,Rn
		 *  xxxx0001 0100dddd DDDDssss 1yx0mmmm  SMLALxy RdL,RdH,Rm,Rs
		 *  xxxx0001 0010dddd nnnnssss 1y00mmmm  SMLAWy Rd,Rm,Rs,Rn
		 *  xxxx0001 0110dddd 0000ssss 1yx0mmmm  SMULxy Rd,Rm,Rs
		 *  xxxx0001 0010dddd 0000ssss 1y10mmmm  SMULWy Rd,Rm,Rs
		 */
		if ((iw & 0x0ff00090) == 0x01000080) {
			debug("smla%s%s%s\t",
			    iw & 0x20? "t" : "b", iw & 0x40? "t" : "b",
			    condition);
			debug("%s,%s,%s,%s\n", arm_regname[r16],
			    arm_regname[iw&15], arm_regname[r8],
			    arm_regname[r12]);
			break;
		}
		if ((iw & 0x0ff00090) == 0x01400080) {
			debug("smlal%s%s%s\t",
			    iw & 0x20? "t" : "b", iw & 0x40? "t" : "b",
			    condition);
			debug("%s,%s,%s,%s\n", arm_regname[r12],
			    arm_regname[r16], arm_regname[iw&15],
			    arm_regname[r8]);
			break;
		}
		if ((iw & 0x0ff000b0) == 0x01200080) {
			debug("smlaw%s%s\t", iw & 0x40? "t" : "b",
			    condition);
			debug("%s,%s,%s,%s\n", arm_regname[r16],
			    arm_regname[iw&15], arm_regname[r8],
			    arm_regname[r12]);
			break;
		}
		if ((iw & 0x0ff0f090) == 0x01600080) {
			debug("smul%s%s%s\t",
			    iw & 0x20? "t" : "b", iw & 0x40? "t" : "b",
			    condition);
			debug("%s,%s,%s\n", arm_regname[r16],
			    arm_regname[iw&15], arm_regname[r8]);
			break;
		}
		if ((iw & 0x0ff0f0b0) == 0x012000a0) {
			debug("smulw%s%s\t", iw & 0x40? "t" : "b",
			    condition);
			debug("%s,%s,%s\n", arm_regname[r16],
			    arm_regname[iw&15], arm_regname[r8]);
			break;
		}

		/*
		 *  xxxx000P U1WLnnnn ddddHHHH 1SH1LLLL load/store rd,imm(rn)
		 */
		if ((iw & 0x0e000090) == 0x00000090) {
			const char *op = "st";
			int imm = ((iw >> 4) & 0xf0) | (iw & 0xf);
			int regform = !(iw & 0x00400000);
			p_bit = main_opcode & 1;
			/*
			 *  TODO: detect some illegal variants:
			 *  signed store,  or  unsigned byte load/store
			 */
			if (!l_bit && (iw & 0xd0) == 0xd0 && (r12 & 1)) {
				debug("TODO: r12 odd, not load/store\n");
				break;
			}
			/*  Semi-generic case:  */
			if (iw & 0x00100000)
				op = "ld";
			if (!l_bit && (iw & 0xd0) == 0xd0) {
				if (iw & 0x20)
					op = "st";
				else
					op = "ld";
			}
			debug("%sr%s", op, condition);
			if (!l_bit && (iw & 0xd0) == 0xd0) {
				debug("d");		/*  Double-register  */
			} else {
				if (iw & 0x40)
					debug("s");	/*  signed  */
				if (iw & 0x20)
					debug("h");	/*  half-word  */
				else
					debug("b");	/*  byte  */
			}
			debug("\t%s,[%s", arm_regname[r12], arm_regname[r16]);
			if (p_bit) {
				/*  Pre-index:  */
				if (regform)
					debug(",%s%s", u_bit? "" : "-",
					    arm_regname[iw & 15]);
				else {
					if (imm != 0)
						debug(",#%s%i", u_bit? "" : "-",
						    imm);
				}
				debug("]%s\n", w_bit? "!" : "");
			} else {
				/*  Post-index:  */
				debug("],");
				if (regform)
					debug("%s%s\n", u_bit? "" : "-",
					    arm_regname[iw & 15]);
				else
					debug("#%s%i\n", u_bit? "" : "-", imm);
			}
			break;
		}

		/*  Other special cases:  */
		if (iw & 0x80 && !(main_opcode & 2) && iw & 0x10) {
			debug("UNIMPLEMENTED reg (c!=0), t odd\n");
			break;
		}

		/*
		 *  Generic Data Processing Instructions:
		 *
		 *  xxxx000a aaaSnnnn ddddcccc ctttmmmm  Register form
		 *  xxxx001a aaaSnnnn ddddrrrr bbbbbbbb  Immediate form
		 */

		debug("%s%s%s\t", arm_dpiname[secondary_opcode],
		    condition, s_bit? "s" : "");
		if (arm_dpi_uses_d[secondary_opcode])
			debug("%s,", arm_regname[r12]);
		if (arm_dpi_uses_n[secondary_opcode])
			debug("%s,", arm_regname[r16]);

		if (main_opcode & 2) {
			/*  Immediate form:  */
			int r = (iw >> 7) & 30;
			uint32_t b = iw & 0xff;
			while (r-- > 0)
				b = (b >> 1) | ((b & 1) << 31);
			if (b < 15)
				debug("#%i", b);
			else
				debug("#0x%x", b);
		} else {
			/*  Register form:  */
			int t = (iw >> 4) & 7;
			int c = (iw >> 7) & 31;
			debug("%s", arm_regname[iw & 15]);
			switch (t) {
			case 0:	if (c != 0)
					debug(", lsl #%i", c);
				break;
			case 1:	debug(", lsl %s", arm_regname[c >> 1]);
				break;
			case 2:	debug(", lsr #%i", c? c : 32);
				break;
			case 3:	debug(", lsr %s", arm_regname[c >> 1]);
				break;
			case 4:	debug(", asr #%i", c? c : 32);
				break;
			case 5:	debug(", asr %s", arm_regname[c >> 1]);
				break;
			case 6:	if (c != 0)
					debug(", ror #%i", c);
				else
					debug(", rrx");
				break;
			case 7:	debug(", ror %s", arm_regname[c >> 1]);
				break;
			}

			/*  mov pc,reg:  */
			if (running && t == 0 && c == 0 && secondary_opcode
			    == 0xd && r12 == ARM_PC && (iw&15)!=ARM_PC) {
				symbol = get_symbol_name(&cpu->machine->
				    symbol_context, cpu->cd.arm.r[iw & 15],
				    &offset);
				if (symbol != NULL)
					debug(" \t<%s>", symbol);
			}
		}
		debug("\n");
		break;
	case 0x4:				/*  Single Data Transfer  */
	case 0x5:
	case 0x6:
	case 0x7:
		/*  Special case first:  */
		if ((iw & 0xfc70f000) == 0xf450f000) {
			/*  Preload:  */
			debug("pld\t[%s]\n", arm_regname[r16]);
			break;
		}

		/*
		 *  xxxx010P UBWLnnnn ddddoooo oooooooo  Immediate form
		 *  xxxx011P UBWLnnnn ddddcccc ctt0mmmm  Register form
		 */
		p_bit = main_opcode & 1;
		if (main_opcode >= 6 && iw & 0x10) {
			debug("TODO: single data transf. but 0x10\n");
			break;
		}
		debug("%s%s%s", l_bit? "ldr" : "str",
		    condition, b_bit? "b" : "");
		if (!p_bit && w_bit)
			debug("t");
		debug("\t%s,[%s", arm_regname[r12], arm_regname[r16]);
		if ((iw & 0x0e000000) == 0x04000000) {
			/*  Immediate form:  */
			uint32_t imm = iw & 0xfff;
			if (!p_bit)
				debug("]");
			if (imm != 0)
				debug(",#%s%i", u_bit? "" : "-", imm);
			if (p_bit)
				debug("]");
		} else if ((iw & 0x0e000010) == 0x06000000) {
			/*  Register form:  */
			if (!p_bit)
				debug("]");
			if ((iw & 0xfff) != 0)
				debug(",%s%s", u_bit? "" : "-",
				    arm_regname[iw & 15]);
			if ((iw & 0xff0) != 0x000) {
				int c = (iw >> 7) & 31;
				int t = (iw >> 4) & 7;
				switch (t) {
				case 0:	if (c != 0)
						debug(", lsl #%i", c);
					break;
				case 2:	debug(", lsr #%i", c? c : 32);
					break;
				case 4:	debug(", asr #%i", c? c : 32);
					break;
				case 6:	if (c != 0)
						debug(", ror #%i", c);
					else
						debug(", rrx");
					break;
				}
			}
			if (p_bit)
				debug("]");
		} else {
			debug("UNKNOWN\n");
			break;
		}
		debug("%s", (p_bit && w_bit)? "!" : "");
		if ((iw & 0x0f000000) == 0x05000000 &&
		    (r16 == ARM_PC || running)) {
			unsigned char tmpw[4];
			uint32_t imm = iw & 0xfff;
			uint32_t addr = (u_bit? imm : -imm);
			if (r16 == ARM_PC)
				addr += dumpaddr + 8;
			else
				addr += cpu->cd.arm.r[r16];
			symbol = get_symbol_name(&cpu->machine->symbol_context,
			    addr, &offset);
			if (symbol != NULL)
				debug(" \t<%s", symbol);
			else
				debug(" \t<0x%08x", addr);
			if ((l_bit && cpu->memory_rw(cpu, cpu->mem, addr, tmpw,
			    b_bit? 1 : sizeof(tmpw), MEM_READ, NO_EXCEPTIONS))
			    || (!l_bit && running)) {
				if (l_bit) {
					if (cpu->byte_order ==
					    EMUL_LITTLE_ENDIAN)
						addr = tmpw[0] +(tmpw[1] << 8) +
						    (tmpw[2]<<16)+(tmpw[3]<<24);
					else
						addr = tmpw[3] + (tmpw[2]<<8) +
						    (tmpw[1]<<16)+(tmpw[0]<<24);
				} else {
					tmpw[0] = addr = cpu->cd.arm.r[r12];
					if (r12 == ARM_PC)
						addr = cpu->pc + 8;
				}
				debug(": ");
				if (b_bit)
					debug("%i", tmpw[0]);
				else {
					symbol = get_symbol_name(&cpu->machine->
					    symbol_context, addr, &offset);
					if (symbol != NULL)
						debug("%s", symbol);
					else if ((int32_t)addr > -256 &&
						    (int32_t)addr < 256)
						debug("%i", addr);
					else
						debug("0x%x", addr);
				}
			}
			debug(">");
		}
		debug("\n");
		break;
	case 0x8:				/*  Block Data Transfer  */
	case 0x9:
		/*  xxxx100P USWLnnnn llllllll llllllll  */
		p_bit = main_opcode & 1;
		s_bit = b_bit;
		debug("%s%s", l_bit? "ldm" : "stm", condition);
		switch (u_bit * 2 + p_bit) {
		case 0:	debug("da"); break;
		case 1:	debug("db"); break;
		case 2:	debug("ia"); break;
		case 3:	debug("ib"); break;
		}
		debug("\t%s", arm_regname[r16]);
		if (w_bit)
			debug("!");
		debug(",{");
		n = 0;
		for (i=0; i<16; i++)
			if ((iw >> i) & 1) {
				debug("%s%s", (n > 0)? ",":"", arm_regname[i]);
				n++;
			}
		debug("}");
		if (s_bit)
			debug("^");
		debug("\n");
		break;
	case 0xa:				/*  B: branch  */
	case 0xb:				/*  BL: branch and link  */
		debug("b%s%s\t", main_opcode == 0xa? "" : "l", condition);
		tmp = (iw & 0x00ffffff) << 2;
		if (tmp & 0x02000000)
			tmp |= 0xfc000000;
		tmp = (int32_t)(dumpaddr + tmp + 8);
		debug("0x%x", (int)tmp);
		symbol = get_symbol_name(&cpu->machine->symbol_context,
		    tmp, &offset);
		if (symbol != NULL)
			debug(" \t<%s>", symbol);
		debug("\n");
		break;
	case 0xc:				/*  Coprocessor  */
	case 0xd:				/*  LDC/STC  */
		/*
		 *  xxxx1100 0100nnnn ddddcccc oooommmm    MCRR c,op,Rd,Rn,CRm
		 *  xxxx1100 0101nnnn ddddcccc oooommmm    MRRC c,op,Rd,Rn,CRm
		 */
		if ((iw & 0x0fe00fff) == 0x0c400000) {
			debug("%s%s\t", iw & 0x100000? "mra" : "mar",
			    condition);
			if (iw & 0x100000)
				debug("%s,%s,acc0\n",
				    arm_regname[r12], arm_regname[r16]);
			else
				debug("acc0,%s,%s\n",
				    arm_regname[r12], arm_regname[r16]);
			break;
		}
		if ((iw & 0x0fe00000) == 0x0c400000) {
			debug("%s%s\t", iw & 0x100000? "mrrc" : "mcrr",
			    condition);
			debug("%i,%i,%s,%s,cr%i\n", r8, (iw >> 4) & 15,
			    arm_regname[r12], arm_regname[r16], iw & 15);
			break;
		}

		/*  xxxx110P UNWLnnnn DDDDpppp oooooooo LDC/STC  */
		debug("TODO: coprocessor LDC/STC\n");
		break;
	case 0xe:				/*  CDP (Coprocessor Op)  */
		/*				    or MRC/MCR!
		 *  xxxx1110 oooonnnn ddddpppp qqq0mmmm		CDP
		 *  xxxx1110 oooLNNNN ddddpppp qqq1MMMM		MRC/MCR
		 */
		if ((iw & 0x0ff00ff0) == 0x0e200010) {
			/*  Special case: mia* DSP instructions  */
			switch ((iw >> 16) & 0xf) {
			case  0: debug("mia"); break;
			case  8: debug("miaph"); break;
			case 12: debug("miaBB"); break;
			case 13: debug("miaTB"); break;
			case 14: debug("miaBT"); break;
			case 15: debug("miaTT"); break;
			default: debug("UNKNOWN mia vector instruction?");
			}
			debug("%s\t", condition);
			debug("acc%i,%s,%s\n", ((iw >> 5) & 7),
			    arm_regname[iw & 15], arm_regname[r12]);
			break;
		}
		if (iw & 0x10) {
			debug("%s%s\t",
			    (iw & 0x00100000)? "mrc" : "mcr", condition);
			debug("%i,%i,r%i,cr%i,cr%i,%i",
			    (int)((iw >> 8) & 15), (int)((iw >>21) & 7),
			    (int)((iw >>12) & 15), (int)((iw >>16) & 15),
			    (int)((iw >> 0) & 15), (int)((iw >> 5) & 7));
		} else {
			debug("cdp%s\t", condition);
			debug("%i,%i,cr%i,cr%i,cr%i",
			    (int)((iw >> 8) & 15),
			    (int)((iw >>20) & 15),
			    (int)((iw >>12) & 15),
			    (int)((iw >>16) & 15),
			    (int)((iw >> 0) & 15));
			if ((iw >> 5) & 7)
				debug(",0x%x", (int)((iw >> 5) & 7));
		}
		debug("\n");
		break;
	case 0xf:				/*  SWI  */
		debug("swi%s\t", condition);
		debug("0x%x\n", (int)(iw & 0x00ffffff));
		break;
	default:debug("UNIMPLEMENTED\n");
	}

	return sizeof(uint32_t);
}


/*****************************************************************************/


/*
 *  arm_mcr_mrc():
 *
 *  Coprocessor register move.
 *
 *  The program counter should be synched before calling this function (to
 *  make debug output with the correct PC value possible).
 */
void arm_mcr_mrc(struct cpu *cpu, uint32_t iword)
{
	int opcode1 = (iword >> 21) & 7;
	int l_bit = (iword >> 20) & 1;
	int crn = (iword >> 16) & 15;
	int rd = (iword >> 12) & 15;
	int cp_num = (iword >> 8) & 15;
	int opcode2 = (iword >> 5) & 7;
	int crm = iword & 15;

	if (cpu->cd.arm.coproc[cp_num] != NULL)
		cpu->cd.arm.coproc[cp_num](cpu, opcode1, opcode2, l_bit,
		    crn, crm, rd);
	else {
		fatal("[ arm_mcr_mrc: pc=0x%08x, iword=0x%08x: "
		    "cp_num=%i ]\n", (int)cpu->pc, iword, cp_num);
		arm_exception(cpu, ARM_EXCEPTION_UND);
		/*  exit(1);  */
	}
}


/*
 *  arm_cdp():
 *
 *  Coprocessor operations.
 *
 *  The program counter should be synched before calling this function (to
 *  make debug output with the correct PC value possible).
 */
void arm_cdp(struct cpu *cpu, uint32_t iword)
{
	fatal("[ arm_cdp: pc=0x%08x, iword=0x%08x ]\n", (int)cpu->pc, iword);
	arm_exception(cpu, ARM_EXCEPTION_UND);
	/*  exit(1);  */
}


/*****************************************************************************/


#include "tmp_arm_tail.cc"

