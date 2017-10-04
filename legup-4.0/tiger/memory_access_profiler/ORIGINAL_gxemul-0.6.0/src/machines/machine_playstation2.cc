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
 *  COMMENT: Sony PlayStation 2
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cpu.h"
#include "device.h"
#include "devices.h"
#include "diskimage.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#define PLAYSTATION2_BDA        0xffffffffa0001000ULL
#define PLAYSTATION2_OPTARGS    0xffffffff81fff100ULL
#define PLAYSTATION2_SIFBIOS    0xffffffffbfc10000ULL


static int int_to_bcd(int i)
{
	return (i/10) * 16 + (i % 10);
}


MACHINE_SETUP(playstation2)
{
	char tmpstr[200];
	int tmplen;
	char *tmp;
	time_t timet;
	struct tm *tm_ptr;

	machine->machine_name = strdup("Playstation 2");
	cpu->byte_order = EMUL_LITTLE_ENDIAN;

	if (machine->physical_ram_in_mb != 32)
		fprintf(stderr, "WARNING! Playstation 2 machines are supposed "
		    "to have exactly 32 MB RAM. Continuing anyway.\n");
	if (!machine->x11_md.in_use)
		fprintf(stderr, "WARNING! Playstation 2 without -X is pretty "
		    "meaningless. Continuing anyway.\n");

	/*
	 *  According to NetBSD:
	 *
	 *	Hardware irq 0 is timer/interrupt controller
	 *	Hardware irq 1 is dma controller
	 *
	 *  Some things are not yet emulated (at all), and hence are detected
	 *  incorrectly:
	 *
	 *	sbus0 at mainbus0: controller type 2
	 *	ohci0 at sbus0		(at 0x1f801600, according to linux)
	 *	ohci0: OHCI version 1.0
	 */

	device_add(machine, "ps2 addr=0x10000000");
	device_add(machine, "ps2_gs addr=0x12000000");
	device_add(machine, "ps2_ether addr=0x14001000");

	/*  TODO: how much?  */
	dev_ram_init(machine, 0x1c000000, 4 * 1048576, DEV_RAM_RAM, 0);

	/*  OHCI at SBUS irq 1:  */
	snprintf(tmpstr, sizeof(tmpstr), "ohci addr=0x1f801600 irq="
	    "%s.cpu[%i].ps2_sbus.1", machine->path, machine->bootstrap_cpu);
	device_add(machine, tmpstr);

	/*  Set the Harddisk controller present flag, if either
	    disk 0 or 1 is present:  */
	if (diskimage_exist(machine, 0, DISKIMAGE_IDE) ||
	    diskimage_exist(machine, 1, DISKIMAGE_IDE)) {
		if (machine->prom_emulation)
			store_32bit_word(cpu, 0xa0000000 + machine->
			    physical_ram_in_mb*1048576 - 0x1000 + 0x0, 0x100);
		device_add(machine, "ps2_spd addr=0x14000000");
	}

	if (!machine->prom_emulation)
		return;


	tmplen = 1000;
	CHECK_ALLOCATION(tmp = (char *) malloc(tmplen));

	add_symbol_name(&machine->symbol_context,
	    PLAYSTATION2_SIFBIOS, 0x10000, "[SIFBIOS entry]", 0, 0);
	store_32bit_word(cpu, PLAYSTATION2_BDA + 0,
	    PLAYSTATION2_SIFBIOS);
	store_buf(cpu, PLAYSTATION2_BDA + 4, "PS2b", 4);

	/*  "Magic trap" instruction for software PROM emulation:  */
	store_32bit_word(cpu, PLAYSTATION2_SIFBIOS, 0x00c0de0c);

	store_32bit_word(cpu, 0xa0000000 + machine->physical_ram_in_mb
	    * 1048576 - 0x1000 + 0x4, PLAYSTATION2_OPTARGS);

	strlcpy(tmp, "root=/dev/hda1 crtmode=vesa0,60", tmplen);

	if (machine->boot_string_argument[0])
		snprintf(tmp+strlen(tmp), tmplen-strlen(tmp),
		    " %s", machine->boot_string_argument);
	tmp[tmplen-1] = '\0';

	machine->bootstr = tmp;
	store_string(cpu, PLAYSTATION2_OPTARGS, machine->bootstr);

	/*  TODO:  netbsd's bootinfo.h, for symbolic names  */

	/*  RTC data given by the BIOS:  */
	timet = time(NULL) + 9*3600;	/*  PS2 uses Japanese time  */
	tm_ptr = gmtime(&timet);
	/*  TODO:  are these 0- or 1-based?  */
	store_byte(cpu, 0xa0000000 + machine->physical_ram_in_mb
	    * 1048576 - 0x1000 + 0x10 + 1, int_to_bcd(tm_ptr->tm_sec));
	store_byte(cpu, 0xa0000000 + machine->physical_ram_in_mb
	    * 1048576 - 0x1000 + 0x10 + 2, int_to_bcd(tm_ptr->tm_min));
	store_byte(cpu, 0xa0000000 + machine->physical_ram_in_mb
	    * 1048576 - 0x1000 + 0x10 + 3, int_to_bcd(tm_ptr->tm_hour));
	store_byte(cpu, 0xa0000000 + machine->physical_ram_in_mb
	    * 1048576 - 0x1000 + 0x10 + 5, int_to_bcd(tm_ptr->tm_mday));
	store_byte(cpu, 0xa0000000 + machine->physical_ram_in_mb
	    * 1048576 - 0x1000 + 0x10 + 6, int_to_bcd(tm_ptr->tm_mon+1));
	store_byte(cpu, 0xa0000000 + machine->physical_ram_in_mb
	    * 1048576 - 0x1000 + 0x10 + 7, int_to_bcd(tm_ptr->tm_year-100));

	/*  "BOOTINFO_PCMCIA_TYPE" in NetBSD's bootinfo.h. This
	    contains the sbus controller type.  */
	store_32bit_word(cpu, 0xa0000000 + machine->physical_ram_in_mb
	    * 1048576 - 0x1000 + 0x1c, 2);
}


MACHINE_DEFAULT_CPU(playstation2)
{
	machine->cpu_name = strdup("R5900");
}


MACHINE_DEFAULT_RAM(playstation2)
{
	machine->physical_ram_in_mb = 32;
}


MACHINE_REGISTER(playstation2)
{
	MR_DEFAULT(playstation2, "Playstation 2", ARCH_MIPS, MACHINE_PS2);

	machine_entry_add_alias(me, "playstation2");
	machine_entry_add_alias(me, "ps2");

	me->set_default_ram = machine_default_ram_playstation2;
}

