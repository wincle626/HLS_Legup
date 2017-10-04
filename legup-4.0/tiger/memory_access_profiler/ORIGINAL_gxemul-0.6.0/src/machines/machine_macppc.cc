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
 *  COMMENT: Generic PowerPC-based Macintosh machines
 *
 *  See also:
 *
 *	NetBSD/macppc (http://www.netbsd.org/ports/macppc/)
 *	OpenBSD/macppc (http://www.openbsd.org/macppc.html)
 *
 *  Currently, these are skeletons for generic PowerMac G3, G4, and G5 systems.
 *  They do not model real PowerMacs, but should be enough to begin
 *  experimenting with running NetBSD/macppc and OpenBSD/macppc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bus_pci.h"
#include "cpu.h"
#include "device.h"
#include "devices.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"
#include "of.h"


MACHINE_SETUP(macppc)
{
	char tmpstr[300];
	struct pci_data *pci_data;
	struct vfb_data *fb;
	uint64_t b, a;
	int i;

	machine->machine_name = strdup("Macintosh (PPC)");
	if (machine->emulated_hz == 0)
		machine->emulated_hz = 40000000;

	device_add(machine, "gc addr=0xf3000000");

	pci_data = dev_uninorth_init(machine, machine->memory, 0xe2000000,
	    64 /*  isa irq base */, 0 /*  pci irq: TODO */);

	/*  bus_pci_add(
	    machine, pci_data, machine->memory, 0, 12, 0, "dec21143");  */
	bus_pci_add(machine, pci_data, machine->memory, 0, 15, 0, "gc_obio");

	if (machine->x11_md.in_use)
		bus_pci_add(machine, pci_data, machine->memory, 0, 16, 0,
		    "ati_radeon_9200_2");

	snprintf(tmpstr, sizeof(tmpstr), "z8530 addr=0xf3013000 irq="
	    "%s.cpu[%i].gc.lo.23 addr_mult=0x10",
	    machine->path, machine->bootstrap_cpu);
	machine->main_console_handle = (size_t)device_add(machine, tmpstr);

	fb = dev_fb_init(machine, machine->memory, 0xf1000000,
	    VFB_GENERIC | VFB_REVERSE_START, 1024,768, 1024,768, 8, "ofb");

	device_add(machine, "hammerhead addr=0xf2800000");

	snprintf(tmpstr, sizeof(tmpstr), "adb addr=0xf3016000 irq="
	    "%s.cpu[%i].gc.lo.1", machine->path, machine->bootstrap_cpu);
	device_add(machine, tmpstr);

	if (!machine->prom_emulation)
		return;


	b = 8 * 1048576; a = b - 0x800;

	of_emul_init(machine, fb, 0xf1000000, 1024, 768);
	of_emul_init_uninorth(machine);

	if (machine->x11_md.in_use)
		of_emul_init_adb(machine);
	else
		of_emul_init_zs(machine);

	/*
	 *  r3 = pointer to boot_args (for the Mach kernel).
	 *  See http://darwinsource.opendarwin.org/10.3/
	 *  BootX-59/bootx.tproj/include.subproj/boot_args.h
	 *  for more info.
	 */
	cpu->cd.ppc.gpr[3] = a;
	store_16bit_word(cpu, a + 0x0000, 1);	/*  revision  */
	store_16bit_word(cpu, a + 0x0002, 2);	/*  version  */
	store_buf(cpu, a + 0x0004, machine->boot_string_argument, 256);
	/*  26 dram banks; "long base; long size"  */
	store_32bit_word(cpu, a + 0x0104, 0);	/*  base  */
	store_32bit_word(cpu, a + 0x0108, machine->physical_ram_in_mb
	    * 256);		/*  size (in pages)  */
	for (i=8; i<26*8; i+= 4)
		store_32bit_word(cpu, a + 0x0104 + i, 0);
	a += (0x104 + 26 * 8);
	/*  Video info:  */
	store_32bit_word(cpu, a+0, 0xf1000000);	/*  video base  */
	store_32bit_word(cpu, a+4, 0);		/*  display code (?)  */
	store_32bit_word(cpu, a+8, 1024);	/*  bytes per pixel row  */
	store_32bit_word(cpu, a+12, 1024);	/*  width  */
	store_32bit_word(cpu, a+16, 768);	/*  height  */
	store_32bit_word(cpu, a+20, 8);		/*  pixel depth  */
	a += 24;
	store_32bit_word(cpu, a+0, 127);	/*  gestalt number (TODO)  */
	store_32bit_word(cpu, a+4, 0);		/*  device tree pointer (TODO)*/
	store_32bit_word(cpu, a+8, 0);		/*  device tree length  */
	store_32bit_word(cpu, a+12, b);	/*  last address of kernel data area  */

	/*  r4 = "MOSX" (0x4D4F5358)  */
	cpu->cd.ppc.gpr[4] = 0x4D4F5358;

	/*
	 *  r5 = OpenFirmware entry point.  NOTE: See
	 *  cpu_ppc.c for the rest of this semi-ugly hack.
	 */
	dev_ram_init(machine, cpu->cd.ppc.of_emul_addr,
	    0x1000, DEV_RAM_RAM, 0x0);
	store_32bit_word(cpu, cpu->cd.ppc.of_emul_addr,
	    0x44ee0002);
	cpu->cd.ppc.gpr[5] = cpu->cd.ppc.of_emul_addr;

#if 0
	/*  r6 = args  */
	cpu->cd.ppc.gpr[1] -= 516;
	cpu->cd.ppc.gpr[6] = cpu->cd.ppc.gpr[1] + 4;
	store_string(cpu, cpu->cd.ppc.gpr[6],
	    machine->boot_string_argument);
	/*  should be something like '/controller/disk/bsd'  */

	/*  r7 = length? TODO  */
	cpu->cd.ppc.gpr[7] = 5;
#endif
}


MACHINE_DEFAULT_CPU(macppc)
{
	switch (machine->machine_subtype) {
	case MACHINE_MACPPC_G3:
		machine->cpu_name = strdup("PPC750");
		break;
	case MACHINE_MACPPC_G4:
		machine->cpu_name = strdup("MPC7400");
		break;
	case MACHINE_MACPPC_G5:
		machine->cpu_name = strdup("PPC970");
		break;
	}
}


MACHINE_DEFAULT_RAM(macppc)
{
	machine->physical_ram_in_mb = 64;
}


MACHINE_REGISTER(macppc)
{
	MR_DEFAULT(macppc, "Macintosh", ARCH_PPC, MACHINE_MACPPC);

	machine_entry_add_alias(me, "macppc");

	machine_entry_add_subtype(me, "MacPPC G3", MACHINE_MACPPC_G3,
	    "g3", NULL);

	machine_entry_add_subtype(me, "MacPPC G4", MACHINE_MACPPC_G4,
	    "g4", NULL);

	machine_entry_add_subtype(me, "MacPPC G5", MACHINE_MACPPC_G5,
	    "g5", NULL);

	me->set_default_ram = machine_default_ram_macppc;
}

