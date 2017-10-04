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
 *  COMMENT: I-O DATA LanDisk USL-5P
 *
 *  This machine consists of:
 *
 *	o)  An SH4 processor, which includes serial console etc,
 *	o)  64 MB RAM (at 0x0c000000),
 *	o)  an IDE controller at address 0x14000000 (irq 10),
 *	o)  a RS5C313 real time clock, connected to the SH4 SCI port,
 *	o)  a PCI controller (PCIC),
 *	o)  and a minimal SH-IPL+G PROM emulation layer (required to make
 *	    OpenBSD/landisk boot).
 *
 *  TODO:
 *	Realtek NIC (irq 5), PCIIDE (irq 6), USB controllers, etc.
 *
 *  TODO 2:
 *	Make it possible to select different landisk models.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "devices.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/sh4_exception.h"
#include "thirdparty/sh4_scireg.h"


/*  This is not really implemented yet: (experimental)  */
/*  #define INCLUDE_LANDISK_NIC  */


MACHINE_SETUP(landisk)
{
#ifdef INCLUDE_LANDISK_NIC
	struct pci_data *pcibus =
	    machine->cpus[machine->bootstrap_cpu]->cd.sh.pcic_pcibus;
#endif
	char tmpstr[300];

	machine->machine_name = strdup("Landisk USL-5P");

	/*  266.67 MHz SH4 CPU clock:  */
	if (machine->emulated_hz == 0)
		machine->emulated_hz = 266666666;

	/*  33.33 MHz SH4 PCLOCK:  */
	machine->cpus[machine->bootstrap_cpu]->cd.sh.pclock = 33333333;

	/*  Note: 64 MB RAM at 0x0c000000, not at 0x00000000.  */
	dev_ram_init(machine, 0x0c000000, 64 * 1048576, DEV_RAM_RAM, 0x0);

	/*  wdc0 at obio0 port 0x14000000-0x1400000f irq 10  */
	snprintf(tmpstr, sizeof(tmpstr), "wdc irq=%s.cpu[%i].irq[0x%x]"
	    " addr_mult=2 addr=0x14000000",
	    machine->path, machine->bootstrap_cpu, SH4_INTEVT_IRQ10);
	device_add(machine, tmpstr);

	/*  rsclock0 at shb0: RS5C313 real time clock  */
	snprintf(tmpstr, sizeof(tmpstr), "rs5c313 addr=0x%"PRIx64,
	    (uint64_t) SCI_DEVICE_BASE);
	device_add(machine, tmpstr);

#ifdef INCLUDE_LANDISK_NIC
	/*  Realtek PCI NIC:  */
	bus_pci_add(machine, pcibus, machine->memory, 0, 0, 0, "rtl8139c");
#endif

	if (!machine->prom_emulation)
		return;

	/*
	 *  Ugly hardcoded register contents at bootup:
	 *
	 *  r4 (arg 0) = boot howto flags
	 *  r5 (arg 1) = bootinfo pointer for NetBSD (?) and
	 *               symbol end pointer for OpenBSD (?)
	 *
	 *  TODO: Make nicer.
	 */
	cpu->cd.sh.r[4] = 0;
	cpu->cd.sh.r[5] = 0x8c000000 + 10 * 1048576;	/*  Note/TODO:
				Assuming hardcoded 10 MB kernel size!  */

	sh_ipl_g_emul_init(machine);
}


MACHINE_DEFAULT_CPU(landisk)
{
	/*  Hitachi SH4 7751R, 266.67 MHz  */
	machine->cpu_name = strdup("SH7751R");
}


MACHINE_DEFAULT_RAM(landisk)
{
	/*  Note: This is the size of the boot ROM area, since the
	    Landisk's RAM isn't located at physical address zero.  */
	machine->physical_ram_in_mb = 2;
}


MACHINE_REGISTER(landisk)
{
	MR_DEFAULT(landisk, "Landisk", ARCH_SH, MACHINE_LANDISK);
	me->set_default_ram = machine_default_ram_landisk;

	machine_entry_add_alias(me, "landisk");
	machine_entry_add_alias(me, "usl-5p");
}

