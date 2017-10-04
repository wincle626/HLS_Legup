#ifndef	CPU_ARM_H
#define	CPU_ARM_H

/*
 *  Copyright (C) 2005-2010  Anders Gavare.  All rights reserved.
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
 *  ARM CPU definitions.
 */

#include "misc.h"
#include "interrupt.h"

struct cpu_family;

/*  ARM CPU types:  */
struct arm_cpu_type_def {
	const char	*name;
	uint32_t	cpu_id;
	int		flags;
	int		icache_shift;
	int		iway;
	int		dcache_shift;
	int		dway;
};


#define	ARM_SL			10
#define	ARM_FP			11
#define	ARM_IP			12
#define	ARM_SP			13
#define	ARM_LR			14
#define	ARM_PC			15
#define	N_ARM_REGS		16

#define	ARM_REG_NAMES		{				\
	"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",		\
	"r8", "r9", "sl", "fp", "ip", "sp", "lr", "pc"  }

#define	ARM_CONDITION_STRINGS	{				\
	"eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc",		\
	"hi", "ls", "ge", "lt", "gt", "le", "" /*Always*/ , "(INVALID)" }

/*  Names of Data Processing Instructions:  */
#define	ARM_DPI_NAMES		{				\
	"and", "eor", "sub", "rsb", "add", "adc", "sbc", "rsc",	\
	"tst", "teq", "cmp", "cmn", "orr", "mov", "bic", "mvn" }

#define	ARM_IC_ENTRIES_SHIFT		10

#define	ARM_N_IC_ARGS			3
#define	ARM_INSTR_ALIGNMENT_SHIFT	2
#define	ARM_IC_ENTRIES_PER_PAGE		(1 << ARM_IC_ENTRIES_SHIFT)
#define	ARM_PC_TO_IC_ENTRY(a)		(((a)>>ARM_INSTR_ALIGNMENT_SHIFT) \
					& (ARM_IC_ENTRIES_PER_PAGE-1))
#define	ARM_ADDR_TO_PAGENR(a)		((a) >> (ARM_IC_ENTRIES_SHIFT \
					+ ARM_INSTR_ALIGNMENT_SHIFT))

#define	ARM_F_N		8	/*  Same as ARM_FLAG_*, but        */
#define	ARM_F_Z		4	/*  for the 'flags' field instead  */
#define	ARM_F_C		2	/*  of cpsr.                       */
#define	ARM_F_V		1

#define	ARM_FLAG_N	0x80000000	/*  Negative flag  */
#define	ARM_FLAG_Z	0x40000000	/*  Zero flag  */
#define	ARM_FLAG_C	0x20000000	/*  Carry flag  */
#define	ARM_FLAG_V	0x10000000	/*  Overflow flag  */
#define	ARM_FLAG_Q	0x08000000	/*  DSP saturation overflow  */
#define	ARM_FLAG_I	0x00000080	/*  Interrupt disable  */
#define	ARM_FLAG_F	0x00000040	/*  Fast Interrupt disable  */
#define	ARM_FLAG_T	0x00000020	/*  Thumb mode  */

#define	ARM_FLAG_MODE	0x0000001f
#define	ARM_MODE_USR26	      0x00
#define	ARM_MODE_FIQ26	      0x01
#define	ARM_MODE_IRQ26	      0x02
#define	ARM_MODE_SVC26	      0x03
#define	ARM_MODE_USR32	      0x10
#define	ARM_MODE_FIQ32	      0x11
#define	ARM_MODE_IRQ32	      0x12
#define	ARM_MODE_SVC32	      0x13
#define	ARM_MODE_ABT32	      0x17
#define	ARM_MODE_UND32	      0x1b
#define	ARM_MODE_SYS32	      0x1f

#define ARM_EXCEPTION_TO_MODE	{	\
	ARM_MODE_SVC32, ARM_MODE_UND32, ARM_MODE_SVC32, ARM_MODE_ABT32, \
	ARM_MODE_ABT32, 0,              ARM_MODE_IRQ32, ARM_MODE_FIQ32 	}

#define	N_ARM_EXCEPTIONS	8

#define	ARM_EXCEPTION_RESET	0
#define	ARM_EXCEPTION_UND	1
#define	ARM_EXCEPTION_SWI	2
#define	ARM_EXCEPTION_PREF_ABT	3
#define	ARM_EXCEPTION_DATA_ABT	4
/*  5 was address exception in 26-bit ARM  */
#define	ARM_EXCEPTION_IRQ	6
#define	ARM_EXCEPTION_FIQ	7

DYNTRANS_MISC_DECLARATIONS(arm,ARM,uint32_t)

#define	ARM_MAX_VPH_TLB_ENTRIES		384


struct arm_cpu {
	/*
	 *  Misc.:
	 */
	struct arm_cpu_type_def	cpu_type;
	uint32_t		of_emul_addr;

	void			(*coproc[16])(struct cpu *, int opcode1,
				    int opcode2, int l_bit, int crn, int crm,
				    int rd);

	/*
	 *  General Purpose Registers (including the program counter):
	 *
	 *  r[] always contains the current register set. The others are
	 *  only used to swap to/from when changing modes. (An exception is
	 *  r[0..7], which are never swapped out, they are always present.)
	 */

	uint32_t		r[N_ARM_REGS];

	uint32_t		default_r8_r14[7];	/*  usr and sys  */
	uint32_t		fiq_r8_r14[7];
	uint32_t		irq_r13_r14[2];
	uint32_t		svc_r13_r14[2];
	uint32_t		abt_r13_r14[2];
	uint32_t		und_r13_r14[2];

	uint32_t		tmp_pc;		/*  Used for load/stores  */

	/*
	 *  Flag/status registers:
	 *
	 *  NOTE: 'flags' just contains the 4 flag bits. When cpsr is read,
	 *  the flags should be copied from 'flags', and when cpsr is written
	 *  to, 'flags' should be updated as well.
	 */
	size_t			flags;
	uint32_t		cpsr;
	uint32_t		spsr_svc;
	uint32_t		spsr_abt;
	uint32_t		spsr_und;
	uint32_t		spsr_irq;
	uint32_t		spsr_fiq;


	/*
	 *  System Control Coprocessor registers:
	 */
	uint32_t		cachetype;	/*  Cache Type Register  */
	uint32_t		control;	/*  Control Register  */
	uint32_t		auxctrl;	/*  Aux. Control Register  */
	uint32_t		ttb;		/*  Translation Table Base  */
	uint32_t		dacr;		/*  Domain Access Control  */
	uint32_t		fsr;		/*  Fault Status Register  */
	uint32_t		far;		/*  Fault Address Register  */
	uint32_t		pid;		/*  Process Id Register  */
	uint32_t		cpar;		/*  CoProcessor Access Reg.  */

	/*  i80321 Coprocessor 6: ICU (Interrupt controller)  */
	uint32_t		i80321_inten;	/*  enable  */
	uint32_t		i80321_isteer;
	uint32_t		i80321_isrc;	/*  current assertions  */
	uint32_t		tmr0;
	uint32_t		tmr1;
	struct interrupt	tmr0_irq;
	struct interrupt	tmr1_irq;
	uint32_t		tcr0;
	uint32_t		tcr1;
	uint32_t		trr0;
	uint32_t		trr1;
	uint32_t		tisr;
	uint32_t		wdtcr;

	/*  XScale Coprocessor 14: (Performance Monitoring Unit)  */
	/*  XSC1 access style:  */
	uint32_t		xsc1_pmnc;	/*  Perf. Monitor Ctrl Reg.  */
	uint32_t		xsc1_ccnt;	/*  Clock Counter  */
	uint32_t		xsc1_pmn0;	/*  Perf. Counter Reg. 0  */
	uint32_t		xsc1_pmn1;	/*  Perf. Counter Reg. 1  */
	/*  XSC2 access style:  */
	uint32_t		xsc2_pmnc;	/*  Perf. Monitor Ctrl Reg.  */
	uint32_t		xsc2_ccnt;	/*  Clock Counter  */
	uint32_t		xsc2_inten;	/*  Interrupt Enable  */
	uint32_t		xsc2_flag;	/*  Overflow Flag Register  */
	uint32_t		xsc2_evtsel;	/*  Event Selection Register  */
	uint32_t		xsc2_pmn0;	/*  Perf. Counter Reg. 0  */
	uint32_t		xsc2_pmn1;	/*  Perf. Counter Reg. 1  */
	uint32_t		xsc2_pmn2;	/*  Perf. Counter Reg. 2  */
	uint32_t		xsc2_pmn3;	/*  Perf. Counter Reg. 3  */

	/*  For caching the host address of the L1 translation table:  */
	unsigned char		*translation_table;
	uint32_t		last_ttb;

	/*
	 *  Interrupts:
	 */
	int			irq_asserted;


	/*
	 *  Instruction translation cache, and 32-bit virtual -> physical ->
	 *  host address translation:
	 */
	DYNTRANS_ITC(arm)
	VPH_TLBS(arm,ARM)
	VPH32_16BITVPHENTRIES(arm,ARM)

	/*  ARM specific: */
	uint32_t			is_userpage[N_VPH32_ENTRIES/32];
};


/*  System Control Coprocessor, control bits:  */
#define	ARM_CONTROL_MMU		0x0001
#define	ARM_CONTROL_ALIGN	0x0002
#define	ARM_CONTROL_CACHE	0x0004
#define	ARM_CONTROL_WBUFFER	0x0008
#define	ARM_CONTROL_PROG32	0x0010
#define	ARM_CONTROL_DATA32	0x0020
#define	ARM_CONTROL_BIG		0x0080
#define	ARM_CONTROL_S		0x0100
#define	ARM_CONTROL_R		0x0200
#define	ARM_CONTROL_F		0x0400
#define	ARM_CONTROL_Z		0x0800
#define	ARM_CONTROL_ICACHE	0x1000
#define	ARM_CONTROL_V		0x2000
#define	ARM_CONTROL_RR		0x4000
#define	ARM_CONTROL_L4		0x8000

/*  Auxiliary Control Register bits:  */
#define	ARM_AUXCTRL_MD		0x30	/*  MiniData Cache Attribute  */
#define	ARM_AUXCTRL_MD_SHIFT	4
#define	ARM_AUXCTRL_P		0x02	/*  Page Table Memory Attribute  */
#define	ARM_AUXCTRL_K		0x01	/*  Write Buffer Coalescing Disable  */

/*  Cache Type register bits:  */
#define	ARM_CACHETYPE_CLASS		0x1e000000
#define	ARM_CACHETYPE_CLASS_SHIFT	25
#define	ARM_CACHETYPE_HARVARD		0x01000000
#define	ARM_CACHETYPE_HARVARD_SHIFT	24
#define	ARM_CACHETYPE_DSIZE		0x001c0000
#define	ARM_CACHETYPE_DSIZE_SHIFT	18
#define	ARM_CACHETYPE_DASSOC		0x00038000
#define	ARM_CACHETYPE_DASSOC_SHIFT	15
#define	ARM_CACHETYPE_DLINE		0x00003000
#define	ARM_CACHETYPE_DLINE_SHIFT	12
#define	ARM_CACHETYPE_ISIZE		0x000001c0
#define	ARM_CACHETYPE_ISIZE_SHIFT	6
#define	ARM_CACHETYPE_IASSOC		0x00000038
#define	ARM_CACHETYPE_IASSOC_SHIFT	3
#define	ARM_CACHETYPE_ILINE		0x00000003
#define	ARM_CACHETYPE_ILINE_SHIFT	0

/*  cpu_arm.c:  */
void arm_setup_initial_translation_table(struct cpu *cpu, uint32_t ttb_addr);
void arm_translation_table_set_l1(struct cpu *cpu, uint32_t vaddr,
	uint32_t paddr);
void arm_translation_table_set_l1_b(struct cpu *cpu, uint32_t vaddr,
	uint32_t paddr);
void arm_exception(struct cpu *, int);
int arm_run_instr(struct cpu *cpu);
void arm_update_translation_table(struct cpu *cpu, uint64_t vaddr_page,
	unsigned char *host_page, int writeflag, uint64_t paddr_page);
void arm_invalidate_translation_caches(struct cpu *cpu, uint64_t, int);
void arm_invalidate_code_translation(struct cpu *cpu, uint64_t, int);
void arm_load_register_bank(struct cpu *cpu);
void arm_save_register_bank(struct cpu *cpu);
int arm_memory_rw(struct cpu *cpu, struct memory *mem, uint64_t vaddr,
	unsigned char *data, size_t len, int writeflag, int cache_flags);
int arm_cpu_family_init(struct cpu_family *);

/*  cpu_arm_coproc.c:  */
void arm_coproc_15(struct cpu *cpu, int opcode1, int opcode2, int l_bit,
	int crn, int crm, int rd);
void arm_coproc_i80321_6(struct cpu *cpu, int opcode1, int opcode2, int l_bit,
	int crn, int crm, int rd);
void arm_coproc_xscale_14(struct cpu *cpu, int opcode1, int opcode2, int l_bit,
	int crn, int crm, int rd);

/*  memory_arm.c:  */
int arm_translate_v2p(struct cpu *cpu, uint64_t vaddr,
	uint64_t *return_addr, int flags);
int arm_translate_v2p_mmu(struct cpu *cpu, uint64_t vaddr,
	uint64_t *return_addr, int flags);

#endif	/*  CPU_ARM_H  */
