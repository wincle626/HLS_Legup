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
 *  Emulation of MIPS coprocessors.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "cop0.h"
#include "cpu.h"
#include "cpu_mips.h"
#include "emul.h"
#include "float_emul.h"
#include "machine.h"
#include "memory.h"
#include "mips_cpu_types.h"
#include "misc.h"
#include "opcodes_mips.h"
#include "timer.h"


extern volatile int single_step;

static const char *cop0_names[] = COP0_NAMES;
static const char *regnames[] = MIPS_REGISTER_NAMES;


/*
 *  initialize_cop0_config():
 *
 *  Helper function, called from mips_coproc_new().
 */
static void initialize_cop0_config(struct cpu *cpu, struct mips_coproc *c)
{
	const int m16 = 0;	/*  TODO: MIPS16 support  */
	int IB, DB, SB, IC, DC, SC, IA, DA;

	/*  Generic case for MIPS32/64:  */
	if (cpu->cd.mips.cpu_type.isa_level == 32 ||
	    cpu->cd.mips.cpu_type.isa_level == 64) {
		/*  According to the MIPS64 (5K) User's Manual:  */
		c->reg[COP0_CONFIG] =
		      (   (uint32_t)1 << 31)/*  Config 1 present bit  */
		    | (   0 << 20)	/*  ISD:  instruction scheduling
					    disable (=1)  */
		    | (   0 << 17)	/*  DID:  dual issue disable  */
		    | (   0 << 16)	/*  BM:   burst mode  */
		    | ((cpu->byte_order == EMUL_BIG_ENDIAN? 1 : 0) << 15)
				 	/*  endian mode  */
		    | ((cpu->cd.mips.cpu_type.isa_level == 64? 2 : 0) << 13)
					/*  0=MIPS32, 1=64S, 2=64  */
		    | (   0 << 10)	/*  Architecture revision  */
		    | (   1 <<  7)	/*  MMU type: 1=TLB, 3=FMT  */
		    | (   2 <<  0)	/*  kseg0 cache coherency algorithm  */
		    ;
		/*  Config select 1: caches etc. TODO: Don't use
			cpu->machine for this stuff!  */
		IB = cpu->cd.mips.cache_picache_linesize - 1;
		IB = IB < 0? 0 : (IB > 7? 7 : IB);
		DB = cpu->cd.mips.cache_pdcache_linesize - 1;
		DB = DB < 0? 0 : (DB > 7? 7 : DB);
		IC = cpu->cd.mips.cache_picache -
		    cpu->cd.mips.cache_picache_linesize - 7;
		DC = cpu->cd.mips.cache_pdcache -
		    cpu->cd.mips.cache_pdcache_linesize - 7;
		IA = cpu->cd.mips.cpu_type.piways - 1;
		DA = cpu->cd.mips.cpu_type.pdways - 1;
		cpu->cd.mips.cop0_config_select1 =
		    ((cpu->cd.mips.cpu_type.nr_of_tlb_entries - 1) << 25)
		    | (IC << 22)	/*  IS: I-cache sets per way  */
		    | (IB << 19)	/*  IL: I-cache line-size  */
		    | (IA << 16)	/*  IA: I-cache assoc. (ways-1)  */
		    | (DC << 13)	/*  DS: D-cache sets per way  */
		    | (DB << 10)	/*  DL: D-cache line-size  */
		    | (DA <<  7)	/*  DA: D-cache assoc. (ways-1)  */
		    | (16 * 0)		/*  Existance of PerformanceCounters  */
		    | ( 8 * 0)		/*  Existance of Watch Registers  */
		    | ( 4 * m16)	/*  Existance of MIPS16  */
		    | ( 2 * 0)		/*  Existance of EJTAG  */
		    | ( 1 * 1)		/*  Existance of FPU  */
		    ;

		return;
	}

	switch (cpu->cd.mips.cpu_type.rev) {
	case MIPS_R2000:
	case MIPS_R3000:
		/*  No config register.  */
		break;
	case MIPS_R4000:	/*  according to the R4000 manual  */
	case MIPS_R4600:
		IB = cpu->cd.mips.cache_picache_linesize - 4;
		IB = IB < 0? 0 : (IB > 1? 1 : IB);
		DB = cpu->cd.mips.cache_pdcache_linesize - 4;
		DB = DB < 0? 0 : (DB > 1? 1 : DB);
		SB = cpu->cd.mips.cache_secondary_linesize - 4;
		SB = SB < 0? 0 : (SB > 3? 3 : SB);
		IC = cpu->cd.mips.cache_picache - 12;
		IC = IC < 0? 0 : (IC > 7? 7 : IC);
		DC = cpu->cd.mips.cache_pdcache - 12;
		DC = DC < 0? 0 : (DC > 7? 7 : DC);
		SC = cpu->cd.mips.cache_secondary? 0 : 1;
		c->reg[COP0_CONFIG] =
		      (   0 << 31)	/*  Master/Checker present bit  */
		    | (0x00 << 28)	/*  EC: system clock divisor,
					    0x00 = '2'  */
		    | (0x00 << 24)	/*  EP  */
		    | (  SB << 22)	/*  SB  */
		    | (0x00 << 21)	/*  SS: 0 = mixed i/d scache  */
		    | (0x00 << 20)	/*  SW  */
		    | (0x00 << 18)	/*  EW: 0=64-bit  */
		    | (  SC << 17)	/*  SC: 0=secondary cache present,
					    1=non-present  */
		    | (0x00 << 16)	/*  SM: (todo)  */
		    | ((cpu->byte_order==EMUL_BIG_ENDIAN? 1 : 0) << 15)
				 	/*  endian mode  */
		    | (0x01 << 14)	/*  ECC: 0=enabled, 1=disabled  */
		    | (0x00 << 13)	/*  EB: (todo)  */
		    | (0x00 << 12)	/*  0 (resered)  */
		    | (  IC <<  9)	/*  IC: I-cache = 2^(12+IC) bytes
					    (1 = 8KB, 4=64K)  */
		    | (  DC <<  6)	/*  DC: D-cache = 2^(12+DC) bytes
					    (1 = 8KB, 4=64K)  */
		    | (  IB <<  5)	/*  IB: I-cache line size (0=16,
					    1=32)  */
		    | (  DB <<  4)	/*  DB: D-cache line size (0=16,
					    1=32)  */
		    | (   0 <<  3)	/*  CU: todo  */
		    | (   0 <<  0)	/*  kseg0 coherency algorithm
						(TODO)  */
		    ;
		break;
	case MIPS_R4100:	/*  According to the VR4131 manual:  */
		IB = cpu->cd.mips.cache_picache_linesize - 4;
		IB = IB < 0? 0 : (IB > 1? 1 : IB);
		DB = cpu->cd.mips.cache_pdcache_linesize - 4;
		DB = DB < 0? 0 : (DB > 1? 1 : DB);
		IC = cpu->cd.mips.cache_picache - 10;
		IC = IC < 0? 0 : (IC > 7? 7 : IC);
		DC = cpu->cd.mips.cache_pdcache - 10;
		DC = DC < 0? 0 : (DC > 7? 7 : DC);
		c->reg[COP0_CONFIG] =
		      (   0 << 31)	/*  IS: Instruction Streaming bit  */
		    | (0x01 << 28)	/*  EC: system clock divisor,
					    0x01 = 2  */
		    | (0x00 << 24)	/*  EP  */
		    | (0x00 << 23)	/*  AD: Accelerate data mode
					    (0=VR4000-compatible)  */
		    | ( m16 << 20)	/*  M16: MIPS16 support  */
		    | (   1 << 17)	/*  '1'  */
		    | (0x00 << 16)	/*  BP: 'Branch forecast'
					    (0 = enabled)  */
		    | ((cpu->byte_order==EMUL_BIG_ENDIAN? 1 : 0) << 15)
				 	/*  endian mode  */
		    | (   2 << 13)	/*  '2' hardcoded on VR4131  */
		    | (   1 << 12)	/*  CS: Cache size mode
					    (1 on VR4131)  */
		    | (  IC <<  9)	/*  IC: I-cache = 2^(10+IC) bytes
					    (0 = 1KB, 4=16K)  */
		    | (  DC <<  6)	/*  DC: D-cache = 2^(10+DC) bytes
					    (0 = 1KB, 4=16K)  */
		    | (  IB <<  5)	/*  IB: I-cache line size (0=16,
					    1=32)  */
		    | (  DB <<  4)	/*  DB: D-cache line size (0=16,
					    1=32)  */
		    | (   0 <<  0)	/*  kseg0 coherency algorithm (TODO)  */
		    ;
		break;
	case MIPS_R5000:
	case MIPS_RM5200:	/*  rm5200 is just a wild guess  */
		/*  These are just guesses: (the comments are wrong) */
		c->reg[COP0_CONFIG] =
		      (   0 << 31)	/*  Master/Checker present bit  */
		    | (0x00 << 28)	/*  EC: system clock divisor,
					    0x00 = '2'  */
		    | (0x00 << 24)	/*  EP  */
		    | (0x00 << 22)	/*  SB  */
		    | (0x00 << 21)	/*  SS  */
		    | (0x00 << 20)	/*  SW  */
		    | (0x00 << 18)	/*  EW: 0=64-bit  */
		    | (0x01 << 17)	/*  SC: 0=secondary cache present,
					    1=non-present  */
		    | (0x00 << 16)	/*  SM: (todo)  */
		    | ((cpu->byte_order==EMUL_BIG_ENDIAN? 1 : 0) << 15)
				 	/*  endian mode  */
		    | (0x01 << 14)	/*  ECC: 0=enabled, 1=disabled  */
		    | (0x00 << 13)	/*  EB: (todo)  */
		    | (0x00 << 12)	/*  0 (resered)  */
		    | (   3 <<  9)	/*  IC: I-cache = 2^(12+IC) bytes
					    (1 = 8KB, 4=64K)  */
		    | (   3 <<  6)	/*  DC: D-cache = 2^(12+DC) bytes
					    (1 = 8KB, 4=64K)  */
		    | (   1 <<  5)	/*  IB: I-cache line size (0=16,
					    1=32)  */
		    | (   1 <<  4)	/*  DB: D-cache line size (0=16,
					    1=32)  */
		    | (   0 <<  3)	/*  CU: todo  */
		    | (   2 <<  0)	/*  kseg0 coherency algorithm
						(TODO)  */
		    ;
		break;
	case MIPS_R10000:
	case MIPS_R12000:
	case MIPS_R14000:
		IC = cpu->cd.mips.cache_picache - 12;
		IC = IC < 0? 0 : (IC > 7? 7 : IC);
		DC = cpu->cd.mips.cache_pdcache - 12;
		DC = DC < 0? 0 : (DC > 7? 7 : DC);
		SC = cpu->cd.mips.cache_secondary - 19;
		SC = SC < 0? 0 : (SC > 7? 7 : SC);
		/*  According to the R10000 User's Manual:  */
		c->reg[COP0_CONFIG] =
		      (  IC << 29)	/*  Primary instruction cache size
					    (3 = 32KB)  */
		    | (  DC << 26)	/*  Primary data cache size (3 =
					    32KB)  */
		    | (   0 << 19)	/*  SCClkDiv  */
		    | (  SC << 16)	/*  SCSize, secondary cache size.
					    0 = 512KB. powers of two  */
		    | (   0 << 15)	/*  MemEnd  */
		    | (   0 << 14)	/*  SCCorEn  */
		    | (   1 << 13)	/*  SCBlkSize. 0=16 words,
					    1=32 words  */
		    | (   0 <<  9)	/*  SysClkDiv  */
		    | (   0 <<  7)	/*  PrcReqMax  */
		    | (   0 <<  6)	/*  PrcElmReq  */
		    | (   0 <<  5)	/*  CohPrcReqTar  */
		    | (   0 <<  3)	/*  Device number  */
		    | (   2 <<  0)	/*  Cache coherency algorithm for
					    kseg0  */
		    ;
		break;
	case MIPS_R5900:
		/*
		 *  R5900 is supposed to have the following (according
		 *  to NetBSD/playstation2):
		 *	cpu0: 16KB/64B 2-way set-associative L1 Instruction
		 *		cache, 48 TLB entries
		 *	cpu0: 8KB/64B 2-way set-associative write-back L1
		 *		Data cache
		 *  The following settings are just guesses:
		 *  (comments are incorrect)
		 */
		c->reg[COP0_CONFIG] =
		      (   0 << 31)	/*  Master/Checker present bit  */
		    | (0x00 << 28)	/*  EC: system clock divisor,
					    0x00 = '2'  */
		    | (0x00 << 24)	/*  EP  */
		    | (0x00 << 22)	/*  SB  */
		    | (0x00 << 21)	/*  SS  */
		    | (0x00 << 20)	/*  SW  */
		    | (0x00 << 18)	/*  EW: 0=64-bit  */
		    | (0x01 << 17)	/*  SC: 0=secondary cache present,
					    1=non-present  */
		    | (0x00 << 16)	/*  SM: (todo)  */
		    | ((cpu->byte_order==EMUL_BIG_ENDIAN? 1 : 0) << 15)
				 	/*  endian mode  */
		    | (0x01 << 14)	/*  ECC: 0=enabled, 1=disabled  */
		    | (0x00 << 13)	/*  EB: (todo)  */
		    | (0x00 << 12)	/*  0 (resered)  */
		    | (   3 <<  9)	/*  IC: I-cache = 2^(12+IC) bytes
					    (1 = 8KB, 4=64K)  */
		    | (   3 <<  6)	/*  DC: D-cache = 2^(12+DC) bytes
					    (1 = 8KB, 4=64K)  */
		    | (   1 <<  5)	/*  IB: I-cache line size (0=16,
					    1=32)  */
		    | (   1 <<  4)	/*  DB: D-cache line size (0=16,
					    1=32)  */
		    | (   0 <<  3)	/*  CU: todo  */
		    | (   0 <<  0)	/*  kseg0 coherency algorithm
						(TODO)  */
		    ;
		break;
	default:fatal("Internal error: No initialization code for"
		    " config0? cpu rev = 0x%x", cpu->cd.mips.cpu_type.rev);
		exit(1);
	}
}


/*
 *  initialize_cop1():
 *
 *  Helper function, called from mips_coproc_new().
 */
static void initialize_cop1(struct cpu *cpu, struct mips_coproc *c)
{
	int fpu_rev;
	uint64_t other_stuff = 0;

	switch (cpu->cd.mips.cpu_type.rev & 0xff) {
	case MIPS_R2000:	fpu_rev = MIPS_R2010;	break;
	case MIPS_R3000:	fpu_rev = MIPS_R3010;
				other_stuff |= 0x40;	/*  or 0x30? TODO  */
				break;
	case MIPS_R6000:	fpu_rev = MIPS_R6010;	break;
	case MIPS_R4000:	fpu_rev = MIPS_R4010;	break;
	case MIPS_4Kc:		/*  TODO: Is this the same as 5Kc?  */
	case MIPS_5Kc:		other_stuff = COP1_REVISION_DOUBLE
				    | COP1_REVISION_SINGLE;
	case MIPS_R5000:
	case MIPS_RM5200:	fpu_rev = cpu->cd.mips.cpu_type.rev;
				other_stuff |= 0x10;
				/*  or cpu->cd.mips.cpu_type.sub ? TODO  */
				break;
	case MIPS_R10000:	fpu_rev = MIPS_R10000;	break;
	case MIPS_R12000:	fpu_rev = 0x9;	break;
	default:		fpu_rev = MIPS_SOFT;
	}

	c->fcr[COP1_REVISION] = (fpu_rev << 8) | other_stuff;

#if 0
	/*  These are mentioned in the MIPS64 documentation:  */
	    + (1 << 16)		/*  single  */
	    + (1 << 17)		/*  double  */
	    + (1 << 18)		/*  paired-single  */
	    + (1 << 19)		/*  3d  */
#endif
}


/*
 *  mips_coproc_new():
 *
 *  Create a new MIPS coprocessor object.
 */
struct mips_coproc *mips_coproc_new(struct cpu *cpu, int coproc_nr)
{
	struct mips_coproc *c;

	CHECK_ALLOCATION(c = (struct mips_coproc *) malloc(sizeof(struct mips_coproc)));
	memset(c, 0, sizeof(struct mips_coproc));

	c->coproc_nr = coproc_nr;

	if (coproc_nr == 0) {
		c->nr_of_tlbs = cpu->cd.mips.cpu_type.nr_of_tlb_entries;
		c->tlbs = (struct mips_tlb *) zeroed_alloc(c->nr_of_tlbs * sizeof(struct mips_tlb));

		/*
		 *  Start with nothing in the status register. This makes sure
		 *  that we are running in kernel mode with all interrupts
		 *  disabled.
		 */
		c->reg[COP0_STATUS] = 0;

		/*  Hm. Enable coprocessors 0 and 1 even if we're not just
		    emulating userland? TODO: Think about this.  */
		/*  if (cpu->machine->prom_emulation)  */
			c->reg[COP0_STATUS] |=
			    ((uint32_t)0x3 << STATUS_CU_SHIFT);

		if (!cpu->machine->prom_emulation)
			c->reg[COP0_STATUS] |= STATUS_BEV;

		/*  Ugly hack for R5900/TX79/C790:  */
		if (cpu->cd.mips.cpu_type.rev == MIPS_R5900)
			c->reg[COP0_STATUS] |= R5900_STATUS_EIE;

		/*  Default pagesize = 4 KB  (i.e. dualpage = 8KB)  */
		c->reg[COP0_PAGEMASK] = 0x1fff;

		/*  Note: .rev may contain the company ID as well!  */
		c->reg[COP0_PRID] =
		      (0x00 << 24)			/*  Company Options  */
		    | (0x00 << 16)			/*  Company ID       */
		    | (cpu->cd.mips.cpu_type.rev <<  8)	/*  Processor ID     */
		    | (cpu->cd.mips.cpu_type.sub)	/*  Revision         */
		    ;

		c->reg[COP0_WIRED] = 0;

		initialize_cop0_config(cpu, c);

		/*  Make sure the status register is sign-extended nicely:  */
		c->reg[COP0_STATUS] = (int32_t)c->reg[COP0_STATUS];
	}

	if (coproc_nr == 1)
		initialize_cop1(cpu, c);

	return c;
}


/*
 *  mips_timer_tick():
 */
static void mips_timer_tick(struct timer *timer, void *extra)
{
	struct cpu *cpu = (struct cpu *) extra;

	cpu->cd.mips.compare_interrupts_pending ++;

	if ((int32_t) (cpu->cd.mips.coproc[0]->reg[COP0_COUNT] -
	    cpu->cd.mips.coproc[0]->reg[COP0_COMPARE]) < 0) {
		cpu->cd.mips.coproc[0]->reg[COP0_COUNT] =
		    cpu->cd.mips.coproc[0]->reg[COP0_COMPARE];
	}
}


/*
 *  mips_coproc_tlb_set_entry():
 *
 *  Used by machine setup code, if a specific machine emulation starts up
 *  with hardcoded virtual to physical mappings.
 */
void mips_coproc_tlb_set_entry(struct cpu *cpu, int entrynr, int size,
	uint64_t vaddr, uint64_t paddr0, uint64_t paddr1,
	int valid0, int valid1, int dirty0, int dirty1, int global, int asid,
	int cachealgo0, int cachealgo1)
{
	if (entrynr < 0 || entrynr >= cpu->cd.mips.coproc[0]->nr_of_tlbs) {
		printf("mips_coproc_tlb_set_entry(): invalid entry nr: %i\n",
		    entrynr);
		exit(1);
	}

	switch (cpu->cd.mips.cpu_type.mmu_model) {
	case MMU3K:
		if (size != 4096) {
			printf("mips_coproc_tlb_set_entry(): invalid pagesize "
			    "(%i) for MMU3K\n", size);
			exit(1);
		}
		cpu->cd.mips.coproc[0]->tlbs[entrynr].hi =
		    (vaddr & R2K3K_ENTRYHI_VPN_MASK) |
		    ((asid << R2K3K_ENTRYHI_ASID_SHIFT) & 
		    R2K3K_ENTRYHI_ASID_MASK);
		cpu->cd.mips.coproc[0]->tlbs[entrynr].lo0 =
		    (paddr0 & R2K3K_ENTRYLO_PFN_MASK) |
		    (cachealgo0? R2K3K_ENTRYLO_N : 0) |
		    (dirty0? R2K3K_ENTRYLO_D : 0) |
		    (valid0? R2K3K_ENTRYLO_V : 0) |
		    (global? R2K3K_ENTRYLO_G : 0);
		break;
	default:
		/*  MMU4K and MMU10K, etc:  */
		if (cpu->cd.mips.cpu_type.mmu_model == MMU10K)
			cpu->cd.mips.coproc[0]->tlbs[entrynr].hi =
			    (vaddr & ENTRYHI_VPN2_MASK_R10K) |
			    (vaddr & ENTRYHI_R_MASK) |
			    (asid & ENTRYHI_ASID) |
			    (global? TLB_G : 0);
		else
			cpu->cd.mips.coproc[0]->tlbs[entrynr].hi =
			    (vaddr & ENTRYHI_VPN2_MASK) |
			    (vaddr & ENTRYHI_R_MASK) |
			    (asid & ENTRYHI_ASID) |
			    (global? TLB_G : 0);
		/*  NOTE: The pagemask size is for a "dual" page:  */
		cpu->cd.mips.coproc[0]->tlbs[entrynr].mask =
		    (2*size - 1) & ~0x1fff;
		cpu->cd.mips.coproc[0]->tlbs[entrynr].lo0 =
		    (((paddr0 >> 12) << ENTRYLO_PFN_SHIFT) &
			ENTRYLO_PFN_MASK) |
		    (dirty0? ENTRYLO_D : 0) |
		    (valid0? ENTRYLO_V : 0) |
		    (global? ENTRYLO_G : 0) |
		    ((cachealgo0 << ENTRYLO_C_SHIFT) & ENTRYLO_C_MASK);
		cpu->cd.mips.coproc[0]->tlbs[entrynr].lo1 =
		    (((paddr1 >> 12) << ENTRYLO_PFN_SHIFT) &
			ENTRYLO_PFN_MASK) |
		    (dirty1? ENTRYLO_D : 0) |
		    (valid1? ENTRYLO_V : 0) |
		    (global? ENTRYLO_G : 0) |
		    ((cachealgo1 << ENTRYLO_C_SHIFT) & ENTRYLO_C_MASK);
		/*  TODO: R4100, 1KB pages etc  */
	}
}


/*
 *  invalidate_asid():
 *
 *  Go through all entries in the TLB. If an entry has a matching asid, is
 *  valid, and is not global (i.e. the ASID matters), then its virtual address
 *  translation is invalidated.
 *
 *  Note: In the R3000 case, the asid argument is shifted 6 bits.
 */
static void invalidate_asid(struct cpu *cpu, unsigned int asid)
{
	struct mips_coproc *cp = cpu->cd.mips.coproc[0];
	unsigned int i, ntlbs = cp->nr_of_tlbs;
	struct mips_tlb *tlb = cp->tlbs;

	if (cpu->cd.mips.cpu_type.mmu_model == MMU3K) {
		for (i=0; i<ntlbs; i++)
			if ((tlb[i].hi & R2K3K_ENTRYHI_ASID_MASK) == asid
			    && (tlb[i].lo0 & R2K3K_ENTRYLO_V)
			    && !(tlb[i].lo0 & R2K3K_ENTRYLO_G)) {
				cpu->invalidate_translation_caches(cpu,
				    tlb[i].hi & R2K3K_ENTRYHI_VPN_MASK,
				    INVALIDATE_VADDR);
			}
	} else {
		int non4kpages = 0;
		uint64_t topbit = 1, fillmask = 0xffffff0000000000ULL;

		if (cpu->is_32bit) {
			topbit = 0x80000000;
			fillmask = 0xffffffff00000000ULL;
		} else if (cpu->cd.mips.cpu_type.mmu_model == MMU10K) {
			topbit <<= 43;
			fillmask <<= 4;
		} else {
			topbit <<= 39;
		}

		for (i=0; i<ntlbs; i++) {
			if (tlb[i].mask != 0 && tlb[i].mask != 0x1800) {
				non4kpages = 1;
				continue;
			}

			if ((tlb[i].hi & ENTRYHI_ASID) == asid &&
			    !(tlb[i].hi & TLB_G)) {
				uint64_t vaddr0, vaddr1;
				vaddr0 = cp->tlbs[i].hi & ~fillmask;
				if (vaddr0 & topbit)
					vaddr0 |= fillmask;
				vaddr1 = vaddr0 | 0x1000;  /*  TODO: mask  */

				if (tlb[i].lo0 & ENTRYLO_V)
					cpu->invalidate_translation_caches(cpu,
					    vaddr0, INVALIDATE_VADDR);
				if (tlb[i].lo1 & ENTRYLO_V)
					cpu->invalidate_translation_caches(cpu,
					    vaddr1, INVALIDATE_VADDR);
			}
		}

		if (non4kpages) {
			cpu->invalidate_translation_caches(cpu,
			    0, INVALIDATE_ALL);
		}
	}
}


/*
 *  coproc_register_read();
 *
 *  Read a value from a MIPS coprocessor register.
 */
void coproc_register_read(struct cpu *cpu,
	struct mips_coproc *cp, int reg_nr, uint64_t *ptr, int select)
{
	int unimpl = 1;

	if (cp->coproc_nr==0 && reg_nr==COP0_INDEX)	unimpl = 0;
	if (cp->coproc_nr==0 && reg_nr==COP0_RANDOM)	unimpl = 0;
	if (cp->coproc_nr==0 && reg_nr==COP0_ENTRYLO0)	unimpl = 0;
	if (cp->coproc_nr==0 && reg_nr==COP0_ENTRYLO1)	unimpl = 0;
	if (cp->coproc_nr==0 && reg_nr==COP0_CONTEXT)	unimpl = 0;
	if (cp->coproc_nr==0 && reg_nr==COP0_PAGEMASK)	unimpl = 0;
	if (cp->coproc_nr==0 && reg_nr==COP0_WIRED)	unimpl = 0;
	if (cp->coproc_nr==0 && reg_nr==COP0_BADVADDR)	unimpl = 0;
	if (cp->coproc_nr==0 && reg_nr==COP0_COUNT) {
		/*  TODO: Increase count in a more meaningful way!  */
		cp->reg[COP0_COUNT] = (int32_t) (cp->reg[COP0_COUNT] + 1);
		unimpl = 0;
	}
	if (cp->coproc_nr==0 && reg_nr==COP0_ENTRYHI)	unimpl = 0;
	if (cp->coproc_nr==0 && reg_nr==COP0_COMPARE)	unimpl = 0;
	if (cp->coproc_nr==0 && reg_nr==COP0_STATUS)	unimpl = 0;
	if (cp->coproc_nr==0 && reg_nr==COP0_CAUSE)	unimpl = 0;
	if (cp->coproc_nr==0 && reg_nr==COP0_EPC)	unimpl = 0;
	if (cp->coproc_nr==0 && reg_nr==COP0_PRID)	unimpl = 0;
	if (cp->coproc_nr==0 && reg_nr==COP0_CONFIG) {
		if (select > 0) {
			switch (select) {
			case 1:	*ptr = cpu->cd.mips.cop0_config_select1;
				break;
			default:fatal("coproc_register_read(): unimplemented"
				    " config register select %i\n", select);
				exit(1);
			}
			return;
		}
		unimpl = 0;
	}
	if (cp->coproc_nr==0 && reg_nr==COP0_LLADDR)	unimpl = 0;
	if (cp->coproc_nr==0 && reg_nr==COP0_WATCHLO)	unimpl = 0;
	if (cp->coproc_nr==0 && reg_nr==COP0_WATCHHI)	unimpl = 0;
	if (cp->coproc_nr==0 && reg_nr==COP0_XCONTEXT)	unimpl = 0;
	if (cp->coproc_nr==0 && reg_nr==COP0_ERRCTL)	unimpl = 0;
	if (cp->coproc_nr==0 && reg_nr==COP0_CACHEERR)	unimpl = 0;
	if (cp->coproc_nr==0 && reg_nr==COP0_TAGDATA_LO)	unimpl = 0;
	if (cp->coproc_nr==0 && reg_nr==COP0_TAGDATA_HI)	unimpl = 0;
	if (cp->coproc_nr==0 && reg_nr==COP0_ERROREPC)	unimpl = 0;
	if (cp->coproc_nr==0 && reg_nr==COP0_RESERV22) {
		/*  Used by Linux on Linksys WRT54G  */
		unimpl = 0;
	}
	if (cp->coproc_nr==0 && reg_nr==COP0_DEBUG)	unimpl = 0;
	if (cp->coproc_nr==0 && reg_nr==COP0_PERFCNT)	unimpl = 0;
	if (cp->coproc_nr==0 && reg_nr==COP0_DESAVE)	unimpl = 0;

	if (cp->coproc_nr==1)	unimpl = 0;

	if (unimpl) {
		fatal("cpu%i: warning: read from unimplemented coproc%i"
		    " register %i (%s)\n", cpu->cpu_id, cp->coproc_nr, reg_nr,
		    cp->coproc_nr==0? cop0_names[reg_nr] : "?");

		mips_cpu_exception(cpu, EXCEPTION_CPU, 0, 0,
		    cp->coproc_nr, 0, 0, 0);
		return;
	}

	*ptr = cp->reg[reg_nr];
}


/*
 *  coproc_register_write();
 *
 *  Write a value to a MIPS coprocessor register.
 */
void coproc_register_write(struct cpu *cpu,
	struct mips_coproc *cp, int reg_nr, uint64_t *ptr, int flag64,
	int select)
{
	int unimpl = 1;
	int readonly = 0;
	uint64_t tmp = *ptr;
	uint64_t tmp2 = 0, old;
	int inval = 0;
	unsigned int old_asid;
	uint64_t oldmode;

	switch (cp->coproc_nr) {
	case 0:
		/*  COPROC 0:  */
		switch (reg_nr) {
		case COP0_INDEX:
		case COP0_RANDOM:
			unimpl = 0;
			break;
		case COP0_ENTRYLO0:
			unimpl = 0;
			if (cpu->cd.mips.cpu_type.mmu_model == MMU3K &&
			    (tmp & 0xff)!=0) {
				/*  char *symbol;
				    uint64_t offset;
				    symbol = get_symbol_name(cpu->pc, &offset);
				    fatal("YO! pc = 0x%08llx <%s> "
				    "lo=%016llx\n", (long long)
				    cpu->pc, symbol? symbol :
				    "no symbol", (long long)tmp); */
				tmp &= (R2K3K_ENTRYLO_PFN_MASK |
				    R2K3K_ENTRYLO_N | R2K3K_ENTRYLO_D |
				    R2K3K_ENTRYLO_V | R2K3K_ENTRYLO_G);
			} else if (cpu->cd.mips.cpu_type.mmu_model == MMU4K) {
				tmp &= (ENTRYLO_PFN_MASK | ENTRYLO_C_MASK |
				    ENTRYLO_D | ENTRYLO_V | ENTRYLO_G);
			}
			break;
		case COP0_BADVADDR:
			/*  Hm. Irix writes to this register. (Why?)  */
			unimpl = 0;
			break;
		case COP0_ENTRYLO1:
			unimpl = 0;
			if (cpu->cd.mips.cpu_type.mmu_model == MMU4K) {
				tmp &= (ENTRYLO_PFN_MASK | ENTRYLO_C_MASK |
				    ENTRYLO_D | ENTRYLO_V | ENTRYLO_G);
			}
			break;
		case COP0_CONTEXT:
			old = cp->reg[COP0_CONTEXT];
			cp->reg[COP0_CONTEXT] = tmp;
			if (cpu->cd.mips.cpu_type.mmu_model == MMU3K) {
				cp->reg[COP0_CONTEXT] &=
				    ~R2K3K_CONTEXT_BADVPN_MASK;
				cp->reg[COP0_CONTEXT] |=
				    (old & R2K3K_CONTEXT_BADVPN_MASK);
			} else if (cpu->cd.mips.cpu_type.rev == MIPS_R4100) {
				cp->reg[COP0_CONTEXT] &=
				    ~CONTEXT_BADVPN2_MASK_R4100;
				cp->reg[COP0_CONTEXT] |=
				    (old & CONTEXT_BADVPN2_MASK_R4100);
			} else {
				cp->reg[COP0_CONTEXT] &=
				    ~CONTEXT_BADVPN2_MASK;
				cp->reg[COP0_CONTEXT] |=
				    (old & CONTEXT_BADVPN2_MASK);
			}
			return;
		case COP0_PAGEMASK:
			tmp2 = tmp >> PAGEMASK_SHIFT;
			if (tmp2 != 0x000 &&
			    tmp2 != 0x003 &&
			    tmp2 != 0x00f &&
			    tmp2 != 0x03f &&
			    tmp2 != 0x0ff &&
			    tmp2 != 0x3ff &&
			    tmp2 != 0xfff)
				fatal("cpu%i: trying to write an invalid"
				    " pagemask 0x%08lx to COP0_PAGEMASK\n",
				    cpu->cpu_id, (long)tmp);
			unimpl = 0;
			break;
		case COP0_WIRED:
			if (cpu->cd.mips.cpu_type.mmu_model == MMU3K) {
				fatal("cpu%i: r2k/r3k wired register must "
				    "always be 8\n", cpu->cpu_id);
				tmp = 8;
			}
			cp->reg[COP0_RANDOM] = cp->nr_of_tlbs-1;
			tmp &= INDEX_MASK;
			unimpl = 0;
			break;
		case COP0_COUNT:
			if (tmp != (uint64_t)(int64_t)(int32_t)tmp)
				fatal("WARNING: trying to write a 64-bit value"
				    " to the COUNT register!\n");
			tmp = (int64_t)(int32_t)tmp;
			unimpl = 0;
			break;
		case COP0_COMPARE:
			if (cpu->machine->emulated_hz > 0) {
				int32_t compare_diff = tmp -
				    cp->reg[COP0_COMPARE];
				double hz;

				if (compare_diff < 0)
					hz = tmp - cp->reg[COP0_COUNT];

				if (compare_diff == 0)
					hz = 0;
				else
					hz = (double)cpu->machine->emulated_hz
					    / (double)compare_diff;
/*
 *  TODO: DON'T HARDCODE THIS!
 */
hz = 100.0;

				/*  Initialize or re-set the periodic timer:  */
				if (hz > 0) {
					if (cpu->cd.mips.timer == NULL)
						cpu->cd.mips.timer = timer_add(
						    hz, mips_timer_tick, cpu);
					else
						timer_update_frequency(
						    cpu->cd.mips.timer, hz);
				}
			}

			/*  Ack the periodic timer, if it was asserted:  */
			if (cp->reg[COP0_CAUSE] & 0x8000 &&
			    cpu->cd.mips.compare_interrupts_pending > 0)
				cpu->cd.mips.compare_interrupts_pending --;

			/*  Clear the timer interrupt assertion (bit 7):  */
			cp->reg[COP0_CAUSE] &= ~0x8000;

			if (tmp != (uint64_t)(int64_t)(int32_t)tmp)
				fatal("WARNING: trying to write a 64-bit value"
				    " to the COMPARE register!\n");

			tmp = (int64_t)(int32_t)tmp;
			cpu->cd.mips.compare_register_set = 1;
			unimpl = 0;
			break;
		case COP0_ENTRYHI:
			/*
			 *  Translation caches must be invalidated if the
			 *  ASID changes:
			 */
			switch (cpu->cd.mips.cpu_type.mmu_model) {
			case MMU3K:
				old_asid = cp->reg[COP0_ENTRYHI] &
				    R2K3K_ENTRYHI_ASID_MASK;
				if ((cp->reg[COP0_ENTRYHI] &
				    R2K3K_ENTRYHI_ASID_MASK) !=
				    (tmp & R2K3K_ENTRYHI_ASID_MASK))
					inval = 1;
				break;
			default:
				old_asid = cp->reg[COP0_ENTRYHI] & ENTRYHI_ASID;
				if ((cp->reg[COP0_ENTRYHI] & ENTRYHI_ASID) !=
				    (tmp & ENTRYHI_ASID))
					inval = 1;
				break;
			}

			if (inval)
				invalidate_asid(cpu, old_asid);

			unimpl = 0;
			if (cpu->cd.mips.cpu_type.mmu_model == MMU3K &&
			    (tmp & 0x3f)!=0) {
				/* char *symbol;
				   uint64_t offset;
				   symbol = get_symbol_name(cpu->pc,
				    &offset);
				   fatal("YO! pc = 0x%08llx <%s> "
				    "hi=%016llx\n", (long long)cpu->pc,
				    symbol? symbol :
				    "no symbol", (long long)tmp);  */
				tmp &= ~0x3f;
			}
			if (cpu->cd.mips.cpu_type.mmu_model == MMU3K)
				tmp &= (R2K3K_ENTRYHI_VPN_MASK |
				    R2K3K_ENTRYHI_ASID_MASK);
			else if (cpu->cd.mips.cpu_type.mmu_model == MMU10K)
				tmp &= (ENTRYHI_R_MASK |
				    ENTRYHI_VPN2_MASK_R10K | ENTRYHI_ASID);
			else if (cpu->cd.mips.cpu_type.rev == MIPS_R4100)
				tmp &= (ENTRYHI_R_MASK | ENTRYHI_VPN2_MASK |
				    0x1800 | ENTRYHI_ASID);
			else
				tmp &= (ENTRYHI_R_MASK | ENTRYHI_VPN2_MASK |
				    ENTRYHI_ASID);
			break;
		case COP0_EPC:
			unimpl = 0;
			break;
		case COP0_PRID:
			readonly = 1;
			break;
		case COP0_CONFIG:
			if (select > 0) {
				switch (select) {
				case 1:	cpu->cd.mips.cop0_config_select1 = tmp;
					break;
				default:fatal("coproc_register_write(): unimpl"
					    "emented config register select "
					    "%i\n", select);
					exit(1);
				}
				return;
			}

			/*  fatal("COP0_CONFIG: modifying K0 bits: "
			    "0x%08x => ", cp->reg[reg_nr]);  */
			tmp = *ptr;
			tmp &= 0x3;	/*  only bits 2..0 can be written  */
			cp->reg[reg_nr] &= ~(0x3);  cp->reg[reg_nr] |= tmp;
			/*  fatal("0x%08x\n", cp->reg[reg_nr]);  */
			return;
		case COP0_STATUS:
			oldmode = cp->reg[COP0_STATUS];
			tmp &= ~(1 << 21);	/*  bit 21 is read-only  */

			/*
			 *  When isolating caches, invalidate all translations.
			 *  During the isolation, a special hack in memory_rw.c
			 *  prevents translation tables from being updated, so
			 *  the translation caches don't have to be invalidated
			 *  when switching back to normal mode.
			 */
			if (cpu->cd.mips.cpu_type.mmu_model == MMU3K &&
			    (oldmode & MIPS1_ISOL_CACHES) !=
			    (tmp & MIPS1_ISOL_CACHES)) {
				/*  Invalidate everything if we are switching
				    to isolated mode:  */
				if (tmp & MIPS1_ISOL_CACHES) {
					cpu->invalidate_translation_caches(
					    cpu, 0, INVALIDATE_ALL);
				}
			}
			unimpl = 0;
			break;
		case COP0_CAUSE:
			/*  A write to the cause register only
			    affects IM bits 0 and 1:  */
			cp->reg[reg_nr] &= ~(0x3 << STATUS_IM_SHIFT);
			cp->reg[reg_nr] |= (tmp & (0x3 << STATUS_IM_SHIFT));
			return;
		case COP0_FRAMEMASK:
			/*  TODO: R10000  */
			unimpl = 0;
			break;
		case COP0_TAGDATA_LO:
		case COP0_TAGDATA_HI:
			/*  TODO: R4300 and others?  */
			unimpl = 0;
			break;
		case COP0_LLADDR:
			unimpl = 0;
			break;
		case COP0_WATCHLO:
		case COP0_WATCHHI:
			unimpl = 0;
			break;
		case COP0_XCONTEXT:
			/*
			 *  TODO:  According to the R10000 manual, the R4400
			 *  shares the PTEbase portion of the context registers
			 *  (that is, xcontext and context). On R10000, they
			 *  are separate registers.
			 */
			/*  debug("[ xcontext 0x%016llx ]\n", tmp);  */
			unimpl = 0;
			break;

		/*  Most of these are actually TODOs:  */
		case COP0_ERROREPC:
		case COP0_DEPC:
		case COP0_RESERV22:	/*  Used by Linux on Linksys WRT54G  */
		case COP0_DESAVE:
		case COP0_PERFCNT:
		case COP0_ERRCTL:	/*  R10000  */
			unimpl = 0;
			break;
		}
		break;

	case 1:
		/*  COPROC 1:  */
		unimpl = 0;
		break;
	}

	if (unimpl) {
		fatal("cpu%i: warning: write to unimplemented coproc%i "
		    "register %i (%s), data = 0x%016llx\n", cpu->cpu_id,
		    cp->coproc_nr, reg_nr, cp->coproc_nr==0?
		    cop0_names[reg_nr] : "?", (long long)tmp);

		/*  mips_cpu_exception(cpu, EXCEPTION_CPU, 0, 0,
		    cp->coproc_nr, 0, 0, 0);
		return;  */
	}

	if (readonly) {
		fatal("cpu%i: warning: write to READONLY coproc%i register "
		    "%i ignored\n", cpu->cpu_id, cp->coproc_nr, reg_nr);
		return;
	}

	cp->reg[reg_nr] = tmp;

	if (!flag64)
		cp->reg[reg_nr] = (int64_t)(int32_t)cp->reg[reg_nr];
}


/*
 *  MIPS floating-point stuff:
 *
 *  TODO:  Move this to some other file?
 */
static int mips_fmt_to_ieee_fmt[32] = {
	0, 0, 0, 0,  0, 0, 0, 0,
	0, 0, 0, 0,  0, 0, 0, 0,
	IEEE_FMT_S, IEEE_FMT_D, 0, 0,
	IEEE_FMT_W, IEEE_FMT_L, /* PS (Paired Single) */ 0, 0,
	0, 0, 0, 0,  0, 0, 0, 0  };

static const char *fmtname[32] = {
	 "0",  "1",  "2",  "3",  "4",  "5",  "6",  "7",
	 "8",  "9", "10", "11", "12", "13", "14", "15",
	 "s",  "d", "18", "19",  "w",  "l", "ps", "23",
	"24", "25", "26", "27", "28", "29", "30", "31" 	};

static const char *ccname[16] = {
	"f",  "un",   "eq",  "ueq", "olt", "ult", "ole", "ule",
	"sf", "ngle", "seq", "ngl", "lt",  "nge", "le",  "ngt"  };

#define	FPU_OP_ADD	1
#define	FPU_OP_SUB	2
#define	FPU_OP_MUL	3
#define	FPU_OP_DIV	4
#define	FPU_OP_SQRT	5
#define	FPU_OP_MOV	6
#define	FPU_OP_CVT	7
#define	FPU_OP_C	8
#define	FPU_OP_ABS	9
#define	FPU_OP_NEG	10
/*  TODO: CEIL.L, CEIL.W, FLOOR.L, FLOOR.W, RECIP, ROUND.L, ROUND.W, RSQRT  */


/*
 *  fpu_store_float_value():
 *
 *  Stores a float value (actually a double) in fmt format.
 */
static void fpu_store_float_value(struct mips_coproc *cp, int fd,
	double nf, int fmt, int nan)
{
	int ieee_fmt = mips_fmt_to_ieee_fmt[fmt];
	uint64_t r = ieee_store_float_value(nf, ieee_fmt, nan);

	/*
	 *  TODO: This is for 32-bit mode. It has to be updated later
	 *        for 64-bit coprocessor functionality!
	 */
	if (fmt == COP1_FMT_D || fmt == COP1_FMT_L) {
		cp->reg[fd] = r & 0xffffffffULL;
		cp->reg[(fd+1) & 31] = (r >> 32) & 0xffffffffULL;

		if (cp->reg[fd] & 0x80000000ULL)
			cp->reg[fd] |= 0xffffffff00000000ULL;
		if (cp->reg[fd+1] & 0x80000000ULL)
			cp->reg[fd+1] |= 0xffffffff00000000ULL;
	} else {
		cp->reg[fd] = r & 0xffffffffULL;

		if (cp->reg[fd] & 0x80000000ULL)
			cp->reg[fd] |= 0xffffffff00000000ULL;
	}
}


/*
 *  fpu_op():
 *
 *  Perform a floating-point operation. For those of fs and ft that are >= 0,
 *  those numbers are interpreted into local variables.
 *
 *  Only FPU_OP_C (compare) returns anything of interest, 1 for true, 0 for
 *  false.
 */
static int fpu_op(struct cpu *cpu, struct mips_coproc *cp, int op, int fmt,
	int ft, int fs, int fd, int cond, int output_fmt)
{
	/*  Potentially two input registers, fs and ft  */
	struct ieee_float_value float_value[2];
	int unordered, nan, ieee_fmt = mips_fmt_to_ieee_fmt[fmt];
	uint64_t fs_v = 0;
	double nf;

	if (fs >= 0) {
		fs_v = cp->reg[fs];
		/*  TODO: register-pair mode and plain
		    register mode? "FR" bit?  */
		if (fmt == COP1_FMT_D || fmt == COP1_FMT_L)
			fs_v = (fs_v & 0xffffffffULL) +
			    (cp->reg[(fs + 1) & 31] << 32);
		ieee_interpret_float_value(fs_v, &float_value[0], ieee_fmt);
	}
	if (ft >= 0) {
		uint64_t v = cp->reg[ft];
		/*  TODO: register-pair mode and
		    plain register mode? "FR" bit?  */
		if (fmt == COP1_FMT_D || fmt == COP1_FMT_L)
			v = (v & 0xffffffffULL) +
			    (cp->reg[(ft + 1) & 31] << 32);
		ieee_interpret_float_value(v, &float_value[1], ieee_fmt);
	}

	switch (op) {
	case FPU_OP_ADD:
		nf = float_value[0].f + float_value[1].f;
		/*  debug("  add: %f + %f = %f\n",
		    float_value[0].f, float_value[1].f, nf);  */
		fpu_store_float_value(cp, fd, nf, output_fmt,
		    float_value[0].nan || float_value[1].nan);
		break;
	case FPU_OP_SUB:
		nf = float_value[0].f - float_value[1].f;
		/*  debug("  sub: %f - %f = %f\n",
		    float_value[0].f, float_value[1].f, nf);  */
		fpu_store_float_value(cp, fd, nf, output_fmt,
		    float_value[0].nan || float_value[1].nan);
		break;
	case FPU_OP_MUL:
		nf = float_value[0].f * float_value[1].f;
		/*  debug("  mul: %f * %f = %f\n",
		    float_value[0].f, float_value[1].f, nf);  */
		fpu_store_float_value(cp, fd, nf, output_fmt,
		    float_value[0].nan || float_value[1].nan);
		break;
	case FPU_OP_DIV:
		nan = float_value[0].nan || float_value[1].nan;
		if (fabs(float_value[1].f) > 0.00000000001)
			nf = float_value[0].f / float_value[1].f;
		else {
			fatal("DIV by zero !!!!\n");
			nf = 0.0;	/*  TODO  */
			nan = 1;
		}
		/*  debug("  div: %f / %f = %f\n",
		    float_value[0].f, float_value[1].f, nf);  */
		fpu_store_float_value(cp, fd, nf, output_fmt, nan);
		break;
	case FPU_OP_SQRT:
		nan = float_value[0].nan;
		if (float_value[0].f >= 0.0)
			nf = sqrt(float_value[0].f);
		else {
			fatal("SQRT by less than zero, %f !!!!\n",
			    float_value[0].f);
			nf = 0.0;	/*  TODO  */
			nan = 1;
		}
		/*  debug("  sqrt: %f => %f\n", float_value[0].f, nf);  */
		fpu_store_float_value(cp, fd, nf, output_fmt, nan);
		break;
	case FPU_OP_ABS:
		nf = fabs(float_value[0].f);
		/*  debug("  abs: %f => %f\n", float_value[0].f, nf);  */
		fpu_store_float_value(cp, fd, nf, output_fmt,
		    float_value[0].nan);
		break;
	case FPU_OP_NEG:
		nf = - float_value[0].f;
		/*  debug("  neg: %f => %f\n", float_value[0].f, nf);  */
		fpu_store_float_value(cp, fd, nf, output_fmt,
		    float_value[0].nan);
		break;
	case FPU_OP_CVT:
		nf = float_value[0].f;
		/*  debug("  mov: %f => %f\n", float_value[0].f, nf);  */
		fpu_store_float_value(cp, fd, nf, output_fmt,
		    float_value[0].nan);
		break;
	case FPU_OP_MOV:
		/*  Non-arithmetic move:  */
		/*
		 *  TODO:  this is for 32-bit mode. It has to be updated later
		 *		for 64-bit coprocessor stuff.
		 */
		if (output_fmt == COP1_FMT_D || output_fmt == COP1_FMT_L) {
			cp->reg[fd] = fs_v & 0xffffffffULL;
			cp->reg[(fd+1) & 31] = (fs_v >> 32) & 0xffffffffULL;
			if (cp->reg[fd] & 0x80000000ULL)
				cp->reg[fd] |= 0xffffffff00000000ULL;
			if (cp->reg[fd+1] & 0x80000000ULL)
				cp->reg[fd+1] |= 0xffffffff00000000ULL;
		} else {
			cp->reg[fd] = fs_v & 0xffffffffULL;
			if (cp->reg[fd] & 0x80000000ULL)
				cp->reg[fd] |= 0xffffffff00000000ULL;
		}
		break;
	case FPU_OP_C:
		/*  debug("  c: cond=%i\n", cond);  */

		unordered = 0;
		if (float_value[0].nan || float_value[1].nan)
			unordered = 1;

		switch (cond) {
		case 2:	/*  Equal  */
			return (float_value[0].f == float_value[1].f);
		case 4:	/*  Ordered or Less than  */
			return (float_value[0].f < float_value[1].f)
			    || !unordered;
		case 5:	/*  Unordered or Less than  */
			return (float_value[0].f < float_value[1].f)
			    || unordered;
		case 6:	/*  Ordered or Less than or Equal  */
			return (float_value[0].f <= float_value[1].f)
			    || !unordered;
		case 7:	/*  Unordered or Less than or Equal  */
			return (float_value[0].f <= float_value[1].f)
			    || unordered;
		case 12:/*  Less than  */
			return (float_value[0].f < float_value[1].f);
		case 14:/*  Less than or equal  */
			return (float_value[0].f <= float_value[1].f);

		/*  The following are not commonly used, so I'll move these out
		    of the if-0 on a case-by-case basis.  */
#if 0
case 0:	return 0;					/*  False  */
case 1:	return 0;					/*  Unordered  */
case 3:	return (float_value[0].f == float_value[1].f);
			/*  Unordered or Equal  */
case 8:	return 0;				/*  Signaling false  */
case 9:	return 0;	/*  Not Greater than or Less than or Equal  */
case 10:return (float_value[0].f == float_value[1].f);	/*  Signaling Equal  */
case 11:return (float_value[0].f == float_value[1].f);	/*  Not Greater
		than or Less than  */
case 13:return !(float_value[0].f >= float_value[1].f);	/*  Not greater
		than or equal */
case 15:return !(float_value[0].f > float_value[1].f);	/*  Not greater than  */
#endif

		default:
			fatal("fpu_op(): unimplemented condition "
			    "code %i. see cpu_mips_coproc.c\n", cond);
		}
		break;
	default:
		fatal("fpu_op(): unimplemented op %i\n", op);
	}

	return 0;
}


/*
 *  fpu_function():
 *
 *  Returns 1 if function was implemented, 0 otherwise.
 *  Debug trace should be printed for known instructions.
 */
static int fpu_function(struct cpu *cpu, struct mips_coproc *cp,
	uint32_t function, int unassemble_only)
{
	int fd, fs, ft, fmt, cond, cc;

	fmt = (function >> 21) & 31;
	ft = (function >> 16) & 31;
	fs = (function >> 11) & 31;
	cc = (function >> 8) & 7;
	fd = (function >> 6) & 31;
	cond = (function >> 0) & 15;


	/*  bc1f, bc1t, bc1fl, bc1tl:  */
	if ((function & 0x03e00000) == 0x01000000) {
		int nd, tf, imm;
		const char *instr_mnem;

		/*  cc are bits 20..18:  */
		cc = (function >> 18) & 7;
		nd = (function >> 17) & 1;
		tf = (function >> 16) & 1;
		imm = function & 65535;
		if (imm >= 32768)
			imm -= 65536;

		instr_mnem = NULL;
		if (nd == 0 && tf == 0)  instr_mnem = "bc1f";
		if (nd == 0 && tf == 1)  instr_mnem = "bc1t";
		if (nd == 1 && tf == 0)  instr_mnem = "bc1fl";
		if (nd == 1 && tf == 1)  instr_mnem = "bc1tl";

		if (cpu->machine->instruction_trace || unassemble_only)
			debug("%s\t%i,0x%016llx\n", instr_mnem, cc,
			    (long long) (cpu->pc + (imm << 2)));
		if (unassemble_only)
			return 1;

		fatal("INTERNAL ERROR: MIPS coprocessor branches should not"
		    " be implemented in cpu_mips_coproc.c, but in"
		    " cpu_mips_instr.c!\n");
		exit(1);
	}

	/*  add.fmt: Floating-point add  */
	if ((function & 0x0000003f) == 0x00000000) {
		if (cpu->machine->instruction_trace || unassemble_only)
			debug("add.%s\tr%i,r%i,r%i\n",
			    fmtname[fmt], fd, fs, ft);
		if (unassemble_only)
			return 1;

		fpu_op(cpu, cp, FPU_OP_ADD, fmt, ft, fs, fd, -1, fmt);
		return 1;
	}

	/*  sub.fmt: Floating-point subtract  */
	if ((function & 0x0000003f) == 0x00000001) {
		if (cpu->machine->instruction_trace || unassemble_only)
			debug("sub.%s\tr%i,r%i,r%i\n",
			    fmtname[fmt], fd, fs, ft);
		if (unassemble_only)
			return 1;

		fpu_op(cpu, cp, FPU_OP_SUB, fmt, ft, fs, fd, -1, fmt);
		return 1;
	}

	/*  mul.fmt: Floating-point multiply  */
	if ((function & 0x0000003f) == 0x00000002) {
		if (cpu->machine->instruction_trace || unassemble_only)
			debug("mul.%s\tr%i,r%i,r%i\n",
			    fmtname[fmt], fd, fs, ft);
		if (unassemble_only)
			return 1;

		fpu_op(cpu, cp, FPU_OP_MUL, fmt, ft, fs, fd, -1, fmt);
		return 1;
	}

	/*  div.fmt: Floating-point divide  */
	if ((function & 0x0000003f) == 0x00000003) {
		if (cpu->machine->instruction_trace || unassemble_only)
			debug("div.%s\tr%i,r%i,r%i\n",
			    fmtname[fmt], fd, fs, ft);
		if (unassemble_only)
			return 1;

		fpu_op(cpu, cp, FPU_OP_DIV, fmt, ft, fs, fd, -1, fmt);
		return 1;
	}

	/*  sqrt.fmt: Floating-point square-root  */
	if ((function & 0x001f003f) == 0x00000004) {
		if (cpu->machine->instruction_trace || unassemble_only)
			debug("sqrt.%s\tr%i,r%i\n", fmtname[fmt], fd, fs);
		if (unassemble_only)
			return 1;

		fpu_op(cpu, cp, FPU_OP_SQRT, fmt, -1, fs, fd, -1, fmt);
		return 1;
	}

	/*  abs.fmt: Floating-point absolute value  */
	if ((function & 0x001f003f) == 0x00000005) {
		if (cpu->machine->instruction_trace || unassemble_only)
			debug("abs.%s\tr%i,r%i\n", fmtname[fmt], fd, fs);
		if (unassemble_only)
			return 1;

		fpu_op(cpu, cp, FPU_OP_ABS, fmt, -1, fs, fd, -1, fmt);
		return 1;
	}

	/*  mov.fmt: Floating-point (non-arithmetic) move  */
	if ((function & 0x0000003f) == 0x00000006) {
		if (cpu->machine->instruction_trace || unassemble_only)
			debug("mov.%s\tr%i,r%i\n", fmtname[fmt], fd, fs);
		if (unassemble_only)
			return 1;

		fpu_op(cpu, cp, FPU_OP_MOV, fmt, -1, fs, fd, -1, fmt);
		return 1;
	}

	/*  neg.fmt: Floating-point negate  */
	if ((function & 0x001f003f) == 0x00000007) {
		if (cpu->machine->instruction_trace || unassemble_only)
			debug("neg.%s\tr%i,r%i\n", fmtname[fmt], fd, fs);
		if (unassemble_only)
			return 1;

		fpu_op(cpu, cp, FPU_OP_NEG, fmt, -1, fs, fd, -1, fmt);
		return 1;
	}

	/*  trunc.l.fmt: Truncate  */
	if ((function & 0x001f003f) == 0x00000009) {
		if (cpu->machine->instruction_trace || unassemble_only)
			debug("trunc.l.%s\tr%i,r%i\n", fmtname[fmt], fd, fs);
		if (unassemble_only)
			return 1;

		/*  TODO: not CVT?  */

		fpu_op(cpu, cp, FPU_OP_CVT, fmt, -1, fs, fd, -1, COP1_FMT_L);
		return 1;
	}

	/*  trunc.w.fmt: Truncate  */
	if ((function & 0x001f003f) == 0x0000000d) {
		if (cpu->machine->instruction_trace || unassemble_only)
			debug("trunc.w.%s\tr%i,r%i\n", fmtname[fmt], fd, fs);
		if (unassemble_only)
			return 1;

		/*  TODO: not CVT?  */

		fpu_op(cpu, cp, FPU_OP_CVT, fmt, -1, fs, fd, -1, COP1_FMT_W);
		return 1;
	}

	/*  c.cond.fmt: Floating-point compare  */
	if ((function & 0x000000f0) == 0x00000030) {
		int cond_true;
		int bit;

		if (cpu->machine->instruction_trace || unassemble_only)
			debug("c.%s.%s\tcc%i,r%i,r%i\n", ccname[cond],
			    fmtname[fmt], cc, fs, ft);
		if (unassemble_only)
			return 1;

		cond_true = fpu_op(cpu, cp, FPU_OP_C, fmt,
		    ft, fs, -1, cond, fmt);

		/*
		 *  Both the FCCR and FCSR contain condition code bits:
		 *	FCCR:  bits 7..0
		 *	FCSR:  bits 31..25 and 23
		 */
		cp->fcr[MIPS_FPU_FCCR] &= ~(1 << cc);
		if (cond_true)
			cp->fcr[MIPS_FPU_FCCR] |= (1 << cc);

		if (cc == 0) {
			bit = 1 << MIPS_FCSR_FCC0_SHIFT;
			cp->fcr[MIPS_FPU_FCSR] &= ~bit;
			if (cond_true)
				cp->fcr[MIPS_FPU_FCSR] |= bit;
		} else {
			bit = 1 << (MIPS_FCSR_FCC1_SHIFT + cc-1);
			cp->fcr[MIPS_FPU_FCSR] &= ~bit;
			if (cond_true)
				cp->fcr[MIPS_FPU_FCSR] |= bit;
		}

		return 1;
	}

	/*  cvt.s.fmt: Convert to single floating-point  */
	if ((function & 0x001f003f) == 0x00000020) {
		if (cpu->machine->instruction_trace || unassemble_only)
			debug("cvt.s.%s\tr%i,r%i\n", fmtname[fmt], fd, fs);
		if (unassemble_only)
			return 1;

		fpu_op(cpu, cp, FPU_OP_CVT, fmt, -1, fs, fd, -1, COP1_FMT_S);
		return 1;
	}

	/*  cvt.d.fmt: Convert to double floating-point  */
	if ((function & 0x001f003f) == 0x00000021) {
		if (cpu->machine->instruction_trace || unassemble_only)
			debug("cvt.d.%s\tr%i,r%i\n", fmtname[fmt], fd, fs);
		if (unassemble_only)
			return 1;

		fpu_op(cpu, cp, FPU_OP_CVT, fmt, -1, fs, fd, -1, COP1_FMT_D);
		return 1;
	}

	/*  cvt.w.fmt: Convert to word fixed-point  */
	if ((function & 0x001f003f) == 0x00000024) {
		if (cpu->machine->instruction_trace || unassemble_only)
			debug("cvt.w.%s\tr%i,r%i\n", fmtname[fmt], fd, fs);
		if (unassemble_only)
			return 1;

		fpu_op(cpu, cp, FPU_OP_CVT, fmt, -1, fs, fd, -1, COP1_FMT_W);
		return 1;
	}

	return 0;
}


/*
 *  coproc_tlbpr():
 *
 *  'tlbp' and 'tlbr'.
 */
void coproc_tlbpr(struct cpu *cpu, int readflag)
{
	struct mips_coproc *cp = cpu->cd.mips.coproc[0];
	int i, found, g_bit;
	uint64_t vpn2, xmask;

	/*  Read:  */
	if (readflag) {
		if (cpu->cd.mips.cpu_type.mmu_model == MMU3K) {
			i = (cp->reg[COP0_INDEX] & R2K3K_INDEX_MASK) >>
			    R2K3K_INDEX_SHIFT;
			if (i >= cp->nr_of_tlbs) {
				/*  TODO:  exception?  */
				fatal("[ warning: tlbr from index %i (too "
				    "high) ]\n", i);
				return;
			}

			cp->reg[COP0_ENTRYHI]  = cp->tlbs[i].hi;
			cp->reg[COP0_ENTRYLO0] = cp->tlbs[i].lo0;
		} else {
			/*  R4000:  */
			i = cp->reg[COP0_INDEX] & INDEX_MASK;
			if (i >= cp->nr_of_tlbs)	/*  TODO:  exception  */
				return;

			cp->reg[COP0_PAGEMASK] = cp->tlbs[i].mask;
			cp->reg[COP0_ENTRYHI]  = cp->tlbs[i].hi;
			cp->reg[COP0_ENTRYLO1] = cp->tlbs[i].lo1;
			cp->reg[COP0_ENTRYLO0] = cp->tlbs[i].lo0;

			if (cpu->cd.mips.cpu_type.rev == MIPS_R4100) {
				/*  R4100 don't have the G bit in entryhi  */
			} else {
				/*  R4000 etc:  */
				cp->reg[COP0_ENTRYHI] &= ~TLB_G;
				g_bit = cp->tlbs[i].hi & TLB_G;

				cp->reg[COP0_ENTRYLO0] &= ~ENTRYLO_G;
				cp->reg[COP0_ENTRYLO1] &= ~ENTRYLO_G;
				if (g_bit) {
					cp->reg[COP0_ENTRYLO0] |= ENTRYLO_G;
					cp->reg[COP0_ENTRYLO1] |= ENTRYLO_G;
				}
			}
		}

		return;
	}

	/*  Probe:  */
	if (cpu->cd.mips.cpu_type.mmu_model == MMU3K) {
		vpn2 = cp->reg[COP0_ENTRYHI] & R2K3K_ENTRYHI_VPN_MASK;
		found = -1;
		for (i=0; i<cp->nr_of_tlbs; i++)
			if ( ((cp->tlbs[i].hi & R2K3K_ENTRYHI_ASID_MASK) ==
			    (cp->reg[COP0_ENTRYHI] & R2K3K_ENTRYHI_ASID_MASK))
			    || cp->tlbs[i].lo0 & R2K3K_ENTRYLO_G)
				if ((cp->tlbs[i].hi & R2K3K_ENTRYHI_VPN_MASK)
				    == vpn2) {
					found = i;
					break;
				}
	} else {
		/*  R4000 and R10000:  */
		if (cpu->cd.mips.cpu_type.mmu_model == MMU10K)
			xmask = ENTRYHI_R_MASK | ENTRYHI_VPN2_MASK_R10K;
		else if (cpu->cd.mips.cpu_type.rev == MIPS_R4100)
			xmask = ENTRYHI_R_MASK | ENTRYHI_VPN2_MASK | 0x1800;
		else
			xmask = ENTRYHI_R_MASK | ENTRYHI_VPN2_MASK;
		vpn2 = cp->reg[COP0_ENTRYHI] & xmask;
		found = -1;
		for (i=0; i<cp->nr_of_tlbs; i++) {
			int gbit = cp->tlbs[i].hi & TLB_G;
			if (cpu->cd.mips.cpu_type.rev == MIPS_R4100)
				gbit = (cp->tlbs[i].lo0 & ENTRYLO_G) &&
				    (cp->tlbs[i].lo1 & ENTRYLO_G);

			if ( ((cp->tlbs[i].hi & ENTRYHI_ASID) ==
			    (cp->reg[COP0_ENTRYHI] & ENTRYHI_ASID)) || gbit) {
				uint64_t a = vpn2 & ~cp->tlbs[i].mask;
				uint64_t b = (cp->tlbs[i].hi & xmask) &
				    ~cp->tlbs[i].mask;
				if (a == b) {
					found = i;
					break;
				}
			}
		}
	}
	if (found == -1)
		cp->reg[COP0_INDEX] = INDEX_P;
	else {
		if (cpu->cd.mips.cpu_type.mmu_model == MMU3K)
			cp->reg[COP0_INDEX] = found << R2K3K_INDEX_SHIFT;
		else
			cp->reg[COP0_INDEX] = found;
	}

	/*  Sign extend the index register:  */
	if ((cp->reg[COP0_INDEX] >> 32) == 0 &&
	    cp->reg[COP0_INDEX] & 0x80000000)
		cp->reg[COP0_INDEX] |=
		    0xffffffff00000000ULL;
}


/*
 *  coproc_tlbwri():
 *
 *  MIPS TLB write random (tlbwr) and write indexed (tlbwi) instructions.
 */
void coproc_tlbwri(struct cpu *cpu, int randomflag)
{
	struct mips_coproc *cp = cpu->cd.mips.coproc[0];
	int index, g_bit;
	uint64_t oldvaddr;

	if (randomflag) {
		if (cpu->cd.mips.cpu_type.exc_model == EXC3K) {
			index = ((cp->reg[COP0_RANDOM] & R2K3K_RANDOM_MASK)
			    >> R2K3K_RANDOM_SHIFT) - 1;
			/*  R3000 always has 8 wired entries:  */
			if (index < 8)
				index = cp->nr_of_tlbs - 1;
			cp->reg[COP0_RANDOM] = index << R2K3K_RANDOM_SHIFT;
		} else {
			cp->reg[COP0_RANDOM] = cp->reg[COP0_WIRED] + (random()
			    % (cp->nr_of_tlbs - cp->reg[COP0_WIRED]));
			index = cp->reg[COP0_RANDOM] & RANDOM_MASK;
		}
	} else {
		if (cpu->cd.mips.cpu_type.mmu_model == MMU3K)
			index = (cp->reg[COP0_INDEX] & R2K3K_INDEX_MASK)
			    >> R2K3K_INDEX_SHIFT;
		else
			index = cp->reg[COP0_INDEX] & INDEX_MASK;
	}

	if (index >= cp->nr_of_tlbs) {
		fatal("warning: tlb index %i too high (max is %i)\n",
		    index, cp->nr_of_tlbs - 1);
		/*  TODO:  cause an exception?  */
		return;
	}


#if 0
	/*  Debug dump of the previous entry at that index:  */
	fatal("{ old TLB entry at index %02x:", index);
	if (cpu->cd.mips.cpu_type.mmu_model == MMU3K) {
		fatal(" hi=%08"PRIx32, (uint32_t)cp->tlbs[index].hi);
		fatal(" lo=%08"PRIx32, (uint32_t)cp->tlbs[index].lo0);
	} else {
		if (cpu->is_32bit) {
			fatal(" mask=%08"PRIx32,(uint32_t)cp->tlbs[index].mask);
			fatal(" hi=%08"PRIx32, (uint32_t)cp->tlbs[index].hi);
			fatal(" lo0=%08"PRIx32, (uint32_t)cp->tlbs[index].lo0);
			fatal(" lo1=%08"PRIx32, (uint32_t)cp->tlbs[index].lo1);
		} else {
			fatal(" mask=%016"PRIx64, cp->tlbs[index].mask);
			fatal(" hi=%016"PRIx64, cp->tlbs[index].hi);
			fatal(" lo0=%016"PRIx64, cp->tlbs[index].lo0);
			fatal(" lo1=%016"PRIx64, cp->tlbs[index].lo1);
		}
	}
	fatal(" }\n");
#endif

	/*
	 *  Any virtual address translation for the old TLB entry must be
	 *  invalidated first:
	 *
	 *  (Only Valid entries need to be invalidated, and only those that
	 *  are either Global, or have the same ASID as the new entry will
	 *  have. No other address translations should be active anyway.)
	 */

	switch (cpu->cd.mips.cpu_type.mmu_model) {

	case MMU3K:
		oldvaddr = cp->tlbs[index].hi & R2K3K_ENTRYHI_VPN_MASK;
		oldvaddr = (int32_t) oldvaddr;

		if (cp->tlbs[index].lo0 & R2K3K_ENTRYLO_V &&
		    (cp->tlbs[index].lo0 & R2K3K_ENTRYLO_G ||
		    (cp->tlbs[index].hi & R2K3K_ENTRYHI_ASID_MASK) ==
		    (cp->reg[COP0_ENTRYHI] & R2K3K_ENTRYHI_ASID_MASK) ))
			cpu->invalidate_translation_caches(cpu, oldvaddr,
			    INVALIDATE_VADDR);

		break;

	default:if (cpu->cd.mips.cpu_type.mmu_model == MMU10K) {
			oldvaddr = cp->tlbs[index].hi &
			    (ENTRYHI_VPN2_MASK_R10K | ENTRYHI_R_MASK);
			/*  44 addressable bits:  */
			if (oldvaddr & 0x80000000000ULL)
				oldvaddr |= 0x3ffff00000000000ULL;
		} else if (cpu->is_32bit) {
			/*  MIPS32 etc.:  */
			oldvaddr = cp->tlbs[index].hi & ENTRYHI_VPN2_MASK;
			oldvaddr = (int32_t)oldvaddr;
		} else {
			/*  Assume MMU4K  */
			oldvaddr = cp->tlbs[index].hi &
			    (ENTRYHI_R_MASK | ENTRYHI_VPN2_MASK);
			/*  40 addressable bits:  */
			if (oldvaddr & 0x8000000000ULL)
				oldvaddr |= 0x3fffff0000000000ULL;
		}

		/*
		 *  TODO: non-4KB page sizes!
		 */
		if (cp->tlbs[index].lo0 & ENTRYLO_V)
			cpu->invalidate_translation_caches(cpu, oldvaddr,
			    INVALIDATE_VADDR);
		if (cp->tlbs[index].lo1 & ENTRYLO_V)
			cpu->invalidate_translation_caches(cpu, oldvaddr|0x1000,
			    INVALIDATE_VADDR);
	}

#if 0
	/*
	 *  Check for duplicate entries.  (There should not be two mappings
	 *  from one virtual address to physical addresses.)
	 *
	 *  TODO: Do this for MMU3K and R4100 too.
	 *
	 *  TODO: Make this detection more robust.
	 */
	if (cpu->cd.mips.cpu_type.mmu_model != MMU3K &&
	    cpu->cd.mips.cpu_type.rev != MIPS_R4100) {
		uint64_t vaddr1, vaddr2;
		int i;
		unsigned int asid;

		vaddr1 = cp->reg[COP0_ENTRYHI] &
		    (ENTRYHI_R_MASK | ENTRYHI_VPN2_MASK_R10K);
		asid = cp->reg[COP0_ENTRYHI] & ENTRYHI_ASID;
		/*  Since this is just a warning, it's probably not necessary
		    to use R4000 masks etc.  */

		for (i=0; i<cp->nr_of_tlbs; i++) {
			if (i == index && !randomflag)
				continue;

			if (!(cp->tlbs[i].hi & TLB_G) &&
			    (cp->tlbs[i].hi & ENTRYHI_ASID) != asid)
				continue;

			vaddr2 = cp->tlbs[i].hi &
			    (ENTRYHI_R_MASK | ENTRYHI_VPN2_MASK_R10K);
			if (vaddr1 == vaddr2 && ((cp->tlbs[i].lo0 &
			    ENTRYLO_V) || (cp->tlbs[i].lo1 & ENTRYLO_V)))
				fatal("\n[ WARNING! tlbw%s to index 0x%02x "
				    "vaddr=0x%llx (asid 0x%02x) is already in"
				    " the TLB (entry 0x%02x) ! ]\n\n",
				    randomflag? "r" : "i", index,
				    (long long)vaddr1, asid, i);
		}
	}
#endif

	/*  Write the new entry:  */

	if (cpu->cd.mips.cpu_type.mmu_model == MMU3K) {
		uint32_t vaddr, paddr;
		int wf = cp->reg[COP0_ENTRYLO0] & R2K3K_ENTRYLO_D? 1 : 0;
		unsigned char *memblock = NULL;

		cp->tlbs[index].hi = cp->reg[COP0_ENTRYHI];
		cp->tlbs[index].lo0 = cp->reg[COP0_ENTRYLO0];

		vaddr =  cp->reg[COP0_ENTRYHI] & R2K3K_ENTRYHI_VPN_MASK;
		paddr = cp->reg[COP0_ENTRYLO0] & R2K3K_ENTRYLO_PFN_MASK;

		memblock = memory_paddr_to_hostaddr(cpu->mem, paddr, 0);

		/*  Invalidate any code translation, if we are writing
		    a Dirty page to the TLB:  */
		if (wf) {
			cpu->invalidate_code_translation(cpu, paddr,
			    INVALIDATE_PADDR);
		}

		/*  Set new last_written_tlb_index hint:  */
		cpu->cd.mips.last_written_tlb_index = index;

		if (cp->reg[COP0_STATUS] & MIPS1_ISOL_CACHES) {
			fatal("Wow! Interesting case; tlbw* while caches"
			    " are isolated. TODO\n");
			/*  Don't update the translation table in this
			    case...  */
			exit(1);
		}

		/*  If we have a memblock (host page) for the physical
		    page, then add a translation for it immediately:  */
		if (memblock != NULL &&
		    cp->reg[COP0_ENTRYLO0] & R2K3K_ENTRYLO_V)
			cpu->update_translation_table(cpu, vaddr, memblock,
			    wf, paddr);
	} else {
		/*  R4000 etc.:  */
		unsigned char *memblock = NULL;
		int pfn_shift = 12, vpn_shift = 12;
		int wf0, wf1, mask;
		uint64_t vaddr0, vaddr1, paddr0, paddr1, ptmp;
		uint64_t psize;

		cp->tlbs[index].mask = cp->reg[COP0_PAGEMASK];
		cp->tlbs[index].hi   = cp->reg[COP0_ENTRYHI];
		cp->tlbs[index].lo1  = cp->reg[COP0_ENTRYLO1];
		cp->tlbs[index].lo0  = cp->reg[COP0_ENTRYLO0];

		wf0 = cp->tlbs[index].lo0 & ENTRYLO_D;
		wf1 = cp->tlbs[index].lo1 & ENTRYLO_D;

		mask = cp->reg[COP0_PAGEMASK];
		if (cpu->cd.mips.cpu_type.rev == MIPS_R4100) {
			pfn_shift = 10;
			mask |= 0x07ff;
		} else {
			mask |= 0x1fff;
		}
		switch (mask) {
		case 0x00007ff:
			if (cp->tlbs[index].lo0 & ENTRYLO_V ||
			    cp->tlbs[index].lo1 & ENTRYLO_V) {
				fatal("1KB pages don't work with dyntrans.\n");
				exit(1);
			}
			vpn_shift = 10;
			break;
		case 0x0001fff:	break;
		case 0x0007fff:	vpn_shift = 14; break;
		case 0x001ffff:	vpn_shift = 16; break;
		case 0x007ffff:	vpn_shift = 18; break;
		case 0x01fffff:	vpn_shift = 20; break;
		case 0x07fffff:	vpn_shift = 22; break;
		case 0x1ffffff:	vpn_shift = 24; break;
		case 0x7ffffff:	vpn_shift = 26; break;
		default:fatal("Unimplemented MASK = 0x%016x\n", mask);
			exit(1);
		}

		paddr0 = ((cp->tlbs[index].lo0 & ENTRYLO_PFN_MASK)
		    >> ENTRYLO_PFN_SHIFT) << pfn_shift
		    >> vpn_shift << vpn_shift;
		paddr1 = ((cp->tlbs[index].lo1 & ENTRYLO_PFN_MASK)
		    >> ENTRYLO_PFN_SHIFT) << pfn_shift
		    >> vpn_shift << vpn_shift;

		if (cpu->cd.mips.cpu_type.mmu_model == MMU10K) {
			vaddr0 = cp->tlbs[index].hi &
			    (ENTRYHI_R_MASK | ENTRYHI_VPN2_MASK_R10K);
			/*  44 addressable bits:  */
			if (vaddr0 & 0x80000000000ULL)
				vaddr0 |= 0x3ffff00000000000ULL;
		} else if (cpu->is_32bit) {
			/*  MIPS32 etc.:  */
			vaddr0 = cp->tlbs[index].hi & ENTRYHI_VPN2_MASK;
			vaddr0 = (int32_t)vaddr0;
		} else {
			/*  Assume MMU4K  */
			vaddr0 = cp->tlbs[index].hi &
			    (ENTRYHI_R_MASK | ENTRYHI_VPN2_MASK);
			/*  40 addressable bits:  */
			if (vaddr0 & 0x8000000000ULL)
				vaddr0 |= 0x3fffff0000000000ULL;
		}

		vaddr1 = vaddr0 | (1 << vpn_shift);

		g_bit = (cp->reg[COP0_ENTRYLO0] &
		    cp->reg[COP0_ENTRYLO1]) & ENTRYLO_G;

		if (cpu->cd.mips.cpu_type.rev == MIPS_R4100) {
			/*  NOTE: The VR4131 (and possibly others) don't have
			    a Global bit in entryhi  */
			cp->tlbs[index].hi &= ~cp->reg[COP0_PAGEMASK];
		} else {
			cp->tlbs[index].lo0 &= ~ENTRYLO_G;
			cp->tlbs[index].lo1 &= ~ENTRYLO_G;

			cp->tlbs[index].hi &= ~TLB_G;
			if (g_bit)
				cp->tlbs[index].hi |= TLB_G;
		}

		/*
		 *  Invalidate any code translations, if we are writing Dirty
		 *  pages to the TLB:  (TODO: 4KB hardcoded... ugly)
		 */
		psize = 1 << pfn_shift;
		for (ptmp = 0; ptmp < psize; ptmp += 0x1000) {
			if (wf0)
				cpu->invalidate_code_translation(cpu,
				    paddr0 + ptmp, INVALIDATE_PADDR);
			if (wf1)
				cpu->invalidate_code_translation(cpu,
				    paddr1 + ptmp, INVALIDATE_PADDR);
		}

		/*
		 *  If we have a memblock (host page) for the physical page,
		 *  then add a translation for it immediately, to save some
		 *  time. (It would otherwise be added later on anyway,
		 *  because of a translation miss.)
		 *
		 *  NOTE/TODO: This is only for 4KB pages so far. It would
		 *             be too expensive to add e.g. 16MB pages like
		 *             this.
		 */
		memblock = memory_paddr_to_hostaddr(cpu->mem, paddr0, 0);
		if (memblock != NULL && cp->reg[COP0_ENTRYLO0] & ENTRYLO_V)
			cpu->update_translation_table(cpu, vaddr0, memblock,
			    wf0, paddr0);
		memblock = memory_paddr_to_hostaddr(cpu->mem, paddr1, 0);
		if (memblock != NULL && cp->reg[COP0_ENTRYLO1] & ENTRYLO_V)
			cpu->update_translation_table(cpu, vaddr1, memblock,
			    wf1, paddr1);

		/*  Set new last_written_tlb_index hint:  */
		cpu->cd.mips.last_written_tlb_index = index;
	}
}


/*
 *  coproc_eret():
 *
 *  Return from exception. (R4000 etc.)
 */
void coproc_eret(struct cpu *cpu)
{
	if (cpu->cd.mips.coproc[0]->reg[COP0_STATUS] & STATUS_ERL) {
		cpu->pc = cpu->cd.mips.coproc[0]->reg[COP0_ERROREPC];
		cpu->cd.mips.coproc[0]->reg[COP0_STATUS] &= ~STATUS_ERL;
	} else {
		cpu->pc = cpu->cd.mips.coproc[0]->reg[COP0_EPC];
		cpu->delay_slot = 0;
		cpu->cd.mips.coproc[0]->reg[COP0_STATUS] &= ~STATUS_EXL;
	}

	cpu->cd.mips.rmw = 0;	/*  the "LL bit"  */
}


/*
 *  coproc_function():
 *
 *  Execute a coprocessor specific instruction. cp must be != NULL.
 *  Debug trace should be printed for known instructions, if
 *  unassemble_only is non-zero. (This will NOT execute the instruction.)
 *
 *  TODO:  This is a mess and should be restructured (again).
 */
void coproc_function(struct cpu *cpu, struct mips_coproc *cp, int cpnr,
	uint32_t function, int unassemble_only, int running)
{
	int co_bit, op, rt, rd, fs, copz;
	uint64_t tmpvalue;

	if (cp == NULL) {
		if (unassemble_only) {
			debug("cop%i\t0x%08x (coprocessor not available)\n",
			    cpnr, (int)function);
			return;
		}
		fatal("[ pc=0x%016llx cop%i\t0x%08x (coprocessor not "
		    "available)\n", (long long)cpu->pc, cpnr, (int)function);
		return;
	}

	/*  No FPU?  */
	if (cpnr == 1 && (cpu->cd.mips.cpu_type.flags & NOFPU)) {
		mips_cpu_exception(cpu, EXCEPTION_CPU, 0, 0, cpnr, 0, 0, 0);
		return;
	}

	/*  For quick reference:  */
	copz = (function >> 21) & 31;
	rt = (function >> 16) & 31;
	rd = (function >> 11) & 31;

	if (cpnr < 2 && (((function & 0x03e007f8) == (COPz_MFCz << 21))
	              || ((function & 0x03e007f8) == (COPz_DMFCz << 21)))) {
		if (unassemble_only) {
			debug("%s%i\t%s,", copz==COPz_DMFCz? "dmfc" : "mfc",
			    cpnr, regnames[rt]);
			if (cpnr == 0)
				debug("%s", cop0_names[rd]);
			else
				debug("r%i", rd);
			if (function & 7)
				debug(",%i", (int)(function & 7));
			debug("\n");
			return;
		}
		coproc_register_read(cpu, cpu->cd.mips.coproc[cpnr],
		    rd, &tmpvalue, function & 7);
		cpu->cd.mips.gpr[rt] = tmpvalue;
		if (copz == COPz_MFCz) {
			/*  Sign-extend:  */
			cpu->cd.mips.gpr[rt] &= 0xffffffffULL;
			if (cpu->cd.mips.gpr[rt] & 0x80000000ULL)
				cpu->cd.mips.gpr[rt] |= 0xffffffff00000000ULL;
		}
		return;
	}

	if (cpnr < 2 && (((function & 0x03e007f8) == (COPz_MTCz << 21))
	              || ((function & 0x03e007f8) == (COPz_DMTCz << 21)))) {
		if (unassemble_only) {
			debug("%s%i\t%s,", copz==COPz_DMTCz? "dmtc" : "mtc",
			    cpnr, regnames[rt]);
			if (cpnr == 0)
				debug("%s", cop0_names[rd]);
			else
				debug("r%i", rd);
			if (function & 7)
				debug(",%i", (int)(function & 7));
			debug("\n");
			return;
		}
		tmpvalue = cpu->cd.mips.gpr[rt];
		if (copz == COPz_MTCz) {
			/*  Sign-extend:  */
			tmpvalue &= 0xffffffffULL;
			if (tmpvalue & 0x80000000ULL)
				tmpvalue |= 0xffffffff00000000ULL;
		}
		coproc_register_write(cpu, cpu->cd.mips.coproc[cpnr], rd,
		    &tmpvalue, copz == COPz_DMTCz, function & 7);
		return;
	}

	if (cpnr <= 1 && (((function & 0x03e007ff) == (COPz_CFCz << 21))
	              || ((function & 0x03e007ff) == (COPz_CTCz << 21)))) {
		switch (copz) {
		case COPz_CFCz:		/*  Copy from FPU control register  */
			rt = (function >> 16) & 31;
			fs = (function >> 11) & 31;
			if (unassemble_only) {
				debug("cfc%i\t%s,r%i\n", cpnr,
				    regnames[rt], fs);
				return;
			}
			cpu->cd.mips.gpr[rt] = (int32_t)cp->fcr[fs];
			/*  TODO: implement delay for gpr[rt]
			    (for MIPS I,II,III only)  */
			return;
		case COPz_CTCz:		/*  Copy to FPU control register  */
			rt = (function >> 16) & 31;
			fs = (function >> 11) & 31;
			if (unassemble_only) {
				debug("ctc%i\t%s,r%i\n", cpnr,
				    regnames[rt], fs);
				return;
			}

			switch (cpnr) {
			case 0:	/*  System coprocessor  */
				fatal("[ warning: unimplemented ctc%i, "
				    "0x%08x -> ctl reg %i ]\n", cpnr,
				    (int)cpu->cd.mips.gpr[rt], fs);
				break;
			case 1:	/*  FPU  */
				if (fs == 0)
					fatal("[ Attempt to write to FPU "
					    "control register 0 (?) ]\n");
				else {
					uint64_t tmp = cpu->cd.mips.gpr[rt];
					cp->fcr[fs] = tmp;

					/*  TODO: writing to control register 31
					    should cause exceptions, depending
					    on status bits!  */

					switch (fs) {
					case MIPS_FPU_FCCR:
						cp->fcr[MIPS_FPU_FCSR] =
						    (cp->fcr[MIPS_FPU_FCSR] &
						    0x017fffffULL) | ((tmp & 1)
						    << MIPS_FCSR_FCC0_SHIFT)
						    | (((tmp & 0xfe) >> 1) <<
						    MIPS_FCSR_FCC1_SHIFT);
						break;
					case MIPS_FPU_FCSR:
						cp->fcr[MIPS_FPU_FCCR] =
						    (cp->fcr[MIPS_FPU_FCCR] &
						    0xffffff00ULL) | ((tmp >>
						    MIPS_FCSR_FCC0_SHIFT) & 1) |
						    (((tmp >>
						    MIPS_FCSR_FCC1_SHIFT)
						    & 0x7f) << 1);
						break;
					default:
						;
					}
				}
				break;
			}

			/*  TODO: implement delay for gpr[rt]
			    (for MIPS I,II,III only)  */
			return;
		default:
			;
		}
	}

	/*  Math (Floating point) coprocessor calls:  */
	if (cpnr==1) {
		if (fpu_function(cpu, cp, function, unassemble_only))
			return;
	}


	/*  Ugly R5900 hacks:  */
	if (cpu->cd.mips.cpu_type.rev == MIPS_R5900) {
		if ((function & 0xfffff) == COP0_EI) {
			if (unassemble_only) {
				debug("ei\n");
				return;
			}
			cpu->cd.mips.coproc[0]->reg[COP0_STATUS] |=
			    R5900_STATUS_EIE;
			return;
		}

		if ((function & 0xfffff) == COP0_DI) {
			if (unassemble_only) {
				debug("di\n");
				return;
			}
			cpu->cd.mips.coproc[0]->reg[COP0_STATUS] &=
			    ~R5900_STATUS_EIE;
			return;
		}
	}

	co_bit = (function >> 25) & 1;

	/*  TLB operations and other things:  */
	if (cp->coproc_nr == 0) {
		if (!unassemble_only) {
			fatal("FATAL INTERNAL ERROR: Should be implemented"
			    " with dyntrans instead.\n");
			exit(1);
		}

		switch (co_bit) {
		case 0:
			if ((function & 0x03e0ffdf) == 0x01606000) {
				debug("%ci", function & 0x20? 'e' : 'd');
				if (rt != MIPS_GPR_ZERO)
					debug("\t%s", regnames[rt]);
				debug("\n");
				return;
			}
			break;
		case 1:
			op = (function) & 0xff;
			switch (op) {
			case COP0_TLBR:		/*  Read indexed TLB entry  */
				debug("tlbr\n");
				return;
			case COP0_TLBWI:	/*  Write indexed  */
			case COP0_TLBWR:	/*  Write random  */
				if (op == COP0_TLBWI)
					debug("tlbwi");
				else
					debug("tlbwr");
				if (!running) {
					debug("\n");
					return;
				}
				debug("\tindex=%08llx",
				    (long long)cp->reg[COP0_INDEX]);
				debug(", random=%08llx",
				    (long long)cp->reg[COP0_RANDOM]);
				debug(", mask=%016llx",
				    (long long)cp->reg[COP0_PAGEMASK]);
				debug(", hi=%016llx",
				    (long long)cp->reg[COP0_ENTRYHI]);
				debug(", lo0=%016llx",
				    (long long)cp->reg[COP0_ENTRYLO0]);
				debug(", lo1=%016llx\n",
				    (long long)cp->reg[COP0_ENTRYLO1]);
				return;
			case COP0_TLBP:		/*  Probe TLB for
						    matching entry  */
				debug("tlbp\n");
				return;
			case COP0_RFE:		/*  R2000/R3000 only:
						    Return from Exception  */
				debug("rfe\n");
				return;
			case COP0_ERET:	/*  R4000: Return from exception  */
				debug("eret\n");
				return;
			case COP0_DERET:
				debug("deret\n");
				return;
			case COP0_WAIT:
				{
					int code = (function >> 6) & 0x7ffff;
					debug("wait");
					if (code > 0)
						debug("\t0x%x", code);
					debug("\n");
				}
				return;
			case COP0_STANDBY:
				debug("standby\n");
				return;
			case COP0_SUSPEND:
				debug("suspend\n");
				return;
			case COP0_HIBERNATE:
				debug("hibernate\n");
				return;
			default:
				;
			}
			break;
		}
	}

	/*  TODO: coprocessor R2020 on DECstation?  */
	if ((cp->coproc_nr==0 || cp->coproc_nr==3) && function == 0x0100ffff) {
		if (unassemble_only) {
			debug("decstation_r2020_writeback\n");
			return;
		}
		/*  TODO  */
		return;
	}

	if (unassemble_only) {
		debug("cop%i\t0x%08x (unimplemented)\n", cpnr, (int)function);
		return;
	}

	fatal("cpu%i: UNIMPLEMENTED coproc%i function %08"PRIx32" "
	    "(pc = %016"PRIx64")\n", cpu->cpu_id, cp->coproc_nr,
	    (uint32_t)function, cpu->pc);

	mips_cpu_exception(cpu, EXCEPTION_CPU, 0, 0, cp->coproc_nr, 0, 0, 0);
}

