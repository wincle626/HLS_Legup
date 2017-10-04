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
 *  COMMENT: Handheld MIPS-based machines
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


MACHINE_SETUP(hpcmips)
{
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

	cpu->byte_order = EMUL_LITTLE_ENDIAN;
	memset(&hpc_bootinfo, 0, sizeof(hpc_bootinfo));

	/*
	 *  NOTE: See http://forums.projectmayo.com/viewtopic.php?topic=2743&
	 *	forum=23 for info on framebuffer addresses.
	 */

	switch (machine->machine_subtype) {

	case MACHINE_HPCMIPS_CASIO_BE300:
		/*  166MHz VR4131  */
		machine->machine_name = strdup("Casio Cassiopeia BE-300");
		hpc_fb_addr = 0x0a200000;
		hpc_fb_xsize = 240;
		hpc_fb_ysize = 320;
		hpc_fb_xsize_mem = 256;
		hpc_fb_ysize_mem = 320;
		hpc_fb_bits = 15;
		hpc_fb_encoding = BIFB_D16_0000;

		/*  TODO: irq?  */
		snprintf(tmpstr, sizeof(tmpstr), "ns16550 irq=0 addr=0x"
		    "0a008680 addr_mult=4 in_use=%i", !machine->x11_md.in_use);
		machine->main_console_handle = (size_t)
		    device_add(machine, tmpstr);

		dev_vr41xx_init(machine, machine->memory, 4131);

		hpc_platid_cpu_arch = 1;	/*  MIPS  */
		hpc_platid_cpu_series = 1;	/*  VR  */
		hpc_platid_cpu_model = 1;	/*  VR41XX  */
		hpc_platid_cpu_submodel = 6;	/*  VR4131  */
		hpc_platid_vendor = 3;		/*  Casio  */
		hpc_platid_series = 1;		/*  CASSIOPEIAE  */
		hpc_platid_model = 2;		/*  EXXX  */
		hpc_platid_submodel = 3;	/*  E500  */
		/*  TODO: Don't use model number for E500, it's a BE300!  */
		break;

	case MACHINE_HPCMIPS_CASIO_E105:
		/*  131MHz VR4121  */
		machine->machine_name = strdup("Casio Cassiopeia E-105");
		hpc_fb_addr = 0x0a200000;	/*  TODO?  */
		hpc_fb_xsize = 240;
		hpc_fb_ysize = 320;
		hpc_fb_xsize_mem = 256;
		hpc_fb_ysize_mem = 320;
		hpc_fb_bits = 16;
		hpc_fb_encoding = BIFB_D16_0000;

		/*  TODO: irq?  */
		snprintf(tmpstr, sizeof(tmpstr), "ns16550 irq=0 addr=0x"
		    "0a008680 addr_mult=4 in_use=%i", !machine->x11_md.in_use);
		machine->main_console_handle = (size_t)
		    device_add(machine, tmpstr);

		dev_vr41xx_init(machine, machine->memory, 4121);

		hpc_platid_cpu_arch = 1;	/*  MIPS  */
		hpc_platid_cpu_series = 1;	/*  VR  */
		hpc_platid_cpu_model = 1;	/*  VR41XX  */
		hpc_platid_cpu_submodel = 3;	/*  VR4121  */
		hpc_platid_vendor = 3;		/*  Casio  */
		hpc_platid_series = 1;		/*  CASSIOPEIAE  */
		hpc_platid_model = 2;		/*  EXXX  */
		hpc_platid_submodel = 2;	/*  E105  */
		break;

	case MACHINE_HPCMIPS_NEC_MOBILEPRO_770:
		/*  131 MHz VR4121  */
		machine->machine_name = strdup("NEC MobilePro 770");
		hpc_fb_addr = 0xa000000;
		hpc_fb_xsize = 640;
		hpc_fb_ysize = 240;
		hpc_fb_xsize_mem = 800;
		hpc_fb_ysize_mem = 240;
		hpc_fb_bits = 16;
		hpc_fb_encoding = BIFB_D16_0000;

		dev_vr41xx_init(machine, machine->memory, 4121);

		hpc_platid_cpu_arch = 1;	/*  MIPS  */
		hpc_platid_cpu_series = 1;	/*  VR  */
		hpc_platid_cpu_model = 1;	/*  VR41XX  */
		hpc_platid_cpu_submodel = 3;	/*  VR4121  */
		hpc_platid_vendor = 1;		/*  NEC  */
		hpc_platid_series = 2;		/*  NEC MCR  */
		hpc_platid_model = 2;		/*  MCR 5XX  */
		hpc_platid_submodel = 4;	/*  MCR 520A  */
		break;

	case MACHINE_HPCMIPS_NEC_MOBILEPRO_780:
		/*  166 (or 168) MHz VR4121  */
		machine->machine_name = strdup("NEC MobilePro 780");
		hpc_fb_addr = 0xa180100;
		hpc_fb_xsize = 640;
		hpc_fb_ysize = 240;
		hpc_fb_xsize_mem = 640;
		hpc_fb_ysize_mem = 240;
		hpc_fb_bits = 16;
		hpc_fb_encoding = BIFB_D16_0000;

		dev_vr41xx_init(machine, machine->memory, 4121);

		hpc_platid_cpu_arch = 1;	/*  MIPS  */
		hpc_platid_cpu_series = 1;	/*  VR  */
		hpc_platid_cpu_model = 1;	/*  VR41XX  */
		hpc_platid_cpu_submodel = 3;	/*  VR4121  */
		hpc_platid_vendor = 1;		/*  NEC  */
		hpc_platid_series = 2;		/*  NEC MCR  */
		hpc_platid_model = 2;		/*  MCR 5XX  */
		hpc_platid_submodel = 8;	/*  MCR 530A  */
		break;

	case MACHINE_HPCMIPS_NEC_MOBILEPRO_800:
		/*  131 MHz VR4121  */
		machine->machine_name = strdup("NEC MobilePro 800");
		hpc_fb_addr = 0xa000000;
		hpc_fb_xsize = 800;
		hpc_fb_ysize = 600;
		hpc_fb_xsize_mem = 800;
		hpc_fb_ysize_mem = 600;
		hpc_fb_bits = 16;
		hpc_fb_encoding = BIFB_D16_0000;

		dev_vr41xx_init(machine, machine->memory, 4121);

		hpc_platid_cpu_arch = 1;	/*  MIPS  */
		hpc_platid_cpu_series = 1;	/*  VR  */
		hpc_platid_cpu_model = 1;	/*  VR41XX  */
		hpc_platid_cpu_submodel = 3;	/*  VR4121  */
		hpc_platid_vendor = 1;		/*  NEC  */
		hpc_platid_series = 2;		/*  NEC MCR  */
		hpc_platid_model = 3;		/*  MCR 7XX  */
		hpc_platid_submodel = 2;	/*  MCR 700A  */
		break;

	case MACHINE_HPCMIPS_NEC_MOBILEPRO_880:
		/*  168 MHz VR4121  */
		machine->machine_name = strdup("NEC MobilePro 880");
		hpc_fb_addr = 0xa0ea600;
		hpc_fb_xsize = 800;
		hpc_fb_ysize = 600;
		hpc_fb_xsize_mem = 800;
		hpc_fb_ysize_mem = 600;
		hpc_fb_bits = 16;
		hpc_fb_encoding = BIFB_D16_0000;

		dev_vr41xx_init(machine, machine->memory, 4121);

		hpc_platid_cpu_arch = 1;	/*  MIPS  */
		hpc_platid_cpu_series = 1;	/*  VR  */
		hpc_platid_cpu_model = 1;	/*  VR41XX  */
		hpc_platid_cpu_submodel = 3;	/*  VR4121  */
		hpc_platid_vendor = 1;		/*  NEC  */
		hpc_platid_series = 2;		/*  NEC MCR  */
		hpc_platid_model = 3;		/*  MCR 7XX  */
		hpc_platid_submodel = 4;	/*  MCR 730A  */
		break;

	case MACHINE_HPCMIPS_AGENDA_VR3:
		/*  66 MHz VR4181  */
		machine->machine_name = strdup("Agenda VR3");
		/*  TODO:  */
		hpc_fb_addr = 0x1000;
		hpc_fb_xsize = 160;
		hpc_fb_ysize = 240;
		hpc_fb_xsize_mem = 160;
		hpc_fb_ysize_mem = 240;
		hpc_fb_bits = 4;
		hpc_fb_encoding = BIFB_D4_M2L_F;

		dev_vr41xx_init(machine, machine->memory, 4181);

		/*  TODO: Hm... irq 17 according to linux, but
		    VRIP_INTR_SIU (=9) here?  */
		{
			int x;
			snprintf(tmpstr, sizeof(tmpstr),
			    "ns16550 irq=%i addr=0x0c000010", 8+VRIP_INTR_SIU);
			x = (size_t)device_add(machine, tmpstr);

			if (!machine->x11_md.in_use)
				machine->main_console_handle = x;
		}

		hpc_platid_cpu_arch = 1;	/*  MIPS  */
		hpc_platid_cpu_series = 1;	/*  VR  */
		hpc_platid_cpu_model = 1;	/*  VR41XX  */
		hpc_platid_cpu_submodel = 4;	/*  VR4181  */
		hpc_platid_vendor = 15;		/*  Agenda  */
		hpc_platid_series = 1;		/*  VR  */
		hpc_platid_model = 1;		/*  VR3  */
		hpc_platid_submodel = 0;	/*  -  */

		dev_ram_init(machine, 0x0f000000, 0x01000000,
		    DEV_RAM_MIRROR | DEV_RAM_MIGHT_POINT_TO_DEVICES, 0x0);
		break;

	case MACHINE_HPCMIPS_IBM_WORKPAD_Z50:
		/*  131 MHz VR4121  */
		machine->machine_name = strdup("IBM Workpad Z50");
		/*  TODO:  */
		hpc_fb_addr = 0xa000000;
		hpc_fb_xsize = 640;
		hpc_fb_ysize = 480;
		hpc_fb_xsize_mem = 640;
		hpc_fb_ysize_mem = 480;
		hpc_fb_bits = 16;
		hpc_fb_encoding = BIFB_D16_0000;

		dev_vr41xx_init(machine, machine->memory, 4121);

		hpc_platid_cpu_arch = 1;	/*  MIPS  */
		hpc_platid_cpu_series = 1;	/*  VR  */
		hpc_platid_cpu_model = 1;	/*  VR41XX  */
		hpc_platid_cpu_submodel = 3;	/*  VR4121  */
		hpc_platid_vendor = 9;		/*  IBM  */
		hpc_platid_series = 1;		/*  WorkPad  */
		hpc_platid_model = 1;		/*  Z50  */
		hpc_platid_submodel = 0;	/*  0  */
		break;

	default:printf("Unimplemented hpcmips machine number.\n");
		exit(1);
	}

	store_32bit_word_in_host(cpu, (unsigned char *)&hpc_bootinfo.platid_cpu,
	      (hpc_platid_cpu_arch << 26) + (hpc_platid_cpu_series << 20)
	    + (hpc_platid_cpu_model << 14) + (hpc_platid_cpu_submodel <<  8)
	    + hpc_platid_flags);
	store_32bit_word_in_host(cpu, (unsigned char *)&hpc_bootinfo.
	    platid_machine, (hpc_platid_vendor << 22) + (hpc_platid_series<<16)
	    + (hpc_platid_model <<  8) + hpc_platid_submodel);

	if (hpc_fb_addr != 0) {
		dev_fb_init(machine, machine->memory, hpc_fb_addr, VFB_HPC,
		    hpc_fb_xsize, hpc_fb_ysize,
		    hpc_fb_xsize_mem, hpc_fb_ysize_mem,
		    hpc_fb_bits, machine->machine_name);

		/*  NetBSD/hpcmips uses framebuffer at physical
		    address 0x8.......:  */
		dev_ram_init(machine, 0x80000000, 0x20000000,
		    DEV_RAM_MIRROR | DEV_RAM_MIGHT_POINT_TO_DEVICES, 0x0);
	}

	if (!machine->prom_emulation)
		return;


	/*  NetBSD/hpcmips and possibly others expects the following:  */

	cpu->cd.mips.gpr[MIPS_GPR_A0] = 1;	/*  argc  */
	cpu->cd.mips.gpr[MIPS_GPR_A1] = machine->physical_ram_in_mb * 1048576
	    + 0xffffffff80000000ULL - 512;	/*  argv  */
	cpu->cd.mips.gpr[MIPS_GPR_A2] = machine->physical_ram_in_mb * 1048576
	    + 0xffffffff80000000ULL - 256;	/*  ptr to hpc_bootinfo  */

	machine->bootstr = machine->boot_kernel_filename;
	store_32bit_word(cpu, 0x80000000ULL + (machine->physical_ram_in_mb <<
	    20) - 512, 0x80000000ULL + (machine->physical_ram_in_mb << 20)
	    - 512 + 16);
	store_32bit_word(cpu, 0x80000000ULL + (machine->physical_ram_in_mb <<
	    20) - 512 + 4, 0);
	store_string(cpu, 0x80000000ULL + (machine->physical_ram_in_mb <<
	    20) - 512 + 16, machine->bootstr);

	/*  Special case for the Agenda VR3:  */
	if (machine->machine_subtype == MACHINE_HPCMIPS_AGENDA_VR3) {
		const int tmplen = 1000;
		char *tmp = (char *) malloc(tmplen);

		cpu->cd.mips.gpr[MIPS_GPR_A0] = 2;	/*  argc  */

		store_32bit_word(cpu, 0x80000000 + machine->physical_ram_in_mb
		    * 1048576-512 + 4, 0x80000000 + machine->physical_ram_in_mb
		    * 1048576 - 512 + 64);
		store_32bit_word(cpu, 0x80000000 + machine->physical_ram_in_mb
		    * 1048576 - 512 + 8, 0);

		snprintf(tmp, tmplen, "root=/dev/rom video=vr4181fb:xres:160,y"
		    "res:240,bpp:4,gray,hpck:3084,inv ether=0,0x03fe0300,eth0");
		tmp[tmplen-1] = '\0';

		if (!machine->x11_md.in_use)
			snprintf(tmp+strlen(tmp), tmplen-strlen(tmp),
			    " console=ttyS0,115200");
		tmp[tmplen-1] = '\0';

		if (machine->boot_string_argument[0])
			snprintf(tmp+strlen(tmp), tmplen-strlen(tmp), " %s",
			    machine->boot_string_argument);
		tmp[tmplen-1] = '\0';

		store_string(cpu, 0x80000000 + machine->physical_ram_in_mb
		    * 1048576 - 512 + 64, tmp);

		machine->bootarg = tmp;
	} else if (machine->boot_string_argument[0]) {
		cpu->cd.mips.gpr[MIPS_GPR_A0] ++;	/*  argc  */

		store_32bit_word(cpu, 0x80000000 + machine->physical_ram_in_mb
		    * 1048576-512 + 4, 0x80000000 + machine->physical_ram_in_mb
		    * 1048576 - 512 + 64);
		store_32bit_word(cpu, 0x80000000 + machine->physical_ram_in_mb
		    * 1048576 - 512 + 8, 0);

		store_string(cpu, 0x80000000 + machine->physical_ram_in_mb *
		    1048576 - 512 + 64, machine->boot_string_argument);

		machine->bootarg = machine->boot_string_argument;
	}

	store_16bit_word_in_host(cpu, (unsigned char *)&hpc_bootinfo.length,
	    sizeof(hpc_bootinfo));
	store_32bit_word_in_host(cpu, (unsigned char *)&hpc_bootinfo.magic,
	    HPC_BOOTINFO_MAGIC);
	store_32bit_word_in_host(cpu, (unsigned char *)&hpc_bootinfo.fb_addr,
	    0x80000000 + hpc_fb_addr);
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

	/*  printf("hpc_bootinfo.platid_cpu     = 0x%08x\n",
	    hpc_bootinfo.platid_cpu);
	    printf("hpc_bootinfo.platid_machine = 0x%08x\n",
	    hpc_bootinfo.platid_machine);  */
	store_32bit_word_in_host(cpu,(unsigned char *)&hpc_bootinfo.timezone,0);
	store_buf(cpu, 0x80000000 + machine->physical_ram_in_mb *
	    1048576 - 256, (char *)&hpc_bootinfo, sizeof(hpc_bootinfo));
}


MACHINE_DEFAULT_CPU(hpcmips)
{
	switch (machine->machine_subtype) {
	case MACHINE_HPCMIPS_CASIO_BE300:
		machine->cpu_name = strdup("VR4131");
		break;
	case MACHINE_HPCMIPS_CASIO_E105:
		machine->cpu_name = strdup("VR4121");
		break;
	case MACHINE_HPCMIPS_NEC_MOBILEPRO_770:
	case MACHINE_HPCMIPS_NEC_MOBILEPRO_780:
	case MACHINE_HPCMIPS_NEC_MOBILEPRO_800:
	case MACHINE_HPCMIPS_NEC_MOBILEPRO_880:
		machine->cpu_name = strdup("VR4121");
		break;
	case MACHINE_HPCMIPS_AGENDA_VR3:
		machine->cpu_name = strdup("VR4181");
		break;
	case MACHINE_HPCMIPS_IBM_WORKPAD_Z50:
		machine->cpu_name = strdup("VR4121");
		break;
	default:printf("Unimplemented HPCMIPS model?\n");
		exit(1);
	}
}


MACHINE_DEFAULT_RAM(hpcmips)
{
	/*  Most have 32 MB by default.  */
	machine->physical_ram_in_mb = 32;

	switch (machine->machine_subtype) {
	case MACHINE_HPCMIPS_CASIO_BE300:
		machine->physical_ram_in_mb = 16;
		break;
	case MACHINE_HPCMIPS_CASIO_E105:
		machine->physical_ram_in_mb = 32;
		break;
	case MACHINE_HPCMIPS_AGENDA_VR3:
		machine->physical_ram_in_mb = 16;
		break;
	}
}


MACHINE_REGISTER(hpcmips)
{
	MR_DEFAULT(hpcmips, "Handhelp MIPS (HPCmips)",
	    ARCH_MIPS, MACHINE_HPCMIPS);

	machine_entry_add_alias(me, "hpcmips");

	machine_entry_add_subtype(me, "Casio Cassiopeia BE-300",
	    MACHINE_HPCMIPS_CASIO_BE300, "be-300", "be300", NULL);

	machine_entry_add_subtype(me, "Casio Cassiopeia E-105",
	    MACHINE_HPCMIPS_CASIO_E105, "e-105", "e105", NULL);

	machine_entry_add_subtype(me, "Agenda VR3", MACHINE_HPCMIPS_AGENDA_VR3,
	    "agenda", "vr3", NULL);

	machine_entry_add_subtype(me, "IBM WorkPad Z50",
	    MACHINE_HPCMIPS_IBM_WORKPAD_Z50, "workpad", "z50", NULL);

	machine_entry_add_subtype(me, "NEC MobilePro 770",
	    MACHINE_HPCMIPS_NEC_MOBILEPRO_770, "mobilepro770", NULL);

	machine_entry_add_subtype(me, "NEC MobilePro 780",
	    MACHINE_HPCMIPS_NEC_MOBILEPRO_780, "mobilepro780", NULL);

	machine_entry_add_subtype(me, "NEC MobilePro 800",
	    MACHINE_HPCMIPS_NEC_MOBILEPRO_800, "mobilepro800", NULL);

	machine_entry_add_subtype(me, "NEC MobilePro 880",
	    MACHINE_HPCMIPS_NEC_MOBILEPRO_880, "mobilepro880", NULL);

	me->set_default_ram = machine_default_ram_hpcmips;
}

