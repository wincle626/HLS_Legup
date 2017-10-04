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
 *  COMMENT: A Iyonix machine
 *
 *  According to NetBSD/iyonix, for Tungsten motherboards:
 *
 *	0x90000000 (1 MB)	obio
 *	    0x900002f8 = uart1
 *	0xa0000000 (8 MB)	Flash memory
 *	0xfe400000		iopxs vbase
 *
 *  A dmesg can be found at the end of the following URL:
 *  http://www.sarasarado.org/~hatano/diary/d200411.html
 */

#include <stdio.h>
#include <string.h>

#include "bus_isa.h"
#include "cpu.h"
#include "device.h"
#include "devices.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/netbsd_iyonix_bootconfig.h"
#include "thirdparty/i80321reg.h"


MACHINE_SETUP(iyonix)
{
	char tmpstr[1000];
	uint32_t bootblock_addr;
	struct bootconfig bootconfig;

	machine->machine_name = strdup("Iyonix");

	cpu->cd.arm.coproc[6] = arm_coproc_i80321_6;

	/*  TODO: Is it flash, or just RAM?  */
	dev_ram_init(machine, 0xa0000000,
	    machine->physical_ram_in_mb * 1048576, DEV_RAM_MIRROR, 0x0);

	/*  Uncached RAM?  */
	dev_ram_init(machine, 0xc0000000,
	    machine->physical_ram_in_mb * 1048576, DEV_RAM_MIRROR, 0x0);

	snprintf(tmpstr, sizeof(tmpstr), "i80321 irq=%s.cpu[%i].irq "
	    "addr=0x%08x", machine->path, machine->bootstrap_cpu,
	    (int) VERDE_PMMR_BASE);
	device_add(machine, tmpstr);

	/*  TODO: actual irq on the i80321!  */
	snprintf(tmpstr, sizeof(tmpstr), "%s.cpu[%i].irq.i80321.1",
	    machine->path, machine->bootstrap_cpu);
	bus_isa_init(machine, tmpstr, 0, 0x90000000ULL, 0x98000000ULL);

	if (!machine->prom_emulation)
		return;


	/*
	 *  Set up suitable virtual memory mappings and bootconfig struct
	 *  in order to boot a plain NetBSD/iyonix ELF kernel:
	 *
	 *  r0 is expected to point to the bootconfig struct.
	 */

	arm_setup_initial_translation_table(cpu,
	    machine->physical_ram_in_mb * 1048576 - 65536);
	arm_translation_table_set_l1(cpu, 0x90000000, 0x90000000);
	arm_translation_table_set_l1(cpu, 0xf0000000, 0x00000000);
	arm_translation_table_set_l1_b(cpu, 0xff000000, 0xff000000);

	bootblock_addr = machine->physical_ram_in_mb * 1048576 - 65536 - 16384;
	cpu->cd.arm.r[0] = bootblock_addr;
	memset(&bootconfig, 0, sizeof(bootconfig));

	store_32bit_word_in_host(cpu, (unsigned char *)
	    &bootconfig.magic, BOOTCONFIG_MAGIC);
	store_32bit_word_in_host(cpu, (unsigned char *)
	    &bootconfig.version, BOOTCONFIG_VERSION);

	/*  TODO: More fields.  */

	store_buf(cpu, bootblock_addr, (char *)&bootconfig, sizeof(bootconfig));
}


MACHINE_DEFAULT_CPU(iyonix)
{
	machine->cpu_name = strdup("80321_600_B0");
}


MACHINE_DEFAULT_RAM(iyonix)
{
	machine->physical_ram_in_mb = 64;
}


MACHINE_REGISTER(iyonix)
{
	MR_DEFAULT(iyonix, "Iyonix", ARCH_ARM, MACHINE_IYONIX);

	machine_entry_add_alias(me, "iyonix");

	me->set_default_ram = machine_default_ram_iyonix;
}

