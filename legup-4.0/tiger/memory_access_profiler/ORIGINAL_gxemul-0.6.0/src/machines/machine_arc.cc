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
 *  COMMENT: ARC (Advanced RISC Computing) machines
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arcbios.h"
#include "cpu.h"
#include "device.h"
#include "devices.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#define	MACHINE_NAME_MAXBUF	100


MACHINE_SETUP(arc)
{
	void *jazz_data;
	struct memory *mem = machine->memory;
	char tmpstr[1000];
	char tmpstr2[1000];
	int i, j;
	const char *eaddr_string = "eaddr=10:20:30:40:50:60";		/*  bogus  */
	unsigned char macaddr[6];

	CHECK_ALLOCATION(machine->machine_name = (char *) malloc(MACHINE_NAME_MAXBUF));

	cpu->byte_order = EMUL_LITTLE_ENDIAN;
	snprintf(machine->machine_name, MACHINE_NAME_MAXBUF, "ARC");

	switch (machine->machine_subtype) {

	case MACHINE_ARC_JAZZ_PICA:
	case MACHINE_ARC_JAZZ_MAGNUM:
		/*
		 *  "PICA-61"
		 *
		 *  According to NetBSD 1.6.2:
		 *
		 *  jazzio0 at mainbus0
		 *  timer0 at jazzio0 addr 0xe0000228
		 *  mcclock0 at jazzio0 addr 0xe0004000: mc146818 or compatible
		 *  lpt at jazzio0 addr 0xe0008000 intr 0 not configured
		 *  fdc at jazzio0 addr 0xe0003000 intr 1 not configured
		 *  MAGNUM at jazzio0 addr 0xe000c000 intr 2 not configured
		 *  ALI_S3 at jazzio0 addr 0xe0800000 intr 3 not configured
		 *  sn0 at jazzio0 addr 0xe0001000 intr 4: SONIC Ethernet
		 *  sn0: Ethernet address 69:6a:6b:6c:00:00
		 *  asc0 at jazzio0 addr 0xe0002000 intr 5: NCR53C94, target 0
		 *  pckbd at jazzio0 addr 0xe0005000 intr 6 not configured
		 *  pms at jazzio0 addr 0xe0005000 intr 7 not configured
		 *  com0 at jazzio0 addr 0xe0006000 intr 8: ns16550a,
		 *	working fifo
		 *  com at jazzio0 addr 0xe0007000 intr 9 not configured
		 *  jazzisabr0 at mainbus0
		 *  isa0 at jazzisabr0 isa_io_base 0xe2000000 isa_mem_base
		 *	0xe3000000
		 *
		 *  "Microsoft-Jazz", "MIPS Magnum"
		 *
		 *  timer0 at jazzio0 addr 0xe0000228
		 *  mcclock0 at jazzio0 addr 0xe0004000: mc146818 or compatible
		 *  lpt at jazzio0 addr 0xe0008000 intr 0 not configured
		 *  fdc at jazzio0 addr 0xe0003000 intr 1 not configured
		 *  MAGNUM at jazzio0 addr 0xe000c000 intr 2 not configured
		 *  VXL at jazzio0 addr 0xe0800000 intr 3 not configured
		 *  sn0 at jazzio0 addr 0xe0001000 intr 4: SONIC Ethernet
		 *  sn0: Ethernet address 69:6a:6b:6c:00:00
		 *  asc0 at jazzio0 addr 0xe0002000 intr 5: NCR53C94, target 0
		 *  scsibus0 at asc0: 8 targets, 8 luns per target
		 *  pckbd at jazzio0 addr 0xe0005000 intr 6 not configured
		 *  pms at jazzio0 addr 0xe0005000 intr 7 not configured
		 *  com0 at jazzio0 addr 0xe0006000 intr 8: ns16550a,
		 *	working fifo
		 *  com at jazzio0 addr 0xe0007000 intr 9 not configured
		 *  jazzisabr0 at mainbus0
		 *  isa0 at jazzisabr0 isa_io_base 0xe2000000 isa_mem_base
		 *	0xe3000000
		 */

		switch (machine->machine_subtype) {
		case MACHINE_ARC_JAZZ_PICA:
			strlcat(machine->machine_name,
			    " (Microsoft Jazz, Acer PICA-61)",
			    MACHINE_NAME_MAXBUF);
			break;
		case MACHINE_ARC_JAZZ_MAGNUM:
			strlcat(machine->machine_name,
			    " (Microsoft Jazz, MIPS Magnum)",
			    MACHINE_NAME_MAXBUF);
			break;
		default:
			fatal("error in machine.c. jazz\n");
			exit(1);
		}

		jazz_data = device_add(machine, "jazz addr=0x80000000");

		/*  Keyboard IRQ is jazz.6, mouse is jazz.7  */
		snprintf(tmpstr, sizeof(tmpstr),
		    "%s.cpu[%i].jazz.6", machine->path,
		    machine->bootstrap_cpu);
		snprintf(tmpstr2, sizeof(tmpstr2),
		    "%s.cpu[%i].jazz.7", machine->path,
		    machine->bootstrap_cpu);
		i = dev_pckbc_init(machine, mem, 0x80005000ULL,
		    PCKBC_JAZZ, tmpstr, tmpstr2,
		    machine->x11_md.in_use, 0);

		/*  Serial controllers at JAZZ irq 8 and 9:  */
		snprintf(tmpstr, sizeof(tmpstr),
		    "ns16550 irq=%s.cpu[%i].jazz.8 addr=0x80006000"
		    " in_use=%i name2=tty0", machine->path,
		    machine->bootstrap_cpu, machine->x11_md.in_use? 0 : 1);
		j = (size_t)device_add(machine, tmpstr);
		snprintf(tmpstr, sizeof(tmpstr),
		    "ns16550 irq=%s.cpu[%i].jazz.9 addr=0x80007000"
		    " in_use=0 name2=tty1", machine->path,
		    machine->bootstrap_cpu);
		device_add(machine, tmpstr);

		if (machine->x11_md.in_use)
			machine->main_console_handle = i;
		else
			machine->main_console_handle = j;

		switch (machine->machine_subtype) {
		case MACHINE_ARC_JAZZ_PICA:
			if (machine->x11_md.in_use) {
				dev_vga_init(machine, mem, 0x400a0000ULL,
				    0x600003c0ULL, machine->machine_name);
				arcbios_console_init(machine,
				    0x400b8000ULL, 0x600003c0ULL);
			}
			break;
		case MACHINE_ARC_JAZZ_MAGNUM:
			/*  PROM mirror?  */
			dev_ram_init(machine, 0xfff00000, 0x100000,
			    DEV_RAM_MIRROR | DEV_RAM_MIGHT_POINT_TO_DEVICES,
			    0x1fc00000);

			/*  VXL. TODO  */
			/*  control at 0x60100000?  */
			dev_fb_init(machine, mem, 0x60200000ULL,
			    VFB_GENERIC, 1024,768, 1024,768, 8, "VXL");
			break;
		}

		/*  SN at JAZZ irq 4  */
		snprintf(tmpstr, sizeof(tmpstr),
		    "sn addr=0x80001000 irq=%s.cpu[%i].jazz.4",
		    machine->path, machine->bootstrap_cpu);
		device_add(machine, tmpstr);

		/*  ASC at JAZZ irq 5  */
		snprintf(tmpstr, sizeof(tmpstr), "%s.cpu[%i].jazz.5", 
		    machine->path, machine->bootstrap_cpu);
		dev_asc_init(machine, mem, 0x80002000ULL, tmpstr, NULL,
		    DEV_ASC_PICA, dev_jazz_dma_controller, jazz_data);

		/*  FDC at JAZZ irq 1  */
		snprintf(tmpstr, sizeof(tmpstr),
		    "fdc addr=0x80003000 irq=%s.cpu[%i].jazz.1",
		    machine->path, machine->bootstrap_cpu);
		device_add(machine, tmpstr);

		/*  MC146818 at MIPS irq 2:  */
		snprintf(tmpstr, sizeof(tmpstr), "%s.cpu[%i].2", 
		    machine->path, machine->bootstrap_cpu);
		dev_mc146818_init(machine, mem,
		    0x80004000ULL, tmpstr, MC146818_ARC_JAZZ, 1);

#if 0
Not yet.
		/*  WDC at ISA irq 14  */
		device_add(machine, "wdc addr=0x900001f0, irq=38");
#endif

		break;

	default:fatal("Unimplemented ARC machine type %i\n",
		    machine->machine_subtype);
		exit(1);
	}

	/*
	 *  NOTE: ARCBIOS shouldn't be used before this point. (The only
	 *  exception is that arcbios_console_init() may be called.)
	 */

	if (!machine->prom_emulation)
		return;

	arcbios_init(machine, 0, 0, eaddr_string, macaddr);
}


MACHINE_DEFAULT_CPU(arc)
{
	switch (machine->machine_subtype) {

	case MACHINE_ARC_JAZZ_PICA:
		CHECK_ALLOCATION(machine->cpu_name = strdup("R4000"));
		break;

	default:
		CHECK_ALLOCATION(machine->cpu_name = strdup("R4400"));
	}
}


MACHINE_DEFAULT_RAM(arc)
{
	machine->physical_ram_in_mb = 64;
}


MACHINE_REGISTER(arc)
{
	MR_DEFAULT(arc, "ARC", ARCH_MIPS, MACHINE_ARC);

	me->set_default_ram = machine_default_ram_arc;

	machine_entry_add_alias(me, "arc");

	machine_entry_add_subtype(me, "Acer PICA-61", MACHINE_ARC_JAZZ_PICA,
	    "pica-61", "acer pica", "pica", NULL);

	machine_entry_add_subtype(me, "Jazz Magnum", MACHINE_ARC_JAZZ_MAGNUM,
	    "magnum", "jazz magnum", NULL);
}

