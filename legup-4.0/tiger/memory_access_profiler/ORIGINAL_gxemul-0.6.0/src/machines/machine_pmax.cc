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
 *  COMMENT: Digital DECstation ("PMAX") machines
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "devices.h"
#include "diskimage.h"
#include "machine.h"
#include "machine_pmax.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/dec_prom.h"
#include "thirdparty/dec_bootinfo.h"
#include "thirdparty/dec_5100.h"
#include "thirdparty/dec_kn01.h"
#include "thirdparty/dec_kn02.h"  
#include "thirdparty/dec_kn03.h"
#include "thirdparty/dec_kmin.h"
#include "thirdparty/dec_maxine.h"

#define	BOOTARG_BUFLEN		2000


MACHINE_SETUP(pmax)
{
	const char *framebuffer_console_name, *serial_console_name;
	char *init_bootpath;
	int color_fb_flag, i;
	int boot_scsi_boardnumber = 3, boot_net_boardnumber = 3;
	const char *turbochannel_default_gfx_card = "PMAG-BA";
		/*  PMAG-AA, -BA, -CA/DA/EA/FA, -JA, -RO, PMAGB-BA  */
	struct xx {
		struct btinfo_magic a;
		struct btinfo_bootpath b;
		struct btinfo_symtab c;
	} xx;
	char tmpstr[1000];
	struct vfb_data *fb;
	struct memory *mem = machine->memory;
	uint64_t addr;

	cpu->byte_order = EMUL_LITTLE_ENDIAN;

	/*
	 *  Add an R2020 or R3220 writeback memory thing:
	 */
	cpu->cd.mips.coproc[3] = mips_coproc_new(cpu, 3);

	/*  There aren't really any good standard values...  */
	framebuffer_console_name = "osconsole=0,3";
	serial_console_name      = "osconsole=1";

	switch (machine->machine_subtype) {

	case MACHINE_DEC_PMAX_3100:		/*  type  1, KN01  */
		/*  Supposed to have 12MHz or 16.67MHz R2000 CPU, R2010 FPC,
		    R2020 Memory coprocessor  */
		machine->machine_name = strdup("DEC PMAX 3100 (KN01)");

		/*  12 MHz for 2100, 16.67 MHz for 3100  */
		if (machine->emulated_hz == 0)
			machine->emulated_hz = 16670000;

		if (machine->physical_ram_in_mb > 24)
			fprintf(stderr, "WARNING! Real DECstation 3100 machines"
			    " cannot have more than 24MB RAM.\n");

		if ((machine->physical_ram_in_mb % 4) != 0)
			fprintf(stderr, "WARNING! Real DECstation 3100 machines"
			    " have an integer multiple of 4 MBs of RAM.\n");

		/*  1 for color, 0 for mono. TODO: command line option?  */
		color_fb_flag = 1;

		/*
		 *  According to NetBSD/pmax:
		 *
		 *  pm0 at ibus0 addr 0xfc00000: 1024x864x1  (or x8 for color)
		 *  dc0 at ibus0 addr 0x1c000000
		 *  le0 at ibus0 addr 0x18000000: address 00:00:00:00:00:00
		 *  sii0 at ibus0 addr 0x1a000000
		 *  mcclock0 at ibus0 addr 0x1d000000: mc146818 or compatible
		 *  0x1e000000 = system status and control register
		 */
		fb = dev_fb_init(machine, mem, KN01_PHYS_FBUF_START,
		    color_fb_flag? VFB_DEC_VFB02 : VFB_DEC_VFB01,
		    0,0,0,0,0, color_fb_flag? "VFB02":"VFB01");
		dev_colorplanemask_init(mem, KN01_PHYS_COLMASK_START,
		    &fb->color_plane_mask);
		dev_vdac_init(mem, KN01_SYS_VDAC, fb->rgb_palette,
		    color_fb_flag);

		snprintf(tmpstr, sizeof(tmpstr), "%s.cpu[%i].%i",
		    machine->path, machine->bootstrap_cpu, KN01_INT_LANCE);
		dev_le_init(machine, mem, KN01_SYS_LANCE,
		    KN01_SYS_LANCE_B_START, KN01_SYS_LANCE_B_END,
		    tmpstr, 4*1048576);

		snprintf(tmpstr, sizeof(tmpstr), "%s.cpu[%i].%i",
		    machine->path, machine->bootstrap_cpu, KN01_INT_SII);
		dev_sii_init(machine, mem, KN01_SYS_SII, KN01_SYS_SII_B_START,
		    KN01_SYS_SII_B_END, tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%s.cpu[%i].%i",
		    machine->path, machine->bootstrap_cpu, KN01_INT_DZ);
		dev_dc7085_init(machine, mem, KN01_SYS_DZ, tmpstr,
		    machine->x11_md.in_use);

		snprintf(tmpstr, sizeof(tmpstr), "%s.cpu[%i].%i",
		    machine->path, machine->bootstrap_cpu, KN01_INT_CLOCK);
		dev_mc146818_init(machine, mem, KN01_SYS_CLOCK, tmpstr,
		    MC146818_DEC, 1);

		dev_kn01_init(mem, KN01_SYS_CSR, color_fb_flag);

		framebuffer_console_name = "osconsole=0,3";	/*  fb,keyb  */
		serial_console_name      = "osconsole=3";	/*  3  */
		break;

	case MACHINE_DEC_3MAX_5000:		/*  type  2, KN02  */
		/*  Supposed to have 25MHz R3000 CPU, R3010 FPC,  */
		/*  and a R3220 Memory coprocessor  */
		machine->machine_name = strdup("DECstation 5000/200 (3MAX, KN02)");

		if (machine->emulated_hz == 0)
			machine->emulated_hz = 25000000;

		if (machine->physical_ram_in_mb < 8)
			fprintf(stderr, "WARNING! Real KN02 machines do not "
			    "have less than 8MB RAM. Continuing anyway.\n");
		if (machine->physical_ram_in_mb > 480)
			fprintf(stderr, "WARNING! Real KN02 machines cannot "
			    "have more than 480MB RAM. Continuing anyway.\n");

		/*  An R3220 memory thingy:  */
		cpu->cd.mips.coproc[3] = mips_coproc_new(cpu, 3);

		/*
		 *  According to NetBSD/pmax:
		 *  asc0 at tc0 slot 5 offset 0x0
		 *  le0 at tc0 slot 6 offset 0x0
		 *  ibus0 at tc0 slot 7 offset 0x0
		 *  dc0 at ibus0 addr 0x1fe00000
		 *  mcclock0 at ibus0 addr 0x1fe80000: mc146818
		 */

		/*  KN02 mainbus (TurboChannel interrupt controller):  */
		snprintf(tmpstr, sizeof(tmpstr), "kn02 addr=0x%x "
		    "irq=%s.cpu[%i].2", (int) KN02_SYS_CSR,
		    machine->path, machine->bootstrap_cpu);
		device_add(machine, tmpstr);

		/*
		 *  TURBOchannel slots 0, 1, and 2 are free for option cards.
		 *  Let's put in zero or more graphics boards:
		 *
		 *  TODO: It's also possible to have larger graphics cards that
		 *  occupy several slots. How should this be solved nicely?
		 */

		snprintf(tmpstr, sizeof(tmpstr), "%s.cpu[%i].2.kn02.%i",
		    machine->path, machine->bootstrap_cpu, 0);
		dev_turbochannel_init(machine, mem, 0,
		    KN02_PHYS_TC_0_START, KN02_PHYS_TC_0_END,
		    machine->n_gfx_cards >= 1?
			turbochannel_default_gfx_card : "",
		    tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%s.cpu[%i].2.kn02.%i",
		    machine->path, machine->bootstrap_cpu, 1);
		dev_turbochannel_init(machine, mem, 1,
		    KN02_PHYS_TC_1_START, KN02_PHYS_TC_1_END,
		    machine->n_gfx_cards >= 2?
			turbochannel_default_gfx_card : "",
		    tmpstr);

		snprintf(tmpstr, sizeof(tmpstr), "%s.cpu[%i].2.kn02.%i",
		    machine->path, machine->bootstrap_cpu, 2);
		dev_turbochannel_init(machine, mem, 2,
		    KN02_PHYS_TC_2_START, KN02_PHYS_TC_2_END,
		    machine->n_gfx_cards >= 3?
			turbochannel_default_gfx_card : "",
		    tmpstr);

		/*  TURBOchannel slots 3 and 4 are reserved.  */

		/*  TURBOchannel slot 5 is PMAZ-AA ("asc" SCSI).  */
		snprintf(tmpstr, sizeof(tmpstr), "%s.cpu[%i].2.kn02.%i",
		    machine->path, machine->bootstrap_cpu, 5);
		dev_turbochannel_init(machine, mem, 5,
		    KN02_PHYS_TC_5_START, KN02_PHYS_TC_5_END,
		    "PMAZ-AA", tmpstr);

		/*  TURBOchannel slot 6 is PMAD-AA ("le" ethernet).  */
		snprintf(tmpstr, sizeof(tmpstr), "%s.cpu[%i].2.kn02.%i",
		    machine->path, machine->bootstrap_cpu, 6);
		dev_turbochannel_init(machine, mem, 6,
		    KN02_PHYS_TC_6_START, KN02_PHYS_TC_6_END,
		    "PMAD-AA", tmpstr);

		/*  TURBOchannel slot 7 is system stuff.  */
		snprintf(tmpstr, sizeof(tmpstr), "%s.cpu[%i].2.kn02.%i",
		    machine->path, machine->bootstrap_cpu, 7);
		machine->main_console_handle =
		    dev_dc7085_init(machine, mem,
		    KN02_SYS_DZ, tmpstr, machine->x11_md.in_use);

		snprintf(tmpstr, sizeof(tmpstr), "%s.cpu[%i].%i",
		    machine->path, machine->bootstrap_cpu, KN02_INT_CLOCK);
		dev_mc146818_init(machine, mem,
		    KN02_SYS_CLOCK, tmpstr, MC146818_DEC, 1);

		framebuffer_console_name = "osconsole=0,7";
								/*  fb,keyb  */
		serial_console_name      = "osconsole=2";
		boot_scsi_boardnumber = 5;
		boot_net_boardnumber = 6;	/*  TODO: 3?  */
		break;

	case MACHINE_DEC_3MIN_5000:		/*  type 3, KN02BA  */
		machine->machine_name = strdup("DECstation 5000/112 or 145 (3MIN,"
		    " KN02BA)");
		if (machine->emulated_hz == 0)
			machine->emulated_hz = 33000000;
		if (machine->physical_ram_in_mb > 128)
			fprintf(stderr, "WARNING! Real 3MIN machines cannot "
			    "have more than 128MB RAM. Continuing anyway.\n");

		/*  KMIN interrupts:  */
fatal("TODO: Legacy rewrite\n");
abort();
//		machine->md_interrupt = kmin_interrupt;

		/*
		 *  tc0 at mainbus0: 12.5 MHz clock  (0x10000000,slotsize=64MB)
		 *  tc slot 1:   0x14000000
		 *  tc slot 2:   0x18000000
		 *  ioasic0 at tc0 slot 3 offset 0x0	(0x1c000000) slot 0
		 *  asic regs				(0x1c040000) slot 1
		 *  station's ether address		(0x1c080000) slot 2
		 *  le0 at ioasic0 offset 0xc0000: address 00:00:00:00:00:00
		 *					(0x1c0c0000) slot 3
		 *  scc0 at ioasic0 offset 0x100000	(0x1c100000) slot 4
		 *  scc1 at ioasic0 offset 0x180000: console(0x1c180000) slot 6
		 *  mcclock0 at ioasic0 offset 0x200000: mc146818 or
		 *	compatible			(0x1c200000) slot 8
		 *  asc0 at ioasic0 offset 0x300000: NCR53C94, 25MHz,
		 *	SCSI ID 7			(0x1c300000) slot 12
		 *  dma for asc0			(0x1c380000) slot 14
		 */
fatal("TODO: dec_ioasic legacy rewrite\n");
abort();
//		machine->md_int.dec_ioasic_data = dev_dec_ioasic_init(cpu,
//		    mem, 0x1c000000, 0);
fatal("TODO: kmin dev_le_init.\n");
abort();
//		dev_le_init(machine, mem, 0x1c0c0000, 0, 0,
//		    KMIN_INTR_LANCE + 8, 4 * 65536);
		dev_scc_init(machine, mem, 0x1c100000, KMIN_INTR_SCC_0 + 8,
		    machine->x11_md.in_use, 0, 1);
		dev_scc_init(machine, mem, 0x1c180000, KMIN_INTR_SCC_1 + 8,
		    machine->x11_md.in_use, 1, 1);
fatal("TODO: mc146818 irq\n");
abort();
//		dev_mc146818_init(machine, mem, 0x1c200000, 
//KMIN_INTR_CLOCK + 8,
//		    MC146818_DEC, 1);
fatal("TODO: kmin asc init\n");
abort();
//		dev_asc_init(machine, mem, 0x1c300000, KMIN_INTR_SCSI +8,
//		    NULL, DEV_ASC_DEC, NULL, NULL);

		/*
		 *  TURBOchannel slots 0, 1, and 2 are free for
		 *  option cards.  These are by default filled with
		 *  zero or more graphics boards.
		 *
		 *  TODO: irqs 
		 */
fatal("TODO: turbochannel init rewrite!\n");
abort();
#if 0
		dev_turbochannel_init(machine, mem, 0, 0x10000000, 0x103fffff,
		    machine->n_gfx_cards >= 1?
			turbochannel_default_gfx_card : "", KMIN_INT_TC0);

		dev_turbochannel_init(machine, mem, 1, 0x14000000, 0x143fffff,
		    machine->n_gfx_cards >= 2?
		    turbochannel_default_gfx_card : "", KMIN_INT_TC1);

		dev_turbochannel_init(machine, mem, 2, 0x18000000, 0x183fffff,
		    machine->n_gfx_cards >= 3?
			turbochannel_default_gfx_card : "", KMIN_INT_TC2);
#endif
		/*  (kmin shared irq numbers (IP) are offset by +8 in the
		    emulator)  */
		/*  kmin_csr = dev_kmin_init(cpu, mem, KMIN_REG_INTR);  */

		framebuffer_console_name = "osconsole=0,3";	/* fb,keyb(?) */
		serial_console_name      = "osconsole=3";	/* ? */
		break;

	case MACHINE_DEC_3MAXPLUS_5000:	/*  type 4, KN03  */
		machine->machine_name = strdup("DECsystem 5900 or 5000 (3MAX+) (KN03)");

		/*  5000/240 (KN03-GA, R3000): 40 MHz  */
		/*  5000/260 (KN05-NB, R4000): 60 MHz  */
		/*  TODO: are both these type 4?  */
		if (machine->emulated_hz == 0)
			machine->emulated_hz = 40000000;
		if (machine->physical_ram_in_mb > 480)
			fprintf(stderr, "WARNING! Real KN03 machines cannot "
			    "have more than 480MB RAM. Continuing anyway.\n");

		/*  KN03 interrupts:  */
fatal("TODO: Legacy rewrite\n");
abort();
//		machine->md_interrupt = kn03_interrupt;

		/*
		 *  tc0 at mainbus0: 25 MHz clock (slot 0)	(0x1e000000)
		 *  tc0 slot 1					(0x1e800000)
		 *  tc0 slot 2					(0x1f000000)
		 *  ioasic0 at tc0 slot 3 offset 0x0		(0x1f800000)
		 *    something that has to do with interrupts? (?)(0x1f840000?)
		 *  le0 at ioasic0 offset 0xc0000		(0x1f8c0000)
		 *  scc0 at ioasic0 offset 0x100000		(0x1f900000)
		 *  scc1 at ioasic0 offset 0x180000: console	(0x1f980000)
		 *  mcclock0 at ioasic0 offset 0x200000: mc146818 or
		 *	compatible				(0x1fa00000)
		 *  asc0 at ioasic0 offset 0x300000: NCR53C94, 25MHz,
		 *	SCSI ID 7				(0x1fb00000)
		 */
fatal("TODO: dec_ioasic legacy rewrite\n");
abort();
//		machine->md_int.dec_ioasic_data = dev_dec_ioasic_init(cpu,
//		    mem, 0x1f800000, 0);

fatal("TODO: kn03 dev_le_init rewrite\n");
abort();
//		dev_le_init(machine, mem, KN03_SYS_LANCE, 0, 0,
//		    KN03_INTR_LANCE +8, 4 * 65536);

fatal("TODO: dec_ioasic legacy rewrite\n");
abort();
//		machine->md_int.dec_ioasic_data->dma_func[3] =
//		    dev_scc_dma_func;
//		machine->md_int.dec_ioasic_data->dma_func_extra[2] =
//		    dev_scc_init(machine, mem, KN03_SYS_SCC_0,
//		    KN03_INTR_SCC_0 +8, machine->x11_md.in_use, 0, 1);
//		machine->md_int.dec_ioasic_data->dma_func[2] =
//		    dev_scc_dma_func;
//		machine->md_int.dec_ioasic_data->dma_func_extra[3] =
//		    dev_scc_init(machine, mem, KN03_SYS_SCC_1,
//		    KN03_INTR_SCC_1 +8, machine->x11_md.in_use, 1, 1);

fatal("TODO: mc146818 irq\n");
abort();
//		dev_mc146818_init(machine, mem, KN03_SYS_CLOCK, KN03_INT_RTC,
//		    MC146818_DEC, 1);
fatal("TODO: asc init rewrite\n");
abort();
//		dev_asc_init(machine, mem, KN03_SYS_SCSI,
//		    KN03_INTR_SCSI +8, NULL, DEV_ASC_DEC, NULL, NULL);

		/*
		 *  TURBOchannel slots 0, 1, and 2 are free for
		 *  option cards.  These are by default filled with
		 *  zero or more graphics boards.
		 *
		 *  TODO: irqs 
		 */
fatal("TODO: turbochannel rewrite init\n");
abort();
#if 0
		dev_turbochannel_init(machine, mem, 0,
		    KN03_PHYS_TC_0_START, KN03_PHYS_TC_0_END,
		    machine->n_gfx_cards >= 1?
			turbochannel_default_gfx_card : "",
		    KN03_INTR_TC_0 +8);

		dev_turbochannel_init(machine, mem, 1,
		    KN03_PHYS_TC_1_START, KN03_PHYS_TC_1_END,
		    machine->n_gfx_cards >= 2?
			turbochannel_default_gfx_card : "",
		    KN03_INTR_TC_1 +8);

		dev_turbochannel_init(machine, mem, 2,
		    KN03_PHYS_TC_2_START, KN03_PHYS_TC_2_END,
		    machine->n_gfx_cards >= 3?
			turbochannel_default_gfx_card : "",
		    KN03_INTR_TC_2 +8);
#endif

		/*  TODO: interrupts  */
		/*  shared (turbochannel) interrupts are +8  */

		framebuffer_console_name = "osconsole=0,3";	/* fb,keyb(?) */
		serial_console_name      = "osconsole=3";	/* ? */
		break;

	case MACHINE_DEC_5800:		/*  type 5, KN5800  */
		machine->machine_name = strdup("DECsystem 5800");

		/*  TODO: this is incorrect, banks multiply by 8 etc  */
		if (machine->physical_ram_in_mb < 48)
			fprintf(stderr, "WARNING! 5800 will probably not run "
			    "with less than 48MB RAM. Continuing anyway.\n");

		/*
		 *  According to
		 *  http://www2.no.netbsd.org/ports/pmax/models.html,
		 *  the 5800-series is based on VAX 6000/300.
		 */

		/*
		 *  Ultrix might support SMP on this machine type.
		 *
		 *  Something at 0x10000000.
		 *  ssc serial console at 0x10140000, interrupt 2 (shared
		 *  with XMI?).
		 *  xmi 0 at address 0x11800000   (node x at offset x*0x80000)
		 *  Clock uses interrupt 3 (shared with XMI?).
		 */

		device_add(machine, "dec5800 addr=0x10000000");
		device_add(machine, "decbi addr=0x10000000");

		snprintf(tmpstr, sizeof(tmpstr), "%s.cpu[%i].dec5800.28",
		    machine->path, machine->bootstrap_cpu);
		dev_ssc_init(machine, mem, 0x10140000,
		    tmpstr, machine->x11_md.in_use);

		dev_decxmi_init(mem, 0x11800000);
		dev_deccca_init(mem, DEC_DECCCA_BASEADDR);

		break;

	case MACHINE_DEC_5400:		/*  type 6, KN210  */
		machine->machine_name = strdup("DECsystem 5400 (KN210)");
		/*
		 *  Misc. info from the KN210 manual:
		 *
		 *  Interrupt lines:
		 *	irq5	fpu
		 *	irq4	halt
		 *	irq3	pwrfl -> mer1 -> mer0 -> wear
		 *	irq2	100 Hz -> birq7
		 *	irq1	dssi -> ni -> birq6
		 *	irq0	birq5 -> console -> timers -> birq4
		 *
		 *  Interrupt status register at 0x10048000.
		 *  Main memory error status register at 0x1008140.
		 *  Interval Timer Register (ITR) at 0x10084010.
		 *  Q22 stuff at 0x10088000 - 0x1008ffff.
		 *  TODR at 0x1014006c.
		 *  TCR0 (timer control register 0) 0x10140100.
		 *  TIR0 (timer interval register 0) 0x10140104.
		 *  TCR1 (timer control register 1) 0x10140110.
		 *  TIR1 (timer interval register 1) 0x10140114.
		 *  VRR0 (Vector Read Register 0) at 0x16000050.
		 *  VRR1 (Vector Read Register 1) at 0x16000054.
		 *  VRR2 (Vector Read Register 2) at 0x16000058.
		 *  VRR3 (Vector Read Register 3) at 0x1600005c.
		 */
		/*  ln (ethernet) at 0x10084x00 ? and 0x10120000 ?  */
		/*  error registers (?) at 0x17000000 and 0x10080000  */
		/*  device_add(machine, "kn210 addr=0x10080000");  */
		dev_ssc_init(machine, mem, 0x10140000, "irq? TODO",
		    machine->x11_md.in_use);
		break;

	case MACHINE_DEC_MAXINE_5000:	/*  type 7, KN02CA  */
		machine->machine_name = strdup("Personal DECstation 5000/xxx "
		    "(MAXINE) (KN02CA)");
		if (machine->emulated_hz == 0)
			machine->emulated_hz = 33000000;

		if (machine->physical_ram_in_mb < 8)
			fprintf(stderr, "WARNING! Real KN02CA machines do not "
			    "have less than 8MB RAM. Continuing anyway.\n");
		if (machine->physical_ram_in_mb > 40)
			fprintf(stderr, "WARNING! Real KN02CA machines cannot "
			    "have more than 40MB RAM. Continuing anyway.\n");

		/*  Maxine interrupts:  */
fatal("TODO: Legacy rewrite\n");
abort();

//		machine->md_interrupt = maxine_interrupt;

		/*
		 *  Something at address 0xca00000. (?)
		 *  Something at address 0xe000000. (?)
		 *  tc0 slot 0					(0x10000000)
		 *  tc0 slot 1					(0x14000000)
		 *  (tc0 slot 2 used by the framebuffer)
		 *  ioasic0 at tc0 slot 3 offset 0x0		(0x1c000000)
		 *  le0 at ioasic0 offset 0xc0000: address 00:00:00:00:00:00
		 *						(0x1c0c0000)
		 *  scc0 at ioasic0 offset 0x100000: console  <-- serial
		 *						(0x1c100000)
		 *  mcclock0 at ioasic0 offset 0x200000: mc146818 (0x1c200000)
		 *  isdn at ioasic0 offset 0x240000 not configured (0x1c240000)
		 *  bba0 at ioasic0 offset 0x240000 (audio0 at bba0)
		 *	^--- which one of isdn and bba0?
		 *  dtop0 at ioasic0 offset 0x280000		(0x1c280000)
		 *  fdc at ioasic0 offset 0x2c0000 not configured
		 *	^-- floppy				(0x1c2c0000)
		 *  asc0 at ioasic0 offset 0x300000: NCR53C94, 25MHz, SCSI
		 *	ID 7					(0x1c300000)
		 *  xcfb0 at tc0 slot 2 offset 0x0: 1024x768x8
		 *	built-in framebuffer			(0xa000000)
		 */
fatal("TODO: dec_ioasic legacy rewrite\n");
abort();
//		machine->md_int.dec_ioasic_data =
//		    dev_dec_ioasic_init(cpu, mem, 0x1c000000, 0);

fatal("TODO: turbochannel rewrite!\n");
abort();
#if 0
		/*  TURBOchannel slots (0 and 1):  */
		dev_turbochannel_init(machine, mem, 0,
		    0x10000000, 0x103fffff,
		    machine->n_gfx_cards >= 2?
			turbochannel_default_gfx_card : "",
		    XINE_INTR_TC_0 +8);
		dev_turbochannel_init(machine, mem, 1,
		    0x14000000, 0x143fffff,
		    machine->n_gfx_cards >= 3?
			turbochannel_default_gfx_card : "",
		    XINE_INTR_TC_1 +8);

		/*
		 *  TURBOchannel slot 2 is hardwired to be used by
		 *  the framebuffer: (NOTE: 0x8000000, not 0x18000000)
		 */
		dev_turbochannel_init(machine, mem, 2,
		    0x8000000, 0xbffffff, "PMAG-DV", 0);
#endif
		/*
		 *  TURBOchannel slot 3: fixed, ioasic
		 *  (the system stuff), 0x1c000000
		 */
fatal("TODO: xine dev_le_init rewrite\n");
abort();
//		dev_le_init(machine, mem, 0x1c0c0000, 0, 0,
//		    XINE_INTR_LANCE +8, 4*65536);
		dev_scc_init(machine, mem, 0x1c100000,
		    XINE_INTR_SCC_0 +8, machine->x11_md.in_use, 0, 1);
fatal("TODO: mc146818 irq\n");
abort();
//		dev_mc146818_init(machine, mem, 0x1c200000,
//		    XINE_INT_TOY, MC146818_DEC, 1);
fatal("TODO: xine asc init rewrite\n");
abort();
//		dev_asc_init(machine, mem, 0x1c300000,
//		    XINE_INTR_SCSI +8, NULL, DEV_ASC_DEC, NULL, NULL);

		framebuffer_console_name = "osconsole=3,2";	/*  keyb,fb?  */
		serial_console_name      = "osconsole=3";
		break;

	case MACHINE_DEC_5500:	/*  type 11, KN220  */
		machine->machine_name = strdup("DECsystem 5500 (KN220)");

		/*
		 *  According to NetBSD's pmax ports page:
		 *  KN220-AA is a "30 MHz R3000 CPU with R3010 FPU"
		 *  with "512 kBytes of Prestoserve battery backed RAM."
		 */
		if (machine->emulated_hz == 0)
			machine->emulated_hz = 30000000;

		/*
		 *  See KN220 docs for more info.
		 *
		 *  something at 0x10000000
		 *  something at 0x10001000
		 *  something at 0x10040000
		 *  scc at 0x10140000
		 *  qbus at (or around) 0x10080000
		 *  dssi (disk controller) buffers at 0x10100000,
		 *	registers at 0x10160000.
		 *  sgec (ethernet) registers at 0x10008000, station
		 *	addresss at 0x10120000.
		 *  asc (scsi) at 0x17100000.
		 */

		dev_ssc_init(machine, mem, 0x10140000, "TODO: irq",
		    machine->x11_md.in_use);

		/*  something at 0x17000000, ultrix says "cpu 0 panic: "
		    "DS5500 I/O Board is missing" if this is not here  */
		dev_dec5500_ioboard_init(cpu, mem, 0x17000000);

		dev_sgec_init(mem, 0x10008000, 0);		/*  irq?  */

		/*  The asc controller might be TURBOchannel-ish?  */
#if 0
		dev_turbochannel_init(machine, mem, 0, 0x17100000,
		    0x171fffff, "PMAZ-AA", 0);	/*  irq?  */
#else
		dev_asc_init(machine, mem, 0x17100000, 0, NULL,
		    DEV_ASC_DEC, NULL, NULL);		/*  irq?  */
#endif

		framebuffer_console_name = "osconsole=0,0";	/*  TODO (?)  */
		serial_console_name      = "osconsole=0";
		break;

	case MACHINE_DEC_MIPSMATE_5100:	/*  type 12  */
		machine->machine_name = strdup("DEC MIPSMATE 5100 (KN230)");

		if (machine->emulated_hz == 0)
			machine->emulated_hz = 20000000;

		if (machine->physical_ram_in_mb > 128)
			fprintf(stderr, "WARNING! Real MIPSMATE 5100 machines "
			    "cannot have more than 128MB RAM. Continuing"
			    " anyway.\n");

		if (machine->x11_md.in_use)
			fprintf(stderr, "WARNING! Real MIPSMATE 5100 machines "
			    "cannot have a graphical framebuffer. "
			    "Continuing anyway.\n");

		/*  KN230 mainbus / interrupt controller:  */
		snprintf(tmpstr, sizeof(tmpstr),
		    "kn230 addr=0x%"PRIx64, (uint64_t) KN230_SYS_ICSR);
		device_add(machine, tmpstr);

		/*
		 *  According to NetBSD/pmax:
		 *  dc0 at ibus0 addr 0x1c000000
		 *  le0 at ibus0 addr 0x18000000: address 00:00:00:00:00:00
		 *  sii0 at ibus0 addr 0x1a000000
		 */

		snprintf(tmpstr, sizeof(tmpstr), "%s.cpu[%i].%i",
		    machine->path, machine->bootstrap_cpu, 4);
		dev_mc146818_init(machine, mem, KN230_SYS_CLOCK, tmpstr,
		    MC146818_DEC, 1);

		snprintf(tmpstr, sizeof(tmpstr), "%s.cpu[%i].kn230.0x%x",
		    machine->path, machine->bootstrap_cpu, KN230_CSR_INTR_DZ0);
		dev_dc7085_init(machine, mem, KN230_SYS_DZ0,
		    tmpstr, machine->x11_md.in_use);

		/* dev_dc7085_init(machine, mem, KN230_SYS_DZ1,
		    KN230_CSR_INTR_OPT0, machine->x11_md.in_use);  */
		/* dev_dc7085_init(machine, mem, KN230_SYS_DZ2,
		    KN230_CSR_INTR_OPT1, machine->x11_md.in_use);  */

		snprintf(tmpstr, sizeof(tmpstr), "%s.cpu[%i].kn230.0x%x",
		    machine->path, machine->bootstrap_cpu,
		    KN230_CSR_INTR_LANCE);
		dev_le_init(machine, mem, KN230_SYS_LANCE,
		    KN230_SYS_LANCE_B_START, KN230_SYS_LANCE_B_END,
		    tmpstr, 4*1048576);

		snprintf(tmpstr, sizeof(tmpstr), "%s.cpu[%i].kn230.0x%x",
		    machine->path, machine->bootstrap_cpu, KN230_CSR_INTR_SII);
		dev_sii_init(machine, mem, KN230_SYS_SII,
		    KN230_SYS_SII_B_START, KN230_SYS_SII_B_END, tmpstr);

		serial_console_name = "osconsole=0";
		break;

	default:fatal("Unknown DEC machine type: %i\n",
		    machine->machine_subtype);
		exit(1);
	}

	/*
	 *  Most OSes on DECstation use physical addresses below
	 *  0x20000000, but both OSF/1 and Sprite use 0xbe...... as if
	 *  it was 0x1e......, so we need this hack:
	 */
	dev_ram_init(machine, 0xa0000000, 0x20000000,
	    DEV_RAM_MIRROR | DEV_RAM_MIGHT_POINT_TO_DEVICES, 0x0);

	if (!machine->prom_emulation)
		return;


	/*  DECstation PROM stuff:  (TODO: endianness)  */
	for (i=0; i<150; i++)
		store_32bit_word(cpu, DEC_PROM_CALLBACK_STRUCT + i*4,
		    DEC_PROM_EMULATION + i*8);

	/*  Fill PROM with special "magic trap" instructions:  */
	for (i=0; i<150; i++) {
		store_32bit_word(cpu, DEC_PROM_EMULATION + i*8,
		    0x00c0de0c);	/*  trap instruction  */
		store_32bit_word(cpu, DEC_PROM_EMULATION + i*8 + 4,
		    0x00000000);	/*  nop  */
	}

	/*  Jumptable at beginning of PROM:  also "magic trap" instructions:  */
	for (i=0; i<0x180; i+=8) {
		store_32bit_word(cpu, 0xbfc00000 + i,
		    0x00c0de0c);	/*  trap instruction  */
		store_32bit_word(cpu, 0xbfc00000 + i + 4,
		    0x00000000);	/*  nop  */
	}


	/*
	 *  According to dec_prom.h from NetBSD:
	 *
	 *  "Programs loaded by the new PROMs pass the following arguments:
	 *	a0	argc
	 *	a1	argv
	 *	a2	DEC_PROM_MAGIC
	 *	a3	The callback vector defined below"
	 *
	 *  So we try to emulate a PROM, even though no such thing has been
	 *  loaded.
	 */

	cpu->cd.mips.gpr[MIPS_GPR_A0] = 3;
	cpu->cd.mips.gpr[MIPS_GPR_A1] = DEC_PROM_INITIAL_ARGV;
	cpu->cd.mips.gpr[MIPS_GPR_A2] = DEC_PROM_MAGIC;
	cpu->cd.mips.gpr[MIPS_GPR_A3] = DEC_PROM_CALLBACK_STRUCT;

	store_32bit_word(cpu, INITIAL_STACK_POINTER + 0x10, BOOTINFO_MAGIC);
	store_32bit_word(cpu, INITIAL_STACK_POINTER + 0x14, BOOTINFO_ADDR);

	store_32bit_word(cpu, DEC_PROM_INITIAL_ARGV,
	    (DEC_PROM_INITIAL_ARGV + 0x10));
	store_32bit_word(cpu, DEC_PROM_INITIAL_ARGV+4,
	    (DEC_PROM_INITIAL_ARGV + 0x70));
	store_32bit_word(cpu, DEC_PROM_INITIAL_ARGV+8,
	    (DEC_PROM_INITIAL_ARGV + 0xe0));
	store_32bit_word(cpu, DEC_PROM_INITIAL_ARGV+12, 0);

	/*
	 *  NetBSD and Ultrix expect the boot args to be like this:
	 *
	 *	"boot" "bootdev" [args?]
	 *
	 *  where bootdev is supposed to be "rz(0,0,0)netbsd" for
	 *  3100/2100 (although that crashes Ultrix :-/), and
	 *  "5/rz0a/netbsd" for all others.  The number '5' is the
	 *  slot number of the boot device.
	 *
	 *  'rz' for disks, 'tz' for tapes.
	 *
	 *  TODO:  Make this nicer.
	 */
	{
		char bootpath[200];
#if 0
		if (machine->machine_subtype == MACHINE_DEC_PMAX_3100)
			strlcpy(bootpath, "rz(0,0,0)", sizeof(bootpath));
		else
#endif
			strlcpy(bootpath, "5/rz1/", sizeof(bootpath));

		if (machine->bootdev_id < 0 || machine->force_netboot) {
			/*  tftp boot:  */
			strlcpy(bootpath, "5/tftp/", sizeof(bootpath));
			bootpath[0] = '0' + boot_net_boardnumber;
		} else {
			/*  disk boot:  */
			bootpath[0] = '0' + boot_scsi_boardnumber;
			if (diskimage_is_a_tape(machine, machine->bootdev_id,
			    machine->bootdev_type))
				bootpath[2] = 't';
			bootpath[4] = '0' + machine->bootdev_id;
		}

		init_bootpath = bootpath;
	}

	CHECK_ALLOCATION(machine->bootarg = (char *) malloc(BOOTARG_BUFLEN));
	strlcpy(machine->bootarg, init_bootpath, BOOTARG_BUFLEN);
	if (strlcat(machine->bootarg, machine->boot_kernel_filename,
	    BOOTARG_BUFLEN) > BOOTARG_BUFLEN) {
		fprintf(stderr, "bootarg truncated?\n");
		exit(1);
	}

	machine->bootstr = strdup("boot");

	store_string(cpu, DEC_PROM_INITIAL_ARGV+0x10, machine->bootstr);
	store_string(cpu, DEC_PROM_INITIAL_ARGV+0x70, machine->bootarg);
	store_string(cpu, DEC_PROM_INITIAL_ARGV+0xe0,
	    machine->boot_string_argument);

	/*  Decrease the nr of args, if there are no args :-)  */
	if (machine->boot_string_argument == NULL ||
	    machine->boot_string_argument[0] == '\0')
		cpu->cd.mips.gpr[MIPS_GPR_A0] --;

	if (machine->boot_string_argument[0] != '\0') {
		strlcat(machine->bootarg, " ", BOOTARG_BUFLEN);
		if (strlcat(machine->bootarg, machine->boot_string_argument,
		    BOOTARG_BUFLEN) >= BOOTARG_BUFLEN) {
			fprintf(stderr, "bootstr truncated?\n");
			exit(1);
		}
	}

	xx.a.common.next = (char *)&xx.b - (char *)&xx;
	xx.a.common.type = BTINFO_MAGIC;
	xx.a.magic = BOOTINFO_MAGIC;

	xx.b.common.next = (char *)&xx.c - (char *)&xx.b;
	xx.b.common.type = BTINFO_BOOTPATH;
	strlcpy(xx.b.bootpath, machine->bootstr, sizeof(xx.b.bootpath));

	xx.c.common.next = 0;
	xx.c.common.type = BTINFO_SYMTAB;
	xx.c.nsym = 0;
	xx.c.ssym = 0;
	xx.c.esym = machine->file_loaded_end_addr;

	store_buf(cpu, BOOTINFO_ADDR, (char *)&xx, sizeof(xx));

	CHECK_ALLOCATION(machine->md.pmax = (struct machine_pmax *)
	    malloc(sizeof(struct machine_pmax)));
	memset(machine->md.pmax, 0, sizeof(struct machine_pmax));

	/*  The system's memmap:  */
	CHECK_ALLOCATION(machine->md.pmax->memmap = (struct dec_memmap *)
	    malloc(sizeof(struct dec_memmap)));
	store_32bit_word_in_host(cpu,
	    (unsigned char *)&machine->md.pmax->memmap->pagesize, 4096);
	{
		unsigned int i;
		for (i=0; i<sizeof(machine->md.pmax->memmap->bitmap); i++)
			machine->md.pmax->memmap->bitmap[i] = ((int)i * 4096*8 <
			    1048576*machine->physical_ram_in_mb)? 0xff : 0x00;
	}
	store_buf(cpu, DEC_MEMMAP_ADDR,
	    (char *)machine->md.pmax->memmap, sizeof(struct dec_memmap));

	/*  Environment variables:  */
	addr = DEC_PROM_STRINGS;

	if (machine->x11_md.in_use && machine->n_gfx_cards > 0)
		/*  (0,3)  Keyboard and Framebuffer  */
		add_environment_string(cpu, framebuffer_console_name, &addr);
	else
		/*  Serial console  */
		add_environment_string(cpu, serial_console_name, &addr);

	/*
	 *  The KN5800 (SMP system) uses a CCA (console communications
	 *  area):  (See VAX 6000 documentation for details.)
	 */
	{
		char tmps[300];
		snprintf(tmps, sizeof(tmps), "cca=%"PRIx32,
		    (uint32_t) (DEC_DECCCA_BASEADDR + 0xa0000000ULL));
		add_environment_string(cpu, tmps, &addr);
	}

	/*  These are needed for Sprite to boot:  */
	{
		char tmps[500];

		snprintf(tmps, sizeof(tmps), "boot=%s", machine->bootarg);
		tmps[sizeof(tmps)-1] = '\0';
		add_environment_string(cpu, tmps, &addr);

		snprintf(tmps, sizeof(tmps), "bitmap=0x%"PRIx32, (uint32_t)
		    ( (DEC_MEMMAP_ADDR + sizeof(uint32_t) /* skip the
			page size and point to the memmap */
		    ) & 0xffffffffULL) );
		tmps[sizeof(tmps)-1] = '\0';
		add_environment_string(cpu, tmps, &addr);

		snprintf(tmps, sizeof(tmps), "bitmaplen=0x%"PRIx32, (uint32_t)
		    ( machine->physical_ram_in_mb * 1048576 / 4096 / 8) );
		tmps[sizeof(tmps)-1] = '\0';
		add_environment_string(cpu, tmps, &addr);
	}

	add_environment_string(cpu, "scsiid0=7", &addr);
	add_environment_string(cpu, "bootmode=a", &addr);
	add_environment_string(cpu, "testaction=q", &addr);
	add_environment_string(cpu, "haltaction=h", &addr);
	add_environment_string(cpu, "more=24", &addr);

	/*  Used in at least Ultrix on the 5100:  */
	add_environment_string(cpu, "scsiid=7", &addr);
	add_environment_string(cpu, "baud0=9600", &addr);
	add_environment_string(cpu, "baud1=9600", &addr);
	add_environment_string(cpu, "baud2=9600", &addr);
	add_environment_string(cpu, "baud3=9600", &addr);
	add_environment_string(cpu, "iooption=0x1", &addr);

	/*  The end:  */
	add_environment_string(cpu, "", &addr);
}


MACHINE_DEFAULT_CPU(pmax)
{
	if (machine->machine_subtype > 2)
		CHECK_ALLOCATION(machine->cpu_name = strdup("R3000A"));

	if (machine->machine_subtype > 1 && machine->cpu_name == NULL)
		CHECK_ALLOCATION(machine->cpu_name = strdup("R3000"));

	if (machine->cpu_name == NULL)
		CHECK_ALLOCATION(machine->cpu_name = strdup("R2000"));
}


MACHINE_DEFAULT_RAM(pmax)
{
	switch (machine->machine_subtype) {
	case MACHINE_DEC_PMAX_3100:
		machine->physical_ram_in_mb = 24;
		break;
	case MACHINE_DEC_3MAX_5000:
		machine->physical_ram_in_mb = 64;
		break;
	default:machine->physical_ram_in_mb = 32;
	}
}


MACHINE_REGISTER(pmax)
{
	MR_DEFAULT(pmax, "DECstation/DECsystem", ARCH_MIPS, MACHINE_PMAX);

	machine_entry_add_alias(me, "decstation");
	machine_entry_add_alias(me, "decsystem");
	machine_entry_add_alias(me, "dec");

	machine_entry_add_subtype(me, "DECstation 3100 (PMAX)",
	    MACHINE_DEC_PMAX_3100, "pmax", "3100", "2100", NULL);

	machine_entry_add_subtype(me, "DECstation 5000/200 (3MAX)",
	    MACHINE_DEC_3MAX_5000, "3max", "5000/200", NULL);

	machine_entry_add_subtype(me, "DECstation 5000/1xx (3MIN)",
	    MACHINE_DEC_3MIN_5000, "3min", "5000/1xx", NULL);

	machine_entry_add_subtype(me, "DECstation 5000 (3MAXPLUS)",
	    MACHINE_DEC_3MAXPLUS_5000, "3maxplus", "3max+", NULL);

	machine_entry_add_subtype(me, "DECsystem 58x0",
	    MACHINE_DEC_5800, "5800", "58x0", NULL);

	machine_entry_add_subtype(me, "DECsystem 5400",
	    MACHINE_DEC_5400, "5400", NULL);

	machine_entry_add_subtype(me, "DECstation Maxine (5000)",
	    MACHINE_DEC_MAXINE_5000, "maxine", NULL);

	machine_entry_add_subtype(me, "DECsystem 5500",
	    MACHINE_DEC_5500, "5500", NULL);

	machine_entry_add_subtype(me, "DECstation MipsMate (5100)",
	    MACHINE_DEC_MIPSMATE_5100, "5100", "mipsmate", NULL);

	me->set_default_ram = machine_default_ram_pmax;
}

