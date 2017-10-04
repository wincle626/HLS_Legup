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
 *  COMMENT: Brooktree BT459, used by TURBOchannel graphics cards
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "devices.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"
#include "x11.h"

#include "thirdparty/bt459.h"


#ifdef WITH_X11
#include <X11/Xlib.h>     
#include <X11/Xutil.h>
#endif

extern int quiet_mode;


/*  #define BT459_DEBUG  */
/*  #define WITH_CURSOR_DEBUG  */
#define BT459_TICK_SHIFT	14

struct bt459_data {
	uint32_t	bt459_reg[DEV_BT459_NREGS];

	unsigned char	cur_addr_hi;
	unsigned char	cur_addr_lo;

	int		planes;
	int		type;

	struct interrupt irq;
	int		interrupts_enable;
	int		interrupt_time;
	int		interrupt_time_reset_value;

	int		cursor_x_add;
	int		cursor_y_add;

	int		need_to_redraw_whole_screen;

	int		need_to_update_cursor_shape;
	int		cursor_on;
	int		cursor_x;
	int		cursor_y;
	int		cursor_xsize;
	int		cursor_ysize;

	int		palette_sub_offset;	/*  0, 1, or 2  */

	struct vfb_data *vfb_data;

	/*
	 *  There is one pointer to the framebuffer's RGB palette,
	 *  and then a local copy of the palette.  256 * 3 bytes (r,g,b).
	 *  The reason for this is that when we need to blank the screen
	 *  (ie video_on = 0), we can set the framebuffer's palette to all
	 *  zeroes, but keep our own copy intact, to be reused later again
	 *  when the screen is unblanked.
	 */
	int		video_on;
	unsigned char	*rgb_palette;		/*  256 * 3 (r,g,b)  */
	unsigned char	local_rgb_palette[256 * 3];
};


/*
 *  bt459_update_X_cursor():
 *
 *  This routine takes the color values in the cursor RAM area, and put them
 *  in the framebuffer window's cursor_pixels.
 *
 *  d->cursor_xsize and ysize are also updated.
 */
static void bt459_update_X_cursor(struct cpu *cpu, struct bt459_data *d)
{
	int i, x,y, xmax=0, ymax=0;
	int bw_only = 1;

	/*  First, let's calculate the size of the cursor:  */
	for (y=0; y<64; y++)
		for (x=0; x<64; x+=4) {
			int reg = BT459_REG_CRAM_BASE + y*16 + x/4;
			unsigned char data = d->bt459_reg[reg];

			if (data)
				ymax = y;

			for (i=0; i<4; i++) {
				int color = (data >> (6-2*i)) & 3;
				if (color != 0)
					xmax = x + i;
				if (color != 0 && color != 3)
					bw_only = 0;
			}
		}

	d->cursor_xsize = xmax + 1;
	d->cursor_ysize = ymax + 1;

	/*
	 *  The 'bw_only' hack is because it is nicer to have the b/w
	 *  text cursor invert whatever it is standing on, not just overwrite
	 *  it with a big white box.
	 *
	 *  The following seems to work with NetBSD/OpenBSD/Ultrix/Sprite:
	 *	0 = transparent, 1 and 2 = use the color specified by
	 *	BT459_REG_CCOLOR_2, 3 = reverse of color 1/2.
	 */

#ifdef WITH_X11
	if (cpu->machine->x11_md.in_use && d->vfb_data->fb_window != NULL) {
		for (y=0; y<=ymax; y++) {
			for (x=0; x<=xmax; x+=4) {
				struct fb_window *win = d->vfb_data->fb_window;
				int reg = BT459_REG_CRAM_BASE + y*16 + x/4;
				unsigned char data = d->bt459_reg[reg];

				for (i=0; i<4; i++) {
					int color = (data >> (6-2*i)) & 3;
					int pixelvalue;

					if (bw_only) {
						if (color)
							pixelvalue =
							    CURSOR_COLOR_INVERT;
						else
							pixelvalue = 0;
					} else {
						pixelvalue =
						    CURSOR_COLOR_TRANSPARENT;
						switch (color) {
						case 1:
						case 2:	pixelvalue = (d->
							    bt459_reg[
							    BT459_REG_CCOLOR_2]
							    >> 4) & 0xf;
							break;
						case 3:	pixelvalue = 15 -
							    ((d->bt459_reg[
							    BT459_REG_CCOLOR_2]
							    >> 4) & 0xf);
							break;
						}
					}

					win->cursor_pixels[y][x+i] =
					    pixelvalue;
#ifdef WITH_CURSOR_DEBUG
printf("%i", color);
#endif
				}
			}
#ifdef WITH_CURSOR_DEBUG
printf("\n");
#endif
		}
#ifdef WITH_CURSOR_DEBUG
printf("color 1,2,3 = 0x%02x, 0x%02x, 0x%02x\n",
    d->bt459_reg[BT459_REG_CCOLOR_1],
    d->bt459_reg[BT459_REG_CCOLOR_2],
    d->bt459_reg[BT459_REG_CCOLOR_3]);
printf("\n");
#endif
		/*
		 *  Make sure the cursor is redrawn, if it is on:
		 *
		 *  How does this work? Well, 0 is off, and non-zero is on,
		 *  but if the old and new differ, the cursor is redrawn.
		 *  (Hopefully this will "never" overflow.)
		 */
		if (d->cursor_on)
			d->cursor_on ++;
	}
#endif
}


/*
 *  bt459_update_cursor_position():
 */
static void bt459_update_cursor_position(struct bt459_data *d,
	int old_cursor_on)
{
	int new_cursor_x = (d->bt459_reg[BT459_REG_CXLO] & 255) +
	    ((d->bt459_reg[BT459_REG_CXHI] & 255) << 8) - d->cursor_x_add;
	int new_cursor_y = (d->bt459_reg[BT459_REG_CYLO] & 255) +
	    ((d->bt459_reg[BT459_REG_CYHI] & 255) << 8) - d->cursor_y_add;

	if (new_cursor_x != d->cursor_x || new_cursor_y != d->cursor_y ||
	    d->cursor_on != old_cursor_on) {
		int on;

		d->cursor_x = new_cursor_x;
		d->cursor_y = new_cursor_y;

		if (!quiet_mode)
			debug("[ bt459: cursor = %03i,%03i ]\n",
			    d->cursor_x, d->cursor_y);

		on = d->cursor_on;
		if (d->cursor_xsize == 0 || d->cursor_ysize == 0)
			on = 0;

		dev_fb_setcursor(d->vfb_data, d->cursor_x, d->cursor_y,
		    on, d->cursor_xsize, d->cursor_ysize);
	}
}


DEVICE_TICK(bt459)
{
	struct bt459_data *d = (struct bt459_data *) extra;
	int old_cursor_on = d->cursor_on;

	if (d->need_to_update_cursor_shape) {
		d->need_to_update_cursor_shape = 0;
		bt459_update_X_cursor(cpu, d);
		bt459_update_cursor_position(d, old_cursor_on);
	}

	if (d->need_to_redraw_whole_screen) {
		d->vfb_data->update_x1 = 0;
		d->vfb_data->update_x2 = d->vfb_data->xsize - 1;
		d->vfb_data->update_y1 = 0;
		d->vfb_data->update_y2 = d->vfb_data->ysize - 1;
		d->need_to_redraw_whole_screen = 0;
	}

	/*
	 *  Vertical retrace interrupts. (This hack is kind of ugly.)
	 *  Once every 'interrupt_time_reset_value', the interrupt is
	 *  asserted. It is acked either manually (by someone reading
	 *  a normal BT459 register or the Interrupt ack register),
	 *  or after another tick has passed.  (This is to prevent
	 *  lockups from unhandled interrupts.)
	 */
	if (d->type != BT459_PX && d->interrupts_enable) {
		d->interrupt_time --;
		if (d->interrupt_time < 0) {
			d->interrupt_time = d->interrupt_time_reset_value;
			INTERRUPT_ASSERT(d->irq);
		} else
			INTERRUPT_DEASSERT(d->irq);
	}
}


DEVICE_ACCESS(bt459_irq)
{
	struct bt459_data *d = (struct bt459_data *) extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

#ifdef BT459_DEBUG
	fatal("[ bt459: IRQ ack ]\n");
#endif

	d->interrupts_enable = 1;

	INTERRUPT_DEASSERT(d->irq);

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVICE_ACCESS(bt459)
{
	struct bt459_data *d = (struct bt459_data *) extra;
	uint64_t idata = 0, odata = 0;
	int btaddr, old_cursor_on = d->cursor_on, modified;

	idata = memory_readmax64(cpu, data, len);

#ifdef BT459_DEBUG
	if (writeflag == MEM_WRITE)
		fatal("[ bt459: write to addr 0x%02x: %08x ]\n",
		    (int)relative_addr, (int)idata);
#endif

	/*
	 *  Vertical retrace interrupts are acked either by
	 *  accessing a normal BT459 register, or the irq register,
	 *  or by simply "missing" it.
	 */
	INTERRUPT_DEASSERT(d->irq);

	/*  ID register is read-only, should always be 0x4a or 0x4a4a4a:  */
	if (d->planes == 24)
		d->bt459_reg[BT459_REG_ID] = 0x4a4a4a;
	else {
		/*
		 *  TODO:  Is it really 0x4a, or 0x4a0000?
		 *  Ultrix panics with a "bad VDAC ID" message if 0x4a
		 *  is returned.
		 */
		d->bt459_reg[BT459_REG_ID] = 0x4a0000;
	}

	btaddr = ((d->cur_addr_hi << 8) + d->cur_addr_lo) % DEV_BT459_NREGS;

	/*  Read from/write to the bt459:  */
	switch (relative_addr) {
	case 0x00:		/*  Low byte of address:  */
		if (writeflag == MEM_WRITE) {
			if (!quiet_mode)
				debug("[ bt459: write to Low Address Byte, "
				    "0x%02x ]\n", (int)idata);
			d->cur_addr_lo = idata;
			d->palette_sub_offset = 0;
		} else {
			odata = d->cur_addr_lo;
			if (!quiet_mode)
				debug("[ bt459: read from Low Address Byte: "
				    "0x%0x ]\n", (int)odata);
		}
		break;
	case 0x04:		/*  High byte of address:  */
		if (writeflag == MEM_WRITE) {
			if (!quiet_mode)
				debug("[ bt459: write to High Address Byte, "
				    "0x%02x ]\n", (int)idata);
			d->cur_addr_hi = idata;
			d->palette_sub_offset = 0;
		} else {
			odata = d->cur_addr_hi;
			if (!quiet_mode)
				debug("[ bt459: read from High Address Byte: "
				    "0x%0x ]\n", (int)odata);
		}
		break;
	case 0x08:		/*  Register access:  */
		if (writeflag == MEM_WRITE) {
			if (!quiet_mode)
				debug("[ bt459: write to BT459 register "
				    "0x%04x, value 0x%02x ]\n", btaddr,
				    (int)idata);
			modified = (d->bt459_reg[btaddr] != idata);
			d->bt459_reg[btaddr] = idata;

			switch (btaddr) {
			case BT459_REG_CCOLOR_1:
			case BT459_REG_CCOLOR_2:
			case BT459_REG_CCOLOR_3:
				if (modified)
					d->need_to_update_cursor_shape = 1;
				break;
			case BT459_REG_PRM:
				/*
				 *  NetBSD writes 0x00 to this register to
				 *  blank the screen (video off), and 0xff
				 *  to turn the screen on.
				 */
				switch (idata & 0xff) {
				case 0:	d->video_on = 0;
					memset(d->rgb_palette, 0, 256*3);
					d->need_to_redraw_whole_screen = 1;
					debug("[ bt459: video OFF ]\n");
					break;
				default:d->video_on = 1;
					memcpy(d->rgb_palette,
					    d->local_rgb_palette, 256*3);
					d->need_to_redraw_whole_screen = 1;
					debug("[ bt459: video ON ]\n");
				}
				break;
			case BT459_REG_CCR:
				/*  Cursor control register:  */
				switch (idata & 0xff) {
				case 0x00:	d->cursor_on = 0; break;
				case 0xc0:
				case 0xc1:	d->cursor_on = 1; break;
				default:
					fatal("[ bt459: unimplemented CCR "
					    "value 0x%08x ]\n", (int)idata);
				}
				if (modified)
					d->need_to_update_cursor_shape = 1;
				break;
			default:
				if (btaddr < 0x100)
					fatal("[ bt459: write to BT459 "
					    "register 0x%04x, value 0x%02x ]\n",
					    btaddr, (int)idata);
			}

			/*  Write to cursor bitmap:  */
			if (btaddr >= BT459_REG_CRAM_BASE && modified)
				d->need_to_update_cursor_shape = 1;
		} else {
			odata = d->bt459_reg[btaddr];

			/*  Perhaps this hack is not necessary:  */
			if (btaddr == BT459_REG_ID && len==1)
				odata = (odata >> 16) & 255;

			if (!quiet_mode)
				debug("[ bt459: read from BT459 register "
				    "0x%04x, value 0x%02x ]\n", btaddr,
				    (int)odata);
		}

		/*  Go to next register:  */
		d->cur_addr_lo ++;
		if (d->cur_addr_lo == 0)
			d->cur_addr_hi ++;
		break;
	case 0xc:		/*  Color map:  */
		if (writeflag == MEM_WRITE) {
			idata &= 255;
			if (!quiet_mode)
				debug("[ bt459: write to BT459 colormap "
				    "0x%04x subaddr %i, value 0x%02x ]\n",
				    btaddr, d->palette_sub_offset, (int)idata);

			if (btaddr < 0x100) {
				if (d->video_on &&
				    d->local_rgb_palette[(btaddr & 0xff) * 3
				    + d->palette_sub_offset] != idata)
					d->need_to_redraw_whole_screen = 1;

				/*
				 *  Actually, the palette should only be
				 *  updated after the third write,
				 *  but this should probably work fine too:
				 */
				d->local_rgb_palette[(btaddr & 0xff) * 3
				    + d->palette_sub_offset] = idata;

				if (d->video_on)
					d->rgb_palette[(btaddr & 0xff) * 3
					    + d->palette_sub_offset] = idata;
			}
		} else {
			if (btaddr < 0x100)
				odata = d->local_rgb_palette[(btaddr & 0xff)
				    * 3 + d->palette_sub_offset];
			if (!quiet_mode)
				debug("[ bt459: read from BT459 colormap "
				    "0x%04x subaddr %i, value 0x%02x ]\n",
				    btaddr, d->palette_sub_offset, (int)odata);
		}

		d->palette_sub_offset ++;
		if (d->palette_sub_offset >= 3) {
			d->palette_sub_offset = 0;

			d->cur_addr_lo ++;
			if (d->cur_addr_lo == 0)
				d->cur_addr_hi ++;
		}

		break;
	default:
		if (writeflag == MEM_WRITE) {
			debug("[ bt459: unimplemented write to address 0x%x, "
			    "data=0x%02x ]\n", (int)relative_addr, (int)idata);
		} else {
			debug("[ bt459: unimplemented read from address "
			    "0x%x ]\n", (int)relative_addr);
		}
	}


	bt459_update_cursor_position(d, old_cursor_on);

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

#ifdef BT459_DEBUG
	if (writeflag == MEM_READ)
		fatal("[ bt459: read from addr 0x%02x: %08x ]\n",
		    (int)relative_addr, (int)idata);
#endif

	return 1;
}


/*
 *  dev_bt459_init():
 */
void dev_bt459_init(struct machine *machine, struct memory *mem,
	uint64_t baseaddr, uint64_t baseaddr_irq, struct vfb_data *vfb_data,
	int planes, const char *irq_path, int type)
{
	struct bt459_data *d;

	CHECK_ALLOCATION(d = (struct bt459_data *) malloc(sizeof(struct bt459_data)));
	memset(d, 0, sizeof(struct bt459_data));

	INTERRUPT_CONNECT(irq_path, d->irq);

	d->vfb_data     = vfb_data;
	d->rgb_palette  = vfb_data->rgb_palette;
	d->planes       = planes;
	d->type		= type;
	d->cursor_x     = -1;
	d->cursor_y     = -1;
	d->cursor_xsize = d->cursor_ysize = 0;	/*  anything  */
	d->video_on     = 1;

	/*
	 *  These offsets are based on those mentioned in NetBSD,
	 *  and then adjusted to look good with both NetBSD and
	 *  Ultrix:
	 */
	switch (d->type) {
	case BT459_PX:
		d->cursor_x_add = 370;
		d->cursor_y_add =  37;
		break;
	case BT459_BA:
		d->cursor_x_add = 220;
		d->cursor_y_add =  35;
		break;
	case BT459_BBA:
		if (vfb_data->xsize == 1280) {
			/*  1280x1024:  */
			d->cursor_x_add = 368;
			d->cursor_y_add =  38;
		} else {
			/*  1024x864:  */
			d->cursor_x_add = 220;
			d->cursor_y_add =  35;
		}
		break;
	}

	d->interrupt_time_reset_value = 500;

	memory_device_register(mem, "bt459", baseaddr, DEV_BT459_LENGTH,
	    dev_bt459_access, (void *)d, DM_DEFAULT, NULL);

	if (baseaddr_irq != 0)
		memory_device_register(mem, "bt459_irq", baseaddr_irq, 0x10000,
		    dev_bt459_irq_access, (void *)d, DM_DEFAULT, NULL);

	machine_add_tickfunction(machine, dev_bt459_tick, d,
	    BT459_TICK_SHIFT);
}

