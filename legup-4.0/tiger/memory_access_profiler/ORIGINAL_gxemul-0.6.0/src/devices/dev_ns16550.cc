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
 *  COMMENT: NS16550 serial controller
 *
 *  TODO: Implement the FIFO.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "console.h"
#include "cpu.h"
#include "device.h"
#include "interrupt.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/comreg.h"


/*  #define debug fatal  */

#define	TICK_SHIFT		14
#define	DEV_NS16550_LENGTH	8

struct ns_data {
	int		addrmult;
	int		in_use;
	const char	*name;
	int		console_handle;
	int		enable_fifo;

	struct interrupt irq;

	unsigned char	reg[DEV_NS16550_LENGTH];
	unsigned char	fcr;		/*  FIFO control register  */
	int		int_asserted;
	int		dlab;		/*  Divisor Latch Access bit  */
	int		divisor;

	int		databits;
	char		parity;
	const char	*stopbits;
};


DEVICE_TICK(ns16550)
{
	/*
	 *  This function is called at regular intervals. An interrupt is
	 *  asserted if there is a character available for reading, or if the
	 *  transmitter slot is empty (i.e. the ns16550 is ready to transmit).
	 */
	struct ns_data *d = (struct ns_data *) extra;

	d->reg[com_iir] &= ~IIR_RXRDY;
	if (console_charavail(d->console_handle))
		d->reg[com_iir] |= IIR_RXRDY;

	/*
	 *  If interrupts are enabled, and interrupts are pending, then
	 *  cause a CPU interrupt.
 	 */

	if (((d->reg[com_ier] & IER_ETXRDY) && (d->reg[com_iir] & IIR_TXRDY)) ||
	    ((d->reg[com_ier] & IER_ERXRDY) && (d->reg[com_iir] & IIR_RXRDY))) {
		d->reg[com_iir] &= ~IIR_NOPEND;
		if (d->reg[com_mcr] & MCR_IENABLE) {
			INTERRUPT_ASSERT(d->irq);
			d->int_asserted = 1;
		}
	} else {
		d->reg[com_iir] |= IIR_NOPEND;
		if (d->int_asserted)
			INTERRUPT_DEASSERT(d->irq);
		d->int_asserted = 0;
	}
}


DEVICE_ACCESS(ns16550)
{
	uint64_t idata = 0, odata=0;
	size_t i;
	struct ns_data *d = (struct ns_data *) extra;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

#if 0
	/*  The NS16550 should be accessed using byte read/writes:  */
	if (len != 1)
		fatal("[ ns16550 (%s): len=%i, idata=0x%16llx! ]\n",
		    d->name, len, (long long)idata);
#endif

	/*
	 *  Always ready to transmit:
	 */
	d->reg[com_lsr] |= LSR_TXRDY | LSR_TSRE;
	d->reg[com_msr] |= MSR_DCD | MSR_DSR | MSR_CTS;

	d->reg[com_iir] &= ~0xf0;
	if (d->enable_fifo)
		d->reg[com_iir] |= ((d->fcr << 5) & 0xc0);

	d->reg[com_lsr] &= ~LSR_RXRDY;
	if (console_charavail(d->console_handle))
		d->reg[com_lsr] |= LSR_RXRDY;

	relative_addr /= d->addrmult;

	if (relative_addr >= DEV_NS16550_LENGTH) {
		fatal("[ ns16550 (%s): outside register space? relative_addr="
		    "0x%llx. bad addrmult? bad device length? ]\n", d->name,
		    (long long)relative_addr);
		return 0;
	}

	switch (relative_addr) {

	case com_data:	/*  data AND low byte of the divisor  */
		/*  Read/write of the Divisor value:  */
		if (d->dlab) {
			/*  Write or read the low byte of the divisor:  */
			if (writeflag == MEM_WRITE)
				d->divisor = (d->divisor & 0xff00) | idata;
			else
				odata = d->divisor & 0xff;
			break;
		}

		/*  Read/write of data:  */
		if (writeflag == MEM_WRITE) {
			if (d->reg[com_mcr] & MCR_LOOPBACK)
				console_makeavail(d->console_handle, idata);
			else
				console_putchar(d->console_handle, idata);
			d->reg[com_iir] |= IIR_TXRDY;
		} else {
			int x = console_readchar(d->console_handle);
			odata = x < 0? 0 : x;
		}
		dev_ns16550_tick(cpu, d);
		break;

	case com_ier:	/*  interrupt enable AND high byte of the divisor  */
		/*  Read/write of the Divisor value:  */
		if (d->dlab) {
			if (writeflag == MEM_WRITE) {
				/*  Set the high byte of the divisor:  */
				d->divisor = (d->divisor & 0xff) | (idata << 8);
				debug("[ ns16550 (%s): speed set to %i bps ]\n",
				    d->name, (int)(115200 / d->divisor));
			} else
				odata = d->divisor >> 8;
			break;
		}

		/*  IER:  */
		if (writeflag == MEM_WRITE) {
			/*  This is to supress Linux' behaviour  */
			if (idata != 0)
				debug("[ ns16550 (%s): write to ier: 0x%02x ]"
				    "\n", d->name, (int)idata);

			/*  Needed for NetBSD 2.0.x, but not 1.6.2?  */
			if (!(d->reg[com_ier] & IER_ETXRDY)
			    && (idata & IER_ETXRDY))
				d->reg[com_iir] |= IIR_TXRDY;

			d->reg[com_ier] = idata;
			dev_ns16550_tick(cpu, d);
		} else
			odata = d->reg[com_ier];
		break;

	case com_iir:	/*  interrupt identification (r), fifo control (w)  */
		if (writeflag == MEM_WRITE) {
			debug("[ ns16550 (%s): write to fifo control: 0x%02x ]"
			    "\n", d->name, (int)idata);
			d->fcr = idata;
		} else {
			odata = d->reg[com_iir];
			if (d->reg[com_iir] & IIR_TXRDY)
				d->reg[com_iir] &= ~IIR_TXRDY;
			debug("[ ns16550 (%s): read from iir: 0x%02x ]\n",
			    d->name, (int)odata);
			dev_ns16550_tick(cpu, d);
		}
		break;

	case com_lsr:
		if (writeflag == MEM_WRITE) {
			debug("[ ns16550 (%s): write to lsr: 0x%02x ]\n",
			    d->name, (int)idata);
			d->reg[com_lsr] = idata;
		} else {
			odata = d->reg[com_lsr];
			/*  debug("[ ns16550 (%s): read from lsr: 0x%02x ]\n",
			    d->name, (int)odata);  */
		}
		break;

	case com_msr:
		if (writeflag == MEM_WRITE) {
			debug("[ ns16550 (%s): write to msr: 0x%02x ]\n",
			    d->name, (int)idata);
			d->reg[com_msr] = idata;
		} else {
			odata = d->reg[com_msr];
			debug("[ ns16550 (%s): read from msr: 0x%02x ]\n",
			    d->name, (int)odata);
		}
		break;

	case com_lctl:
		if (writeflag == MEM_WRITE) {
			d->reg[com_lctl] = idata;
			switch (idata & 0x7) {
			case 0:	d->databits = 5; d->stopbits = "1"; break;
			case 1:	d->databits = 6; d->stopbits = "1"; break;
			case 2:	d->databits = 7; d->stopbits = "1"; break;
			case 3:	d->databits = 8; d->stopbits = "1"; break;
			case 4:	d->databits = 5; d->stopbits = "1.5"; break;
			case 5:	d->databits = 6; d->stopbits = "2"; break;
			case 6:	d->databits = 7; d->stopbits = "2"; break;
			case 7:	d->databits = 8; d->stopbits = "2"; break;
			}
			switch ((idata & 0x38) / 0x8) {
			case 0:	d->parity = 'N'; break;		/*  none  */
			case 1:	d->parity = 'O'; break;		/*  odd  */
			case 2:	d->parity = '?'; break;
			case 3:	d->parity = 'E'; break;		/*  even  */
			case 4:	d->parity = '?'; break;
			case 5:	d->parity = 'Z'; break;		/*  zero  */
			case 6:	d->parity = '?'; break;
			case 7:	d->parity = 'o'; break;		/*  one  */
			}

			d->dlab = idata & 0x80? 1 : 0;

			debug("[ ns16550 (%s): write to lctl: 0x%02x (%s%s"
			    "setting mode %i%c%s) ]\n", d->name, (int)idata,
			    d->dlab? "Divisor Latch access, " : "",
			    idata&0x40? "sending BREAK, " : "",
			    d->databits, d->parity, d->stopbits);
		} else {
			odata = d->reg[com_lctl];
			debug("[ ns16550 (%s): read from lctl: 0x%02x ]\n",
			    d->name, (int)odata);
		}
		break;

	case com_mcr:
		if (writeflag == MEM_WRITE) {
			d->reg[com_mcr] = idata;
			debug("[ ns16550 (%s): write to mcr: 0x%02x ]\n",
			    d->name, (int)idata);
			if (!(d->reg[com_iir] & IIR_TXRDY)
			    && (idata & MCR_IENABLE))
				d->reg[com_iir] |= IIR_TXRDY;
			dev_ns16550_tick(cpu, d);
		} else {
			odata = d->reg[com_mcr];
			debug("[ ns16550 (%s): read from mcr: 0x%02x ]\n",
			    d->name, (int)odata);
		}
		break;

	default:
		if (writeflag==MEM_READ) {
			debug("[ ns16550 (%s): read from reg %i ]\n",
			    d->name, (int)relative_addr);
			odata = d->reg[relative_addr];
		} else {
			debug("[ ns16550 (%s): write to reg %i:",
			    d->name, (int)relative_addr);
			for (i=0; i<len; i++)
				debug(" %02x", data[i]);
			debug(" ]\n");
			d->reg[relative_addr] = idata;
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(ns16550)
{
	struct ns_data *d;
	size_t nlen;
	char *name;

	CHECK_ALLOCATION(d = (struct ns_data *) malloc(sizeof(struct ns_data)));
	memset(d, 0, sizeof(struct ns_data));

	d->addrmult	= devinit->addr_mult;
	d->in_use	= devinit->in_use;
	d->enable_fifo	= 1;
	d->dlab		= 0;
	d->divisor	= 115200 / 9600;
	d->databits	= 8;
	d->parity	= 'N';
	d->stopbits	= "1";
	d->name		= devinit->name2 != NULL? devinit->name2 : "";
	d->console_handle =
	    console_start_slave(devinit->machine, devinit->name2 != NULL?
	    devinit->name2 : devinit->name, d->in_use);

	INTERRUPT_CONNECT(devinit->interrupt_path, d->irq);

	nlen = strlen(devinit->name) + 10;
	if (devinit->name2 != NULL)
		nlen += strlen(devinit->name2);
	CHECK_ALLOCATION(name = (char *) malloc(nlen));
	if (devinit->name2 != NULL && devinit->name2[0])
		snprintf(name, nlen, "%s [%s]", devinit->name, devinit->name2);
	else
		snprintf(name, nlen, "%s", devinit->name);

	memory_device_register(devinit->machine->memory, name, devinit->addr,
	    DEV_NS16550_LENGTH * d->addrmult, dev_ns16550_access, d,
	    DM_DEFAULT, NULL);
	machine_add_tickfunction(devinit->machine,
	    dev_ns16550_tick, d, TICK_SHIFT);

	/*
	 *  NOTE:  Ugly cast into a pointer, because this is a convenient way
	 *         to return the console handle to code in src/machines/.
	 */
	devinit->return_ptr = (void *)(size_t)d->console_handle;

	return 1;
}

