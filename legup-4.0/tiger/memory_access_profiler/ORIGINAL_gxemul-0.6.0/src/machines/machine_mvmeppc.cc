/*
 *  Copyright (C) 2006-2009  Anders Gavare.  All rights reserved.
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
 *  COMMENT: MVMEPPC machines
 *
 *  This is for experiments with NetBSD/mvmeppc or RTEMS.
 *  (ftp://ftp.netbsd.org/pub/NetBSD/arch/mvmeppc/snapshot/20020302/README)
 *
 *  Note:  MVME machines that really adhere to the PReP standard should be
 *         in machine_prep.c instead.
 *
 *
 *  TODO: This is mostly bogus.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bus_isa.h"
#include "bus_pci.h"
#include "cpu.h"
#include "device.h"
#include "devices.h"
#include "diskimage.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"


MACHINE_SETUP(mvmeppc)
{
	char tmpstr[300];
	struct pci_data *pci_data = NULL;

	switch (machine->machine_subtype) {

	case MACHINE_MVMEPPC_1600:
		machine->machine_name = strdup("MVME1600");

		snprintf(tmpstr, sizeof(tmpstr), "eagle irq=%s.cpu[%i]",
		    machine->path, machine->bootstrap_cpu);
		device_add(machine, tmpstr);

		bus_pci_add(machine, pci_data, machine->memory,
		    0, 14, 0, "dec21143");

		device_add(machine, "nvram addr=0x80000074 name2=mvme1600");

		/*
		 *  "DRAM size register": TODO: turn this into a device?
		 *  See the definition of p160x_dram_size in NetBSD's
		 *  .../arch/mvmeppc/platform_160x.c for details.
		 *
		 *  0x11 = two banks of 32 MB each.
		 */
		dev_ram_init(machine, 0x80000804, 1, DEV_RAM_RAM, 0);
		store_byte(cpu, 0x80000804, 0x11);

		break;

	case MACHINE_MVMEPPC_2100:
		machine->machine_name = strdup("MVME2100");

		/*  0xfe000000 isa bus space  */
		/*  0xfec00000 pci indirect addr  */
		/*  0xfee00000 pci indirect data  */

		/*  TODO: irq  */
		device_add(machine, "ns16550 irq=0 addr=0xffe10000");

		break;

	case MACHINE_MVMEPPC_5500:
		machine->machine_name = strdup("MVME5500");

		/*  GT64260 interrupt and PCI controller:  */
		pci_data = dev_gt_init(machine, machine->memory,
		    0xf1000000, "TODO: timer irq", "TODO: isa irq", 260);

		/*  TODO: irq  */
		device_add(machine, "ns16550 irq=0 addr=0xf1120000");

		break;

	default:fatal("Unimplemented MVMEPPC machine subtype %i\n",
		    machine->machine_subtype);
		exit(1);
	}

	if (!machine->prom_emulation)
		return;

	/*  r3 = start of kernel, r4 = end of kernel (for NetBSD/mvmeppc)  */
	cpu->cd.ppc.gpr[3] = 0;
	cpu->cd.ppc.gpr[4] = 1048576 * 10;
	cpu->cd.ppc.gpr[5] = machine->physical_ram_in_mb * 1048576-0x100;
	store_string(cpu, cpu->cd.ppc.gpr[5]+ 44, "PC16550");
	store_32bit_word(cpu, cpu->cd.ppc.gpr[5]+ 68, 9600);
	store_32bit_word(cpu, cpu->cd.ppc.gpr[5]+ 72, 0);

	store_16bit_word(cpu, cpu->cd.ppc.gpr[5]+ 76, 0x1600);
	store_32bit_word(cpu, cpu->cd.ppc.gpr[5]+ 80,
	    machine->physical_ram_in_mb * 1048576);
	store_32bit_word(cpu, cpu->cd.ppc.gpr[5]+ 84, 33 * 1000000);
	store_32bit_word(cpu, cpu->cd.ppc.gpr[5]+ 88, 33 * 1000000);
#if 0
0         u_int32_t       bi_boothowto;
4         u_int32_t       bi_bootaddr;
8         u_int16_t       bi_bootclun;
10        u_int16_t       bi_bootdlun;
12        char            bi_bootline[BOOTLINE_LEN];  (32)
44        char            bi_consoledev[CONSOLEDEV_LEN]; (16)
60        u_int32_t       bi_consoleaddr;
64        u_int32_t       bi_consolechan;
68        u_int32_t       bi_consolespeed;
72        u_int32_t       bi_consolecflag;
76        u_int16_t       bi_modelnumber;
80        u_int32_t       bi_memsize;
84        u_int32_t       bi_mpuspeed;
88        u_int32_t       bi_busspeed;
92        u_int32_t       bi_clocktps;
#endif
}


MACHINE_DEFAULT_CPU(mvmeppc)
{
	switch (machine->machine_subtype) {

	case MACHINE_MVMEPPC_1600:
		/*  Suitable for NetBSD/mvmeppc:  */
		machine->cpu_name = strdup("PPC603e");
		break;

	case MACHINE_MVMEPPC_2100:
		machine->cpu_name = strdup("PPC603e");
		break;

	case MACHINE_MVMEPPC_5500:
		machine->cpu_name = strdup("PPC750");
		break;

	default:fatal("Unimplemented MVMEPPC machine subtype %i\n",
		    machine->machine_subtype);
		exit(1);
	}
}


MACHINE_DEFAULT_RAM(mvmeppc)
{
	machine->physical_ram_in_mb = 64;
}


MACHINE_REGISTER(mvmeppc)
{
	MR_DEFAULT(mvmeppc, "MVMEPPC", ARCH_PPC, MACHINE_MVMEPPC);

	machine_entry_add_alias(me, "mvmeppc");

	machine_entry_add_subtype(me, "MVME1600", MACHINE_MVMEPPC_1600,
	    "mvme1600", NULL);

	machine_entry_add_subtype(me, "MVME2100", MACHINE_MVMEPPC_2100,
	    "mvme2100", NULL);

	machine_entry_add_subtype(me, "MVME5500", MACHINE_MVMEPPC_5500,
	    "mvme5500", NULL);

	me->set_default_ram = machine_default_ram_mvmeppc;
}

