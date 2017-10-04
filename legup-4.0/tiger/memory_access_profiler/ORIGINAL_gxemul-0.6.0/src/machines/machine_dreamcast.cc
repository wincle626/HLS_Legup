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
 *  COMMENT: SEGA Dreamcast
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


MACHINE_SETUP(dreamcast)
{
	machine->machine_name = strdup("Dreamcast");

	if (machine->emulated_hz == 0)
		machine->emulated_hz = 200000000;

	/*  50 MHz SH4 PCLOCK:  */
	machine->cpus[0]->cd.sh.pclock = 50000000;

	if (!machine->x11_md.in_use)
		fprintf(stderr, "-------------------------------------"
		    "------------------------------------------\n"
		    "\n  WARNING!  You are emulating a Dreamcast without -X."
		    "\n            You will miss graphical output!\n\n"
		    "-------------------------------------"
		    "------------------------------------------\n");

	/*
	 *  Physical address layout on the Dreamcast, according to a
	 *  combination of sources:  NetBSD sources, KalistOS sources,
	 *  http://www.boob.co.uk/docs/Dreamcast_memory.txt, and
	 *  http://www.ludd.luth.se/~jlo/dc/memory.txt, and possibly some
	 *  others:
	 *
	 *  0x00000000 - 0x001fffff	Boot ROM (2 MB)
	 *  0x00200000 - 0x003fffff	Flash (256 KB)
	 *  0x005f6800 - ...		PowerVR2 DMA registers
	 *  0x005f6900 - ...		ASIC registers
	 *  0x005f6c00 - ...		Maple registers (controller ports)
	 *  0x005f7000 - ...		GDROM registers
	 *  0x005f7400 - ...		???
	 *  0x005f74e4 - ...		GDROM re-enable disabled drive (?)
	 *  0x005f7800 - ...		G2 External DMA registers
	 *  0x005f7c00 - ...		???
	 *  0x005f8000 - 0x005f9fff	PVR registers (graphics)
	 *  0x00600400 - 0x0060047f	LAN Adapter (MB86967) registers
	 *  0x00700000 - ...		SPU registers (sound)
	 *  0x00702c00 -		Cable select and AICA (?) (*3)
	 *  0x00710000 - 0x00710007	RTC registers
	 *  0x00800000 - 0x009fffff	Sound RAM (2 MB)
	 *  0x01000000 - ...		Parallel port registers
	 *  0x02000000 - ...		CD-ROM port registers
	 *  0x04000000 - 0x047fffff	Video RAM (*)     (64-bit)
	 *  0x05000000 - 0x057fffff	Video RAM (8 MB)  (32-bit)
	 *  0x0c000000 - 0x0cffffff	RAM (16 MB)
	 *  0x0e000000 - 0x0effffff	Copy of RAM? (*2)
	 *  0x10000000 - ...		Tile accelerator command area
	 *  0x10800000 - ...		Write-only mirror of Video RAM (?)
	 *  0x11000000 - ...		PVR DMA area??
	 *  0x14000000 - ...		G2 (?)  Or Modem/Extension port?
	 *
	 *  (*) = with banks 0 and 1 switched; 64-bit read/write access...
	 *  (*3) = See VOUTC in Linux' drivers/video/pvr2fb.c.
	 */

	dev_ram_init(machine, 0x00702c00, 4, DEV_RAM_RAM, 0);

	/*  Sound RAM:  */
	dev_ram_init(machine, 0x00800000, 2 * 1048576, DEV_RAM_RAM, 0);

	/*
	 *  HACK!  TODO: Remove this device at 0x00a00000 once NetBSD has
	 *  been fixed to not clear 6 MB beyound the sound RAM area.
	 */
	dev_ram_init(machine, 0x00a00000, 6 * 1048576, DEV_RAM_RAM, 0);

	dev_ram_init(machine, 0x0c000000, 16 * 1048576, DEV_RAM_RAM, 0x0);

	/*  The "luftvarg" 4KB intro uses memory at paddr 0x0ef00000...  */
	/*  (*2)   (TODO: Make this a _mirror_ of 0x0c000000?)  */
	dev_ram_init(machine, 0x0e000000, 16 * 1048576, DEV_RAM_RAM, 0);

	device_add(machine, "pvr");
/*	device_add(machine, "mb8696x addr=0x600400 addr_mult=4");  */
	device_add(machine, "dreamcast_asic");
	device_add(machine, "dreamcast_g2");
	device_add(machine, "dreamcast_gdrom");
	device_add(machine, "dreamcast_maple");
	device_add(machine, "dreamcast_rtc");

	if (!machine->prom_emulation)
		return;

	dreamcast_machine_setup(machine);
}


MACHINE_DEFAULT_CPU(dreamcast)
{
	/*  Hitachi SH4, 200 MHz  */
	machine->cpu_name = strdup("SH7750");
}


MACHINE_DEFAULT_RAM(dreamcast)
{
	/*  Note: This is the size of the boot ROM area, since the
	    Dreamcast's RAM isn't located at physical address zero.  */
	machine->physical_ram_in_mb = 2;
}


MACHINE_REGISTER(dreamcast)
{
	MR_DEFAULT(dreamcast, "Dreamcast", ARCH_SH, MACHINE_DREAMCAST);
	me->set_default_ram = machine_default_ram_dreamcast;
	machine_entry_add_alias(me, "dreamcast");
}

