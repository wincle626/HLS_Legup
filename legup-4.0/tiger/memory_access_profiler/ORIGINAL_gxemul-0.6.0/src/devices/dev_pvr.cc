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
 *  COMMENT: PowerVR CLX2 (graphics controller used in the Dreamcast)
 *
 *  Implemented by reading http://www.ludd.luth.se/~jlo/dc/powervr-reg.txt and
 *  http://mc.pp.se/dc/pvr.html, source code of various demos and KalistOS,
 *  and doing a lot of guessing.
 *
 *  TODO: Almost everything
 *
 *	x)  Change resolution during runtime (PAL/NTSC/???)
 *
 *	x)  Lots of work on the 3D "Tile Accelerator" engine.
 *		Recognize commands and turn into OpenGL or similar
 *		commands on the host?
 *		Color clipping.
 *		Wire-frame when running on a host without XGL?
 *
 *  Multiple lists of various kinds (6?).
 *  Lists growing downwards!
 *  Pixel clip for rendering.
 *  Real Rendering, using OpenGL if possible.
 *  Tile bins... with 6 pointers for each tile (?)
 *  PVR DMA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "devices.h"
#include "float_emul.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"
#include "timer.h"

#include "thirdparty/dreamcast_pvr.h"
#include "thirdparty/dreamcast_sysasicvar.h"


#define	TA_DEBUG
#define debug fatal

#define	INTERNAL_FB_ADDR	0x300000000ULL
#define	PVR_FB_TICK_SHIFT	19

#define	VRAM_SIZE		(8*1048576)

/*  DMA:  */
#define	PVR_DMA_MEMLENGTH	0x100
#define	N_PVR_DMA_REGS		(PVR_DMA_MEMLENGTH / sizeof(uint32_t))

#define	PVR_ADDR		0x00
#define	PVR_COUNT		0x04
#define	PVR_MODE		0x08
#define	PVR_LMMODE0		0x84  
#define	PVR_LMMODE1		0x88


#define	PVR_VBLANK_HZ		60.0
#define	PVR_MARGIN		16

struct pvr_data {
	struct vfb_data		*fb;
	int			fb_update_x1;
	int			fb_update_y1;
	int			fb_update_x2;
	int			fb_update_y2;

	struct timer		*vblank_timer;
	int			vblank_interrupts_pending;

	/*  PVR registers:  */
	uint32_t		reg[PVRREG_REGSIZE / sizeof(uint32_t)];

	/*  Calculated by pvr_geometry_updated():  */
	int			xsize, ysize;
	int			bytes_per_pixel;

	/*  Cached values (from registers):  */
	/*  DIWMODE:  */
	int			clock_double;
	int			strip_buffer_enabled;
	int			strip_length;
	int			argb8888_threshold;
	int			extend;
	int			pixelmode;
	int			line_double;
	int			display_enabled;
	/*  BRDCOLR:  */
	int			border_updated;
	/*  SYNCCONF:  */
	int			video_enabled;
	int			broadcast_standard;
	int			interlaced;
	int			h_sync_positive;
	int			v_sync_positive;
	/*  TILEBUF_SIZE:  */
	int			tilebuf_xsize;
	int			tilebuf_ysize;

	/*  Tile Accelerator Command:  */
	uint32_t		ta[64 / sizeof(uint32_t)];

	uint8_t			*vram;

	/*  DMA registers:  */
	uint32_t		dma_reg[N_PVR_DMA_REGS];
};

struct pvr_data_alt {
	struct pvr_data		*d;
};


#define	REG(x)		(d->reg[(x)/sizeof(uint32_t)])
#define	DEFAULT_WRITE	REG(relative_addr) = idata;


/*  Forward declaration.  */
DEVICE_ACCESS(pvr_ta);


void pvr_dma_transfer(struct cpu *cpu, struct pvr_data *d)
{
	const int channel = 3;
	uint32_t sar = cpu->cd.sh.dmac_sar[channel] & 0x1fffffff;
	uint32_t dar = cpu->cd.sh.dmac_dar[channel] & 0x1fffffff;
	uint32_t count = cpu->cd.sh.dmac_tcr[channel] & 0x1fffffff;
	uint32_t chcr = cpu->cd.sh.dmac_chcr[channel];
	int transmit_size = 1;
	int src_delta = 0, dst_delta = 0;
	int cause_interrupt = chcr & CHCR_IE;
        
	/*  DMAC not enabled?  */
	if (!(chcr & CHCR_TD)) {
		fatal("pvr_dma_transfer: SH4 dma not enabled?\n");
		exit(1);
	}

	/*  Transfer End already set? Then don't transfer again.  */
	if (chcr & CHCR_TE)
		return;

	/*  Special case: 0 means 16777216:  */
	if (count == 0)
		count = 16777216;

	switch (chcr & CHCR_TS) {
	case CHCR_TS_8BYTE: transmit_size = 8; break;
	case CHCR_TS_1BYTE: transmit_size = 1; break;
	case CHCR_TS_2BYTE: transmit_size = 2; break;
	case CHCR_TS_4BYTE: transmit_size = 4; break;
	case CHCR_TS_32BYTE: transmit_size = 32; break;
	default: fatal("Unimplemented transmit size?! CHCR[%i] = 0x%08x\n",
	    channel, chcr);
	exit(1);
	}

	switch (chcr & CHCR_DM) {
	case CHCR_DM_FIXED:       dst_delta = 0; break;
	case CHCR_DM_INCREMENTED: dst_delta = 1; break;
	case CHCR_DM_DECREMENTED: dst_delta = -1; break;
	default: fatal("Unimplemented destination delta?! CHCR[%i] = 0x%08x\n",
	    channel, chcr);
	exit(1);
	}

	switch (chcr & CHCR_SM) {
	case CHCR_SM_FIXED:       src_delta = 0; break;
	case CHCR_SM_INCREMENTED: src_delta = 1; break;
	case CHCR_SM_DECREMENTED: src_delta = -1; break;
	default: fatal("Unimplemented source delta?! CHCR[%i] = 0x%08x\n",
	    channel, chcr);
	exit(1);
	}

	src_delta *= transmit_size;
	dst_delta *= transmit_size;

	switch (chcr & CHCR_RS) {
	case 0x200:
		dar = d->dma_reg[PVR_ADDR / sizeof(uint32_t)];

		if (dar != 0x10000000) {
			fatal("[TODO: DMA to non-TA? dar=%08x\n", (int)dar);
			cpu->cd.sh.dmac_chcr[channel] |= CHCR_TE;
			break;
		}

		while (count > 0) {
			unsigned char buf[32];
			int ofs;
			size_t chunksize = transmit_size;

			if (chunksize > sizeof(uint32_t))
				chunksize = sizeof(uint32_t);

			for (ofs = 0; ofs < transmit_size; ofs += chunksize) {
				cpu->memory_rw(cpu, cpu->mem, sar + ofs, buf,
				    chunksize, MEM_READ, NO_EXCEPTIONS | PHYSICAL);

				dev_pvr_ta_access(cpu, cpu->mem, ofs, buf, chunksize,
				    MEM_WRITE, d);

				/*  cpu->memory_rw(cpu, cpu->mem, dar + ofs, buf,
				    chunksize, MEM_WRITE, NO_EXCEPTIONS | PHYSICAL);  */
			}

			count --;
			sar += src_delta;
		}

		/*  Transfer End:  */
		cpu->cd.sh.dmac_chcr[channel] |= CHCR_TE;

		break;
	default:
		fatal("Unimplemented SH4 RS DMAC: 0x%08x (PVR)\n",
		    (int) (chcr & CHCR_RS));
		exit(1);
	}

	if (cause_interrupt) {
		fatal("TODO: pvr sh4 dmac interrupt!\n");
		exit(1);
	}
}


DEVICE_ACCESS(pvr_dma)
{
	struct pvr_data *d = (struct pvr_data *) extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	/*  Default read:  */
	if (writeflag == MEM_READ)
		odata = d->dma_reg[relative_addr / sizeof(uint32_t)];

	switch (relative_addr) {

	case PVR_ADDR:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr_dma: ADDR set to 0x%08x ]\n",
			    (int) idata);
		}
		break;

	case PVR_COUNT:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr_dma: LEN set to 0x%08x ]\n",
			    (int) idata);
		}
		break;

	case PVR_MODE:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr_dma: MODE set to 0x%08x ]\n",
			    (int) idata);
			if (idata != 0) {
				pvr_dma_transfer(cpu, d);
				idata = 0;
			}
		}
		break;

	/*  These are written to by the Dreamcast ROM, but I have not
	    found them documented anywhere.  */
	case 0x10:
	case 0x14:
		if (writeflag == MEM_WRITE && idata != 0x0cff0000) {
			fatal("[ pvr_dma: TODO: unknown_0x%02x set to "
			    "0x%08x ]\n", (int) relative_addr, (int) idata);
			exit(1);
		}
		break;

	case 0x18:
	case 0x1c:
	case 0x20:
	case 0x40:
	case 0x44:
	case 0x48:
	case 0x4c:
		if (writeflag == MEM_WRITE && idata != 0) {
			fatal("[ pvr_dma: TODO: unknown_0x%02x set to "
			    "0x%08x ]\n", (int) relative_addr, (int) idata);
			exit(1);
		}
		break;

	case PVR_LMMODE0:	/*  0x84  */
		if (writeflag == MEM_WRITE && idata != 0) {
			fatal("[ pvr_dma: TODO: LMMODE0 set to "
			    "0x%08x ]\n", (int) idata);
			exit(1);
		}
		break;

	case PVR_LMMODE1:	/*  0x84  */
		if (writeflag == MEM_WRITE && idata != 0) {
			fatal("[ pvr_dma: TODO: LMMODE1 set to "
			    "0x%08x ]\n", (int) idata);
			exit(1);
		}
		break;

	case 0x8c:
		if (writeflag == MEM_WRITE) {
			fatal("[ pvr_dma: write to 0x8c: TODO ]\n");
			exit(1);
		} else {
			/*  0x20 means G2 DMA in progress?  */
			/*  0x11 = mask which has to do with AICA  */
			odata = 0x11 * (random() & 1);
		}
		break;

	case 0x9c:
		/*  TODO  */
		break;

	case 0xa0:
		if (writeflag == MEM_WRITE && idata != 0x80000000) {
			fatal("[ pvr_dma: TODO: unknown_0x%02x set to "
			    "0x%08x ]\n", (int) relative_addr, (int) idata);
			exit(1);
		}
		break;

	case 0xa4:
	case 0xac:
		if (writeflag == MEM_WRITE && idata != 0) {
			fatal("[ pvr_dma: TODO: unknown_0x%02x set to "
			    "0x%08x ]\n", (int) relative_addr, (int) idata);
			exit(1);
		}
		break;

	default:if (writeflag == MEM_READ) {
			fatal("[ pvr_dma: read from addr 0x%x ]\n",
			    (int)relative_addr);
		} else {
			fatal("[ pvr_dma: write to addr 0x%x: 0x%x ]\n",
			    (int)relative_addr, (int)idata);
		}

		exit(1);
	}

	/*  Default write:  */
	if (writeflag == MEM_WRITE)
		d->dma_reg[relative_addr / sizeof(uint32_t)] = idata;

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


/*
 *  pvr_fb_invalidate():
 */
void pvr_fb_invalidate(struct pvr_data *d, int start, int stop)
{
	d->fb_update_x1 = d->fb_update_y1 = 0;
	d->fb_update_x2 = d->xsize - 1;
	d->fb_update_y2 = d->ysize - 1;
}


/*
 *  pvr_vblank_timer_tick():
 *
 *  This function is called PVR_VBLANK_HZ times per real-world second. Its job
 *  is to fake vertical retrace interrupts.
 */     
static void pvr_vblank_timer_tick(struct timer *t, void *extra)
{
	struct pvr_data *d = (struct pvr_data *) extra;
	d->vblank_interrupts_pending ++;
}


/*
 *  pvr_geometry_updated():
 *
 *  This function should be called every time a register is written to which
 *  affects the framebuffer geometry (size, bit-depth, starting position, etc).
 */
void pvr_geometry_updated(struct pvr_data *d)
{
	/*  Make sure to redraw border on geometry changes.  */
	d->border_updated = 1;

	d->xsize = (REG(PVRREG_DIWSIZE) >> DIWSIZE_DPL_SHIFT) & DIWSIZE_MASK;
	d->ysize = (REG(PVRREG_DIWSIZE) >> DIWSIZE_LPF_SHIFT) & DIWSIZE_MASK;

	/*  E.g. 319x479  =>  320x480  */
	d->xsize = (d->xsize + 1) * sizeof(uint32_t);
	d->ysize ++;

	switch (d->pixelmode) {
	case 0:
	case 1:	d->bytes_per_pixel = 2; break;
	case 2:	d->bytes_per_pixel = 3; break;
	case 3:	d->bytes_per_pixel = 4; break;
	}

	d->xsize /= d->bytes_per_pixel;

	if (REG(PVRREG_DIWCONF) & DIWCONF_LR)
		d->xsize /= 2;

	if (d->line_double)
		d->ysize /= 2;

	/*  Only show geometry debug message if output is enabled:  */
	if (!d->video_enabled || !d->display_enabled)
		return;

	debug("[ pvr_geometry_updated: %i x %i, ", d->xsize, d->ysize);

	switch (d->pixelmode) {
	case 0: debug("RGB0555 (16-bit)"); break;
	case 1: debug("RGB565 (16-bit)"); break;
	case 2: debug("RGB888 (24-bit)"); break;
	case 3: debug("RGB0888 (32-bit)"); break;
	}

	debug(" ]\n");
}


/*  Ugly quick-hack:  */
static void line(struct pvr_data *d, int x1, int y1, int x2, int y2)
{
	int fb_base = REG(PVRREG_FB_RENDER_ADDR1);
	int i;
	for (i=0; i<256; i++) {
		int px = (i * x2 + (256-i) * x1) >> 8;
		int py = (i * y2 + (256-i) * y1) >> 8;
		if (px > 0 && py > 0 && px < d->xsize && py < d->ysize) {
			int ofs = fb_base + (px + py * d->xsize) *
                            d->bytes_per_pixel;
			d->vram[(ofs+0) % VRAM_SIZE] = 255;
			d->vram[(ofs+1) % VRAM_SIZE] = 255;
		}
	}
}


/*
 *  pvr_render():
 *
 *  Render from the Object Buffer to the framebuffer.
 *
 *  TODO: This function is totally bogus so far, the format of the Object
 *        Buffer is just a quick made-up hack to see if it works at all.
 */
void pvr_render(struct cpu *cpu, struct pvr_data *d)
{
	int ob_ofs = REG(PVRREG_OB_ADDR);
	int fb_base = REG(PVRREG_FB_RENDER_ADDR1);
	int wf_point_nr, texture = 0;
	int wf_x[4], wf_y[4];

	debug("[ pvr_render: rendering to FB offset 0x%x ]\n", fb_base);

	/*  Clear all pixels first:  */
	/*  TODO  */
	memset(d->vram + fb_base, 0, d->xsize * d->ysize * d->bytes_per_pixel);

	wf_point_nr = 0;

	for (;;) {
		uint8_t cmd = d->vram[ob_ofs % VRAM_SIZE];

		if (ob_ofs >= VRAM_SIZE)
			fatal("[ pvr_render: WARNING: ob_ofs > VRAM_SIZE! ]\n");

		if (cmd == 0)
			break;
		else if (cmd == 1) {
			int16_t px = d->vram[(ob_ofs+2)%VRAM_SIZE] +
			    d->vram[(ob_ofs+3)%VRAM_SIZE]*256;
			int16_t py = d->vram[(ob_ofs+4)%VRAM_SIZE] +
			    d->vram[(ob_ofs+5)%VRAM_SIZE]*256;

			wf_x[wf_point_nr] = px;
			wf_y[wf_point_nr] = py;

			wf_point_nr ++;
			if (wf_point_nr == 4) {
#if 1
				line(d, wf_x[0], wf_y[0], wf_x[1], wf_y[1]);
				line(d, wf_x[0], wf_y[0], wf_x[2], wf_y[2]);
				line(d, wf_x[1], wf_y[1], wf_x[3], wf_y[3]);
				line(d, wf_x[2], wf_y[2], wf_x[3], wf_y[3]);
				wf_point_nr = 0;
				wf_x[0] = wf_x[2]; wf_y[0] = wf_y[2];
				wf_x[1] = wf_x[3]; wf_y[1] = wf_y[3];
#else
				draw_texture(d, wf_x[0], wf_y[0],
				    wf_x[1], wf_y[1],
				    wf_x[2], wf_y[2],
				    wf_x[3], wf_y[3], texture);
#endif
			}

		} else if (cmd == 2) {
			wf_point_nr = 0;
			texture = d->vram[(ob_ofs+4)%VRAM_SIZE] +
			    (d->vram[(ob_ofs+5)%VRAM_SIZE]
			    << 8) + (d->vram[(ob_ofs+6)%VRAM_SIZE] << 16) +
			    (d->vram[(ob_ofs+7)%VRAM_SIZE] << 24);
			texture <<= 3;
			texture &= 0x7fffff;
			printf("TEXTURE = %x\n", texture);
		} else {
			fatal("pvr_render: internal error, unknown cmd\n");
		}

		ob_ofs += sizeof(uint64_t);
	}

	SYSASIC_TRIGGER_EVENT(SYSASIC_EVENT_RENDERDONE);
}


/*
 *  pvr_reset_ta():
 *
 *  Reset the Tile Accelerator.
 */
static void pvr_reset_ta(struct pvr_data *d)
{
	REG(PVRREG_DIWCONF) = DIWCONF_MAGIC;
}


/*
 *  pvr_reset():
 *
 *  Reset the PVR.
 */
static void pvr_reset(struct pvr_data *d)
{
	/*  TODO  */
}


/*
 *  pvr_ta_init():
 *
 *  Initialize the Tile Accelerator. This makes the TA ready to receive
 *  commands (via address 0x10000000).
 */
void pvr_ta_init(struct cpu *cpu, struct pvr_data *d)
{
	REG(PVRREG_TA_OPB_POS) = REG(PVRREG_TA_OPB_START);
	REG(PVRREG_TA_OB_POS) = REG(PVRREG_TA_OB_START);
}


/*
 *  pvr_ta_command():
 *
 *  Read a command (e.g. parts of a polygon primitive) from d->ta[], and output
 *  "compiled commands" into the Object list and Object Pointer list.
 */
static void pvr_ta_command(struct cpu *cpu, struct pvr_data *d, int list_ofs)
{
	int ob_ofs;
	int16_t x, y;
	uint32_t *ta = &d->ta[list_ofs];

#ifdef TA_DEBUG
	/*  Dump the Tile Accelerator command for debugging:  */
	{
		int i;
		fatal("TA cmd:");
		for (i=0; i<8; i++)
			fatal(" %08x", (int) ta[i]);
		fatal("\n");
	}
#endif

	/*
	 *  TODO: REWRITE!!!
	 *
	 *  This is just a quick hack to see if I can get out at least
	 *  the pixel coordinates.
	 */

	{
		struct ieee_float_value fx, fy;
		ieee_interpret_float_value(ta[1], &fx, IEEE_FMT_S);
		ieee_interpret_float_value(ta[2], &fy, IEEE_FMT_S);
		x = (int16_t) fx.f; y = (int16_t) fy.f;
	}

	ob_ofs = REG(PVRREG_TA_OB_POS);

	if (ob_ofs >= VRAM_SIZE - 8)
		fatal("[ WARNING: ob_ofs >= VRAM_SIZE - 8 ]\n");

	switch (ta[0] >> 28) {
	case 0x8:
		d->vram[ob_ofs + 0] = 2;
		d->vram[ob_ofs + 4] = ta[3];
		d->vram[ob_ofs + 5] = ta[3] >> 8;
		d->vram[ob_ofs + 6] = ta[3] >> 16;
		d->vram[ob_ofs + 7] = ta[3] >> 24;
		REG(PVRREG_TA_OB_POS) = ob_ofs + sizeof(uint64_t);
		break;
	case 0xe:
	case 0xf:
		/*  Point.  */
		d->vram[ob_ofs + 0] = 1;
		d->vram[ob_ofs + 2] = x & 255;
		d->vram[ob_ofs + 3] = x >> 8;
		d->vram[ob_ofs + 4] = y & 255;
		d->vram[ob_ofs + 5] = y >> 8;
		REG(PVRREG_TA_OB_POS) = ob_ofs + sizeof(uint64_t);
		break;
	case 0x0:
		if (ta[1] == 0) {
			/*  End of list.  */
			uint32_t opb_cfg = REG(PVRREG_TA_OPB_CFG);
			d->vram[ob_ofs + 0] = 0;
			REG(PVRREG_TA_OB_POS) = ob_ofs + sizeof(uint64_t);
			if (opb_cfg & TA_OPB_CFG_OPAQUEPOLY_MASK)
				SYSASIC_TRIGGER_EVENT(SYSASIC_EVENT_OPAQUEDONE);
			if (opb_cfg & TA_OPB_CFG_OPAQUEMOD_MASK)
				SYSASIC_TRIGGER_EVENT(
				    SYSASIC_EVENT_OPAQUEMODDONE);
			if (opb_cfg & TA_OPB_CFG_TRANSPOLY_MASK)
				SYSASIC_TRIGGER_EVENT(SYSASIC_EVENT_TRANSDONE);
			if (opb_cfg & TA_OPB_CFG_TRANSMOD_MASK)
				SYSASIC_TRIGGER_EVENT(
				    SYSASIC_EVENT_TRANSMODDONE);
			if (opb_cfg & TA_OPB_CFG_PUNCHTHROUGH_MASK)
				SYSASIC_TRIGGER_EVENT(SYSASIC_EVENT_PVR_PTDONE);
		}
		break;
	case 2:	/*  Ignore for now.  */
	case 3:	/*  Ignore for now.  */
		/*  TODO  */
		break;
	default:fatal("Unimplemented top TA nibble %i\n", ta[0] >> 28);
		exit(1);
	}
}


DEVICE_ACCESS(pvr_ta)
{
	struct pvr_data *d = (struct pvr_data *) extra;
	uint64_t idata = 0, odata = 0;

#if 0
	if (writeflag == MEM_WRITE)
		fatal("[ pvr_ta: WRITE addr=%08x value=%08x\n ]\n",
		    (int)relative_addr, (int)idata);
	else
		fatal("[ pvr_ta: READ addr=%08x ]\n",
		    (int)relative_addr);
#endif

	if (len != sizeof(uint32_t)) {
		fatal("pvr_ta access len = %i: TODO\n", (int) len);
		exit(1);
	}

	if (writeflag == MEM_WRITE) {
		idata = memory_readmax64(cpu, data, len);

		/*  Write to the tile accelerator command buffer:  */
		d->ta[relative_addr / sizeof(uint32_t)] = idata;

		/*  Execute the command, after a complete write:  */
		if (relative_addr == 0x1c)
			pvr_ta_command(cpu, d, 0);
		if (relative_addr == 0x3c)
			pvr_ta_command(cpu, d, 8);
	} else {
		odata = d->ta[relative_addr / sizeof(uint32_t)];
		memory_writemax64(cpu, data, len, odata);
	}

	return 1;
}


DEVICE_ACCESS(pvr)
{
	struct pvr_data *d = (struct pvr_data *) extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	/*  Default read action: Read from reg[]:  */
	if (writeflag == MEM_READ)
		odata = d->reg[relative_addr / sizeof(uint32_t)];

	/*  Fog table access:  */
	if (relative_addr >= PVRREG_FOG_TABLE &&
	    relative_addr < PVRREG_FOG_TABLE + PVR_FOG_TABLE_SIZE) {
		if (writeflag == MEM_WRITE)
			DEFAULT_WRITE;
		goto return_ok;
	}

	switch (relative_addr) {

	case PVRREG_ID:
		/*  ID for Set 5.xx versions of the Dreamcast, according
		    to http://www.ludd.luth.se/~jlo/dc/powervr-reg.txt:  */
		odata = 0x17fd11db;
		break;

	case PVRREG_REVISION:
		/*  Revision 1.1, for Dreamcast Set 5.2x.  */
		odata = 0x00000011;
		break;

	case PVRREG_RESET:
		if (writeflag == MEM_WRITE) {
			if (idata != 0) {
				debug("[ pvr: RESET ");
				if (idata & PVR_RESET_PVR)
					pvr_reset(d);
				if (idata & PVR_RESET_TA)
					pvr_reset_ta(d);
				debug("]\n");
			}
			idata = 0;
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_STARTRENDER:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: STARTRENDER ]\n");
			pvr_render(cpu, d);
		} else {
			fatal("[ pvr: huh? read from STARTRENDER ]\n");
		}
		break;

	case PVRREG_OB_ADDR:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: OB_ADDR set to 0x%08"PRIx32" ]\n",
			    (uint32_t)(idata & PVR_OB_ADDR_MASK));
			/*  if (idata & ~PVR_OB_ADDR_MASK) {
				fatal("[ pvr: OB_ADDR: Fatal error: Unknown"
				    " bits set: 0x%08"PRIx32" ]\n",
				    (uint32_t)(idata & ~PVR_OB_ADDR_MASK));
				exit(1);
			}
			idata &= PVR_OB_ADDR_MASK;
			*/
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_TILEBUF_ADDR:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: TILEBUF_ADDR set to 0x%08"PRIx32" ]\n",
			    (uint32_t)(idata & PVR_TILEBUF_ADDR_MASK));
			if (idata & ~PVR_TILEBUF_ADDR_MASK)
				fatal("[ pvr: TILEBUF_ADDR: WARNING: Unknown"
				    " bits set: 0x%08"PRIx32" ]\n",
				    (uint32_t)(idata & ~PVR_TILEBUF_ADDR_MASK));
			idata &= PVR_TILEBUF_ADDR_MASK;
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_SPANSORT:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: SPANSORT: ");
			if (idata & PVR_SPANSORT_SPAN0)
				debug("SPAN0 ");
			if (idata & PVR_SPANSORT_SPAN1)
				debug("SPAN1 ");
			if (idata & PVR_SPANSORT_TSP_CACHE_ENABLE)
				debug("TSP_CACHE_ENABLE ");
			debug("]\n");
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_BRDCOLR:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: BRDCOLR set to 0x%06"PRIx32" ]\n",
			    (int)idata);
			DEFAULT_WRITE;
			d->border_updated = 1;
		}
		break;

	case PVRREG_DIWMODE:
		if (writeflag == MEM_WRITE) {
			d->clock_double = idata & DIWMODE_C_MASK? 1:0;
			d->strip_buffer_enabled = idata & DIWMODE_SE_MASK? 1:0;
			d->strip_length = (idata & DIWMODE_SL_MASK)
			    >> DIWMODE_SL_SHIFT;
			d->argb8888_threshold = (idata & DIWMODE_TH_MASK)
			    >> DIWMODE_TH_SHIFT;
			d->extend = (idata & DIWMODE_EX_MASK)
			    >> DIWMODE_EX_SHIFT;
			d->pixelmode = (idata & DIWMODE_COL_MASK)
			    >> DIWMODE_COL_SHIFT;
			d->line_double = idata & DIWMODE_SD_MASK? 1:0;
			d->display_enabled = idata & DIWMODE_DE_MASK? 1:0;

			debug("[ pvr: DIWMODE set to: ");
			debug("clock_double=%i, ", d->clock_double);
			debug("strip_buffer_enabled=%i, ",
			    d->strip_buffer_enabled);
			debug("strip_length=%i, ", d->strip_length);
			debug("argb8888_threshold=%i, ", d->argb8888_threshold);
			debug("extend=0x%x, ", d->extend);
			debug("pixelmode=");
			switch (d->pixelmode) {
			case 0: debug("RGB0555 (16-bit)"); break;
			case 1: debug("RGB565 (16-bit)"); break;
			case 2: debug("RGB888 (24-bit)"); break;
			case 3: debug("RGB0888 (32-bit)"); break;
			}
			debug(", line_double=%i, ", d->line_double);
			debug("display_enabled=%i", d->display_enabled);
			debug(" ]\n");

			DEFAULT_WRITE;
			pvr_geometry_updated(d);
			pvr_fb_invalidate(d, -1, -1);
		}
		break;

	case PVRREG_DIWSIZE:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: DIWSIZE set to modulo=%i, "
			    "width=%i, height=%i ]\n", (int)
			    ((idata >> DIWSIZE_MODULO_SHIFT) & DIWSIZE_MASK),
			    (int)((idata >> DIWSIZE_DPL_SHIFT) & DIWSIZE_MASK),
			    (int)((idata >> DIWSIZE_LPF_SHIFT) & DIWSIZE_MASK));
			DEFAULT_WRITE;
			pvr_geometry_updated(d);
			pvr_fb_invalidate(d, -1, -1);
		}
		break;

	case PVRREG_FB_RENDER_ADDR1:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: FB_RENDER_ADDR1 set to 0x%08"PRIx32
			    " ]\n", (int) idata);
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_FB_RENDER_ADDR2:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: FB_RENDER_ADDR2 set to 0x%08"PRIx32
			    " ]\n", (int) idata);
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_FB_CLIP_X:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: FB_CLIP_X set to min=%i, "
			    "max=%i ]\n", (int) (idata & FB_CLIP_XY_MIN_MASK),
			    (int) ((idata & FB_CLIP_XY_MAX_MASK)
			    >> FB_CLIP_XY_MAX_SHIFT));
			DEFAULT_WRITE;
			pvr_geometry_updated(d);
			pvr_fb_invalidate(d, -1, -1);
		}
		break;

	case PVRREG_FB_CLIP_Y:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: FB_CLIP_Y set to min=%i, "
			    "max=%i ]\n", (int) (idata & FB_CLIP_XY_MIN_MASK),
			    (int) ((idata & FB_CLIP_XY_MAX_MASK)
			    >> FB_CLIP_XY_MAX_SHIFT));
			DEFAULT_WRITE;
			pvr_geometry_updated(d);
			pvr_fb_invalidate(d, -1, -1);
		}
		break;

	case PVRREG_SHADOW:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: SHADOW set to enable=%i, "
			    "intensity=%i ]\n",
			    (int) (idata & SHADOW_ENABLE? 1 : 0),
			    (int) (idata & SHADOW_INTENSITY_MASK));
			DEFAULT_WRITE;
			pvr_geometry_updated(d);
			pvr_fb_invalidate(d, -1, -1);
		}
		break;

	case PVRREG_OBJECT_CLIP:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: OBJECT_CLIP 0x%08x ]\n", (int)idata);
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_OB_CFG:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: OB_CFG 0x%08x ]\n", (int)idata);
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_UNKNOWN_80:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: UNKNOWN_80 0x%08x ]\n", (int)idata);
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_UNKNOWN_84:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: UNKNOWN_84 0x%08x ]\n", (int)idata);
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_BGPLANE_Z:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: BGPLANE_Z 0x%08x ]\n", (int)idata);
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_BGPLANE_CFG:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: BGPLANE_CFG 0x%08x ]\n", (int)idata);
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_ISP_CFG:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: ISP_CFG 0x%08x ]\n", (int)idata);
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_VRAM_CFG1:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: VRAM_CFG1 set to 0x%08"PRIx32,
			    (int) idata);
			if (idata != VRAM_CFG1_GOOD_REFRESH_VALUE)
				fatal("{ VRAM_CFG1 = 0x%08"PRIx32" is not "
				    "yet implemented! }", (int) idata);
			debug(" ]\n");
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_VRAM_CFG2:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: VRAM_CFG2 set to 0x%08"PRIx32,
			    (int) idata);
			if (idata != VRAM_CFG2_UNKNOWN_MAGIC)
				fatal("{ VRAM_CFG2 = 0x%08"PRIx32" is not "
				    "yet implemented! }", (int) idata);
			debug(" ]\n");
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_VRAM_CFG3:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: VRAM_CFG3 set to 0x%08"PRIx32,
			    (int) idata);
			if (idata != VRAM_CFG3_UNKNOWN_MAGIC)
				fatal("{ VRAM_CFG3 = 0x%08"PRIx32" is not "
				    "yet implemented! }", (int) idata);
			debug(" ]\n");
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_FOG_TABLE_COL:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: FOG_TABLE_COL set to 0x%06"PRIx32" ]\n",
			    (int) idata);
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_FOG_VERTEX_COL:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: FOG_VERTEX_COL set to 0x%06"PRIx32" ]\n",
			    (int) idata);
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_FOG_DENSITY:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: FOG_DENSITY set to 0x%08"PRIx32" ]\n",
			    (int) idata);
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_CLAMP_MAX:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: CLAMP_MAX set to 0x%06"PRIx32" ]\n",
			    (int) idata);
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_CLAMP_MIN:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: CLAMP_MIN set to 0x%06"PRIx32" ]\n",
			    (int) idata);
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_FB_RENDER_CFG:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: PVRREG_FB_RENDER_CFG set to 0x%08x ]\n",
			    (int) idata);
			/*  TODO  */
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_FB_RENDER_MODULO:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: PVRREG_FB_RENDER_MODULO set to %i ]\n",
			    (int) idata);
			/*  TODO  */
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_DIWADDRL:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: DIWADDRL set to 0x%08"PRIx32" ]\n",
			    (int) idata);
			pvr_fb_invalidate(d, -1, -1);
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_DIWADDRS:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: DIWADDRS set to 0x%08"PRIx32" ]\n",
			    (int) idata);
			pvr_fb_invalidate(d, -1, -1);
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_HPOS_IRQ:
		DEFAULT_WRITE;
		break;

	case PVRREG_RASEVTPOS:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: RASEVTPOS pos1=%i pos2=%i ]\n",
			    (int)((idata & RASEVTPOS_POS1_MASK)
			    >> RASEVTPOS_POS1_SHIFT),
			    (int)(idata & RASEVTPOS_POS2_MASK));
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_SYNCCONF:
		if (writeflag == MEM_WRITE) {
			d->video_enabled = idata & SYNCCONF_VO_MASK? 1:0;
			d->broadcast_standard = (idata & SYNCCONF_BC_MASK)
			    >> SYNCCONF_BC_SHIFT;
			d->interlaced = idata & SYNCCONF_I_MASK? 1:0;
			d->h_sync_positive = idata & SYNCCONF_HP_MASK? 1:0;
			d->v_sync_positive = idata & SYNCCONF_VP_MASK? 1:0;

			debug("[ pvr: SYNCCONF set to: ");
			debug("video_enabled=%i, ", d->video_enabled);
			switch (d->broadcast_standard) {
			case SYNCCONF_BC_VGA: debug("VGA"); break;
			case SYNCCONF_BC_NTSC: debug("NTSC"); break;
			case SYNCCONF_BC_PAL: debug("PAL"); break;
			default: debug("*UNKNOWN*"); break;
			}
			debug(", interlaced=%i, ", d->interlaced);
			debug("hsync=%i, ", d->h_sync_positive);
			debug("vsync=%i ]\n", d->v_sync_positive);

			DEFAULT_WRITE;
			pvr_geometry_updated(d);
			pvr_fb_invalidate(d, -1, -1);
		}
		break;

	case PVRREG_BRDHORZ:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: BRDHORZ start=%i stop=%i ]\n",
			    (int)((idata & BRDHORZ_START_MASK)
			    >> BRDHORZ_START_SHIFT),
			    (int)(idata & BRDHORZ_STOP_MASK));
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_SYNCSIZE:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: SYNCSIZE v=%i h=%i ]\n",
			    (int)((idata & SYNCSIZE_V_MASK)
			    >> SYNCSIZE_V_SHIFT),
			    (int)(idata & SYNCSIZE_H_MASK));
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_BRDVERT:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: BRDVERT start=%i stop=%i ]\n",
			    (int)((idata & BRDVERT_START_MASK)
			    >> BRDVERT_START_SHIFT),
			    (int)(idata & BRDVERT_STOP_MASK));
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_SYNCH_WIDTH:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: SYNCH_WIDTH 0x%08x ]\n", (int)idata);
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_TSP_CFG:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: TSP_CFG 0x%08x ]\n", (int)idata);
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_DIWCONF:
		if (writeflag == MEM_WRITE) {
			if ((idata & DIWCONF_MAGIC_MASK) !=
			    DIWCONF_MAGIC && (idata & DIWCONF_MAGIC_MASK)
			    != 0) {
				fatal("PVRREG_DIWCONF magic not set to "
				    "Magic value. 0x%08x\n", (int)idata);
				exit(1);
			}
			if (idata & DIWCONF_BLANK)
				debug("[ pvr: PVRREG_DIWCONF: BLANK: TODO ]\n");

			DEFAULT_WRITE;
			pvr_geometry_updated(d);
		}
		break;

	case PVRREG_DIWHSTRT:
		if (writeflag == MEM_WRITE) {
			int v = idata & DIWVSTRT_HPOS_MASK;
			debug("[ pvr: DIWHSTRT hpos=%i (%s) ]\n",
			    v, v == 174? "PAL" :
			    (v == 164? "NTSC" :
			    (v == 144? "VGA" : "unknown!")));
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_DIWVSTRT:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: DIWVSTRT v2=%i v1=%i ]\n",
			    (int)((idata & DIWVSTRT_V2_MASK)
			    >> DIWVSTRT_V2_SHIFT),
			    (int)(idata & DIWVSTRT_V1_MASK));
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_SCALER_CFG:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: SCALER_CFG 0x%08x ]\n", (int)idata);
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_SYNC_STAT:
		/*  TODO. Ugly hack, but it works:  */
		odata = random();
		break;

	case PVRREG_MAGIC_110:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: MAGIC_110 set to 0x%08"PRIx32,
			    (int) idata);
			if (idata != MAGIC_110_VALUE)
				fatal("{ MAGIC_110 = 0x%08"PRIx32" is not "
				    "yet implemented! }", (int) idata);
			debug(" ]\n");
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_TA_LUMINANCE:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: TA_LUMINANCE set to 0x%08"PRIx32" ]\n",
			    (int) idata);
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_TA_OPB_START:
		if (writeflag == MEM_WRITE) {
			if (idata & ~TA_OPB_START_MASK) {
				fatal("[ pvr: UNEXPECTED bits in "
				    "TA_OPB_START: 0x%08x ]\n", (int)idata);
				exit(1);
			}
			idata &= TA_OPB_START_MASK;
			debug("[ pvr: TA_OPB_START set to 0x%x ]\n",
			    (int) idata);
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_TA_OB_START:
		if (writeflag == MEM_WRITE) {
			if (idata & ~TA_OB_START_MASK) {
				fatal("[ pvr: UNEXPECTED bits in "
				    "TA_OB_START: 0x%08x ]\n", (int)idata);
				exit(1);
			}
			idata &= TA_OB_START_MASK;
			debug("[ pvr: TA_OB_START set to 0x%x ]\n",
			    (int) idata);
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_TA_OPB_END:
		if (writeflag == MEM_WRITE) {
			idata &= TA_OPB_END_MASK;
			debug("[ pvr: TA_OPB_END set to 0x%x ]\n",
			    (int) idata);
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_TA_OB_END:
		if (writeflag == MEM_WRITE) {
			idata &= TA_OB_END_MASK;
			debug("[ pvr: TA_OB_END set to 0x%x ]\n",
			    (int) idata);
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_TA_OPB_POS:
		if (writeflag == MEM_WRITE) {
			idata &= TA_OPB_POS_MASK;
			debug("[ pvr: TA_OPB_POS set to 0x%x ]\n",
			    (int) idata);
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_TA_OB_POS:
		if (writeflag == MEM_WRITE) {
			idata &= TA_OB_POS_MASK;
			debug("[ pvr: TA_OB_POS set to 0x%x ]\n",
			    (int) idata);
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_TA_OPL_INIT:
		if (writeflag == MEM_WRITE) {
			idata &= PVR_TA_OPL_INIT_MASK;
			debug("[ pvr: TA_OPL_INIT set to 0x%x ]\n",
			    (int) idata);
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_TILEBUF_SIZE:
		if (writeflag == MEM_WRITE) {
			d->tilebuf_ysize = (idata & TILEBUF_SIZE_HEIGHT_MASK)
			    >> TILEBUF_SIZE_HEIGHT_SHIFT;
			d->tilebuf_xsize = idata & TILEBUF_SIZE_WIDTH_MASK;
			d->tilebuf_xsize ++; d->tilebuf_ysize ++;
			debug("[ pvr: TILEBUF_SIZE set to %i x %i ]\n",
			    d->tilebuf_xsize, d->tilebuf_ysize);
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_TA_OPB_CFG:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: TA_OPB_CFG set to 0x%x ]\n",
			    (int) idata);
			DEFAULT_WRITE;
		}
		break;

	case PVRREG_TA_INIT:
		if (writeflag == MEM_WRITE) {
			debug("[ pvr: TA_INIT ]\n");

			if (idata & PVR_TA_INIT)
				pvr_ta_init(cpu, d);

			if (idata != PVR_TA_INIT && idata != 0)
				fatal("{ TA_INIT = 0x%08"PRIx32" is not "
				    "yet implemented! }", (int) idata);

			/*  Always reset to 0.  */
			idata = 0;
			DEFAULT_WRITE;
		}
		break;

	default:if (writeflag == MEM_READ) {
			fatal("[ pvr: read from UNIMPLEMENTED addr 0x%x ]\n",
			    (int)relative_addr);
		} else {
			fatal("[ pvr: write to UNIMPLEMENTED addr 0x%x: 0x%x"
			    " ]\n", (int)relative_addr, (int)idata);
			DEFAULT_WRITE;
		}

		exit(1);
	}

return_ok:
	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


void pvr_extend_update_region(struct pvr_data *d, uint64_t low, 
	uint64_t high)
{
	int vram_ofs = REG(PVRREG_DIWADDRL);
	int bytes_per_line = d->xsize * d->bytes_per_pixel;

	low -= vram_ofs;
	high -= vram_ofs;

	/*  Access inside visible part of VRAM?  */
	if ((int64_t)high >= 0 && (int64_t)low <
	    bytes_per_line * d->ysize) {
		int new_y1, new_y2;

		d->fb_update_x1 = 0;
		d->fb_update_x2 = d->xsize - 1;

		/*  Calculate which line the low and high addresses
		    correspond to:  */
		new_y1 = low / bytes_per_line;
		new_y2 = high / bytes_per_line + 1;

		if (d->fb_update_y1 < 0 || new_y1 < d->fb_update_y1)
			d->fb_update_y1 = new_y1;
		if (d->fb_update_y2 < 0 || new_y2 > d->fb_update_y2)
			d->fb_update_y2 = new_y2;

		if (d->fb_update_y1 < 0)
			d->fb_update_y1 = 0;
		if (d->fb_update_y2 >= d->ysize)
			d->fb_update_y2 = d->ysize - 1;
	}
}


DEVICE_TICK(pvr_fb)
{
	struct pvr_data *d = (struct pvr_data *) extra;
	uint64_t high, low = (uint64_t)(int64_t) -1;
	int vram_ofs = REG(PVRREG_DIWADDRL), pixels_to_copy;
	int y, bytes_per_line = d->xsize * d->bytes_per_pixel;
	int fb_ofs, p;
	uint8_t *fb = (uint8_t *) d->fb->framebuffer;
	uint8_t *vram = (uint8_t *) d->vram;


	/*
	 *  Vertical retrace interrupts:
	 *
	 *  TODO: Maybe it would be even more realistic to have the timer run
	 *        at, say, 60*4 = 240 Hz, and have the following events:
	 *
	 *	  (tick & 3) == 0	SYSASIC_EVENT_VBLINT
	 *	  (tick & 3) == 1	SYSASIC_EVENT_PVR_SCANINT1
	 *	  (tick & 3) == 2	nothing
	 *	  (tick & 3) == 3	SYSASIC_EVENT_PVR_SCANINT2
	 */

	if (d->vblank_interrupts_pending > 0) {
		SYSASIC_TRIGGER_EVENT(SYSASIC_EVENT_VBLINT);
		SYSASIC_TRIGGER_EVENT(SYSASIC_EVENT_PVR_SCANINT1);
		SYSASIC_TRIGGER_EVENT(SYSASIC_EVENT_PVR_SCANINT2);

		/*  TODO: For now, I don't care about missed interrupts:  */
		d->vblank_interrupts_pending = 0;
	}


	/*
	 *  Framebuffer update:
	 */

	/*  Border changed?  */
	if (d->border_updated) {
		/*  Fill border with border color:  */
		int rgb = REG(PVRREG_BRDCOLR), addr = 0;
		int x, y, b = rgb & 0xff, g = (rgb >> 8) & 0xff, r = rgb >> 16;
		int skiplen = (d->fb->xsize-2*PVR_MARGIN) * d->fb->bit_depth/8;

		for (y=0; y<d->fb->ysize; y++) {
			int xskip = y < PVR_MARGIN || y >=
			    d->fb->ysize - PVR_MARGIN? -1 : PVR_MARGIN;
			for (x=0; x<d->fb->xsize; x++) {
				if (x == xskip) {
					x = d->fb->xsize - PVR_MARGIN;
					addr += skiplen;
				}
				fb[addr] = r;
				fb[addr+1] = g;
				fb[addr+2] = b;
				addr += 3;
			}
		}

		/*  Full redraw of the framebuffer:  */
		d->fb->update_x1 = 0; d->fb->update_x2 = d->fb->xsize - 1;
		d->fb->update_y1 = 0; d->fb->update_y2 = d->fb->ysize - 1;
	}

	memory_device_dyntrans_access(cpu, cpu->mem, extra, &low, &high);
	if ((int64_t)low != -1)
		pvr_extend_update_region(d, low, high);

	if (d->fb_update_x1 == -1)
		return;

	/*  Copy (part of) the VRAM to the framebuffer:  */
	if (d->fb_update_x2 >= d->xsize)
		d->fb_update_x2 = d->xsize - 1;
	if (d->fb_update_y2 >= d->ysize)
		d->fb_update_y2 = d->ysize - 1;

	vram_ofs += d->fb_update_y1 * bytes_per_line;
	vram_ofs += d->fb_update_x1 * d->bytes_per_pixel;
	pixels_to_copy = (d->fb_update_x2 - d->fb_update_x1 + 1);
	fb_ofs = (d->fb_update_y1 + PVR_MARGIN) * d->fb->bytes_per_line;
	fb_ofs += (d->fb_update_x1 + PVR_MARGIN) * d->fb->bit_depth / 8;

	/*  Copy the actual pixels: (Four manually inlined, for speed.)  */

	switch (d->pixelmode) {
	case 0:	/*  RGB0555 (16-bit)  */
		for (y=d->fb_update_y1; y<=d->fb_update_y2; y++) {
			int fo = fb_ofs, vo = vram_ofs;
			for (p=0; p<pixels_to_copy; p++) {
				/*  0rrrrrgg(high) gggbbbbb(low)  */
				fb[fo] = (vram[(vo+1)%VRAM_SIZE] << 1) & 0xf8;
				fb[fo+1] = ((vram[vo%VRAM_SIZE] >> 2) & 0x38) +
				    (vram[(vo+1)%VRAM_SIZE] << 6);
				fb[fo+2] = (vram[vo%VRAM_SIZE] & 0x1f) << 3;
				fo += 3; vo += 2;
			}
			vram_ofs += bytes_per_line;
			fb_ofs += d->fb->bytes_per_line;
		}
		break;

	case 1: /*  RGB565 (16-bit)  */
		for (y=d->fb_update_y1; y<=d->fb_update_y2; y++) {
			int fo = fb_ofs, vo = vram_ofs;
			for (p=0; p<pixels_to_copy; p++) {
				/*  rrrrrggg(high) gggbbbbb(low)  */
				fb[fo] = vram[(vo+1)%VRAM_SIZE] & 0xf8;
				fb[fo+1] = ((vram[vo%VRAM_SIZE] >> 3) & 0x1c) +
				    (vram[(vo+1)%VRAM_SIZE] << 5);
				fb[fo+2] = (vram[vo%VRAM_SIZE] & 0x1f) << 3;
				fo += 3; vo += 2;
			}
			vram_ofs += bytes_per_line;
			fb_ofs += d->fb->bytes_per_line;
		}
		break;

	case 2: /*  RGB888 (24-bit)  */
		for (y=d->fb_update_y1; y<=d->fb_update_y2; y++) {
			/*  TODO: Reverse colors, like in the 32-bit case?  */
			memcpy(fb+fb_ofs, vram+(vram_ofs%VRAM_SIZE), 3*pixels_to_copy);
			vram_ofs += bytes_per_line;
			fb_ofs += d->fb->bytes_per_line;
		}
		break;

	case 3: /*  RGB0888 (32-bit)  */
		for (y=d->fb_update_y1; y<=d->fb_update_y2; y++) {
			int fo = fb_ofs, vo = vram_ofs;
			for (p=0; p<pixels_to_copy; p++) {
				fb[fo] = vram[(vo+2)%VRAM_SIZE];
				fb[fo+1] = vram[(vo+1)%VRAM_SIZE];
				fb[fo+2] = vram[(vo+0)%VRAM_SIZE];
				fo += 3; vo += 4;
			}
			vram_ofs += bytes_per_line;
			fb_ofs += d->fb->bytes_per_line;
		}
		break;
	}

	/*
	 *  Extend the real framebuffer to encompass the area
	 *  just written to:
	 */

	/*  Offset to take the margin into account first...  */
	d->fb_update_x1 += PVR_MARGIN; d->fb_update_y1 += PVR_MARGIN;
	d->fb_update_x2 += PVR_MARGIN; d->fb_update_y2 += PVR_MARGIN;

	if (d->fb_update_x1 < d->fb->update_x1 || d->fb->update_x1 < 0)
		d->fb->update_x1 = d->fb_update_x1;
	if (d->fb_update_x2 > d->fb->update_x2 || d->fb->update_x2 < 0)
		d->fb->update_x2 = d->fb_update_x2;
	if (d->fb_update_y1 < d->fb->update_y1 || d->fb->update_y1 < 0)
		d->fb->update_y1 = d->fb_update_y1;
	if (d->fb_update_y2 > d->fb->update_y2 || d->fb->update_y2 < 0)
		d->fb->update_y2 = d->fb_update_y2;

	/*  Clear the PVR's update region:  */
	d->fb_update_x1 = d->fb_update_x2 =
	    d->fb_update_y1 = d->fb_update_y2 = -1;
}


DEVICE_ACCESS(pvr_vram_alt)
{
	struct pvr_data_alt *d_alt = (struct pvr_data_alt *) extra;
	struct pvr_data *d = d_alt->d;
	size_t i;

	if (writeflag == MEM_READ) {
		/*  Copy from real vram:  */
		for (i=0; i<len; i++) {
			int addr = relative_addr + i;
			addr = ((addr & 4) << 20) | (addr & 3)
			    | ((addr & 0x7ffff8) >> 1);
			data[i] = d->vram[addr % VRAM_SIZE];
		}
		return 1;
	}

	/*
	 *  Convert writes to alternative VRAM, into normal writes:
	 */

	for (i=0; i<len; i++) {
		int addr = relative_addr + i;
		addr = ((addr & 4) << 20) | (addr & 3)
		    | ((addr & 0x7ffff8) >> 1);
		d->vram[addr % VRAM_SIZE] = data[i];
	}

	return 1;
}


DEVICE_ACCESS(pvr_vram)
{
	struct pvr_data *d = (struct pvr_data *) extra;

	if (writeflag == MEM_READ) {
		memcpy(data, d->vram + relative_addr, len);
		return 1;
	}

	/*
	 *  Write to VRAM:
	 *
	 *  Calculate which part of the framebuffer this write corresponds to,
	 *  if any, and increase the update region to encompass the written
	 *  memory range.
	 */

	memcpy(d->vram + relative_addr, data, len);
	pvr_extend_update_region(d, relative_addr, relative_addr + len - 1);

	return 1;
}


DEVINIT(pvr)
{
	struct machine *machine = devinit->machine;
	struct pvr_data *d;
	struct pvr_data_alt *d_alt;

	CHECK_ALLOCATION(d = (struct pvr_data *) malloc(sizeof(struct pvr_data)));
	memset(d, 0, sizeof(struct pvr_data));

	CHECK_ALLOCATION(d_alt = (struct pvr_data_alt *) malloc(sizeof(struct pvr_data_alt)));
	memset(d_alt, 0, sizeof(struct pvr_data_alt));

	d_alt->d = d;

	memory_device_register(machine->memory, devinit->name,
	    PVRREG_REGSTART, PVRREG_REGSIZE, dev_pvr_access, d,
	    DM_DEFAULT, NULL);

	/*  8 MB video RAM:  */
	d->vram = (uint8_t *) zeroed_alloc(VRAM_SIZE);
	memory_device_register(machine->memory, "pvr_vram", 0x05000000,
	    VRAM_SIZE, dev_pvr_vram_access, (void *)d,
	    DM_DYNTRANS_OK | DM_DYNTRANS_WRITE_OK
	    | DM_READS_HAVE_NO_SIDE_EFFECTS, d->vram);

	/*  8 MB video RAM, when accessed at 0xa4000000:  */
	memory_device_register(machine->memory, "pvr_alt_vram", 0x04000000,
	    VRAM_SIZE, dev_pvr_vram_alt_access, (void *)d_alt,
	    DM_DEFAULT, NULL);

	/*  Tile Accelerator command area at 0x10000000:  */
	memory_device_register(machine->memory, "pvr_ta",
	    0x10000000, sizeof(d->ta), dev_pvr_ta_access, d, DM_DEFAULT, NULL);

	/*  PVR2 DMA registers at 0x5f6800:  */
	memory_device_register(machine->memory, "pvr_dma", 0x005f6800,
	    PVR_DMA_MEMLENGTH, dev_pvr_dma_access, d, DM_DEFAULT, NULL);

	d->xsize = 640;
	d->ysize = 480;
	d->pixelmode = 1;	/*  RGB565  */
	d->bytes_per_pixel = 2;

	d->fb = dev_fb_init(machine, machine->memory, INTERNAL_FB_ADDR,
	    VFB_GENERIC, d->xsize + PVR_MARGIN*2, d->ysize + PVR_MARGIN*2,
	    d->xsize + PVR_MARGIN*2, d->ysize + PVR_MARGIN*2,
	    24, "Dreamcast PVR");

	d->vblank_timer = timer_add(PVR_VBLANK_HZ, pvr_vblank_timer_tick, d);

	pvr_reset(d);
	pvr_reset_ta(d);

	machine_add_tickfunction(machine, dev_pvr_fb_tick, d,
	    PVR_FB_TICK_SHIFT);

	return 1;
}

