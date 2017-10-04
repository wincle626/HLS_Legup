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
 *  COMMENT: Silicon Graphics' MIPS-based machines
 *
 *  http://obsolete.majix.org/computers/sgi/iptable.shtml contains a
 *  pretty detailed list of IP ("Inhouse Processor") model numbers.
 *
 *  See also: http://hardware.majix.org/computers/sgi/iptable.shtml
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arcbios.h"
#include "bus_pci.h"
#include "cpu.h"
#include "device.h"
#include "devices.h"
#include "diskimage.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"
#include "net.h"

#include "thirdparty/sgi_arcbios.h"
#include "thirdparty/crimereg.h"


#define ETHERNET_STRING_MAXLEN  	40
#define	MACHINE_NAME_MAXBUF		100


MACHINE_SETUP(sgi)
{
	uint64_t sgi_ram_offset = 0;
	int arc_wordlen = sizeof(uint32_t);
	struct memory *mem = machine->memory;
	char tmpstr[1000];
	int i, j;
	char *eaddr_string = strdup("eaddr=10:20:30:40:50:60");		/*  bogus  */
	unsigned char macaddr[6];

	struct pci_data *pci_data = NULL;

	CHECK_ALLOCATION(machine->machine_name = (char *) malloc(MACHINE_NAME_MAXBUF));

	cpu->byte_order = EMUL_BIG_ENDIAN;
	snprintf(machine->machine_name, MACHINE_NAME_MAXBUF,
	    "SGI-IP%i", machine->machine_subtype);

	sgi_ram_offset = 1048576 * machine->memory_offset_in_mb;

	/*  Special cases for IP20,22,24,26 memory offset:  */
	if (machine->machine_subtype == 20 || machine->machine_subtype == 22 ||
	    machine->machine_subtype == 24 || machine->machine_subtype == 26) {
		dev_ram_init(machine, 0x00000000, 0x10000, DEV_RAM_MIRROR
		    | DEV_RAM_MIGHT_POINT_TO_DEVICES, sgi_ram_offset);
		dev_ram_init(machine, 0x00050000, sgi_ram_offset-0x50000,
		    DEV_RAM_MIRROR | DEV_RAM_MIGHT_POINT_TO_DEVICES,
		    sgi_ram_offset + 0x50000);
	}

	/*  Special cases for IP28,30 memory offset:  */
	if (machine->machine_subtype == 28 || machine->machine_subtype == 30) {
		/*  TODO: length below should maybe not be 128MB?  */
		dev_ram_init(machine, 0x00000000, 128*1048576, DEV_RAM_MIRROR
		    | DEV_RAM_MIGHT_POINT_TO_DEVICES, sgi_ram_offset);
	}

	net_generate_unique_mac(machine, macaddr);
	CHECK_ALLOCATION(eaddr_string = (char *) malloc(ETHERNET_STRING_MAXLEN));

	switch (machine->machine_subtype) {

	case 10:
		strlcat(machine->machine_name, " (4D/25)", MACHINE_NAME_MAXBUF);
		/*  TODO  */
		break;

	case 12:
		strlcat(machine->machine_name,
		    " (Iris Indigo IP12)", MACHINE_NAME_MAXBUF);

		/*  TODO  */
		/*  33 MHz R3000, according to http://www.irisindigo.com/  */
		/*  "capable of addressing up to 96MB of memory."  */

		break;

	case 19:
		strlcat(machine->machine_name,
		    " (Everest IP19)", MACHINE_NAME_MAXBUF);
		machine->main_console_handle = (size_t)device_add(machine,
		    "z8530 addr=0x1fbd9830 irq=0 addr_mult=4");
		dev_scc_init(machine, mem, 0x10086000, 0,
		    machine->x11_md.in_use, 0, 8);	/*  serial? irix?  */

		device_add(machine, "sgi_ip19 addr=0x18000000");

		/*  Irix' <everest_du_init+0x130> reads this device:  */
		device_add(machine, "random addr=0x10006000 len=16");

		/*  Irix' get_mpconf() looks for this:  (TODO)  */
		store_32bit_word(cpu, 0xa0000000 + 0x3000,
		    0xbaddeed2);

		/*  Memory size, not 4096 byte pages, but 256 bytes?
		    (16 is size of kernel... approx)  */
		store_32bit_word(cpu, 0xa0000000 + 0x26d0, 30000);
		  /* (machine->physical_ram_in_mb - 16) * (1048576 / 256));  */

		break;

	case 20:
		strlcat(machine->machine_name,
		    " (Indigo)", MACHINE_NAME_MAXBUF);

		/*
		 *  Guesses based on NetBSD 2.0 beta, 20040606.
		 *
		 *  int0 at mainbus0 addr 0x1fb801c0: bus 1MHz, CPU 2MHz
		 *  imc0 at mainbus0 addr 0x1fa00000: revision 0
		 *  gio0 at imc0
		 *  unknown GIO card (product 0x00 revision 0x00)
		 *	at gio0 slot 0 addr 0x1f400000 not configured
		 *  unknown GIO card (product 0x00 revision 0x00)
		 *	at gio0 slot 1 addr 0x1f600000 not configured
		 *  unknown GIO card (product 0x00 revision 0x00)
		 *	at gio0 slot 2 addr 0x1f000000 not configured
		 *  hpc0 at gio0 addr 0x1fb80000: SGI HPC1
		 *  zsc0 at hpc0 offset 0xd10   (channels 0 and 1,
		 *				 channel 1 for console)
		 *  zsc1 at hpc0 offset 0xd00   (2 channels)
		 *  sq0 at hpc0 offset 0x100: SGI Seeq 80c03
		 *  wdsc0 at hpc0 offset 0x11f
		 *  dpclock0 at hpc0 offset 0xe00
		 */

		/*  int0 at mainbus0 addr 0x1fb801c0  */
fatal("TODO: SGI legacy interrupt system rewrite!\n");
abort();
//		machine->md_int.sgi_ip20_data = dev_sgi_ip20_init(cpu, mem,
//		    DEV_SGI_IP20_BASE);

		/*  imc0 at mainbus0 addr 0x1fa00000: revision 0:
		    TODO (or in dev_sgi_ip20?)  */

		machine->main_console_handle = (size_t)device_add(machine,
		    "z8530 addr=0x1fbd9830 irq=0 addr_mult=4");

		/*  This is the zsc0 reported by NetBSD:  TODO: irqs  */
		machine->main_console_handle = (size_t)device_add(machine,
		    "z8530 addr=0x1fb80d10 irq=0 addr_mult=4");
		machine->main_console_handle = (size_t)device_add(machine,
		    "z8530 addr=0x1fb80d00 irq=0 addr_mult=4");

		/*  WDSC SCSI controller:  */
/*		dev_wdsc_init(machine, mem, 0x1fb8011f, 0, 0);  */

		/*  Return memory read errors so that hpc1
		    and hpc2 are not detected:  */
		device_add(machine, "unreadable addr=0x1fb00000 len=0x10000");
		device_add(machine, "unreadable addr=0x1f980000 len=0x10000");

		/*  Return nothing for gio slots 0, 1, and 2: */
		device_add(machine, "unreadable addr=0x1f400000 len=0x1000");
		device_add(machine, "unreadable addr=0x1f600000 len=0x1000");
		device_add(machine, "unreadable addr=0x1f000000 len=0x1000");

		break;

	case 21:
		strlcat(machine->machine_name,	/*  TODO  */
		    " (uknown SGI-IP21 ?)", MACHINE_NAME_MAXBUF);
		/*  NOTE:  Special case for arc_wordlen:  */
		arc_wordlen = sizeof(uint64_t);

		device_add(machine, "random addr=0x418000200, len=0x20000");

		break;

	case 22:
	case 24:
		if (machine->machine_subtype == 22) {
			strlcat(machine->machine_name,
			    " (Indy, Indigo2, Challenge S; Full-house)",
			    MACHINE_NAME_MAXBUF);
fatal("TODO: SGI legacy interrupt system rewrite!\n");
abort();
//			machine->md_int.sgi_ip22_data =
//			    dev_sgi_ip22_init(machine, mem, 0x1fbd9000, 0);
		} else {
			strlcat(machine->machine_name,
			    " (Indy, Indigo2, Challenge S; Guiness)",
			    MACHINE_NAME_MAXBUF);
fatal("TODO: SGI legacy interrupt system rewrite!\n");
abort();
//			machine->md_int.sgi_ip22_data =
//			    dev_sgi_ip22_init(machine, mem, 0x1fbd9880, 1);
		}

/*
Why is this here? TODO
		dev_ram_init(machine, 0x88000000ULL,
		    128 * 1048576, DEV_RAM_MIRROR, 0x08000000);
*/

fatal("TODO: Legacy rewrite\n");
abort();
//		machine->md_interrupt = sgi_ip22_interrupt;

		/*
		 *  According to NetBSD 1.6.2:
		 *
		 *  imc0 at mainbus0 addr 0x1fa00000, Revision 0
		 *  gio0 at imc0
		 *  hpc0 at gio0 addr 0x1fb80000: SGI HPC3
		 *  zsc0 at hpc0 offset 0x59830
		 *  zstty0 at zsc0 channel 1 (console i/o)
		 *  zstty1 at zsc0 channel 0
		 *  sq0 at hpc0 offset 0x54000: SGI Seeq 80c03	(Ethernet)
		 *  wdsc0 at hpc0 offset 0x44000: WD33C93 SCSI, rev=0, target 7
		 *  scsibus2 at wdsc0: 8 targets, 8 luns per target
		 *  dsclock0 at hpc0 offset 0x60000
		 *
		 *  According to Linux/IP22:
		 *  tty00 at 0xbfbd9830 (irq = 45) is a Zilog8530
		 *  tty01 at 0xbfbd9838 (irq = 45) is a Zilog8530
		 *
		 *  and according to NetBSD 2.0_BETA (20040606):
		 *
		 *  haltwo0 at hpc0 offset 0x58000: HAL2 revision 0.0.0
		 *  audio0 at haltwo0: half duplex
		 *
		 *  IRQ numbers are of the form 8 + x, where x=0..31 for local0
		 *  interrupts, and 32..63 for local1.  + y*65 for "mappable".
		 */

		/*  zsc0 serial console. 8 + 32 + 3 + 64*5 = 43+64*5 = 363 */
		i = (size_t)device_add(machine,
		    "z8530 addr=0x1fbd9830 irq=363 addr_mult=4");

		/*  Not supported by NetBSD 1.6.2, but by 2.0_BETA:  */
fatal("TODO: legacy rewrite\n");
abort();
//		j = dev_pckbc_init(machine, mem, 0x1fbd9840, PCKBC_8242,
//		    0, 0, machine->x11_md.in_use, 0);  /*  TODO: irq numbers  */
j = 0;

		if (machine->x11_md.in_use)
			machine->main_console_handle = j;

		/*  sq0: Ethernet.  TODO:  This should have irq_nr = 8 + 3  */
		/*  dev_sq_init...  */

		/*  wdsc0: SCSI  */
/*		dev_wdsc_init(machine, mem, 0x1fbc4000, 0, 8 + 1);  */

		/*  wdsc1: SCSI  TODO: irq nr  */
/*		dev_wdsc_init(machine, mem, 0x1fbcc000, 1, 8 + 1);  */

		/*  dsclock0: TODO:  possibly irq 8 + 33  */

		/*  Return memory read errors so that hpc1 and hpc2 are
		    not detected:  */
		device_add(machine, "unreadable addr=0x1fb00000, len=0x10000");
		device_add(machine, "unreadable addr=0x1f980000, len=0x10000");

		/*  Similarly for gio slots 0, 1, and 2:  */
		device_add(machine, "unreadable addr=0x1f400000, len=0x1000");
		device_add(machine, "unreadable addr=0x1f600000, len=0x1000");
		device_add(machine, "unreadable addr=0x1f000000, len=0x1000");

		break;

	case 25:
		/*  NOTE:  Special case for arc_wordlen:  */
		arc_wordlen = sizeof(uint64_t);
		strlcat(machine->machine_name,
		    " (Everest IP25)", MACHINE_NAME_MAXBUF);

		 /*  serial? irix?  */
		dev_scc_init(machine, mem,
		    0x400086000ULL, 0, machine->x11_md.in_use, 0, 8);

		/*  NOTE: ip19! (perhaps not really the same  */
		device_add(machine, "sgi_ip19 addr=0x18000000");

		/*
		 *  Memory size, not 4096 byte pages, but 256
		 *  bytes?  (16 is size of kernel... approx)
		 */
		store_32bit_word(cpu, 0xa0000000ULL + 0x26d0,
		    30000);  /* (machine->physical_ram_in_mb - 16)
				 * (1048576 / 256));  */

		break;

	case 26:
		/*  NOTE:  Special case for arc_wordlen:  */
		arc_wordlen = sizeof(uint64_t);
		strlcat(machine->machine_name, " (uknown SGI-IP26 ?)",
		    MACHINE_NAME_MAXBUF);	/*  TODO  */
		machine->main_console_handle = (size_t)device_add(machine,
		    "z8530 addr=0x1fbd9830 irq=0 addr_mult=4");
		break;

	case 27:
		strlcat(machine->machine_name, " (Origin 200/2000, Onyx2)",
		    MACHINE_NAME_MAXBUF);
		arc_wordlen = sizeof(uint64_t);
		/*  2 cpus per node  */

		machine->main_console_handle = (size_t)device_add(machine,
		    "z8530 addr=0x1fbd9830 irq=0 addr_mult=4");
		break;

	case 28:
		/*  NOTE:  Special case for arc_wordlen:  */
		arc_wordlen = sizeof(uint64_t);
		strlcat(machine->machine_name,
		    " (Impact Indigo2 ?)", MACHINE_NAME_MAXBUF);

		device_add(machine, "random addr=0x1fbe0000, len=1");

		/*  Something at paddr 0x1880fb0000.  */

		break;

	case 30:
		/*  NOTE:  Special case for arc_wordlen:  */
		arc_wordlen = sizeof(uint64_t);
		strlcat(machine->machine_name, " (Octane)",
		    MACHINE_NAME_MAXBUF);

fatal("TODO: SGI legacy interrupt system rewrite!\n");
abort();
//		machine->md_int.sgi_ip30_data =
//		    dev_sgi_ip30_init(machine, mem, 0x0ff00000);

fatal("TODO: Legacy rewrite\n");
abort();
//		machine->md_interrupt = sgi_ip30_interrupt;

		dev_ram_init(machine, 0xa0000000ULL, 128 * 1048576,
		    DEV_RAM_MIRROR | DEV_RAM_MIGHT_POINT_TO_DEVICES,
		    0x00000000);

		dev_ram_init(machine,    0x80000000ULL,
		    32 * 1048576, DEV_RAM_RAM, 0x00000000);

		/*
		 *  Something at paddr=1f022004: TODO
		 *  Something at paddr=813f0510 - paddr=813f0570 ?
		 *  Something at paddr=813f04b8
		 *  Something at paddr=f8000003c  used by Linux/Octane
		 *
		 *  16550 serial port at paddr=1f620178, addr mul 1
		 *  (Error messages are printed to this serial port by
		 *  the PROM.)
		 *
		 *  There seems to also be a serial port at 1f620170. The
		 *  "symmon" program dumps something there, but it doesn't
		 *  look like readable text.  (TODO)
		 */

		/*  TODO: irq!  */
		snprintf(tmpstr, sizeof(tmpstr), "ns16550 irq=0 addr="
		    "0x1f620170 name2=tty0 in_use=%i",
		    machine->x11_md.in_use? 0 : 1);
		machine->main_console_handle = (size_t)device_add(machine,
		    tmpstr);
		snprintf(tmpstr, sizeof(tmpstr), "ns16550 irq=0 addr="
		    "0x1f620178 name2=tty1 in_use=0");
		device_add(machine, tmpstr);

		/*  MardiGras graphics:  */
		device_add(machine, "sgi_mardigras addr=0x1c000000");

		break;

	case 32:
		strlcat(machine->machine_name, " (O2)", MACHINE_NAME_MAXBUF);

		/*  TODO: Find out where the phys ram is actually located.  */
		dev_ram_init(machine, 0x07ffff00ULL,           256,
		    DEV_RAM_MIRROR, 0x03ffff00);
		dev_ram_init(machine, 0x10000000ULL,           256,
		    DEV_RAM_MIRROR, 0x00000000);
		dev_ram_init(machine, 0x11ffff00ULL,           256,
		    DEV_RAM_MIRROR, 0x01ffff00);
		dev_ram_init(machine, 0x12000000ULL,           256,
		    DEV_RAM_MIRROR, 0x02000000);
		dev_ram_init(machine, 0x17ffff00ULL,           256,
		    DEV_RAM_MIRROR, 0x03ffff00);
		dev_ram_init(machine, 0x20000000ULL, 128 * 1048576,
		    DEV_RAM_MIRROR, 0x00000000);
		dev_ram_init(machine, 0x40000000ULL, 128 * 1048576,
		    DEV_RAM_MIRROR, 0x10000000);

		/*  Connect CRIME (Interrupt Controller) to MIPS irq 2:  */
		snprintf(tmpstr, sizeof(tmpstr), "%s.cpu[%i].2",
		    machine->path, machine->bootstrap_cpu);
		dev_crime_init(machine, mem, 0x14000000, tmpstr,
		    machine->x11_md.in_use);			/*  crime0  */
		dev_sgi_mte_init(mem, 0x15000000);		/*  mte ???  */
		dev_sgi_gbe_init(machine, mem, 0x16000000);	/*  gbe?  */

		/*
		 *  A combination of NetBSD and Linux info:
		 *
		 *      17000000	vice (Video Image Compression Engine)
		 *	1f000000	mace
		 *	1f080000	macepci
		 *	1f100000	vin1
		 *	1f180000	vin2
		 *	1f200000	vout
		 *	1f280000	enet (mec0, MAC-110 Ethernet)
		 *	1f300000	perif:
		 *	  1f300000	  audio
		 *	  1f310000	  isa
		 *	    1f318000	    (accessed by Irix'
		 *			     pciio_pio_write64)
		 *	  1f320000	  kbdms
		 *	  1f330000	  i2c
		 *	  1f340000	  ust
		 *	1f380000	isa ext
		 * 	  1f390000	  com0 (serial)
		 * 	  1f398000	  com1 (serial)
		 * 	  1f3a0000	  mcclock0
		 */

		/*
		 *  IRQ mapping is really ugly.  TODO: fix
		 *
		 *  com0 at mace0 offset 0x390000 intr 4 intrmask
		 *	0x3f00000: ns16550a, working fifo
		 *  com1 at mace0 offset 0x398000 intr 4 intrmask
		 *	0xfc000000: ns16550a, working fifo
		 *  pckbc0 at mace0 offset 0x320000 intr 5 intrmask 0x0
		 *  mcclock0 at mace0 offset 0x3a0000 intrmask 0x0
		 *  macepci0 at mace0 offset 0x80000 intr 7 intrmask 0x0: rev 1
		 *
		 *  intr 4 = MACE_PERIPH_SERIAL
		 *  intr 5 = MACE_PERIPH_MISC
		 *  intr 7 = MACE_PCI_BRIDGE
		 */

		snprintf(eaddr_string, ETHERNET_STRING_MAXLEN,
		    "eaddr=%02x:%02x:%02x:%02x:%02x:%02x",
		    macaddr[0], macaddr[1], macaddr[2],
		    macaddr[3], macaddr[4], macaddr[5]);

		snprintf(tmpstr, sizeof(tmpstr), "%s.cpu[%i].2.crime.0x%x",
		    machine->path, machine->bootstrap_cpu, MACE_ETHERNET);
		dev_sgi_mec_init(machine, mem, 0x1f280000,
		    tmpstr, macaddr);

		dev_sgi_ust_init(mem, 0x1f340000);  /*  ust?  */

		snprintf(tmpstr, sizeof(tmpstr),
		    "ns16550 irq=%s.cpu[%i].2.crime.0x%x.mace.%i addr="
		    "0x1f390000 addr_mult=0x100 in_use=%i name2=tty0",
		    machine->path, machine->bootstrap_cpu,
		    MACE_PERIPH_SERIAL, 20, machine->x11_md.in_use? 0 : 1);
		j = (size_t)device_add(machine, tmpstr);
		snprintf(tmpstr, sizeof(tmpstr),
		    "ns16550 irq=%s.cpu[%i].2.crime.0x%x.mace.%i addr="
		    "0x1f398000 addr_mult=0x100 in_use=%i name2=tty1",
		    machine->path, machine->bootstrap_cpu,
		    MACE_PERIPH_SERIAL, 26, 0);
		device_add(machine, tmpstr);

		machine->main_console_handle = j;

		/*  TODO: Once this works, it should be enabled
		    always, not just when using X!  */
#if 0
fatal("TODO: legacy SGI rewrite\n");
abort();
		if (machine->x11_md.in_use) {
			i = dev_pckbc_init(machine, mem, 0x1f320000,
			    PCKBC_8242, 0x200 + MACE_PERIPH_MISC,
			    0x800 + MACE_PERIPH_MISC, machine->x11_md.in_use,
				0);
				/*  keyb+mouse (mace irq numbers)  */
			machine->main_console_handle = i;
		}
#endif

		snprintf(tmpstr, sizeof(tmpstr),
		    "%s.cpu[%i].2.crime.0x%x.mace.%i",
		    machine->path, machine->bootstrap_cpu,
		    MACE_PERIPH_MISC, 8);
		dev_mc146818_init(machine, mem, 0x1f3a0000, tmpstr,
		    MC146818_SGI, 0x40);  /*  mcclock0  */

		/*  TODO: _WHERE_ does the z8530 interrupt?  */
		snprintf(tmpstr, sizeof(tmpstr), "z8530 addr=0x1fbd9830 "
		    "irq=%s.cpu[%i].2 addr_mult=4",
		    machine->path, machine->bootstrap_cpu);
		machine->main_console_handle = (size_t)
		    device_add(machine, tmpstr);

		/*
		 *  PCI devices:   (according to NetBSD's GENERIC
		 *  config file for sgimips)
		 *
		 *	ne*             at pci? dev ? function ?
		 *	ahc0            at pci0 dev 1 function ?
		 *	ahc1            at pci0 dev 2 function ?
		 */

		snprintf(tmpstr, sizeof(tmpstr),
		    "%s.cpu[%i].2.crime.0x%x", machine->path,
		    machine->bootstrap_cpu, MACE_PCI_BRIDGE);
		pci_data = dev_macepci_init(machine, mem, 0x1f080000,
		    tmpstr);		/*  macepci0  */
		/*  bus_pci_add(machine, pci_data, mem, 0, 0, 0,
		    "ne2000");  TODO  */

		/*  TODO: make this nicer  */
		if (diskimage_exist(machine, 0, DISKIMAGE_SCSI) ||
		    diskimage_exist(machine, 1, DISKIMAGE_SCSI) ||
		    diskimage_exist(machine, 2, DISKIMAGE_SCSI) ||
		    diskimage_exist(machine, 3, DISKIMAGE_SCSI) ||
		    diskimage_exist(machine, 4, DISKIMAGE_SCSI) ||
		    diskimage_exist(machine, 5, DISKIMAGE_SCSI) ||
		    diskimage_exist(machine, 6, DISKIMAGE_SCSI) ||
		    diskimage_exist(machine, 7, DISKIMAGE_SCSI))
			bus_pci_add(machine, pci_data, mem, 0, 1, 0, "ahc");

		/*  TODO: second ahc  */
		/*  bus_pci_add(machine, pci_data, mem, 0, 2, 0, "ahc");  */

		/*
		 *  An additional PCI IDE controller, for NetBSD/sgimips
		 *  experiments:  (Not found in a regular O2.)
		 */
		bus_pci_add(machine, pci_data, mem, 0, 3, 0, "symphony_82c105");

		break;

	case 35:
		strlcat(machine->machine_name,
		    " (Origin 3000)", MACHINE_NAME_MAXBUF);
		/*  4 cpus per node  */

		machine->main_console_handle = (size_t)device_add(machine,
		    "z8530 addr=0x1fbd9830 irq=0 addr_mult=4");
		break;

	case 53:
		strlcat(machine->machine_name, " (Origin 350)",
		    MACHINE_NAME_MAXBUF);

		/*
		 *  According to http://kumba.drachentekh.net/xml/myguide.html
		 *  Origin 350, Tezro IP53 R16000
		 */
		break;

	default:
		fatal("unimplemented SGI machine type IP%i\n",
		    machine->machine_subtype);
		exit(1);
	}

	if (!machine->prom_emulation)
		return;

	arcbios_init(machine, arc_wordlen == sizeof(uint64_t), sgi_ram_offset,
	    eaddr_string, macaddr);
}


MACHINE_DEFAULT_CPU(sgi)
{
	if (machine->machine_subtype <= 12)
	        machine->cpu_name = strdup("R3000");
	if (machine->cpu_name == NULL && machine->machine_subtype == 35)
	        machine->cpu_name = strdup("R12000");
	if (machine->cpu_name == NULL && (machine->machine_subtype == 25 ||
	    machine->machine_subtype == 27 || machine->machine_subtype == 28 ||
	    machine->machine_subtype == 30 || machine->machine_subtype == 32))
	        machine->cpu_name = strdup("R10000");
	if (machine->cpu_name == NULL && (machine->machine_subtype == 21 ||
	    machine->machine_subtype == 26))
	        machine->cpu_name = strdup("R8000");
	if (machine->cpu_name == NULL && machine->machine_subtype == 24)
	        machine->cpu_name = strdup("R5000");
                        
	/*  Other SGIs should probably work with
	    R4000, R4400 or R5000 or similar:  */
	if (machine->cpu_name == NULL)
	        machine->cpu_name = strdup("R4400");
}


MACHINE_DEFAULT_RAM(sgi)
{
	machine->physical_ram_in_mb = 64;
}


MACHINE_REGISTER(sgi)
{
	MR_DEFAULT(sgi, "SGI", ARCH_MIPS, MACHINE_SGI);

	me->set_default_ram = machine_default_ram_sgi;

	machine_entry_add_alias(me, "silicon graphics");
	machine_entry_add_alias(me, "sgi");

	machine_entry_add_subtype(me, "IP12", 12, "ip12", NULL);

	machine_entry_add_subtype(me, "IP19", 19, "ip19", NULL);

	machine_entry_add_subtype(me, "IP20", 20, "ip20", NULL);

	machine_entry_add_subtype(me, "IP22", 22, "ip22", "indy", NULL);

	machine_entry_add_subtype(me, "IP24", 24, "ip24", NULL);

	machine_entry_add_subtype(me, "IP27", 27,
	    "ip27", "origin 200", "origin 2000", NULL);

	machine_entry_add_subtype(me, "IP28", 28, "ip28", NULL);

	machine_entry_add_subtype(me, "IP30", 30, "ip30", "octane", NULL);

	machine_entry_add_subtype(me, "IP32", 32, "ip32", "o2", NULL); 

	machine_entry_add_subtype(me, "IP35", 35, "ip35", NULL); 
}

