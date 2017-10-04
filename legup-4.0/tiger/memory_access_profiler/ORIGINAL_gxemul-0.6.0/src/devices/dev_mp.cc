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
 *  COMMENT: Generic Multi-processor controller for the test machines
 *
 *  This is a fake multiprocessor (MP) device. It can be useful for
 *  theoretical experiments, but probably bares no resemblance to any
 *  multiprocessor controller used in any real machine.
 *
 *  NOTE: The devinit irq string should be the part _after_ "cpu[%i].".
 *        For MIPS, it will be MIPS_IPI_INT.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "machine.h"
#include "interrupt.h"
#include "memory.h"
#include "misc.h"

#include "testmachine/dev_mp.h"


struct mp_data {
	struct cpu	**cpus;
	uint64_t	startup_addr;
	uint64_t	stack_addr;
	uint64_t	pause_addr;

	/*  Each CPU has an array of pending ipis.  */
	int		*n_pending_ipis;
	int		**ipi;

	/*  Connections to all CPUs' IPI pins:  */
	struct interrupt *ipi_irq;
};


extern int single_step;


DEVICE_ACCESS(mp)
{
	struct mp_data *d = (struct mp_data *) extra;
	int i, which_cpu;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
	        idata = memory_readmax64(cpu, data, len);

	/*
	 *  NOTE: It is up to the user of this device to read or write
	 *  correct addresses. (A write to NCPUS is pretty useless,
	 *  for example.)
	 */

	switch (relative_addr) {

	case DEV_MP_WHOAMI:
		odata = cpu->cpu_id;
		break;

	case DEV_MP_NCPUS:
		odata = cpu->machine->ncpus;
		break;

	case DEV_MP_STARTUPCPU:
		which_cpu = idata;
		d->cpus[which_cpu]->pc = d->startup_addr;
		switch (cpu->machine->arch) {
		case ARCH_MIPS:
			d->cpus[which_cpu]->cd.mips.gpr[MIPS_GPR_SP] =
			    d->stack_addr;
			break;
		case ARCH_PPC:
			d->cpus[which_cpu]->cd.ppc.gpr[1] = d->stack_addr;
			break;
		default:
			fatal("dev_mp(): DEV_MP_STARTUPCPU: not for this"
			    " arch yet!\n");
			exit(1);
		}
		d->cpus[which_cpu]->running = 1;
		/*  debug("[ dev_mp: starting up cpu%i at 0x%llx ]\n", 
		    which_cpu, (long long)d->startup_addr);  */
		break;

	case DEV_MP_STARTUPADDR:
		if (len==4 && (idata >> 32) == 0 && (idata & 0x80000000ULL))
			idata |= 0xffffffff00000000ULL;
		d->startup_addr = idata;
		break;

	case DEV_MP_PAUSE_ADDR:
		d->pause_addr = idata;
		break;

	case DEV_MP_PAUSE_CPU:
		/*  Pause all cpus except a specific CPU:  */
		which_cpu = idata;

		for (i=0; i<cpu->machine->ncpus; i++)
			if (i != which_cpu)
				d->cpus[i]->running = 0;
		break;

	case DEV_MP_UNPAUSE_CPU:
		/*  Unpause a specific CPU:  */
		which_cpu = idata;

		if (which_cpu >= 0 && which_cpu <cpu->machine->ncpus)
			d->cpus[which_cpu]->running = 1;
		break;

	case DEV_MP_STARTUPSTACK:
		if (len == 4 && (idata >> 32) == 0 && (idata & 0x80000000ULL))
			idata |= 0xffffffff00000000ULL;
		d->stack_addr = idata;
		break;

	case DEV_MP_HARDWARE_RANDOM:
		/*
		 *  Return (up to) 64 bits of "hardware random":
		 *
		 *  NOTE: Remember that random() is (usually) 31 bits of
		 *        random data, _NOT_ 32, hence this construction.
		 */
		odata = random();
		odata = (odata << 31) ^ random();
		odata = (odata << 31) ^ random();
		break;

	case DEV_MP_MEMORY:
		/*
		 *  Return the number of bytes of memory in the system.
		 *
		 *  (It is assumed to be located at physical address 0.
		 *  It is actually located at machine->memory_offset_in_mb
		 *  but that is only used for SGI emulation so far.)
		 */
		odata = cpu->machine->physical_ram_in_mb * 1048576;
		break;

	case DEV_MP_IPI_ONE:
	case DEV_MP_IPI_MANY:
		/*
		 *  idata should be of the form:
		 *
		 *		(IPI_nr << 16) | cpu_id
		 *
		 *  This will send an Inter-processor interrupt to a specific
		 *  CPU. (DEV_MP_IPI_MANY sends to all _except_ the specific
		 *  CPU.)
		 *
		 *  Sending an IPI means adding the IPI last in the list of
		 *  pending IPIs, and asserting the IPI "pin".
		 */
		which_cpu = (idata & 0xffff);
		for (i=0; i<cpu->machine->ncpus; i++) {
			int send_it = 0;
			if (relative_addr == DEV_MP_IPI_ONE && i == which_cpu)
				send_it = 1;
			if (relative_addr == DEV_MP_IPI_MANY && i != which_cpu)
				send_it = 1;
			if (send_it) {
				d->n_pending_ipis[i] ++;
				CHECK_ALLOCATION(d->ipi[i] = (int *) realloc(d->ipi[i],
				    d->n_pending_ipis[i] * sizeof(int)));

				/*  Add the IPI last in the array:  */
				d->ipi[i][d->n_pending_ipis[i] - 1] =
				    idata >> 16;

				INTERRUPT_ASSERT(d->ipi_irq[i]);
			}
		}
		break;

	case DEV_MP_IPI_READ:
		/*
		 *  If the current CPU has any IPIs pending, accessing this
		 *  address reads the IPI value. (Writing to this address
		 *  discards _all_ pending IPIs.)  If there is no pending
		 *  IPI, then 0 is returned. Usage of the value 0 for real
		 *  IPIs should thus be avoided.
		 */
		if (writeflag == MEM_WRITE) {
			d->n_pending_ipis[cpu->cpu_id] = 0;
		}
		odata = 0;
		if (d->n_pending_ipis[cpu->cpu_id] > 0) {
			odata = d->ipi[cpu->cpu_id][0];
			if (d->n_pending_ipis[cpu->cpu_id]-- > 1)
				memmove(&d->ipi[cpu->cpu_id][0],
				    &d->ipi[cpu->cpu_id][1],
				    d->n_pending_ipis[cpu->cpu_id]);
		}

		/*  Deassert the interrupt, if there are no pending IPIs:  */
		if (d->n_pending_ipis[cpu->cpu_id] == 0)
			INTERRUPT_DEASSERT(d->ipi_irq[cpu->cpu_id]);
		break;

	case DEV_MP_NCYCLES:
		/*
		 *  Return _approximately_ the number of cycles executed
		 *  on this CPU.
		 *
		 *  (This value is not updated for each instruction.)
		 */
		odata = cpu->ninstrs;
		break;

	default:
		fatal("[ dev_mp: unimplemented relative addr 0x%x ]\n",
		    relative_addr);
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(mp)
{
	struct mp_data *d;
	int n, i;

	CHECK_ALLOCATION(d = (struct mp_data *) malloc(sizeof(struct mp_data)));
	memset(d, 0, sizeof(struct mp_data));

	d->cpus = devinit->machine->cpus;
	d->startup_addr = INITIAL_PC;
	d->stack_addr = INITIAL_STACK_POINTER;

	n = devinit->machine->ncpus;

	/*  Connect to all CPUs' IPI pins:  */
	CHECK_ALLOCATION(d->ipi_irq = (struct interrupt *)
	    malloc(n * sizeof(struct interrupt)));

	for (i=0; i<n; i++) {
		char tmpstr[200];
		snprintf(tmpstr, sizeof(tmpstr), "%s.cpu[%i].%s",
		    devinit->machine->path, i, devinit->interrupt_path);
		INTERRUPT_CONNECT(tmpstr, d->ipi_irq[i]);
	}

	CHECK_ALLOCATION(d->n_pending_ipis = (int *) malloc(n * sizeof(int)));
	memset(d->n_pending_ipis, 0, sizeof(int) * n);

	CHECK_ALLOCATION(d->ipi = (int **) malloc(n * sizeof(int *)));
	memset(d->ipi, 0, sizeof(int *) * n);

	memory_device_register(devinit->machine->memory, devinit->name,
	    devinit->addr, DEV_MP_LENGTH, dev_mp_access, d, DM_DEFAULT, NULL);

	return 1;
}

