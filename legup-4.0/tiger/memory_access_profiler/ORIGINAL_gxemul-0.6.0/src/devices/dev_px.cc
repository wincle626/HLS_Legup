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
 *  COMMENT: TURBOchannel Pixelstamp graphics card
 *
 *	PMAG-CA = PX
 *	PMAG-DA = PXG
 *	PMAG-EA = PXG+
 *	PMAG-FA = PXG+ TURBO
 *
 *  See include/pxreg.h (and NetBSD's arch/pmax/dev/px.c) for more information.
 *
 *  The emulation of this device is far from complete. Different pixelstamp
 *  boards are recognizes under different names depending on operating system:
 *
 *	NetBSD/pmax:  (works fine both with and without console on framebuffer)
 *		PMAG-CA:	px0 at tc0 slot 0 offset 0x0: 2D, 4x1 stamp,
 *				  8 plane
 *		PMAG-DA:	px0 at tc0 slot 0 offset 0x0: 3D, 4x1 stamp,
 *				  8 plane, 128KB SRAM
 *		PMAG-EA:	(not supported)
 *		PMAG-FA:	px0 at tc0 slot 0 offset 0x0: 3D, 5x2 stamp,
 *				  24 plane, 128KB SRAM
 *
 *	Ultrix 4.2A rev 47:  (usually crashes if the device is installed, but
 *						serial console is used)
 *		PMAG-CA:	px0 at ibus0, pa0 (5x1 8+8+0+0)
 *		PMAG-DA:	px0 at ibus0, pq0 (5x1 16+16+16+0 128KB)
 *				    or (5x1 0+0+16+0 128KB)
 *		PMAG-EA:	(not supported)
 *		PMAG-FA:	px0 at ibus0, pq0 (5x2 24+24+16+16 128KB)
 *
 *	Ultrix 4.2 rev 85:  (usually crashes if the device is installed,
 *					 but serial console is used)
 *		PMAG-CA:	ga0 at ibus0, ga0 ( 8 planes 4x1 stamp )
 *		PMAG-DA:	gq0 at ibus0, gq0 ( 8+8+16Z+0X plane 4x1 stamp )
 *		PMAG-EA:	(not supported)
 *		PMAG-FA:	gq0 at ibus0, gq0 ( 24+24+24Z+24X plane
 *				 5x2 stamp )  (crashes in serial console mode)
 *
 *  TODO:  A lot of stuff:
 *
 *	Read http://www.mit.edu/afs/athena/system/pmax_ul3/srvd.73/sys/
 *		io/tc/gq.h
 *	and try to figure out the interrupt and memory management stuff.
 *
 *	Color support: foreground, background, 8-bit palette?
 *	2D and 3D stuff: polygons? shading?
 *	Don't use so many hardcoded values.
 *	Actually interpret the values in each command, don't just
 *		assume NetBSD/Ultrix usage.
 *	Factor out the DMA read (main memory vs sram).
 *	Interrupts?
 *	Make sure that everything works with both NetBSD and Ultrix.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "devices.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/pxreg.h"

#define	PX_XSIZE	1280
#define	PX_YSIZE	1024

/* #define PX_DEBUG  */


DEVICE_TICK(px)
{
#if 0
	struct px_data *d = extra;

	if (d->intr & STIC_INT_P_EN)		/*  or _WE ?  */
		INTERRUPT_ASSERT(d->irq);
#endif
}


/*
 *  px_readword():
 *
 *  Helper function to read 32-bit words from DMA memory,
 *  to allow both little and big endian accesses.
 *  (DECstations probably only use little endian access,
 *  but endianness-independance is probably nice to have anyway.)
 */
uint32_t px_readword(struct cpu *cpu, unsigned char *dma_buf, int ofs)
{
	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		return dma_buf[ofs+0] + (dma_buf[ofs+1] << 8) +
		    (dma_buf[ofs+2] << 16) + (dma_buf[ofs+3] << 24);
	else
		return dma_buf[ofs+3] + (dma_buf[ofs+2] << 8) +
		    (dma_buf[ofs+1] << 16) + (dma_buf[ofs+0] << 24);
}


/*
 *  dev_px_dma():
 *
 *  This routine performs a (fake) DMA transfer of STAMP commands
 *  and executes them.
 *
 *  For the "PX" board, read from main memory (cpu->mem). For all other
 *  boards, read from the i860 SRAM portion of the device (d->sram).
 */
void dev_px_dma(struct cpu *cpu, uint32_t sys_addr, struct px_data *d)
{
	unsigned char dma_buf[32768];
	size_t dma_len = sizeof(dma_buf);
	int bytesperpixel;
	uint32_t cmdword;

	bytesperpixel = d->bitdepth >> 3;

	dma_len = 56 * 4;	/*  TODO: this is just enough for NetBSD's
					 putchar  */

	if (d->type == DEV_PX_TYPE_PX) {
		cpu->memory_rw(cpu, cpu->mem, sys_addr, dma_buf,
		    dma_len, MEM_READ, NO_EXCEPTIONS | PHYSICAL);
	} else {
		/*  TODO:  past end of sram?  */
		memmove(dma_buf, &d->sram[sys_addr & 0x1ffff], dma_len);
	}

	if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
		cmdword = dma_buf[0] + (dma_buf[1] << 8) +
		    (dma_buf[2] << 16) + (dma_buf[3] << 24);
	else
		cmdword = dma_buf[3] + (dma_buf[2] << 8) +
		    (dma_buf[1] << 16) + (dma_buf[0] << 24);

#ifdef PX_DEBUG
	debug("[ px: dma from 0x%08x: ", (int)sys_addr);

	debug("cmd=");
	switch (cmdword & 0xf) {
	case STAMP_CMD_POINTS:		debug("points");	break;
	case STAMP_CMD_LINES:		debug("lines");		break;
	case STAMP_CMD_TRIANGLES:	debug("triangles");	break;
	case STAMP_CMD_COPYSPANS:	debug("copyspans");	break;
	case STAMP_CMD_READSPANS:	debug("readspans");	break;
	case STAMP_CMD_WRITESPANS:	debug("writespans");	break;
	case STAMP_CMD_VIDEO:		debug("video");		break;
	default:
		debug("0x%x (?)", cmdword & 0xf);
	}

	debug(",rgb=");
	switch (cmdword & 0x30) {
	case STAMP_RGB_NONE:	debug("none");		break;
	case STAMP_RGB_CONST:	debug("const");		break;
	case STAMP_RGB_FLAT:	debug("flat");		break;
	case STAMP_RGB_SMOOTH:	debug("smooth");	break;
	default:
		debug("0x%x (?)", cmdword & 0x30);
	}

	debug(",z=");
	switch (cmdword & 0xc0) {
	case STAMP_Z_NONE:	debug("none");		break;
	case STAMP_Z_CONST:	debug("const");		break;
	case STAMP_Z_FLAT:	debug("flat");		break;
	case STAMP_Z_SMOOTH:	debug("smooth");	break;
	default:
		debug("0x%x (?)", cmdword & 0xc0);
	}

	debug(",xy=");
	switch (cmdword & 0x300) {
	case STAMP_XY_NONE:		debug("none");		break;
	case STAMP_XY_PERPACKET:	debug("perpacket");	break;
	case STAMP_XY_PERPRIMATIVE:	debug("perprimative");	break;
	default:
		debug("0x%x (?)", cmdword & 0x300);
	}

	debug(",lw=");
	switch (cmdword & 0xc00) {
	case STAMP_LW_NONE:		debug("none");		break;
	case STAMP_LW_PERPACKET:	debug("perpacket");	break;
	case STAMP_LW_PERPRIMATIVE:	debug("perprimative");	break;
	default:
		debug("0x%x (?)", cmdword & 0xc00);
	}

	if (cmdword & STAMP_CLIPRECT)
		debug(",CLIPRECT");
	if (cmdword & STAMP_MESH)
		debug(",MESH");
	if (cmdword & STAMP_AALINE)
		debug(",AALINE");
	if (cmdword & STAMP_HS_EQUALS)
		debug(",HS_EQUALS");

	{
		size_t i;
		for (i=0; i<dma_len; i++)
			debug(" %02x", dma_buf[i]);
	}

	debug(" ]\n");
#endif	/*  PX_DEBUG  */

	/*  NetBSD and Ultrix copyspans  */
	if (cmdword == 0x405) {
		uint32_t nspans, lw;
		uint32_t spannr, ofs;
		uint32_t span_len, span_src, span_dst;
		/*  unsigned char pixels[PX_XSIZE * 3];  */

		if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
			nspans = dma_buf[4] + (dma_buf[5] << 8) +
			    (dma_buf[6] << 16) + (dma_buf[7] << 24);
		else
			nspans = dma_buf[7] + (dma_buf[6] << 8) +
			    (dma_buf[5] << 16) + (dma_buf[4] << 24);

		if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
			lw = dma_buf[16] + (dma_buf[17] << 8) +
			    (dma_buf[18] << 16) + (dma_buf[19] << 24);
		else
			lw = dma_buf[19] + (dma_buf[18] << 8) +
			    (dma_buf[17] << 16) + (dma_buf[16] << 24);

		nspans >>= 24;
		/*  Why not this?  lw = (lw + 1) >> 2;  */

#ifdef PX_DEBUG
		debug("[ px: copyspans:  nspans = %i, lw = %i ]\n", nspans, lw);
#endif

		/*  Reread copyspans command if it wasn't completely read:  */
		if (dma_len < 4*(5 + nspans*3)) {
			dma_len = 4 * (5+nspans*3);
			if (d->type == DEV_PX_TYPE_PX)
				cpu->memory_rw(cpu, cpu->mem, sys_addr,
				    dma_buf, dma_len, MEM_READ,
				    NO_EXCEPTIONS | PHYSICAL);
			else
				memmove(dma_buf, &d->sram[sys_addr & 0x1ffff],
				    dma_len);	/*  TODO:  past end of sram?  */
		}

		ofs = 4*5;
		for (spannr=0; spannr<nspans; spannr++) {
			if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
				span_len = dma_buf[ofs+0] + (dma_buf[ofs+1] <<
				    8) + (dma_buf[ofs+2] << 16) +
				    (dma_buf[ofs+3] << 24);
			else
				span_len = dma_buf[ofs+3] + (dma_buf[ofs+2] <<
				    8) + (dma_buf[ofs+1] << 16) +
				    (dma_buf[ofs+0] << 24);
			ofs += 4;

			if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
				span_src = dma_buf[ofs+0] + (dma_buf[ofs+1] <<
				    8) + (dma_buf[ofs+2] << 16) +
				    (dma_buf[ofs+3] << 24);
			else
				span_src = dma_buf[ofs+3] + (dma_buf[ofs+2] <<
				    8) + (dma_buf[ofs+1] << 16) +
				    (dma_buf[ofs+0] << 24);
			ofs += 4;

			if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
				span_dst = dma_buf[ofs+0] + (dma_buf[ofs+1] <<
				    8) + (dma_buf[ofs+2] << 16) +
				    (dma_buf[ofs+3] << 24);
			else
				span_dst = dma_buf[ofs+3] + (dma_buf[ofs+2] <<
				    8) + (dma_buf[ofs+1] << 16) +
				    (dma_buf[ofs+0] << 24);
			ofs += 4;

			span_len >>= 3;
			span_dst >>= 3;
			span_src >>= 3;

			if (span_len > PX_XSIZE)
				span_len = PX_XSIZE;

			/*  debug("  span %i: len=%i src=%i dst=%i\n",
			    spannr, span_len, span_src, span_dst);  */

			memmove(d->vfb_data->framebuffer + span_dst *
			    PX_XSIZE * bytesperpixel, d->vfb_data->framebuffer
			    + span_src * PX_XSIZE * bytesperpixel, span_len *
			    bytesperpixel);

			d->vfb_data->update_x1 = 0; d->vfb_data->update_x2 =
			    PX_XSIZE-1;
			if ((int32_t)span_dst < d->vfb_data->update_y1)
				d->vfb_data->update_y1 = span_dst;
			if ((int32_t)span_dst > d->vfb_data->update_y2)
				d->vfb_data->update_y2 = span_dst;
			if ((int32_t)span_src < d->vfb_data->update_y1)
				d->vfb_data->update_y1 = span_src;
			if ((int32_t)span_src > d->vfb_data->update_y2)
				d->vfb_data->update_y2 = span_src;
		}
	}

	/*  NetBSD and Ultrix erasecols/eraserows  */
	if (cmdword == 0x411) {
		uint32_t v1, v2, attr;
		int32_t lw;
		int x,y,x2,y2;
		int fb_y;
		int bg_r, bg_g, bg_b;
		unsigned char pixels[PX_XSIZE * 3];

		lw = px_readword(cpu, dma_buf, 16);
		attr = px_readword(cpu, dma_buf, 20);
		v1 = px_readword(cpu, dma_buf, 24);
		v2 = px_readword(cpu, dma_buf, 28);
#if 0
		if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
			lw = dma_buf[16] + (dma_buf[17] << 8) +
			    (dma_buf[18] << 16) + (dma_buf[19] << 24);
		else
			lw = dma_buf[19] + (dma_buf[18] << 8) +
			    (dma_buf[17] << 16) + (dma_buf[16] << 24);

		if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
			v1 = dma_buf[24] + (dma_buf[25] << 8) +
			    (dma_buf[26] << 16) + (dma_buf[27] << 24);
		else
			v1 = dma_buf[27] + (dma_buf[26] << 8) +
			    (dma_buf[25] << 16) + (dma_buf[24] << 24);

		if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
			v2 = dma_buf[28] + (dma_buf[29] << 8) +
			    (dma_buf[30] << 16) + (dma_buf[31] << 24);
		else
			v2 = dma_buf[31] + (dma_buf[30] << 8) +
			    (dma_buf[29] << 16) + (dma_buf[28] << 24);
#endif
		bg_r = (attr >> 16) & 255;
		bg_g = (attr >> 8) & 255;
		bg_b = attr & 255;
		if (bg_r == 0)
			bg_r = bg_g = bg_b = 0;
		else
		if (bg_r == 7)
			bg_r = bg_g = bg_b = 192;
		else
			bg_r = bg_g = bg_b = 255;

		v1 -= lw;
		v2 -= lw;

		x = (v1 >> 19) & 2047;
		y = (v1 >> 3) & 1023;
		x2 = (v2 >> 19) & 2047;
		y2 = (v2 >> 3) & 1023;

		lw = (lw + 1) >> 2;

		if (x2 - x > PX_XSIZE)
			x2 = PX_XSIZE;

#ifdef PX_DEBUG
		debug("[ px: clear/fill: v1 = 0x%08x  v2 = 0x%08x "
		    "lw=%i x=%i y=%i x2=%i y2=%i ]\n", (int)v1, (int)v2,
		    lw, x,y, x2,y2);
#endif
		if (bytesperpixel == 3) {
			int xi;
			for (xi=0; xi<x2-x; xi++) {
				/*  TODO:  rgb order?  */
				pixels[xi*3 + 0] = bg_r;
				pixels[xi*3 + 1] = bg_g;
				pixels[xi*3 + 2] = bg_b;
			}
		} else
			memset(pixels, attr, (x2 - x) * bytesperpixel);

		if (x < d->vfb_data->update_x1)
			d->vfb_data->update_x1 = x;
		if (x2 > d->vfb_data->update_x2)
			d->vfb_data->update_x2 = x2;

		for (fb_y=y; fb_y < y2 + lw; fb_y ++) {
			memcpy(d->vfb_data->framebuffer + (fb_y * PX_XSIZE + x)
			    * bytesperpixel, pixels, (x2-x)*bytesperpixel);

			if (fb_y < d->vfb_data->update_y1)
				d->vfb_data->update_y1 = fb_y;
			if (fb_y > d->vfb_data->update_y2)
				d->vfb_data->update_y2 = fb_y;
		}
	}

	/*  NetBSD and Ultrix putchar  */
	if (cmdword == 0xa21) {
		/*  Ugly test code:  */
		unsigned char pixels[16 * 3];
		int pixels_len = 16;
		uint32_t v1, v2, fgcolor, bgcolor;
		int x, y, x2,y2, i, maxi;
		int xbit;
		int suby;
		int fg_r, fg_g, fg_b;
		int bg_r, bg_g, bg_b;

		v1 = px_readword(cpu, dma_buf, 52);
		v2 = px_readword(cpu, dma_buf, 56);
		fgcolor = px_readword(cpu, dma_buf, 16 * 4);
		bgcolor = px_readword(cpu, dma_buf, 29 * 4);

		/*
		 *  TODO:  Which one is r, which one is g, and which one is b?
		 *  TODO 2:  Use the BT459 palette, these values are hardcoded
		 *  for NetBSD and Ultrix grayscale only.
		 */
		fg_r = (fgcolor >> 16) & 255;
		fg_g = (fgcolor >> 8) & 255;
		fg_b = fgcolor & 255;
		if (fg_r == 0)
			fg_r = fg_g = fg_b = 0;
		else
		if (fg_r == 7)
			fg_r = fg_g = fg_b = 192;
		else
			fg_r = fg_g = fg_b = 255;

		bg_r = (bgcolor >> 16) & 255;
		bg_g = (bgcolor >> 8) & 255;
		bg_b = bgcolor & 255;
		if (bg_r == 0)
			bg_r = bg_g = bg_b = 0;
		else
		if (bg_r == 7)
			bg_r = bg_g = bg_b = 192;
		else
			bg_r = bg_g = bg_b = 255;

		x = (v1 >> 19) & 2047;
		y = ((v1 - 63) >> 3) & 1023;
		x2 = (v2 >> 19) & 2047;
		y2 = ((v2 - 63) >> 3) & 1023;

#ifdef PX_DEBUG
		debug("[ px putchar: v1 = 0x%08x  v2 = 0x%08x x=%i y=%i ]\n",
		    (int)v1, (int)v2, x,y, x2,y2);
#endif
		x %= PX_XSIZE;
		y %= PX_YSIZE;
		x2 %= PX_XSIZE;
		y2 %= PX_YSIZE;

		pixels_len = x2 - x;

		suby = 0;
		maxi = 12;
		maxi = 33;

		for (i=4; i<maxi; i++) {
			int j;

			if (i == 12)
				i = 30;

			for (j=0; j<2; j++) {
				for (xbit = 0; xbit < 8; xbit ++) {
					if (bytesperpixel == 3) {
						/*  24-bit:  */
						/*  TODO:  Which one is r,
						    which one is g, and b?  */
						pixels[xbit * 3 + 0] =
						    (dma_buf[i*4 + j*2 + 0] &
						    (1 << xbit))? fg_r : bg_r;
						pixels[xbit * 3 + 1] =
						    (dma_buf[i*4 + j*2 + 0] &
						    (1 << xbit))? fg_g : bg_g;
						pixels[xbit * 3 + 2] =
						    (dma_buf[i*4 + j*2 + 0] &
						    (1 << xbit))? fg_b : bg_b;
						pixels[(xbit + 8) * 3 + 0] =
						    (dma_buf[i*4 + j*2 + 1] &
						    (1 << xbit))? fg_r : bg_r;
						pixels[(xbit + 8) * 3 + 1] =
						    (dma_buf[i*4 + j*2 + 1] &
						    (1 << xbit))? fg_g : bg_g;
						pixels[(xbit + 8) * 3 + 2] =
						    (dma_buf[i*4 + j*2 + 1] &
						    (1 << xbit))? fg_b : bg_b;
					} else {
						/*  8-bit:  */
						pixels[xbit] = (dma_buf[i*4 +
						    j*2 + 0] & (1 << xbit))?
						    (fgcolor & 255) :
						    (bgcolor & 255);
						pixels[xbit + 8] = (dma_buf[i*4
						    + j*2 + 1] & (1 << xbit))?
						    (fgcolor & 255) :
						    (bgcolor & 255);
					}
				}

				memcpy(d->vfb_data->framebuffer + ((y+suby)
				    * PX_XSIZE + x) * bytesperpixel,
				    pixels, pixels_len * bytesperpixel);

				if (y+suby < d->vfb_data->update_y1)
					d->vfb_data->update_y1 = y+suby;
				if (y+suby > d->vfb_data->update_y2)
					d->vfb_data->update_y2 = y+suby;

				suby ++;
			}

		if (x < d->vfb_data->update_x1)
			d->vfb_data->update_x1 = x;
		if (x2 > d->vfb_data->update_x2)
			d->vfb_data->update_x2 = x2;
		}
	}
}


DEVICE_ACCESS(px)
{
	uint64_t idata = 0, odata = 0;
	struct px_data *d = (struct px_data *) extra;
	size_t i;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	if (relative_addr < 0x0c0000) {
		/*
		 *  DMA poll:  a read from this address should start a DMA
		 *  transfer, and return 1 in odata while the DMA is in
		 *  progress (STAMP_BUSY), and then 0 (STAMP_OK) once we're
		 *  done.
		 *
		 *  According to NetBSD's pxreg.h, the following formula gets
		 *  us from system address to DMA address:  (v is the system
		 *  address)
		 *
		 *	dma_addr = ( ( ((v & ~0x7fff) << 3) |
		 *		(v & 0x7fff) ) & 0x1ffff800) >> 9;
		 *
		 *  Hopefully, this is a good enough reversal of that formula:
		 *
		 *	sys_addr = ((dma_addr << 9) & 0x7800) +
		 *		   ((dma_addr << 6) & 0xffff8000);
		 *
		 *  If the board type is "PX" then the system address is an
		 *  address in host memory.  Otherwise, it is relative to
		 *  0x200000 (the i860's memory space on the board).
		 */
		uint32_t sys_addr;	/*  system address for DMA transfers  */
		sys_addr = ((relative_addr << 9) & 0x7800) +
		    ((relative_addr << 6) & 0xffff8000);

		/*
		 *  If the system address is sane enough, then start a DMA
		 *  transfer:  (for the "PX" board type, don't allow obviously
		 *  too-low physical addresses)
		 */
		if (sys_addr >= 0x4000 || d->type != DEV_PX_TYPE_PX)
			dev_px_dma(cpu, sys_addr, d);

		/*  Pretend that it was always OK:  */
		odata = STAMP_OK;
	}

	/*  N10 sram:  */
	if (relative_addr >= 0x200000 && relative_addr < 0x280000) {
		if (d->type == DEV_PX_TYPE_PX)
			fatal("WARNING: the vdac should be at this "
			    "address. overlap problems?\n");

		if (writeflag == MEM_WRITE) {
			for (i=0; i<len; i++)
				d->sram[relative_addr - 0x200000 + i] = data[i];
			/*  NOTE:  this return here supresses debug output
			    (which would be printed if we continue)  */
			return 1;
		} else {
			/*
			 *  Huh? Why have I commented out this? TODO
			 */
			/*  for (i=0; i<len; i++)
				data[i] = d->sram[relative_addr - 0x200000
			    + i];  */
			odata = 1;
		}
	}

	/*  TODO:  Most of these aren't implemented yet.  */

	switch (relative_addr) {

	case 0x180008:		/*  hsync  */
		if (writeflag==MEM_READ) {
			debug("[ px: read from hsync: 0x%08llx ]\n",
			    (long long)odata);
		} else {
			debug("[ px: write to hsync: 0x%08llx ]\n",
			    (long long)idata);
		}
		break;

	case 0x18000c:		/*  hsync2  */
		if (writeflag==MEM_READ) {
			debug("[ px: read from hsync2: 0x%08llx ]\n",
			    (long long)odata);
		} else {
			debug("[ px: write to hsync2: 0x%08llx ]\n",
			    (long long)idata);
		}
		break;

	case 0x180010:		/*  hblank  */
		if (writeflag==MEM_READ) {
			debug("[ px: read from hblank: 0x%08llx ]\n",
			    (long long)odata);
		} else {
			debug("[ px: write to hblank: 0x%08llx ]\n",
			    (long long)idata);
		}
		break;

	case 0x180014:		/*  vsync  */
		if (writeflag==MEM_READ) {
			debug("[ px: read from vsync: 0x%08llx ]\n",
			    (long long)odata);
		} else {
			debug("[ px: write to vsync: 0x%08llx ]\n",
			    (long long)idata);
		}
		break;

	case 0x180018:		/*  vblank  */
		if (writeflag==MEM_READ) {
			debug("[ px: read from vblank: 0x%08llx ]\n",
			    (long long)odata);
		} else {
			debug("[ px: write to vblank: 0x%08llx ]\n",
			    (long long)idata);
		}
		break;

	case 0x180020:		/*  ipdvint  */
		if (writeflag==MEM_READ) {
			odata = d->intr;

/*  TODO:  how do interrupts work on the pixelstamp boards?  */
odata = random();

			debug("[ px: read from ipdvint: 0x%08llx ]\n",
			    (long long)odata);
		} else {
			d->intr = idata;
			if (idata & STIC_INT_E_WE)
				d->intr &= ~STIC_INT_E;
			if (idata & STIC_INT_V_WE)
				d->intr &= ~STIC_INT_V;
			if (idata & STIC_INT_P_WE)
				d->intr &= ~STIC_INT_P;
			debug("[ px: write to ipdvint: 0x%08llx ]\n",
			    (long long)idata);
		}
		break;

	case 0x180028:		/*  sticsr  */
		if (writeflag==MEM_READ) {
			debug("[ px: read from sticsr: 0x%08llx ]\n",
			    (long long)odata);
		} else {
			debug("[ px: write to sticsr: 0x%08llx ]\n",
			    (long long)idata);
		}
		break;

	case 0x180038:		/*  buscsr  */
		if (writeflag==MEM_READ) {
			debug("[ px: read from buscsr: 0x%08llx ]\n",
			    (long long)odata);
		} else {
			debug("[ px: write to buscsr: 0x%08llx ]\n",
			    (long long)idata);
		}
		break;

	case 0x18003c:		/*  modcl  */
		if (writeflag==MEM_READ) {
			odata = (d->type << 12) + (d->xconfig << 11) +
			    (d->yconfig << 9);
			debug("[ px: read from modcl: 0x%llx ]\n",
			    (long long)odata);
		} else {
			debug("[ px: write to modcl: 0x%llx ]\n",
			    (long long)idata);
		}
		break;

	default:
		if (writeflag==MEM_READ) {
			debug("[ px: read from addr 0x%x: 0x%llx ]\n",
			    (int)relative_addr, (long long)odata);
		} else {
			debug("[ px: write to addr 0x%x: 0x%llx ]\n",
			    (int)relative_addr, (long long)idata);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


void dev_px_init(struct machine *machine, struct memory *mem,
	uint64_t baseaddr, int px_type, const char *irq_path)
{
	struct px_data *d;

	CHECK_ALLOCATION(d = (struct px_data *) malloc(sizeof(struct px_data)));
	memset(d, 0, sizeof(struct px_data));

	d->type = px_type;

	INTERRUPT_CONNECT(irq_path, d->irq);

	d->xconfig = d->yconfig = 0;	/*  4x1  */

	d->bitdepth = 24;
	d->px_name = "(invalid)";

	switch (d->type) {
	case DEV_PX_TYPE_PX:
		d->bitdepth = 8;
		d->px_name = "PX";
		break;
	case DEV_PX_TYPE_PXG:
		d->bitdepth = 8;
		d->px_name = "PXG";
		break;
	case DEV_PX_TYPE_PXGPLUS:
		d->px_name = "PXG+";
		break;
	case DEV_PX_TYPE_PXGPLUSTURBO:
		d->px_name = "PXG+ TURBO";
		d->xconfig = d->yconfig = 1;	/*  5x2  */
		break;
	default:
		fatal("dev_px_init(): unimplemented px_type\n");
	}

	d->fb_mem = memory_new(PX_XSIZE * PX_YSIZE * d->bitdepth / 8,
	    machine->arch);
	if (d->fb_mem == NULL) {
		fprintf(stderr, "dev_px_init(): out of memory (1)\n");
		exit(1);
	}

	d->vfb_data = dev_fb_init(machine, d->fb_mem, 0, VFB_GENERIC,
	    PX_XSIZE, PX_YSIZE, PX_XSIZE, PX_YSIZE, d->bitdepth, d->px_name);
	if (d->vfb_data == NULL) {
		fprintf(stderr, "dev_px_init(): out of memory (2)\n");
		exit(2);
	}

	switch (d->type) {
	case DEV_PX_TYPE_PX:
		dev_bt459_init(machine, mem, baseaddr + 0x200000, 0,
		    d->vfb_data, 8, irq_path, BT459_PX);
		break;
	case DEV_PX_TYPE_PXG:
	case DEV_PX_TYPE_PXGPLUS:
	case DEV_PX_TYPE_PXGPLUSTURBO:
		dev_bt459_init(machine, mem, baseaddr + 0x300000, 0,
		    d->vfb_data, d->bitdepth, irq_path, BT459_PX);
		break;
	default:
		fatal("dev_px_init(): unimplemented px_type\n");
	}

	memory_device_register(mem, "px", baseaddr, DEV_PX_LENGTH,
	    dev_px_access, d, DM_DEFAULT, NULL);
	machine_add_tickfunction(machine, dev_px_tick, d, 14);
}

