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
 *  COMMENT: Generic framebuffer device
 *
 *	DECstation VFB01 monochrome framebuffer, 1024x864
 *	DECstation VFB02 8-bit color framebuffer, 1024x864
 *	DECstation Maxine, 1024x768 8-bit color
 *	Playstation 2 (24-bit color)
 *	Generic (any resolution, several bit depths possible, useful for
 *		testmachines)
 *
 *
 *  TODO:  This should actually be independent of X11, but that
 *         might be too hard to do right now.
 *
 *  TODO:  playstation 2 pixels are stored in another format, actually
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
#include "x11.h"

#ifdef WITH_X11
#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xutil.h>
#endif


#define	FB_TICK_SHIFT		19


/*  #define FB_DEBUG  */

/*
 *  set_grayscale_palette():
 *
 *  Fill d->rgb_palette with grayscale values. ncolors should
 *  be something like 2, 4, 16, or 256.
 */
void set_grayscale_palette(struct vfb_data *d, int ncolors)
{
	int i, gray;

	for (i=0; i<256; i++) {
		gray = 255*i/(ncolors-1);
		d->rgb_palette[i*3 + 0] = gray;
		d->rgb_palette[i*3 + 1] = gray;
		d->rgb_palette[i*3 + 2] = gray;
	}
}


/*
 *  set_blackwhite_palette():
 *
 *  Set color 0 = black, all others to white.
 */
void set_blackwhite_palette(struct vfb_data *d, int ncolors)
{
	int i, gray;

	for (i=0; i<256; i++) {
		gray = i==0? 0 : 255;
		d->rgb_palette[i*3 + 0] = gray;
		d->rgb_palette[i*3 + 1] = gray;
		d->rgb_palette[i*3 + 2] = gray;
	}
}


static void set_title(struct vfb_data *d)
{
	snprintf(d->title, sizeof(d->title),"GXemul: %ix%ix%i %s framebuffer",
	    d->visible_xsize, d->visible_ysize, d->bit_depth, d->name);
	d->title[sizeof(d->title)-1] = '\0';
}


/*
 *  dev_fb_resize():
 *
 *  Resize a framebuffer window. (This functionality is probably a bit buggy,
 *  because I didn't think of including it from the start.)
 *
 *  SUPER-IMPORTANT: Anyone who resizes a framebuffer by calling this function
 *  must also clear all dyntrans address translations manually, in all cpus
 *  which might have access to the framebuffer!
 */
void dev_fb_resize(struct vfb_data *d, int new_xsize, int new_ysize)
{
	unsigned char *new_framebuffer;
	int y, new_bytes_per_line;
	size_t size;

	if (d == NULL) {
		fatal("dev_fb_resize(): d == NULL\n");
		return;
	}

	if (new_xsize < 10 || new_ysize < 10) {
		fatal("dev_fb_resize(): size too small.\n");
		exit(1);
	}

	new_bytes_per_line = new_xsize * d->bit_depth / 8;
	size = new_ysize * new_bytes_per_line;

	CHECK_ALLOCATION(new_framebuffer = (unsigned char *) malloc(size));

	/*  Copy the old framebuffer to the new:  */
	if (d->framebuffer != NULL) {
		for (y=0; y<new_ysize; y++) {
			size_t fromofs = d->bytes_per_line * y;
			size_t toofs = new_bytes_per_line * y;
			size_t len_to_copy = d->bytes_per_line <
			    new_bytes_per_line? d->bytes_per_line	
			    : new_bytes_per_line;
			memset(new_framebuffer + toofs, 0, new_bytes_per_line);
			if (y < d->x11_ysize)
				memmove(new_framebuffer + toofs,
				    d->framebuffer + fromofs, len_to_copy);
		}

		free(d->framebuffer);
	}

	d->framebuffer = new_framebuffer;
	d->framebuffer_size = size;

	if (new_xsize > d->xsize || new_ysize > d->ysize) {
		d->update_x1 = d->update_y1 = 0;
		d->update_x2 = new_xsize - 1;
		d->update_y2 = new_ysize - 1;
	}

	d->bytes_per_line = new_bytes_per_line;
	d->xsize = d->visible_xsize = new_xsize;
	d->ysize = d->visible_ysize = new_ysize;

	d->x11_xsize = d->xsize / d->vfb_scaledown;
	d->x11_ysize = d->ysize / d->vfb_scaledown;

	memory_device_update_data(d->memory, d, d->framebuffer);

	set_title(d);

#ifdef WITH_X11
	if (d->fb_window != NULL) {
		x11_fb_resize(d->fb_window, new_xsize, new_ysize);
		x11_set_standard_properties(d->fb_window, d->title);
	}
#endif
}


/*
 *  dev_fb_setcursor():
 */
void dev_fb_setcursor(struct vfb_data *d, int cursor_x, int cursor_y, int on,
	int cursor_xsize, int cursor_ysize)
{
	if (cursor_x < 0)
		cursor_x = 0;
	if (cursor_y < 0)
		cursor_y = 0;
	if (cursor_x + cursor_xsize >= d->xsize)
		cursor_x = d->xsize - cursor_xsize;
	if (cursor_y + cursor_ysize >= d->ysize)
		cursor_y = d->ysize - cursor_ysize;

#ifdef WITH_X11
	if (d->fb_window != NULL) {
		d->fb_window->cursor_x      = cursor_x;
		d->fb_window->cursor_y      = cursor_y;
		d->fb_window->cursor_on     = on;
		d->fb_window->cursor_xsize  = cursor_xsize;
		d->fb_window->cursor_ysize  = cursor_ysize;
	}
#endif

	/*  debug("dev_fb_setcursor(%i,%i, size %i,%i, on=%i)\n",
	    cursor_x, cursor_y, cursor_xsize, cursor_ysize, on);  */
}


/*
 *  framebuffer_blockcopyfill():
 *
 *  This function should be used by devices that are capable of doing
 *  block copy/fill.
 *
 *  If fillflag is non-zero, then fill_[rgb] should contain the color
 *  with which to fill. (In 8-bit mode, only fill_r is used.)
 *
 *  If fillflag is zero, copy mode is used, and from_[xy] should contain
 *  the offset on the framebuffer where we should copy from.
 *
 *  NOTE:  Overlapping copies are undefined!
 */
void framebuffer_blockcopyfill(struct vfb_data *d, int fillflag, int fill_r,
	int fill_g, int fill_b, int x1, int y1, int x2, int y2,
	int from_x, int from_y)
{
	int x, y;
	long from_ofs, dest_ofs, linelen;

	if (fillflag)
		debug("framebuffer_blockcopyfill(FILL, %i,%i, %i,%i, "
		    "color %i,%i,%i)\n", x1,y1, x2,y2, fill_r, fill_g, fill_b);
	else
		debug("framebuffer_blockcopyfill(COPY, %i,%i, %i,%i, from "
		    "%i,%i)\n", x1,y1, x2,y2, from_x,from_y);

	/*  Clip x:  */
	if (x1 < 0)		x1 = 0;
	if (x1 >= d->xsize)	x1 = d->xsize-1;
	if (x2 < 0)		x2 = 0;
	if (x2 >= d->xsize)	x2 = d->xsize-1;

	dest_ofs = d->bytes_per_line * y1 + (d->bit_depth/8) * x1;
	linelen = (x2-x1 + 1) * (d->bit_depth/8);
	/*  NOTE: linelen is nr of bytes, not pixels  */

	if (fillflag) {
		for (y=y1; y<=y2; y++) {
			if (y>=0 && y<d->ysize) {
				unsigned char *buf =
				    d->framebuffer + dest_ofs;

				if (d->bit_depth == 24) {
					for (x=0; x<linelen && x <
					    (int) sizeof(buf); x += 3) {
						buf[x] = fill_r;
						buf[x+1] = fill_g;
						buf[x+2] = fill_b;
					}
				} else if (d->bit_depth == 8) {
					memset(buf, fill_r, linelen);
				} else {
					fatal("Unimplemented bit-depth (%i)"
					    " for fb fill\n", d->bit_depth);
					exit(1);
				}
			}

			dest_ofs += d->bytes_per_line;
		}
	} else {
		from_ofs = d->bytes_per_line * from_y +
		    (d->bit_depth/8) * from_x;
		for (y=y1; y<=y2; y++) {
			if (y >= 0 && y < d->ysize) {
				if (from_y >= 0 && from_y < d->ysize)
					memmove(d->framebuffer + dest_ofs,
					    d->framebuffer + from_ofs, linelen);
				else
					memset(d->framebuffer + dest_ofs,
					    0, linelen);
			}
			from_y ++;
			from_ofs += d->bytes_per_line;
			dest_ofs += d->bytes_per_line;
		}
	}

	if (x1 < d->update_x1 || d->update_x1 == -1)	d->update_x1 = x1;
	if (x1 > d->update_x2 || d->update_x2 == -1)	d->update_x2 = x1;
	if (x2 < d->update_x1 || d->update_x1 == -1)	d->update_x1 = x2;
	if (x2 > d->update_x2 || d->update_x2 == -1)	d->update_x2 = x2;

	if (y1 < d->update_y1 || d->update_y1 == -1)	d->update_y1 = y1;
	if (y1 > d->update_y2 || d->update_y2 == -1)	d->update_y2 = y1;
	if (y2 < d->update_y1 || d->update_y1 == -1)	d->update_y1 = y2;
	if (y2 > d->update_y2 || d->update_y2 == -1)	d->update_y2 = y2;
}


#ifdef WITH_X11

#define	REDRAW	redraw_fallback
#include "fb_include.cc"
#undef REDRAW

#define FB_24
#define REDRAW	redraw_24
#include "fb_include.cc"
#undef REDRAW
#undef FB_24
#define FB_16
#define REDRAW	redraw_16
#include "fb_include.cc"
#undef FB_16
#undef REDRAW
#define FB_15
#define REDRAW	redraw_15
#include "fb_include.cc"
#undef REDRAW
#undef FB_15

#define FB_BO
#define FB_24
#define REDRAW	redraw_24_bo
#include "fb_include.cc"
#undef REDRAW
#undef FB_24
#define FB_16
#define REDRAW	redraw_16_bo
#include "fb_include.cc"
#undef FB_16
#undef REDRAW
#define FB_15
#define REDRAW	redraw_15_bo
#include "fb_include.cc"
#undef REDRAW
#undef FB_15
#undef FB_BO

#define FB_SCALEDOWN

#define	REDRAW	redraw_fallback_sd
#include "fb_include.cc"
#undef REDRAW

#define FB_24
#define REDRAW	redraw_24_sd
#include "fb_include.cc"
#undef REDRAW
#undef FB_24
#define FB_16
#define REDRAW	redraw_16_sd
#include "fb_include.cc"
#undef FB_16
#undef REDRAW
#define FB_15
#define REDRAW	redraw_15_sd
#include "fb_include.cc"
#undef REDRAW
#undef FB_15

#define FB_BO
#define FB_24
#define REDRAW	redraw_24_bo_sd
#include "fb_include.cc"
#undef REDRAW
#undef FB_24
#define FB_16
#define REDRAW	redraw_16_bo_sd
#include "fb_include.cc"
#undef FB_16
#undef REDRAW
#define FB_15
#define REDRAW	redraw_15_bo_sd
#include "fb_include.cc"
#undef REDRAW
#undef FB_15
#undef FB_BO

void (*redraw[2 * 4 * 2])(struct vfb_data *, int, int) = {
	redraw_fallback, redraw_fallback,
	redraw_15, redraw_15_bo,
	redraw_16, redraw_16_bo,
	redraw_24, redraw_24_bo,
	redraw_fallback_sd, redraw_fallback_sd,
	redraw_15_sd, redraw_15_bo_sd,
	redraw_16_sd, redraw_16_bo_sd,
	redraw_24_sd, redraw_24_bo_sd  };

#endif	/*  WITH_X11  */


DEVICE_TICK(fb)
{
	struct vfb_data *d = (struct vfb_data *) extra;
#ifdef WITH_X11
	int need_to_flush_x11 = 0;
	int need_to_redraw_cursor = 0;
#endif

	if (!cpu->machine->x11_md.in_use)
		return;

	do {
		uint64_t high, low = (uint64_t)(int64_t) -1;
		int x, y;

		memory_device_dyntrans_access(cpu, cpu->mem,
		    extra, &low, &high);
		if ((int64_t)low == -1)
			break;

		/*  printf("low=%016llx high=%016llx\n",
		    (long long)low, (long long)high);  */

		x = (low % d->bytes_per_line) * 8 / d->bit_depth;
		y = low / d->bytes_per_line;
		if (x < d->update_x1 || d->update_x1 == -1)
			d->update_x1 = x;
		if (x > d->update_x2 || d->update_x2 == -1)
			d->update_x2 = x;
		if (y < d->update_y1 || d->update_y1 == -1)
			d->update_y1 = y;
		if (y > d->update_y2 || d->update_y2 == -1)
			d->update_y2 = y;

		x = ((low+7) % d->bytes_per_line) * 8 / d->bit_depth;
		y = (low+7) / d->bytes_per_line;
		if (x < d->update_x1 || d->update_x1 == -1)
			d->update_x1 = x;
		if (x > d->update_x2 || d->update_x2 == -1)
			d->update_x2 = x;
		if (y < d->update_y1 || d->update_y1 == -1)
			d->update_y1 = y;
		if (y > d->update_y2 || d->update_y2 == -1)
			d->update_y2 = y;

		x = (high % d->bytes_per_line) * 8 / d->bit_depth;
		y = high / d->bytes_per_line;
		if (x < d->update_x1 || d->update_x1 == -1)
			d->update_x1 = x;
		if (x > d->update_x2 || d->update_x2 == -1)
			d->update_x2 = x;
		if (y < d->update_y1 || d->update_y1 == -1)
			d->update_y1 = y;
		if (y > d->update_y2 || d->update_y2 == -1)
			d->update_y2 = y;

		x = ((high+7) % d->bytes_per_line) * 8 / d->bit_depth;
		y = (high+7) / d->bytes_per_line;
		if (x < d->update_x1 || d->update_x1 == -1)
			d->update_x1 = x;
		if (x > d->update_x2 || d->update_x2 == -1)
			d->update_x2 = x;
		if (y < d->update_y1 || d->update_y1 == -1)
			d->update_y1 = y;
		if (y > d->update_y2 || d->update_y2 == -1)
			d->update_y2 = y;

		/*
		 *  An update covering more than one line will automatically
		 *  force an update of all the affected lines:
		 */
		if (d->update_y1 != d->update_y2) {
			d->update_x1 = 0;
			d->update_x2 = d->xsize-1;
		}
	} while (0);

#ifdef WITH_X11
	/*  Do we need to redraw the cursor?  */
	if (d->fb_window->cursor_on != d->fb_window->OLD_cursor_on ||
	    d->fb_window->cursor_x != d->fb_window->OLD_cursor_x ||
	    d->fb_window->cursor_y != d->fb_window->OLD_cursor_y ||
	    d->fb_window->cursor_xsize != d->fb_window->OLD_cursor_xsize ||
	    d->fb_window->cursor_ysize != d->fb_window->OLD_cursor_ysize)
		need_to_redraw_cursor = 1;

	if (d->update_x2 != -1) {
		if (((d->update_x1 >= d->fb_window->OLD_cursor_x &&
		      d->update_x1 < (d->fb_window->OLD_cursor_x +
		      d->fb_window->OLD_cursor_xsize)) ||
		     (d->update_x2 >= d->fb_window->OLD_cursor_x &&
		      d->update_x2 < (d->fb_window->OLD_cursor_x +
		      d->fb_window->OLD_cursor_xsize)) ||
		     (d->update_x1 <  d->fb_window->OLD_cursor_x &&
		      d->update_x2 >= (d->fb_window->OLD_cursor_x +
		      d->fb_window->OLD_cursor_xsize)) ) &&
		   ( (d->update_y1 >= d->fb_window->OLD_cursor_y &&
		      d->update_y1 < (d->fb_window->OLD_cursor_y +
		      d->fb_window->OLD_cursor_ysize)) ||
		     (d->update_y2 >= d->fb_window->OLD_cursor_y &&
		      d->update_y2 < (d->fb_window->OLD_cursor_y +
		      d->fb_window->OLD_cursor_ysize)) ||
		     (d->update_y1 <  d->fb_window->OLD_cursor_y &&
		      d->update_y2 >= (d->fb_window->OLD_cursor_y +
		     d->fb_window->OLD_cursor_ysize)) ) )
			need_to_redraw_cursor = 1;
	}

	if (need_to_redraw_cursor) {
		/*  Remove old cursor, if any:  */
		if (d->fb_window->OLD_cursor_on) {
			XPutImage(d->fb_window->x11_display,
			    d->fb_window->x11_fb_window,
			    d->fb_window->x11_fb_gc, d->fb_window->fb_ximage,
			    d->fb_window->OLD_cursor_x/d->vfb_scaledown,
			    d->fb_window->OLD_cursor_y/d->vfb_scaledown,
			    d->fb_window->OLD_cursor_x/d->vfb_scaledown,
			    d->fb_window->OLD_cursor_y/d->vfb_scaledown,
			    d->fb_window->OLD_cursor_xsize/d->vfb_scaledown + 1,
			    d->fb_window->OLD_cursor_ysize/d->vfb_scaledown +1);
		}
	}
#endif

	if (d->update_x2 != -1) {
#ifdef WITH_X11
		int y;
#endif
		int addr, addr2, q = d->vfb_scaledown;

		if (d->update_x1 >= d->visible_xsize)
			d->update_x1 = d->visible_xsize - 1;
		if (d->update_x2 >= d->visible_xsize)
			d->update_x2 = d->visible_xsize - 1;
		if (d->update_y1 >= d->visible_ysize)
			d->update_y1 = d->visible_ysize - 1;
		if (d->update_y2 >= d->visible_ysize)
			d->update_y2 = d->visible_ysize - 1;

		/*  Without these, we might miss the rightmost/bottom pixel:  */
		d->update_x2 += (q - 1);
		d->update_y2 += (q - 1);

		d->update_x1 = d->update_x1 / q * q;
		d->update_x2 = d->update_x2 / q * q;
		d->update_y1 = d->update_y1 / q * q;
		d->update_y2 = d->update_y2 / q * q;

		addr  = d->update_y1 * d->bytes_per_line +
		    d->update_x1 * d->bit_depth / 8;
		addr2 = d->update_y1 * d->bytes_per_line +
		    d->update_x2 * d->bit_depth / 8;

#ifdef WITH_X11
		for (y=d->update_y1; y<=d->update_y2; y+=q) {
			d->redraw_func(d, addr, addr2 - addr);
			addr  += d->bytes_per_line * q;
			addr2 += d->bytes_per_line * q;
		}

		XPutImage(d->fb_window->x11_display, d->fb_window->
		    x11_fb_window, d->fb_window->x11_fb_gc, d->fb_window->
		    fb_ximage, d->update_x1/d->vfb_scaledown, d->update_y1/
		    d->vfb_scaledown, d->update_x1/d->vfb_scaledown,
		    d->update_y1/d->vfb_scaledown,
		    (d->update_x2 - d->update_x1)/d->vfb_scaledown + 1,
		    (d->update_y2 - d->update_y1)/d->vfb_scaledown + 1);

		need_to_flush_x11 = 1;
#endif

		d->update_x1 = d->update_y1 = 99999;
		d->update_x2 = d->update_y2 = -1;
	}

#ifdef WITH_X11
	if (need_to_redraw_cursor) {
		/*  Paint new cursor:  */
		if (d->fb_window->cursor_on) {
			x11_redraw_cursor(cpu->machine,
			    d->fb_window->fb_number);
			d->fb_window->OLD_cursor_on = d->fb_window->cursor_on;
			d->fb_window->OLD_cursor_x = d->fb_window->cursor_x;
			d->fb_window->OLD_cursor_y = d->fb_window->cursor_y;
			d->fb_window->OLD_cursor_xsize = d->fb_window->
			    cursor_xsize;
			d->fb_window->OLD_cursor_ysize = d->fb_window->
			    cursor_ysize;
			need_to_flush_x11 = 1;
		}
	}
#endif

#ifdef WITH_X11
	if (need_to_flush_x11)
		XFlush(d->fb_window->x11_display);
#endif
}


DEVICE_ACCESS(fb)
{
	struct vfb_data *d = (struct vfb_data *) extra;
	size_t i;

#ifdef FB_DEBUG
	if (writeflag == MEM_WRITE) { if (data[0]) {
		fatal("[ dev_fb: write  to addr=%08lx, data = ",
		    (long)relative_addr);
		for (i=0; i<len; i++)
			fatal("%02x ", data[i]);
		fatal("]\n");
	} else {
		fatal("[ dev_fb: read from addr=%08lx, data = ",
		    (long)relative_addr);
		for (i=0; i<len; i++)
			fatal("%02x ", d->framebuffer[relative_addr + i]);
		fatal("]\n");
	}
#endif

	if (relative_addr >= d->framebuffer_size)
		return 0;

	/*  See if a write actually modifies the framebuffer contents:  */
	if (writeflag == MEM_WRITE) {
		for (i=0; i<len; i++) {
			if (data[i] != d->framebuffer[relative_addr + i])
				break;

			/*  If all bytes are equal to what is already stored
			    in the framebuffer, then simply return:  */
			if (i == len-1)
				return 1;
		}
	}

	/*
	 *  If the framebuffer is modified, then we should keep a track
	 *  of which area(s) we modify, so that the display isn't updated
	 *  unnecessarily.
	 */
	if (writeflag == MEM_WRITE && cpu->machine->x11_md.in_use) {
		int x, y, x2,y2;

		x = (relative_addr % d->bytes_per_line) * 8 / d->bit_depth;
		y = relative_addr / d->bytes_per_line;
		x2 = ((relative_addr + len) % d->bytes_per_line)
		    * 8 / d->bit_depth;
		y2 = (relative_addr + len) / d->bytes_per_line;

		if (x < d->update_x1 || d->update_x1 == -1)
			d->update_x1 = x;
		if (x > d->update_x2 || d->update_x2 == -1)
			d->update_x2 = x;

		if (y < d->update_y1 || d->update_y1 == -1)
			d->update_y1 = y;
		if (y > d->update_y2 || d->update_y2 == -1)
			d->update_y2 = y;

		if (x2 < d->update_x1 || d->update_x1 == -1)
			d->update_x1 = x2;
		if (x2 > d->update_x2 || d->update_x2 == -1)
			d->update_x2 = x2;

		if (y2 < d->update_y1 || d->update_y1 == -1)
			d->update_y1 = y2;
		if (y2 > d->update_y2 || d->update_y2 == -1)
			d->update_y2 = y2;

		/*
		 *  An update covering more than one line will automatically
		 *  force an update of all the affected lines:
		 */
		if (y != y2) {
			d->update_x1 = 0;
			d->update_x2 = d->xsize-1;
		}
	}

	/*
	 *  Read from/write to the framebuffer:
	 *  (TODO: take the color_plane_mask into account)
	 *
	 *  Calling memcpy() is probably overkill, as it usually is just one
	 *  or a few bytes that are read/written at a time.
	 */
	if (writeflag == MEM_WRITE) {
		if (len > 8)
			memcpy(d->framebuffer + relative_addr, data, len);
		else {
			for (i=0; i<len; i++)
				d->framebuffer[relative_addr + i] = data[i];
		}
	} else {
		if (len > 8)
			memcpy(data, d->framebuffer + relative_addr, len);
		else {
			for (i=0; i<len; i++)
				data[i] = d->framebuffer[relative_addr + i];
		}
	}

	return 1;
}


/*
 *  dev_fb_init():
 *
 *  This function is big and ugly, but the point is to initialize a framebuffer
 *  device. :-)
 *
 *  visible_xsize and visible_ysize are the sizes of the visible display area.
 *  xsize and ysize tell how much memory is actually allocated (for example
 *  visible_xsize could be 640, but xsize could be 1024, for better alignment).
 *
 *  vfb_type is useful for selecting special features.
 *
 *  type = VFB_GENERIC is the most useful type, especially when bit_depth = 24.
 *
 *  VFB_DEC_VFB01, _VFB02, and VFB_DEC_MAXINE are DECstation specific.
 *
 *  VFB_HPC is like generic, but the color encoding is done as on HPCmips
 *  and Dreamcast.
 *
 *  If bit_depth = -15 (note the minus sign), then a special hack is used for
 *  the Playstation Portable's 5-bit R, 5-bit G, 5-bit B.
 */
struct vfb_data *dev_fb_init(struct machine *machine, struct memory *mem,
	uint64_t baseaddr, int vfb_type, int visible_xsize, int visible_ysize,
	int xsize, int ysize, int bit_depth, const char *name)
{
	struct vfb_data *d;
	size_t size, nlen;
	int flags;
	int reverse_start = 0;
	char *name2;

	CHECK_ALLOCATION(d = (struct vfb_data *) malloc(sizeof(struct vfb_data)));
	memset(d, 0, sizeof(struct vfb_data));

	if (vfb_type & VFB_REVERSE_START) {
		vfb_type &= ~VFB_REVERSE_START;
		reverse_start = 1;
	}

	d->memory = mem;
	d->vfb_type = vfb_type;

	/*  Defaults:  */
	d->xsize = xsize;  d->visible_xsize = visible_xsize;
	d->ysize = ysize;  d->visible_ysize = visible_ysize;

	d->bit_depth = bit_depth;

	if (bit_depth == 15) {
		d->color32k = 1;
		bit_depth = d->bit_depth = 16;
	} else if (bit_depth == -15) {
		d->psp_15bit = 1;
		bit_depth = d->bit_depth = 16;
	}

	/*  Specific types:  */
	switch (vfb_type) {
	case VFB_DEC_VFB01:
		/*  DECstation VFB01 (monochrome)  */
		d->xsize = 2048;  d->visible_xsize = 1024;
		d->ysize = 1024;  d->visible_ysize = 864;
		d->bit_depth = 1;
		break;
	case VFB_DEC_VFB02:
		/*  DECstation VFB02 (color)  */
		d->xsize = 1024;  d->visible_xsize = 1024;
		d->ysize = 1024;  d->visible_ysize = 864;
		d->bit_depth = 8;
		break;
	case VFB_DEC_MAXINE:
		/*  DECstation Maxine (1024x768x8)  */
		d->xsize = 1024; d->visible_xsize = d->xsize;
		d->ysize = 768;  d->visible_ysize = d->ysize;
		d->bit_depth = 8;
		break;
	case VFB_PLAYSTATION2:
		/*  Playstation 2  */
		d->xsize = xsize;  d->visible_xsize = d->xsize;
		d->ysize = ysize;  d->visible_ysize = d->ysize;
		d->bit_depth = 24;
		break;
	}

	if (d->bit_depth == 2 || d->bit_depth == 4)
		set_grayscale_palette(d, 1 << d->bit_depth);
	else if (d->bit_depth == 8 || d->bit_depth == 1)
		set_blackwhite_palette(d, 1 << d->bit_depth);

	d->vfb_scaledown = machine->x11_md.scaledown;

	d->bytes_per_line = d->xsize * d->bit_depth / 8;
	size = d->ysize * d->bytes_per_line;

	CHECK_ALLOCATION(d->framebuffer = (unsigned char *) malloc(size));

	/*  Clear the framebuffer (all black pixels):  */
	d->framebuffer_size = size;
	memset(d->framebuffer, reverse_start? 255 : 0, size);

	d->x11_xsize = d->visible_xsize / d->vfb_scaledown;
	d->x11_ysize = d->visible_ysize / d->vfb_scaledown;

	/*  Only "update" from the start if we need to fill with white.  */
	/*  (The Ximage will be black from the start anyway.)  */
	if (reverse_start) {
		d->update_x1 = d->update_y1 = 0;
		d->update_x2 = d->xsize - 1;
		d->update_y2 = d->ysize - 1;
	} else {
		d->update_x1 = d->update_y1 = 99999;
		d->update_x2 = d->update_y2 = -1;
	}

	CHECK_ALLOCATION(d->name = strdup(name));
	set_title(d);

#ifdef WITH_X11
	if (machine->x11_md.in_use) {
		int i = 0;
		d->fb_window = x11_fb_init(d->x11_xsize, d->x11_ysize,
		    d->title, machine->x11_md.scaledown, machine);
		switch (d->fb_window->x11_screen_depth) {
		case 15: i = 2; break;
		case 16: i = 4; break;
		case 24: i = 6; break;
		}
		if (d->fb_window->fb_ximage->byte_order)
			i ++;
		if (d->vfb_scaledown > 1)
			i += 8;
		d->redraw_func = redraw[i];
	} else
#endif
		d->fb_window = NULL;

	nlen = strlen(name) + 10;
	CHECK_ALLOCATION(name2 = (char *) malloc(nlen));

	snprintf(name2, nlen, "fb [%s]", name);

	flags = DM_DEFAULT;
	if ((baseaddr & 0xfff) == 0)
		flags = DM_DYNTRANS_OK | DM_DYNTRANS_WRITE_OK;

	flags |= DM_READS_HAVE_NO_SIDE_EFFECTS;

	memory_device_register(mem, name2, baseaddr, size, dev_fb_access,
	    d, flags, d->framebuffer);

	machine_add_tickfunction(machine, dev_fb_tick, d, FB_TICK_SHIFT);

	return d;
}

