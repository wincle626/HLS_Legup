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
 *  COMMENT: Machine mimicing QEMU's default MIPS emulation mode
 *
 *  See e.g. http://fabrice.bellard.free.fr/qemu/mips-test-0.2.tar.gz
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bus_isa.h"
#include "cpu.h"
#include "device.h"
#include "devices.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"


MACHINE_SETUP(qemu_mips)
{
	char tmpstr[300];

	machine->machine_name = strdup("QEMU MIPS");
	cpu->byte_order = EMUL_BIG_ENDIAN;

	/*
	 *  An ISA bus, I/O ports at 0x14000000, memory at 0x10000000,
	 *  connected to MIPS irq 2:
	 */
	snprintf(tmpstr, sizeof(tmpstr), "%s.cpu[%i].2",
	    machine->path, machine->bootstrap_cpu);
	bus_isa_init(machine, tmpstr, BUS_ISA_IDE0 | BUS_ISA_IDE1,
	    0x14000000ULL, 0x10000000ULL);

	if (!machine->prom_emulation)
		return;

	/*
	 *  Registers at startup do not seem to be defined in QEMU, but
	 *  bootargs and memory size are placed just below 16 MB.
	 *
	 *  Remember to start the emulator with options, e.g.:
	 *
	 *	-o "console=ttyS0 root=/dev/ram rd_start=0x80800000
	 *		rd_size=10000000 init=/bin/sh"
	 */

	store_string(cpu, (int32_t)(0x80000000 + 16*1048576 - 256),
	    machine->boot_string_argument);
	store_32bit_word(cpu, (int32_t)(0x80000000 + 16*1048576 - 260),
	    0x12345678);
	store_32bit_word(cpu, (int32_t)(0x80000000 + 16*1048576 - 264),
	    machine->physical_ram_in_mb * 1048576);
}


MACHINE_DEFAULT_CPU(qemu_mips)
{
	/*  QEMU emulates a MIPS32 rev 1, so 4Kc will do just fine.  */
	machine->cpu_name = strdup("4Kc");
}


MACHINE_DEFAULT_RAM(qemu_mips)
{
	machine->physical_ram_in_mb = 64;
}


MACHINE_REGISTER(qemu_mips)
{
	MR_DEFAULT(qemu_mips, "QEMU MIPS", ARCH_MIPS, MACHINE_QEMU_MIPS);
	me->set_default_ram = machine_default_ram_qemu_mips;
	machine_entry_add_alias(me, "qemu_mips");
}

