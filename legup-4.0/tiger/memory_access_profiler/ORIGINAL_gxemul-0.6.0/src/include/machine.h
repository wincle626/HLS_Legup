#ifndef	MACHINE_H
#define	MACHINE_H

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
 */

#include <sys/types.h>

#include "symbol.h"

struct cpu_family;
struct diskimage;
struct emul;
struct fb_window;
struct machine_arcbios;
struct machine_pmax;
struct memory;
struct of_data;
struct settings;


/*  TODO: This should probably go away...  */
struct isa_pic_data {
	struct pic8259_data	*pic1;
	struct pic8259_data	*pic2;

	int			*pending_timer_interrupts;
	int			last_int;
};

struct breakpoints {
	int		n;

	/*  Arrays, with one element for each entry:  */
	char		**string;
	uint64_t	*addr;
};

struct statistics {
	char	*filename;
	FILE	*file;
	int	enabled;
	char	*fields;		/*  "vpi" etc.  */
};

struct tick_functions {
	int	n_entries;

	/*  Arrays, with one element for each entry:  */
	int	*ticks_till_next;
	int	*ticks_reset_value;
	void	(*(*f))(struct cpu *, void *);
	void	**extra;
};

struct x11_md {
	/*  X11/framebuffer stuff:  */
	int	in_use;
	int	scaledown;
	int	scaleup;
	int	n_display_names;
	char	**display_names;
	int	current_display_name_nr;	/*  updated by x11.c  */

	int	n_fb_windows;
	struct fb_window **fb_windows;
};


/*
 *  The machine struct:
 */
struct machine {
	/*  Pointer back to the emul struct we are in:  */
	struct emul *emul;

	/*  Settings:  */
	struct settings *settings;

	/*  Name as choosen by the user:  */
	char	*name;

	/*  Full "path" to the machine, e.g. "machine[0]":  */
	char	*path;

	int	arch;			/*  ARCH_MIPS, ARCH_PPC, ..  */
	int	machine_type;		/*  MACHINE_PMAX, ..  */
	int	machine_subtype;	/*  MACHINE_DEC_3MAX_5000, ..  */

	/*  Name set by code in src/machines/machine_*.c:  */
	char	*machine_name;

	/*  The serial number is mostly used when emulating multiple machines
	    in a network. nr_of_nics is the current nr of network cards, which
	    is useful when emulating multiple cards in one machine:  */
	int	serial_nr;
	int	nr_of_nics;

	/*  TODO: How about multiple cpu familys in one machine?  */
	struct cpu_family *cpu_family;

	struct memory *memory;

	int	main_console_handle;

	/*  Tick functions (e.g. hardware devices):  */
	struct tick_functions tick_functions;

	char	*cpu_name;  /*  TODO: remove this, there could be several
				cpus with different names in a machine  */
	int	byte_order_override;
	int	bootstrap_cpu;
	int	use_random_bootstrap_cpu;
	int	start_paused;
	int	ncpus;
	struct cpu **cpus;

	struct diskimage *first_diskimage;

	struct symbol_context symbol_context;

	int	random_mem_contents;
	int	physical_ram_in_mb;
	int	memory_offset_in_mb;
	int	prom_emulation;
	int	register_dump;
	int	arch_pagesize;

	int	bootdev_type;
	int	bootdev_id;
	char	*bootstr;
	char	*bootarg;

	/*  Breakpoints:  */
	struct breakpoints breakpoints;

	int	halt_on_nonexistant_memaccess;
	int	instruction_trace;
	int	show_nr_of_instructions;
	int	show_trace_tree;
	int	emulated_hz;
	int	allow_instruction_combinations;
	int	force_netboot;
	int	slow_serial_interrupts_hack_for_linux;
	uint64_t file_loaded_end_addr;
	char	*boot_kernel_filename;
	char	*boot_string_argument;
	int	exit_without_entering_debugger;
	int	n_gfx_cards;

	/*  Instruction statistics:  */
	struct statistics statistics;

	/*  X11/framebuffer stuff (per machine):  */
	struct x11_md x11_md;

	/*  Machine-dependent: (PROM stuff, etc.)  */
	union {
		struct machine_arcbios	*arc;
		struct machine_pmax	*pmax;
		struct of_data		*of_data;
	} md;

	/*  Bus-specific interrupt data:  */
	/*  TODO: Remove!  */
	struct isa_pic_data isa_pic_data;
};


/*  Tick function "prototype":  */
#define	DEVICE_TICK(x)	void dev_ ## x ## _tick(struct cpu *cpu, void *extra)


/*
 *  Machine emulation types:
 */

#define	ARCH_NOARCH		0
#define	ARCH_MIPS		1
#define	ARCH_PPC		2
#define	ARCH_ARM		3
#define	ARCH_SH			4
#define	ARCH_M88K		5

/*  MIPS:  */
#define	MACHINE_BAREMIPS	1000
#define	MACHINE_TESTMIPS	1001
#define	MACHINE_PMAX		1002
#define	MACHINE_COBALT		1003
#define	MACHINE_HPCMIPS		1004
#define	MACHINE_PS2		1005
#define	MACHINE_SGI		1006
#define	MACHINE_ARC		1007
#define	MACHINE_EVBMIPS		1008
#define	MACHINE_ALGOR		1009
#define	MACHINE_QEMU_MIPS	1010

/*  PPC:  */
#define	MACHINE_BAREPPC		2000
#define	MACHINE_TESTPPC		2001
#define	MACHINE_PMPPC		2002
#define	MACHINE_PREP		2003
#define	MACHINE_MACPPC		2004
#define	MACHINE_MVMEPPC		2005

/*  ARM:  */
#define	MACHINE_BAREARM		5000
#define	MACHINE_TESTARM		5001
#define	MACHINE_CATS		5002
#define	MACHINE_HPCARM		5003
#define	MACHINE_NETWINDER	5004
#define	MACHINE_IQ80321		5005
#define	MACHINE_IYONIX		5006
#define	MACHINE_QEMU_ARM	5007

/*  SH:  */
#define	MACHINE_BARESH		6000
#define	MACHINE_TESTSH		6001
#define	MACHINE_HPCSH		6002
#define	MACHINE_DREAMCAST	6003
#define	MACHINE_LANDISK		6004

/*  M88K:  */
#define	MACHINE_BAREM88K	7000
#define	MACHINE_TESTM88K	7001
#define	MACHINE_MVME88K		7002

/*  Other "pseudo"-machines:  */
#define	MACHINE_NONE		0

/*  DEC:  */
#define	MACHINE_DEC_PMAX_3100		1
#define	MACHINE_DEC_3MAX_5000		2
#define	MACHINE_DEC_3MIN_5000		3
#define	MACHINE_DEC_3MAXPLUS_5000	4
#define	MACHINE_DEC_5800		5
#define	MACHINE_DEC_5400		6
#define	MACHINE_DEC_MAXINE_5000		7
#define	MACHINE_DEC_5500		11
#define	MACHINE_DEC_MIPSMATE_5100	12

#define	DEC_PROM_CALLBACK_STRUCT	0xffffffffbfc04000ULL
#define	DEC_PROM_EMULATION		0xffffffffbfc08000ULL
#define	DEC_PROM_INITIAL_ARGV		(INITIAL_STACK_POINTER + 0x80)
#define	DEC_PROM_STRINGS		0xffffffffbfc20000ULL
#define	DEC_PROM_TCINFO			0xffffffffbfc2c000ULL
#define	DEC_MEMMAP_ADDR			0xffffffffbfc30000ULL

/*  HPCmips:  */
#define	MACHINE_HPCMIPS_CASIO_BE300		1
#define	MACHINE_HPCMIPS_CASIO_E105		2
#define	MACHINE_HPCMIPS_NEC_MOBILEPRO_770	3
#define	MACHINE_HPCMIPS_NEC_MOBILEPRO_780	4
#define	MACHINE_HPCMIPS_NEC_MOBILEPRO_800	5
#define	MACHINE_HPCMIPS_NEC_MOBILEPRO_880	6
#define	MACHINE_HPCMIPS_AGENDA_VR3		7
#define	MACHINE_HPCMIPS_IBM_WORKPAD_Z50		8

/*  HPCarm:  */
#define	MACHINE_HPCARM_IPAQ			1
#define	MACHINE_HPCARM_JORNADA720		2
#define	MACHINE_HPCARM_JORNADA728		3

/*  HPCsh:  */
#define	MACHINE_HPCSH_JORNADA680		1
#define	MACHINE_HPCSH_JORNADA690		2

/*  SGI and ARC:  */
#define	MACHINE_ARC_JAZZ_PICA		1
#define	MACHINE_ARC_JAZZ_MAGNUM		2

/*  Algor:  */
#define	MACHINE_ALGOR_P4032		1
#define	MACHINE_ALGOR_P5064		2

/*  EVBMIPS:  */
#define	MACHINE_EVBMIPS_MALTA		1
#define	MACHINE_EVBMIPS_MALTA_BE	2

/*  PReP:  */
#define	MACHINE_PREP_IBM6050		1
#define	MACHINE_PREP_MVME2400		2

/*  MacPPC:  TODO: Real model names  */
#define	MACHINE_MACPPC_G3		1
#define	MACHINE_MACPPC_G4		2
#define	MACHINE_MACPPC_G5		3

/*  MVMEPPC  */
#define	MACHINE_MVMEPPC_1600		1
#define	MACHINE_MVMEPPC_2100		2
#define	MACHINE_MVMEPPC_5500		3

/*  MVME88K  */
#define	MACHINE_MVME88K_187		1
#define	MACHINE_MVME88K_188		2
#define	MACHINE_MVME88K_197		3


/*  For the automachine system:  */
struct machine_entry_subtype {
	int			machine_subtype;/*  Old-style subtype  */
	const char		*name;		/*  Official name  */
	int			n_aliases;
	char			**aliases;	/*  Aliases  */
};

struct machine_entry {
	struct machine_entry	*next;

	/*  Machine type:  */
	int			arch;
	int			machine_type;	/*  Old-style type  */
	const char		*name;		/*  Official name  */
	int			n_aliases;
	char			**aliases;	/*  Aliases  */

	void			(*setup)(struct machine *, struct cpu *);
	void			(*set_default_cpu)(struct machine *);
	void			(*set_default_ram)(struct machine *);

	/*  Machine subtypes:  */
	int			n_subtypes;
	struct machine_entry_subtype **subtype;
};

#define	MACHINE_SETUP_TYPE(n)	void (*n)(struct machine *, struct cpu *)
#define	MACHINE_SETUP(x)	void machine_setup_ ## x(struct machine *machine, \
				    struct cpu *cpu)
#define	MACHINE_DEFAULT_CPU(x)	void machine_default_cpu_ ## x(struct machine *machine)
#define	MACHINE_DEFAULT_RAM(x)	void machine_default_ram_ ## x(struct machine *machine)
#define	MACHINE_REGISTER(x)	void machine_register_ ## x(void)
#define	MR_DEFAULT(x,name,arch,type) struct machine_entry 		\
	    *me = machine_entry_new(name,arch,type);			\
	me->setup = machine_setup_ ## x;				\
	me->set_default_cpu = machine_default_cpu_ ## x;		\
	machine_entry_register(me, arch);
void automachine_init(void);


/*  machine.c:  */
struct machine *machine_new(char *name, struct emul *emul, int id);
void machine_destroy(struct machine *machine);
int machine_name_to_type(char *stype, char *ssubtype,
	int *type, int *subtype, int *arch);
void machine_add_breakpoint_string(struct machine *machine, char *str);
void machine_add_tickfunction(struct machine *machine,
	void (*func)(struct cpu *, void *), void *extra, int clockshift);
void machine_statistics_init(struct machine *, char *fname);
void machine_register(char *name, MACHINE_SETUP_TYPE(setup));
void machine_setup(struct machine *);
void machine_memsize_fix(struct machine *);
void machine_default_cputype(struct machine *);
void machine_dumpinfo(struct machine *);
int machine_run(struct machine *machine);
void machine_list_available_types_and_cpus(void);
struct machine_entry *machine_entry_new(const char *name, 
	int arch, int oldstyle_type);
void machine_entry_add_alias(struct machine_entry *me, const char *name);
void machine_entry_add_subtype(struct machine_entry *me, const char *name,
	int oldstyle_subtype, ...);
void machine_entry_register(struct machine_entry *me, int arch);
void machine_init(void);


#endif	/*  MACHINE_H  */
