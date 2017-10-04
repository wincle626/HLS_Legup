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
 *  COMMENT: Simtec Electronics' CATS board
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bus_isa.h"
#include "bus_pci.h"
#include "cpu.h"
#include "device.h"
#include "devices.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/cyclone_boot.h"


MACHINE_SETUP(cats)
{
	struct ebsaboot ebsaboot;
	struct pci_data *pci_bus;
	char bs[300], tmpstr[400];
	int boot_id = machine->bootdev_id >= 0? machine->bootdev_id : 0;

	machine->machine_name = strdup("CATS evaluation board");

	if (machine->emulated_hz == 0)
		machine->emulated_hz = 50000000;

	if (machine->physical_ram_in_mb > 256)
		fprintf(stderr, "WARNING! Real CATS machines cannot"
		    " have more than 256 MB RAM. Continuing anyway.\n");

	snprintf(tmpstr, sizeof(tmpstr), "footbridge irq=%s.cpu[%i].irq"
	    " addr=0x42000000", machine->path, machine->bootstrap_cpu);
	pci_bus = (struct pci_data *) device_add(machine, tmpstr);

	/*  DC21285_ROM_BASE (256 KB at 0x41000000)  */
	dev_ram_init(machine, 0x41000000, 256 * 1024, DEV_RAM_RAM, 0);

	/*  NetBSD, OpenBSD, and Linux (?) clean their caches here:  */
	dev_ram_init(machine, 0x50000000, 0x10000, DEV_RAM_RAM, 0);

	/*  Interrupt ack space?  */
	dev_ram_init(machine, 0x80000000, 0x1000, DEV_RAM_RAM, 0);

	/*  Linux uses 0xc0000000 as phys.:  */
	dev_ram_init(machine, 0xc0000000, 0x20000000, DEV_RAM_MIRROR, 0x0);

	/*  OpenBSD reboot needs 0xf??????? to be mapped to phys.:  */
	dev_ram_init(machine, 0xf0000000, 0x1000000, DEV_RAM_MIRROR, 0x0);

	bus_pci_add(machine, pci_bus, machine->memory, 0xc0, 8, 0, "s3_virge");

	if (!machine->prom_emulation)
		return;

	/*  See cyclone_boot.h for details.  */

	/*  DC21285_ROM_BASE "reboot" code:  (works with NetBSD)  */
	store_32bit_word(cpu, 0x41000008ULL, 0xef8c64ebUL);

	cpu->cd.arm.r[0] = /* machine->physical_ram_in_mb */
	    7 * 1048576 - 0x1000;

	memset(&ebsaboot, 0, sizeof(struct ebsaboot));
	store_32bit_word_in_host(cpu, (unsigned char *)
	    &(ebsaboot.bt_magic), BT_MAGIC_NUMBER_CATS);
	store_32bit_word_in_host(cpu, (unsigned char *)
	    &(ebsaboot.bt_vargp), 0);
	store_32bit_word_in_host(cpu, (unsigned char *)
	    &(ebsaboot.bt_pargp), 0);
	store_32bit_word_in_host(cpu, (unsigned char *)
	    &(ebsaboot.bt_args), cpu->cd.arm.r[0]
	    + sizeof(struct ebsaboot));
	store_32bit_word_in_host(cpu, (unsigned char *)
	    &(ebsaboot.bt_l1), 7 * 1048576 - 32768);
	store_32bit_word_in_host(cpu, (unsigned char *)
	    &(ebsaboot.bt_memstart), 0);
	store_32bit_word_in_host(cpu, (unsigned char *)
	    &(ebsaboot.bt_memend),
	    machine->physical_ram_in_mb * 1048576);
	store_32bit_word_in_host(cpu, (unsigned char *)
	    &(ebsaboot.bt_memavail), 7 * 1048576);
	store_32bit_word_in_host(cpu, (unsigned char *)
	    &(ebsaboot.bt_fclk), 50 * 1000000);
	store_32bit_word_in_host(cpu, (unsigned char *)
	    &(ebsaboot.bt_pciclk), 66 * 1000000);
	/*  TODO: bt_vers  */
	/*  TODO: bt_features  */

	store_buf(cpu, cpu->cd.arm.r[0],
	    (char *)&ebsaboot, sizeof(struct ebsaboot));

	snprintf(bs, sizeof(bs), "(hd%i)%s root=/dev/wd%i%s%s",
	    boot_id, machine->boot_kernel_filename, boot_id,
	    (machine->boot_string_argument[0])? " " : "",
	    machine->boot_string_argument);

	store_string(cpu, cpu->cd.arm.r[0]+sizeof(struct ebsaboot), bs);

	arm_setup_initial_translation_table(cpu, 7 * 1048576 - 32768);
}


MACHINE_DEFAULT_CPU(cats)
{
	machine->cpu_name = strdup("SA110");
}


MACHINE_DEFAULT_RAM(cats)
{
	machine->physical_ram_in_mb = 64;
}


MACHINE_REGISTER(cats)
{
	MR_DEFAULT(cats, "CATS evaluation board", ARCH_ARM,
	    MACHINE_CATS);

	machine_entry_add_alias(me, "cats");

	me->set_default_ram = machine_default_ram_cats;
}

