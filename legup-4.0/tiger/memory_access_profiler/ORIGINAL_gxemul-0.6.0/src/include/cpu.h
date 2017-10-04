#ifndef	CPU_H
#define	CPU_H

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
 *  CPU-related definitions.
 */


#include <sys/types.h>
#include <inttypes.h>
#include <sys/time.h>

/*  This is needed for undefining 'mips', 'ppc' etc. on weird systems:  */
#include "../../config.h"

#include "timer.h"


/*
 *  Dyntrans misc declarations, used throughout the dyntrans code.
 *
 *  Note that there is space for all instruction calls within a page, and then
 *  two more. The first one of these "extra" instruction slots is the end-of-
 *  page slot. It transfers control to the first instruction slot on the next
 *  (virtual) page.
 *
 *  The second of these extra instruction slots is an additional end-of-page
 *  slot for delay-slot architectures. On e.g. MIPS, a branch instruction can
 *  "nullify" (skip) the delay-slot. If the end-of-page slot is skipped, then
 *  we end up one step after that. That's where the end_of_page2 slot is. :)
 *
 *  next_ofs points to the next page in a chain of possible pages. (Several
 *  pages can be in the same chain, but only one matches the specific physaddr.)
 *
 *  translations_bitmap is a tiny bitmap indicating which parts of the page have
 *  actual translations. Bit 0 corresponds to the lowest 1/32th of the page, bit
 *  1 to the second-lowest 1/32th, and so on. This speeds up page invalidations,
 *  since only part of the page need to be reset.
 *
 *  translation_ranges_ofs is an offset within the translation cache to a short
 *  list of ranges for this physpage which contain code. The list is of fixed
 *  length; to extend the list, the list should be made to point to another
 *  list, and so forth. (Bad, O(n) find/insert complexity. Should be fixed some
 *  day. TODO)  See definition of physpage_ranges below.
 */
#define DYNTRANS_MISC_DECLARATIONS(arch,ARCH,addrtype)  struct \
	arch ## _instr_call {					\
		void	(*f)(struct cpu *, struct arch ## _instr_call *); \
		size_t	arg[ARCH ## _N_IC_ARGS];			\
	};								\
									\
	/*  Translation cache struct for each physical page:  */	\
	struct arch ## _tc_physpage {					\
		struct arch ## _instr_call ics[ARCH ## _IC_ENTRIES_PER_PAGE+2];\
		uint32_t	next_ofs;	/*  (0 for end of chain)  */ \
		uint32_t	translations_bitmap;			\
		uint32_t	translation_ranges_ofs;			\
		addrtype	physaddr;				\
	};								\
									\
	struct arch ## _vpg_tlb_entry {					\
		uint8_t		valid;					\
		uint8_t		writeflag;				\
		addrtype	vaddr_page;				\
		addrtype	paddr_page;				\
		unsigned char	*host_page;				\
	};

#define	DYNTRANS_MISC64_DECLARATIONS(arch,ARCH,tlbindextype)		\
	struct arch ## _l3_64_table {					\
		unsigned char	*host_load[1 << ARCH ## _L3N];		\
		unsigned char	*host_store[1 << ARCH ## _L3N];		\
		uint64_t	phys_addr[1 << ARCH ## _L3N];		\
		tlbindextype	vaddr_to_tlbindex[1 << ARCH ## _L3N];	\
		struct arch ## _tc_physpage *phys_page[1 << ARCH ## _L3N]; \
		struct arch ## _l3_64_table	*next;			\
		int		refcount;				\
	};								\
	struct arch ## _l2_64_table {					\
		struct arch ## _l3_64_table	*l3[1 << ARCH ## _L2N];	\
		struct arch ## _l2_64_table	*next;			\
		int				refcount;		\
	};


/*
 *  This structure contains a list of ranges within an emulated
 *  physical page that contain translatable code.
 */
#define	PHYSPAGE_RANGES_ENTRIES_PER_LIST		20
struct physpage_ranges {
	uint32_t	next_ofs;	/*  0 for end of chain  */
	uint32_t	n_entries_used;
	uint16_t	base[PHYSPAGE_RANGES_ENTRIES_PER_LIST];
	uint16_t	length[PHYSPAGE_RANGES_ENTRIES_PER_LIST];
	uint16_t	count[PHYSPAGE_RANGES_ENTRIES_PER_LIST];
};


/*
 *  Dyntrans "Instruction Translation Cache":
 *
 *  cur_physpage is a pointer to the current physpage. (It _HAPPENS_ to
 *  be the same as cur_ic_page, because all the instrcalls should be placed
 *  first in the physpage struct!)
 *
 *  cur_ic_page is a pointer to an array of xxx_IC_ENTRIES_PER_PAGE
 *  instruction call entries.
 *
 *  next_ic points to the next such instruction call to be executed.
 *
 *  combination_check, when set to non-NULL, is executed automatically after
 *  an instruction has been translated. (It check for combinations of
 *  instructions; low_addr is the offset of the translated instruction in the
 *  current page, NOT shifted right.)
 */
#define DYNTRANS_ITC(arch)	struct arch ## _tc_physpage *cur_physpage;  \
				struct arch ## _instr_call  *cur_ic_page;   \
				struct arch ## _instr_call  *next_ic;       \
				struct arch ## _tc_physpage *physpage_template;\
				void (*combination_check)(struct cpu *,     \
				    struct arch ## _instr_call *, int low_addr);

/*
 *  Virtual -> physical -> host address translation TLB entries:
 *  ------------------------------------------------------------
 *
 *  Regardless of whether 32-bit or 64-bit address translation is used, the
 *  same TLB entry structure is used.
 */
#define	VPH_TLBS(arch,ARCH)						\
	struct arch ## _vpg_tlb_entry					\
	    vph_tlb_entry[ARCH ## _MAX_VPH_TLB_ENTRIES];

/*
 *  32-bit dyntrans emulated Virtual -> physical -> host address translation:
 *  -------------------------------------------------------------------------
 *
 *  This stuff assumes that 4 KB pages are used. 20 bits to select a page
 *  means just 1 M entries needed. This is small enough that a couple of
 *  full-size tables can fit in virtual memory on modern hosts (both 32-bit
 *  and 64-bit hosts). :-)
 *
 *  Usage: e.g. VPH32(arm,ARM)
 *           or VPH32(sparc,SPARC)
 *
 *  The vph_tlb_entry entries are cpu dependent tlb entries.
 *
 *  The host_load and host_store entries point to host pages; the phys_addr
 *  entries are uint32_t (emulated physical addresses).
 *
 *  phys_page points to translation cache physpages.
 *
 *  vaddr_to_tlbindex is a virtual address to tlb index hint table.
 *  The values in this array are the tlb index plus 1, so a value of, say,
 *  3 means tlb index 2. A value of 0 would mean a tlb index of -1, which
 *  is not a valid index. (I.e. no hit.)
 *
 *  The VPH32EXTENDED variant adds an additional postfix to the array
 *  names. Used so far only for usermode addresses in M88K emulation.
 */
#define	N_VPH32_ENTRIES		1048576
#define	VPH32(arch,ARCH)						\
	unsigned char		*host_load[N_VPH32_ENTRIES];		\
	unsigned char		*host_store[N_VPH32_ENTRIES];		\
	uint32_t		phys_addr[N_VPH32_ENTRIES];		\
	struct arch ## _tc_physpage  *phys_page[N_VPH32_ENTRIES];	\
	uint8_t			vaddr_to_tlbindex[N_VPH32_ENTRIES];
#define	VPH32_16BITVPHENTRIES(arch,ARCH)				\
	unsigned char		*host_load[N_VPH32_ENTRIES];		\
	unsigned char		*host_store[N_VPH32_ENTRIES];		\
	uint32_t		phys_addr[N_VPH32_ENTRIES];		\
	struct arch ## _tc_physpage  *phys_page[N_VPH32_ENTRIES];	\
	uint16_t		vaddr_to_tlbindex[N_VPH32_ENTRIES];
#define	VPH32EXTENDED(arch,ARCH,ex)					\
	unsigned char		*host_load_ ## ex[N_VPH32_ENTRIES];	\
	unsigned char		*host_store_ ## ex[N_VPH32_ENTRIES];	\
	uint32_t		phys_addr_ ## ex[N_VPH32_ENTRIES];	\
	struct arch ## _tc_physpage  *phys_page_ ## ex[N_VPH32_ENTRIES];\
	uint8_t			vaddr_to_tlbindex_ ## ex[N_VPH32_ENTRIES];


/*
 *  64-bit dyntrans emulated Virtual -> physical -> host address translation:
 *  -------------------------------------------------------------------------
 *
 *  Usage: e.g. VPH64(alpha,ALPHA)
 *           or VPH64(sparc,SPARC)
 *
 *  l1_64 is an array containing poiners to l2 tables.
 *
 *  l2_64_dummy is a pointer to a "dummy l2 table". Instead of having NULL
 *  pointers in l1_64 for unused slots, a pointer to the dummy table can be
 *  used.
 */
#define	DYNTRANS_L1N		17
#define	VPH64(arch,ARCH)						\
	struct arch ## _l3_64_table	*l3_64_dummy;			\
	struct arch ## _l3_64_table	*next_free_l3;			\
	struct arch ## _l2_64_table	*l2_64_dummy;			\
	struct arch ## _l2_64_table	*next_free_l2;			\
	struct arch ## _l2_64_table	*l1_64[1 << DYNTRANS_L1N];


/*  Include all CPUs' header files here:  */
#include "cpu_arm.h"
#include "cpu_m88k.h"
#include "cpu_mips.h"
#include "cpu_ppc.h"
#include "cpu_sh.h"

struct cpu;
struct emul;
struct machine;
struct memory;
struct settings;


/*
 *  cpu_family
 *  ----------
 *
 *  This structure consists of various pointers to functions, performing
 *  architecture-specific functions.
 *
 *  Except for the next and arch fields at the top, all fields in the
 *  cpu_family struct are filled in by ecah CPU family's init function.
 */
struct cpu_family {
	struct cpu_family	*next;
	int			arch;

	/*  Familty name, e.g. "MIPS", "Alpha" etc.  */
	char			*name;

	/*  Fill in architecture specific parts of a struct cpu.  */
	int			(*cpu_new)(struct cpu *cpu, struct memory *mem,
				    struct machine *machine,
				    int cpu_id, char *cpu_type_name);

	/*  Initialize various translation tables.  */
	void			(*init_tables)(struct cpu *cpu);

	/*  List available CPU types for this architecture.  */
	void			(*list_available_types)(void);

	/*  Disassemble an instruction.  */
	int			(*disassemble_instr)(struct cpu *cpu,
				    unsigned char *instr, int running,
				    uint64_t dumpaddr);

	/*  Dump CPU registers in readable format.  */
	void			(*register_dump)(struct cpu *cpu,
				    int gprs, int coprocs);

	/*  Dump generic CPU info in readable format.  */
	void			(*dumpinfo)(struct cpu *cpu);

	/*  Dump TLB data for CPU id x.  */
	void			(*tlbdump)(struct machine *m, int x,
				    int rawflag);

	/*  Print architecture-specific function call arguments.
	    (This is called for each function call, if running with -t.)  */
	void			(*functioncall_trace)(struct cpu *,
				    int n_args);
};


/*
 *  More dyntrans stuff:
 *
 *  The translation cache begins with N_BASE_TABLE_ENTRIES uint32_t offsets
 *  into the cache, for possible translation cache structs for physical pages.
 */

/*  Meaning of delay_slot:  */
#define	NOT_DELAYED			0
#define	DELAYED				1
#define	TO_BE_DELAYED			2
#define	EXCEPTION_IN_DELAY_SLOT		8

#define	N_SAFE_DYNTRANS_LIMIT_SHIFT	14
#define	N_SAFE_DYNTRANS_LIMIT	((1 << (N_SAFE_DYNTRANS_LIMIT_SHIFT - 1)) - 1)

#define	MAX_DYNTRANS_READAHEAD		128

#define	DEFAULT_DYNTRANS_CACHE_SIZE	(48*1048576)
#define	DYNTRANS_CACHE_MARGIN		200000

#define	N_BASE_TABLE_ENTRIES		65536
#define	PAGENR_TO_TABLE_INDEX(a)	((a) & (N_BASE_TABLE_ENTRIES-1))


/*
 *  The generic CPU struct:
 */

struct cpu {
	/*  Pointer back to the machine this CPU is in:  */
	struct machine	*machine;

	/*  Settings:  */
	struct settings *settings;

	/*  CPU-specific name, e.g. "R2000", "21164PC", etc.  */
	char		*name;

	/*  Full "path" to the CPU, e.g. "machine[0].cpu[0]":  */
	char		*path;

	/*  Nr of instructions executed, etc.:  */
	int64_t		ninstrs;
	int64_t		ninstrs_show;
	int64_t		ninstrs_flush;
	int64_t		ninstrs_since_gettimeofday;
	struct timeval	starttime;

	/*  EMUL_LITTLE_ENDIAN or EMUL_BIG_ENDIAN.  */
	uint8_t		byte_order;

	/*  0 for emulated 64-bit CPUs, 1 for 32-bit.  */
	uint8_t		is_32bit;

	/*  1 while running, 0 when paused/stopped.  */
	uint8_t		running;

	/*  See comment further up.  */
	uint8_t		delay_slot;

	/*  0-based CPU id, in an emulated SMP system.  */
	int		cpu_id;

	/*  A pointer to the main memory connected to this CPU.  */
	struct memory	*mem;

	int		(*run_instr)(struct cpu *cpu);
	int		(*memory_rw)(struct cpu *cpu,
			    struct memory *mem, uint64_t vaddr,
			    unsigned char *data, size_t len,
			    int writeflag, int cache_flags);
	int		(*translate_v2p)(struct cpu *, uint64_t vaddr,
			    uint64_t *return_paddr, int flags);
	void		(*update_translation_table)(struct cpu *,
			    uint64_t vaddr_page, unsigned char *host_page,
			    int writeflag, uint64_t paddr_page);
	void		(*invalidate_translation_caches)(struct cpu *,
			    uint64_t paddr, int flags);
	void		(*invalidate_code_translation)(struct cpu *,
			    uint64_t paddr, int flags);
	void		(*useremul_syscall)(struct cpu *cpu, uint32_t code);
	int		(*instruction_has_delayslot)(struct cpu *cpu,
			    unsigned char *ib);

	/*  The program counter. (For 32-bit modes, not all bits are used.)  */
	uint64_t	pc;

	/*  The current depth of function call tracing.  */
	int		trace_tree_depth;

	/*
	 *  If is_halted is true when an interrupt trap occurs, the pointer
	 *  to the next instruction to execute will be the instruction
	 *  following the halt instruction, not the halt instrucion itself.
	 *
	 *  If has_been_idling is true when printing the number of executed
	 *  instructions per second, "idling" is printed instead. (The number
	 *  of instrs per second when idling is meaningless anyway.)
	 */
	char		is_halted;
	char		has_been_idling;

	/*
	 *  Dynamic translation:
	 *
	 *  The number of translated instructions is assumed to be 1 per
	 *  instruction call. For each case where this differs from the
	 *  truth, n_translated_instrs should be modified. E.g. if 1000
	 *  instruction calls are done, and n_translated_instrs is 50, then
	 *  1050 emulated instructions were actually executed.
	 *
	 *  Note that it can also be adjusted negatively, that is, the way
	 *  to "get out" of a dyntrans loop is to set the current instruction
	 *  call pointer to the "nothing" instruction. This instruction
	 *  _decreases_ n_translated_instrs by 1. That way, once the dyntrans
	 *  loop exits, only real instructions will be counted, and not the
	 *  "nothing" instructions.
	 *
	 *  The translation cache is a relative large chunk of memory (say,
	 *  32 MB) which is used for translations. When it has been used up,
	 *  everything restarts from scratch.
	 *
	 *  translation_readahead is non-zero when translating instructions
	 *  ahead of the current (emulated) instruction pointer.
	 */

	int		translation_readahead;

	/*  Instruction translation cache:  */
	int		n_translated_instrs;
	unsigned char	*translation_cache;
	size_t		translation_cache_cur_ofs;


	/*
	 *  CPU-family dependent:
	 *
	 *  These contain everything ranging from general purpose registers,
	 *  control registers, memory management, status words, interrupt
	 *  specifics, etc.
	 */
	union {
		struct arm_cpu        arm;
		struct m88k_cpu       m88k;
		struct mips_cpu       mips;
		struct ppc_cpu        ppc;
		struct sh_cpu         sh;
	} cd;
};


/*  cpu.c:  */
struct cpu *cpu_new(struct memory *mem, struct machine *machine,
        int cpu_id, char *cpu_type_name);
void cpu_destroy(struct cpu *cpu);

void cpu_tlbdump(struct machine *m, int x, int rawflag);
void cpu_register_dump(struct machine *m, struct cpu *cpu,
	int gprs, int coprocs);
int cpu_disassemble_instr(struct machine *m, struct cpu *cpu,
	unsigned char *instr, int running, uint64_t addr);

void cpu_functioncall_trace(struct cpu *cpu, uint64_t f);
void cpu_functioncall_trace_return(struct cpu *cpu);

void cpu_create_or_reset_tc(struct cpu *cpu);

void cpu_run_init(struct machine *machine);
void cpu_run_deinit(struct machine *machine);

void cpu_dumpinfo(struct machine *m, struct cpu *cpu);
void cpu_list_available_types(void);
void cpu_show_cycles(struct machine *machine, int forced);

struct cpu_family *cpu_family_ptr_by_number(int arch);
void cpu_init(void);


#define	JUST_MARK_AS_NON_WRITABLE	1
#define	INVALIDATE_ALL			2
#define	INVALIDATE_PADDR		4
#define	INVALIDATE_VADDR		8
#define	INVALIDATE_VADDR_UPPER4		16	/*  useful for PPC emulation  */


/*  Note: 64-bit processors running in 32-bit mode use a 32-bit
    display format, even though the underlying data is 64-bits.  */
#define	CPU_SETTINGS_ADD_REGISTER64(name, var)				   \
	settings_add(cpu->settings, name, 1, SETTINGS_TYPE_UINT64,	   \
	    cpu->is_32bit? SETTINGS_FORMAT_HEX32 : SETTINGS_FORMAT_HEX64,  \
	    (void *) &(var));
#define	CPU_SETTINGS_ADD_REGISTER32(name, var)				   \
	settings_add(cpu->settings, name, 1, SETTINGS_TYPE_UINT32,	   \
	    SETTINGS_FORMAT_HEX32, (void *) &(var));
#define	CPU_SETTINGS_ADD_REGISTER16(name, var)				   \
	settings_add(cpu->settings, name, 1, SETTINGS_TYPE_UINT16,	   \
	    SETTINGS_FORMAT_HEX16, (void *) &(var));
#define	CPU_SETTINGS_ADD_REGISTER8(name, var)				   \
	settings_add(cpu->settings, name, 1, SETTINGS_TYPE_UINT8,	   \
	    SETTINGS_FORMAT_HEX8, (void *) &(var));


#define CPU_FAMILY_INIT(n,s)	int n ## _cpu_family_init(		\
	struct cpu_family *fp) {					\
	/*  Fill in the cpu_family struct with valid data for this arch.  */ \
	fp->name = strdup(s);						\
	fp->cpu_new = n ## _cpu_new;					\
	fp->list_available_types = n ## _cpu_list_available_types;	\
	fp->disassemble_instr = n ## _cpu_disassemble_instr;		\
	fp->register_dump = n ## _cpu_register_dump;			\
	fp->dumpinfo = n ## _cpu_dumpinfo;				\
	fp->functioncall_trace = n ## _cpu_functioncall_trace;		\
	fp->tlbdump = n ## _cpu_tlbdump;				\
	fp->init_tables = n ## _cpu_init_tables;			\
	return 1;							\
	}


#endif	/*  CPU_H  */
