/*
 *  Copyright (C) 2003-2009  Anders Gavare.  All rights reserved.
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
 *  MIPS core CPU emulation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <unistd.h>

#include "../../config.h"

#include "arcbios.h"
#include "cop0.h"
#include "cpu.h"
#include "cpu_mips.h"
#include "debugger.h"
#include "devices.h"
#include "emul.h"
#include "machine.h"
#include "memory.h"
#include "mips_cpu_types.h"
#include "opcodes_mips.h"
#include "settings.h"
#include "symbol.h"


static const char *exception_names[] = EXCEPTION_NAMES;

static const char *hi6_names[] = HI6_NAMES;
static const char *regimm_names[] = REGIMM_NAMES;
static const char *special_names[] = SPECIAL_NAMES;
static const char *special_rot_names[] = SPECIAL_ROT_NAMES;
static const char *special2_names[] = SPECIAL2_NAMES;
static const char *mmi_names[] = MMI_NAMES;
static const char *mmi0_names[] = MMI0_NAMES;
static const char *mmi1_names[] = MMI1_NAMES;
static const char *mmi2_names[] = MMI2_NAMES;
static const char *mmi3_names[] = MMI3_NAMES;
static const char *special3_names[] = SPECIAL3_NAMES;

static const char *regnames[] = MIPS_REGISTER_NAMES;
static const char *cop0_names[] = COP0_NAMES;


#define DYNTRANS_DUALMODE_32
#define DYNTRANS_DELAYSLOT
#include "tmp_mips_head.cc"

void mips_pc_to_pointers(struct cpu *);
void mips32_pc_to_pointers(struct cpu *);


/*
 *  mips_cpu_new():
 *
 *  Create a new MIPS cpu object.
 *
 *  Returns 1 on success, 0 if there was no valid MIPS processor with
 *  a matching name.
 */
int mips_cpu_new(struct cpu *cpu, struct memory *mem, struct machine *machine,
	int cpu_id, char *cpu_type_name)
{
	int i, found, j, tags_size, n_cache_lines, size_per_cache_line;
	struct mips_cpu_type_def cpu_type_defs[] = MIPS_CPU_TYPE_DEFS;
	int64_t secondary_cache_size;
	int x, linesize;

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

	cpu->memory_rw                = mips_memory_rw;
	cpu->cd.mips.cpu_type         = cpu_type_defs[found];
	cpu->name                     = strdup(cpu->cd.mips.cpu_type.name);
	cpu->byte_order               = EMUL_LITTLE_ENDIAN;
	cpu->cd.mips.gpr[MIPS_GPR_SP] = INITIAL_STACK_POINTER;

	if (cpu->cd.mips.cpu_type.isa_level <= 2 ||
	    cpu->cd.mips.cpu_type.isa_level == 32)
		cpu->is_32bit = 1;

	if (cpu->is_32bit) {
		cpu->run_instr = mips32_run_instr;
		cpu->update_translation_table = mips32_update_translation_table;
		cpu->invalidate_translation_caches =
		    mips32_invalidate_translation_caches;
		cpu->invalidate_code_translation =
		    mips32_invalidate_code_translation;
	} else {
		cpu->run_instr = mips_run_instr;
		cpu->update_translation_table = mips_update_translation_table;
		cpu->invalidate_translation_caches =
		    mips_invalidate_translation_caches;
		cpu->invalidate_code_translation =
		    mips_invalidate_code_translation;
	}

	cpu->instruction_has_delayslot = mips_cpu_instruction_has_delayslot;

	if (cpu_id == 0)
		debug("%s", cpu->cd.mips.cpu_type.name);

	/*
	 *  CACHES:
	 *
	 *  1) Use DEFAULT_PCACHE_SIZE and DEFAULT_PCACHE_LINESIZE etc.
	 *  2) If there are specific values defined for this type of cpu,
	 *     in its cpu_type substruct, then let's use those.
	 *  3) Values in the emul struct override both of the above.
	 *
	 *  Once we've decided which values to use, they are stored in
	 *  the emul struct so they can be used from src/machine.c etc.
	 */

	x = DEFAULT_PCACHE_SIZE;
	if (cpu->cd.mips.cpu_type.pdcache)
		x = cpu->cd.mips.cpu_type.pdcache;
	if (cpu->cd.mips.cache_pdcache == 0)
		cpu->cd.mips.cache_pdcache = x;

	x = DEFAULT_PCACHE_SIZE;
	if (cpu->cd.mips.cpu_type.picache)
		x = cpu->cd.mips.cpu_type.picache;
	if (cpu->cd.mips.cache_picache == 0)
		cpu->cd.mips.cache_picache = x;

	if (cpu->cd.mips.cache_secondary == 0)
		cpu->cd.mips.cache_secondary = cpu->cd.mips.cpu_type.scache;

	linesize = DEFAULT_PCACHE_LINESIZE;
	if (cpu->cd.mips.cpu_type.pdlinesize)
		linesize = cpu->cd.mips.cpu_type.pdlinesize;
	if (cpu->cd.mips.cache_pdcache_linesize == 0)
		cpu->cd.mips.cache_pdcache_linesize = linesize;

	linesize = DEFAULT_PCACHE_LINESIZE;
	if (cpu->cd.mips.cpu_type.pilinesize)
		linesize = cpu->cd.mips.cpu_type.pilinesize;
	if (cpu->cd.mips.cache_picache_linesize == 0)
		cpu->cd.mips.cache_picache_linesize = linesize;

	linesize = 0;
	if (cpu->cd.mips.cpu_type.slinesize)
		linesize = cpu->cd.mips.cpu_type.slinesize;
	if (cpu->cd.mips.cache_secondary_linesize == 0)
		cpu->cd.mips.cache_secondary_linesize = linesize;


	/*
	 *  Primary Data and Instruction caches:
	 */
	for (i=CACHE_DATA; i<=CACHE_INSTRUCTION; i++) {
		switch (i) {
		case CACHE_DATA:
			x = 1 << cpu->cd.mips.cache_pdcache;
			linesize = 1 << cpu->cd.mips.cache_pdcache_linesize;
			break;
		case CACHE_INSTRUCTION:
			x = 1 << cpu->cd.mips.cache_picache;
			linesize = 1 << cpu->cd.mips.cache_picache_linesize;
			break;
		}

		/*  Primary cache size and linesize:  */
		cpu->cd.mips.cache_size[i] = x;
		cpu->cd.mips.cache_linesize[i] = linesize;

		switch (cpu->cd.mips.cpu_type.rev) {
		case MIPS_R2000:
		case MIPS_R3000:
			size_per_cache_line = sizeof(struct r3000_cache_line);
			break;
		default:
			size_per_cache_line = 32;	/*  TODO  */
		}

		cpu->cd.mips.cache_mask[i] = cpu->cd.mips.cache_size[i] - 1;

		CHECK_ALLOCATION(cpu->cd.mips.cache[i] = (unsigned char *)
		    malloc(cpu->cd.mips.cache_size[i]));

		n_cache_lines = cpu->cd.mips.cache_size[i] /
		    cpu->cd.mips.cache_linesize[i];
		tags_size = n_cache_lines * size_per_cache_line;

		CHECK_ALLOCATION(cpu->cd.mips.cache_tags[i] =
		    malloc(tags_size));

		/*  Initialize the cache tags:  */
		switch (cpu->cd.mips.cpu_type.rev) {
		case MIPS_R2000:
		case MIPS_R3000:
			for (j=0; j<n_cache_lines; j++) {
				struct r3000_cache_line *rp;
				rp = (struct r3000_cache_line *)
				    cpu->cd.mips.cache_tags[i];
				rp[j].tag_paddr = 0;
				rp[j].tag_valid = 0;
			}
			break;
		default:
			;
		}

		/*  Set cache_last_paddr to something "impossible":  */
		cpu->cd.mips.cache_last_paddr[i] = IMPOSSIBLE_PADDR;
	}

	/*
	 *  Secondary cache:
	 */
	secondary_cache_size = 0;
	if (cpu->cd.mips.cache_secondary)
		secondary_cache_size = 1 << cpu->cd.mips.cache_secondary;
	/*  TODO: linesize...  */

	if (cpu_id == 0) {
		debug(" (I+D = %i+%i KB",
		    (int)(cpu->cd.mips.cache_size[CACHE_INSTRUCTION] / 1024),
		    (int)(cpu->cd.mips.cache_size[CACHE_DATA] / 1024));

		if (secondary_cache_size != 0) {
			debug(", L2 = ");
			if (secondary_cache_size >= 1048576)
				debug("%i MB", (int)
				    (secondary_cache_size / 1048576));
			else
				debug("%i KB", (int)
				    (secondary_cache_size / 1024));
		}

		debug(")");
	}

	/*  Register the CPU's interrupts:  */
	for (i=2; i<8; i++) {
		struct interrupt templ;
		char name[50];
		snprintf(name, sizeof(name), "%s.%i", cpu->path, i);
		memset(&templ, 0, sizeof(templ));
		templ.line = 1 << (STATUS_IM_SHIFT + i);
		templ.name = name;
		templ.extra = cpu;
		templ.interrupt_assert = mips_cpu_interrupt_assert;
		templ.interrupt_deassert = mips_cpu_interrupt_deassert;
		interrupt_handler_register(&templ);

		if (i == 7)
			INTERRUPT_CONNECT(name, cpu->cd.mips.irq_compare);
	}

	/*  System coprocessor (0), and FPU (1):  */
	cpu->cd.mips.coproc[0] = mips_coproc_new(cpu, 0);
	cpu->cd.mips.coproc[1] = mips_coproc_new(cpu, 1);

	switch (cpu->cd.mips.cpu_type.mmu_model) {
	case MMU3K:
		cpu->translate_v2p = translate_v2p_mmu3k;
		break;
	case MMU8K:
		cpu->translate_v2p = translate_v2p_mmu8k;
		break;
	case MMU10K:
		cpu->translate_v2p = translate_v2p_mmu10k;
		break;
	default:
		if (cpu->cd.mips.cpu_type.rev == MIPS_R4100)
			cpu->translate_v2p = translate_v2p_mmu4100;
		else
			cpu->translate_v2p = translate_v2p_generic;
	}

	if (cpu->machine->prom_emulation) {
		/*
		 *  Default behaviour of jumping to 0xbfc00000 should be
		 *  a reboot, unless machine-specific initialization code
		 *  overrides this.
		 *
		 *  Note: Specifically big-endian machines should override
		 *  this, since the default MIPS CPU is little-endian!
		 */
		store_32bit_word(cpu, 0xffffffff9fc00000ULL, 0x00c0de0d);
	}

	/*  Add all register names to the settings:  */
	CPU_SETTINGS_ADD_REGISTER64("pc", cpu->pc);
	CPU_SETTINGS_ADD_REGISTER64("hi", cpu->cd.mips.hi);
	CPU_SETTINGS_ADD_REGISTER64("lo", cpu->cd.mips.lo);
	for (i=0; i<N_MIPS_GPRS; i++)
		CPU_SETTINGS_ADD_REGISTER64(regnames[i], cpu->cd.mips.gpr[i]);
	/*  TODO: Write via special handler function!  */
	for (i=0; i<N_MIPS_COPROC_REGS; i++)
		CPU_SETTINGS_ADD_REGISTER64(cop0_names[i],
		    cpu->cd.mips.coproc[0]->reg[i]);

	return 1;
}


/*
 *  mips_cpu_dumpinfo():
 *
 *  Debug dump of MIPS-specific CPU data for specific CPU.
 */
void mips_cpu_dumpinfo(struct cpu *cpu)
{
	int iadd = DEBUG_INDENTATION;
	struct mips_cpu_type_def *ct = &cpu->cd.mips.cpu_type;

	debug_indentation(iadd);

	debug("\n%i-bit %s-endian (MIPS",
	    cpu->is_32bit? 32 : 64,
	    cpu->byte_order == EMUL_BIG_ENDIAN? "Big" : "Little");

	switch (ct->isa_level) {
	case 1:	debug(" ISA I"); break;
	case 2:	debug(" ISA II"); break;
	case 3:	debug(" ISA III"); break;
	case 4:	debug(" ISA IV"); break;
	case 5:	debug(" ISA V"); break;
	case 32:
	case 64:debug("%i, revision %i", ct->isa_level, ct->isa_revision);
		break;
	default:debug(" ISA level %i", ct->isa_level);
	}

	debug("), ");
	if (ct->nr_of_tlb_entries)
		debug("%i TLB entries", ct->nr_of_tlb_entries);
	else
		debug("no TLB");
	debug("\n");

	if (ct->picache) {
		debug("L1 I-cache: %i KB", (1 << ct->picache) / 1024);
		if (ct->pilinesize)
			debug(", %i bytes per line", 1 << ct->pilinesize);
		if (ct->piways > 1)
			debug(", %i-way", ct->piways);
		else
			debug(", direct-mapped");
		debug("\n");
	}

	if (ct->pdcache) {
		debug("L1 D-cache: %i KB", (1 << ct->pdcache) / 1024);
		if (ct->pdlinesize)
			debug(", %i bytes per line", 1 << ct->pdlinesize);
		if (ct->pdways > 1)
			debug(", %i-way", ct->pdways);
		else
			debug(", direct-mapped");
		debug("\n");
	}

	if (ct->scache) {
		int kb = (1 << ct->scache) / 1024;
		debug("L2 cache: %i %s",
		    kb >= 1024? kb / 1024 : kb, kb >= 1024? "MB":"KB");
		if (ct->slinesize)
			debug(", %i bytes per line", 1 << ct->slinesize);
		if (ct->sways > 1)
			debug(", %i-way", ct->sways);
		else
			debug(", direct-mapped");
		debug("\n");
	}

	debug_indentation(-iadd);
}


/*
 *  mips_cpu_list_available_types():
 *
 *  Print a list of available MIPS CPU types.
 */
void mips_cpu_list_available_types(void)
{
	int i, j;
	struct mips_cpu_type_def cpu_type_defs[] = MIPS_CPU_TYPE_DEFS;

	i = 0;
	while (cpu_type_defs[i].name != NULL) {
		debug("%s", cpu_type_defs[i].name);
		for (j=10 - strlen(cpu_type_defs[i].name); j>0; j--)
			debug(" ");
		i++;
		if ((i % 6) == 0 || cpu_type_defs[i].name == NULL)
			debug("\n");
	}
}


/*
 *  mips_cpu_instruction_has_delayslot():
 *
 *  Return 1 if an opcode is a branch, 0 otherwise.
 */
int mips_cpu_instruction_has_delayslot(struct cpu *cpu, unsigned char *ib)
{
	uint32_t iword = *((uint32_t *)&ib[0]);

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		iword = LE32_TO_HOST(iword);
	else
		iword = BE32_TO_HOST(iword);

	switch (iword >> 26) {
	case HI6_SPECIAL:
		switch (iword & 0x3f) {
		case SPECIAL_JR:
		case SPECIAL_JALR:
			return 1;
		}
		break;
	case HI6_REGIMM:
		switch ((iword >> 16) & 0x1f) {
		case REGIMM_BLTZ:
		case REGIMM_BGEZ:
		case REGIMM_BLTZL:
		case REGIMM_BGEZL:
		case REGIMM_BLTZAL:
		case REGIMM_BLTZALL:
		case REGIMM_BGEZAL:
		case REGIMM_BGEZALL:
			return 1;
		}
		break;
	case HI6_BEQ:
	case HI6_BEQL:
	case HI6_BNE:
	case HI6_BNEL:
	case HI6_BGTZ:
	case HI6_BGTZL:
	case HI6_BLEZ:
	case HI6_BLEZL:
	case HI6_J:
	case HI6_JAL:
		return 1;
	}

	return 0;
}


/*
 *  mips_cpu_tlbdump():
 *
 *  Called from the debugger to dump the TLB in a readable format.
 *  x is the cpu number to dump, or -1 to dump all CPUs.
 *
 *  If rawflag is nonzero, then the TLB contents isn't formated nicely,
 *  just dumped.
 */
void mips_cpu_tlbdump(struct machine *m, int x, int rawflag)
{
	int i, j;

	/*  Raw output:  */
	if (rawflag) {
		for (i=0; i<m->ncpus; i++) {
			struct mips_coproc *cop0 =
			    m->cpus[i]->cd.mips.coproc[0];

			if (x >= 0 && i != x)
				continue;

			/*  Print index, random, and wired:  */
			printf("cpu%i: (", i);

			if (m->cpus[i]->is_32bit)
				printf("index=0x%08x random=0x%08x",
				    (int) cop0->reg[COP0_INDEX],
				    (int) cop0->reg[COP0_RANDOM]);
			else
				printf("index=0x%016"PRIx64
				    " random=0x%016"PRIx64,
				    (uint64_t) cop0->reg[COP0_INDEX],
				    (uint64_t) cop0->reg[COP0_RANDOM]);

			if (m->cpus[i]->cd.mips.cpu_type.isa_level >= 3)
				printf(" wired=0x%"PRIx64,
				    (uint64_t) cop0->reg[COP0_WIRED]);

			printf(")\n");

			for (j=0; j<m->cpus[i]->cd.mips.cpu_type.
			    nr_of_tlb_entries; j++) {
				if (m->cpus[i]->cd.mips.cpu_type.mmu_model ==
				    MMU3K)
					printf("%3i: hi=0x%08"PRIx32" lo=0x%08"
					    PRIx32"\n", j,
					    (uint32_t) cop0->tlbs[j].hi,
					    (uint32_t) cop0->tlbs[j].lo0);
				else if (m->cpus[i]->is_32bit)
					printf("%3i: hi=0x%08"PRIx32" mask=0x"
					    "%08"PRIx32" lo0=0x%08"PRIx32
					    " lo1=0x%08"PRIx32"\n", j,
					    (uint32_t) cop0->tlbs[j].hi,
					    (uint32_t) cop0->tlbs[j].mask,
					    (uint32_t) cop0->tlbs[j].lo0,
					    (uint32_t) cop0->tlbs[j].lo1);
				else
					printf("%3i: hi=0x%016"PRIx64" mask="
					    "0x%016"PRIx64" lo0=0x%016"PRIx64
					    " lo1=0x%016"PRIx64"\n", j,
					    (uint64_t) cop0->tlbs[j].hi,
					    (uint64_t) cop0->tlbs[j].mask,
					    (uint64_t) cop0->tlbs[j].lo0,
					    (uint64_t) cop0->tlbs[j].lo1);
			}
		}

		return;
	}

	/*  Nicely formatted output:  */
	for (i=0; i<m->ncpus; i++) {
		int pageshift = 12;
		struct mips_coproc *cop0 = m->cpus[i]->cd.mips.coproc[0];

		if (x >= 0 && i != x)
			continue;

		if (m->cpus[i]->cd.mips.cpu_type.rev == MIPS_R4100)
			pageshift = 10;

		/*  Print index, random, and wired:  */
		printf("cpu%i: (", i);
		switch (m->cpus[i]->cd.mips.cpu_type.isa_level) {
		case 1:
		case 2:	printf("index=0x%x random=0x%x",
			    (int) ((cop0->reg[COP0_INDEX] & R2K3K_INDEX_MASK)
			    >> R2K3K_INDEX_SHIFT),
			    (int) ((cop0->reg[COP0_RANDOM] & R2K3K_RANDOM_MASK)
			    >> R2K3K_RANDOM_SHIFT));
			break;
		default:printf("index=0x%x random=0x%x",
			    (int) (cop0->reg[COP0_INDEX] & INDEX_MASK),
			    (int) (cop0->reg[COP0_RANDOM] & RANDOM_MASK));
			printf(" wired=0x%"PRIx64,
			    (uint64_t) cop0->reg[COP0_WIRED]);
		}

		printf(")\n");

		for (j=0; j<m->cpus[i]->cd.mips.cpu_type.
		    nr_of_tlb_entries; j++) {
			uint64_t hi = cop0->tlbs[j].hi;
			uint64_t lo0 = cop0->tlbs[j].lo0;
			uint64_t lo1 = cop0->tlbs[j].lo1;
			uint64_t mask = cop0->tlbs[j].mask;
			uint64_t psize;

			mask |= (1 << (pageshift+1)) - 1;
			/*  here mask = e.g. 0x1fff for 4KB pages  */

			printf("%3i: ", j);

			switch (m->cpus[i]->cd.mips.cpu_type.mmu_model) {
			case MMU3K:
				if (!(lo0 & R2K3K_ENTRYLO_V)) {
					printf("(invalid)\n");
					continue;
				}
				printf("vaddr=0x%08x ",
				    (int) (hi&R2K3K_ENTRYHI_VPN_MASK));
				if (lo0 & R2K3K_ENTRYLO_G)
					printf("(global), ");
				else
					printf("(asid %02x),", (int) ((hi &
					    R2K3K_ENTRYHI_ASID_MASK)
					    >> R2K3K_ENTRYHI_ASID_SHIFT));
				printf(" paddr=0x%08x ",
				    (int) (lo0&R2K3K_ENTRYLO_PFN_MASK));
				if (lo0 & R2K3K_ENTRYLO_N)
					printf("N");
				if (lo0 & R2K3K_ENTRYLO_D)
					printf("D");
				printf("\n");
				break;
			default:switch (m->cpus[i]->cd.mips.cpu_type.mmu_model){
				case MMU32:
					printf("vaddr=0x%08"PRIx32" ",
					    (uint32_t) (hi & ~mask));
					break;
				default:/*  R4x00, R1x000, MIPS64, etc.  */
					printf("vaddr=%016"PRIx64" ",
					    (uint64_t) (hi & ~mask));
				}
				if (hi & TLB_G)
					printf("(global): ");
				else
					printf("(asid %02x):",
					    (int) (hi & ENTRYHI_ASID));

				/*  TODO: Coherency bits  */

				if (!(lo0 & ENTRYLO_V))
					printf(" p0=(invalid)   ");
				else {
					uint64_t paddr = lo0 & ENTRYLO_PFN_MASK;
					paddr >>= ENTRYLO_PFN_SHIFT;
					paddr <<= pageshift;
					paddr &= ~(mask >> 1);
					printf(" p0=0x%09"PRIx64" ",
					    (uint64_t) paddr);
				}
				printf(lo0 & ENTRYLO_D? "D" : " ");

				if (!(lo1 & ENTRYLO_V))
					printf(" p1=(invalid)   ");
				else {
					uint64_t paddr = lo1 & ENTRYLO_PFN_MASK;
					paddr >>= ENTRYLO_PFN_SHIFT;
					paddr <<= pageshift;
					paddr &= ~(mask >> 1);
					printf(" p1=0x%09"PRIx64" ",
					    (uint64_t) paddr);
				}
				printf(lo1 & ENTRYLO_D? "D" : " ");

				/*  convert e.g. 0x1fff to 4096  */
				psize = (mask + 1) >> 1;

				if (psize >= 1024 && psize <= 256*1024)
					printf(" (%iKB)", (int) (psize >> 10));
				else if (psize >= 1024*1024 && psize <=
				    64*1024*1024)
					printf(" (%iMB)", (int) (psize >> 20));
				else
					printf(" (?)");

				printf("\n");
			}
		}
	}
}


/*
 *  mips_cpu_disassemble_instr():
 *
 *  Convert an instruction word into human readable format, for instruction
 *  tracing.
 *
 *  If running is 1, cpu->pc should be the address of the instruction.
 *
 *  If running is 0, things that depend on the runtime environment (eg.
 *  register contents) will not be shown, and addr will be used instead of
 *  cpu->pc for relative addresses.
 *
 *  NOTE 2:  coprocessor instructions are not decoded nicely yet  (TODO)
 */
int mips_cpu_disassemble_instr(struct cpu *cpu, unsigned char *originstr,
	int running, uint64_t dumpaddr)
{
	int hi6, special6, regimm5, sub;
	int rt, rd, rs, sa, imm, copz, cache_op, which_cache, showtag;
	uint64_t addr, offset;
	uint32_t instrword;
	unsigned char instr[4];
	char *symbol;

	if (running)
		dumpaddr = cpu->pc;

	if ((dumpaddr & 3) != 0)
		printf("WARNING: Unaligned address!\n");

	symbol = get_symbol_name(&cpu->machine->symbol_context,
	    dumpaddr, &offset);
	if (symbol != NULL && offset==0)
		debug("<%s>\n", symbol);

	if (cpu->machine->ncpus > 1 && running)
		debug("cpu%i: ", cpu->cpu_id);

	if (cpu->is_32bit)
		debug("%08"PRIx32, (uint32_t)dumpaddr);
	else
		debug("%016"PRIx64, (uint64_t)dumpaddr);

	memcpy(instr, originstr, sizeof(uint32_t));

	/*
	 *  The rest of the code is written for little endian,
	 *  so swap if necessary:
	 */
	if (cpu->byte_order == EMUL_BIG_ENDIAN) {
		int tmp = instr[0]; instr[0] = instr[3];
		    instr[3] = tmp;
		tmp = instr[1]; instr[1] = instr[2];
		    instr[2] = tmp;
	}

	debug(": %02x%02x%02x%02x",
	    instr[3], instr[2], instr[1], instr[0]);

	if (running && cpu->delay_slot)
		debug(" (d)");

	debug("\t");

	/*
	 *  Decode the instruction:
	 */

	hi6 = (instr[3] >> 2) & 0x3f;

	switch (hi6) {
	case HI6_SPECIAL:
		special6 = instr[0] & 0x3f;
		switch (special6) {
		case SPECIAL_SLL:
		case SPECIAL_SRL:
		case SPECIAL_SRA:
		case SPECIAL_DSLL:
		case SPECIAL_DSRL:
		case SPECIAL_DSRA:
		case SPECIAL_DSLL32:
		case SPECIAL_DSRL32:
		case SPECIAL_DSRA32:
			sub = ((instr[3] & 3) << 3) + ((instr[2] >> 5) & 7);
			rt = instr[2] & 31;
			rd = (instr[1] >> 3) & 31;
			sa = ((instr[1] & 7) << 2) + ((instr[0] >> 6) & 3);

			if (rd == 0 && special6 == SPECIAL_SLL) {
				if (sa == 0)
					debug("nop");
				else if (sa == 1)
					debug("ssnop");
				else if (sa == 3)
					debug("ehb");
				else
					debug("nop (weird, sa=%i)", sa);
				break;
			}

			switch (sub) {
			case 0x00:
				debug("%s\t%s,", special_names[special6],
				    regnames[rd]);
				debug("%s,%i", regnames[rt], sa);
				break;
			case 0x01:
				debug("%s\t%s,",
				    special_rot_names[special6],
				    regnames[rd]);
				debug("%s,%i", regnames[rt], sa);
				break;
			default:debug("UNIMPLEMENTED special, sub=0x%02x\n",
				    sub);
			}
			break;
		case SPECIAL_DSRLV:
		case SPECIAL_DSRAV:
		case SPECIAL_DSLLV:
		case SPECIAL_SLLV:
		case SPECIAL_SRAV:
		case SPECIAL_SRLV:
			rs = ((instr[3] & 3) << 3) + ((instr[2] >> 5) & 7);
			rt = instr[2] & 31;
			rd = (instr[1] >> 3) & 31;
			sub = ((instr[1] & 7) << 2) + ((instr[0] >> 6) & 3);

			switch (sub) {
			case 0x00:
				debug("%s\t%s", special_names[special6],
				    regnames[rd]);
				debug(",%s", regnames[rt]);
				debug(",%s", regnames[rs]);
				break;
			case 0x01:
				debug("%s\t%s", special_rot_names[special6],
				    regnames[rd]);
				debug(",%s", regnames[rt]);
				debug(",%s", regnames[rs]);
				break;
			default:debug("UNIMPLEMENTED special, sub=0x%02x\n",
				    sub);
			}
			break;
		case SPECIAL_JR:
			rs = ((instr[3] & 3) << 3) + ((instr[2] >> 5) & 7);
			symbol = get_symbol_name(&cpu->machine->symbol_context,
			    cpu->cd.mips.gpr[rs], &offset);
			/*  .hb = hazard barrier hint on MIPS32/64 rev 2  */
			debug("jr%s\t%s",
			    (instr[1] & 0x04) ? ".hb" : "",
			    regnames[rs]);
			if (running && symbol != NULL)
				debug("\t<%s>", symbol);
			break;
		case SPECIAL_JALR:
			rs = ((instr[3] & 3) << 3) + ((instr[2] >> 5) & 7);
			rd = (instr[1] >> 3) & 31;
			symbol = get_symbol_name(&cpu->machine->symbol_context,
			    cpu->cd.mips.gpr[rs], &offset);
			/*  .hb = hazard barrier hint on MIPS32/64 rev 2  */
			debug("jalr%s\t%s",
			    (instr[1] & 0x04) ? ".hb" : "",
			    regnames[rd]);
			debug(",%s", regnames[rs]);
			if (running && symbol != NULL)
				debug("\t<%s>", symbol);
			break;
		case SPECIAL_MFHI:
		case SPECIAL_MFLO:
			rd = (instr[1] >> 3) & 31;
			debug("%s\t%s", special_names[special6], regnames[rd]);
			break;
		case SPECIAL_MTLO:
		case SPECIAL_MTHI:
			rs = ((instr[3] & 3) << 3) + ((instr[2] >> 5) & 7);
			debug("%s\t%s", special_names[special6], regnames[rs]);
			break;
		case SPECIAL_ADD:
		case SPECIAL_ADDU:
		case SPECIAL_SUB:
		case SPECIAL_SUBU:
		case SPECIAL_AND:
		case SPECIAL_OR:
		case SPECIAL_XOR:
		case SPECIAL_NOR:
		case SPECIAL_SLT:
		case SPECIAL_SLTU: 
		case SPECIAL_DADD:
		case SPECIAL_DADDU:
		case SPECIAL_DSUB:
		case SPECIAL_DSUBU:
		case SPECIAL_MOVZ:
		case SPECIAL_MOVN:
			rs = ((instr[3] & 3) << 3) + ((instr[2] >> 5) & 7);
			rt = instr[2] & 31;
			rd = (instr[1] >> 3) & 31;
			if (cpu->is_32bit && (special6 == SPECIAL_ADDU ||
			    special6 == SPECIAL_SUBU) && rt == 0) {
				/*  Special case 1: addu/subu with
				    rt = the zero register ==> move  */
				debug("move\t%s", regnames[rd]);
				debug(",%s", regnames[rs]);
			} else if (special6 == SPECIAL_ADDU && cpu->is_32bit
			    && rs == 0) {
				/*  Special case 2: addu with
				    rs = the zero register ==> move  */
				debug("move\t%s", regnames[rd]);
				debug(",%s", regnames[rt]);
			} else {
				debug("%s\t%s", special_names[special6],
				    regnames[rd]);
				debug(",%s", regnames[rs]);
				debug(",%s", regnames[rt]);
			}
			break;
		case SPECIAL_MULT:
		case SPECIAL_MULTU:
		case SPECIAL_DMULT:
		case SPECIAL_DMULTU:
		case SPECIAL_DIV:
		case SPECIAL_DIVU:
		case SPECIAL_DDIV:
		case SPECIAL_DDIVU:
			rs = ((instr[3] & 3) << 3) + ((instr[2] >> 5) & 7);
			rt = instr[2] & 31;
			rd = (instr[1] >> 3) & 31;
			debug("%s\t", special_names[special6]);
			if (rd != 0) {
				if (cpu->cd.mips.cpu_type.rev == MIPS_R5900) {
					if (special6 == SPECIAL_MULT ||
					    special6 == SPECIAL_MULTU)
						debug("%s,", regnames[rd]);
					else
						debug("WEIRD_R5900_RD,");
				} else {
					debug("WEIRD_RD_NONZERO,");
				}
			}
			debug("%s", regnames[rs]);
			debug(",%s", regnames[rt]);
			break;
		case SPECIAL_TGE:
		case SPECIAL_TGEU:
		case SPECIAL_TLT:
		case SPECIAL_TLTU:
		case SPECIAL_TEQ:
		case SPECIAL_TNE:
			rs = ((instr[3] & 3) << 3) + ((instr[2] >> 5) & 7);
			rt = instr[2] & 31;
			rd = ((instr[1] << 8) + instr[0]) >> 6;	// code, not rd
			debug("%s\t", special_names[special6]);
			debug("%s", regnames[rs]);
			debug(",%s", regnames[rt]);
			if (rd != 0)
				debug(",0x%x", rd);
			break;
		case SPECIAL_SYNC:
			imm = ((instr[1] & 7) << 2) + (instr[0] >> 6);
			debug("sync\t0x%02x", imm);
			break;
		case SPECIAL_SYSCALL:
			imm = (((instr[3] << 24) + (instr[2] << 16) +
			    (instr[1] << 8) + instr[0]) >> 6) & 0xfffff;
			if (imm != 0)
				debug("syscall\t0x%05x", imm);
			else
				debug("syscall");
			break;
		case SPECIAL_BREAK:
			imm = (((instr[3] << 24) + (instr[2] << 16) +
			    (instr[1] << 8) + instr[0]) >> 6) & 0xfffff;
			if (imm != 0)
				debug("break\t0x%05x", imm);
			else
				debug("break");
			break;
		case SPECIAL_MFSA:
			if (cpu->cd.mips.cpu_type.rev == MIPS_R5900) {
				rd = (instr[1] >> 3) & 31;
				debug("mfsa\t%s", regnames[rd]);
			} else {
				debug("unimplemented special 0x28");
			}
			break;
		case SPECIAL_MTSA:
			if (cpu->cd.mips.cpu_type.rev == MIPS_R5900) {
				rs = ((instr[3] & 3) << 3) +
				    ((instr[2] >> 5) & 7);
				debug("mtsa\t%s", regnames[rs]);
			} else {
				debug("unimplemented special 0x29");
			}
			break;
		default:
			debug("%s\t= UNIMPLEMENTED", special_names[special6]);
		}
		break;
	case HI6_BEQ:
	case HI6_BEQL:
	case HI6_BNE:
	case HI6_BNEL:
	case HI6_BGTZ:
	case HI6_BGTZL:
	case HI6_BLEZ:
	case HI6_BLEZL:
		rs = ((instr[3] & 3) << 3) + ((instr[2] >> 5) & 7);
		rt = instr[2] & 31;
		imm = (instr[1] << 8) + instr[0];
		if (imm >= 32768)
			imm -= 65536;
		addr = (dumpaddr + 4) + (imm << 2);

		if (hi6 == HI6_BEQ && rt == MIPS_GPR_ZERO &&
		    rs == MIPS_GPR_ZERO)
			debug("b\t");
		else {
			debug("%s\t", hi6_names[hi6]);
			switch (hi6) {
			case HI6_BEQ:
			case HI6_BEQL:
			case HI6_BNE:
			case HI6_BNEL:
				debug("%s,", regnames[rt]);
			}
			debug("%s,", regnames[rs]);
		}

		if (cpu->is_32bit)
			debug("0x%08"PRIx32, (uint32_t)addr);
		else
			debug("0x%016"PRIx64, (uint64_t)addr);

		symbol = get_symbol_name(&cpu->machine->symbol_context,
		    addr, &offset);
		if (symbol != NULL && offset != addr)
			debug("\t<%s>", symbol);
		break;
	case HI6_ADDI:
	case HI6_ADDIU:
	case HI6_DADDI:
	case HI6_DADDIU:
	case HI6_SLTI:
	case HI6_SLTIU:
	case HI6_ANDI:
	case HI6_ORI:
	case HI6_XORI:
		rs = ((instr[3] & 3) << 3) + ((instr[2] >> 5) & 7);
		rt = instr[2] & 31;
		imm = (instr[1] << 8) + instr[0];
		if (imm >= 32768)
			imm -= 65536;
		debug("%s\t%s,", hi6_names[hi6], regnames[rt]);
		debug("%s,", regnames[rs]);
		if (hi6 == HI6_ANDI || hi6 == HI6_ORI || hi6 == HI6_XORI)
			debug("0x%04x", imm & 0xffff);
		else
			debug("%i", imm);
		break;
	case HI6_LUI:
		rt = instr[2] & 31;
		imm = (instr[1] << 8) + instr[0];
		debug("lui\t%s,0x%x", regnames[rt], imm);
		break;
	case HI6_LB:
	case HI6_LBU:
	case HI6_LH:
	case HI6_LHU:
	case HI6_LW:
	case HI6_LWU:
	case HI6_LD:
	case HI6_LQ_MDMX:
	case HI6_LWC1:
	case HI6_LWC2:
	case HI6_LWC3:
	case HI6_LDC1:
	case HI6_LDC2:
	case HI6_LL:
	case HI6_LLD:
	case HI6_SB:
	case HI6_SH:
	case HI6_SW:
	case HI6_SD:
	case HI6_SQ_SPECIAL3:
	case HI6_SC:
	case HI6_SCD:
	case HI6_SWC1:
	case HI6_SWC2:
	case HI6_SWC3:
	case HI6_SDC1:
	case HI6_SDC2:
	case HI6_LWL:   
	case HI6_LWR:
	case HI6_LDL:
	case HI6_LDR:
	case HI6_SWL:
	case HI6_SWR:
	case HI6_SDL:
	case HI6_SDR:
		if (hi6 == HI6_LQ_MDMX &&
		    cpu->cd.mips.cpu_type.rev != MIPS_R5900) {
			debug("mdmx\t(UNIMPLEMENTED)");
			break;
		}
		if (hi6 == HI6_SQ_SPECIAL3 &&
		    cpu->cd.mips.cpu_type.rev != MIPS_R5900) {
			int msbd, lsb, sub10;
			special6 = instr[0] & 0x3f;
			rs = ((instr[3] & 3) << 3) + ((instr[2] >> 5) & 7);
			rt = instr[2] & 31;
			rd = msbd = (instr[1] >> 3) & 31;
			lsb = ((instr[1] & 7) << 2) | (instr[0] >> 6);
			sub10 = (rs << 5) | lsb;

			switch (special6) {

			case SPECIAL3_EXT:
			case SPECIAL3_DEXT:
			case SPECIAL3_DEXTM:
			case SPECIAL3_DEXTU:
				debug("%s", special3_names[special6]);
				if (special6 == SPECIAL3_DEXTM)
					msbd += 32;
				if (special6 == SPECIAL3_DEXTU)
					lsb += 32;
				debug("\t%s", regnames[rt]);
				debug(",%s", regnames[rs]);
				debug(",%i,%i", lsb, msbd + 1);
				break;

			case SPECIAL3_INS:
			case SPECIAL3_DINS:
			case SPECIAL3_DINSM:
			case SPECIAL3_DINSU:
				debug("%s", special3_names[special6]);
				if (special6 == SPECIAL3_DINSM)
					msbd += 32;
				if (special6 == SPECIAL3_DINSU) {
					lsb += 32;
					msbd += 32;
				}
				msbd -= lsb;
				debug("\t%s", regnames[rt]);
				debug(",%s", regnames[rs]);
				debug(",%i,%i", lsb, msbd + 1);
				break;

			case SPECIAL3_BSHFL:
				switch (sub10) {
				case BSHFL_WSBH:
				case BSHFL_SEB:
				case BSHFL_SEH:
					switch (sub10) {
					case BSHFL_WSBH: debug("wsbh"); break;
					case BSHFL_SEB:  debug("seb"); break;
					case BSHFL_SEH:  debug("seh"); break;
					}
					debug("\t%s", regnames[rd]);
					debug(",%s", regnames[rt]);
					break;
				default:debug("%s", special3_names[special6]);
					debug("\t(UNIMPLEMENTED)");
				}
				break;

			case SPECIAL3_DBSHFL:
				switch (sub10) {
				case BSHFL_DSBH:
				case BSHFL_DSHD:
					switch (sub10) {
					case BSHFL_DSBH: debug("dsbh"); break;
					case BSHFL_DSHD: debug("dshd"); break;
					}
					debug("\t%s", regnames[rd]);
					debug(",%s", regnames[rt]);
					break;
				default:debug("%s", special3_names[special6]);
					debug("\t(UNIMPLEMENTED)");
				}
				break;

			case SPECIAL3_RDHWR:
				debug("%s", special3_names[special6]);
				debug("\t%s", regnames[rt]);
				debug(",hwr%i", rd);
				break;

			default:debug("%s", special3_names[special6]);
				debug("\t(UNIMPLEMENTED)");
			}
			break;
		}

		rs = ((instr[3] & 3) << 3) + ((instr[2] >> 5) & 7);
		rt = instr[2] & 31;
		imm = (instr[1] << 8) + instr[0];
		if (imm >= 32768)
			imm -= 65536;
		symbol = get_symbol_name(&cpu->machine->symbol_context,
		    cpu->cd.mips.gpr[rs] + imm, &offset);

		/*  LWC3 is PREF in the newer ISA levels:  */
		/*  TODO: Which ISAs? IV? V? 32? 64?  */
		if (cpu->cd.mips.cpu_type.isa_level >= 4 && hi6 == HI6_LWC3) {
			debug("pref\t0x%x,%i(%s)",
			    rt, imm, regnames[rs]);

			if (running) {
				debug("\t[0x%016"PRIx64" = %s]",
				    (uint64_t)(cpu->cd.mips.gpr[rs] + imm));
				if (symbol != NULL)
					debug(" = %s", symbol);
				debug("]");
			}
			goto disasm_ret;
		}

		debug("%s\t", hi6_names[hi6]);

		if (hi6 == HI6_SWC1 || hi6 == HI6_SWC2 || hi6 == HI6_SWC3 ||
		    hi6 == HI6_SDC1 || hi6 == HI6_SDC2 ||
		    hi6 == HI6_LWC1 || hi6 == HI6_LWC2 || hi6 == HI6_LWC3 ||
		    hi6 == HI6_LDC1 || hi6 == HI6_LDC2)
			debug("r%i", rt);
		else
			debug("%s", regnames[rt]);

		debug(",%i(%s)", imm, regnames[rs]);

		if (running) {
			debug("\t[");

			if (cpu->is_32bit)
				debug("0x%08"PRIx32,
				    (uint32_t) (cpu->cd.mips.gpr[rs] + imm));
			else
				debug("0x%016"PRIx64,
				    (uint64_t) (cpu->cd.mips.gpr[rs] + imm));

			if (symbol != NULL)
				debug(" = %s", symbol);

			/*  TODO: In some cases, it is possible to peek into
			    memory, and display that data here, like for the
			    other emulation modes.  */

			debug("]");
		}
		break;

	case HI6_J:
	case HI6_JAL:
		imm = (((instr[3] & 3) << 24) + (instr[2] << 16) +
		    (instr[1] << 8) + instr[0]) << 2;
		addr = (dumpaddr + 4) & ~((1 << 28) - 1);
		addr |= imm;
		symbol = get_symbol_name(&cpu->machine->symbol_context,
		    addr, &offset);
		debug("%s\t0x", hi6_names[hi6]);
		if (cpu->is_32bit)
			debug("%08"PRIx32, (uint32_t) addr);
		else
			debug("%016"PRIx64, (uint64_t) addr);
		if (symbol != NULL)
			debug("\t<%s>", symbol);
		break;

	case HI6_COP0:
	case HI6_COP1:
	case HI6_COP2:
	case HI6_COP3:
		imm = (instr[3] << 24) + (instr[2] << 16) +
		     (instr[1] << 8) + instr[0];
		imm &= ((1 << 26) - 1);

		/*  Call coproc_function(), but ONLY disassembly, no exec:  */
		coproc_function(cpu, cpu->cd.mips.coproc[hi6 - HI6_COP0],
		    hi6 - HI6_COP0, imm, 1, running);
		return sizeof(instrword);

	case HI6_CACHE:
		rt   = ((instr[3] & 3) << 3) + (instr[2] >> 5); /*  base  */
		copz = instr[2] & 31;
		imm  = (instr[1] << 8) + instr[0];
		cache_op    = copz >> 2;
		which_cache = copz & 3;
		showtag = 0;
		debug("cache\t0x%02x,0x%04x(%s)", copz, imm, regnames[rt]);
		if (which_cache==0)	debug("  [ primary I-cache");
		if (which_cache==1)	debug("  [ primary D-cache");
		if (which_cache==2)	debug("  [ secondary I-cache");
		if (which_cache==3)	debug("  [ secondary D-cache");
		debug(", ");
		if (cache_op==0)	debug("index invalidate");
		if (cache_op==1)	debug("index load tag");
		if (cache_op==2)	debug("index store tag"), showtag=1;
		if (cache_op==3)	debug("create dirty exclusive");
		if (cache_op==4)	debug("hit invalidate");
		if (cache_op==5)     debug("fill OR hit writeback invalidate");
		if (cache_op==6)	debug("hit writeback");
		if (cache_op==7)	debug("hit set virtual");
		if (running)
			debug(", addr 0x%016"PRIx64,
			    (uint64_t)(cpu->cd.mips.gpr[rt] + imm));
		if (showtag)
		debug(", taghi=%08lx lo=%08lx",
		    (long)cpu->cd.mips.coproc[0]->reg[COP0_TAGDATA_HI],
		    (long)cpu->cd.mips.coproc[0]->reg[COP0_TAGDATA_LO]);
		debug(" ]");
		break;

	case HI6_SPECIAL2:
		special6 = instr[0] & 0x3f;
		instrword = (instr[3] << 24) + (instr[2] << 16) +
		    (instr[1] << 8) + instr[0];
		rs = ((instr[3] & 3) << 3) + ((instr[2] >> 5) & 7);
		rt = instr[2] & 31;
		rd = (instr[1] >> 3) & 31;

		if (cpu->cd.mips.cpu_type.rev == MIPS_R5900) {
			int c790mmifunc = (instrword >> 6) & 0x1f;
			if (special6 != MMI_MMI0 && special6 != MMI_MMI1 &&
			    special6 != MMI_MMI2 && special6 != MMI_MMI3)
				debug("%s\t", mmi_names[special6]);

			switch (special6) {

			case MMI_MADD:
			case MMI_MADDU:
				if (rd != MIPS_GPR_ZERO) {
					debug("%s,", regnames[rd]);
				}
				debug("%s,%s", regnames[rs], regnames[rt]);
				break;

			case MMI_MMI0:
				debug("%s\t", mmi0_names[c790mmifunc]);
				switch (c790mmifunc) {

				case MMI0_PEXTLB:
				case MMI0_PEXTLH:
				case MMI0_PEXTLW:
				case MMI0_PMAXH:
				case MMI0_PMAXW:
				case MMI0_PPACB:
				case MMI0_PPACH:
				case MMI0_PPACW:
					debug("%s,%s,%s", regnames[rd],
					    regnames[rs], regnames[rt]);
					break;

				default:debug("(UNIMPLEMENTED)");
				}
				break;

			case MMI_MMI1:
				debug("%s\t", mmi1_names[c790mmifunc]);
				switch (c790mmifunc) {

				case MMI1_PEXTUB:
				case MMI1_PEXTUH:
				case MMI1_PEXTUW:
				case MMI1_PMINH:
				case MMI1_PMINW:
					debug("%s,%s,%s", regnames[rd],
					    regnames[rs], regnames[rt]);
					break;

				default:debug("(UNIMPLEMENTED)");
				}
				break;

			case MMI_MMI2:
				debug("%s\t", mmi2_names[c790mmifunc]);
				switch (c790mmifunc) {

				case MMI2_PMFHI:
				case MMI2_PMFLO:
					debug("%s", regnames[rd]);
					break;

				case MMI2_PHMADH:
				case MMI2_PHMSBH:
				case MMI2_PINTH:
				case MMI2_PMADDH:
				case MMI2_PMADDW:
				case MMI2_PMSUBH:
				case MMI2_PMSUBW:
				case MMI2_PMULTH:
				case MMI2_PMULTW:
				case MMI2_PSLLVW:
					debug("%s,%s,%s", regnames[rd],
					    regnames[rs], regnames[rt]);
					break;

				default:debug("(UNIMPLEMENTED)");
				}
				break;

			case MMI_MMI3:
				debug("%s\t", mmi3_names[c790mmifunc]);
				switch (c790mmifunc) {

				case MMI3_PMTHI:
				case MMI3_PMTLO:
					debug("%s", regnames[rs]);
					break;

				case MMI3_PINTEH:
				case MMI3_PMADDUW:
				case MMI3_PMULTUW:
				case MMI3_PNOR:
				case MMI3_POR:
				case MMI3_PSRAVW:
					debug("%s,%s,%s", regnames[rd],
					    regnames[rs], regnames[rt]);
					break;

				default:debug("(UNIMPLEMENTED)");
				}
				break;

			default:debug("(UNIMPLEMENTED)");
			}
			break;
		}

		/*  SPECIAL2:  */
		debug("%s\t", special2_names[special6]);

		switch (special6) {

		case SPECIAL2_MADD:
		case SPECIAL2_MADDU:
		case SPECIAL2_MSUB:
		case SPECIAL2_MSUBU:
			if (rd != MIPS_GPR_ZERO) {
				debug("WEIRD_NONZERO_RD(%s),",
				    regnames[rd]);
			}
			debug("%s,%s", regnames[rs], regnames[rt]);
			break;

		case SPECIAL2_MUL:
			/*  Apparently used both on R5900 and MIPS32:  */
			debug("%s,%s,%s", regnames[rd],
			    regnames[rs], regnames[rt]);
			break;

		case SPECIAL2_CLZ:
		case SPECIAL2_CLO:
		case SPECIAL2_DCLZ:
		case SPECIAL2_DCLO:
			debug("%s,%s", regnames[rd], regnames[rs]);
			break;

		default:
			debug("(UNIMPLEMENTED)");
		}
		break;

	case HI6_REGIMM:
		regimm5 = instr[2] & 0x1f;
		rs = ((instr[3] & 3) << 3) + ((instr[2] >> 5) & 7);
		imm = (instr[1] << 8) + instr[0];
		if (imm >= 32768)               
			imm -= 65536;

		switch (regimm5) {

		case REGIMM_BLTZ:
		case REGIMM_BGEZ:
		case REGIMM_BLTZL:
		case REGIMM_BGEZL:
		case REGIMM_BLTZAL:
		case REGIMM_BLTZALL:
		case REGIMM_BGEZAL:
		case REGIMM_BGEZALL:
			debug("%s\t%s,", regimm_names[regimm5], regnames[rs]);

			addr = (dumpaddr + 4) + (imm << 2);

			if (cpu->is_32bit)
				debug("0x%08"PRIx32, (uint32_t) addr);
			else
				debug("0x%016"PRIx64, (uint64_t) addr);
			break;

		case REGIMM_SYNCI:
			debug("%s\t%i(%s)", regimm_names[regimm5],
			    imm, regnames[rs]);
			break;

		default:
			debug("unimplemented regimm5 = 0x%02x", regimm5);
		}
		break;
	default:
		debug("unimplemented hi6 = 0x%02x", hi6);
	}

disasm_ret:
	debug("\n");
	return sizeof(instrword);
}


/*
 *  mips_cpu_register_dump():
 *
 *  Dump cpu registers in a relatively readable format.
 *
 *  gprs: set to non-zero to dump GPRs and hi/lo/pc
 *  coprocs: set bit 0..3 to dump registers in coproc 0..3.
 */
void mips_cpu_register_dump(struct cpu *cpu, int gprs, int coprocs)
{
	int coprocnr, i, bits32;
	uint64_t offset;
	char *symbol;
	int bits128 = cpu->cd.mips.cpu_type.rev == MIPS_R5900;

	bits32 = cpu->is_32bit;

	if (gprs) {
		/*  Special registers (pc, hi/lo) first:  */
		symbol = get_symbol_name(&cpu->machine->symbol_context,
		    cpu->pc, &offset);

		if (bits32)
			debug("cpu%i:  pc = %08"PRIx32,
			    cpu->cpu_id, (uint32_t) cpu->pc);
		else if (bits128)
			debug("cpu%i:  pc=%016"PRIx64,
			    cpu->cpu_id, (uint64_t) cpu->pc);
		else
			debug("cpu%i:    pc = 0x%016"PRIx64,
			    cpu->cpu_id, (uint64_t) cpu->pc);

		debug("    <%s>\n", symbol != NULL? symbol :
		    " no symbol ");

		if (bits32)
			debug("cpu%i:  hi = %08"PRIx32"  lo = %08"PRIx32"\n",
			    cpu->cpu_id, (uint32_t) cpu->cd.mips.hi,
			    (uint32_t) cpu->cd.mips.lo);
		else if (bits128) {
			debug("cpu%i:  hi=%016"PRIx64"%016"PRIx64"  lo="
			    "%016"PRIx64"%016"PRIx64"\n", cpu->cpu_id,
			    cpu->cd.mips.hi1, cpu->cd.mips.hi,
			    cpu->cd.mips.lo1, cpu->cd.mips.lo);
		} else {
			debug("cpu%i:    hi = 0x%016"PRIx64"    lo = 0x%016"
			    PRIx64"\n", cpu->cpu_id,
			    (uint64_t) cpu->cd.mips.hi,
			    (uint64_t) cpu->cd.mips.lo);
		}

		/*  General registers:  */
		if (bits128) {
			/*  128-bit:  */
			for (i=0; i<32; i++) {
				int r = (i >> 1) + ((i & 1) << 4);
				if ((i & 1) == 0)
					debug("cpu%i:", cpu->cpu_id);
				if (r == MIPS_GPR_ZERO)
					debug("                           "
					    "          ");
				else
					debug(" %3s=%016"PRIx64"%016"PRIx64,
					    regnames[r], (uint64_t)
					    cpu->cd.mips.gpr_quadhi[r],
					    (uint64_t)cpu->cd.mips.gpr[r]);
				if ((i & 1) == 1)
					debug("\n");
			}
		} else if (bits32) {
			/*  32-bit:  */
			for (i=0; i<32; i++) {
				if ((i & 3) == 0)
					debug("cpu%i:", cpu->cpu_id);
				if (i == MIPS_GPR_ZERO)
					debug("               ");
				else
					debug(" %3s = %08"PRIx32, regnames[i],
					    (uint32_t)cpu->cd.mips.gpr[i]);
				if ((i & 3) == 3)
					debug("\n");
			}
		} else {
			/*  64-bit:  */
			for (i=0; i<32; i++) {
				int r = (i >> 1) + ((i & 1) << 4);
				if ((i & 1) == 0)
					debug("cpu%i:", cpu->cpu_id);
				if (r == MIPS_GPR_ZERO)
					debug("                           ");
				else
					debug("   %3s = 0x%016"PRIx64,
					    regnames[r],
					    (uint64_t)cpu->cd.mips.gpr[r]);
				if ((i & 1) == 1)
					debug("\n");
			}
		}
	}

	for (coprocnr=0; coprocnr<4; coprocnr++) {
		int nm1 = 1;

		if (bits32)
			nm1 = 3;

		if (!(coprocs & (1<<coprocnr)))
			continue;
		if (cpu->cd.mips.coproc[coprocnr] == NULL) {
			debug("cpu%i: no coprocessor %i\n",
			    cpu->cpu_id, coprocnr);
			continue;
		}

		/*  Coprocessor registers:  */
		for (i=0; i<32; i++) {
			/*  32-bit:  */
			if ((i & nm1) == 0)
				debug("cpu%i:", cpu->cpu_id);

			if (coprocnr == 0)
				debug(" %8s", cop0_names[i]);
			else
				debug(" c%i,%02i", coprocnr, i);

			if (bits32)
				debug("=%08x", (int)cpu->cd.mips.
				    coproc[coprocnr]->reg[i]);
			else {
				if (coprocnr == 0 && (i == COP0_COUNT
				    || i == COP0_COMPARE || i == COP0_INDEX
				    || i == COP0_RANDOM || i == COP0_WIRED))
					debug(" =         0x%08x",
					    (int) cpu->cd.mips.coproc[
					    coprocnr]->reg[i]);
				else
					debug(" = 0x%016"PRIx64, (uint64_t)
					    cpu->cd.mips.coproc[
					    coprocnr]->reg[i]);
			}

			if ((i & nm1) == nm1)
				debug("\n");

			/*  Skip the last 16 cop0 registers on R3000 etc.  */
			if (coprocnr == 0 && cpu->cd.mips.cpu_type.isa_level < 3
			    && i == 15)
				i = 31;
		}

		if (coprocnr == 0 && cpu->cd.mips.cpu_type.isa_level >= 32) {
			debug("cpu%i: ", cpu->cpu_id);
			debug("config_select1 = 0x");
			if (cpu->is_32bit)
				debug("%08"PRIx32,
				    (uint32_t)cpu->cd.mips.cop0_config_select1);
			else
				debug("%016"PRIx64,
				    (uint64_t)cpu->cd.mips.cop0_config_select1);
			debug("\n");
		}

		/*  Floating point control registers:  */
		if (coprocnr == 1) {
			for (i=0; i<32; i++)
				switch (i) {
				case MIPS_FPU_FCIR:
					printf("cpu%i: fcr0  (fcir) = 0x%08x\n",
					    cpu->cpu_id, (int)cpu->cd.mips.
					    coproc[coprocnr]->fcr[i]);
					break;
				case MIPS_FPU_FCCR:
					printf("cpu%i: fcr25 (fccr) = 0x%08x\n",
					    cpu->cpu_id, (int)cpu->cd.mips.
					    coproc[coprocnr]->fcr[i]);
					break;
				case MIPS_FPU_FCSR:
					printf("cpu%i: fcr31 (fcsr) = 0x%08x\n",
					    cpu->cpu_id, (int)cpu->cd.mips.
					    coproc[coprocnr]->fcr[i]);
					break;
				}
		}
	}

	if (cpu->cd.mips.rmw) {
		printf("cpu%i: Read-Modify-Write in progress, address "
		    "0x%016"PRIx64"\n", cpu->cpu_id, cpu->cd.mips.rmw_addr);
	}
}


/*
 *  mips_cpu_interrupt_assert(), mips_cpu_interrupt_deassert():
 *
 *  Assert or deassert a MIPS CPU interrupt by masking in or out bits
 *  in the CAUSE register of coprocessor 0.
 */
void mips_cpu_interrupt_assert(struct interrupt *interrupt)
{
	struct cpu *cpu = (struct cpu *) interrupt->extra;
	cpu->cd.mips.coproc[0]->reg[COP0_CAUSE] |= interrupt->line;
}
void mips_cpu_interrupt_deassert(struct interrupt *interrupt)
{
	struct cpu *cpu = (struct cpu *) interrupt->extra;
	cpu->cd.mips.coproc[0]->reg[COP0_CAUSE] &= ~interrupt->line;
}


/*
 *  mips_cpu_exception():
 *
 *  Cause an exception in a CPU.  This sets a couple of coprocessor 0
 *  registers, and the program counter.
 *
 *	exccode		the exception code
 *	tlb		set to non-zero if the exception handler at
 *			0x80000000 should be used. (normal = 0x80000180)
 *	vaddr		virtual address (for some exceptions)
 *	coproc_nr	coprocessor number (for some exceptions)
 *	vaddr_vpn2	vpn2 (for some exceptions)
 *	vaddr_asid	asid (for some exceptions)
 *	x_64		non-zero for 64-bit mode for R4000-style tlb misses
 */
void mips_cpu_exception(struct cpu *cpu, int exccode, int tlb, uint64_t vaddr,
	int coproc_nr, uint64_t vaddr_vpn2, int vaddr_asid, int x_64)
{
	uint64_t base;
	uint64_t *reg = &cpu->cd.mips.coproc[0]->reg[0];
	int exc_model = cpu->cd.mips.cpu_type.exc_model;

	if (cpu->is_halted) {
		/*
		 *  If the exception occurred on a 'wait' instruction, then let
		 *  the instruction following the wait instruction be the one
		 *  we continue at when the interrupt service routine returns.
		 */
		cpu->is_halted = 0;
		cpu->pc += sizeof(uint32_t);
	}

	if (!quiet_mode) {
		uint64_t offset;
		int x;
		char *symbol = get_symbol_name(&cpu->machine->symbol_context,
		    cpu->pc, &offset);

		debug("[ ");
		if (cpu->machine->ncpus > 1)
			debug("cpu%i: ", cpu->cpu_id);

		debug("exception %s%s",
		    exception_names[exccode], tlb? " <tlb>" : "");

		switch (exccode) {

		case EXCEPTION_INT:
			debug(" cause_im=0x%02x", (int)
			    ((reg[COP0_CAUSE] & CAUSE_IP_MASK)
			    >> CAUSE_IP_SHIFT));
			break;

		case EXCEPTION_SYS:
			debug(" v0=%i", (int)cpu->cd.mips.gpr[MIPS_GPR_V0]);
			for (x=0; x<4; x++) {
				int64_t d = cpu->cd.mips.gpr[MIPS_GPR_A0 + x];
				char strbuf[30];

				if (d > -256 && d < 256) {
					debug(" a%i=%i", x, (int)d);
				} else if (memory_points_to_string(cpu,
				    cpu->mem, d, 1)) {
					debug(" a%i=\"%s\"", x,
					    memory_conv_to_string(cpu, cpu->mem,
					    d, strbuf, sizeof(strbuf)));
				} else {
					if (cpu->is_32bit)
						debug(" a%i=0x%"PRIx32, x,
						    (uint32_t)d);
					else
						debug(" a%i=0x%"PRIx64, x,
						    (uint64_t)d);
				}
			}
			break;

		case EXCEPTION_CPU:
			debug(" coproc_nr=%i", coproc_nr);
			break;

		default:
			if (cpu->is_32bit)
				debug(" vaddr=0x%08x", (int)vaddr);
			else
				debug(" vaddr=0x%016"PRIx64, (uint64_t)vaddr);
		}

		if (cpu->is_32bit)
			debug(" pc=0x%08"PRIx32" ", (uint32_t)cpu->pc);
		else
			debug(" pc=0x%016"PRIx64" ", (uint64_t)cpu->pc);

		if (symbol != NULL)
			debug("<%s> ]\n", symbol);
		else
			debug("]\n");
	}

	if (tlb && vaddr < 0x1000) {
		uint64_t offset;
		char *symbol = get_symbol_name(&cpu->machine->symbol_context,
		    cpu->pc, &offset);
		fatal("[ ");
		if (cpu->machine->ncpus > 1)
			fatal("cpu%i: ", cpu->cpu_id);
		fatal("warning: LOW reference: vaddr=");
		if (cpu->is_32bit)
			fatal("0x%08"PRIx32, (uint32_t) vaddr);
		else
			fatal("0x%016"PRIx64, (uint64_t) vaddr);
		fatal(", exception %s, pc=", exception_names[exccode]);
		if (cpu->is_32bit)
			fatal("0x%08"PRIx32, (uint32_t) cpu->pc);
		else
			fatal("0x%016"PRIx64, (uint64_t)cpu->pc);
		fatal(" <%s> ]\n", symbol? symbol : "(no symbol)");
	}

	/*  Clear the exception code bits of the cause register...  */
	if (exc_model == EXC3K)
		reg[COP0_CAUSE] &= ~R2K3K_CAUSE_EXCCODE_MASK;
	else
		reg[COP0_CAUSE] &= ~CAUSE_EXCCODE_MASK;

	/*  ... and OR in the exception code:  */
	reg[COP0_CAUSE] |= (exccode << CAUSE_EXCCODE_SHIFT);

	/*  Always set CE (according to the R5000 manual):  */
	reg[COP0_CAUSE] &= ~CAUSE_CE_MASK;
	reg[COP0_CAUSE] |= (coproc_nr << CAUSE_CE_SHIFT);

	if (tlb || (exccode >= EXCEPTION_MOD && exccode <= EXCEPTION_ADES) ||
	    exccode == EXCEPTION_VCEI || exccode == EXCEPTION_VCED) {
		reg[COP0_BADVADDR] = vaddr;
		if (cpu->is_32bit)
			reg[COP0_BADVADDR] = (int32_t)reg[COP0_BADVADDR];

		if (exc_model == EXC3K) {
			reg[COP0_CONTEXT] &= ~R2K3K_CONTEXT_BADVPN_MASK;
			reg[COP0_CONTEXT] |= ((vaddr_vpn2 <<
			    R2K3K_CONTEXT_BADVPN_SHIFT) &
			    R2K3K_CONTEXT_BADVPN_MASK);

			reg[COP0_ENTRYHI] = (vaddr & R2K3K_ENTRYHI_VPN_MASK)
			    | (vaddr_asid << R2K3K_ENTRYHI_ASID_SHIFT);

			/*  Sign-extend:  */
			reg[COP0_CONTEXT] = (int64_t)(int32_t)reg[COP0_CONTEXT];
			reg[COP0_ENTRYHI] = (int64_t)(int32_t)reg[COP0_ENTRYHI];
		} else {
			if (cpu->cd.mips.cpu_type.rev == MIPS_R4100) {
				reg[COP0_CONTEXT] &=
				    ~CONTEXT_BADVPN2_MASK_R4100;
				reg[COP0_CONTEXT] |= ((vaddr_vpn2 <<
				    CONTEXT_BADVPN2_SHIFT) &
				    CONTEXT_BADVPN2_MASK_R4100);

				/*  TODO:  fix these  */
				reg[COP0_XCONTEXT] &= ~XCONTEXT_R_MASK;
				reg[COP0_XCONTEXT] &= ~XCONTEXT_BADVPN2_MASK;
				reg[COP0_XCONTEXT] |= (vaddr_vpn2 << XCONTEXT_BADVPN2_SHIFT) & XCONTEXT_BADVPN2_MASK;
				reg[COP0_XCONTEXT] |= ((vaddr >> 62) & 0x3) << XCONTEXT_R_SHIFT;

				/*  reg[COP0_PAGEMASK] = cpu->cd.mips.coproc[0]->tlbs[0].mask & PAGEMASK_MASK;  */

				reg[COP0_ENTRYHI] = (vaddr & (ENTRYHI_R_MASK | ENTRYHI_VPN2_MASK | 0x1800)) | vaddr_asid;
			} else {
				reg[COP0_CONTEXT] &= ~CONTEXT_BADVPN2_MASK;
				reg[COP0_CONTEXT] |= ((vaddr_vpn2 << CONTEXT_BADVPN2_SHIFT) & CONTEXT_BADVPN2_MASK);

				reg[COP0_XCONTEXT] &= ~XCONTEXT_R_MASK;
				reg[COP0_XCONTEXT] &= ~XCONTEXT_BADVPN2_MASK;
				reg[COP0_XCONTEXT] |= (vaddr_vpn2 << XCONTEXT_BADVPN2_SHIFT) & XCONTEXT_BADVPN2_MASK;
				reg[COP0_XCONTEXT] |= ((vaddr >> 62) & 0x3) << XCONTEXT_R_SHIFT;

				/*  reg[COP0_PAGEMASK] = cpu->cd.mips.coproc[0]->tlbs[0].mask & PAGEMASK_MASK;  */

				if (cpu->cd.mips.cpu_type.mmu_model == MMU10K)
					reg[COP0_ENTRYHI] = (vaddr & (ENTRYHI_R_MASK | ENTRYHI_VPN2_MASK_R10K)) | vaddr_asid;
				else
					reg[COP0_ENTRYHI] = (vaddr & (ENTRYHI_R_MASK | ENTRYHI_VPN2_MASK)) | vaddr_asid;
			}
		}
	}

	if (exc_model != EXC3K && reg[COP0_STATUS] & STATUS_EXL) {
		/*
		 *  Don't set EPC if STATUS_EXL is set, for R4000 and up.
		 *  This actually happens when running IRIX and Ultrix, when
		 *  they handle interrupts and/or tlb updates, I think, so
		 *  printing this with debug() looks better than with fatal().
		 */
		/*  debug("[ warning: cpu%i exception while EXL is set,"
		    " not setting EPC ]\n", cpu->cpu_id);  */
	} else {
		if (cpu->delay_slot) {
			reg[COP0_EPC] = cpu->pc - 4;
			reg[COP0_CAUSE] |= CAUSE_BD;
		} else {
			reg[COP0_EPC] = cpu->pc;
			reg[COP0_CAUSE] &= ~CAUSE_BD;
		}
	}

	if (cpu->delay_slot)
		cpu->delay_slot = EXCEPTION_IN_DELAY_SLOT;
	else
		cpu->delay_slot = NOT_DELAYED;

	/*  TODO: This is true for MIPS64, but how about others?  */
	if (reg[COP0_STATUS] & STATUS_BEV)
		base = 0xffffffffbfc00200ULL;
	else
		base = 0xffffffff80000000ULL;

	switch (exc_model) {
	case EXC3K:
		/*  Userspace tlb, vs others:  */
		if (tlb && !(vaddr & 0x80000000ULL) &&
		    (exccode == EXCEPTION_TLBL || exccode == EXCEPTION_TLBS) )
			cpu->pc = base + 0x000;
		else
			cpu->pc = base + 0x080;
		break;
	default:
		/*
		 *  These offsets are according to the MIPS64 manual, but
		 *  should work with R4000 and the rest too (I hope).
		 *
		 *  0x000  TLB refill, if EXL=0
		 *  0x080  64-bit XTLB refill, if EXL=0
		 *  0x100  cache error  (not implemented yet)
		 *  0x180  general exception
		 *  0x200  interrupt (if CAUSE_IV is set)
		 */
		if (tlb && (exccode == EXCEPTION_TLBL ||
		    exccode == EXCEPTION_TLBS) &&
		    !(reg[COP0_STATUS] & STATUS_EXL)) {
			if (x_64)
				cpu->pc = base + 0x080;
			else
				cpu->pc = base + 0x000;
		} else {
			if (exccode == EXCEPTION_INT &&
			    (reg[COP0_CAUSE] & CAUSE_IV))
				cpu->pc = base + 0x200;
			else
				cpu->pc = base + 0x180;
		}
	}

	if (exc_model == EXC3K) {
		/*  R{2,3}000:  Shift the lowest 6 bits to the left two steps:*/
		reg[COP0_STATUS] = (reg[COP0_STATUS] & ~0x3f) +
		    ((reg[COP0_STATUS] & 0xf) << 2);
	} else {
		/*  R4000:  */
		reg[COP0_STATUS] |= STATUS_EXL;
	}

	/*  Sign-extend:  */
	reg[COP0_CAUSE] = (int64_t)(int32_t)reg[COP0_CAUSE];
	reg[COP0_STATUS] = (int64_t)(int32_t)reg[COP0_STATUS];

	if (cpu->is_32bit) {
		reg[COP0_EPC] = (int64_t)(int32_t)reg[COP0_EPC];
		mips32_pc_to_pointers(cpu);
	} else {
		mips_pc_to_pointers(cpu);
	}
}


#include "memory_mips.cc"


#include "tmp_mips_tail.cc"

