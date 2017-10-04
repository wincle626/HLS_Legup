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
 *  COMMENT: LK201 keyboard and mouse, used by the dc7085 and scc controllers
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "console.h"
#include "devices.h"
#include "machine.h"
#include "misc.h"

#include "thirdparty/dc7085.h"	/*  for port names  */
#include "thirdparty/lk201.h"


/*
 *  lk201_convert_ascii_to_keybcode():
 *
 *  Converts ascii console input to LK201 keyboard scan codes, and adds
 *  those scancodes using the add_to_rx_queue() function.
 */
void lk201_convert_ascii_to_keybcode(struct lk201_data *d, unsigned char ch)
{
	int i, found=-1, shifted = 0, controlled = 0;

	if (d->keyb_buf_pos > 0 && d->keyb_buf_pos < (int)sizeof(d->keyb_buf)) {
		/*  Escape sequence:  */
		d->keyb_buf[d->keyb_buf_pos] = ch;
		d->keyb_buf_pos ++;

		if (d->keyb_buf_pos == 2) {
			if (ch == '[')
				return;
			d->keyb_buf_pos = 0;
			/*  not esc+[, output as normal key  */
		} else {
			/*  Inside an esc+[ sequence  */

			switch (ch) {
			case 'A':  found = 0xaa;  /*  Up     */  break;
			case 'B':  found = 0xa9;  /*  Down   */  break;
			case 'C':  found = 0xa8;  /*  Right  */  break;
			case 'D':  found = 0xa7;  /*  Left   */  break;
			/*  TODO: pageup, pagedown, ...  */
			default:
				;
			}

			d->keyb_buf_pos = 0;
		}
	} else
		d->keyb_buf_pos = 0;

	if (found == -1) {
		switch (ch) {
		case '\b':
			found = 0xbc;
			break;
		case '\n':
		case '\r':
			found = 0xbd;
			break;
		case '\t':
			found = 0xbe;
			break;
		case 27:	/*  esc  */
			d->keyb_buf[0] = 27;
			d->keyb_buf_pos = 1;
			return;
		default:
			if (ch >= 1 && ch <= 26) {
				ch = 'a' + ch - 1;
				controlled = 1;
			}

			shifted = 0;
			for (i=0; i<256; i++) {
				/*  Skip numeric digits, so that the normal
					digits are used instead.  */
				if (i >= 0x92 && i<=0xa0)
					continue;

				if (unshiftedAscii[i] == ch) {
					found = i;
					break;
				}
			}

			if (found == -1) {
				/*  unshift ch:  */
				if (ch >= 'A' && ch <= 'Z')
					ch = ch + ('a' - 'A');
				for (i=0; i<256; i++)
					if (shiftedAscii[i] == ch) {
						found = i;
						shifted = 1;
						break;
					}
			}
		}
	}

	if (!shifted)
		d->add_to_rx_queue(d->add_data, KEY_UP, DCKBD_PORT);
	else {
		d->add_to_rx_queue(d->add_data, KEY_UP, DCKBD_PORT);
		d->add_to_rx_queue(d->add_data, KEY_SHIFT, DCKBD_PORT);
	}

	if (controlled)
		d->add_to_rx_queue(d->add_data, KEY_CONTROL, DCKBD_PORT);

	/*  Send the actual scan code:  */
	d->add_to_rx_queue(d->add_data, found, DCKBD_PORT);

	/*  Release the key:  */
	d->add_to_rx_queue(d->add_data, KEY_UP, DCKBD_PORT);
}


/*
 *  lk201_send_mouse_update_sequence():
 *
 *  mouse_x, _y, _buttons contains the coordinates on the host's display, the
 *  "goal" of where we want to move.
 *
 *  d->mouse_x, _y, _buttons contain the last values transmitted to the
 *  emulated machine.
 */
static int lk201_send_mouse_update_sequence(struct lk201_data *d, int mouse_x,
	int mouse_y, int mouse_buttons, int mouse_fb_nr)
{
	int xsign, xdelta, ysign, ydelta, m;

	xdelta = mouse_x - d->mouse_x;
	ydelta = mouse_y - d->mouse_y;

	/*  If no change, then don't send any update!  */
	if (xdelta == 0 && ydelta == 0 && d->mouse_buttons == mouse_buttons)
		return 0;

	m = 20;

	if (xdelta > m)
		xdelta = m;
	if (xdelta < -m)
		xdelta = -m;
	if (ydelta > m)
		ydelta = m;
	if (ydelta < -m)
		ydelta = -m;

	d->mouse_x += xdelta;
	d->mouse_y += ydelta;
	d->mouse_buttons = mouse_buttons;

	/*
	 *  TODO: Update d->mouse_framebuffer_nr some way!
	 */

	xsign = xdelta < 0? 1 : 0;
	ysign = ydelta < 0? 1 : 0;

	switch (d->mouse_mode) {

	case 0:
		/*  Do nothing (before the mouse is initialized)  */
		return 0;

	case MOUSE_INCREMENTAL:
		if (xdelta < 0)
			xdelta = -xdelta;
		if (ydelta < 0)
			ydelta = -ydelta;

		/*  Reverse sign of x:  (this is needed for some reason)  */
		xsign ^= 1;

		d->add_to_rx_queue(d->add_data, MOUSE_START_FRAME +
		    MOUSE_X_SIGN*xsign + MOUSE_Y_SIGN*ysign +
		    (mouse_buttons & 7), DCMOUSE_PORT);
		d->add_to_rx_queue(d->add_data, xdelta, DCMOUSE_PORT);
		d->add_to_rx_queue(d->add_data, ydelta, DCMOUSE_PORT);
		break;

	default:
		/*  TODO:  prompt mode and perhaps more stuff  */
		fatal("[ lk201: mouse mode 0x%02x unknown: TODO ]\n",
		    d->mouse_mode);
		exit(1);
	}

	return 1;
}


/*
 *  lk201_tick():
 *
 *  This function should be called "every now and then".
 *  If a key is available from the keyboard, add it to the rx queue.
 *  If other bits are set, an interrupt might need to be caused.
 */
void lk201_tick(struct machine *machine, struct lk201_data *d)
{
	int mouse_x, mouse_y, mouse_buttons, mouse_fb_nr;

	if (console_charavail(d->console_handle)) {
		unsigned char ch = console_readchar(d->console_handle);
		if (d->use_fb)
			lk201_convert_ascii_to_keybcode(d, ch);
		else {
			/*
			 *  This is ugly, but necessary because different
			 *  machines seem to use different ports for their
			 *  serial console:
			 *
			 *  DEC MIPSMATE 5100 uses the keyboard port.
			 *  DECstation 3100 (PMAX) and 5000/2000 (3MAX) use
			 *  the printer port.
			 *  Others seem to use the comm port.
			 */
			if (machine->machine_type == MACHINE_PMAX) {
				switch (machine->machine_subtype) {
				case MACHINE_DEC_MIPSMATE_5100:
					d->add_to_rx_queue(d->add_data,
					    ch, DCKBD_PORT);
					break;
				case MACHINE_DEC_PMAX_3100:
				case MACHINE_DEC_3MAX_5000:
					d->add_to_rx_queue(d->add_data,
					    ch, DCPRINTER_PORT);
					break;
				default:
					d->add_to_rx_queue(d->add_data,
					    ch, DCCOMM_PORT);
				}
			} else {
				d->add_to_rx_queue(d->add_data,
				    ch, DCCOMM_PORT);
			}
		}
	}

	/*  Don't do mouse updates if we're running in serial console mode:  */
	if (!d->use_fb)
		return;

	console_getmouse(&mouse_x, &mouse_y, &mouse_buttons,
	    &mouse_fb_nr);

	lk201_send_mouse_update_sequence(d, mouse_x, mouse_y,
	    mouse_buttons, mouse_fb_nr);
}


void lk201_tx_data(struct lk201_data *d, int port, int idata)
{
	switch (port) {
	case DCKBD_PORT:		/*  port 0  */
		if (!d->use_fb) {
			/*  Simply print the character to stdout:  */
			console_putchar(d->console_handle, idata);
		} else {
			switch (idata) {
			case LK_LED_DISABLE:	/*  0x11  */
				break;
			case LK_LED_ENABLE:	/*  0x13  */
				break;
			case LK_BELL_ENABLE:	/*  0x23  */
				break;
			case LED_1:
			case LED_2:
				break;
			case LED_3:
				break;
			case LED_4:
				break;
			case LK_KBD_ENABLE:	/*  0x8b  */
				break;
			case LED_ALL:		/*  0x8f  */
				break;
			case LK_RING_BELL:	/*  0xa7  */
				break;
			case 0xab:	/*  Get Keyboard ID:  */
				/*
				 *  First byte:
				 *	1 = LK201
				 *	2 = LK401
				 *	3 = LK443
				 *	4 = LK421
				 *
				 *  TODO: What about the second byte?
				 */
				d->add_to_rx_queue(d->add_data,
				    0x01, DCKBD_PORT);
				d->add_to_rx_queue(d->add_data,
				    0x22, DCKBD_PORT);
				break;
			case LK_DEFAULTS:	/*  0xd3  */
				/*  TODO?  */
				break;
			case 0xfd:	/*  Keyboard self-test:  */
				/*  Suitable return values according to
				    Mach/PMAX source code:  */
				d->add_to_rx_queue(d->add_data,
				    0x01, DCKBD_PORT);
				d->add_to_rx_queue(d->add_data,
				    0x00, DCKBD_PORT);
				d->add_to_rx_queue(d->add_data,
				    0x00, DCKBD_PORT);
				d->add_to_rx_queue(d->add_data,
				    0x00, DCKBD_PORT);
				break;
			default:
				debug("[ lk201: keyboard control: 0x%x ]\n",
				    idata);
			}
		}
		break;
	case DCMOUSE_PORT:		/*  port 1  */
		debug("[ lk201: writing data to MOUSE: 0x%x", idata);
		if (idata == MOUSE_INCREMENTAL) {
			d->mouse_mode = MOUSE_INCREMENTAL;
		} else if (idata == MOUSE_SELF_TEST) {
			/*
			 *  Mouse self-test:
			 *
			 *  TODO: Find out if this is correct. The lowest
			 *        four bits of the second byte should be
			 *        0x2, according to NetBSD/pmax. But the
			 *        other bits and bytes?
			 */
			debug(" (mouse self-test request)");
			d->add_to_rx_queue(d->add_data,
			    0xa0 | d->mouse_revision, DCMOUSE_PORT);
			d->add_to_rx_queue(d->add_data, 0x02, DCMOUSE_PORT);
			d->add_to_rx_queue(d->add_data, 0x00, DCMOUSE_PORT);
			d->add_to_rx_queue(d->add_data, 0x00, DCMOUSE_PORT);
		} else
			debug(" UNKNOWN byte; TODO");
		debug(" ]\n");
		break;
	case DCCOMM_PORT:		/*  port 2  */
	case DCPRINTER_PORT:		/*  port 3  */
		/*  Simply print the character to stdout:  */
		console_putchar(d->console_handle, idata);
	}
}


/*
 *  lk201_init():
 *
 *  Initialize lk201 keyboard/mouse settings.
 */
void lk201_init(struct lk201_data *d, int use_fb,
	void (*add_to_rx_queue)(void *,int,int),
	int console_handle, void *add_data)
{
	memset(d, 0, sizeof(struct lk201_data));

	d->add_to_rx_queue = add_to_rx_queue;
	d->add_data = add_data;

	d->use_fb = use_fb;
	d->mouse_mode = 0;
	d->mouse_revision = 0;	/*  0..15  */
	d->console_handle = console_handle;
}

