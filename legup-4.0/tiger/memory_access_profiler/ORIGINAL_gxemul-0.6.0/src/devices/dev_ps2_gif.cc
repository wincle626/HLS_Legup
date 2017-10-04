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
 *  COMMENT: PlayStation 2 "gif" graphics device
 *
 *  TODO:  Convert dev_fb_access() accesses into direct framebuffer reads and
 *         writes, to improve performance.
 *
 *  TODO 2:  The way things are now, rgb bytes are copied from emulated
 *           space to the framebuffer as rgb, but on X Windows servers on
 *           big-endian machines that should be bgr.  (?) Hm...
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "devices.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"


#define	DEV_PS2_GIF_LENGTH		0x10000

#define	PS2_FB_ADDR	0x60000000ULL 	/*  hopefully nothing else here  */


struct gif_data {
	struct cpu	*cpu;
	int		xsize, ysize;
	int		bytes_per_pixel;
	int		transparent_text;
	struct vfb_data *vfb_data;
};


/*
 *  test_triangle():
 *
 *  Draw a triangle:
 */
void test_triangle(struct gif_data *d,
	int x1, int y1, int r1, int g1, int b1,
	int x2, int y2, int r2, int g2, int b2,
	int x3, int y3, int r3, int g3, int b3)
{
	unsigned char *line;
	int y, tmp, scale = 32768;
	int xofs, xlen, savedxlen, xdir, x;
	int r, g, b;			/*  scaled  */
	int xstart, xstop;		/*  scaled  */
	int rstart, rstop;		/*  scaled  */
	int gstart, gstop;		/*  scaled  */
	int bstart, bstop;		/*  scaled  */
	int rpx, gpx, bpx;		/*  scaled  */
	int xpy12, xpy13, xpy23;
	int rpy12, rpy13, rpy23;
	int gpy12, gpy13, gpy23;
	int bpy12, bpy13, bpy23;

	CHECK_ALLOCATION(line = (unsigned char *) malloc(d->xsize * d->bytes_per_pixel));

	if (y2 > y3) {
		tmp = x2; x2 = x3; x3 = tmp;
		tmp = y2; y2 = y3; y3 = tmp;
		tmp = r2; r2 = r3; r3 = tmp;
		tmp = g2; g2 = g3; g3 = tmp;
		tmp = b2; b2 = b3; b3 = tmp;
	}

	if (y1 > y2) {
		tmp = x1; x1 = x2; x2 = tmp;
		tmp = y1; y1 = y2; y2 = tmp;
		tmp = r1; r1 = r2; r2 = tmp;
		tmp = g1; g1 = g2; g2 = tmp;
		tmp = b1; b1 = b2; b2 = tmp;
	}

	if (y1 > y3) {
		tmp = x1; x1 = x3; x3 = tmp;
		tmp = y1; y1 = y3; y3 = tmp;
		tmp = r1; r1 = r3; r3 = tmp;
		tmp = g1; g1 = g3; g3 = tmp;
		tmp = b1; b1 = b3; b3 = tmp;
	}

	/*  X change per line:  */
	xpy12 = (y2-y1)? scale*(x2-x1)/(y2-y1) : 0;
	xpy13 = (y3-y1)? scale*(x3-x1)/(y3-y1) : 0;
	xpy23 = (y3-y2)? scale*(x3-x2)/(y3-y2) : 0;

	/*  Color change per line:  */
	rpy12 = (y2-y1)? scale*(r2-r1)/(y2-y1) : 0;
	rpy13 = (y3-y1)? scale*(r3-r1)/(y3-y1) : 0;
	rpy23 = (y3-y2)? scale*(r3-r2)/(y3-y2) : 0;

	gpy12 = (y2-y1)? scale*(g2-g1)/(y2-y1) : 0;
	gpy13 = (y3-y1)? scale*(g3-g1)/(y3-y1) : 0;
	gpy23 = (y3-y2)? scale*(g3-g2)/(y3-y2) : 0;

	bpy12 = (y2-y1)? scale*(b2-b1)/(y2-y1) : 0;
	bpy13 = (y3-y1)? scale*(b3-b1)/(y3-y1) : 0;
	bpy23 = (y3-y2)? scale*(b3-b2)/(y3-y2) : 0;

	xstart = xstop = x1 * scale;
	rstart = rstop = r1 * scale;
	gstart = gstop = g1 * scale;
	bstart = bstop = b1 * scale;

	for (y=y1; y<=y3; y++) {
		if (xstart < xstop)
			xofs = xstart/scale, xlen = (xstop-xstart)/scale + 1;
		else
			xofs = xstop/scale, xlen = (xstart-xstop)/scale + 1;

		savedxlen = xlen;
		xdir = (xstart < xstop)? 1 : -1;
		r = rstart; g = gstart; b = bstart;

		rpx = (xstop-xstart)? (rstop-rstart) / ((xstop-xstart)
		    / scale) : 0;
		gpx = (xstop-xstart)? (gstop-gstart) / ((xstop-xstart)
		    / scale) : 0;
		bpx = (xstop-xstart)? (bstop-bstart) / ((xstop-xstart)
		    / scale): 0;

		x = xstart / scale;
		while (xlen > 0) {
			if (x>=0 && x<d->xsize) {
				int c;
				c = r/scale;
				line[x * d->bytes_per_pixel + 0] = c<0?
				    0 : (c > 255? 255 : c);
				c = g/scale;
				line[x * d->bytes_per_pixel + 1] = c<0?
				    0 : (c > 255? 255 : c);
				c = b/scale;
				line[x * d->bytes_per_pixel + 2] = c<0?
				    0 : (c > 255? 255 : c);
			}
			r += rpx;
			g += gpx;
			b += bpx;
			x += xdir;
			xlen --;
		}

		dev_fb_access(d->cpu, d->cpu->mem,
		    (y*d->xsize + xofs) * d->bytes_per_pixel,
		    line + xofs * d->bytes_per_pixel,
		    savedxlen * d->bytes_per_pixel,
		    MEM_WRITE, d->vfb_data);

		if (y<y2) {
			xstart += xpy12;
			rstart += rpy12;
			gstart += gpy12;
			bstart += bpy12;
		} else {
			xstart += xpy23;
			rstart += rpy23;
			gstart += gpy23;
			bstart += bpy23;
		}
		xstop += xpy13;
		rstop += rpy13;
		gstop += gpy13;
		bstop += bpy13;

		if (y==y2) {
			xstart = x2 * scale;
			rstart = r2 * scale;
			gstart = g2 * scale;
			bstart = b2 * scale;
		}
	}
}


DEVICE_ACCESS(ps2_gif)
{
	unsigned int i;
	struct gif_data *d = (struct gif_data *) extra;

	if (relative_addr + len > DEV_PS2_GIF_LENGTH)
		return 0;

	if (writeflag==MEM_READ) {
		debug("[ gif read from addr 0x%x, len=%i ]\n",
		    (int)relative_addr, (int)len);
	} else {
		if (data[0] == 0x08 && data[1] == 0x80) {
			/*  Possibly "initialize 640x480 mode":  */
			debug("[ gif: initialize video mode (?) ]\n");
		} else if (data[0] == 0x04 && data[1] == 0x00 && len > 300) {
			/*  Possibly "output 8x16 character":  */
			int xbase, ybase, xsize, ysize, x, y;

			xbase = data[9*4 + 0] + (data[9*4 + 1] << 8);
			ybase = data[9*4 + 2] + (data[9*4 + 3] << 8);

			xsize = data[12*4 + 0] + (data[12*4 + 1] << 8);
			ysize = data[13*4 + 0] + (data[13*4 + 1] << 8);
			ysize &= ~0xf;	/*  multple of 16  */

			/*  debug("[ gif: putchar at (%i,%i), size (%i,%i) "
			    "]\n", xbase, ybase, xsize, ysize);  */

			/*
			 *  NetBSD and Linux:
			 *
			 *  [ gif write to addr 0x0 (len=608):
			 *  04 00 00 00 00 00 00 10, 0e 00 00 00 00 00 00 00,
			 *  00 00 00 00 00 00 0a 00, 50 00 00 00 00 00 00 00,
			 *  00 00 00 00 00 00 00 00, 51 00 00 00 00 00 00 00,
			 *  08 00 00 00 16 00 00 00, 52 00 00 00 00 00 00 00,
			 *  00 00 00 00 00 00 00 00, 53 00 00 00 00 00 00 00,
			 *  20 80 00 00 00 00 00 08, 00 00 00 00 00 00 00 00,
			 *  00 00 aa 80 00 00 aa 80, 00 00 aa 80 00 00 aa 80,
			 *  00 00 aa 80 00 00 aa 80, 00 00 aa 80 00 00 aa 80,
			 *  aa aa 00 80 aa aa 00 80, 00 00 aa 80 00 00 aa 80,
			 *  00 00 aa 80 00 00 aa 80, 00 00 aa 80 00 00 aa 80,
			 */

			/*
			fatal("[ gif write to addr 0x%x (len=%i):",
			    (int)relative_addr, (int)len);
			for (i=0; i<len; i++) {
				fatal(" %02x", data[i]);
				if ((i & 7) == 7)
					fatal(",");
				if ((i & 31) == 31)
					fatal("\n");
			}
			fatal(" ]\n");
			*/

			for (y=0; y<ysize; y++) {
				int fb_addr = (xbase + (ybase+y) * d->xsize)
				    * d->bytes_per_pixel;
				int addr = (24 + y*xsize) * 4;
				for (x=0; x<xsize; x++) {
					/*  There are three bytes (r,g,b) at
					    data[addr + 0] .. [addr + 2].
					    TODO: This should be translated to a
					    direct update of the framebuffer. */

					dev_fb_access(d->cpu, d->cpu->mem,
					    fb_addr, data + addr, 3, MEM_WRITE,
					    d->vfb_data);

					fb_addr += d->bytes_per_pixel;
					addr += 4;
				}
			}
		} else if (data[0] == 0x04 && data[1] == 0x80 && len == 0x50) {
			/*  blockcopy  */
			int y_source, y_dest, x_source, x_dest, x_size, y_size;
			x_source = data[8*4 + 0] + ((data[8*4 + 1]) << 8);
			y_source = data[8*4 + 2] + ((data[8*4 + 3]) << 8);
			x_dest   = data[9*4 + 0] + ((data[9*4 + 1]) << 8);
			y_dest   = data[9*4 + 2] + ((data[9*4 + 3]) << 8);
			x_size   = data[12*4 + 0] + ((data[12*4 + 1]) << 8);
			y_size   = data[13*4 + 0] + ((data[13*4 + 1]) << 8);

			/*  debug("[ gif: blockcopy (%i,%i) -> (%i,%i), size="
			    "(%i,%i) ]\n", x_source,y_source, x_dest,y_dest,
			    x_size,y_size);  */

			framebuffer_blockcopyfill(d->vfb_data, 0, 0,0,0,
			    x_dest, y_dest, x_dest + x_size - 1, y_dest +
			    y_size - 1, x_source, y_source);
		} else if (data[8] == 0x10 && data[9] == 0x55 && len == 48) {
			/*  Linux "clear":  This is used by linux to clear the
			    lowest 16 pixels of the framebuffer.  */
			int xbase, ybase, xend, yend;

			xbase = (data[8*4 + 0] + (data[8*4 + 1] << 8)) / 16;
			ybase = (data[8*4 + 2] + (data[8*4 + 3] << 8)) / 16;
			xend  = (data[8*5 + 0] + (data[8*5 + 1] << 8)) / 16;
			yend  = (data[8*5 + 2] + (data[8*5 + 3] << 8)) / 16;

			/*  debug("[ gif: linux \"clear\" (%i,%i)-(%i,%i) ]\n",
			    xbase, ybase, xend, yend);  */

			framebuffer_blockcopyfill(d->vfb_data, 1, 0,0,0,
			    xbase, ybase, xend - 1, yend - 1, 0,0);
		} else if (data[0] == 0x07 && data[1] == 0x80 && len == 128) {
			/*  NetBSD "output cursor":  */
			int xbase, ybase, xend, yend, x, y;

			xbase = (data[20*4 + 0] + (data[20*4 + 1] << 8)) / 16;
			ybase = (data[20*4 + 2] + (data[20*4 + 3] << 8)) / 16;
			xend  = (data[28*4 + 0] + (data[28*4 + 1] << 8)) / 16;
			yend  = (data[28*4 + 2] + (data[28*4 + 3] << 8)) / 16;

			/*  debug("[ gif: NETBSD cursor at (%i,%i)-(%i,%i) ]\n",
			    xbase, ybase, xend, yend);  */

			/*  Output the cursor to framebuffer memory:  */

			for (y=ybase; y<=yend; y++)
				for (x=xbase; x<=xend; x++) {
					int fb_addr = (x + y * d->xsize) *
					    d->bytes_per_pixel;
					unsigned char pixels[3];

					dev_fb_access(d->cpu, d->cpu->mem,
					    fb_addr, pixels, sizeof(pixels),
					    MEM_READ, d->vfb_data);

					pixels[0] = 0xff - pixels[0];
					pixels[1] = 0xff - pixels[1];
					pixels[2] = 0xff - pixels[2];

					dev_fb_access(d->cpu, d->cpu->mem,
					    fb_addr, pixels, sizeof(pixels),
					    MEM_WRITE, d->vfb_data);
				}
		} else if (data[0] == 0x01 && data[1] == 0x00 && len == 80) {
			/*  Linux "output cursor":  */
			int xbase, ybase, xend, yend, x, y;

			xbase = (data[7*8 + 0] + (data[7*8 + 1] << 8)) / 16;
			ybase = (data[7*8 + 2] + (data[7*8 + 3] << 8)) / 16;
			xend  = (data[8*8 + 0] + (data[8*8 + 1] << 8)) / 16;
			yend  = (data[8*8 + 2] + (data[8*8 + 3] << 8)) / 16;

			debug("[ gif: LINUX cursor at (%i,%i)-(%i,%i) ]\n",
			    xbase, ybase, xend, yend);

			/*  Output the cursor to framebuffer memory:  */

			for (y=ybase; y<=yend; y++)
				for (x=xbase; x<=xend; x++) {
					int fb_addr = (x + y * d->xsize) *
					    d->bytes_per_pixel;
					unsigned char pixels[3];

					dev_fb_access(d->cpu, d->cpu->mem,
					    fb_addr, pixels, sizeof(pixels),
					    MEM_READ, d->vfb_data);

					pixels[0] = 0xff - pixels[0];
					pixels[1] = 0xff - pixels[1];
					pixels[2] = 0xff - pixels[2];

					dev_fb_access(d->cpu, d->cpu->mem,
					    fb_addr, pixels, sizeof(pixels),
					    MEM_WRITE, d->vfb_data);
				}
		} else {		/*  Unknown command:  */
			fatal("[ gif write to addr 0x%x (len=%i):",
			    (int)relative_addr, len);
			for (i=0; i<len; i++)
				fatal(" %02x", data[i]);
			fatal(" ]\n");
/*			fatal("Unknown gif command.\n");
			cpu->running = 0;
*/		}
	}

	return 1;
}


/*
 *  devinit_ps2_gif():
 *
 *  Attached to separate memory by devinit_ps2_gs().
 */
DEVINIT(ps2_gif)
{
	struct gif_data *d;

	CHECK_ALLOCATION(d = (struct gif_data *) malloc(sizeof(struct gif_data)));
	memset(d, 0, sizeof(struct gif_data));

	d->transparent_text = 0;
	d->cpu = devinit->machine->cpus[0];		/*  TODO  */
	d->xsize = 640; d->ysize = 480;
	d->bytes_per_pixel = 3;

	d->vfb_data = dev_fb_init(devinit->machine, devinit->machine->memory,
	    PS2_FB_ADDR, VFB_PLAYSTATION2,
	    d->xsize, d->ysize, d->xsize, d->ysize, 24, "Playstation 2");
	if (d->vfb_data == NULL) {
		fprintf(stderr, "could not initialize fb, out of memory\n");
		exit(1);
	}

#if 0
	test_triangle(d, 300,50, 255,0,0,  50,150, 0,255,0,  600,400, 0,0,255);
	test_triangle(d, 310,210, 128,32,0,  175,410, 0,32,0,
	    500,470, 125,255,125);
	test_triangle(d, 100,450, 255,255,0,  250,370, 0,255,255,
	    400,470, 255,0,255);
#endif

	memory_device_register(devinit->machine->memory, devinit->name,
	    devinit->addr, DEV_PS2_GIF_LENGTH, dev_ps2_gif_access, d,
	    DM_DEFAULT, NULL);

	return 1;
}

