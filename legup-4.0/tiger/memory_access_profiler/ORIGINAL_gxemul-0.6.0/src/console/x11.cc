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
 *  X11-related functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "console.h"
#include "emul.h"
#include "machine.h"
#include "misc.h"
#include "x11.h"


#ifndef	WITH_X11


/*  Dummy functions:  */
void x11_redraw_cursor(struct machine *m, int i) { }
void x11_redraw(struct machine *m, int x) { }
void x11_putpixel_fb(struct machine *m, int fb, int x, int y, int color) { }
void x11_init(struct machine *machine) { }
struct fb_window *x11_fb_init(int xsize, int ysize, char *name,
	int scaledown, struct machine *machine)
    { return NULL; }
void x11_check_event(struct emul *emul) { }


#else	/*  WITH_X11  */


#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>


/*
 *  x11_redraw_cursor():
 *
 *  Redraw a framebuffer's X11 cursor.
 *
 *  NOTE: It is up to the caller to call XFlush.
 */
void x11_redraw_cursor(struct machine *m, int i)
{
	int last_color_used = 0;
	int n_colors_used = 0;
	struct fb_window *fbwin = m->x11_md.fb_windows[i];

	/*  Remove old cursor, if any:  */
	if (fbwin->x11_display != NULL && fbwin->OLD_cursor_on) {
		XPutImage(fbwin->x11_display, fbwin->x11_fb_window,
		    fbwin->x11_fb_gc, fbwin->fb_ximage,
		    fbwin->OLD_cursor_x/fbwin->scaledown,
		    fbwin->OLD_cursor_y/fbwin->scaledown,
		    fbwin->OLD_cursor_x/fbwin->scaledown,
		    fbwin->OLD_cursor_y/fbwin->scaledown,
		    fbwin->OLD_cursor_xsize/fbwin->scaledown + 1,
		    fbwin->OLD_cursor_ysize/fbwin->scaledown + 1);
	}

	if (fbwin->x11_display != NULL && fbwin->cursor_on) {
		int x, y, subx, suby;
		XImage *xtmp;

		CHECK_ALLOCATION(xtmp = XSubImage(fbwin->fb_ximage,
		    fbwin->cursor_x/fbwin->scaledown,
		    fbwin->cursor_y/fbwin->scaledown,
		    fbwin->cursor_xsize/fbwin->scaledown + 1,
		    fbwin->cursor_ysize/fbwin->scaledown + 1));

		for (y=0; y<fbwin->cursor_ysize;
		    y+=fbwin->scaledown)
			for (x=0; x<fbwin->cursor_xsize;
			    x+=fbwin->scaledown) {
				int px = x/fbwin->scaledown;
				int py = y/fbwin->scaledown;
				int p = 0, n = 0, c = 0;
				unsigned long oldcol;

				for (suby=0; suby<fbwin->scaledown;
				    suby++)
					for (subx=0; subx<fbwin->
					    scaledown; subx++) {
						c = fbwin->
						    cursor_pixels[y+suby]
						    [x+subx];
						if (c >= 0) {
							p += c;
							n++;
						}
					}
				if (n > 0)
					p /= n;
				else
					p = c;

				if (n_colors_used == 0) {
					last_color_used = p;
					n_colors_used = 1;
				} else
					if (p != last_color_used)
						n_colors_used = 2;

				switch (p) {
				case CURSOR_COLOR_TRANSPARENT:
					break;
				case CURSOR_COLOR_INVERT:
					oldcol = XGetPixel(xtmp, px, py);
					if (oldcol != fbwin->
					    x11_graycolor[N_GRAYCOLORS-1].pixel)
						oldcol = fbwin->
						    x11_graycolor[N_GRAYCOLORS
						    -1].pixel;
					else
						oldcol = fbwin->
						    x11_graycolor[0].pixel;
					XPutPixel(xtmp, px, py, oldcol);
					break;
				default:	/*  Normal grayscale:  */
					XPutPixel(xtmp, px, py, fbwin->
					    x11_graycolor[p].pixel);
				}
			}

		XPutImage(fbwin->x11_display,
		    fbwin->x11_fb_window,
		    fbwin->x11_fb_gc,
		    xtmp, 0, 0,
		    fbwin->cursor_x/fbwin->scaledown,
		    fbwin->cursor_y/fbwin->scaledown,
		    fbwin->cursor_xsize/fbwin->scaledown,
		    fbwin->cursor_ysize/fbwin->scaledown);

		XDestroyImage(xtmp);

		fbwin->OLD_cursor_on = fbwin->cursor_on;
		fbwin->OLD_cursor_x = fbwin->cursor_x;
		fbwin->OLD_cursor_y = fbwin->cursor_y;
		fbwin->OLD_cursor_xsize =
		    fbwin->cursor_xsize;
		fbwin->OLD_cursor_ysize =
		    fbwin->cursor_ysize;
	}

	/*  printf("n_colors_used = %i\n", n_colors_used);  */

	if (fbwin->host_cursor != 0 && n_colors_used < 2) {
		/*  Remove the old X11 host cursor:  */
		XUndefineCursor(fbwin->x11_display,
		    fbwin->x11_fb_window);
		XFreeCursor(fbwin->x11_display,
		    fbwin->host_cursor);
		fbwin->host_cursor = 0;
	}

	if (n_colors_used >= 2 && fbwin->host_cursor == 0) {
		GC tmpgc;

		/*  Create a new X11 host cursor:  */
		/*  cursor = XCreateFontCursor(fbwin->x11_display,
		    XC_coffee_mug);  :-)  */
		if (fbwin->host_cursor_pixmap != 0) {
			XFreePixmap(fbwin->x11_display,
			    fbwin->host_cursor_pixmap);
			fbwin->host_cursor_pixmap = 0;
		}
		fbwin->host_cursor_pixmap =
		    XCreatePixmap(fbwin->x11_display,
		    fbwin->x11_fb_window, 1, 1, 1);
		XSetForeground(fbwin->x11_display,
		    fbwin->x11_fb_gc,
		    fbwin->x11_graycolor[0].pixel);

		tmpgc = XCreateGC(fbwin->x11_display,
		    fbwin->host_cursor_pixmap, 0,0);

		XDrawPoint(fbwin->x11_display,
		    fbwin->host_cursor_pixmap,
		    tmpgc, 0, 0);

		XFreeGC(fbwin->x11_display, tmpgc);

		fbwin->host_cursor =
		    XCreatePixmapCursor(fbwin->x11_display,
		    fbwin->host_cursor_pixmap,
		    fbwin->host_cursor_pixmap,
		    &fbwin->x11_graycolor[N_GRAYCOLORS-1],
		    &fbwin->x11_graycolor[N_GRAYCOLORS-1],
		    0, 0);
		if (fbwin->host_cursor != 0) {
			XDefineCursor(fbwin->x11_display,
			    fbwin->x11_fb_window,
			    fbwin->host_cursor);
		}
	}
}


/*
 *  x11_redraw():
 *
 *  Redraw X11 windows.
 */
void x11_redraw(struct machine *m, int i)
{
	if (i < 0 || i >= m->x11_md.n_fb_windows ||
	    m->x11_md.fb_windows[i]->x11_fb_winxsize <= 0)
		return;

	x11_putimage_fb(m, i);
	x11_redraw_cursor(m, i);
	XFlush(m->x11_md.fb_windows[i]->x11_display);
}


/*
 *  x11_putpixel_fb():
 *
 *  Output a framebuffer pixel. i is the framebuffer number.
 */
void x11_putpixel_fb(struct machine *m, int i, int x, int y, int color)
{
	struct fb_window *fbwin;
	if (i < 0 || i >= m->x11_md.n_fb_windows)
		return;

	fbwin = m->x11_md.fb_windows[i];

	if (fbwin->x11_fb_winxsize <= 0)
		return;

	if (color)
		XSetForeground(fbwin->x11_display,
		    fbwin->x11_fb_gc, fbwin->fg_color);
	else
		XSetForeground(fbwin->x11_display,
		    fbwin->x11_fb_gc, fbwin->bg_color);

	XDrawPoint(fbwin->x11_display,
	    fbwin->x11_fb_window, fbwin->x11_fb_gc, x, y);

	XFlush(fbwin->x11_display);
}


/*
 *  x11_putimage_fb():
 *
 *  Output an entire XImage to a framebuffer window. i is the
 *  framebuffer number.
 */
void x11_putimage_fb(struct machine *m, int i)
{
	struct fb_window *fbwin;
	if (i < 0 || i >= m->x11_md.n_fb_windows)
		return;

	fbwin = m->x11_md.fb_windows[i];

	if (fbwin->x11_fb_winxsize <= 0)
		return;

	XPutImage(fbwin->x11_display,
	    fbwin->x11_fb_window,
	    fbwin->x11_fb_gc, fbwin->fb_ximage, 0,0, 0,0,
	    fbwin->x11_fb_winxsize,
	    fbwin->x11_fb_winysize);
	XFlush(fbwin->x11_display);
}


/*
 *  x11_init():
 *
 *  Initialize X11 stuff (but doesn't create any windows).
 *
 *  It is then up to individual drivers, for example framebuffer devices,
 *  to initialize their own windows.
 */
void x11_init(struct machine *m)
{
	m->x11_md.n_fb_windows = 0;

	if (m->x11_md.n_display_names > 0) {
		int i;
		for (i=0; i<m->x11_md.n_display_names; i++)
			fatal("Using X11 display: %s\n",
			    m->x11_md.display_names[i]);
	}

	m->x11_md.current_display_name_nr = 0;
}


/*
 *  x11_fb_resize():
 *
 *  Set a new size for an X11 framebuffer window.  (NOTE: I didn't think of
 *  this kind of functionality during the initial design, so it is probably
 *  buggy. It also needs some refactoring.)
 */
void x11_fb_resize(struct fb_window *win, int new_xsize, int new_ysize)
{
	int alloc_depth;

	if (win == NULL) {
		fatal("x11_fb_resize(): win == NULL\n");
		return;
	}

	win->x11_fb_winxsize = new_xsize;
	win->x11_fb_winysize = new_ysize;

	alloc_depth = win->x11_screen_depth;
	if (alloc_depth == 24)
		alloc_depth = 32;
	if (alloc_depth == 15)
		alloc_depth = 16;

	/*  Note: ximage_data seems to be freed by XDestroyImage below.  */
	/*  if (win->ximage_data != NULL)
		free(win->ximage_data);  */
	CHECK_ALLOCATION(win->ximage_data = (unsigned char *) malloc(
	    new_xsize * new_ysize * alloc_depth / 8));

	/*  TODO: clear for non-truecolor modes  */
	memset(win->ximage_data, 0, new_xsize * new_ysize * alloc_depth / 8);

	if (win->fb_ximage != NULL)
		XDestroyImage(win->fb_ximage);
	win->fb_ximage = XCreateImage(win->x11_display, CopyFromParent,
	    win->x11_screen_depth, ZPixmap, 0, (char *)win->ximage_data,
	    new_xsize, new_ysize, 8, new_xsize * alloc_depth / 8);
	CHECK_ALLOCATION(win->fb_ximage);

	XResizeWindow(win->x11_display, win->x11_fb_window,
	    new_xsize, new_ysize);
}


/*
 *  x11_set_standard_properties():
 *
 *  Right now, this only sets the title of a window.
 */
void x11_set_standard_properties(struct fb_window *fb_window, char *name)
{
	XSetStandardProperties(fb_window->x11_display,
	    fb_window->x11_fb_window, name, "GXemul "VERSION,
	    None, NULL, 0, NULL);
}


/*
 *  x11_fb_init():
 *
 *  Initialize a framebuffer window.
 */
struct fb_window *x11_fb_init(int xsize, int ysize, char *name,
	int scaledown, struct machine *m)
{
	Display *x11_display;
	int x, y, fb_number = 0;
	size_t alloclen, alloc_depth;
	XColor tmpcolor;
	struct fb_window *fbwin;
	int i;
	char fg[80], bg[80];
	char *display_name;

	fb_number = m->x11_md.n_fb_windows;

	CHECK_ALLOCATION(m->x11_md.fb_windows = 
	    (struct fb_window **) realloc(m->x11_md.fb_windows,
	    sizeof(struct fb_window *) * (m->x11_md.n_fb_windows + 1)));
	CHECK_ALLOCATION(fbwin = m->x11_md.fb_windows[fb_number] =
	    (struct fb_window *) malloc(sizeof(struct fb_window)));

	m->x11_md.n_fb_windows ++;

	memset(fbwin, 0, sizeof(struct fb_window));

	fbwin->x11_fb_winxsize = xsize;
	fbwin->x11_fb_winysize = ysize;

	/*  Which display name?  */
	display_name = NULL;
	if (m->x11_md.n_display_names > 0) {
		display_name = m->x11_md.display_names[
		    m->x11_md.current_display_name_nr];
		m->x11_md.current_display_name_nr ++;
		m->x11_md.current_display_name_nr %= m->x11_md.n_display_names;
	}

	if (display_name != NULL)
		debug("[ x11_fb_init(): framebuffer window %i, %ix%i, DISPLAY"
		    "=%s ]\n", fb_number, xsize, ysize, display_name);

	x11_display = XOpenDisplay(display_name);

	if (x11_display == NULL) {
		fatal("x11_fb_init(\"%s\"): couldn't open display\n", name);
		if (display_name != NULL)
			fatal("display_name = '%s'\n", display_name);
		exit(1);
	}

	fbwin->x11_screen =
	    DefaultScreen(x11_display);
	fbwin->x11_screen_depth =
	    DefaultDepth(x11_display,
	    fbwin->x11_screen);

	if (fbwin->x11_screen_depth != 8 &&
	    fbwin->x11_screen_depth != 15 &&
	    fbwin->x11_screen_depth != 16 &&
	    fbwin->x11_screen_depth != 24) {
		fatal("\n***\n***  WARNING! Your X server is running %i-bit "
		    "color mode. This is not really\n",
		    fbwin->x11_screen_depth);
		fatal("***  supported yet.  8, 15, 16, and 24 bits should "
		    "work.\n***  24-bit server gives color.  Any other bit "
		    "depth gives undefined result!\n***\n\n");
	}

	if (fbwin->x11_screen_depth <= 8)
		debug("WARNING! X11 screen depth is not enough for color; "
		    "using only 16 grayscales instead\n");

	strlcpy(bg, "Black", sizeof(bg));
	strlcpy(fg, "White", sizeof(fg));

	XParseColor(x11_display, DefaultColormap(x11_display,
	    fbwin->x11_screen), fg, &tmpcolor);
	XAllocColor(x11_display, DefaultColormap(x11_display,
	    fbwin->x11_screen), &tmpcolor);
	fbwin->fg_color = tmpcolor.pixel;
	XParseColor(x11_display, DefaultColormap(x11_display,
	    fbwin->x11_screen), bg, &tmpcolor);
	XAllocColor(x11_display, DefaultColormap(x11_display,
	    fbwin->x11_screen), &tmpcolor);
	fbwin->bg_color = tmpcolor.pixel;

	for (i=0; i<N_GRAYCOLORS; i++) {
		char cname[8];
		cname[0] = '#';
		cname[1] = cname[2] = cname[3] =
		    cname[4] = cname[5] = cname[6] =
		    "0123456789ABCDEF"[i];
		cname[7] = '\0';
		XParseColor(x11_display, DefaultColormap(x11_display,
		    fbwin->x11_screen), cname,
		    &fbwin->x11_graycolor[i]);
		XAllocColor(x11_display, DefaultColormap(x11_display,
		    fbwin->x11_screen),
		    &fbwin->x11_graycolor[i]);
	}

        XFlush(x11_display);

	alloc_depth = fbwin->x11_screen_depth;

	if (alloc_depth == 24)
		alloc_depth = 32;
	if (alloc_depth == 15)
		alloc_depth = 16;

	fbwin->x11_fb_window = XCreateWindow(
	    x11_display, DefaultRootWindow(x11_display),
	    0, 0, fbwin->x11_fb_winxsize,
	    fbwin->x11_fb_winysize,
	    0, CopyFromParent, InputOutput, CopyFromParent, 0,0);

	fbwin->x11_display = x11_display;

	x11_set_standard_properties(fbwin, name);

	XSelectInput(x11_display,
	    fbwin->x11_fb_window,
	    StructureNotifyMask | ExposureMask | ButtonPressMask |
	    ButtonReleaseMask | PointerMotionMask | KeyPressMask);
	fbwin->x11_fb_gc = XCreateGC(x11_display,
	    fbwin->x11_fb_window, 0,0);

	/*  Make sure the window is mapped:  */
	XMapRaised(x11_display, fbwin->x11_fb_window);

	XSetBackground(x11_display, fbwin->x11_fb_gc, fbwin->bg_color);
	XSetForeground(x11_display, fbwin->x11_fb_gc, fbwin->bg_color);
	XFillRectangle(x11_display, fbwin->x11_fb_window, fbwin->x11_fb_gc, 0,0,
	    fbwin->x11_fb_winxsize, fbwin->x11_fb_winysize);

	fbwin->scaledown   = scaledown;

	fbwin->fb_number = fb_number;

	alloclen = xsize * ysize * alloc_depth / 8;
	CHECK_ALLOCATION(fbwin->ximage_data = (unsigned char *) malloc(alloclen));

	fbwin->fb_ximage = XCreateImage(fbwin->x11_display, CopyFromParent,
	    fbwin->x11_screen_depth, ZPixmap, 0, (char *)fbwin->ximage_data,
	    xsize, ysize, 8, xsize * alloc_depth / 8);
	CHECK_ALLOCATION(fbwin->fb_ximage);

	/*  Fill the ximage with black pixels:  */
	if (fbwin->x11_screen_depth > 8)
		memset(fbwin->ximage_data, 0, alloclen);
	else {
		debug("x11_fb_init(): clearing the XImage\n");
		for (y=0; y<ysize; y++)
			for (x=0; x<xsize; x++)
				XPutPixel(fbwin->fb_ximage, x, y,
				    fbwin->x11_graycolor[0].pixel);
	}

	x11_putimage_fb(m, fb_number);

	/*  Fill the 64x64 "hardware" cursor with white pixels:  */
	xsize = ysize = 64;

	/*  Fill the cursor ximage with white pixels:  */
	for (y=0; y<ysize; y++)
		for (x=0; x<xsize; x++)
			fbwin->cursor_pixels[y][x] = N_GRAYCOLORS-1;

	return fbwin;
}


/*
 *  x11_check_events_machine():
 *
 *  Check for X11 events on a specific machine.
 *
 *  TODO:  Yuck! This has to be rewritten. Each display should be checked,
 *         and _then_ only those windows that are actually exposed should
 *         be redrawn!
 */
static void x11_check_events_machine(struct emul *emul, struct machine *m)
{
	int fb_nr;

	for (fb_nr = 0; fb_nr < m->x11_md.n_fb_windows; fb_nr ++) {
		struct fb_window *fbwin = m->x11_md.fb_windows[fb_nr];
		XEvent event;
		int need_redraw = 0, found, i, j;

		while (XPending(fbwin->x11_display)) {
			XNextEvent(fbwin->x11_display, &event);

			if (event.type==ConfigureNotify) {
				need_redraw = 1;
			}

			if (event.type==Expose && event.xexpose.count==0) {
				/*
				 *  TODO:  the xexpose struct has x,y,width,
				 *  height. Those could be used to only redraw
				 *  the part of the framebuffer that was
				 *  exposed. Note that the (mouse) cursor must
				 *  be redrawn too.
				 */
				/*  x11_winxsize = event.xexpose.width;
				    x11_winysize = event.xexpose.height;  */
				need_redraw = 1;
			}

			if (event.type == MotionNotify) {
				/*  debug("[ X11 MotionNotify: %i,%i ]\n",
				    event.xmotion.x, event.xmotion.y);  */

				/*  Which window in which machine in
				    which emulation?  */
				found = -1;
				for (j=0; j<emul->n_machines; j++) {
					struct machine *m2 = emul->machines[j];
					for (i=0; i<m2->x11_md.
					    n_fb_windows; i++)
						if (m->x11_md.
						    fb_windows[fb_nr]->
						    x11_display == m2->
						    x11_md.
						    fb_windows[i]->
						    x11_display &&
						    event.xmotion.
						    window == m2->
						    x11_md.
						    fb_windows[i]->
						    x11_fb_window)
							found = i;
				}
				if (found < 0) {
					printf("Internal error in x11.c.\n");
					exit(1);
				}
				console_mouse_coordinates(event.xmotion.x *
				    m->x11_md.fb_windows[found]->scaledown,
				    event.xmotion.y * m->x11_md.fb_windows[
				    found]->scaledown, found);
			}

			if (event.type == ButtonPress) {
				debug("[ X11 ButtonPress: %i ]\n",
				    event.xbutton.button);
				/*  button = 1,2,3 = left,middle,right  */

				console_mouse_button(event.xbutton.button, 1);
			}

			if (event.type == ButtonRelease) {
				debug("[ X11 ButtonRelease: %i ]\n",
				    event.xbutton.button);
				/*  button = 1,2,3 = left,middle,right  */

				console_mouse_button(event.xbutton.button, 0);
			}

			if (event.type==KeyPress) {
				char text[15];
				KeySym key;
				XKeyPressedEvent *ke = &event.xkey;

				memset(text, 0, sizeof(text));

				if (XLookupString(&event.xkey, text,
				    sizeof(text), &key, 0) == 1) {
					console_makeavail(
					    m->main_console_handle, text[0]);
				} else {
					int x = ke->keycode;
					/*
					 *  Special key codes:
					 *
					 *  NOTE/TODO: I'm hardcoding these to
					 *  work with my key map. Maybe they
					 *  should be read from some file...
					 *
					 *  Important TODO 2:  It would be MUCH
					 *  better if these were converted into
					 *  'native scancodes', for example for
					 *  the DECstation's keyboard or the
					 *  PC-style 8042 controller.
					 */
					switch (x) {
					case 9:	/*  Escape  */
						console_makeavail(m->
						    main_console_handle, 27);
						break;
#if 0
					/*  TODO  */

					/*  The numeric keypad:  */
					90=Ins('0')  91=Del(',')

					/*  Above the cursor keys:  */
					106=Ins  107=Del
#endif
					/*  F1..F4:  */
					case 67:	/*  F1  */
					case 68:	/*  F2  */
					case 69:	/*  F3  */
					case 70:	/*  F4  */
						console_makeavail(m->
						    main_console_handle, 27);
						console_makeavail(m->
						    main_console_handle, '[');
						console_makeavail(m->
						    main_console_handle, 'O');
						console_makeavail(m->
						    main_console_handle, 'P' +
						    x - 67);
						break;
					case 71:	/*  F5  */
						console_makeavail(m->
						    main_console_handle, 27);
						console_makeavail(m->
						    main_console_handle, '[');
						console_makeavail(m->
						    main_console_handle, '1');
						console_makeavail(m->
						    main_console_handle, '5');
						break;
					case 72:	/*  F6  */
					case 73:	/*  F7  */
					case 74:	/*  F8  */
						console_makeavail(m->
						    main_console_handle, 27);
						console_makeavail(m->
						    main_console_handle, '[');
						console_makeavail(m->
						    main_console_handle, '1');
						console_makeavail(m->
						    main_console_handle, '7' +
						    x - 72);
						break;
					case 75:	/*  F9  */
					case 76:	/*  F10  */
						console_makeavail(m->
						    main_console_handle, 27);
						console_makeavail(m->
						    main_console_handle, '[');
						console_makeavail(m->
						    main_console_handle, '2');
						console_makeavail(m->
						    main_console_handle, '1' +
						    x - 68);
						break;
					case 95:	/*  F11  */
					case 96:	/*  F12  */
						console_makeavail(m->
						    main_console_handle, 27);
						console_makeavail(m->
						    main_console_handle, '[');
						console_makeavail(m->
						    main_console_handle, '2');
						console_makeavail(m->
						    main_console_handle, '3' +
						    x - 95);
						break;
					/*  Cursor keys:  */
					case 98:	/*  Up  */
					case 104:	/*  Down  */
					case 100:	/*  Left  */
					case 102:	/*  Right  */
						console_makeavail(m->
						    main_console_handle, 27);
						console_makeavail(m->
						    main_console_handle, '[');
						console_makeavail(m->
						    main_console_handle, 
						    x == 98? 'A' : (
						    x == 104? 'B' : (
						    x == 102? 'C' : (
						    'D'))));
						break;
					/*  Numeric keys:  */
					case 80:	/*  Up  */
					case 88:	/*  Down  */
					case 83:	/*  Left  */
					case 85:	/*  Right  */
						console_makeavail(m->
						    main_console_handle, 27);
						console_makeavail(m->
						    main_console_handle, '[');
						console_makeavail(m->
						    main_console_handle, 
						    x == 80? 'A' : (
						    x == 88? 'B' : (
						    x == 85? 'C' : (
						    'D'))));
						break;
					case 97:	/*  Cursor  Home  */
					case 79:	/*  Numeric Home  */
						console_makeavail(m->
						    main_console_handle, 27);
						console_makeavail(m->
						    main_console_handle, '[');
						console_makeavail(m->
						    main_console_handle, 'H');
						break;
					case 103:	/*  Cursor  End  */
					case 87:	/*  Numeric End  */
						console_makeavail(m->
						    main_console_handle, 27);
						console_makeavail(m->
						    main_console_handle, '[');
						console_makeavail(m->
						    main_console_handle, 'F');
						break;
					case 99:	/*  Cursor  PgUp  */
					case 81:	/*  Numeric PgUp  */
						console_makeavail(m->
						    main_console_handle, 27);
						console_makeavail(m->
						    main_console_handle, '[');
						console_makeavail(m->
						    main_console_handle, '5');
						console_makeavail(m->
						    main_console_handle, '~');
						break;
					case 105:	/*  Cursor  PgUp  */
					case 89:	/*  Numeric PgDn  */
						console_makeavail(m->
						    main_console_handle, 27);
						console_makeavail(m->
						    main_console_handle, '[');
						console_makeavail(m->
						    main_console_handle, '6');
						console_makeavail(m->
						    main_console_handle, '~');
						break;
					default:
						debug("[ unimplemented X11 "
						    "keycode %i ]\n", x);
					}
				}
			}
		}

		if (need_redraw)
			x11_redraw(m, fb_nr);
	}
}


/*
 *  x11_check_event():
 *
 *  Check for X11 events.
 */
void x11_check_event(struct emul *emul)
{
	int i;

	for (i=0; i<emul->n_machines; i++)
		x11_check_events_machine(emul, emul->machines[i]);
}

#endif	/*  WITH_X11  */
