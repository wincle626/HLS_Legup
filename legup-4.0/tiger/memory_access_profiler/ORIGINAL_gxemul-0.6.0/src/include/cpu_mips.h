#ifndef	CPU_MIPS_H
#define	CPU_MIPS_H

/*
 *  Copyright (C) 2003-2010  Anders Gavare.  All rights reserved.
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
 *  MIPS CPU definitions.
 */

#include "interrupt.h"
#include "misc.h"

struct cpu_family;
struct emul;
struct machine;
struct timer;

/*
 *  CPU type definitions:  See mips_cpu_types.h.
 */

struct mips_cpu_type_def {
	const char	*name;
	int		rev;
	int		sub;
	char		flags;
	char		exc_model;		/*  EXC3K or EXC4K  */
	char		mmu_model;		/*  MMU3K or MMU4K  */
	char		isa_level;		/*  1, 2, 3, 4, 5, 32, 64  */
	char		isa_revision;		/*  1 or 2 (for MIPS32/64)  */
	int		nr_of_tlb_entries;	/*  32, 48, 64, ...  */
	char		instrs_per_cycle;	/*  simplified, 1, 2, or 4  */
	int		picache;
	int		pilinesize;
	int		piways;
	int		pdcache;
	int		pdlinesize;
	int		pdways;
	int		scache;
	int		slinesize;
	int		sways;
};

#define	INITIAL_PC			0xffffffffbfc00000ULL
#define	INITIAL_STACK_POINTER		(0xffffffffa0008000ULL - 256)


/*
 *  Coproc 0:
 *
 *  NOTE:
 *	On R3000, only hi and lo0 are used, and then only the lowest 32 bits.
 */
#define	N_MIPS_COPROC_REGS	32
struct mips_tlb {
	uint64_t	hi;
	uint64_t	lo0;
	uint64_t	lo1;
	uint64_t	mask;
};


/*
 *  Coproc 1:
 */
/*  FPU control registers:  */
#define	N_MIPS_FCRS			32
#define	MIPS_FPU_FCIR			0
#define	MIPS_FPU_FCCR			25
#define	MIPS_FPU_FCSR			31
#define	   MIPS_FCSR_FCC0_SHIFT		   23
#define	   MIPS_FCSR_FCC1_SHIFT		   25

#define	N_VADDR_TO_TLB_INDEX_ENTRIES	(1 << 20)

struct mips_coproc {
	int		coproc_nr;
	uint64_t	reg[N_MIPS_COPROC_REGS];

	/*  Only for COP0:  */
	struct mips_tlb	*tlbs;
	int		nr_of_tlbs;

	/*  Only for COP1:  floating point control registers  */
	/*  (Maybe also for COP0?)  */
	uint64_t	fcr[N_MIPS_FCRS];
};

#define	N_MIPS_COPROCS		4

#define	N_MIPS_GPRS		32	/*  General purpose registers  */
#define	N_MIPS_FPRS		32	/*  Floating point registers  */

/*
 *  These should all be 2 characters wide:
 *
 *  NOTE: These are for 32-bit ABIs. For the 64-bit ABI, registers 8..11
 *  are used to pass arguments and are then called "a4".."a7".
 *
 *  TODO: Should there be two different variants of this? It's not really
 *  possible to figure out in some easy way if the code running was
 *  written for a 32-bit or 64-bit ABI.
 */
#define MIPS_REGISTER_NAMES	{ \
	"zr", "at", "v0", "v1", "a0", "a1", "a2", "a3", \
	"t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7", \
	"s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", \
	"t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"  }

#define	MIPS_GPR_ZERO		0		/*  zero  */
#define	MIPS_GPR_AT		1		/*  at  */
#define	MIPS_GPR_V0		2		/*  v0  */
#define	MIPS_GPR_V1		3		/*  v1  */
#define	MIPS_GPR_A0		4		/*  a0  */
#define	MIPS_GPR_A1		5		/*  a1  */
#define	MIPS_GPR_A2		6		/*  a2  */
#define	MIPS_GPR_A3		7		/*  a3  */
#define	MIPS_GPR_T0		8		/*  t0  */
#define	MIPS_GPR_T1		9		/*  t1  */
#define	MIPS_GPR_T2		10		/*  t2  */
#define	MIPS_GPR_T3		11		/*  t3  */
#define	MIPS_GPR_T4		12		/*  t4  */
#define	MIPS_GPR_T5		13		/*  t5  */
#define	MIPS_GPR_T6		14		/*  t6  */
#define	MIPS_GPR_T7		15		/*  t7  */
#define	MIPS_GPR_S0		16		/*  s0  */
#define	MIPS_GPR_S1		17		/*  s1  */
#define	MIPS_GPR_S2		18		/*  s2  */
#define	MIPS_GPR_S3		19		/*  s3  */
#define	MIPS_GPR_S4		20		/*  s4  */
#define	MIPS_GPR_S5		21		/*  s5  */
#define	MIPS_GPR_S6		22		/*  s6  */
#define	MIPS_GPR_S7		23		/*  s7  */
#define	MIPS_GPR_T8		24		/*  t8  */
#define	MIPS_GPR_T9		25		/*  t9  */
#define	MIPS_GPR_K0		26		/*  k0  */
#define	MIPS_GPR_K1		27		/*  k1  */
#define	MIPS_GPR_GP		28		/*  gp  */
#define	MIPS_GPR_SP		29		/*  sp  */
#define	MIPS_GPR_FP		30		/*  fp  */
#define	MIPS_GPR_RA		31		/*  ra  */

#define	N_HI6			64
#define	N_SPECIAL		64
#define	N_REGIMM		32


/*  An "impossible" paddr:  */
#define	IMPOSSIBLE_PADDR		0x1212343456566767ULL

#define	DEFAULT_PCACHE_SIZE		15	/*  32 KB  */
#define	DEFAULT_PCACHE_LINESIZE		5	/*  32 bytes  */

struct r3000_cache_line {
	uint32_t	tag_paddr;
	int		tag_valid;
};
#define	R3000_TAG_VALID		1
#define	R3000_TAG_DIRTY		2


#define	MIPS_IC_ENTRIES_SHIFT		10

#define	MIPS_N_IC_ARGS			3
#define	MIPS_INSTR_ALIGNMENT_SHIFT	2
#define	MIPS_IC_ENTRIES_PER_PAGE	(1 << MIPS_IC_ENTRIES_SHIFT)
#define	MIPS_PC_TO_IC_ENTRY(a)		(((a)>>MIPS_INSTR_ALIGNMENT_SHIFT) \
					& (MIPS_IC_ENTRIES_PER_PAGE-1))
#define	MIPS_ADDR_TO_PAGENR(a)		((a) >> (MIPS_IC_ENTRIES_SHIFT \
					+ MIPS_INSTR_ALIGNMENT_SHIFT))

#define	MIPS_L2N		17
#define	MIPS_L3N		18

#define	MIPS_MAX_VPH_TLB_ENTRIES	192

DYNTRANS_MISC_DECLARATIONS(mips,MIPS,uint64_t)
DYNTRANS_MISC64_DECLARATIONS(mips,MIPS,uint8_t)


struct mips_cpu {
	struct mips_cpu_type_def cpu_type;

	/*  General purpose registers:  */
	uint64_t	gpr[N_MIPS_GPRS];

	/*  Dummy destination register when writing to the zero register:  */
	uint64_t	scratch;

	/*  Special purpose registers:  */
	uint64_t	hi;
	uint64_t	lo;

	/*  Coprocessors:  */
	struct mips_coproc *coproc[N_MIPS_COPROCS];
	uint64_t	cop0_config_select1;

	int		last_written_tlb_index;

	/*  Count/compare timer:  */
	int		compare_register_set;
	int		compare_interrupts_pending;
	struct interrupt irq_compare;
	struct timer	*timer;

	int		rmw;		/*  Read-Modify-Write  */
	uint64_t	rmw_len;	/*  Length of rmw modification  */
	uint64_t	rmw_addr;	/*  Address of rmw modification  */

	/*
	 *  NOTE:  The R5900 has 128-bit registers. I'm not really sure
	 *  whether they are used a lot or not, at least with code produced
	 *  with gcc they are not. An important case however is lq and sq
	 *  (load and store of 128-bit values). These "upper halves" of R5900
	 *  quadwords can be used in those cases.
	 *
	 *  hi1 and lo1 are the high 64-bit parts of the hi and lo registers.
	 *  sa is a 32-bit "shift amount" register.
	 *
	 *  TODO:  Generalize this.
	 */
	uint64_t	gpr_quadhi[N_MIPS_GPRS];
	uint64_t	hi1;
	uint64_t	lo1;
	uint32_t	r5900_sa;


	/*
	 *  Data and Instruction caches:
	 */

	/*  Cache sizes: (1 << x) x=0 for default values  */
	/*  This is legacy stuff. TODO: Clean up!  */
	int		cache_picache;
	int		cache_pdcache;
	int		cache_secondary;
	int		cache_picache_linesize;
	int		cache_pdcache_linesize;
	int		cache_secondary_linesize;

	unsigned char	*cache[2];
	void		*cache_tags[2];
	uint64_t	cache_last_paddr[2];
	int		cache_size[2];
	int		cache_linesize[2];
	int		cache_mask[2];


	/*
	 *  Instruction translation cache and Virtual->Physical->Host
	 *  address translation:
	 */
	DYNTRANS_ITC(mips)
	VPH_TLBS(mips,MIPS)
	VPH32(mips,MIPS)
	VPH64(mips,MIPS)
};


/*  cpu_mips.c:  */
void mips_cpu_interrupt_assert(struct interrupt *interrupt);
void mips_cpu_interrupt_deassert(struct interrupt *interrupt);
int mips_cpu_instruction_has_delayslot(struct cpu *cpu, unsigned char *ib);
void mips_cpu_tlbdump(struct machine *m, int x, int rawflag);
void mips_cpu_register_match(struct machine *m, char *name, 
	int writeflag, uint64_t *valuep, int *match_register);
void mips_cpu_register_dump(struct cpu *cpu, int gprs, int coprocs);
int mips_cpu_disassemble_instr(struct cpu *cpu, unsigned char *instr,
        int running, uint64_t addr);
void mips_cpu_exception(struct cpu *cpu, int exccode, int tlb, uint64_t vaddr,
        /*  uint64_t pagemask,  */  int coproc_nr, uint64_t vaddr_vpn2,
        int vaddr_asid, int x_64);
int mips_cpu_run(struct emul *emul, struct machine *machine);
void mips_cpu_dumpinfo(struct cpu *cpu);
void mips_cpu_list_available_types(void);
int mips_cpu_family_init(struct cpu_family *);


/*  cpu_mips_coproc.c:  */
struct mips_coproc *mips_coproc_new(struct cpu *cpu, int coproc_nr);
void mips_coproc_tlb_set_entry(struct cpu *cpu, int entrynr, int size,
        uint64_t vaddr, uint64_t paddr0, uint64_t paddr1,
        int valid0, int valid1, int dirty0, int dirty1, int global, int asid,
        int cachealgo0, int cachealgo1);
void coproc_register_read(struct cpu *cpu,
        struct mips_coproc *cp, int reg_nr, uint64_t *ptr, int select);
void coproc_register_write(struct cpu *cpu,
        struct mips_coproc *cp, int reg_nr, uint64_t *ptr, int flag64,
	int select);
void coproc_tlbpr(struct cpu *cpu, int readflag);
void coproc_tlbwri(struct cpu *cpu, int randomflag);
void coproc_rfe(struct cpu *cpu);
void coproc_eret(struct cpu *cpu);
void coproc_function(struct cpu *cpu, struct mips_coproc *cp, int cpnr,
        uint32_t function, int unassemble_only, int running);


/*  memory_mips.c:  */
int memory_cache_R3000(struct cpu *cpu, int cache, uint64_t paddr,
	int writeflag, size_t len, unsigned char *data);
int mips_memory_rw(struct cpu *cpu, struct memory *mem, uint64_t vaddr,
	unsigned char *data, size_t len, int writeflag, int cache_flags);

int translate_v2p_mmu3k(struct cpu *cpu, uint64_t vaddr,
        uint64_t *return_addr, int flags);
int translate_v2p_mmu8k(struct cpu *cpu, uint64_t vaddr,
        uint64_t *return_addr, int flags);
int translate_v2p_mmu10k(struct cpu *cpu, uint64_t vaddr,
        uint64_t *return_addr, int flags);
int translate_v2p_mmu4100(struct cpu *cpu, uint64_t vaddr,
        uint64_t *return_addr, int flags);
int translate_v2p_generic(struct cpu *cpu, uint64_t vaddr,
        uint64_t *return_addr, int flags);


/*  Dyntrans unaligned load/store:  */
void mips_unaligned_loadstore(struct cpu *cpu, struct mips_instr_call *ic, 
	int is_left, int wlen, int store);


int mips_run_instr(struct cpu *cpu);
void mips_update_translation_table(struct cpu *cpu, uint64_t vaddr_page,
	unsigned char *host_page, int writeflag, uint64_t paddr_page);
void mips_invalidate_translation_caches(struct cpu *cpu, uint64_t, int);
void mips_invalidate_code_translation(struct cpu *cpu, uint64_t, int);
int mips32_run_instr(struct cpu *cpu);
void mips32_update_translation_table(struct cpu *cpu, uint64_t vaddr_page,
	unsigned char *host_page, int writeflag, uint64_t paddr_page);
void mips32_invalidate_translation_caches(struct cpu *cpu, uint64_t, int);
void mips32_invalidate_code_translation(struct cpu *cpu, uint64_t, int);


#endif	/*  CPU_MIPS_H  */
