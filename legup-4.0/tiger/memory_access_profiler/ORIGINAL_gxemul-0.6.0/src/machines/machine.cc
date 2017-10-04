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
 *  Machine registry.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "cpu.h"
#include "device.h"
#include "diskimage.h"
#include "emul.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"
#include "settings.h"
#include "symbol.h"


/*  This is initialized by machine_init():  */
struct machine_entry *first_machine_entry = NULL;


/*
 *  machine_new():
 *
 *  Returns a reasonably initialized struct machine.
 */
struct machine *machine_new(char *name, struct emul *emul, int id)
{
	struct machine *m;

	CHECK_ALLOCATION(m = (struct machine *) malloc(sizeof(struct machine)));
	memset(m, 0, sizeof(struct machine));

	/*  Pointer back to the emul object that this machine belongs to:  */
	m->emul = emul;

	if (name != NULL)
		CHECK_ALLOCATION(m->name = strdup(name));

	/*  Full path, e.g. "machine[0]":  */
	CHECK_ALLOCATION(m->path = (char *) malloc(20));
	snprintf(m->path, 20, "machine[%i]", id);

	/*  Sane default values:  */
	m->serial_nr = 1;
	m->machine_type = MACHINE_NONE;
	m->machine_subtype = MACHINE_NONE;
	m->arch_pagesize = 4096;	/*  Should be overriden in
					    emul.c for other pagesizes.  */
	m->prom_emulation = 1;
	m->allow_instruction_combinations = 1;
	m->byte_order_override = NO_BYTE_ORDER_OVERRIDE;
	m->boot_kernel_filename = strdup("");
	m->boot_string_argument = NULL;
	m->x11_md.scaledown = 1;
	m->x11_md.scaleup = 1;
	m->n_gfx_cards = 1;
	symbol_init(&m->symbol_context);

	/*  Settings:  */
	m->settings = settings_new();
	settings_add(m->settings, "name", 0,
	    SETTINGS_TYPE_STRING, SETTINGS_FORMAT_STRING,
	    (void *) &m->name);
	settings_add(m->settings, "serial_nr", 0,
	    SETTINGS_TYPE_INT, SETTINGS_FORMAT_DECIMAL,
	    (void *) &m->serial_nr);
	settings_add(m->settings, "arch_pagesize", 0,
	    SETTINGS_TYPE_INT, SETTINGS_FORMAT_DECIMAL,
	    (void *) &m->arch_pagesize);
	settings_add(m->settings, "prom_emulation", 0,
	    SETTINGS_TYPE_INT, SETTINGS_FORMAT_YESNO,
	    (void *) &m->prom_emulation);
	settings_add(m->settings, "allow_instruction_combinations", 0,
	    SETTINGS_TYPE_INT, SETTINGS_FORMAT_YESNO,
	    (void *) &m->allow_instruction_combinations);
	settings_add(m->settings, "n_gfx_cards", 0,
	    SETTINGS_TYPE_INT, SETTINGS_FORMAT_DECIMAL,
	    (void *) &m->n_gfx_cards);
	settings_add(m->settings, "statistics_enabled", 1,
	    SETTINGS_TYPE_INT, SETTINGS_FORMAT_YESNO,
	    (void *) &m->statistics.enabled);

	return m;
}


/*
 *  machine_destroy():
 *
 *  Destroys a machine object.
 */
void machine_destroy(struct machine *machine)
{
	int i;

	for (i=0; i<machine->ncpus; i++)
		cpu_destroy(machine->cpus[i]);

	if (machine->name != NULL)
		free(machine->name);

	if (machine->path != NULL)
		free(machine->path);

	/*  Remove any remaining level-1 settings:  */
	settings_remove_all(machine->settings);
	settings_destroy(machine->settings);

	free(machine);
}


/*
 *  machine_name_to_type():
 *
 *  Take a type and a subtype as strings, and convert them into numeric
 *  values used internally throughout the code.
 *
 *  Return value is 1 on success, 0 if there was no match.
 *  Also, any errors/warnings are printed using fatal()/debug().
 */
int machine_name_to_type(char *stype, char *ssubtype,
	int *type, int *subtype, int *arch)
{
	struct machine_entry *me;
	int i, j, k, nmatches = 0;

	*type = MACHINE_NONE;
	*subtype = 0;

	/*  Check stype, and optionally ssubtype:  */
	me = first_machine_entry;
	while (me != NULL) {
		for (i=0; i<me->n_aliases; i++)
			if (strcasecmp(me->aliases[i], stype) == 0) {
				/*  Found a type:  */
				*type = me->machine_type;
				*arch = me->arch;

				if (me->n_subtypes == 0)
					return 1;

				/*  Check for subtype:  */
				for (j=0; j<me->n_subtypes; j++)
					for (k=0; k<me->subtype[j]->n_aliases;
					    k++)
						if (strcasecmp(ssubtype,
						    me->subtype[j]->aliases[k]
						    ) == 0) {
							*subtype = me->subtype[
							    j]->machine_subtype;
							return 1;
						}

				fatal("Unknown subtype '%s' for emulation"
				    " '%s'\n", ssubtype, stype);
				if (!ssubtype[0])
					fatal("(Maybe you forgot the -e"
					    " command line option?)\n");
				exit(1);
			}

		me = me->next;
	}

	/*  Not found? Then just check ssubtype:  */
	me = first_machine_entry;
	while (me != NULL) {
		if (me->n_subtypes == 0) {
			me = me->next;
			continue;
		}

		/*  Check for subtype:  */
		for (j=0; j<me->n_subtypes; j++)
			for (k=0; k<me->subtype[j]->n_aliases; k++)
				if (strcasecmp(ssubtype, me->subtype[j]->
				    aliases[k]) == 0) {
					*type = me->machine_type;
					*arch = me->arch;
					*subtype = me->subtype[j]->
					    machine_subtype;
					nmatches ++;
				}

		me = me->next;
	}

	switch (nmatches) {
	case 0:	fatal("\nSorry, emulation \"%s\"", stype);
		if (ssubtype != NULL && ssubtype[0] != '\0')
			fatal(" (subtype \"%s\")", ssubtype);
		fatal(" is unknown.\n");
		break;
	case 1:	return 1;
	default:fatal("\nSorry, multiple matches for \"%s\"", stype);
		if (ssubtype != NULL && ssubtype[0] != '\0')
			fatal(" (subtype \"%s\")", ssubtype);
		fatal(".\n");
	}

	*type = MACHINE_NONE;
	*subtype = 0;

	fatal("Use the -H command line option to get a list of "
	    "available types and subtypes.\n\n");

	return 0;
}


/*
 *  machine_add_breakpoint_string():
 *
 *  Add a breakpoint string to the machine. Later (in emul.c) these will be
 *  converted to actual breakpoints.
 */
void machine_add_breakpoint_string(struct machine *machine, char *str)
{
	int n = machine->breakpoints.n + 1;

	CHECK_ALLOCATION(machine->breakpoints.string = (char **)
	    realloc(machine->breakpoints.string, n * sizeof(char *)));
	CHECK_ALLOCATION(machine->breakpoints.addr = (uint64_t *)
	    realloc(machine->breakpoints.addr, n * sizeof(uint64_t)));
	CHECK_ALLOCATION(machine->breakpoints.string[machine->breakpoints.n] =
	    strdup(optarg));

	machine->breakpoints.addr[machine->breakpoints.n] = 0;
	machine->breakpoints.n ++;
}


/*
 *  machine_add_tickfunction():
 *
 *  Adds a tick function (a function called every now and then, depending on
 *  clock cycle count) to a machine.
 *
 *  If tickshift is non-zero, a tick will occur every (1 << tickshift) cycles.
 *  This is used for the normal (fast dyntrans) emulation modes.
 *
 *  If tickshift is zero, then this is a cycle-accurate tick function.
 *  The hz value is used in this case.
 */
void machine_add_tickfunction(struct machine *machine, void (*func)
	(struct cpu *, void *), void *extra, int tickshift)
{
	int n = machine->tick_functions.n_entries;

	CHECK_ALLOCATION(machine->tick_functions.ticks_till_next = (int *) realloc(
	    machine->tick_functions.ticks_till_next, (n+1) * sizeof(int)));
	CHECK_ALLOCATION(machine->tick_functions.ticks_reset_value = (int *) realloc(
	    machine->tick_functions.ticks_reset_value, (n+1) * sizeof(int)));
	CHECK_ALLOCATION(machine->tick_functions.f = (void (**)(cpu*,void*)) realloc(
	    machine->tick_functions.f, (n+1) * sizeof(void *)));
	CHECK_ALLOCATION(machine->tick_functions.extra = (void **) realloc(
	    machine->tick_functions.extra, (n+1) * sizeof(void *)));

	/*
	 *  The dyntrans subsystem wants to run code in relatively
	 *  large chunks without checking for external interrupts;
	 *  too low tickshifts are not allowed.
	 */
	if (tickshift < N_SAFE_DYNTRANS_LIMIT_SHIFT) {
		fatal("ERROR! tickshift = %i, less than "
		    "N_SAFE_DYNTRANS_LIMIT_SHIFT (%i)\n",
		    tickshift, N_SAFE_DYNTRANS_LIMIT_SHIFT);
		exit(1);
	}

	machine->tick_functions.ticks_till_next[n]   = 0;
	machine->tick_functions.ticks_reset_value[n] = 1 << tickshift;
	machine->tick_functions.f[n]                 = func;
	machine->tick_functions.extra[n]             = extra;

	machine->tick_functions.n_entries = n + 1;
}


/*
 *  machine_statistics_init():
 *
 *  Initialize the parts of a machine struct that deal with instruction
 *  statistics gathering.
 *
 *  Note: The fname argument contains "flags:filename".
 */
void machine_statistics_init(struct machine *machine, char *fname)
{
	int n_fields = 0;
	char *pcolon = fname;
	const char *mode = "a";	/*  Append by default  */

	machine->allow_instruction_combinations = 0;

	if (machine->statistics.fields != NULL) {
		fprintf(stderr, "Only one -s option is allowed.\n");
		exit(1);
	}

	machine->statistics.enabled = 1;
	CHECK_ALLOCATION(machine->statistics.fields = (char *) malloc(1));
	machine->statistics.fields[0] = '\0';

	while (*pcolon && *pcolon != ':')
		pcolon ++;

	if (*pcolon != ':') {
		fprintf(stderr, "The syntax for the -s option is:    "
		    "-s flags:filename\nYou omitted the flags. Run g"
		    "xemul -h for a list of available flags.\n");
		exit(1);
	}

	while (*fname != ':') {
		switch (*fname) {

		/*  Type flags:  */
		case 'v':
		case 'i':
		case 'p':
			CHECK_ALLOCATION(machine->statistics.fields = (char *) realloc(
			    machine->statistics.fields, strlen(
			    machine->statistics.fields) + 2));
			machine->statistics.fields[n_fields ++] = *fname;
			machine->statistics.fields[n_fields] = '\0';
			break;

		/*  Optional flags:  */
		case 'o':
			mode = "w";
			break;
		case 'd':
			machine->statistics.enabled = 0;
			break;

		default:fprintf(stderr, "Unknown flag '%c' used with the"
			    " -s option. Aborting.\n", *fname);
			exit(1);
		}
		fname ++;
	}

	fname ++;	/*  point to the filename after the colon  */

	CHECK_ALLOCATION(machine->statistics.filename = strdup(fname));
	machine->statistics.file = fopen(machine->statistics.filename, mode);
}


/*
 *  machine_dumpinfo():
 *
 *  Dumps info about a machine in some kind of readable format. (Used by
 *  the 'machine' debugger command.)
 */
void machine_dumpinfo(struct machine *m)
{
	int i;

	debug("serial nr: %i", m->serial_nr);
	if (m->nr_of_nics > 0)
		debug("  (nr of NICs: %i)", m->nr_of_nics);
	debug("\n");

	debug("memory: %i MB", m->physical_ram_in_mb);
	if (m->memory_offset_in_mb != 0)
		debug(" (offset by %i MB)", m->memory_offset_in_mb);
	if (m->random_mem_contents)
		debug(", randomized contents");
	debug("\n");

	if (!m->prom_emulation)
		debug("PROM emulation disabled\n");

	for (i=0; i<m->ncpus; i++)
		cpu_dumpinfo(m, m->cpus[i]);

	if (m->ncpus > 1)
		debug("Bootstrap cpu is nr %i\n", m->bootstrap_cpu);

	if (m->slow_serial_interrupts_hack_for_linux)
		debug("Using slow_serial_interrupts_hack_for_linux\n");

	if (m->x11_md.in_use) {
		debug("Using X11");
		if (m->x11_md.scaledown > 1)
			debug(", scaledown %i", m->x11_md.scaledown);
		if (m->x11_md.scaleup > 1)
			debug(", scaleup %i", m->x11_md.scaleup);
		if (m->x11_md.n_display_names > 0) {
			for (i=0; i<m->x11_md.n_display_names; i++) {
				debug(i? ", " : " (");
				debug("\"%s\"", m->x11_md.display_names[i]);
			}
			debug(")");
		}
		debug("\n");
	}

	diskimage_dump_info(m);

	if (m->force_netboot)
		debug("Forced netboot\n");
}


/*
 *  machine_setup():
 *
 *  This (rather large) function initializes memory, registers, and/or devices
 *  required by specific machine emulations.
 */
void machine_setup(struct machine *machine)
{
	struct memory *mem;
	struct machine_entry *me;

	/*  Abreviation:  :-)  */
	struct cpu *cpu = machine->cpus[machine->bootstrap_cpu];

	machine->bootdev_id = diskimage_bootdev(machine,
	    &machine->bootdev_type);

	mem = cpu->mem;
	machine->machine_name = NULL;

	/*  TODO: Move this somewhere else?  */
	if (machine->boot_string_argument == NULL) {
		switch (machine->machine_type) {
		case MACHINE_ARC:
			machine->boot_string_argument = strdup("-aN");
			break;
		case MACHINE_CATS:
			machine->boot_string_argument = strdup("-A");
			break;
		case MACHINE_PMAX:
			machine->boot_string_argument = strdup("-a");
			break;
		default:
			/*  Important, because boot_string_argument should
			    not be set to NULL:  */
			machine->boot_string_argument = strdup("");
		}
	}


	/*
	 *  If the machine has a setup function in src/machines/machine_*.c
	 *  then use that one, otherwise use the old hardcoded stuff here:
	 */

	me = first_machine_entry;
	while (me != NULL) {
		if (machine->machine_type == me->machine_type &&
		    me->setup != NULL) {
			me->setup(machine, cpu);
			break;
		}
		me = me->next;
	}

	if (me == NULL) {
		fatal("Unknown emulation type %i\n", machine->machine_type);
		exit(1);
	}

	if (machine->machine_name != NULL)
		debug("machine: %s", machine->machine_name);

	if (machine->emulated_hz > 0)
		debug(" (%.2f MHz)", (float)machine->emulated_hz / 1000000);
	debug("\n");

	if (machine->bootstr != NULL) {
		debug("bootstring%s: %s", (machine->bootarg!=NULL &&
		    strlen(machine->bootarg) >= 1)? "(+bootarg)" : "",
		    machine->bootstr);
		if (machine->bootarg != NULL && strlen(machine->bootarg) >= 1)
			debug(" %s", machine->bootarg);
		debug("\n");
	}
}


/*
 *  machine_memsize_fix():
 *
 *  Sets physical_ram_in_mb (if not already set), and memory_offset_in_mb,
 *  depending on machine type.
 */
void machine_memsize_fix(struct machine *m)
{
	if (m == NULL) {
		fatal("machine_defaultmemsize(): m == NULL?\n");
		exit(1);
	}

	if (m->physical_ram_in_mb == 0) {
		struct machine_entry *me = first_machine_entry;
		while (me != NULL) {
			if (m->machine_type == me->machine_type &&
			    me->set_default_ram != NULL) {
				me->set_default_ram(m);
				break;
			}
			me = me->next;
		}
	}

	/*  Special SGI memory offsets:  (TODO: move this somewhere else)  */
	if (m->machine_type == MACHINE_SGI) {
		switch (m->machine_subtype) {
		case 20:
		case 22:
		case 24:
		case 26:
			m->memory_offset_in_mb = 128;
			break;
		case 28:
		case 30:
			m->memory_offset_in_mb = 512;
			break;
		}
	}

	if (m->physical_ram_in_mb == 0)
		m->physical_ram_in_mb = DEFAULT_RAM_IN_MB;
}


/*
 *  machine_default_cputype():
 *
 *  Sets m->cpu_name, if it isn't already set, depending on the machine type.
 */
void machine_default_cputype(struct machine *m)
{
	struct machine_entry *me;

	if (m == NULL) {
		fatal("machine_default_cputype(): m == NULL?\n");
		exit(1);
	}

	/*  Already set? Then return.  */
	if (m->cpu_name != NULL)
		return;

	me = first_machine_entry;
	while (me != NULL) {
		if (m->machine_type == me->machine_type &&
		    me->set_default_cpu != NULL) {
			me->set_default_cpu(m);
			break;
		}
		me = me->next;
	}

	if (m->cpu_name == NULL) {
		fprintf(stderr, "machine_default_cputype(): no default"
		    " cpu for machine type %i subtype %i\n",
		    m->machine_type, m->machine_subtype);
		exit(1);
	}
}


/*****************************************************************************/


/*
 *  machine_run():
 *
 *  Run one or more instructions on all CPUs in this machine. (Usually,
 *  around N_SAFE_DYNTRANS_LIMIT instructions will be run by the dyntrans
 *  system.)
 *
 *  Return value is 1 if any CPU in this machine is still running,
 *  or 0 if all CPUs are stopped.
 */
int machine_run(struct machine *machine)
{
	struct cpu **cpus = machine->cpus;
	int ncpus = machine->ncpus, cpu0instrs = 0, i, te;

	for (i=0; i<ncpus; i++) {
		if (cpus[i]->running) {
			int instrs_run = cpus[i]->run_instr(cpus[i]);
			if (i == 0)
				cpu0instrs += instrs_run;
		}
	}

	/*
	 *  Hardware 'ticks':  (clocks, interrupt sources...)
	 *
	 *  Here, cpu0instrs is the number of instructions executed on cpu0.
	 *
	 *  TODO: This should be redesigned into some "mainbus" stuff instead!
	 */

	for (te=0; te<machine->tick_functions.n_entries; te++) {
		machine->tick_functions.ticks_till_next[te] -= cpu0instrs;
		if (machine->tick_functions.ticks_till_next[te] <= 0) {
			while (machine->tick_functions.ticks_till_next[te]<=0) {
				machine->tick_functions.ticks_till_next[te] +=
				    machine->tick_functions.
				    ticks_reset_value[te];
			}

			machine->tick_functions.f[te](cpus[0],
			    machine->tick_functions.extra[te]);
		}
	}

	/*  Is any CPU still alive?  */
	for (i=0; i<ncpus; i++)
		if (cpus[i]->running)
			return 1;

	return 0;
}


/*****************************************************************************/


/*
 *  machine_entry_new():
 *
 *  This function creates a new machine_entry struct, and fills it with some
 *  valid data; it is up to the caller to add additional data that weren't
 *  passed as arguments to this function, such as alias names and machine
 *  subtypes.
 */
struct machine_entry *machine_entry_new(const char *name, int arch,
	int oldstyle_type)
{
	struct machine_entry *me;

	CHECK_ALLOCATION(me = (struct machine_entry *) malloc(sizeof(struct machine_entry)));
	memset(me, 0, sizeof(struct machine_entry));

	me->name = name;
	me->arch = arch;
	me->machine_type = oldstyle_type;
	me->n_aliases = 0;
	me->aliases = NULL;
	me->n_subtypes = 0;
	me->setup = NULL;

	return me;
}


/*
 *  machine_entry_add_alias():
 *
 *  This function adds an "alias" to a machine entry.
 */
void machine_entry_add_alias(struct machine_entry *me, const char *name)
{
	me->n_aliases ++;

	CHECK_ALLOCATION(me->aliases = (char **) realloc(me->aliases,
	    sizeof(char *) * me->n_aliases));

	me->aliases[me->n_aliases - 1] = (char *) name;
}


/*
 *  machine_entry_add_subtype():
 *
 *  This function adds a subtype to a machine entry. The argument list after
 *  oldstyle_subtype is a list of one or more char *, followed by NULL. E.g.:
 *
 *	machine_entry_add_subtype(me, "Machine X", MACHINE_X,
 *	    "machine-x", "x", NULL);
 */
void machine_entry_add_subtype(struct machine_entry *me, const char *name,
	int oldstyle_subtype, ...)
{
	va_list argp;
	struct machine_entry_subtype *mes;

	/*  Allocate a new subtype struct:  */
	CHECK_ALLOCATION(mes = (struct machine_entry_subtype *)
	    malloc(sizeof(struct machine_entry_subtype)));

	/*  Add the subtype to the machine entry:  */
	me->n_subtypes ++;
	CHECK_ALLOCATION(me->subtype = (struct
	    machine_entry_subtype **) realloc(me->subtype, sizeof(struct
	    machine_entry_subtype *) * me->n_subtypes));
	me->subtype[me->n_subtypes - 1] = mes;

	/*  Fill the struct with subtype data:  */
	memset(mes, 0, sizeof(struct machine_entry_subtype));
	mes->name = name;
	mes->machine_subtype = oldstyle_subtype;

	/*  ... and all aliases:  */
	mes->n_aliases = 0;
	mes->aliases = NULL;

	va_start(argp, oldstyle_subtype);

	for (;;) {
		char *s = va_arg(argp, char *);
		if (s == NULL)
			break;

		mes->n_aliases ++;
		CHECK_ALLOCATION(mes->aliases = (char **)
		    realloc(mes->aliases, sizeof(char *) * mes->n_aliases));

		mes->aliases[mes->n_aliases - 1] = s;
	}

	va_end(argp);
}


/*
 *  machine_entry_register():
 *
 *  Inserts a new machine_entry into the machine entries list.
 */
void machine_entry_register(struct machine_entry *me, int arch)
{
	struct machine_entry *prev, *next;

	/*  Only insert it if the architecture is implemented in this
	    emulator configuration:  */
	if (cpu_family_ptr_by_number(arch) == NULL)
		return;

	prev = NULL;
	next = first_machine_entry;

	for (;;) {
		if (next == NULL)
			break;
		if (strcasecmp(me->name, next->name) < 0)
			break;

		prev = next;
		next = next->next;
	}

	if (prev != NULL)
		prev->next = me;
	else
		first_machine_entry = me;
	me->next = next;
}


/*
 *  machine_list_available_types_and_cpus():
 *
 *  List all available machine types (for example when running the emulator
 *  with just -H as command line argument).
 */
void machine_list_available_types_and_cpus(void)
{
	struct machine_entry *me;
	int iadd = DEBUG_INDENTATION * 2;

	debug("Available CPU types:\n\n");

	debug_indentation(iadd);
	cpu_list_available_types();
	debug_indentation(-iadd);  

	debug("\nMost of the CPU types are bogus, and not really implemented."
	    " The main effect of\nselecting a specific CPU type is to choose "
	    "what kind of 'id' it will have.\n\nAvailable machine types (with "
	    "aliases) and their subtypes:\n\n");

	debug_indentation(iadd);
	me = first_machine_entry;

	if (me == NULL)
		fatal("No machines defined!\n");

	while (me != NULL) {
		int i, j, iadd = DEBUG_INDENTATION;

		debug("%s [%s] (", me->name,
		    cpu_family_ptr_by_number(me->arch)->name);
		for (i=0; i<me->n_aliases; i++)
			debug("%s\"%s\"", i? ", " : "", me->aliases[i]);
		debug(")\n");

		debug_indentation(iadd);
		for (i=0; i<me->n_subtypes; i++) {
			struct machine_entry_subtype *mes;
			mes = me->subtype[i];
			debug("- %s", mes->name);
			debug(" (");
			for (j=0; j<mes->n_aliases; j++)
				debug("%s\"%s\"", j? ", " : "",
				    mes->aliases[j]);
			debug(")\n");
		}
		debug_indentation(-iadd);

		me = me->next;
	}
	debug_indentation(-iadd);

	debug("\nMost of the machine types are bogus too. Please read the "
	    "GXemul documentation\nfor information about which machine types "
	    "that actually work. Use the alias\nwhen selecting a machine type "
	    "or subtype, not the real name.\n");

	debug("\n");
}


/*
 *  machine_init():
 *
 *  This function should be called before any other machine_*() function
 *  is used.  automachine_init() registers all machines in src/machines/.
 */
void machine_init(void)
{
	if (first_machine_entry != NULL) {
		fatal("machine_init(): already called?\n");
		exit(1);
	}

	automachine_init();
}

