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
 *  Common routines for CPU emulation. (Not specific to any CPU type.)
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <string.h>

#include "cpu.h"
#include "machine.h"
#include "memory.h"
#include "settings.h"
#include "timer.h"


extern size_t dyntrans_cache_size;

static struct cpu_family *first_cpu_family = NULL;


/*
 *  cpu_new():
 *
 *  Create a new cpu object.  Each family is tried in sequence until a
 *  CPU family recognizes the cpu_type_name.
 *
 *  If there was no match, NULL is returned. Otherwise, a pointer to an
 *  initialized cpu struct is returned.
 */
struct cpu *cpu_new(struct memory *mem, struct machine *machine,
        int cpu_id, char *name)
{
	struct cpu *cpu;
	struct cpu_family *fp;
	char *cpu_type_name;
	char tmpstr[30];

	if (name == NULL) {
		fprintf(stderr, "cpu_new(): cpu name = NULL?\n");
		exit(1);
	}

	CHECK_ALLOCATION(cpu_type_name = strdup(name));

	cpu = (struct cpu *) zeroed_alloc(sizeof(struct cpu));

	CHECK_ALLOCATION(cpu->path = (char *) malloc(strlen(machine->path) + 15));
	snprintf(cpu->path, strlen(machine->path) + 15,
	    "%s.cpu[%i]", machine->path, cpu_id);

	cpu->memory_rw  = NULL;
	cpu->name       = cpu_type_name;
	cpu->mem        = mem;
	cpu->machine    = machine;
	cpu->cpu_id     = cpu_id;
	cpu->byte_order = EMUL_UNDEFINED_ENDIAN;
	cpu->running    = 0;

	/*  Create settings, and attach to the machine:  */
	cpu->settings = settings_new();
	snprintf(tmpstr, sizeof(tmpstr), "cpu[%i]", cpu_id);
	settings_add(machine->settings, tmpstr, 1,
	    SETTINGS_TYPE_SUBSETTINGS, 0, cpu->settings);

	settings_add(cpu->settings, "name", 0, SETTINGS_TYPE_STRING,
	    SETTINGS_FORMAT_STRING, (void *) &cpu->name);
	settings_add(cpu->settings, "running", 0, SETTINGS_TYPE_UINT8,
	    SETTINGS_FORMAT_YESNO, (void *) &cpu->running);

	cpu_create_or_reset_tc(cpu);

	fp = first_cpu_family;

	while (fp != NULL) {
		if (fp->cpu_new != NULL) {
			if (fp->cpu_new(cpu, mem, machine, cpu_id,
			    cpu_type_name)) {
				/*  Sanity check:  */
				if (cpu->memory_rw == NULL) {
					fatal("\ncpu_new(): memory_rw == "
					    "NULL\n");
					exit(1);
				}
				break;
			}
		}

		fp = fp->next;
	}

	if (fp == NULL) {
		fatal("\ncpu_new(): unknown cpu type '%s'\n", cpu_type_name);
		return NULL;
	}

	fp->init_tables(cpu);

	if (cpu->byte_order == EMUL_UNDEFINED_ENDIAN) {
		fatal("\ncpu_new(): Internal bug: Endianness not set.\n");
		exit(1);
	}

	return cpu;
}


/*
 *  cpu_destroy():
 *
 *  Destroy a cpu object.
 */
void cpu_destroy(struct cpu *cpu)
{
	settings_remove(cpu->settings, "name");
	settings_remove(cpu->settings, "running");

	/*  Remove any remaining level-1 settings:  */
	settings_remove_all(cpu->settings);

	settings_destroy(cpu->settings);

	if (cpu->path != NULL)
		free(cpu->path);

	/*  TODO: This assumes that zeroed_alloc() actually succeeded
	    with using mmap(), and not malloc()!  */
	munmap((void *)cpu, sizeof(struct cpu));
}


/*
 *  cpu_tlbdump():
 *
 *  Called from the debugger to dump the TLB in a readable format.
 *  x is the cpu number to dump, or -1 to dump all CPUs.
 *                                              
 *  If rawflag is nonzero, then the TLB contents isn't formated nicely,
 *  just dumped.
 */
void cpu_tlbdump(struct machine *m, int x, int rawflag)
{
	if (m->cpu_family == NULL || m->cpu_family->tlbdump == NULL)
		fatal("cpu_tlbdump(): NULL\n");
	else
		m->cpu_family->tlbdump(m, x, rawflag);
}


/*
 *  cpu_disassemble_instr():
 *
 *  Convert an instruction word into human readable format, for instruction
 *  tracing.
 */
int cpu_disassemble_instr(struct machine *m, struct cpu *cpu,
	unsigned char *instr, int running, uint64_t addr)
{
	if (m->cpu_family == NULL || m->cpu_family->disassemble_instr == NULL) {
		fatal("cpu_disassemble_instr(): NULL\n");
		return 0;
	} else
		return m->cpu_family->disassemble_instr(cpu, instr,
		    running, addr);
}


/*                       
 *  cpu_register_dump():
 *
 *  Dump cpu registers in a relatively readable format.
 *
 *  gprs: set to non-zero to dump GPRs. (CPU dependent.)
 *  coprocs: set bit 0..x to dump registers in coproc 0..x. (CPU dependent.)
 */
void cpu_register_dump(struct machine *m, struct cpu *cpu,
	int gprs, int coprocs)
{
	if (m->cpu_family == NULL || m->cpu_family->register_dump == NULL)
		fatal("cpu_register_dump(): NULL\n");
	else
		m->cpu_family->register_dump(cpu, gprs, coprocs);
}


/*
 *  cpu_functioncall_trace():
 *
 *  This function should be called if machine->show_trace_tree is enabled, and
 *  a function call is being made. f contains the address of the function.
 */
void cpu_functioncall_trace(struct cpu *cpu, uint64_t f)
{
	int show_symbolic_function_name = 1;
	int i, n_args = -1;
	char *symbol;
	uint64_t offset;

	/*  Special hack for M88K userspace:  */
	if (cpu->machine->arch == ARCH_M88K &&
	    !(cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_MODE))
		show_symbolic_function_name = 0;

	if (cpu->machine->ncpus > 1)
		fatal("cpu%i:\t", cpu->cpu_id);

	if (cpu->trace_tree_depth > 100)
		cpu->trace_tree_depth = 100;
	for (i=0; i<cpu->trace_tree_depth; i++)
		fatal("  ");

	cpu->trace_tree_depth ++;

	fatal("<");
	symbol = get_symbol_name_and_n_args(&cpu->machine->symbol_context,
	    f, &offset, &n_args);
	if (symbol != NULL && show_symbolic_function_name)
		fatal("%s", symbol);
	else {
		if (cpu->is_32bit)
			fatal("0x%"PRIx32, (uint32_t) f);
		else
			fatal("0x%"PRIx64, (uint64_t) f);
	}
	fatal("(");

	if (cpu->machine->cpu_family->functioncall_trace != NULL)
		cpu->machine->cpu_family->functioncall_trace(cpu, n_args);

	fatal(")>\n");

#ifdef PRINT_MEMORY_CHECKSUM
	/*  Temporary hack for finding bugs:  */
	fatal("call chksum=%016"PRIx64"\n", memory_checksum(cpu->mem));
#endif
}


/*
 *  cpu_functioncall_trace_return():
 *
 *  This function should be called if machine->show_trace_tree is enabled, and
 *  a function is being returned from.
 *
 *  TODO: Print return value? This could be implemented similar to the
 *  cpu->functioncall_trace function call above.
 */
void cpu_functioncall_trace_return(struct cpu *cpu)
{
	cpu->trace_tree_depth --;
	if (cpu->trace_tree_depth < 0)
		cpu->trace_tree_depth = 0;
}


/*
 *  cpu_create_or_reset_tc():
 *
 *  Create the translation cache in memory (ie allocate memory for it), if
 *  necessary, and then reset it to an initial state.
 */
void cpu_create_or_reset_tc(struct cpu *cpu)
{
	size_t s = dyntrans_cache_size + DYNTRANS_CACHE_MARGIN;

	if (cpu->translation_cache == NULL)
		cpu->translation_cache = (unsigned char *) zeroed_alloc(s);

	/*  Create an empty table at the beginning of the translation cache:  */
	memset(cpu->translation_cache, 0, sizeof(uint32_t)
	    * N_BASE_TABLE_ENTRIES);

	cpu->translation_cache_cur_ofs =
	    N_BASE_TABLE_ENTRIES * sizeof(uint32_t);

	/*
	 *  There might be other translation pointers that still point to
	 *  within the translation_cache region. Let's invalidate those too:
	 */
	if (cpu->invalidate_code_translation != NULL)
		cpu->invalidate_code_translation(cpu, 0, INVALIDATE_ALL);
}


/*
 *  cpu_dumpinfo():
 *
 *  Dumps info about a CPU using debug(). "cpu0: CPUNAME, running" (or similar)
 *  is outputed, and it is up to CPU dependent code to complete the line.
 */
void cpu_dumpinfo(struct machine *m, struct cpu *cpu)
{
	debug("cpu%i: %s, %s", cpu->cpu_id, cpu->name,
	    cpu->running? "running" : "stopped");

	if (m->cpu_family == NULL || m->cpu_family->dumpinfo == NULL)
		fatal("cpu_dumpinfo(): NULL\n");
	else
		m->cpu_family->dumpinfo(cpu);
}


/*
 *  cpu_list_available_types():
 *
 *  Print a list of available CPU types for each cpu family.
 */
void cpu_list_available_types(void)
{
	struct cpu_family *fp;
	int iadd = DEBUG_INDENTATION;

	fp = first_cpu_family;

	if (fp == NULL) {
		debug("No CPUs defined!\n");
		return;
	}

	while (fp != NULL) {
		debug("%s:\n", fp->name);
		debug_indentation(iadd);
		if (fp->list_available_types != NULL)
			fp->list_available_types();
		else
			debug("(internal error: list_available_types"
			    " = NULL)\n");
		debug_indentation(-iadd);

		fp = fp->next;
	}
}


/*
 *  cpu_run_deinit():
 *
 *  Shuts down all CPUs in a machine when ending a simulation. (This function
 *  should only need to be called once for each machine.)
 */
void cpu_run_deinit(struct machine *machine)
{
	int te;

	/*
	 *  Two last ticks of every hardware device.  This will allow e.g.
	 *  framebuffers to draw the last updates to the screen before halting.
	 *
	 *  TODO: This should be refactored when redesigning the mainbus
	 *        concepts!
	 */
        for (te=0; te<machine->tick_functions.n_entries; te++) {
		machine->tick_functions.f[te](machine->cpus[0],
		    machine->tick_functions.extra[te]);
		machine->tick_functions.f[te](machine->cpus[0],
		    machine->tick_functions.extra[te]);
	}

	if (machine->show_nr_of_instructions)
		cpu_show_cycles(machine, 1);

	fflush(stdout);
}


/*
 *  cpu_show_cycles():
 *
 *  If show_nr_of_instructions is on, then print a line to stdout about how
 *  many instructions/cycles have been executed so far.
 */
void cpu_show_cycles(struct machine *machine, int forced)
{
	uint64_t offset, pc;
	char *symbol;
	int64_t mseconds, ninstrs, is, avg;
	struct timeval tv;
	struct cpu *cpu = machine->cpus[machine->bootstrap_cpu];

	static int64_t mseconds_last = 0;
	static int64_t ninstrs_last = -1;

	pc = cpu->pc;

	gettimeofday(&tv, NULL);
	mseconds = (tv.tv_sec - cpu->starttime.tv_sec) * 1000
	         + (tv.tv_usec - cpu->starttime.tv_usec) / 1000;

	if (mseconds == 0)
		mseconds = 1;

	if (mseconds - mseconds_last == 0)
		mseconds ++;

	ninstrs = cpu->ninstrs_since_gettimeofday;

	/*  RETURN here, unless show_nr_of_instructions (-N) is turned on:  */
	if (!machine->show_nr_of_instructions && !forced)
		goto do_return;

	printf("[ %"PRIi64" instrs", (int64_t) cpu->ninstrs);

	/*  Instructions per second, and average so far:  */
	is = 1000 * (ninstrs-ninstrs_last) / (mseconds-mseconds_last);
	avg = (long long)1000 * ninstrs / mseconds;
	if (is < 0)
		is = 0;
	if (avg < 0)
		avg = 0;

	if (cpu->has_been_idling) {
		printf("; idling");
		cpu->has_been_idling = 0;
	} else
		printf("; i/s=%"PRIi64" avg=%"PRIi64, is, avg);

	symbol = get_symbol_name(&machine->symbol_context, pc, &offset);

	if (machine->ncpus == 1) {
		if (cpu->is_32bit)
			printf("; pc=0x%08"PRIx32, (uint32_t) pc);
		else
			printf("; pc=0x%016"PRIx64, (uint64_t) pc);
	}

	/*  Special hack for M88K userland:  (Don't show symbols.)  */
	if (cpu->machine->arch == ARCH_M88K &&
	    !(cpu->cd.m88k.cr[M88K_CR_PSR] & M88K_PSR_MODE))
		symbol = NULL;

	if (symbol != NULL)
		printf(" <%s>", symbol);
	printf(" ]\n");

do_return:
	ninstrs_last = ninstrs;
	mseconds_last = mseconds;
}


/*
 *  cpu_run_init():
 *
 *  Prepare to run instructions on all CPUs in this machine. (This function
 *  should only need to be called once for each machine.)
 */
void cpu_run_init(struct machine *machine)
{
	int i;

	if (machine->ncpus == 0) {
		printf("Machine with no CPUs? TODO.\n");
		exit(1);
	}

	for (i=0; i<machine->ncpus; i++) {
		struct cpu *cpu = machine->cpus[i];

		cpu->ninstrs_flush = 0;
		cpu->ninstrs = 0;
		cpu->ninstrs_show = 0;

		/*  For performance measurement:  */
		gettimeofday(&cpu->starttime, NULL);
		cpu->ninstrs_since_gettimeofday = 0;
	}
}


/*
 *  add_cpu_family():
 *
 *  Allocates a cpu_family struct and calls an init function for the
 *  family to fill in reasonable data and pointers.
 */
static void add_cpu_family(int (*family_init)(struct cpu_family *), int arch)
{
	struct cpu_family *fp, *tmp;
	int res;

	CHECK_ALLOCATION(fp = (struct cpu_family *) malloc(sizeof(struct cpu_family)));
	memset(fp, 0, sizeof(struct cpu_family));

	/*
	 *  family_init() returns 1 if the struct has been filled with
	 *  valid data, 0 if suppor for the cpu family isn't compiled
	 *  into the emulator.
	 */
	res = family_init(fp);
	if (!res) {
		free(fp);
		return;
	}
	fp->arch = arch;
	fp->next = NULL;

	/*  Add last in family chain:  */
	tmp = first_cpu_family;
	if (tmp == NULL) {
		first_cpu_family = fp;
	} else {
		while (tmp->next != NULL)
			tmp = tmp->next;
		tmp->next = fp;
	}
}


/*
 *  cpu_family_ptr_by_number():
 *
 *  Returns a pointer to a CPU family based on the ARCH_* integers.
 */
struct cpu_family *cpu_family_ptr_by_number(int arch)
{
	struct cpu_family *fp;
	fp = first_cpu_family;

	/*  YUCK! This is too hardcoded! TODO  */

	while (fp != NULL) {
		if (arch == fp->arch)
			return fp;
		fp = fp->next;
	}

	return NULL;
}


/*
 *  cpu_init():
 *
 *  Should be called before any other cpu_*() function.
 *
 *  This function calls add_cpu_family() for each processor architecture.
 *  ADD_ALL_CPU_FAMILIES is defined in the config.h file generated by the
 *  configure script.
 */
void cpu_init(void)
{
	ADD_ALL_CPU_FAMILIES;
}

