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
 *  COMMENT: MardiGras graphics controller on SGI IP30 (Octane)
 *
 *  Most of this is just guesses based on the behaviour of Linux/Octane.
 *
 *  TODO
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "devices.h"
#include "memory.h"
#include "machine.h"
#include "misc.h"


#define debug fatal


#define	DEV_SGI_MARDIGRAS_LENGTH	0x800000

#define	MARDIGRAS_FAKE_OFFSET	0x500000000ULL	/*  hopefully available  */
#define	MARDIGRAS_DEFAULT_XSIZE	1280
#define	MARDIGRAS_DEFAULT_YSIZE	1024

#define	MICROCODE_START		0x50000
#define	MICROCODE_END		0x55000

static int mardigras_xsize = MARDIGRAS_DEFAULT_XSIZE;
static int mardigras_ysize = MARDIGRAS_DEFAULT_YSIZE;

struct sgi_mardigras_data {
	struct vfb_data		*fb;
	unsigned char		microcode_ram[MICROCODE_END - MICROCODE_START];
	uint64_t		palette_reg_select;
	int			currentx;
	int			currenty;
	int			color;
	int			startx;
	int			starty;
	int			stopx;
	int			stopy;
	uint64_t		draw_mode;
};


/*
 *  mardigras_20400():
 */
void mardigras_20400(struct cpu *cpu, struct sgi_mardigras_data *d,
	uint64_t idata)
{
	int i, x, y, r,g,b, len, addr;
	unsigned char pixels[3 * 8000];

	/*  Get rgb from palette:  */
	r = d->fb->rgb_palette[d->color * 3 + 0];
	g = d->fb->rgb_palette[d->color * 3 + 1];
	b = d->fb->rgb_palette[d->color * 3 + 2];

	/*  Set color:  */
	if ((idata & 0x00ffffff00000000ULL) == 0x00185C0400000000ULL) {
		int color = (idata >> 12) & 0xff;
		d->color = color;
		return;
	}

	/*  Set start XY:  */
	if ((idata & 0x00ffffff00000000ULL) == 0x0018460400000000ULL) {
		d->startx = (idata >> 16) & 0xffff;
		d->starty = idata & 0xffff;
		if (d->startx >= mardigras_xsize)
			d->startx = 0;
		if (d->starty >= mardigras_ysize)
			d->starty = 0;
		d->currentx = d->startx;
		d->currenty = d->starty;
		return;
	}

	/*  Set stop XY:  */
	if ((idata & 0x00ffffff00000000ULL) == 0x0018470400000000ULL) {
		d->stopx = (idata >> 16) & 0xffff;
		d->stopy = idata & 0xffff;
		if (d->stopx >= mardigras_xsize)
			d->stopx = 0;
		if (d->stopy >= mardigras_ysize)
			d->stopy = 0;
		return;
	}

	/*  Draw modes: (Rectangle or Bitmap, respectively)  */
	if (idata == 0x0019100400018000ULL ||
	    idata == 0x0019100400418008ULL) {
		d->draw_mode = idata;
		return;
	}

	/*  Send command:  */
	if (idata == 0x001C130400000018ULL) {
		switch (d->draw_mode) {
		/*  Rectangle:  */
		case 0x0019100400018000ULL:
			/*  Fill pixels[] with pixels:  */
			len = 0;
			for (x=d->startx; x<=d->stopx; x++) {
				pixels[len + 0] = r;
				pixels[len + 1] = g;
				pixels[len + 2] = b;
				len += 3;
			}
			if (len == 0)
				break;
			for (y=d->starty; y<=d->stopy; y++) {
				addr = (mardigras_xsize * (mardigras_ysize -
				    1 - y) + d->startx) * 3;
				/*  printf("addr = %i\n", addr);  */

				/*  Write a line:  */
				dev_fb_access(cpu, cpu->mem,
				    addr, pixels, len, MEM_WRITE, d->fb);
			}
			break;
		/*  Bitmap:  */
		case 0x0019100400418008ULL:
			break;
		default:
			fatal("[ sgi_mardigras: unknown draw mode ]\n");
		}
		return;
	}

	/*  Send a line of bitmap data:  */
	if ((idata & 0x00ffffff00000000ULL) == 0x001C700400000000ULL) {
		addr = (mardigras_xsize * (mardigras_ysize - 1 - d->currenty)
		    + d->currentx) * 3;
/*
		printf("addr=%08x curx,y=%4i,%4i startx,y=%4i,%4i "
		    "stopx,y=%4i,%4i\n", addr, d->currentx, d->currenty,
		    d->startx, d->starty, d->stopx, d->stopy);
*/
		len = 8*3;

		if (addr > mardigras_xsize * mardigras_ysize * 3 || addr < 0)
			return;

		/*  Read a line:  */
		dev_fb_access(cpu, cpu->mem,
		    addr, pixels, len, MEM_READ, d->fb);

		i = 0;
		while (i < 8) {
			if ((idata >> (24 + (7-i))) & 1) {
				pixels[i*3 + 0] = r;
				pixels[i*3 + 1] = g;
				pixels[i*3 + 2] = b;
			}
			i ++;

			d->currentx ++;
			if (d->currentx > d->stopx) {
				d->currentx = d->startx;
				d->currenty ++;
				if (d->currenty > d->stopy)
					d->currenty = d->starty;
			}
		}

		/*  Write a line:  */
		dev_fb_access(cpu, cpu->mem,
		    addr, pixels, len, MEM_WRITE, d->fb);

		return;
	}

	debug("mardigras_20400(): 0x%016"PRIx64"\n", (uint64_t) idata);
}


DEVICE_ACCESS(sgi_mardigras)
{
	uint64_t idata = 0, odata = 0;
	struct sgi_mardigras_data *d = (struct sgi_mardigras_data *) extra;
	int i;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	/*  Accessing the microcode_ram works like ordinary ram:  */
	if (relative_addr >= MICROCODE_START &&
	    relative_addr <  MICROCODE_END) {
		relative_addr -= MICROCODE_START;
		if (writeflag == MEM_WRITE)
			memcpy(d->microcode_ram + relative_addr, data, len);
		else
			memcpy(data, d->microcode_ram + relative_addr, len);
		return 1;
	}

	switch (relative_addr) {

	case 0x00004:
		/*  xtalk data:  (according to Linux/IP30)  */
		/*  (mfgr & 0x7ff) << 1  */
		/*  (part & 0xffff) << 12  */
		/*  (rev  & 0xf) << 28  */
		odata = (2 << 28) | (0xc003 << 12) | (0x2aa << 1);
		break;

	case 0x20008:	/*  Fifo status  */
		break;

	case 0x20200:
		break;

	case 0x20400:
		if (writeflag == MEM_WRITE)
			mardigras_20400(cpu, d, idata);
		else
			debug("[ sgi_mardigras: read from 0x20400? ]\n");
		break;

	case 0x58040:
		/*  HQ4 microcode stuff  */
		break;

	case 0x70c30:
		/*  Palette register select?  */
		if (writeflag == MEM_WRITE)
			d->palette_reg_select = idata;
		else
			odata = d->palette_reg_select;
		break;

	case 0x70d18:
		/*  Palette register read/write?  */
		i = 3 * ((d->palette_reg_select >> 8) & 0xff);
		if (writeflag == MEM_WRITE) {
			d->fb->rgb_palette[i + 0] = (idata >> 24) & 0xff;
			d->fb->rgb_palette[i + 1] = (idata >> 16) & 0xff;
			d->fb->rgb_palette[i + 2] = (idata >>  8) & 0xff;
		} else {
			odata = (d->fb->rgb_palette[i+0] << 24) +
				(d->fb->rgb_palette[i+1] << 16) +
				(d->fb->rgb_palette[i+2] << 8);
		}
		break;

	case 0x71208:
		odata = 8;
		break;

	default:
		if (writeflag==MEM_READ) {
			debug("[ sgi_mardigras: read from 0x%08lx ]\n",
			    (long)relative_addr);
		} else {
			debug("[ sgi_mardigras: write to  0x%08lx: 0x%016"PRIx64
			    " ]\n", (long) relative_addr, (uint64_t) idata);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(sgi_mardigras)
{
	struct sgi_mardigras_data *d;

	CHECK_ALLOCATION(d = (struct sgi_mardigras_data *) malloc(sizeof(struct sgi_mardigras_data)));
	memset(d, 0, sizeof(struct sgi_mardigras_data));

	d->fb = dev_fb_init(devinit->machine, devinit->machine->memory,
	    MARDIGRAS_FAKE_OFFSET, VFB_GENERIC,
	    mardigras_xsize, mardigras_ysize,
	    mardigras_xsize, mardigras_ysize, 24, "SGI MardiGras");
	if (d->fb == NULL) {
		fprintf(stderr, "dev_sgi_mardigras_init(): out of memory\n");
		exit(1);
	}

	memory_device_register(devinit->machine->memory, devinit->name,
	    devinit->addr, DEV_SGI_MARDIGRAS_LENGTH, dev_sgi_mardigras_access,
	    d, DM_DEFAULT, NULL);

	return 1;
}

