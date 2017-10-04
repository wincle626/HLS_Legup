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
 *  COMMENT: TURBOchannel bus framework, used in DECstation machines
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "devices.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/sfbreg.h"


#define	DEVICE_MAX_NAMELEN		9
#define	CARD_NAME_BUFLEN		9
#define	CARD_FIRMWARE_BUFLEN		5

struct turbochannel_data {
	int		slot_nr;
	uint64_t	baseaddr;
	uint64_t	endaddr;

	int		rom_skip;

	char		device_name[DEVICE_MAX_NAMELEN];  /*  NUL-terminated  */

	/*  These should be terminated with spaces  */
	char		card_firmware_version[CARD_NAME_BUFLEN];
	char		card_vendor_name[CARD_NAME_BUFLEN];
	char		card_module_name[CARD_NAME_BUFLEN];
	char		card_firmware_type[CARD_FIRMWARE_BUFLEN];
};


DEVICE_ACCESS(turbochannel)
{
	struct turbochannel_data *d = (struct turbochannel_data *) extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	relative_addr += d->rom_skip;

	if (writeflag == MEM_READ) {
		debug("[ turbochannel: read from slot %i addr 0x%08lx (",
		    d->slot_nr, (long)relative_addr);

		relative_addr &= 0x7fff;

		switch (relative_addr) {
		case 0x3e0:
			odata = 0x00000001; debug("ROM width");
			break;
		case 0x3e4:
			odata = 0x00000004; debug("ROM stride");
			break;
		case 0x3e8:
			/*  8KB * romsize  */
			odata = 0x00000001; debug("ROM size");
			break;
		case 0x3ec:
			/*  4MB * slotsize  */
			odata = 0x00000001; debug("slot size");
			break;
		case 0x3f0:
			odata = 0x55555555; debug("ROM signature byte 0");
			break;
		case 0x3f4:
			odata = 0x00000000; debug("ROM signature byte 1");
			break;
		case 0x3f8:
			odata = 0xaaaaaaaa; debug("ROM signature byte 2");
			break;
		case 0x3fc:
			odata = 0xffffffff; debug("ROM signature byte 3");
			break;
		case 0x470:
			/*  0=nothing, 1=parity  */
			odata = 0x00000000; debug("flags"); break;
		default:
			if (relative_addr >= 0x400 && relative_addr < 0x420)
				odata = d->card_firmware_version[
				    (relative_addr-0x400)/4];
			else if (relative_addr >= 0x420 &&
			    relative_addr < 0x440)
				odata = d->card_vendor_name[
				    (relative_addr-0x420)/4];
			else if (relative_addr >= 0x440 &&
			    relative_addr < 0x460)
				odata = d->card_module_name[
				    (relative_addr-0x440)/4];
			else if (relative_addr >= 0x460 &&
			    relative_addr < 0x470)
				odata = d->card_firmware_type[
				    (relative_addr-0x460)/4];
			else {
				debug("?");
			}
		}

		/*
		 *  If this slot is empty, return an error so that a DBE
		 *  exception is caused. (This is the way DECstation operating
		 *  systems have to detect the absence of cards in a
		 *  TURBOchannel slot.)
		 *
		 *  NOTE:  The Sprite kernel reads from offsets 0x3e0..0x400
		 *  without handling the DBE exception, and both Ultrix and
		 *  NetBSD seem to detect the DBE exception using addresses
		 *  around offset 0x0 or 0x3c0000.  This code seems to work
		 *  with all of the those OS kernels:
		 */
		if (d->card_module_name[0] ==  ' ') {
			/*  Return no data for empty slot:  */
			odata = 0;

			/*  Return DBE exception in some cases:  */
			if (relative_addr < 0x3e0 || relative_addr >= 0x500)
				return 0;
		}

		debug(") ]\n");
	} else {
		/*  debug("[ turbochannel: write to  0x%08lx: 0x%08x ]\n",
		    (long)relative_addr, (int)idata);  */
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


/*
 *  dev_turbochannel_init():
 *
 *  This is a generic turbochannel card device.  device_name should point
 *  to a string such as "PMAG-BA".
 *
 *  TODO: When running for example dual-head, maybe the name of each
 *        framebuffer should include the card slot number?
 */
void dev_turbochannel_init(struct machine *machine, struct memory *mem,
	int slot_nr, uint64_t baseaddr, uint64_t endaddr,
	const char *device_name, const char *irq_path)
{
	struct vfb_data *fb;
	struct turbochannel_data *d;
	int rom_offset=0x3c0000, rom_length=DEV_TURBOCHANNEL_LEN, rom_skip=0;
	char *name2;
	size_t nlen;

	if (device_name == NULL)
		return;

	if (strlen(device_name) > 8) {
		fprintf(stderr, "dev_turbochannel_init(): bad device_name\n");
		exit(1);
	}

	CHECK_ALLOCATION(d = (struct turbochannel_data *) malloc(sizeof(struct turbochannel_data)));
	memset(d, 0, sizeof(struct turbochannel_data));

	d->slot_nr  = slot_nr;
	d->baseaddr = baseaddr;
	d->endaddr  = endaddr;

	strlcpy(d->device_name, device_name, DEVICE_MAX_NAMELEN);

	strncpy(d->card_firmware_version, "V5.3a   ", CARD_NAME_BUFLEN);
	strncpy(d->card_vendor_name,      "DEC     ", CARD_NAME_BUFLEN);
	strncpy(d->card_firmware_type,    "TCF0", CARD_FIRMWARE_BUFLEN);

	memset(d->card_module_name, ' ', 8);
	memcpy(d->card_module_name, device_name, strlen(device_name));

	/*
	 *  According to NetBSD/pmax:
	 *
	 *  PMAD-AA:  le1 at tc0 slot 2 offset 0x0: address 00:00:00:00:00:00
	 *  PMAG-AA:  mfb0 at tc0 slot 2 offset 0x0: 1280x1024x8
	 *  PMAG-BA:  cfb0 at tc0 slot 2 offset 0x0cfb0: 1024x864x8
	 *  PMAG-CA:  px0 at tc0 slot 2 offset 0x0: 2D, 4x1 stamp, 8 plane
	 *	(PMAG-DA,EA,FA,FB are also pixelstamps)
	 *  PMAG-DV:  xcfb0 at tc0 slot 2 offset 0x0: 1024x768x8
	 *  PMAG-JA:  "truecolor" in Ultrix
	 *  PMAGB-BA: sfb0 at tc0 slot 0 offset 0x0: 0x0x8
	 *  PMAGB-VA: sfb0 at tc0 slot 2 offset 0x0: 0x0x8
	 *  PMAZ-AA:  asc0 at tc0 slot 2 offset 0x0: NCR53C94, 25MHz, SCSI ID 7
	 *
	 *  PMAF-FA:  fta0 at tc0 slot 0  (fddi network, "DEC", "V1.0b")
	 */

	if (strcmp(device_name, "PMAD-AA")==0) {
		/*  le in NetBSD, Lance ethernet  */
		dev_le_init(machine, mem, baseaddr, 0, 0,
		    irq_path, DEV_LE_LENGTH);
		/*  One ROM at 0x1c03e0, and one at 0x3c0000.  */
		rom_skip = 0x300;
		rom_offset = 0x1c0000;
		rom_length = 0x201000;
	} else if (strcmp(device_name, "PMAZ-AA")==0) {
		/*  asc in NetBSD, SCSI  */
		dev_asc_init(machine, mem, baseaddr, irq_path, d,
		    DEV_ASC_DEC, NULL, NULL);
		rom_offset = 0xc0000;
		/*  There is a copy at 0x0, at least that's where Linux
		    looks for the rom signature  */
	} else if (strcmp(device_name, "PMAG-AA")==0) {
		/*  mfb in NetBSD  */
		fb = dev_fb_init(machine, mem, baseaddr + VFB_MFB_VRAM,
		    VFB_GENERIC, 1280, 1024, 2048, 1024, 8, device_name);
		/*  bt455 = palette, bt431 = cursor  */
		dev_bt455_init(mem, baseaddr + VFB_MFB_BT455, fb);
		dev_bt431_init(mem, baseaddr + VFB_MFB_BT431, fb, 8);
		rom_offset = 0;
	} else if (strcmp(device_name, "PMAG-BA")==0) {
		/*  cfb in NetBSD  */
		fb = dev_fb_init(machine, mem, baseaddr, VFB_GENERIC,
		    1024,864, 1024,1024,8, device_name);
		dev_bt459_init(machine, mem, baseaddr + VFB_CFB_BT459,
		    baseaddr + 0x300000, fb, 8, irq_path, BT459_BA);
		/*  ROM at both 0x380000 and 0x3c0000?  */
		rom_offset = 0x380000;
		rom_length = 0x080000;
	} else if (strcmp(device_name, "PMAGB-BA")==0) {
		/*  sfb in NetBSD  */
		/*  TODO: This is not working with Ultrix yet.  */
		fb = dev_fb_init(machine, mem, baseaddr + SFB_OFFSET_VRAM,
		    VFB_GENERIC, 1280,1024, 1280,1024,8, device_name);
		dev_sfb_init(machine, mem, baseaddr + SFB_ASIC_OFFSET, fb);
		/*  TODO: the CLEAR doesn't get through, as the address
			range is already in use by the asic  */
		dev_bt459_init(machine, mem, baseaddr + SFB_OFFSET_BT459,
		    baseaddr + SFB_CLEAR, fb, 8, irq_path, BT459_BBA);
		rom_offset = 0x0;	/*  ? TODO  */
	} else if (strcmp(device_name, "PMAG-CA")==0) {
		/*  px in NetBSD  */
		dev_px_init(machine, mem, baseaddr, DEV_PX_TYPE_PX, irq_path);
		rom_offset = 0x3c0000;
	} else if (strcmp(device_name, "PMAG-DA")==0) {
		/*  pxg in NetBSD  */
		dev_px_init(machine, mem, baseaddr, DEV_PX_TYPE_PXG, irq_path);
		rom_offset = 0x3c0000;
	} else if (strcmp(device_name, "PMAG-EA")==0) {
		/*  pxg+ in NetBSD: TODO  (not supported by the kernel
		    I've tried)  */
		fatal("TODO (see dev_turbochannel.c)\n");
		rom_offset = 0x3c0000;
	} else if (strcmp(device_name, "PMAG-FA")==0) {
		/*  "pxg+ Turbo" in NetBSD  */
		dev_px_init(machine, mem, baseaddr,
		    DEV_PX_TYPE_PXGPLUSTURBO, irq_path);
		rom_offset = 0x3c0000;
	} else if (strcmp(device_name, "PMAG-DV")==0) {
		/*  xcfb in NetBSD: TODO  */
		fb = dev_fb_init(machine, mem, baseaddr + 0x2000000,
		    VFB_DEC_MAXINE, 0, 0, 0, 0, 0, "PMAG-DV");
		/*  TODO:  not yet usable, needs a IMS332 vdac  */
		rom_offset = 0x3c0000;
	} else if (strcmp(device_name, "PMAG-JA")==0) {
		/*  "Truecolor", mixed 8- and 24-bit  */
		dev_pmagja_init(machine, mem, baseaddr, irq_path);
		rom_offset = 0;		/*  NOTE: 0, not 0x3c0000  */
	} else if (strcmp(device_name, "PMAG-RO")==0) {
		/*  This works at least B/W in Ultrix, so far.  */
		fb = dev_fb_init(machine, mem, baseaddr + 0x200000,
		    VFB_GENERIC, 1280,1024, 1280,1024, 8, "PMAG-RO");
		/*  TODO: bt463 at offset 0x040000, not bt459  */
		dev_bt459_init(machine, mem, baseaddr + 0x40000, 0,
		    fb, 8, irq_path, 0);		/*  TODO: type  */
		dev_bt431_init(mem, baseaddr + 0x40010, fb, 8);  /*  cursor  */
		rom_offset = 0x3c0000;
	} else if (device_name[0] == '\0') {
		/*  If this slot is empty, then occupy the entire
		    4MB slot address range:  */
		rom_offset = 0;
		rom_length = 4*1048576;
	} else {
		fatal("warning: unknown TURBOchannel device name \"%s\"\n",
		    device_name);
	}

	d->rom_skip = rom_skip;

	nlen = strlen(device_name) + 30;
	CHECK_ALLOCATION(name2 = (char *) malloc(nlen));

	if (*device_name)
		snprintf(name2, nlen, "turbochannel [%s]", device_name);
	else
		snprintf(name2, nlen, "turbochannel");

	memory_device_register(mem, name2, baseaddr + rom_offset + rom_skip,
	    rom_length-rom_skip, dev_turbochannel_access, d, DM_DEFAULT, NULL);
}

