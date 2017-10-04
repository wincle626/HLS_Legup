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
 *  COMMENT: VR41xx (VR4122 and VR4131) misc functions
 *
 *  This is just a big hack.
 *
 *  TODO: Implement more functionality some day.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "console.h"
#include "cpu.h"
#include "device.h"
#include "devices.h"
#include "interrupt.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"
#include "timer.h"

#include "thirdparty/bcureg.h"
#include "thirdparty/vripreg.h"
#include "thirdparty/vrkiureg.h"
#include "thirdparty/vr_rtcreg.h"


/*  #define debug fatal  */

#define	DEV_VR41XX_TICKSHIFT		14

#define	DEV_VR41XX_LENGTH		0x800	/*  TODO?  */
struct vr41xx_data {
	struct interrupt cpu_irq;		/*  Connected to MIPS irq 2  */
	int		cpumodel;		/*  Model nr, e.g. 4121  */

	/*  KIU:  */
	int		kiu_console_handle;
	uint32_t	kiu_offset;
	struct interrupt kiu_irq;
	int		kiu_int_assert;
	int		old_kiu_int_assert;

	int		d0, d1, d2, d3, d4, d5;
	int		dont_clear_next;
	int		escape_state;

	/*  Timer:  */
	int		pending_timer_interrupts;
	struct interrupt timer_irq;
	struct timer	*timer;

	/*  See icureg.h in NetBSD for more info.  */
	uint16_t	sysint1;
	uint16_t	msysint1;
	uint16_t	giuint;
	uint16_t	giumask;
	uint16_t	sysint2;
	uint16_t	msysint2;
	struct interrupt giu_irq;
};


/*
 *  vr41xx_vrip_interrupt_assert():
 *  vr41xx_vrip_interrupt_deassert():
 */ 
void vr41xx_vrip_interrupt_assert(struct interrupt *interrupt)
{
	struct vr41xx_data *d = (struct vr41xx_data *) interrupt->extra;
	int line = interrupt->line;
	if (line < 16)
		d->sysint1 |= (1 << line);
	else
		d->sysint2 |= (1 << (line-16));
	if ((d->sysint1 & d->msysint1) | (d->sysint2 & d->msysint2))
                INTERRUPT_ASSERT(d->cpu_irq);
}
void vr41xx_vrip_interrupt_deassert(struct interrupt *interrupt)
{
	struct vr41xx_data *d = (struct vr41xx_data *) interrupt->extra;
	int line = interrupt->line;
	if (line < 16)
		d->sysint1 &= ~(1 << line);
	else
		d->sysint2 &= ~(1 << (line-16));
	if (!(d->sysint1 & d->msysint1) && !(d->sysint2 & d->msysint2))
                INTERRUPT_DEASSERT(d->cpu_irq);
}


/*
 *  vr41xx_giu_interrupt_assert():
 *  vr41xx_giu_interrupt_deassert():
 */ 
void vr41xx_giu_interrupt_assert(struct interrupt *interrupt)
{
	struct vr41xx_data *d = (struct vr41xx_data *) interrupt->extra;
	int line = interrupt->line;
	d->giuint |= (1 << line);
	if (d->giuint & d->giumask)
                INTERRUPT_ASSERT(d->giu_irq);
}
void vr41xx_giu_interrupt_deassert(struct interrupt *interrupt)
{
	struct vr41xx_data *d = (struct vr41xx_data *) interrupt->extra;
	int line = interrupt->line;
	d->giuint &= ~(1 << line);
	if (!(d->giuint & d->giumask))
                INTERRUPT_DEASSERT(d->giu_irq);
}


static void recalc_kiu_int_assert(struct cpu *cpu, struct vr41xx_data *d)
{
	if (d->kiu_int_assert != d->old_kiu_int_assert) {
		d->old_kiu_int_assert = d->kiu_int_assert;
		if (d->kiu_int_assert != 0)
			INTERRUPT_ASSERT(d->kiu_irq);
		else
			INTERRUPT_DEASSERT(d->kiu_irq);
	}
}


/*
 *  vr41xx_keytick():
 */
static void vr41xx_keytick(struct cpu *cpu, struct vr41xx_data *d)
{
	int keychange = 0;

	/*
	 *  Keyboard input:
	 *
	 *  Hardcoded for MobilePro. (See NetBSD's hpckbdkeymap.h for
	 *  info on other keyboard layouts. mobilepro780_keytrans is the
	 *  one used here.)
	 *
	 *  TODO: Make this work with "any" keyboard layout.
	 *
	 *  ofs 0:
	 *	8000='o' 4000='.' 2000=DOWN  1000=UP
	 *	 800=';'  400='''  200='['    100=?
	 *	  80='l'   40=CR    20=RIGHT   10=LEFT
	 *	   8='/'    4='\'    2=']'      1=SPACE
	 *  ofs 2:
	 *	8000='a' 4000='s' 2000='d' 1000='f'
	 *	 800='`'  400='-'  200='='  100=?
	 *	  80='z'   40='x'   20='c'   10='v'
	 *	   8=?      4=?      2=?
	 *  ofs 4:
	 *	8000='9' 4000='0' 2000=?   1000=?
	 *	 800='b'  400='n'  200='m'  100=','
	 *	  80='q'   40='w'   20='e'   10='r'
	 *	   8='5'    4='6'    2='7'    1='8'
	 *  ofs 6:
	 *	8000=ESC 4000=DEL 2000=CAPS 1000=?
	 *	 800='t'  400='y'  200='u'   100='i'
	 *	  80='1'   40='2'   20='3'    10='4'
	 *	   8='g'    4='h'    2='j'     1='k'
	 *  ofs 8:
	 *                      200=ALT_L
	 *	  80=    40=TAB  20='p' 10=BS
	 *	   8=     4=      2=     1=ALT_R
	 *  ofs a:
	 *	800=SHIFT 4=CTRL
	 *
	 *
	 *  The following are for the IBM WorkPad Z50:
	 *  (Not yet implemented, TODO)
	 *
	 *  00    f1      f3      f5      f7      f9      -       -       f11
	 *  08    f2      f4      f6      f8      f10     -       -       f12
	 *  10    '       [       -       0       p       ;       up      /
	 *  18    -       -       -       9       o       l       .       -
	 *  20    left    ]       =       8       i       k       ,       -
	 *  28    h       y       6       7       u       j       m       n
	 *  30    -       bs      num     del     -       \       ent     sp
	 *  38    g       t       5       4       r       f       v       b
	 *  40    -       -       -       3       e       d       c       right
	 *  48    -       -       -       2       w       s       x       down
	 *  50    esc     tab     ~       1       q       a       z       -
	 *  58    menu    Ls      Lc      Rc      La      Ra      Rs      -
	 */

	if (d->d0 != 0 || d->d1 != 0 || d->d2 != 0 ||
	    d->d3 != 0 || d->d4 != 0 || d->d5 != 0)
		keychange = 1;

	/*  Release all keys:  */
	if (!d->dont_clear_next) {
		d->d0 = d->d1 = d->d2 = d->d3 = d->d4 = d->d5 = 0;
	} else
		d->dont_clear_next = 0;

	if (console_charavail(d->kiu_console_handle)) {
		char ch = console_readchar(d->kiu_console_handle);

		if (d->escape_state > 0) {
			switch (d->escape_state) {
			case 1:	/*  expecting a [  */
				d->escape_state = 0;
				if (ch == '[')
					d->escape_state = 2;
				break;
			case 2:	/*  cursor keys etc:  */
				/*  Ugly hack for Mobilepro770:  */
				if (cpu->machine->machine_subtype ==
				    MACHINE_HPCMIPS_NEC_MOBILEPRO_770) {
					switch (ch) {
					case 'A': d->d0 = 0x2000; break;
					case 'B': d->d0 = 0x20; break;
					case 'C': d->d0 = 0x1000; break;
					case 'D': d->d0 = 0x10; break;
					default:  fatal("[ vr41xx kiu: unimpl"
					    "emented escape 0x%02 ]\n", ch);
					}
				} else {
					switch (ch) {
					case 'A': d->d0 = 0x1000; break;
					case 'B': d->d0 = 0x2000; break;
					case 'C': d->d0 = 0x20; break;
					case 'D': d->d0 = 0x10; break;
					default:  fatal("[ vr41xx kiu: unimpl"
					    "emented escape 0x%02 ]\n", ch);
					}
				}
				d->escape_state = 0;
			}
		} else switch (ch) {
		case '+':	console_makeavail(d->kiu_console_handle, '=');
				d->d5 = 0x800; break;
		case '_':	console_makeavail(d->kiu_console_handle, '-');
				d->d5 = 0x800; break;
		case '<':	console_makeavail(d->kiu_console_handle, ',');
				d->d5 = 0x800; break;
		case '>':	console_makeavail(d->kiu_console_handle, '.');
				d->d5 = 0x800; break;
		case '{':	console_makeavail(d->kiu_console_handle, '[');
				d->d5 = 0x800; break;
		case '}':	console_makeavail(d->kiu_console_handle, ']');
				d->d5 = 0x800; break;
		case ':':	console_makeavail(d->kiu_console_handle, ';');
				d->d5 = 0x800; break;
		case '"':	console_makeavail(d->kiu_console_handle, '\'');
				d->d5 = 0x800; break;
		case '|':	console_makeavail(d->kiu_console_handle, '\\');
				d->d5 = 0x800; break;
		case '?':	console_makeavail(d->kiu_console_handle, '/');
				d->d5 = 0x800; break;

		case '!':	console_makeavail(d->kiu_console_handle, '1');
				d->d5 = 0x800; break;
		case '@':	console_makeavail(d->kiu_console_handle, '2');
				d->d5 = 0x800; break;
		case '#':	console_makeavail(d->kiu_console_handle, '3');
				d->d5 = 0x800; break;
		case '$':	console_makeavail(d->kiu_console_handle, '4');
				d->d5 = 0x800; break;
		case '%':	console_makeavail(d->kiu_console_handle, '5');
				d->d5 = 0x800; break;
		case '^':	console_makeavail(d->kiu_console_handle, '6');
				d->d5 = 0x800; break;
		case '&':	console_makeavail(d->kiu_console_handle, '7');
				d->d5 = 0x800; break;
		case '*':	console_makeavail(d->kiu_console_handle, '8');
				d->d5 = 0x800; break;
		case '(':	console_makeavail(d->kiu_console_handle, '9');
				d->d5 = 0x800; break;
		case ')':	console_makeavail(d->kiu_console_handle, '0');
				d->d5 = 0x800; break;

		case '1':	d->d3 = 0x80; break;
		case '2':	d->d3 = 0x40; break;
		case '3':	d->d3 = 0x20; break;
		case '4':	d->d3 = 0x10; break;
		case '5':	d->d2 = 0x08; break;
		case '6':	d->d2 = 0x04; break;
		case '7':	d->d2 = 0x02; break;
		case '8':	d->d2 = 0x01; break;
		case '9':	d->d2 = 0x8000; break;
		case '0':	d->d2 = 0x4000; break;

		case ';':	d->d0 = 0x800; break;
		case '\'':	d->d0 = 0x400; break;
		case '[':	d->d0 = 0x200; break;
		case '/':	d->d0 = 0x8; break;
		case '\\':	d->d0 = 0x4; break;
		case ']':	d->d0 = 0x2; break;

		case 'a':	d->d1 = 0x8000; break;
		case 'b':	d->d2 = 0x800; break;
		case 'c':	d->d1 = 0x20; break;
		case 'd':	d->d1 = 0x2000; break;
		case 'e':	d->d2 = 0x20; break;
		case 'f':	d->d1 = 0x1000; break;
		case 'g':	d->d3 = 0x8; break;
		case 'h':	d->d3 = 0x4; break;
		case 'i':	d->d3 = 0x100; break;
		case 'j':	d->d3 = 0x2; break;
		case 'k':	d->d3 = 0x1; break;
		case 'l':	d->d0 = 0x80; break;
		case 'm':	d->d2 = 0x200; break;
		case 'n':	d->d2 = 0x400; break;
		case 'o':	d->d0 = 0x8000; break;
		case 'p':	d->d4 = 0x20; break;
		case 'q':	d->d2 = 0x80; break;
		case 'r':	d->d2 = 0x10; break;
		case 's':	d->d1 = 0x4000; break;
		case 't':	d->d3 = 0x800; break;
		case 'u':	d->d3 = 0x200; break;
		case 'v':	d->d1 = 0x10; break;
		case 'w':	d->d2 = 0x40; break;
		case 'x':	d->d1 = 0x40; break;
		case 'y':	d->d3 = 0x400; break;
		case 'z':	d->d1 = 0x80; break;

		case ',':	d->d2 = 0x100; break;
		case '.':	d->d0 = 0x4000; break;
		case '-':	d->d1 = 0x400; break;
		case '=':	d->d1 = 0x200; break;

		case '\r':
		case '\n':	d->d0 = 0x40; break;
		case ' ':	d->d0 = 0x01; break;
		case '\b':	d->d4 = 0x10; break;

		case 27:	d->escape_state = 1; break;

		default:
			/*  Shifted:  */
			if (ch >= 'A' && ch <= 'Z') {
				console_makeavail(d->kiu_console_handle,
				    ch + 32);
				d->d5 = 0x800;
				d->dont_clear_next = 1;
				break;
			}

			/*  CTRLed:  */
			if (ch >= 1 && ch <= 26) {
				console_makeavail(d->kiu_console_handle,
				    ch + 96);
				d->d5 = 0x4;
				d->dont_clear_next = 1;
				break;
			}
		}

		if (d->escape_state == 0)
			keychange = 1;
	}

	if (keychange) {
		/*  4=lost data, 2=data complete, 1=key input detected  */
		d->kiu_int_assert |= 3;
		recalc_kiu_int_assert(cpu, d);
	}
}


/*
 *  timer_tick():
 */
static void timer_tick(struct timer *timer, void *extra)
{
	struct vr41xx_data *d = (struct vr41xx_data *) extra;
	d->pending_timer_interrupts ++;
}


DEVICE_TICK(vr41xx)
{
	struct vr41xx_data *d = (struct vr41xx_data *) extra;

	if (d->pending_timer_interrupts > 0)
		INTERRUPT_ASSERT(d->timer_irq);

	if (cpu->machine->x11_md.in_use)
		vr41xx_keytick(cpu, d);
}


/*
 *  vr41xx_kiu():
 *
 *  Keyboard Interface Unit. Return value is "odata".
 *  (See NetBSD's vrkiu.c for more info.)
 */
static uint64_t vr41xx_kiu(struct cpu *cpu, int ofs, uint64_t idata,
	int writeflag, struct vr41xx_data *d)
{
	uint64_t odata = 0;

	switch (ofs) {
	case KIUDAT0:
		odata = d->d0; break;
	case KIUDAT1:
		odata = d->d1; break;
	case KIUDAT2:
		odata = d->d2; break;
	case KIUDAT3:
		odata = d->d3; break;
	case KIUDAT4:
		odata = d->d4; break;
	case KIUDAT5:
		odata = d->d5; break;
	case KIUSCANREP:
		if (writeflag == MEM_WRITE) {
			debug("[ vr41xx KIU: setting KIUSCANREP to 0x%04x ]\n",
			    (int)idata);
			/*  TODO  */
		} else
			fatal("[ vr41xx KIU: unimplemented read from "
			    "KIUSCANREP ]\n");
		break;
	case KIUSCANS:
		if (writeflag == MEM_WRITE) {
			debug("[ vr41xx KIU: write to KIUSCANS: 0x%04x: TODO"
			    " ]\n", (int)idata);
			/*  TODO  */
		} else
			debug("[ vr41xx KIU: unimplemented read from "
			    "KIUSCANS ]\n");
		break;
	case KIUINT:
		/*  Interrupt. A wild guess: zero-on-write  */
		if (writeflag == MEM_WRITE) {
			d->kiu_int_assert &= ~idata;
		} else {
			odata = d->kiu_int_assert;
		}
		recalc_kiu_int_assert(cpu, d);
		break;
	case KIURST:
		/*  Reset.  */
		break;
	default:
		if (writeflag == MEM_WRITE)
			debug("[ vr41xx KIU: unimplemented write to offset "
			    "0x%x, data=0x%016"PRIx64" ]\n", ofs,
			    (uint64_t) idata);
		else
			debug("[ vr41xx KIU: unimplemented read from offset "
			    "0x%x ]\n", ofs);
	}

	return odata;
}


DEVICE_ACCESS(vr41xx)
{
	struct vr41xx_data *d = (struct vr41xx_data *) extra;
	uint64_t idata = 0, odata = 0;
	int regnr;
	int revision = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	regnr = relative_addr / sizeof(uint64_t);

	/*  KIU ("Keyboard Interface Unit") is handled separately.  */
	if (relative_addr >= d->kiu_offset &&
	    relative_addr < d->kiu_offset + 0x20) {
		odata = vr41xx_kiu(cpu, relative_addr - d->kiu_offset,
		    idata, writeflag, d);
		goto ret;
	}

	/*  TODO: Maybe these should be handled separately as well?  */

	switch (relative_addr) {

	/*  BCU:  0x00 .. 0x1c  */
	case BCUREVID_REG_W:	/*  0x010  */
	case BCU81REVID_REG_W:	/*  0x014  */
		/*
		 *  TODO?  Linux seems to read 0x14. The lowest bits are
		 *  a divisor for PClock, bits 8 and up seem to be a
		 *  divisor for VTClock (relative to PClock?)...
		 */
		switch (d->cpumodel) {
		case 4131:	revision = BCUREVID_RID_4131; break;
		case 4122:	revision = BCUREVID_RID_4122; break;
		case 4121:	revision = BCUREVID_RID_4121; break;
		case 4111:	revision = BCUREVID_RID_4111; break;
		case 4102:	revision = BCUREVID_RID_4102; break;
		case 4101:	revision = BCUREVID_RID_4101; break;
		case 4181:	revision = BCUREVID_RID_4181; break;
		}
		odata = (revision << BCUREVID_RIDSHFT) | 0x020c;
		break;
	case BCU81CLKSPEED_REG_W:	/*  0x018  */
		/*
		 *  TODO: Implement this for ALL cpu types:
		 */
		odata = BCUCLKSPEED_DIVT4 << BCUCLKSPEED_DIVTSHFT;
		break;

	/*  DMAAU:  0x20 .. 0x3c  */

	/*  DCU:  0x40 .. 0x5c  */

	/*  CMU:  0x60 .. 0x7c  */

	/*  ICU:  0x80 .. 0xbc  */
	case 0x80:	/*  Level 1 system interrupt reg 1...  */
		if (writeflag == MEM_READ)
			odata = d->sysint1;
		else {
			/*  TODO: clear-on-write-one?  */
			d->sysint1 &= ~idata;
			d->sysint1 &= 0xffff;
		}
		break;
	case 0x88:
		if (writeflag == MEM_READ)
			odata = d->giuint;
		else
			d->giuint &= ~idata;
		break;
	case 0x8c:
		if (writeflag == MEM_READ)
			odata = d->msysint1;
		else
			d->msysint1 = idata;
		break;
	case 0x94:
		if (writeflag == MEM_READ)
			odata = d->giumask;
		else
			d->giumask = idata;
		break;
	case 0xa0:	/*  Level 1 system interrupt reg 2...  */
		if (writeflag == MEM_READ)
			odata = d->sysint2;
		else {
			/*  TODO: clear-on-write-one?  */
			d->sysint2 &= ~idata;
			d->sysint2 &= 0xffff;
		}
		break;
	case 0xa6:
		if (writeflag == MEM_READ)
			odata = d->msysint2;
		else
			d->msysint2 = idata;
		break;

	/*  RTC:  */
	case 0xc0:
	case 0xc2:
	case 0xc4:
		{
			struct timeval tv;
			gettimeofday(&tv, NULL);
			/*  Adjust time by 120 years and 29 days.  */
			tv.tv_sec += (int64_t) (120*365 + 29) * 24*60*60;

			switch (relative_addr) {
			case 0xc0:
				odata = (tv.tv_sec & 1) << 15;
				odata += (uint64_t)tv.tv_usec * 32768 / 1000000;
				break;
			case 0xc2:
				odata = (tv.tv_sec >> 1) & 0xffff;
				break;
			case 0xc4:
				odata = (tv.tv_sec >> 17) & 0xffff;
				break;
			}
		}
		break;

	case 0xd0:	/*  RTCL1_L_REG_W  */
		if (writeflag == MEM_WRITE && idata != 0) {
			int hz = RTCL1_L_HZ / idata;
			debug("[ vr41xx: rtc interrupts at %i Hz ]\n", hz);
			if (d->timer == NULL)
				d->timer = timer_add(hz, timer_tick, d);
			else
				timer_update_frequency(d->timer, hz);
		}
		break;
	case 0xd2:	/*  RTCL1_H_REG_W  */
		break;

	case 0x108:
		if (writeflag == MEM_READ)
			odata = d->giuint;
		else
			d->giuint &= ~idata;
		break;
	/*  case 0x10a:
		"High" part of GIU?
		break;
	*/

	case 0x13e:	/*  on 4181?  */
	case 0x1de:	/*  on 4121?  */
		/*  RTC interrupt register...  */
		/*  Ack. timer interrupts?  */
		INTERRUPT_DEASSERT(d->timer_irq);
		if (d->pending_timer_interrupts > 0)
			d->pending_timer_interrupts --;
		break;

	default:
		if (writeflag == MEM_WRITE)
			debug("[ vr41xx: unimplemented write to address "
			    "0x%"PRIx64", data=0x%016"PRIx64" ]\n",
			    (uint64_t) relative_addr, (uint64_t) idata);
		else
			debug("[ vr41xx: unimplemented read from address "
			    "0x%"PRIx64" ]\n", (uint64_t) relative_addr);
	}

ret:
	/*
	 *  Recalculate interrupt assertions:
	 */
	if (d->giuint & d->giumask)
                INTERRUPT_ASSERT(d->giu_irq);
	else
                INTERRUPT_DEASSERT(d->giu_irq);
	if ((d->sysint1 & d->msysint1) | (d->sysint2 & d->msysint2))
                INTERRUPT_ASSERT(d->cpu_irq);
	else
                INTERRUPT_DEASSERT(d->cpu_irq);

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


/*
 *  dev_vr41xx_init():
 *
 *  machine->path is something like "machine[0]".
 */
struct vr41xx_data *dev_vr41xx_init(struct machine *machine,
	struct memory *mem, int cpumodel)
{
	struct vr41xx_data *d;
	uint64_t baseaddr = 0;
	char tmps[300];
	int i;

	CHECK_ALLOCATION(d = (struct vr41xx_data *) malloc(sizeof(struct vr41xx_data)));
	memset(d, 0, sizeof(struct vr41xx_data));

	/*  Connect to MIPS irq 2:  */
	snprintf(tmps, sizeof(tmps), "%s.cpu[%i].2",
	    machine->path, machine->bootstrap_cpu);
	INTERRUPT_CONNECT(tmps, d->cpu_irq);

	/*
	 *  Register VRIP interrupt lines 0..25:
	 */
	for (i=0; i<=25; i++) {
		struct interrupt templ;
		snprintf(tmps, sizeof(tmps), "%s.cpu[%i].vrip.%i",
		    machine->path, machine->bootstrap_cpu, i);
		memset(&templ, 0, sizeof(templ));
		templ.line = i;
		templ.name = tmps;
		templ.extra = d;
		templ.interrupt_assert = vr41xx_vrip_interrupt_assert;
		templ.interrupt_deassert = vr41xx_vrip_interrupt_deassert;
		interrupt_handler_register(&templ);
	}

	/*
	 *  Register GIU interrupt lines 0..31:
	 */
	for (i=0; i<32; i++) {
		struct interrupt templ;
		snprintf(tmps, sizeof(tmps), "%s.cpu[%i].vrip.%i.giu.%i",
		    machine->path, machine->bootstrap_cpu, VRIP_INTR_GIU, i);
		memset(&templ, 0, sizeof(templ));
		templ.line = i;
		templ.name = tmps;
		templ.extra = d;
		templ.interrupt_assert = vr41xx_giu_interrupt_assert;
		templ.interrupt_deassert = vr41xx_giu_interrupt_deassert;
		interrupt_handler_register(&templ);
	}

	d->cpumodel = cpumodel;

	/*  TODO: VRC4173 has the KIU at offset 0x100?  */
	d->kiu_offset = 0x180;
	d->kiu_console_handle = -1;
	if (machine->x11_md.in_use)
		d->kiu_console_handle = console_start_slave_inputonly(
		    machine, "kiu", 1);

	/*  Connect to the KIU and GIU interrupts:  */
	snprintf(tmps, sizeof(tmps), "%s.cpu[%i].vrip.%i",
	    machine->path, machine->bootstrap_cpu, VRIP_INTR_GIU);
	INTERRUPT_CONNECT(tmps, d->giu_irq);
	snprintf(tmps, sizeof(tmps), "%s.cpu[%i].vrip.%i",
	    machine->path, machine->bootstrap_cpu, VRIP_INTR_KIU);
	INTERRUPT_CONNECT(tmps, d->kiu_irq);

	if (machine->x11_md.in_use)
		machine->main_console_handle = d->kiu_console_handle;  

	switch (cpumodel) {
	case 4101:
	case 4102:
	case 4111:
	case 4121:
		baseaddr = 0xb000000;
		break;
	case 4181:
		baseaddr = 0xa000000;
		dev_ram_init(machine, 0xb000000, 0x1000000, DEV_RAM_MIRROR,
		    0xa000000);
		break;
	case 4122:
	case 4131:
		baseaddr = 0xf000000;
		break;
	default:
		printf("Unimplemented VR cpu model\n");
		exit(1);
	}

	if (d->cpumodel == 4121 || d->cpumodel == 4181)
		snprintf(tmps, sizeof(tmps), "%s.cpu[%i].3",
		    machine->path, machine->bootstrap_cpu);
	else
		snprintf(tmps, sizeof(tmps), "%s.cpu[%i].vrip.%i",
		    machine->path, machine->bootstrap_cpu, VRIP_INTR_ETIMER);
	INTERRUPT_CONNECT(tmps, d->timer_irq);

	memory_device_register(mem, "vr41xx", baseaddr, DEV_VR41XX_LENGTH,
	    dev_vr41xx_access, (void *)d, DM_DEFAULT, NULL);

	/*
	 *  TODO: Find out which controllers are at which addresses on
	 *  which chips.
	 */
	if (cpumodel == 4131) {
		snprintf(tmps, sizeof(tmps), "ns16550 irq=%s.cpu[%i].vrip.%i "
		    "addr=0x%"PRIx64" name2=siu", machine->path,
		    machine->bootstrap_cpu, VRIP_INTR_SIU,
		    (uint64_t) (baseaddr+0x800));
		device_add(machine, tmps);
	} else {
		/*  This is used by Linux and NetBSD:  */
		snprintf(tmps, sizeof(tmps), "ns16550 irq=%s.cpu[%i]."
		    "vrip.%i addr=0x%x name2=serial", machine->path,
		    machine->bootstrap_cpu, VRIP_INTR_SIU, 0xc000000);
		device_add(machine, tmps);
	}

	/*  Hm... maybe this should not be here.  TODO  */
	snprintf(tmps, sizeof(tmps), "pcic irq=%s.cpu[%i].vrip.%i addr="
	    "0x140003e0", machine->path, machine->bootstrap_cpu,
	    VRIP_INTR_GIU);
	device_add(machine, tmps);

	machine_add_tickfunction(machine, dev_vr41xx_tick, d,
	    DEV_VR41XX_TICKSHIFT);

	/*  Some machines (?) use ISA space at 0x15000000 instead of
	    0x14000000, eg IBM WorkPad Z50.  */
	dev_ram_init(machine, 0x15000000, 0x1000000, DEV_RAM_MIRROR,
	    0x14000000);

	return d;
}

