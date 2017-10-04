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
 *  COMMENT: DEC KN01 ("PMAX", DECstation type 1) control register and VDAC
 *
 *  TODO: The CSR isn't really complete.
 *
 *  One of the few usable bits in the csr would be KN01_CSR_MONO.
 *  If that bit is set, the framebuffer is treated as a monochrome
 *  one.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "devices.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/dec_kn01.h"


struct kn01_data {
	int			color_fb;
	uint32_t		csr;
};


struct vdac_data {
	unsigned char	vdac_reg[DEV_VDAC_LENGTH];

	int		color_fb_flag;

	unsigned char	cur_read_addr;
	unsigned char	cur_write_addr;

	int		sub_color;	/*  subcolor. 0, 1, or 2  */
	unsigned char	cur_rgb[3];

	unsigned char	*rgb_palette;		/*  ptr to 256 * 3 (r,g,b)  */

	unsigned char	cur_read_addr_overlay;
	unsigned char	cur_write_addr_overlay;

	int		sub_color_overlay;	/*  subcolor: 0, 1, or 2  */
	unsigned char	cur_rgb_overlay[3];

	unsigned char	rgb_palette_overlay[16 * 3];	/*  16 * 3 (r,g,b)  */
};


DEVICE_ACCESS(kn01)
{
	struct kn01_data *d = (struct kn01_data *) extra;
	int csr;

	if (writeflag == MEM_WRITE) {
		/*  TODO  */
		return 1;
	}

	/*  Read:  */
	if (len != 2 || relative_addr != 0) {
		fatal("[ kn01: trying to read something which is not "
		    "the first half-word of the csr ]");
	}

	csr = d->csr;

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN) {
		data[0] = csr & 0xff;
		data[1] = (csr >> 8) & 0xff;
	} else {
		data[1] = csr & 0xff;
		data[0] = (csr >> 8) & 0xff;
	}

	return 1;
}


/*
 *  dev_vdac_access():
 */
DEVICE_ACCESS(vdac)
{
	struct vdac_data *d = (struct vdac_data *) extra;

	/*  Read from/write to the vdac:  */
	switch (relative_addr) {
	case DEV_VDAC_MAPWA:
		if (writeflag == MEM_WRITE) {
			d->cur_write_addr = data[0];
			d->sub_color = 0;
		} else {
			debug("[ vdac: read from MAPWA ]\n");
			data[0] = d->vdac_reg[relative_addr];
		}
		break;
	case DEV_VDAC_MAP:
		if (writeflag == MEM_WRITE) {
			d->cur_rgb[d->sub_color] = data[0];
			d->sub_color++;

			if (d->sub_color > 2) {
				/*  (Only update for color, not mono mode)  */
				if (d->color_fb_flag)
					memcpy(d->rgb_palette +
					    3*d->cur_write_addr, d->cur_rgb, 3);

				d->sub_color = 0;
				d->cur_write_addr ++;
			}
		} else {
			if (d->sub_color == 0) {
				memcpy(d->cur_rgb, d->rgb_palette +
				    3 * d->cur_read_addr, 3);
			}
			data[0] = d->cur_rgb[d->sub_color];
			d->sub_color++;
			if (d->sub_color > 2) {
				d->sub_color = 0;
				d->cur_read_addr ++;
			}
		}
		break;
	case DEV_VDAC_MAPRA:
		if (writeflag == MEM_WRITE) {
			d->cur_read_addr = data[0];
			d->sub_color = 0;
		} else {
			debug("[ vdac: read from MAPRA ]\n");
			data[0] = d->vdac_reg[relative_addr];
		}
		break;
	case DEV_VDAC_OVERWA:
		if (writeflag == MEM_WRITE) {
			d->cur_write_addr_overlay = data[0];
			d->sub_color_overlay = 0;
		} else {
			debug("[ vdac: read from OVERWA ]\n");
			data[0] = d->vdac_reg[relative_addr];
		}
		break;
	case DEV_VDAC_OVER:
		if (writeflag == MEM_WRITE) {
			d->cur_rgb_overlay[d->sub_color_overlay] = data[0];
			d->sub_color_overlay++;

			if (d->sub_color_overlay > 2) {
				/*  (Only update for color, not mono mode)  */
				if (d->color_fb_flag)
					memcpy(d->rgb_palette_overlay +
					    3 * d->cur_write_addr_overlay,
					    d->cur_rgb_overlay, 3);

				d->sub_color_overlay = 0;
				d->cur_write_addr_overlay ++;
				if (d->cur_write_addr_overlay > 15)
					d->cur_write_addr_overlay = 0;
			}
		} else {
			if (d->sub_color_overlay == 0) {
				memcpy(d->cur_rgb_overlay,
				    d->rgb_palette_overlay +
				    3 * d->cur_read_addr_overlay, 3);
			}
			data[0] = d->cur_rgb_overlay[d->sub_color_overlay];
			d->sub_color_overlay++;
			if (d->sub_color_overlay > 2) {
				d->sub_color_overlay = 0;
				d->cur_read_addr_overlay ++;
				if (d->cur_read_addr_overlay > 15)
					d->cur_read_addr_overlay = 0;
			}
		}
		break;
	case DEV_VDAC_OVERRA:
		if (writeflag == MEM_WRITE) {
			d->cur_read_addr_overlay = data[0];
			d->sub_color_overlay = 0;
		} else {
			debug("[ vdac: read from OVERRA ]\n");
			data[0] = d->vdac_reg[relative_addr];
		}
		break;
	default:
		if (writeflag == MEM_WRITE) {
			debug("[ vdac: unimplemented write to address 0x%x,"
			    " data=0x%02x ]\n", (int)relative_addr, data[0]);
			d->vdac_reg[relative_addr] = data[0];
		} else {
			debug("[ vdac: unimplemented read from address 0x%x"
			    " ]\n", (int)relative_addr);
			data[0] = d->vdac_reg[relative_addr];
		}
	}

	/*  Pretend it was ok:  */
	return 1;
}


/*
 *  dev_vdac_init():
 */
void dev_vdac_init(struct memory *mem, uint64_t baseaddr,
	unsigned char *rgb_palette, int color_fb_flag)
{
	struct vdac_data *d;

	CHECK_ALLOCATION(d = (struct vdac_data *) malloc(sizeof(struct vdac_data)));
	memset(d, 0, sizeof(struct vdac_data));

	d->rgb_palette   = rgb_palette;
	d->color_fb_flag = color_fb_flag;

	memory_device_register(mem, "vdac", baseaddr, DEV_VDAC_LENGTH,
	    dev_vdac_access, (void *)d, DM_DEFAULT, NULL);
}


/*
 *  dev_kn01_init():
 */
void dev_kn01_init(struct memory *mem, uint64_t baseaddr, int color_fb)
{
	struct kn01_data *d;

	CHECK_ALLOCATION(d = (struct kn01_data *) malloc(sizeof(struct kn01_data)));
	memset(d, 0, sizeof(struct kn01_data));

	d->color_fb = color_fb;
	d->csr = 0;
	d->csr |= (color_fb? 0 : KN01_CSR_MONO);

	memory_device_register(mem, "kn01", baseaddr,
	    DEV_KN01_LENGTH, dev_kn01_access, d, DM_DEFAULT, NULL);
}

