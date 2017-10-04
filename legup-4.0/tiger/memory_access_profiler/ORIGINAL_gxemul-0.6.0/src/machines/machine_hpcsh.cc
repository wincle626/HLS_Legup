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
 *  COMMENT: Handheld SuperH-based machines
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

#include "thirdparty/hpc_bootinfo.h"
#include "thirdparty/vripreg.h"


MACHINE_SETUP(hpcsh)
{
/*
	char tmpstr[1000];
	struct hpc_bootinfo hpc_bootinfo;
	int hpc_platid_flags = 0, hpc_platid_cpu_submodel = 0,
	    hpc_platid_cpu_model = 0, hpc_platid_cpu_series = 0,
	    hpc_platid_cpu_arch = 0,
	    hpc_platid_submodel = 0, hpc_platid_model = 0,
	    hpc_platid_series = 0, hpc_platid_vendor = 0;
	uint64_t hpc_fb_addr = 0;
	int hpc_fb_bits = 0, hpc_fb_encoding = 0;
	int hpc_fb_xsize = 0;
	int hpc_fb_ysize = 0;
	int hpc_fb_xsize_mem = 0;
	int hpc_fb_ysize_mem = 0;

	memset(&hpc_bootinfo, 0, sizeof(hpc_bootinfo));
*/
	machine->machine_name = strdup("HPCsh");
	cpu->byte_order = EMUL_LITTLE_ENDIAN;

	if (!machine->x11_md.in_use)
		fprintf(stderr, "-------------------------------------"
		    "------------------------------------------\n"
		    "\n  WARNING!  You are emulating a HPCsh without -X."
		    "\n            You will miss graphical output!\n\n"
		    "-------------------------------------"
		    "------------------------------------------\n");

	/*  32 MB in two parts, each included twice (shadowed):  */
	dev_ram_init(machine, 0x0c000000, 0x01000000, DEV_RAM_MIRROR, 0x0);
	dev_ram_init(machine, 0x0d000000, 0x01000000, DEV_RAM_MIRROR, 0x0);
	dev_ram_init(machine, 0x0e000000, 0x01000000, DEV_RAM_MIRROR,
	    0x01000000);
	dev_ram_init(machine, 0x0f000000, 0x01000000, DEV_RAM_MIRROR,
	    0x01000000);

	dev_fb_init(machine, machine->memory, 0x10000000,
	    VFB_HPC, 640,240, 640,240, 16, machine->machine_name);
}


MACHINE_DEFAULT_CPU(hpcsh)
{
	machine->cpu_name = strdup("SH7750");
}


MACHINE_DEFAULT_RAM(hpcsh)
{
	/*  TODO: Model dependent. */
	machine->physical_ram_in_mb = 32;
}


MACHINE_REGISTER(hpcsh)
{
	MR_DEFAULT(hpcsh, "Handhelp SH (HPCsh)", ARCH_SH, MACHINE_HPCSH);

	machine_entry_add_alias(me, "hpcsh");

	machine_entry_add_subtype(me, "Jornada 680",
	    MACHINE_HPCSH_JORNADA680, "jornada680", NULL);

	machine_entry_add_subtype(me, "Jornada 690",
	    MACHINE_HPCSH_JORNADA690, "jornada690", NULL);

	me->set_default_ram = machine_default_ram_hpcsh;
}

