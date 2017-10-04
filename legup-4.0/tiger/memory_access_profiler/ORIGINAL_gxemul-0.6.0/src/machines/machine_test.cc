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
 *  COMMENT: Various test machines
 *
 *  Generally, the machines are as follows:
 *
 *	bareXYZ:	A bare machine using an XYZ processor.
 *
 *	testXYZ:	A machine with an XYZ processor, and some experimental
 *			devices connected to it.
 *
 *  The experimental devices in the test machines are:
 *
 *	cons		A serial I/O console device.
 *	disk		A device for reading/writing (emulated) disk sectors.
 *	ether		An ethernet device, for sending/receiving ethernet
 *			frames on an emulated network.
 *	fb		Framebuffer (24-bit RGB per pixel).
 *	irqc		A generic interrupt controller.
 *	mp		A multiprocessor controller.
 *	rtc		A real-time clock device.
 */

#include <stdio.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/sh4_exception.h"

#include "testmachine/dev_cons.h"
#include "testmachine/dev_disk.h"
#include "testmachine/dev_ether.h"
#include "testmachine/dev_fb.h"
#include "testmachine/dev_irqc.h"
#include "testmachine/dev_mp.h"
#include "testmachine/dev_rtc.h"


/*
 *  default_test():
 *
 *  Initializes devices for most test machines. (Note: MIPS is different,
 *  because of legacy reasons.)
 */
static void default_test(struct machine *machine, struct cpu *cpu)
{
	char tmpstr[1000];
	char base_irq[1000];
	char end_of_base_irq[50];

	/*
	 *  First add the interrupt controller. Most processor architectures
	 *  in GXemul have only 1 interrupt pin on the CPU, and it is simply
	 *  called "machine[y].cpu[z]".
	 *
	 *  MIPS is an exception, dealt with in a separate setup function.
	 *  ARM and SH are dealt with here.
	 */

	switch (machine->arch) {

	case ARCH_ARM:
		snprintf(end_of_base_irq, sizeof(end_of_base_irq), ".irq");
		break;

	case ARCH_SH:
		snprintf(end_of_base_irq, sizeof(end_of_base_irq),
		    ".irq[0x%x]", SH4_INTEVT_IRQ15);
		break;

	default:
		end_of_base_irq[0] = '\0';
	}

	snprintf(base_irq, sizeof(base_irq), "%s.cpu[%i]%s",
	    machine->path, machine->bootstrap_cpu, end_of_base_irq);

	snprintf(tmpstr, sizeof(tmpstr), "irqc addr=0x%"PRIx64" irq=%s",
	    (uint64_t) DEV_IRQC_ADDRESS, base_irq);
	device_add(machine, tmpstr);


	/*  Now, add the other devices:  */

	snprintf(tmpstr, sizeof(tmpstr), "cons addr=0x%"PRIx64
	    " irq=%s.irqc.2 in_use=%i",
	    (uint64_t) DEV_CONS_ADDRESS, base_irq, machine->arch != ARCH_SH);
	machine->main_console_handle = (size_t)device_add(machine, tmpstr);

	snprintf(tmpstr, sizeof(tmpstr), "mp addr=0x%"PRIx64" irq=%s%sirqc.6",
	    (uint64_t) DEV_MP_ADDRESS,
	    end_of_base_irq[0]? end_of_base_irq + 1 : "",
	    end_of_base_irq[0]? "." : "");
	device_add(machine, tmpstr);

	snprintf(tmpstr, sizeof(tmpstr), "fbctrl addr=0x%"PRIx64,
	    (uint64_t) DEV_FBCTRL_ADDRESS);
	device_add(machine, tmpstr);

	snprintf(tmpstr, sizeof(tmpstr), "disk addr=0x%"PRIx64,
	    (uint64_t) DEV_DISK_ADDRESS);
	device_add(machine, tmpstr);

	snprintf(tmpstr, sizeof(tmpstr), "ether addr=0x%"PRIx64" irq=%s.irqc.3",
	    (uint64_t) DEV_ETHER_ADDRESS, base_irq);
	device_add(machine, tmpstr);

	snprintf(tmpstr, sizeof(tmpstr), "rtc addr=0x%"PRIx64" irq=%s.irqc.4",
	    (uint64_t) DEV_RTC_ADDRESS, base_irq);
	device_add(machine, tmpstr);
}


MACHINE_SETUP(barearm)
{
	machine->machine_name = strdup("Generic \"bare\" ARM machine");
}


MACHINE_SETUP(testarm)
{
	machine->machine_name = strdup("ARM test machine");

	default_test(machine, cpu);

	/*
	 *  Place a tiny stub at end of memory, and set the link register to
	 *  point to it. This stub halts the machine (making it easy to try
	 *  out simple stand-alone C functions).
	 */
	cpu->cd.arm.r[ARM_SP] = machine->physical_ram_in_mb * 1048576 - 4096;
	cpu->cd.arm.r[ARM_LR] = cpu->cd.arm.r[ARM_SP] + 32;
	store_32bit_word(cpu, cpu->cd.arm.r[ARM_LR] + 0, 0xe3a00201);
	store_32bit_word(cpu, cpu->cd.arm.r[ARM_LR] + 4, 0xe5c00010);
	store_32bit_word(cpu, cpu->cd.arm.r[ARM_LR] + 8, 0xeafffffe);
}


MACHINE_DEFAULT_CPU(barearm)
{
	machine->cpu_name = strdup("SA1110");
}


MACHINE_DEFAULT_CPU(testarm)
{
	machine->cpu_name = strdup("SA1110");
}


MACHINE_REGISTER(barearm)
{
	MR_DEFAULT(barearm, "Generic \"bare\" ARM machine",
	    ARCH_ARM, MACHINE_BAREARM);

	machine_entry_add_alias(me, "barearm");
}


MACHINE_REGISTER(testarm)
{
	MR_DEFAULT(testarm, "Test-machine for ARM", ARCH_ARM, MACHINE_TESTARM);

	machine_entry_add_alias(me, "testarm");
}



MACHINE_SETUP(barem88k)
{
	machine->machine_name = strdup("Generic \"bare\" M88K machine");
}


MACHINE_SETUP(oldtestm88k)
{
	machine->machine_name = strdup("M88K test machine");

	default_test(machine, cpu);
}


MACHINE_DEFAULT_CPU(barem88k)
{
	machine->cpu_name = strdup("88110");
}


MACHINE_DEFAULT_CPU(oldtestm88k)
{
	machine->cpu_name = strdup("88110");
}


MACHINE_REGISTER(barem88k)
{
	MR_DEFAULT(barem88k, "Generic \"bare\" M88K machine",
	    ARCH_M88K, MACHINE_BAREM88K);

	machine_entry_add_alias(me, "barem88k");
}


MACHINE_REGISTER(oldtestm88k)
{
	MR_DEFAULT(oldtestm88k, "Test-machine for M88K",
	    ARCH_M88K, MACHINE_TESTM88K);

	machine_entry_add_alias(me, "oldtestm88k");
}


MACHINE_SETUP(baremips)
{
	machine->machine_name = strdup("Generic \"bare\" MIPS machine");
	cpu->byte_order = EMUL_BIG_ENDIAN;
}


MACHINE_SETUP(oldtestmips)
{
	/*
	 *  A MIPS test machine. Originally, this was created as a way for
	 *  me to test my master's thesis code; since then it has both
	 *  evolved to support new things, and suffered bit rot so that it
	 *  no longer can run my thesis code. Well, well...
	 *
	 *  IRQ map:
	 *      7       CPU counter
	 *      6       SMP IPIs
	 *      5       not used yet
	 *      4       rtc
	 *      3       ethernet  
	 *      2       serial console
	 */

	char tmpstr[300];

	machine->machine_name = strdup("MIPS test machine");
	cpu->byte_order = EMUL_BIG_ENDIAN;

	snprintf(tmpstr, sizeof(tmpstr), "cons addr=0x%"PRIx64" irq=%s."
	    "cpu[%i].2", (uint64_t) DEV_CONS_ADDRESS, machine->path,
	    machine->bootstrap_cpu);
	machine->main_console_handle = (size_t)device_add(machine, tmpstr);

	snprintf(tmpstr, sizeof(tmpstr), "mp addr=0x%"PRIx64" irq=6",
	    (uint64_t) DEV_MP_ADDRESS);
	device_add(machine, tmpstr);

	snprintf(tmpstr, sizeof(tmpstr), "fbctrl addr=0x%"PRIx64,
	    (uint64_t) DEV_FBCTRL_ADDRESS);
	device_add(machine, tmpstr);

	snprintf(tmpstr, sizeof(tmpstr), "disk addr=0x%"PRIx64,
	    (uint64_t) DEV_DISK_ADDRESS);
	device_add(machine, tmpstr);

	snprintf(tmpstr, sizeof(tmpstr), "ether addr=0x%"PRIx64" irq=%s."
	    "cpu[%i].3", (uint64_t) DEV_ETHER_ADDRESS, machine->path,
	    machine->bootstrap_cpu);
	device_add(machine, tmpstr);

	snprintf(tmpstr, sizeof(tmpstr), "rtc addr=0x%"PRIx64" irq=%s."
	    "cpu[%i].4", (uint64_t) DEV_RTC_ADDRESS, machine->path,
	    machine->bootstrap_cpu);
	device_add(machine, tmpstr);
}


MACHINE_DEFAULT_CPU(baremips)
{
	machine->cpu_name = strdup("5KE");
}


MACHINE_DEFAULT_CPU(oldtestmips)
{
	machine->cpu_name = strdup("5KE");
}


MACHINE_REGISTER(baremips)
{
	MR_DEFAULT(baremips, "Generic \"bare\" MIPS machine",
	    ARCH_MIPS, MACHINE_BAREMIPS);

	machine_entry_add_alias(me, "baremips");
}


MACHINE_REGISTER(oldtestmips)
{
	MR_DEFAULT(oldtestmips, "Test-machine for MIPS",
	    ARCH_MIPS, MACHINE_TESTMIPS);

	machine_entry_add_alias(me, "oldtestmips");
}


MACHINE_SETUP(bareppc)
{
	machine->machine_name = strdup("Generic \"bare\" PPC machine");
}


MACHINE_SETUP(testppc)
{
	machine->machine_name = strdup("PPC test machine");

	default_test(machine, cpu);
}


MACHINE_DEFAULT_CPU(bareppc)
{
	machine->cpu_name = strdup("PPC970");
}


MACHINE_DEFAULT_CPU(testppc)
{
	machine->cpu_name = strdup("PPC970");
}


MACHINE_REGISTER(bareppc)
{
	MR_DEFAULT(bareppc, "Generic \"bare\" PPC machine",
	    ARCH_PPC, MACHINE_BAREPPC);

	machine_entry_add_alias(me, "bareppc");
}


MACHINE_REGISTER(testppc)
{
	MR_DEFAULT(testppc, "Test-machine for PPC", ARCH_PPC, MACHINE_TESTPPC);

	machine_entry_add_alias(me, "testppc");
}


MACHINE_SETUP(baresh)
{
	machine->machine_name = strdup("Generic \"bare\" SH machine");
}


MACHINE_SETUP(testsh)
{
	machine->machine_name = strdup("SH test machine");

	default_test(machine, cpu);
}


MACHINE_DEFAULT_CPU(baresh)
{
	machine->cpu_name = strdup("SH7750");
}


MACHINE_DEFAULT_CPU(testsh)
{
	machine->cpu_name = strdup("SH7750");
}


MACHINE_REGISTER(baresh)
{
	MR_DEFAULT(baresh, "Generic \"bare\" SH machine",
	    ARCH_SH, MACHINE_BARESH);

	machine_entry_add_alias(me, "baresh");
}


MACHINE_REGISTER(testsh)
{
	MR_DEFAULT(testsh, "Test-machine for SH", ARCH_SH, MACHINE_TESTSH);

	machine_entry_add_alias(me, "testsh");
}



