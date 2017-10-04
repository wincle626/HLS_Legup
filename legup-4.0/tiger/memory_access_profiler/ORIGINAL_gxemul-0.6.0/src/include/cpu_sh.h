#ifndef	CPU_SH_H
#define	CPU_SH_H

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
 *  SuperH CPU definitions.
 *
 *  Note 1: Many things here are SH4-specific, so it probably doesn't work
 *          for SH3 emulation.
 *
 *  Note 2: The SuperH emulation in GXemul does not include SH5/SH64 at
 *          this time. There doesn't seem to be that much interesting code
 *          to run in the emulator for SH5. :-/
 */

#include "interrupt.h"
#include "misc.h"
#include "sh4_dmacreg.h"

#include "thirdparty/sh4_cpu.h"


struct cpu_family;


/*  SH CPU types:  */
struct sh_cpu_type_def {
	const char	*name;
	int		bits;
	int		arch;
	uint32_t	pvr;
	uint32_t	prr;
};

#define	SH_CPU_TYPE_DEFS				{	    \
	{ "SH7708R", 32, 3, 0,              0,                   }, \
	{ "SH7750",  32, 4, SH4_PVR_SH7750, 0	                 }, \
	{ "SH7750R", 32, 4, SH4_PVR_SH7750, SH4_PRR_7750R	 }, \
	{ "SH7751R", 32, 4, SH4_PVR_SH7751, SH4_PRR_7751R	 }, \
	/* { "SH5",  64, 5, 0,              0	                 }, */ \
	{ NULL,       0, 0, 0,              0	                 }  }


#define	SH_N_IC_ARGS			2
#define	SH_INSTR_ALIGNMENT_SHIFT	1
#define	SH_IC_ENTRIES_SHIFT		11
#define	SH_IC_ENTRIES_PER_PAGE		(1 << SH_IC_ENTRIES_SHIFT)
#define	SH_PC_TO_IC_ENTRY(a)		(((a)>>SH_INSTR_ALIGNMENT_SHIFT) \
					& (SH_IC_ENTRIES_PER_PAGE-1))
#define	SH_ADDR_TO_PAGENR(a)		((a) >> (SH_IC_ENTRIES_SHIFT \
					+ SH_INSTR_ALIGNMENT_SHIFT))

DYNTRANS_MISC_DECLARATIONS(sh,SH,uint32_t)

#define	SH_MAX_VPH_TLB_ENTRIES		128


#define	SH_N_GPRS		16
#define	SH_N_GPRS_BANKED	8
#define	SH_N_FPRS		16

#define	SH_N_ITLB_ENTRIES	4
#define	SH_N_UTLB_ENTRIES	64

/*  An instruction with an invalid encoding; used for software
    emulation of PROM calls within GXemul:  */
#define	SH_INVALID_INSTR	0x00fb


struct sh_cpu {
	struct sh_cpu_type_def cpu_type;

	/*  General Purpose Registers:  */
	uint32_t	r[SH_N_GPRS];
	uint32_t	r_bank[SH_N_GPRS_BANKED];

	/*  Floating-Point Registers:  */
	uint32_t	fr[SH_N_FPRS];
	uint32_t	xf[SH_N_FPRS];	/*  "Other bank."  */

	uint32_t	mach;		/*  Multiply-Accumulate High  */
	uint32_t	macl;		/*  Multiply-Accumulate Low  */
	uint32_t	pr;		/*  Procedure Register  */
	uint32_t	fpscr;		/*  Floating-point Status/Control  */
	uint32_t	fpul;		/*  Floating-point Communication Reg  */
	uint32_t	sr;		/*  Status Register  */
	uint32_t	ssr;		/*  Saved Status Register  */
	uint32_t	spc;		/*  Saved PC  */
	uint32_t	gbr;		/*  Global Base Register  */
	uint32_t	vbr;		/*  Vector Base Register  */
	uint32_t	sgr;		/*  Saved General Register  */
	uint32_t	dbr;		/*  Debug Base Register  */

	/*  Cache control:  */
	uint32_t	ccr;		/*  Cache Control Register  */
	uint32_t	qacr0;		/*  Queue Address Control Register 0  */
	uint32_t	qacr1;		/*  Queue Address Control Register 1  */

	/*  MMU/TLB registers:  */
	uint32_t	pteh;		/*  Page Table Entry High  */
	uint32_t	ptel;		/*  Page Table Entry Low  */
	uint32_t	ptea;		/*  Page Table Entry A  */
	uint32_t	ttb;		/*  Translation Table Base  */
	uint32_t	tea;		/*  TLB Exception Address Register  */
	uint32_t	mmucr;		/*  MMU Control Register  */
	uint32_t	itlb_hi[SH_N_ITLB_ENTRIES];
	uint32_t	itlb_lo[SH_N_ITLB_ENTRIES];
	uint32_t	utlb_hi[SH_N_UTLB_ENTRIES];
	uint32_t	utlb_lo[SH_N_UTLB_ENTRIES];

	/*  Exception handling:  */
	uint32_t	tra;		/*  TRAPA Exception Register  */
	uint32_t	expevt;		/*  Exception Event Register  */
	uint32_t	intevt;		/*  Interrupt Event Register  */

	/*  Interrupt controller:  */
	uint16_t	intc_ipra;	/*  Interrupt Priority Registers  */
	uint16_t	intc_iprb;
	uint16_t	intc_iprc;
	uint16_t	intc_iprd;
	uint32_t	intc_intpri00;
	uint32_t	intc_intpri04;
	uint32_t	intc_intpri08;
	uint32_t	intc_intpri0c;
	uint32_t	intc_intreq00;
	uint32_t	intc_intreq04;
	uint32_t	intc_intmsk00;
	uint32_t	intc_intmsk04;
	/*  Cached and calculated values:  */
	uint8_t		int_prio_and_pending[0x1000 / 0x20];
	int16_t		int_to_assert;	/*  Calculated int to assert  */
	unsigned int	int_level;	/*  Calculated int level  */

	/*  Timer/clock functionality:  */
	int		pclock;

	/*  DMA Controller: (4 channels)  */
	uint32_t	dmac_sar[N_SH4_DMA_CHANNELS];
	uint32_t	dmac_dar[N_SH4_DMA_CHANNELS];
	uint32_t	dmac_tcr[N_SH4_DMA_CHANNELS];
	uint32_t	dmac_chcr[N_SH4_DMA_CHANNELS];

	/*  PCI controller:  */
	struct pci_data	*pcic_pcibus;


	/*
	 *  Instruction translation cache and Virtual->Physical->Host
	 *  address translation:
	 */
	DYNTRANS_ITC(sh)
	VPH_TLBS(sh,SH)
	VPH32(sh,SH)
};


/*  Status register bits:  */
#define	SH_SR_T			0x00000001	/*  True/false  */
#define	SH_SR_S			0x00000002	/*  Saturation  */
#define	SH_SR_IMASK		0x000000f0	/*  Interrupt mask  */
#define	SH_SR_IMASK_SHIFT		4
#define	SH_SR_Q			0x00000100	/*  State for Divide Step  */
#define	SH_SR_M			0x00000200	/*  State for Divide Step  */
#define	SH_SR_FD		0x00008000	/*  FPU Disable  */
#define	SH_SR_BL		0x10000000	/*  Exception/Interrupt Block */
#define	SH_SR_RB		0x20000000	/*  Register Bank 0/1  */
#define	SH_SR_MD		0x40000000	/*  Privileged Mode  */

/*  Floating-point status/control register bits:  */
#define	SH_FPSCR_RM_MASK	0x00000003	/*  Rounding Mode  */
#define	   SH_FPSCR_RM_NEAREST	       0x0	/*  Round to nearest  */
#define	   SH_FPSCR_RM_ZERO	       0x1	/*  Round to zero  */
#define	SH_FPSCR_INEXACT	0x00000004	/*  Inexact exception  */
#define	SH_FPSCR_UNDERFLOW	0x00000008	/*  Underflow exception  */
#define	SH_FPSCR_OVERFLOW	0x00000010	/*  Overflow exception  */
#define	SH_FPSCR_DIV_BY_ZERO	0x00000020	/*  Div by zero exception  */
#define	SH_FPSCR_INVALID	0x00000040	/*  Invalid exception  */
#define	SH_FPSCR_EN_INEXACT	0x00000080	/*  Inexact enable  */
#define	SH_FPSCR_EN_UNDERFLOW	0x00000100	/*  Underflow enable  */
#define	SH_FPSCR_EN_OVERFLOW	0x00000200	/*  Overflow enable  */
#define	SH_FPSCR_EN_DIV_BY_ZERO	0x00000400	/*  Div by zero enable  */
#define	SH_FPSCR_EN_INVALID	0x00000800	/*  Invalid enable  */
#define	SH_FPSCR_CAUSE_INEXACT	0x00001000	/*  Cause Inexact  */
#define	SH_FPSCR_CAUSE_UNDERFLOW 0x00002000	/*  Cause Underflow  */
#define	SH_FPSCR_CAUSE_OVERFLOW	0x00004000	/*  Cause Overflow  */
#define	SH_FPSCR_CAUSE_DIVBY0	0x00008000	/*  Cause Div by 0  */
#define	SH_FPSCR_CAUSE_INVALID	0x00010000	/*  Cause Invalid  */
#define	SH_FPSCR_CAUSE_ERROR	0x00020000	/*  Cause Error  */
#define	SH_FPSCR_DN_ZERO	0x00040000	/*  Denormalization Mode  */
#define	SH_FPSCR_PR		0x00080000	/*  Double-Precision Mode  */
#define	SH_FPSCR_SZ		0x00100000	/*  Double-Precision Size  */
#define	SH_FPSCR_FR		0x00200000	/*  Register Bank Select  */


/*  int_prio_and_pending bits:  */
#define	SH_INT_ASSERTED		0x10
#define	SH_INT_PRIO_MASK	0x0f

/*  cpu_sh.c:  */
void sh_cpu_interrupt_assert(struct interrupt *interrupt);
void sh_cpu_interrupt_deassert(struct interrupt *interrupt);
int sh_cpu_instruction_has_delayslot(struct cpu *cpu, unsigned char *ib);
int sh_run_instr(struct cpu *cpu);
void sh_update_translation_table(struct cpu *cpu, uint64_t vaddr_page,
	unsigned char *host_page, int writeflag, uint64_t paddr_page);
void sh_invalidate_translation_caches(struct cpu *cpu, uint64_t, int);
void sh_invalidate_code_translation(struct cpu *cpu, uint64_t, int);
void sh_init_64bit_dummy_tables(struct cpu *cpu);
int sh_memory_rw(struct cpu *cpu, struct memory *mem, uint64_t vaddr,
	unsigned char *data, size_t len, int writeflag, int cache_flags);
int sh_cpu_family_init(struct cpu_family *);

void sh_update_interrupt_priorities(struct cpu *cpu);
void sh_update_sr(struct cpu *cpu, uint32_t new_sr);
void sh_exception(struct cpu *cpu, int expevt, int intevt, uint32_t vaddr);

/*  memory_sh.c:  */
int sh_translate_v2p(struct cpu *cpu, uint64_t vaddr,
	uint64_t *return_addr, int flags);


#endif	/*  CPU_SH_H  */
