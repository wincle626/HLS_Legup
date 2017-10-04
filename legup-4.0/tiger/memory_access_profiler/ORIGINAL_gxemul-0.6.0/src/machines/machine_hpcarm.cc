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
 *  COMMENT: Handheld ARM-based machines
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


MACHINE_SETUP(hpcarm)
{
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

	cpu->byte_order = EMUL_LITTLE_ENDIAN;

	switch (machine->machine_subtype) {

	case MACHINE_HPCARM_IPAQ:
		/*  SA-1110 206MHz  */
		machine->machine_name = strdup("Compaq iPAQ H3600");
		hpc_fb_addr = 0x48200000;	/*  TODO  */
		hpc_fb_xsize = 240;
		hpc_fb_ysize = 320;
		hpc_fb_xsize_mem = 256;
		hpc_fb_ysize_mem = 320;
		hpc_fb_bits = 15;
		hpc_fb_encoding = BIFB_D16_0000;
		hpc_platid_cpu_arch = 3;	/*  ARM  */
		hpc_platid_cpu_series = 1;	/*  StrongARM  */
		hpc_platid_cpu_model = 2;	/*  SA-1110  */
		hpc_platid_cpu_submodel = 0;
		hpc_platid_vendor = 7;		/*  Compaq  */
		hpc_platid_series = 4;		/*  IPAQ  */
		hpc_platid_model = 2;		/*  H36xx  */
		hpc_platid_submodel = 1;	/*  H3600  */
		break;

	case MACHINE_HPCARM_JORNADA720:
	case MACHINE_HPCARM_JORNADA728:
		/*  SA-1110 206MHz  */
		machine->machine_name = (machine->machine_subtype ==
		    MACHINE_HPCARM_JORNADA720) ?
		    (char *)"Jornada 720" : (char *)"Jornada 728";
		hpc_fb_addr = 0x48200000;
		hpc_fb_xsize = 640;
		hpc_fb_ysize = 240;
		hpc_fb_xsize_mem = 640;
		hpc_fb_ysize_mem = 240;
		hpc_fb_bits = 16;
		hpc_fb_encoding = BIFB_D16_0000;
		hpc_platid_cpu_arch = 3;	/*  ARM  */
		hpc_platid_cpu_series = 1;	/*  StrongARM  */
		hpc_platid_cpu_model = 2;	/*  SA-1110  */
		hpc_platid_cpu_submodel = 0;
		hpc_platid_vendor = 11;		/*  HP  */
		hpc_platid_series = 2;		/*  Jornada  */
		hpc_platid_model = 2;		/*  7xx  */
		hpc_platid_submodel = 1;	/*  720  */
		break;

	default:printf("Unimplemented hpcarm machine number.\n");
		exit(1);
	}

	store_32bit_word_in_host(cpu, (unsigned char *)&hpc_bootinfo.platid_cpu,
	      (hpc_platid_cpu_arch << 26) + (hpc_platid_cpu_series << 20)
	    + (hpc_platid_cpu_model << 14) + (hpc_platid_cpu_submodel <<  8)
	    + hpc_platid_flags);
	store_32bit_word_in_host(cpu, (unsigned char *)&hpc_bootinfo.
	    platid_machine,(hpc_platid_vendor << 22) + (hpc_platid_series << 16)
	    + (hpc_platid_model <<  8) + hpc_platid_submodel);

	/*  Physical RAM at 0xc0000000:  */
	dev_ram_init(machine, 0xc0000000, 0x20000000,
	    DEV_RAM_MIRROR, 0x0);

	/*  Cache flush region:  */
	dev_ram_init(machine, 0xe0000000, 0x10000, DEV_RAM_RAM, 0x0);

	if (hpc_fb_addr != 0) {
		dev_fb_init(machine, machine->memory, hpc_fb_addr, VFB_HPC,
		    hpc_fb_xsize, hpc_fb_ysize,
		    hpc_fb_xsize_mem, hpc_fb_ysize_mem,
		    hpc_fb_bits, machine->machine_name);
	}

	if (!machine->prom_emulation)
		return;


	/*  NetBSD/hpcarm and possibly others expects the following:  */

	cpu->cd.arm.r[0] = 1;	/*  argc  */
	cpu->cd.arm.r[1] = machine->physical_ram_in_mb * 1048576 - 512;	/*argv*/
	cpu->cd.arm.r[2] = machine->physical_ram_in_mb * 1048576 - 256;
		/*  r[2] = ptr to hpc_bootinfo  */

	machine->bootstr = machine->boot_kernel_filename;
	store_32bit_word(cpu, machine->physical_ram_in_mb * 1048576 - 512,
	    machine->physical_ram_in_mb * 1048576 - 512 + 16);
	store_32bit_word(cpu, machine->physical_ram_in_mb * 1048576 - 512 +4,0);
	store_string(cpu, machine->physical_ram_in_mb * 1048576 - 512 + 16,
	    machine->bootstr);

	if (machine->boot_string_argument[0]) {
		cpu->cd.arm.r[0] ++;	/*  argc  */

		store_32bit_word(cpu, machine->physical_ram_in_mb * 1048576 -
		    512 + 4, machine->physical_ram_in_mb * 1048576 - 512 + 64);
		store_32bit_word(cpu, machine->physical_ram_in_mb * 1048576 -
		    512 + 8, 0);

		store_string(cpu, machine->physical_ram_in_mb * 1048576 - 512
		    + 64, machine->boot_string_argument);

		machine->bootarg = machine->boot_string_argument;
	}

	store_16bit_word_in_host(cpu, (unsigned char *)&hpc_bootinfo.length,
	    sizeof(hpc_bootinfo));
	store_32bit_word_in_host(cpu, (unsigned char *)&hpc_bootinfo.magic,
	    HPC_BOOTINFO_MAGIC);
	store_32bit_word_in_host(cpu, (unsigned char *)&hpc_bootinfo.fb_addr,
	    hpc_fb_addr);
	store_16bit_word_in_host(cpu, (unsigned char *)&hpc_bootinfo.
	    fb_line_bytes, hpc_fb_xsize_mem * (((hpc_fb_bits-1)|7)+1) / 8);
	store_16bit_word_in_host(cpu, (unsigned char *)&hpc_bootinfo.fb_width,
	    hpc_fb_xsize);
	store_16bit_word_in_host(cpu, (unsigned char *)&hpc_bootinfo.fb_height,
	    hpc_fb_ysize);
	store_16bit_word_in_host(cpu, (unsigned char *)&hpc_bootinfo.fb_type,
	    hpc_fb_encoding);
	store_16bit_word_in_host(cpu, (unsigned char *)&hpc_bootinfo.bi_cnuse,
	    machine->x11_md.in_use? BI_CNUSE_BUILTIN : BI_CNUSE_SERIAL);

	store_32bit_word_in_host(cpu,(unsigned char *)&hpc_bootinfo.timezone,0);
	store_buf(cpu, machine->physical_ram_in_mb * 1048576 - 256,
	    (char *)&hpc_bootinfo, sizeof(hpc_bootinfo));

	/*  TODO: What is a good default stack pointer?  */
	/*  I.e. what does hpcboot.exe for NetBSD/hpcarm usually use?  */
	cpu->cd.arm.r[ARM_SP] = 0xc02dfff0;
}


MACHINE_DEFAULT_CPU(hpcarm)
{
	machine->cpu_name = strdup("SA1110");
}


MACHINE_DEFAULT_RAM(hpcarm)
{
	switch (machine->machine_subtype) {
	case MACHINE_HPCARM_JORNADA728:
		/*  720 has 32 MB, 728 has 64 MB.  */
		machine->physical_ram_in_mb = 64;
		break;
	default:
		/*  Most have 32 MB by default.  */
		machine->physical_ram_in_mb = 32;
	}
}


MACHINE_REGISTER(hpcarm)
{
	MR_DEFAULT(hpcarm, "Handhelp ARM (HPCarm)", ARCH_ARM, MACHINE_HPCARM);

	machine_entry_add_alias(me, "hpcarm");

	machine_entry_add_subtype(me, "Ipaq", MACHINE_HPCARM_IPAQ,
	    "ipaq", NULL);

	machine_entry_add_subtype(me, "Jornada 720", MACHINE_HPCARM_JORNADA720,
	    "jornada720", NULL);

	machine_entry_add_subtype(me, "Jornada 728", MACHINE_HPCARM_JORNADA728,
	    "jornada728", NULL);

	me->set_default_ram = machine_default_ram_hpcarm;
}

