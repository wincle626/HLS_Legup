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
 *  COMMENT: Integraphics Systems "igsfb" Framebuffer graphics card
 *
 *  Used in at least the NetWinder.
 *
 *  TODO:  This is hardcoded to 1024x768x8 right now, and only supports the
 *         two acceleration commands used by NetBSD for scrolling the
 *         framebuffer. The cursor is hardcoded to 12x22 pixels, as that is
 *         what NetBSD/netwinder uses.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "console.h"
#include "device.h"
#include "devices.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"
#include "vga.h"

#include "thirdparty/igsfbreg.h"


struct dev_igsfb_data {
	int		xres;
	int		yres;
	int		bitdepth;
	struct vfb_data *vfb_data;

	/*  VGA palette stuff:  */
	int		palette_write_index;
	int		palette_write_subindex;

	/*
	 *  Various graphics controller registers. See igsfbreg.h for a
	 *  brief explanation of what these do.
	 */
	int		src_map_width;
	int		src2_map_width;
	int		dst_map_width;
	int		src_start;
	int		src2_start;
	int		dst_start;
	int		map_fmt;
	int		ctl;
	int		fg_mix;
	int		bg_mix;
	int		width;
	int		height;
	int		fg;
	int		bg;
	int		pixel_op_0;
	int		pixel_op_1;
	int		pixel_op_2;
	int		pixel_op_3;

	uint8_t		ext_reg_select;
	uint8_t		ext_reg[256];
};


/*
 *  recalc_sprite_position():
 *
 *  TODO: This is hardcoded for NetBSD/netwinder's 12x22 pixel cursor.
 */
static void recalc_sprite_position(struct dev_igsfb_data *d)
{
	int x = d->ext_reg[IGS_EXT_SPRITE_HSTART_LO] +
		d->ext_reg[IGS_EXT_SPRITE_HSTART_HI] * 256;
	int y = d->ext_reg[IGS_EXT_SPRITE_VSTART_LO] +
		d->ext_reg[IGS_EXT_SPRITE_VSTART_HI] * 256;

	dev_fb_setcursor(d->vfb_data, x, y, 1, 12, 22);
}


/*
 *  dev_igsfb_op3_written():
 *
 *  This function is called after the pixel_op_3 register has been written to.
 *  I guess this is what triggers accelerated functions to start executing.
 *
 *  NOTE/TODO: Only those necessary to run NetBSD/netwinder have been
 *             implemented.
 */
static void dev_igsfb_op3_written(struct dev_igsfb_data *d)
{
	if (d->pixel_op_0 == 0x00 && d->pixel_op_1 == 0x80 &&
	    d->pixel_op_2 == 0x00 && d->pixel_op_3 == 0x28 &&
	    d->fg_mix == 0x03 && d->ctl == 0x00) {
		/*  NetBSD scroll-up  */
		framebuffer_blockcopyfill(d->vfb_data, 0, 0,0,0,
		    d->dst_start % d->xres, d->dst_start / d->xres,
		    d->dst_start % d->xres + d->width,
		    d->dst_start / d->xres + d->height,
		    d->src_start % d->xres, d->src_start / d->xres);
		return;
	}

	if (d->pixel_op_0 == 0x00 && d->pixel_op_1 == 0x80 &&
	    d->pixel_op_2 == 0x00 && d->pixel_op_3 == 0x08 &&
	    d->fg_mix == 0x03 && d->ctl == 0x00) {
		/*  NetBSD fill  */
		/*  TODO: Color!  */
		framebuffer_blockcopyfill(d->vfb_data, 1, 0,0,0,
		    d->dst_start % d->xres, d->dst_start / d->xres,
		    d->dst_start % d->xres + d->width,
		    d->dst_start / d->xres + d->height,
		    0, 0);
		return;
	}

	fatal("\nUnimplemented igsfb accelerated framebuffer command:\n");
	fatal("pixel_op_0 = 0x%02x\n", d->pixel_op_0);
	fatal("pixel_op_1 = 0x%02x\n", d->pixel_op_1);
	fatal("pixel_op_2 = 0x%02x\n", d->pixel_op_2);
	fatal("pixel_op_3 = 0x%02x\n", d->pixel_op_3);
	fatal("fg_mix     = 0x%02x\n", d->fg_mix);
	fatal("ctl        = 0x%02x\n", d->ctl);
	fatal("src_start  = 0x%x\n", d->src_start);
	fatal("dst_start  = 0x%x\n", d->dst_start);
	fatal("width      = %i\n", d->width);
	fatal("height     = %i\n", d->height);
	exit(1);
}


DEVICE_ACCESS(igsfb)
{
	struct dev_igsfb_data *d = (struct dev_igsfb_data *) extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	if (relative_addr >= 0x3c0 && relative_addr <= 0x3df) {
		switch (relative_addr - 0x3c0) {
		case VGA_DAC_ADDR_WRITE:		/*  0x08  */
			if (writeflag == MEM_WRITE) {
				d->palette_write_index = idata;
				d->palette_write_subindex = 0;
			} else {
				fatal("[ igsdb: WARNING: Read from "
				    "VGA_DAC_ADDR_WRITE? ]\n");
				odata = d->palette_write_index;
			}
			break;
		case VGA_DAC_DATA:			/*  0x09  */
			if (writeflag == MEM_WRITE) {
				/*  Note: 8-bit color, not 6, so it isn't
				    exactly like normal VGA palette:  */
				int new_ = idata & 0xff;
				int old = d->vfb_data->rgb_palette[d->
				    palette_write_index*3+d->
				    palette_write_subindex];
				d->vfb_data->rgb_palette[d->palette_write_index
				    * 3 + d->palette_write_subindex] = new_;
				/*  Redraw whole screen, if the
				    palette changed:  */
				if (new_ != old) {
					d->vfb_data->update_x1 =
					    d->vfb_data->update_y1 = 0;
					d->vfb_data->update_x2 = d->xres - 1;
					d->vfb_data->update_y2 = d->yres - 1;
				}
				d->palette_write_subindex ++;
				if (d->palette_write_subindex == 3) {
					d->palette_write_index ++;
					d->palette_write_subindex = 0;
				}
			}
			/*  Note/TODO: Reading from the palette isn't
			    implemented here.  */
			break;
		case 0xe:	/*  IGSFB extended register select  */
			if (writeflag == MEM_WRITE)
				d->ext_reg_select = idata;
			else
				odata = d->ext_reg_select;
			break;
		case 0xf:	/*  IGSFB extended register data  */
			if (writeflag == MEM_READ)
				odata = d->ext_reg[d->ext_reg_select];
			else {
				d->ext_reg[d->ext_reg_select] = idata;
				switch (d->ext_reg_select) {
				/*  case IGS_EXT_SPRITE_HSTART_LO:
				case IGS_EXT_SPRITE_HSTART_HI:
				case IGS_EXT_SPRITE_VSTART_LO:  */
				case IGS_EXT_SPRITE_VSTART_HI:
					recalc_sprite_position(d);
					break;
				}
			}
			break;
		}
		return 1;
	}

	if (relative_addr >= IGS_COP_BASE_A &&
	    relative_addr < IGS_COP_BASE_A + IGS_COP_SIZE) {
		fatal("[ igsfb: BASE A not implemented yet, only BASE B ]\n");
		exit(1);
	}

	switch (relative_addr) {

	case IGS_VDO:
		if (writeflag == MEM_WRITE) {
			if (idata & ~(IGS_VDO_ENABLE | IGS_VDO_SETUP)) {
				fatal("[ igsfb: Unimplemented IGS_VDO flags:"
				    " 0x%08x ]\n", (int)idata);
				exit(1);
			}
		}
		break;

	case IGS_VSE:
		if (writeflag == MEM_WRITE) {
			if (idata & ~(IGS_VSE_ENABLE)) {
				fatal("[ igsfb: Unimplemented IGS_VSE flags:"
				    " 0x%08x ]\n", (int)idata);
				exit(1);
			}
		}
		break;

	case IGS_COP_BASE_B + IGS_COP_SRC_MAP_WIDTH_REG:
		if (writeflag == MEM_WRITE)
			d->src_map_width = idata & 0x3ff;
		else
			odata = d->src_map_width;
		break;

	case IGS_COP_BASE_B + IGS_COP_SRC2_MAP_WIDTH_REG:
		if (writeflag == MEM_WRITE)
			d->src2_map_width = idata & 0x3ff;
		else
			odata = d->src2_map_width;
		break;

	case IGS_COP_BASE_B + IGS_COP_DST_MAP_WIDTH_REG:
		if (writeflag == MEM_WRITE)
			d->dst_map_width = idata & 0x3ff;
		else
			odata = d->dst_map_width;
		break;

	case IGS_COP_BASE_B + IGS_COP_MAP_FMT_REG:
		if (writeflag == MEM_WRITE)
			d->map_fmt = idata;
		else
			odata = d->map_fmt;
		break;

	case IGS_COP_BASE_B + IGS_COP_CTL_REG:
		if (writeflag == MEM_WRITE)
			d->ctl = idata;
		else
			odata = d->ctl;
		break;

	case IGS_COP_BASE_B + IGS_COP_FG_MIX_REG:
		if (writeflag == MEM_WRITE)
			d->fg_mix = idata;
		else
			odata = d->fg_mix;
		break;

	case IGS_COP_BASE_B + IGS_COP_BG_MIX_REG:
		if (writeflag == MEM_WRITE)
			d->bg_mix = idata;
		else
			odata = d->bg_mix;
		break;

	case IGS_COP_BASE_B + IGS_COP_WIDTH_REG:
		if (writeflag == MEM_WRITE)
			d->width = idata & 0x3ff;
		else
			odata = d->width;
		break;

	case IGS_COP_BASE_B + IGS_COP_HEIGHT_REG:
		if (writeflag == MEM_WRITE)
			d->height = idata & 0x3ff;
		else
			odata = d->height;
		break;

	case IGS_COP_BASE_B + IGS_COP_SRC_START_REG:
		if (writeflag == MEM_WRITE)
			d->src_start = idata & 0x3fffff;
		else
			odata = d->src_start;
		break;

	case IGS_COP_BASE_B + IGS_COP_SRC2_START_REG:
		if (writeflag == MEM_WRITE)
			d->src2_start = idata & 0x3fffff;
		else
			odata = d->src2_start;
		break;

	case IGS_COP_BASE_B + IGS_COP_DST_START_REG:
		if (writeflag == MEM_WRITE)
			d->dst_start = idata & 0x3fffff;
		else
			odata = d->dst_start;
		break;

	case IGS_COP_BASE_B + IGS_COP_FG_REG:
		if (writeflag == MEM_WRITE)
			d->fg = idata;
		else
			odata = d->fg;
		break;

	case IGS_COP_BASE_B + IGS_COP_BG_REG:
		if (writeflag == MEM_WRITE)
			d->bg = idata;
		else
			odata = d->bg;
		break;

	case IGS_COP_BASE_B + IGS_COP_PIXEL_OP_0_REG:
		if (writeflag == MEM_WRITE)
			d->pixel_op_0 = idata;
		else
			odata = d->pixel_op_0;
		break;

	case IGS_COP_BASE_B + IGS_COP_PIXEL_OP_1_REG:
		if (writeflag == MEM_WRITE)
			d->pixel_op_1 = idata;
		else
			odata = d->pixel_op_1;
		break;

	case IGS_COP_BASE_B + IGS_COP_PIXEL_OP_2_REG:
		if (writeflag == MEM_WRITE)
			d->pixel_op_2 = idata;
		else
			odata = d->pixel_op_2;
		break;

	case IGS_COP_BASE_B + IGS_COP_PIXEL_OP_3_REG:
		if (writeflag == MEM_WRITE) {
			d->pixel_op_3 = idata;
			dev_igsfb_op3_written(d);
		} else {
			odata = d->pixel_op_3;
		}
		break;

	default:if (writeflag == MEM_WRITE) {
			fatal("[ igsfb: unimplemented write to address 0x%x"
			    " data=0x%02x ]\n", (int)relative_addr, (int)idata);
		} else {
			fatal("[ igsfb: unimplemented read from address 0x%x "
			    "]\n", (int)relative_addr);
		}
		exit(1);
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(igsfb)
{
	struct dev_igsfb_data *d;

	CHECK_ALLOCATION(d = (struct dev_igsfb_data *) malloc(sizeof(struct dev_igsfb_data)));
	memset(d, 0, sizeof(struct dev_igsfb_data));

	d->xres = 1024;
	d->yres = 768;
	d->bitdepth = 8;
	d->vfb_data = dev_fb_init(devinit->machine, devinit->machine->memory,
	    0x400000 + devinit->addr, VFB_GENERIC, d->xres, d->yres,
            d->xres, d->yres, d->bitdepth, "igsfb");

        /*  TODO: Palette control etc at 0x3c0 + IGS_MEM_MMIO_SELECT  */

	memory_device_register(devinit->machine->memory, devinit->name,
	    devinit->addr + IGS_MEM_MMIO_SELECT, 0x100000,
	    dev_igsfb_access, d, DM_DEFAULT, NULL);

	return 1;
}

