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
 *  COMMENT: 8042 PC keyboard controller (+ 8242WB Keyboard/Mouse controller)
 *
 *  This module includes emulation of the 8048 keyboard chip too.
 *
 *  Quick source of good info: http://my.execpc.com/~geezer/osd/kbd/kbd.txt
 *
 *
 *  TODOs:
 *	Finish the rewrite for 8242.
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

#include "thirdparty/kbdreg.h"


/*  #define PCKBC_DEBUG  */
/*  #define debug fatal  */


#define	MAX_8042_QUEUELEN	256

#define	PC_DATA			0
#define	PC_CMD			0
#define	PC_STATUS		1

#define	PS2_TXBUF		0
#define	PS2_RXBUF		1
#define	PS2_CONTROL		2
#define	PS2_STATUS		3

#define	PS2	100

#define	PCKBC_TICKSHIFT		15

struct pckbc_data {
	int		console_handle;
	int		in_use;

	int		reg[DEV_PCKBC_LENGTH];

	struct interrupt irq_keyboard;
	struct interrupt irq_mouse;
	int		currently_asserted[2];
	int		type;
	int		pc_style_flag;

	/*  TODO: one of these for each port?  */
	int		clocksignal;
	int		rx_int_enable;
	int		tx_int_enable;

	int		keyscanning_enabled;
	int		translation_table;
	int		state;
	int		cmdbyte;
	int		output_byte;
	int		last_scancode;

	unsigned	key_queue[2][MAX_8042_QUEUELEN];
	int		head[2], tail[2];
};

#define	STATE_NORMAL			0
#define	STATE_LDCMDBYTE			1
#define	STATE_RDCMDBYTE			2
#define	STATE_WAITING_FOR_TRANSLTABLE	3
#define	STATE_WAITING_FOR_RATE		4
#define	STATE_WAITING_FOR_ONEKEY_MB	5
#define	STATE_WAITING_FOR_AUX		6
#define	STATE_WAITING_FOR_AUX_OUT	7
#define	STATE_LDOUTPUT			8
#define	STATE_RDOUTPUT			9


/*
 *  pckbc_add_code():
 *
 *  Adds a byte to the data queue.
 */
void pckbc_add_code(struct pckbc_data *d, int code, int port)
{
	/*  Add at the head, read at the tail:  */
	d->head[port] = (d->head[port]+1) % MAX_8042_QUEUELEN;
	if (d->head[port] == d->tail[port])
		fatal("[ pckbc: queue overrun, port %i! ]\n", port);

	d->key_queue[port][d->head[port]] = code;
}


/*
 *  pckbc_get_code():
 *
 *  Reads a byte from a data queue.
 */
int pckbc_get_code(struct pckbc_data *d, int port)
{
	if (d->head[port] == d->tail[port])
		fatal("[ pckbc: queue empty, port %i! ]\n", port);
	else
		d->tail[port] = (d->tail[port]+1) % MAX_8042_QUEUELEN;
	return d->key_queue[port][d->tail[port]];
}


/*
 *  ascii_to_scancodes_type3():
 *
 *  Conversion from ASCII codes to default (US) keyboard scancodes.
 *  (See http://www.computer-engineering.org/ps2keyboard/scancodes3.html)
 */
static void ascii_to_pc_scancodes_type3(int a, struct pckbc_data *d)
{
	int old_head;
	int p = 0;	/*  port  */
	int shift = 0, ctrl = 0;

	if (a >= 'A' && a <= 'Z') { a += 32; shift = 1; }
	if ((a >= 1 && a <= 26) && (a!='\n' && a!='\t' && a!='\b' && a!='\r'))
		{ a += 96; ctrl = 1; }
	if (a=='!')  {	a = '1'; shift = 1; }
	if (a=='@')  {	a = '2'; shift = 1; }
	if (a=='#')  {	a = '3'; shift = 1; }
	if (a=='$')  {	a = '4'; shift = 1; }
	if (a=='%')  {	a = '5'; shift = 1; }
	if (a=='^')  {	a = '6'; shift = 1; }
	if (a=='&')  {	a = '7'; shift = 1; }
	if (a=='*')  {	a = '8'; shift = 1; }
	if (a=='(')  {	a = '9'; shift = 1; }
	if (a==')')  {	a = '0'; shift = 1; }
	if (a=='_')  {	a = '-'; shift = 1; }
	if (a=='+')  {	a = '='; shift = 1; }
	if (a=='{')  {	a = '['; shift = 1; }
	if (a=='}')  {	a = ']'; shift = 1; }
	if (a==':')  {	a = ';'; shift = 1; }
	if (a=='"')  {	a = '\''; shift = 1; }
	if (a=='|')  {	a = '\\'; shift = 1; }
	if (a=='<')  {	a = ','; shift = 1; }
	if (a=='>')  {	a = '.'; shift = 1; }
	if (a=='?')  {	a = '/'; shift = 1; }

	if (shift)
		pckbc_add_code(d, 0x12, p);
	if (ctrl)
		pckbc_add_code(d, 0x11, p);

	/*
	 *  Note: The ugly hack used to add release codes for all of these
	 *  keys is as follows:  we remember how much of the kbd buf that
	 *  is in use here, before we add any scancode. After we've added
	 *  one or more scancodes (ie an optional shift + another key)
	 *  then we add 0xf0 + the last scancode _if_ the kbd buf was altered.
	 */

	old_head = d->head[p];

	if (a==27)	pckbc_add_code(d, 0x08, p);

	if (a=='1')	pckbc_add_code(d, 0x16, p);
	if (a=='2')	pckbc_add_code(d, 0x1e, p);
	if (a=='3')	pckbc_add_code(d, 0x26, p);
	if (a=='4')	pckbc_add_code(d, 0x25, p);
	if (a=='5')	pckbc_add_code(d, 0x2e, p);
	if (a=='6')	pckbc_add_code(d, 0x36, p);
	if (a=='7')	pckbc_add_code(d, 0x3d, p);
	if (a=='8')	pckbc_add_code(d, 0x3e, p);
	if (a=='9')	pckbc_add_code(d, 0x46, p);
	if (a=='0')	pckbc_add_code(d, 0x45, p);
	if (a=='-')	pckbc_add_code(d, 0x4e, p);
	if (a=='=')	pckbc_add_code(d, 0x55, p);

	if (a=='\b')	pckbc_add_code(d, 0x29, p);

	if (a=='\t')	pckbc_add_code(d, 0x0d, p);
	if (a=='q')	pckbc_add_code(d, 0x15, p);
	if (a=='w')	pckbc_add_code(d, 0x1d, p);
	if (a=='e')	pckbc_add_code(d, 0x24, p);
	if (a=='r')	pckbc_add_code(d, 0x2d, p);
	if (a=='t')	pckbc_add_code(d, 0x2c, p);
	if (a=='y')	pckbc_add_code(d, 0x35, p);
	if (a=='u')	pckbc_add_code(d, 0x3c, p);
	if (a=='i')	pckbc_add_code(d, 0x43, p);
	if (a=='o')	pckbc_add_code(d, 0x44, p);
	if (a=='p')	pckbc_add_code(d, 0x4d, p);

	if (a=='[')	pckbc_add_code(d, 0x54, p);
	if (a==']')	pckbc_add_code(d, 0x5b, p);

	if (a=='\n' || a=='\r')	pckbc_add_code(d, 0x5a, p);

	if (a=='a')	pckbc_add_code(d, 0x1c, p);
	if (a=='s')	pckbc_add_code(d, 0x1b, p);
	if (a=='d')	pckbc_add_code(d, 0x23, p);
	if (a=='f')	pckbc_add_code(d, 0x2b, p);
	if (a=='g')	pckbc_add_code(d, 0x34, p);
	if (a=='h')	pckbc_add_code(d, 0x33, p);
	if (a=='j')	pckbc_add_code(d, 0x3b, p);
	if (a=='k')	pckbc_add_code(d, 0x42, p);
	if (a=='l')	pckbc_add_code(d, 0x4b, p);

	if (a==';')	pckbc_add_code(d, 0x4c, p);
	if (a=='\'')	pckbc_add_code(d, 0x52, p);
/*	if (a=='~')	pckbc_add_code(d, 0x29, p);  ?  */
	if (a=='\\')	pckbc_add_code(d, 0x5c, p);

	if (a=='z')	pckbc_add_code(d, 0x1a, p);
	if (a=='x')	pckbc_add_code(d, 0x22, p);
	if (a=='c')	pckbc_add_code(d, 0x21, p);
	if (a=='v')	pckbc_add_code(d, 0x2a, p);
	if (a=='b')	pckbc_add_code(d, 0x32, p);
	if (a=='n')	pckbc_add_code(d, 0x31, p);
	if (a=='m')	pckbc_add_code(d, 0x3a, p);

	if (a==',')	pckbc_add_code(d, 0x41, p);
	if (a=='.')	pckbc_add_code(d, 0x49, p);
	if (a=='/')	pckbc_add_code(d, 0x4a, p);

	if (a==' ')	pckbc_add_code(d, 0x29, p);

	/*  Add release code, if a key was pressed:  */
	if (d->head[p] != old_head) {
		int code = d->key_queue[p][d->head[p]];
		pckbc_add_code(d, 0xf0, p);
		pckbc_add_code(d, code, p);
	}

	/*  Release shift and ctrl:  */
	if (shift) {
		pckbc_add_code(d, 0xf0, p);
		pckbc_add_code(d, 0x12, p);
	}
	if (ctrl) {
		pckbc_add_code(d, 0xf0, p);
		pckbc_add_code(d, 0x11, p);
	}
}


/*
 *  ascii_to_scancodes_type2():
 *
 *  Conversion from ASCII codes to default (US) keyboard scancodes.
 *  (See http://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html)
 *
 *  NOTE/TODO: This seems to be type 2, not type 1.
 */
static void ascii_to_pc_scancodes_type2(int a, struct pckbc_data *d)
{
	int old_head;
	int p = 0;	/*  port  */
	int shift = 0, ctrl = 0;

	if (d->translation_table == 3) {
		ascii_to_pc_scancodes_type3(a, d);
		return;
	}

	if (d->translation_table != 2) {
		fatal("[ ascii_to_pc_scancodes: unimplemented type! ]\n");
		return;
	}

	if (a >= 'A' && a <= 'Z') { a += 32; shift = 1; }
	if ((a >= 1 && a <= 26) && (a!='\n' && a!='\t' && a!='\b' && a!='\r'))
		{ a += 96; ctrl = 1; }

	if (a=='!')  {	a = '1'; shift = 1; }
	if (a=='@')  {	a = '2'; shift = 1; }
	if (a=='#')  {	a = '3'; shift = 1; }
	if (a=='$')  {	a = '4'; shift = 1; }
	if (a=='%')  {	a = '5'; shift = 1; }
	if (a=='^')  {	a = '6'; shift = 1; }
	if (a=='&')  {	a = '7'; shift = 1; }
	if (a=='*')  {	a = '8'; shift = 1; }
	if (a=='(')  {	a = '9'; shift = 1; }
	if (a==')')  {	a = '0'; shift = 1; }
	if (a=='_')  {	a = '-'; shift = 1; }
	if (a=='+')  {	a = '='; shift = 1; }
	if (a=='{')  {	a = '['; shift = 1; }
	if (a=='}')  {	a = ']'; shift = 1; }
	if (a==':')  {	a = ';'; shift = 1; }
	if (a=='"')  {	a = '\''; shift = 1; }
	if (a=='|')  {	a = '\\'; shift = 1; }
	if (a=='<')  {	a = ','; shift = 1; }
	if (a=='>')  {	a = '.'; shift = 1; }
	if (a=='?')  {	a = '/'; shift = 1; }
	if (a=='~')  {	a = '`'; shift = 1; }

	if (shift)
		pckbc_add_code(d, 0x2a, p);
	else
		pckbc_add_code(d, 0x2a + 0x80, p);

	if (ctrl)
		pckbc_add_code(d, 0x1d, p);

	/*
	 *  Note: The ugly hack used to add release codes for all of these
	 *  keys is as follows:  we remember how much of the kbd buf that
	 *  is in use here, before we add any scancode. After we've added
	 *  one or more scancodes (ie an optional shift + another key)
	 *  then we duplicate the last scancode | 0x80 _if_ the kbd buf
	 *  was altered.
	 */

	old_head = d->head[p];

	if (a==27)	pckbc_add_code(d, 0x01, p);

	if (a=='1')	pckbc_add_code(d, 0x02, p);
	if (a=='2')	pckbc_add_code(d, 0x03, p);
	if (a=='3')	pckbc_add_code(d, 0x04, p);
	if (a=='4')	pckbc_add_code(d, 0x05, p);
	if (a=='5')	pckbc_add_code(d, 0x06, p);
	if (a=='6')	pckbc_add_code(d, 0x07, p);
	if (a=='7')	pckbc_add_code(d, 0x08, p);
	if (a=='8')	pckbc_add_code(d, 0x09, p);
	if (a=='9')	pckbc_add_code(d, 0x0a, p);
	if (a=='0')	pckbc_add_code(d, 0x0b, p);
	if (a=='-')	pckbc_add_code(d, 0x0c, p);
	if (a=='=')	pckbc_add_code(d, 0x0d, p);

	if (a=='!')  {	pckbc_add_code(d, 0x2a, p);
			pckbc_add_code(d, 0x02, p); }
	if (a=='@')  {	pckbc_add_code(d, 0x2a, p);
			pckbc_add_code(d, 0x03, p); }
	if (a=='#')  {	pckbc_add_code(d, 0x2a, p);
			pckbc_add_code(d, 0x04, p); }
	if (a=='$')  {	pckbc_add_code(d, 0x2a, p);
			pckbc_add_code(d, 0x05, p); }
	if (a=='%')  {	pckbc_add_code(d, 0x2a, p);
			pckbc_add_code(d, 0x06, p); }
	if (a=='^')  {	pckbc_add_code(d, 0x2a, p);
			pckbc_add_code(d, 0x07, p); }
	if (a=='&')  {	pckbc_add_code(d, 0x2a, p);
			pckbc_add_code(d, 0x08, p); }
	if (a=='*')  {	pckbc_add_code(d, 0x2a, p);
			pckbc_add_code(d, 0x09, p); }
	if (a=='(')  {	pckbc_add_code(d, 0x2a, p);
			pckbc_add_code(d, 0x0a, p); }

	if (a=='\b')	pckbc_add_code(d, 0x0e, p);

	if (a=='\t')	pckbc_add_code(d, 0x0f, p);
	if (a=='q')	pckbc_add_code(d, 0x10, p);
	if (a=='w')	pckbc_add_code(d, 0x11, p);
	if (a=='e')	pckbc_add_code(d, 0x12, p);
	if (a=='r')	pckbc_add_code(d, 0x13, p);
	if (a=='t')	pckbc_add_code(d, 0x14, p);
	if (a=='y')	pckbc_add_code(d, 0x15, p);
	if (a=='u')	pckbc_add_code(d, 0x16, p);
	if (a=='i')	pckbc_add_code(d, 0x17, p);
	if (a=='o')	pckbc_add_code(d, 0x18, p);
	if (a=='p')	pckbc_add_code(d, 0x19, p);

	if (a=='[')	pckbc_add_code(d, 0x1a, p);
	if (a==']')	pckbc_add_code(d, 0x1b, p);

	if (a=='\n' || a=='\r')	pckbc_add_code(d, 0x1c, p);

	if (a=='a')	pckbc_add_code(d, 0x1e, p);
	if (a=='s')	pckbc_add_code(d, 0x1f, p);
	if (a=='d')	pckbc_add_code(d, 0x20, p);
	if (a=='f')	pckbc_add_code(d, 0x21, p);
	if (a=='g')	pckbc_add_code(d, 0x22, p);
	if (a=='h')	pckbc_add_code(d, 0x23, p);
	if (a=='j')	pckbc_add_code(d, 0x24, p);
	if (a=='k')	pckbc_add_code(d, 0x25, p);
	if (a=='l')	pckbc_add_code(d, 0x26, p);

	if (a==';')	pckbc_add_code(d, 0x27, p);
	if (a=='\'')	pckbc_add_code(d, 0x28, p);
	if (a=='`')	pckbc_add_code(d, 0x29, p);
	if (a=='\\')	pckbc_add_code(d, 0x2b, p);

	if (a=='z')	pckbc_add_code(d, 0x2c, p);
	if (a=='x')	pckbc_add_code(d, 0x2d, p);
	if (a=='c')	pckbc_add_code(d, 0x2e, p);
	if (a=='v')	pckbc_add_code(d, 0x2f, p);
	if (a=='b')	pckbc_add_code(d, 0x30, p);
	if (a=='n')	pckbc_add_code(d, 0x31, p);
	if (a=='m')	pckbc_add_code(d, 0x32, p);

	if (a==',')	pckbc_add_code(d, 0x33, p);
	if (a=='.')	pckbc_add_code(d, 0x34, p);
	if (a=='/')	pckbc_add_code(d, 0x35, p);

	if (a==' ')	pckbc_add_code(d, 0x39, p);

	/*  Add release code, if a key was pressed:  */
	if (d->head[p] != old_head) {
		int code = d->key_queue[p][d->head[p]] | 0x80;
		pckbc_add_code(d, code, p);
	}

	/*  Release ctrl:  */
	if (ctrl)
		pckbc_add_code(d, 0x1d + 0x80, p);
}


DEVICE_TICK(pckbc)
{
	struct pckbc_data *d = (struct pckbc_data *) extra;
	int port_nr, ch, ints_enabled;

	if (d->in_use && console_charavail(d->console_handle)) {
		ch = console_readchar(d->console_handle);
		if (ch >= 0)
			ascii_to_pc_scancodes_type2(ch, d);
	}

	ints_enabled = d->rx_int_enable;

	/*  TODO: mouse movements?  */

	if (d->cmdbyte & KC8_KDISABLE)
		ints_enabled = 0;

	for (port_nr=0; port_nr<2; port_nr++) {
		/*  Cause receive interrupt, if there's something in the
		    receive buffer: (Otherwise deassert the interrupt.)  */

		if (d->head[port_nr] != d->tail[port_nr] && ints_enabled) {
			debug("[ pckbc: interrupt port %i ]\n", port_nr);
			if (port_nr == 0)
				INTERRUPT_ASSERT(d->irq_keyboard);
			else
				INTERRUPT_ASSERT(d->irq_mouse);
			d->currently_asserted[port_nr] = 1;
		} else {
			if (d->currently_asserted[port_nr]) {
				if (port_nr == 0)
					INTERRUPT_DEASSERT(d->irq_keyboard);
				else
					INTERRUPT_DEASSERT(d->irq_mouse);
			}
			d->currently_asserted[port_nr] = 0;
		}
	}
}


/*
 *  dev_pckbc_command():
 *
 *  Handle commands to the 8048 in the emulated keyboard.
 */
static void dev_pckbc_command(struct pckbc_data *d, int port_nr)
{
	int cmd = d->reg[PC_CMD];

	if (d->type == PCKBC_8242)
		cmd = d->reg[PS2_TXBUF];

	if (d->state == STATE_WAITING_FOR_TRANSLTABLE) {
		debug("[ pckbc: (port %i) switching to translation table "
		    "0x%02x ]\n", port_nr, cmd);
		switch (cmd) {
		case 2:
		case 3:	d->translation_table = cmd;
			break;
		default:fatal("[ pckbc: (port %i) translation table "
			    "0x%02x is NOT YET IMPLEMENTED ]\n",
			    port_nr, cmd);
		}
		pckbc_add_code(d, KBR_ACK, port_nr);
		d->state = STATE_NORMAL;
		return;
	}

	if (d->state == STATE_WAITING_FOR_RATE) {
		debug("[ pckbc: (port %i) received Typematic Rate data: "
		    "0x%02x ]\n", port_nr, cmd);
		pckbc_add_code(d, KBR_ACK, port_nr);
		d->state = STATE_NORMAL;
		return;
	}

	if (d->state == STATE_WAITING_FOR_ONEKEY_MB) {
		debug("[ pckbc: (port %i) received One-key make/break data: "
		    "0x%02x ]\n", port_nr, cmd);
		pckbc_add_code(d, KBR_ACK, port_nr);
		d->state = STATE_NORMAL;
		return;
	}

	if (d->state == STATE_WAITING_FOR_AUX) {
		debug("[ pckbc: (port %i) received aux data: "
		    "0x%02x ]\n", port_nr, cmd);
		/*  Echo back.  */
		pckbc_add_code(d, cmd, port_nr);
		d->state = STATE_NORMAL;
		return;
	}

	if (d->state == STATE_WAITING_FOR_AUX_OUT) {
		debug("[ pckbc: (port %i) received aux out data: "
		    "0x%02x ]\n", port_nr, cmd);
		/*  Echo back.  */
		pckbc_add_code(d, cmd, port_nr);
		d->state = STATE_NORMAL;
		return;
	}

	switch (cmd) {

	case 0x00:
		/*
		 *  TODO: What does this do? This is possibly due to an
		 *  error in the handling of some other command code.
		 */
		pckbc_add_code(d, KBR_ACK, port_nr);
		break;

	case KBC_MODEIND:	/*  Set LEDs  */
		/*  Just ACK, no LEDs are actually set.  */
		pckbc_add_code(d, KBR_ACK, port_nr);
		break;

	case KBC_SETTABLE:
		pckbc_add_code(d, KBR_ACK, port_nr);
		d->state = STATE_WAITING_FOR_TRANSLTABLE;
		break;

	case KBC_ENABLE:
		d->keyscanning_enabled = 1;
		pckbc_add_code(d, KBR_ACK, port_nr);
		break;

	case KBC_DISABLE:
		d->keyscanning_enabled = 0;
		pckbc_add_code(d, KBR_ACK, port_nr);
		break;

	case KBC_SETDEFAULT:
		pckbc_add_code(d, KBR_ACK, port_nr);
		break;

	case KBC_GETID:
		/*  Get keyboard ID.  NOTE/TODO: Ugly hardcoded answer.  */
		pckbc_add_code(d, KBR_ACK, port_nr);
		pckbc_add_code(d, 0xab, port_nr);
		pckbc_add_code(d, 0x41, port_nr);
		break;

	case KBC_TYPEMATIC:
		/*  Set typematic (auto-repeat) delay/speed:  */
		pckbc_add_code(d, KBR_ACK, port_nr);
		d->state = STATE_WAITING_FOR_RATE;
		break;

	case KBC_ALLKEYS_TMB:
		/*  "Make all keys typematic/make/break"  */
		pckbc_add_code(d, KBR_ACK, port_nr);
		break;

	case KBC_ONEKEY_MB:
		/*  "Make one key typematic/make/break"  */
		pckbc_add_code(d, KBR_ACK, port_nr);
		d->state = STATE_WAITING_FOR_ONEKEY_MB;
		break;

	case KBC_RESET:
		pckbc_add_code(d, KBR_ACK, port_nr);
		pckbc_add_code(d, KBR_RSTDONE, port_nr);
		/*
		 *  Disable interrupts during reset, or Linux 2.6
		 *  prints warnings about spurious interrupts.
		 */
		d->rx_int_enable = 0;
		break;

	default:
		fatal("[ pckbc: (port %i) UNIMPLEMENTED 8048 command"
		    " 0x%02x ]\n", port_nr, cmd);
		exit(1);
	}
}


DEVICE_ACCESS(pckbc)
{
	struct pckbc_data *d = (struct pckbc_data *) extra;
	uint64_t idata = 0, odata = 0;
	int port_nr = 0;
	size_t i;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

#ifdef PCKBC_DEBUG
	if (writeflag == MEM_WRITE)
		fatal("[ pckbc: write to addr 0x%x: 0x%x ]\n",
		    (int)relative_addr, (int)idata);
	else
		fatal("[ pckbc: read from addr 0x%x ]\n",
		    (int)relative_addr);
#endif

	/*  For JAZZ-based machines:  */
	if (relative_addr >= 0x60) {
		relative_addr -= 0x60;
		if (relative_addr != 0)
			relative_addr = 1;
	} else if (d->type == PCKBC_8242) {
		/*  8242 PS2-style:  */
		/*  when using 8-byte alignment...  */
		relative_addr /= sizeof(uint64_t);
		/*  port_nr = 0 for keyboard, 1 for mouse  */
		port_nr = (relative_addr >> 2);
		relative_addr &= 3;
		relative_addr += PS2;
	} else if (d->pc_style_flag) {
		/*  PC-style:  */
		if (relative_addr != 0 && relative_addr != 4) {
			/*  TODO (port 0x61)  */
			odata = 0x21;
{
static int x = 0;
x++;
if (x&1)
			odata ^= 0x10;
}
			if (writeflag == MEM_READ)
				memory_writemax64(cpu, data, len, odata);
			return 1;
		}
		if (relative_addr != 0)
			relative_addr = 1;
	} else {
		/*  Others... Non-Jazz ARC-based machines etc.  */
		if (relative_addr != 0)
			relative_addr = 1;
	}

	switch (relative_addr) {

	/*
	 *  8042 (PC):
	 */

	case 0:		/*  data  */
		if (writeflag==MEM_READ) {
			switch (d->state) {
			case STATE_RDCMDBYTE:
				odata = d->cmdbyte;
				d->state = STATE_NORMAL;
				break;
			case STATE_RDOUTPUT:
				odata = d->output_byte;
				d->state = STATE_NORMAL;
				break;
			default:if (d->head[0] != d->tail[0]) {
					odata = pckbc_get_code(d, 0);
					d->last_scancode = odata;
				} else {
					odata = d->last_scancode;
					d->last_scancode |= 0x80;
				}
			}
			/*  debug("[ pckbc: read from DATA: 0x%02x ]\n",
			    (int)odata);  */
		} else {
			debug("[ pckbc: write to DATA:");
			for (i=0; i<len; i++)
				debug(" %02x", data[i]);
			debug(" ]\n");

			switch (d->state) {
			case STATE_LDCMDBYTE:
				d->cmdbyte = idata;
				d->rx_int_enable = d->cmdbyte &
				    (KC8_KENABLE | KC8_MENABLE) ? 1 : 0;
				d->state = STATE_NORMAL;
				break;
			case STATE_LDOUTPUT:
				d->output_byte = idata;
				d->state = STATE_NORMAL;
				break;
			default:d->reg[relative_addr] = idata;
				dev_pckbc_command(d, port_nr);
			}
		}
		break;
	case 1:		/*  control  */
		if (writeflag==MEM_READ) {
			dev_pckbc_tick(cpu, d);

			odata = 0;

			/*  "Data in buffer" bit  */
			if (d->head[0] != d->tail[0] ||
			    d->state == STATE_RDCMDBYTE ||
			    d->state == STATE_RDOUTPUT)
				odata |= KBS_DIB;

			if (d->state == STATE_RDCMDBYTE)
				odata |= KBS_OCMD;

			odata |= KBS_NOSEC;
			/*  debug("[ pckbc: read from CTL status port: "
			    "0x%02x ]\n", (int)odata);  */
		} else {
			debug("[ pckbc: write to CTL:");
			for (i=0; i<len; i++)
				debug(" %02x", data[i]);
			debug(" ]\n");
			d->reg[relative_addr] = idata;

			switch (idata) {
			case 0x10:
			case 0x11:
				/*  TODO: For now, don't print warnings about
				    these. NetBSD sends these.  */
				break;
			case K_RDCMDBYTE:
				d->state = STATE_RDCMDBYTE;
				break;
			case K_LDCMDBYTE:
				d->state = STATE_LDCMDBYTE;
				break;
			case 0xa7:
				d->cmdbyte |= KC8_MDISABLE;
				break;
			case 0xa8:
				d->cmdbyte &= ~KC8_MDISABLE;
				break;
			case 0xa9:	/*  test auxiliary port  */
				debug("[ pckbc: CONTROL 0xa9, TODO ]\n");
				break;
			case 0xaa:	/*  keyboard self-test  */
				pckbc_add_code(d, 0x55, port_nr);
				break;
			case 0xab:	/*  keyboard interface self-test  */
				pckbc_add_code(d, 0x00, port_nr);
				break;
			case 0xad:
				d->cmdbyte |= KC8_KDISABLE;
				break;
			case 0xae:
				d->cmdbyte &= ~KC8_KDISABLE;
				break;
			case 0xd0:
				d->state = STATE_RDOUTPUT;
				break;
			case 0xd1:
				d->state = STATE_LDOUTPUT;
				break;
			case 0xd3:	/*  write to auxiliary device
					    output buffer  */
				debug("[ pckbc: CONTROL 0xd3, TODO ]\n");
				d->state = STATE_WAITING_FOR_AUX_OUT;
				break;
			case 0xd4:	/*  write to auxiliary port  */
				debug("[ pckbc: CONTROL 0xd4, TODO ]\n");
				d->state = STATE_WAITING_FOR_AUX;
				break;
			default:
				fatal("[ pckbc: unknown CONTROL 0x%x ]\n",
				    (int)idata);
				d->state = STATE_NORMAL;
			}
		}
		break;

	/*
	 *  8242 (PS2):
	 */

	case PS2 + PS2_TXBUF:
		if (writeflag==MEM_READ) {
			odata = random() & 0xff;
			debug("[ pckbc: read from port %i, PS2_TXBUF: "
			    "0x%x ]\n", port_nr, (int)odata);
		} else {
			debug("[ pckbc: write to port %i, PS2_TXBUF: "
			    "0x%llx ]\n", port_nr, (long long)idata);

			/*  Handle keyboard commands:  */
			d->reg[PS2_TXBUF] = idata;
			dev_pckbc_command(d, port_nr);
		}
		break;

	case PS2 + PS2_RXBUF:
		if (writeflag==MEM_READ) {
			/*  TODO: What should be returned if no data 
			    is available?  */
			odata = random() & 0xff;
			if (d->head[port_nr] != d->tail[port_nr])
				odata = pckbc_get_code(d, port_nr);
			debug("[ pckbc: read from port %i, PS2_RXBUF: "
			    "0x%02x ]\n", port_nr, (int)odata);
		} else {
			debug("[ pckbc: write to port %i, PS2_RXBUF: "
			    "0x%llx ]\n", port_nr, (long long)idata);
		}
		break;

	case PS2 + PS2_CONTROL:
		if (writeflag==MEM_READ) {
			debug("[ pckbc: read from port %i, PS2_CONTROL"
			    " ]\n", port_nr);
		} else {
			debug("[ pckbc: write to port %i, PS2_CONTROL:"
			    " 0x%llx ]\n", port_nr, (long long)idata);
			d->clocksignal = (idata & 0x10) ? 1 : 0;
			d->rx_int_enable = (idata & 0x08) ? 1 : 0;
			d->tx_int_enable = (idata & 0x04) ? 1 : 0;
		}
		break;

	case PS2 + PS2_STATUS:
		if (writeflag==MEM_READ) {
			/* 0x08 = transmit buffer empty  */
			odata = d->clocksignal + 0x08;

			if (d->head[port_nr] != d->tail[port_nr]) {
				/*  0x10 = receicer data available (?)  */
				odata |= 0x10;
			}

			debug("[ pckbc: read from port %i, PS2_STATUS: "
			    "0x%llx ]\n", port_nr, (long long)odata);
		} else {
			debug("[ pckbc: write to port %i, PS2_STATUS: "
			    "0x%llx ]\n", port_nr, (long long)idata);
		}
		break;

	default:
		if (writeflag==MEM_READ) {
			debug("[ pckbc: read from unimplemented reg %i ]\n",
			    (int)relative_addr);
			odata = d->reg[relative_addr % DEV_PCKBC_LENGTH];
		} else {
			debug("[ pckbc: write to unimplemented reg %i:",
			    (int)relative_addr);
			for (i=0; i<len; i++)
				debug(" %02x", data[i]);
			debug(" ]\n");
			d->reg[relative_addr % DEV_PCKBC_LENGTH] = idata;
		}
	}

	/*  SGI? TODO: fix  */
	if (len == 8)
		odata |= (odata << 8) | (odata << 16) | (odata << 24) |
		    (odata << 32) | (odata << 40) | (odata << 48) |
		    (odata << 56);

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	dev_pckbc_tick(cpu, d);

	return 1;
}


/*
 *  dev_pckbc_init():
 *
 *  Type should be PCKBC_8042 or PCKBC_8242.
 */
int dev_pckbc_init(struct machine *machine, struct memory *mem,
	uint64_t baseaddr, int type, char *keyboard_irqpath,
	char *mouse_irqpath, int in_use, int pc_style_flag)
{
	struct pckbc_data *d;
	int len = DEV_PCKBC_LENGTH;

	CHECK_ALLOCATION(d = (struct pckbc_data *) malloc(sizeof(struct pckbc_data)));
	memset(d, 0, sizeof(struct pckbc_data));

	if (type == PCKBC_8242)
		len = 0x18;

	if (type == PCKBC_JAZZ) {
		type = PCKBC_8042;
		len = DEV_PCKBC_LENGTH + 0x60;
	}

	INTERRUPT_CONNECT(keyboard_irqpath, d->irq_keyboard);
	INTERRUPT_CONNECT(mouse_irqpath, d->irq_mouse);

	d->type              = type;
	d->in_use            = in_use;
	d->pc_style_flag     = pc_style_flag;
	d->translation_table = 2;
	d->rx_int_enable     = 1;
	d->output_byte       = 0x02;	/*  A20 enable on PCs  */

	d->console_handle = console_start_slave_inputonly(
	    machine, "pckbc", d->in_use);

	memory_device_register(mem, "pckbc", baseaddr,
	    len, dev_pckbc_access, d, DM_DEFAULT, NULL);
	machine_add_tickfunction(machine, dev_pckbc_tick, d,
	    PCKBC_TICKSHIFT);

	return d->console_handle;
}

