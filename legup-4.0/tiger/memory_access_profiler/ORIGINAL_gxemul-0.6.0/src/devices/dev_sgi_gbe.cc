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
 *  COMMENT: SGI "gbe", graphics controller + framebuffer
 *
 *  Loosely inspired by Linux code.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "console.h"
#include "cpu.h"
#include "devices.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"


/*  Let's hope nothing is there already...  */
#define	FAKE_GBE_FB_ADDRESS	0x38000000

#define	GBE_DEBUG
/*  #define debug fatal  */

#define MTE_TEST

#define	GBE_DEFAULT_XRES		640
#define	GBE_DEFAULT_YRES		480


struct sgi_gbe_data {
	int		xres, yres;

	uint32_t	control;		/* 0x00000  */
	uint32_t	dotclock;		/* 0x00004  */
	uint32_t	i2c;			/* 0x00008  */
	uint32_t	i2cfp;			/* 0x00010  */
	uint32_t	plane0ctrl;		/* 0x30000  */
	uint32_t	frm_control;		/* 0x3000c  */
	int		freeze;

	int		bitdepth;
	struct vfb_data *fb_data;
};


/*
 *  dev_sgi_gbe_tick():
 *
 *  Every now and then, copy data from the framebuffer in normal ram
 *  to the actual framebuffer (which will then redraw the window).
 *  TODO:  This is utterly slow, even slower than the normal framebuffer
 *  which is really slow as it is.
 *
 *  frm_control (bits 31..9) is a pointer to an array of uint16_t.
 *  These numbers (when << 16 bits) are pointers to the tiles. Tiles are
 *  512x128 in 8-bit mode, 256x128 in 16-bit mode, and 128x128 in 32-bit mode.
 */
DEVICE_TICK(sgi_gbe)
{
	struct sgi_gbe_data *d = (struct sgi_gbe_data *) extra;
	int tile_nr = 0, on_screen = 1, xbase = 0, ybase = 0;
	unsigned char tileptr_buf[sizeof(uint16_t)];
	uint64_t tileptr, tiletable;
	int lines_to_copy, pixels_per_line, y;
	unsigned char buf[16384];	/*  must be power of 2, at most 65536 */
	int copy_len, copy_offset;
	uint64_t old_fb_offset = 0;
	int tweaked = 1;

#ifdef MTE_TEST
/*  Actually just a return, but this fools the Compaq compiler...  */
if (cpu != NULL)
return;
#endif

	/*  debug("[ sgi_gbe: dev_sgi_gbe_tick() ]\n");  */

	tiletable = (d->frm_control & 0xfffffe00);
	if (tiletable == 0)
		on_screen = 0;
/*
tweaked = 0;
*/
	while (on_screen) {
		/*  Get pointer to a tile:  */
		cpu->memory_rw(cpu, cpu->mem, tiletable +
		    sizeof(tileptr_buf) * tile_nr,
		    tileptr_buf, sizeof(tileptr_buf), MEM_READ,
		    NO_EXCEPTIONS | PHYSICAL);
		tileptr = 256 * tileptr_buf[0] + tileptr_buf[1];
		/*  TODO: endianness  */
		tileptr <<= 16;

		/*  tileptr is now a physical address of a tile.  */
		debug("[ sgi_gbe:   tile_nr = %2i, tileptr = 0x%08lx, xbase"
		    " = %4i, ybase = %4i ]\n", tile_nr, tileptr, xbase, ybase);

		if (tweaked) {
			/*  Tweaked (linear) mode:  */

			/*
			 *  Copy data from this 64KB physical RAM block to the
			 *  framebuffer:
			 *
			 *  NOTE: Copy it in smaller chunks than 64KB, in case
			 *        the framebuffer device can optimize away
			 *        portions that aren't modified that way.
			 */
			copy_len = sizeof(buf);
			copy_offset = 0;

			while (on_screen && copy_offset < 65536) {
				if (old_fb_offset + copy_len > (uint64_t)
				    (d->xres * d->yres * d->bitdepth / 8)) {
					copy_len = d->xres * d->yres *
					    d->bitdepth / 8 - old_fb_offset;
					/*  Stop after copying this block...  */
					on_screen = 0;
				}

				/*  debug("old_fb_offset = %08x copylen"
				    "=%i\n", old_fb_offset, copy_len);  */

				cpu->memory_rw(cpu, cpu->mem, tileptr +
				    copy_offset, buf, copy_len, MEM_READ,
				    NO_EXCEPTIONS | PHYSICAL);
				dev_fb_access(cpu, cpu->mem, old_fb_offset,
				    buf, copy_len, MEM_WRITE, d->fb_data);
				copy_offset += sizeof(buf);
				old_fb_offset += sizeof(buf);
			}
		} else {
			/*  This is for non-tweaked (tiled) mode. Not really
			    tested with correct image data, but might work:  */

			lines_to_copy = 128;
			if (ybase + lines_to_copy > d->yres)
				lines_to_copy = d->yres - ybase;

			pixels_per_line = 512 * 8 / d->bitdepth;
			if (xbase + pixels_per_line > d->xres)
				pixels_per_line = d->xres - xbase;

			for (y=0; y<lines_to_copy; y++) {
				cpu->memory_rw(cpu, cpu->mem, tileptr + 512 * y,
				    buf, pixels_per_line * d->bitdepth / 8,
				    MEM_READ, NO_EXCEPTIONS | PHYSICAL);
#if 0
{
int i;
for (i=0; i<pixels_per_line * d->bitdepth / 8; i++)
	buf[i] ^= (random() & 0x20);
}
#endif
				dev_fb_access(cpu, cpu->mem, ((ybase + y) *
				    d->xres + xbase) * d->bitdepth / 8,
				    buf, pixels_per_line * d->bitdepth / 8,
				    MEM_WRITE, d->fb_data);
			}

			/*  Go to next tile:  */
			xbase += (512 * 8 / d->bitdepth);
			if (xbase >= d->xres) {
				xbase = 0;
				ybase += 128;
				if (ybase >= d->yres)
					on_screen = 0;
			}
		}

		/*  Go to next tile:  */
		tile_nr ++;
	}

	/*  debug("[ sgi_gbe: dev_sgi_gbe_tick() end]\n");  */
}


DEVICE_ACCESS(sgi_gbe)
{
	struct sgi_gbe_data *d = (struct sgi_gbe_data *) extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

#ifdef GBE_DEBUG
	if (writeflag == MEM_WRITE)
		debug("[ sgi_gbe: DEBUG: write to address 0x%llx, data"
		    "=0x%llx ]\n", (long long)relative_addr, (long long)idata);
#endif

	switch (relative_addr) {

	case 0x0:
		if (writeflag == MEM_WRITE)
			d->control = idata;
		else
			odata = d->control;
		break;

	case 0x4:
		if (writeflag == MEM_WRITE)
			d->dotclock = idata;
		else
			odata = d->dotclock;
		break;

	case 0x8:	/*  i2c?  */
		/*
		 *  "CRT I2C control".
		 *
		 *  I'm not sure what this does. It isn't really commented
		 *  in the linux sources.  The IP32 prom writes the values
		 *  0x03, 0x01, and then 0x00 to this address, and then
		 *  reads back a value.
		 */
		if (writeflag == MEM_WRITE) {
			d->i2c = idata;
		} else {
			odata = d->i2c;
			odata |= 1;	/*  ?  The IP32 prom wants this?  */
		}
		break;

	case 0x10:	/*  i2cfp, flat panel control  */
		if (writeflag == MEM_WRITE) {
			d->i2cfp = idata;
		} else {
			odata = d->i2cfp;
			odata |= 1;	/*  ?  The IP32 prom wants this?  */
		}
		break;

	case 0x10000:		/*  vt_xy, according to Linux  */
		if (writeflag == MEM_WRITE)
			d->freeze = idata & ((uint32_t)1<<31)? 1 : 0;
		else {
			/*  bit 31 = freeze, 23..12 = cury, 11.0 = curx  */
			odata = ((random() % (d->yres + 10)) << 12)
			    + (random() % (d->xres + 10)) +
			    (d->freeze? ((uint32_t)1 << 31) : 0);
odata = random();	/*  testhack for the ip32 prom  */
		}
		break;

	case 0x10004:		/*  vt_xymax, according to Linux  */
		odata = ((d->yres-1) << 12) + d->xres-1;
		/*  ... 12 bits maxy, 12 bits maxx.  */
		break;

	case 0x10034:		/*  vt_hpixen, according to Linux  */
		odata = (0 << 12) + d->xres-1;
		/*  ... 12 bits on, 12 bits off.  */
		break;

	case 0x10038:		/*  vt_vpixen, according to Linux  */
		odata = (0 << 12) + d->yres-1;
		/*  ... 12 bits on, 12 bits off.  */
		break;

	case 0x20004:
		odata = random();	/*  IP32 prom test hack. TODO  */
		/*  IRIX wants 0x20, it seems.  */
		if (random() & 1)
			odata = 0x20;
		break;

	case 0x30000:	/*  normal plane ctrl 0  */
		/*  bit 15 = fifo reset, 14..13 = depth, 
		    12..5 = tile width, 4..0 = rhs  */
		if (writeflag == MEM_WRITE) {
			d->plane0ctrl = idata;
			d->bitdepth = 8 << ((d->plane0ctrl >> 13) & 3);
			debug("[ sgi_gbe: setting color depth to %i bits ]\n",
			    d->bitdepth);
			if (d->bitdepth != 8)
				fatal("sgi_gbe: warning: bitdepth %i not "
				    "really implemented yet\n", d->bitdepth);
		} else
			odata = d->plane0ctrl;
		break;

	case 0x30008:	/*  normal plane ctrl 2  */
		odata = random();	/*  IP32 prom test hack. TODO  */
		/*  IRIX wants 0x20, it seems.  */
		if (random() & 1)
			odata = 0x20;
		break;

	case 0x3000c:	/*  normal plane ctrl 3  */
		/*
		 *  Writes to 3000c should be readable back at 30008?
		 *  At least bit 0 (dma) ctrl 3.
		 *
		 *  Bits 31..9 = tile table pointer bits,
		 *  Bit 1 = linear
		 *  Bit 0 = dma
		 */
		if (writeflag == MEM_WRITE) {
			d->frm_control = idata;
			debug("[ sgi_gbe: frm_control = 0x%08x ]\n",
			    d->frm_control);
		} else
			odata = d->frm_control;
		break;

	case 0x40000:
		odata = random();	/*  IP32 prom test hack. TODO  */
		/*  IRIX wants 0x20, it seems.  */
		if (random() & 1)
			odata = 0x20;
		break;

	/*
	 *  Linux/sgimips seems to write color palette data to offset 0x50000
	 *  to 0x503xx, and gamma correction data to 0x60000 - 0x603ff, as
	 *  32-bit values at addresses divisible by 4 (formated as 0xrrggbb00).
	 *
	 *  sgio2fb: initializing
	 *  sgio2fb: I/O at 0xffffffffb6000000
	 *  sgio2fb: tiles at ffffffffa2ef5000
	 *  sgio2fb: framebuffer at ffffffffa1000000
	 *  sgio2fb: 8192kB memory
	 *  Console: switching to colour frame buffer device 80x30
	 */

	default:
		/*  Gamma correction:  */
		if (relative_addr >= 0x60000 && relative_addr <= 0x603ff) {
			/*  ignore gamma correction for now  */
			break;
		}

		/*  RGB Palette:  */
		if (relative_addr >= 0x50000 && relative_addr <= 0x503ff) {
			int color_nr, r, g, b;
			int old_r, old_g, old_b;

			color_nr = (relative_addr & 0x3ff) / 4;
			r = (idata >> 24) & 0xff;
			g = (idata >> 16) & 0xff;
			b = (idata >>  8) & 0xff;

			old_r = d->fb_data->rgb_palette[color_nr * 3 + 0];
			old_g = d->fb_data->rgb_palette[color_nr * 3 + 1];
			old_b = d->fb_data->rgb_palette[color_nr * 3 + 2];

			d->fb_data->rgb_palette[color_nr * 3 + 0] = r;
			d->fb_data->rgb_palette[color_nr * 3 + 1] = g;
			d->fb_data->rgb_palette[color_nr * 3 + 2] = b;

			if (r != old_r || g != old_g || b != old_b) {
				/*  If the palette has been changed, the entire
				    image needs to be redrawn...  :-/  */
				d->fb_data->update_x1 = 0;
				d->fb_data->update_x2 = d->fb_data->xsize - 1;
				d->fb_data->update_y1 = 0;
				d->fb_data->update_y2 = d->fb_data->ysize - 1;
			}
			break;
		}

		if (writeflag == MEM_WRITE)
			debug("[ sgi_gbe: unimplemented write to address "
			    "0x%llx, data=0x%llx ]\n",
			    (long long)relative_addr, (long long)idata);
		else
			debug("[ sgi_gbe: unimplemented read from address "
			    "0x%llx ]\n", (long long)relative_addr);
	}

	if (writeflag == MEM_READ) {
#ifdef GBE_DEBUG
		debug("[ sgi_gbe: DEBUG: read from address 0x%llx: 0x%llx ]\n",
		    (long long)relative_addr, (long long)odata);
#endif
		memory_writemax64(cpu, data, len, odata);
	}

	return 1;
}


void dev_sgi_gbe_init(struct machine *machine, struct memory *mem,
	uint64_t baseaddr)
{
	struct sgi_gbe_data *d;

	CHECK_ALLOCATION(d = (struct sgi_gbe_data *) malloc(sizeof(struct sgi_gbe_data)));
	memset(d, 0, sizeof(struct sgi_gbe_data));

	/*  640x480 for Linux:  */
	d->xres = GBE_DEFAULT_XRES;
	d->yres = GBE_DEFAULT_YRES;
	d->bitdepth = 8;
	d->control = 0x20aa000;		/*  or 0x00000001?  */

	/*  1280x1024 for booting the O2's PROM:  */
	d->xres = 1280; d->yres = 1024;

	d->fb_data = dev_fb_init(machine, mem, FAKE_GBE_FB_ADDRESS,
	    VFB_GENERIC, d->xres, d->yres, d->xres, d->yres, 8, "SGI GBE");
	set_grayscale_palette(d->fb_data, 256);

	memory_device_register(mem, "sgi_gbe", baseaddr, DEV_SGI_GBE_LENGTH,
	    dev_sgi_gbe_access, d, DM_DEFAULT, NULL);
	machine_add_tickfunction(machine, dev_sgi_gbe_tick, d, 18);
}

