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
 *  COMMENT: MIPS evaluation boards (e.g. Malta)
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

#include "thirdparty/maltareg.h"


MACHINE_SETUP(evbmips)
{
	char tmpstr[1000], tmpstr2[1000];
	struct pci_data *pci_data;
	int i;

	/*  See http://www.netbsd.org/ports/evbmips/ for more info.  */

	switch (machine->machine_subtype) {

	case MACHINE_EVBMIPS_MALTA:
	case MACHINE_EVBMIPS_MALTA_BE:
		if (machine->emulated_hz == 0)
			machine->emulated_hz = 33000000;
		cpu->byte_order = EMUL_LITTLE_ENDIAN;
		machine->machine_name = strdup("MALTA (evbmips, little endian)");

		if (machine->machine_subtype == MACHINE_EVBMIPS_MALTA_BE) {
			machine->machine_name = strdup("MALTA (evbmips, big endian)");
			cpu->byte_order = EMUL_BIG_ENDIAN;
		}

		/*  ISA bus at MIPS irq 2:  */
		snprintf(tmpstr, sizeof(tmpstr), "%s.cpu[%i].2",
		    machine->path, machine->bootstrap_cpu);
		bus_isa_init(machine, tmpstr, 0, 0x18000000, 0x10000000);

		snprintf(tmpstr, sizeof(tmpstr), "ns16550 irq=%s.cpu[%i].4 "
		    "addr=0x%x name2=tty2 in_use=0", machine->path,
		    machine->bootstrap_cpu, MALTA_CBUSUART);
		device_add(machine, tmpstr);

		/*  Add a GT controller; timer interrupts at ISA irq 9:  */
		snprintf(tmpstr, sizeof(tmpstr), "%s.cpu[%i].2.isa.9",
		    machine->path, machine->bootstrap_cpu);
		snprintf(tmpstr2, sizeof(tmpstr2), "%s.cpu[%i].2",
		    machine->path, machine->bootstrap_cpu);
		pci_data = dev_gt_init(machine, machine->memory, 0x1be00000,
		    tmpstr, tmpstr2, 120);

		if (machine->x11_md.in_use) {
			if (strlen(machine->boot_string_argument) < 3) {
				fatal("WARNING: remember to use  -o 'console="
				    "tty0'  if you are emulating Linux. (Not"
				    " needed for NetBSD.)\n");
			}
			bus_pci_add(machine, pci_data, machine->memory,
			    0, 8, 0, "s3_virge");
		}

		bus_pci_add(machine, pci_data, machine->memory,
		    0, 9, 0, "piix4_isa");
		bus_pci_add(machine, pci_data, machine->memory,
		    0, 9, 1, "piix4_ide");

		/*  pcn: Not yet, since it is just a bogus device, so far.  */
		/*  bus_pci_add(machine, pci_data, machine->memory,
		    0, 11, 0, "pcn");  */

		device_add(machine, "malta_lcd addr=0x1f000400");
		break;

	default:fatal("Unimplemented EVBMIPS model.\n");
		exit(1);
	}

	if (!machine->prom_emulation)
		return;


	/*  NetBSD/evbmips wants these: (at least for Malta)  */

	/*  a0 = argc  */
	cpu->cd.mips.gpr[MIPS_GPR_A0] = 2;

	/*  a1 = argv  */
	cpu->cd.mips.gpr[MIPS_GPR_A1] = (int32_t)0x9fc01000;
	store_32bit_word(cpu, (int32_t)0x9fc01000, 0x9fc01040);
	store_32bit_word(cpu, (int32_t)0x9fc01004, 0x9fc01200);
	store_32bit_word(cpu, (int32_t)0x9fc01008, 0);

	machine->bootstr = strdup(machine->boot_kernel_filename);
	machine->bootarg = strdup(machine->boot_string_argument);
	store_string(cpu, (int32_t)0x9fc01040, machine->bootstr);
	store_string(cpu, (int32_t)0x9fc01200, machine->bootarg);

	/*  a2 = (yamon_env_var *)envp  */
	cpu->cd.mips.gpr[MIPS_GPR_A2] = (int32_t)0x9fc01800;

	yamon_machine_setup(machine, cpu->cd.mips.gpr[MIPS_GPR_A2]);

	/*  a3 = memsize  */
	cpu->cd.mips.gpr[MIPS_GPR_A3] = machine->physical_ram_in_mb * 1048576;
	/*  Hm. Linux ignores a3.  */

	/*  Set the Core ID. See maltareg.h for more info.  */
	store_32bit_word(cpu, (int32_t)(0x80000000 + MALTA_REVISION),
	    (1 << 10) + 0x26);

	/*  Call vectors at 0x9fc005xx:  */
	for (i=0; i<0x100; i+=4)
		store_32bit_word(cpu, (int64_t)(int32_t)0x9fc00500 + i,
		    (int64_t)(int32_t)0x9fc00800 + i);

	/*  "Magic trap" PROM instructions at 0x9fc008xx:  */
	for (i=0; i<0x100; i+=4)
		store_32bit_word(cpu, (int64_t)(int32_t)0x9fc00800 + i,
		    0x00c0de0c);
}


MACHINE_DEFAULT_CPU(evbmips)
{
	switch (machine->machine_subtype) {

	case MACHINE_EVBMIPS_MALTA:
	case MACHINE_EVBMIPS_MALTA_BE:
		/*  5Kc = MIPS64 rev 1, 5KE = MIPS64 rev 2  */
		machine->cpu_name = strdup("5Kc");
		break;

	default:fatal("Unimplemented evbmips subtype.\n");
		exit(1);
	}
}


MACHINE_DEFAULT_RAM(evbmips)
{
	machine->physical_ram_in_mb = 128;
}


MACHINE_REGISTER(evbmips)
{
	MR_DEFAULT(evbmips, "MIPS evaluation boards (evbmips)",
	    ARCH_MIPS, MACHINE_EVBMIPS);

	machine_entry_add_alias(me, "evbmips");

	machine_entry_add_subtype(me, "Malta", MACHINE_EVBMIPS_MALTA,
	    "malta", NULL);

	machine_entry_add_subtype(me, "Malta (Big-Endian)",
	    MACHINE_EVBMIPS_MALTA_BE, "maltabe", NULL);

	me->set_default_ram = machine_default_ram_evbmips;
}

