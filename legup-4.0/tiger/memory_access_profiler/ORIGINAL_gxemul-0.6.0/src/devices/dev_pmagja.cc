/*
 *  Copyright (C) 2004-2009  Anders Gavare.  All rights reserved.
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
 *  COMMENT: TURBOchannel PMAG-JA graphics card
 *
 *  TODO
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "devices.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"


#define	XSIZE		1280
#define	YSIZE		1024

#define	PMAGJA_FIRSTOFFSET	0x40030

/*  #define JA_DEBUG  */

struct pmagja_data {
	struct interrupt	irq;
	struct memory		*fb_mem;
	struct vfb_data		*vfb_data;

	unsigned char		pixeldata[XSIZE * YSIZE];

	int			current_r;
	int			current_g;
	int			current_b;

	int			pip_offset;
};


DEVICE_ACCESS(pmagja)
{
	struct pmagja_data *d = (struct pmagja_data *) extra;
	uint64_t idata = 0, odata = 0;
	size_t i, res = 1;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	relative_addr += PMAGJA_FIRSTOFFSET;

#ifdef JA_DEBUG
{
size_t i;
for (i=0; i<len; i++)
  if (data[i] != 0 && data[i] != 1 && data[i]!=0xff)
	if (writeflag)
		fatal("[ pmagja: write to addr 0x%08llx: 0x%08llx ]\n",
		    (long long)relative_addr, (long long)idata);
}
#endif

	switch (relative_addr) {
	case 0x0800c4:		/*  pip offset  */
		if (writeflag==MEM_READ) {
			odata = d->pip_offset;
			debug("[ pmagja: read from pip offset: 0x%08llx ]\n",
			    (long long)odata);
		} else {
			d->pip_offset = idata;
			debug("[ px: write to pip offset: 0x%08llx ]\n",
			    (long long)idata);
		}
		break;
	default:
		if (relative_addr >= 0x200000) {
			/*  8-bit access:  */
			for (i=0; i<len; i++) {
				int ofs = relative_addr - 0x200000 + i;
				int x, y;
				unsigned char newdata[3];
				y = ofs / XSIZE;
				x = ofs - y*XSIZE;

				if (writeflag) {
					d->pixeldata[x + y*XSIZE] = data[i];
					newdata[0] = d->vfb_data->rgb_palette[
					    data[i] * 3 + 0];
					newdata[1] = d->vfb_data->rgb_palette[
					    data[i] * 3 + 1];
					newdata[2] = d->vfb_data->rgb_palette[
					    data[i] * 3 + 2];
					dev_fb_access(cpu, d->fb_mem, (x +
					    y * XSIZE) * 3, newdata, 3,
					    writeflag, d->vfb_data);
				} else {
					data[i] = d->pixeldata[x + y*XSIZE];
				}
			}
			/*  Return success.  */
			return 1;
		} else if (relative_addr >= 0x100000 &&
		    relative_addr < 0x200000) {
			/*  24-bit access:  */
#if 0
{
	if (writeflag)
	    if (idata != 0)
		fatal("[ pmagja: write to addr 0x%08llx: 0x%08llx ]\n",
		    (long long)relative_addr, (long long)idata);
}
#endif
			int x, y, ofs;

			ofs = (relative_addr - 0x100000) * 2;
			y = ofs / XSIZE;
			x = ofs - y * XSIZE;

#if 0
			if (writeflag == MEM_WRITE) {
				int ix;
				for (ix=0; ix<len*2; ix++) {
					unsigned char data[3];
					int ctype;

					data[0] = data[1] = data[2] = 0;
					ctype = (idata >> (ix*4)) & 0xf;
					if (ctype == 0)
						data[0] = 255;
					if (ctype == 3)
						data[1] = 255;
					if (ctype == 0xf)
						data[2] = 255;

					res = dev_fb_access(cpu, d->fb_mem,
					    ofs * 3, data, 3, MEM_WRITE,
					    d->vfb_data);
					ofs ++;
				}
			}
#endif
			return 1;
		} else {
			/*  Unknown:  */
			if (writeflag==MEM_READ) {
				fatal("[ pmagja: read from addr 0x%x: "
				    "0x%llx ]\n", (int)relative_addr,
				    (long long)odata);
			} else {
				fatal("[ pmagja: write to addr 0x%x: "
				    "0x%llx ]\n", (int)relative_addr,
				    (long long)idata);
			}
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

#ifdef JA_DEBUG
/*
	if (!writeflag)
		fatal("[ pmagja: read from addr 0x%08llx: 0x%08llx ]\n",
		    (long long)relative_addr, (long long)odata);
*/
#endif

	return res;
}


void dev_pmagja_init(struct machine *machine, struct memory *mem,
	uint64_t baseaddr, const char *irq_path)
{
	struct pmagja_data *d;

	CHECK_ALLOCATION(d = (struct pmagja_data *) malloc(sizeof(struct pmagja_data)));
	memset(d, 0, sizeof(struct pmagja_data));

	INTERRUPT_CONNECT(irq_path, d->irq);

	d->fb_mem = memory_new(XSIZE * YSIZE * 3, machine->arch);
	if (d->fb_mem == NULL) {
		fprintf(stderr, "dev_pmagja_init(): out of memory (1)\n");
		exit(1);
	}
	d->vfb_data = dev_fb_init(machine, d->fb_mem, 0, VFB_GENERIC,
	    XSIZE, YSIZE, XSIZE, YSIZE, 24, "PMAG-JA");
	if (d->vfb_data == NULL) {
		fprintf(stderr, "dev_pmagja_init(): out of memory (2)\n");
		exit(2);
	}

	/*  TODO: not bt459, but a bt463:  */
	dev_bt459_init(machine, mem, baseaddr + 0x40000, 0, d->vfb_data, 8,
	    irq_path, 0);	/*  palette  (TODO: type)  */
	dev_bt431_init(mem, baseaddr + 0x40010, d->vfb_data, 8);  /*  cursor  */

	memory_device_register(mem, "pmagja", baseaddr + PMAGJA_FIRSTOFFSET,
	    DEV_PMAGJA_LENGTH - PMAGJA_FIRSTOFFSET, dev_pmagja_access, d,
	    DM_DEFAULT, NULL);
}

