#ifndef	X11_H
#define	X11_H

/*
 *  Copyright (C) 2003-2010  Anders Gavare.  All rights reserved.
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
 *  Headerfile for src/x11.c.
 */

#include "misc.h"

struct emul;

#ifdef WITH_X11
#include <X11/Xlib.h>
#endif


/*  x11.c:  */
#define N_GRAYCOLORS            16
#define	CURSOR_COLOR_TRANSPARENT	-1
#define	CURSOR_COLOR_INVERT		-2
#define	CURSOR_MAXY		64
#define	CURSOR_MAXX		64
/*  Framebuffer windows:  */
struct fb_window {
	int		fb_number;

#ifdef WITH_X11
	/*  x11_fb_winxsize > 0 for a valid fb_window  */
	int		x11_fb_winxsize, x11_fb_winysize;
	int		scaledown;
	Display		*x11_display;

	int		x11_screen;
	int		x11_screen_depth;
	unsigned long	fg_color;
	unsigned long	bg_color;
	XColor		x11_graycolor[N_GRAYCOLORS];
	Window		x11_fb_window;
	GC		x11_fb_gc;

	XImage		*fb_ximage;
	unsigned char	*ximage_data;

	/*  -1 means transparent, 0 and up are grayscales  */
	int		cursor_pixels[CURSOR_MAXY][CURSOR_MAXX];
	int		cursor_x;
	int		cursor_y;
	int		cursor_xsize;
	int		cursor_ysize;
	int		cursor_on;
	int		OLD_cursor_x;
	int		OLD_cursor_y;
	int		OLD_cursor_xsize;
	int		OLD_cursor_ysize;
	int		OLD_cursor_on;

	/*  Host's X11 cursor:  */
	Cursor		host_cursor;
	Pixmap		host_cursor_pixmap;
#endif
};
void x11_redraw_cursor(struct machine *, int);
void x11_redraw(struct machine *, int);
void x11_putpixel_fb(struct machine *, int, int x, int y, int color);
#ifdef WITH_X11
void x11_putimage_fb(struct machine *, int);
#endif
void x11_init(struct machine *);
void x11_fb_resize(struct fb_window *win, int new_xsize, int new_ysize);
void x11_set_standard_properties(struct fb_window *fb_window, char *name);
struct fb_window *x11_fb_init(int xsize, int ysize, char *name,
	int scaledown, struct machine *);
void x11_check_event(struct emul *emul);


#endif	/*  X11_H  */
