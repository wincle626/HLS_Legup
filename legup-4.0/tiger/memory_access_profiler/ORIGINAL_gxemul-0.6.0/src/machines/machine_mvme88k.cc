/*
 *  Copyright (C) 2007-2009  Anders Gavare.  All rights reserved.
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
 *  COMMENT: MVME88K machines (MVME187)
 *
 *  This is for experiments with OpenBSD/mvme88k.
 *
 *  MVME187 according to http://mcg.motorola.com/us/products/docs/pdf/187igd.pdf
 *  ("MVME187 RISC Single Board Computer Installation Guide"):
 *
 *	88100 MPU, two MC88200 or MC88204 CMMUs (one for data cache and
 *		one for instruction cache).
 *	82596CA LAN Ethernet
 *	53C710 SCSI
 *	CD2401 SCC SERIAL IO
 *	PRINTER PORT
 *	MK48T08 BBRAM & CLOCK
 *	EPROM
 *	VME bus
 *
 *  ... and more details from OpenBSD/mvme88k sources:
 *
 *	0xff800000 .. 0xffbfffff = BUG PROM
 *	0xffe00000 .. 0xffe1ffff = BUG SRAM
 *	0xfff00000               = PCCTWO
 *	  0xfff40000             = VME bus
 *	  0xfff43000             = MEMC040 (Memory controller)
 *	  0xfff45000             = CD2401 SCC SERIAL IO (cl0)
 *	  0xfff46000             = 82596 Ethernet (ie0)
 *	  0xfff47000             = 53C710 SCSI (osiop0)
 *	  0xfffc0000             = MK48T08 (nvram0)
 *
 *  Note: It may turn out to be easier to support Lance ethernet (via VME)
 *  than to support the 82596 ethernet controller.
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

#include "thirdparty/mvme187.h"
#include "thirdparty/mvme88k_vme.h"
#include "thirdparty/mvme_pcctworeg.h"


MACHINE_SETUP(mvme88k)
{
	char tmpstr[300];

	switch (machine->machine_subtype) {

	case MACHINE_MVME88K_187:
		machine->machine_name = strdup("MVME187");

		/*  The mvme187 device contains 187-specific stuff, such
		    as motherboard registers, and the memory controller:  */
		device_add(machine, "mvme187");

		/*  PCC2 bus at 0xfff00000:  */
		snprintf(tmpstr, sizeof(tmpstr), "pcc2 addr=0x%x", PCC2_BASE);
		device_add(machine, tmpstr);

		/*  VME2 bus at 0xfff40000:  */
		snprintf(tmpstr, sizeof(tmpstr), "vme addr=0x%x", VME2_BASE);
		device_add(machine, tmpstr);

		/*  Cirrus Logic serial console at 0xfff45000:  */
		snprintf(tmpstr, sizeof(tmpstr), "clmpcc irq=%s.cpu[%i].pcc2 "
		    "addr=0x%08x name2=cl0", machine->path,
		    machine->bootstrap_cpu, 0xfff45000);
		machine->main_console_handle =
		    (size_t) device_add(machine, tmpstr);

		/*  ie0 ethernet: TODO  */
		device_add(machine, "unreadable addr=0xfff46000 len=0x1000");

		/*  53C710 SCSI at 0xfff47000:  */
		snprintf(tmpstr, sizeof(tmpstr), "osiop irq=%s.cpu[%i].pcc2.%i "
		    "addr=0x%08x", machine->path,
		    machine->bootstrap_cpu, PCC2V_SCSI, 0xfff47000);
		device_add(machine, tmpstr);

		/*  MK48T08 clock/nvram at 0xfffc0000:  */
		snprintf(tmpstr, sizeof(tmpstr),
		    "mk48txx addr=0x%x", 0xfffc0000);
		device_add(machine, tmpstr);

		/*  VMES (?): TODO  */
		device_add(machine, "unreadable addr=0xff780000 len=0x80000");

		/*  VME devices: TODO  */
		device_add(machine, "unreadable addr=0xffff0000 len=0x10000");

		break;

	case MACHINE_MVME88K_188:
		machine->machine_name = strdup("MVME188");
		/*  TODO  */
		break;

	case MACHINE_MVME88K_197:
		machine->machine_name = strdup("MVME197");
		/*  TODO  */
		break;

	default:fatal("Unimplemented MVME88K machine subtype %i\n",
		    machine->machine_subtype);
		exit(1);
	}

	if (!machine->prom_emulation)
		return;


	mvmeprom_init(machine);

	/*
	 *  Boot loader args, according to OpenBSD/mvme88k's locore.S:
	 *
	 *  r2 = boot flags
	 *  r3 = boot controller physical address
	 *  r4 = esym
	 *  r5 = start of miniroot
	 *  r6 = end of miniroot
	 *  r7 = ((Clun << 8) | Dlun): encoded bootdev
	 *  r8 = board type (0x187, 0x188, 0x197)
	 */

	/*  Some address beyond the loaded kernel:  */
	cpu->cd.m88k.r[4] = 6 * 1048576;

	switch (machine->machine_subtype) {
	case MACHINE_MVME88K_187:  cpu->cd.m88k.r[8] = 0x187; break;
	case MACHINE_MVME88K_188:  cpu->cd.m88k.r[8] = 0x188; break;
	case MACHINE_MVME88K_197:  cpu->cd.m88k.r[8] = 0x197; break;
	}

	/*  TODO: r3 and r7 must be set correctly to let
	    OpenBSD/mvme88k detect correct boot device.  */
}


MACHINE_DEFAULT_CPU(mvme88k)
{
	switch (machine->machine_subtype) {

	case MACHINE_MVME88K_187:
	case MACHINE_MVME88K_188:
		machine->cpu_name = strdup("88100");
		break;

	case MACHINE_MVME88K_197:
		machine->cpu_name = strdup("88110");
		break;

	default:fatal("Unimplemented MVME88K machine subtype %i\n",
		    machine->machine_subtype);
		exit(1);
	}
}


MACHINE_DEFAULT_RAM(mvme88k)
{
	machine->physical_ram_in_mb = 64;
}


MACHINE_REGISTER(mvme88k)
{
	MR_DEFAULT(mvme88k, "MVME88K", ARCH_M88K, MACHINE_MVME88K);

	machine_entry_add_alias(me, "mvme88k");

	machine_entry_add_subtype(me, "MVME187", MACHINE_MVME88K_187,
	    "mvme187", NULL);

	machine_entry_add_subtype(me, "MVME188", MACHINE_MVME88K_188,
	    "mvme188", NULL);

	machine_entry_add_subtype(me, "MVME197", MACHINE_MVME88K_197,
	    "mvme197", NULL);

	me->set_default_ram = machine_default_ram_mvme88k;
}

