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
 *  COMMENT: DC7085 serial controller, used in some DECstation models
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

#include "thirdparty/dc7085.h"


#define	DC_TICK_SHIFT		14

#define	MAX_QUEUE_LEN		32768

struct dc_data {
	struct dc7085regs	regs;

	int			console_handle;

	/*  For slow_serial_interrupts_hack_for_linux:  */
	int			just_transmitted_something;

	unsigned char		rx_queue_char[MAX_QUEUE_LEN];
	char			rx_queue_lineno[MAX_QUEUE_LEN];
	int			cur_rx_queue_pos_write;
	int			cur_rx_queue_pos_read;

	int			tx_scanner;

	struct interrupt	irq;
	int			use_fb;

	struct lk201_data	lk201;
};


/*
 *  Add a character to the receive queue.
 */
void add_to_rx_queue(void *e, int ch, int line_no)
{
	struct dc_data *d = (struct dc_data *) e;
	int entries_in_use = d->cur_rx_queue_pos_write -
	    d->cur_rx_queue_pos_read;

	while (entries_in_use < 0)
		entries_in_use += MAX_QUEUE_LEN;

	/*  Ignore mouse updates, if they come too often:  */
	if (entries_in_use > MAX_QUEUE_LEN/2 && line_no == DCMOUSE_PORT)
		return;

	d->rx_queue_char[d->cur_rx_queue_pos_write]   = ch;
	d->rx_queue_lineno[d->cur_rx_queue_pos_write] = line_no;
	d->cur_rx_queue_pos_write ++;
	if (d->cur_rx_queue_pos_write == MAX_QUEUE_LEN)
		d->cur_rx_queue_pos_write = 0;

	if (d->cur_rx_queue_pos_write == d->cur_rx_queue_pos_read)
		fatal("warning: add_to_rx_queue(): rx_queue overrun!\n");
}


DEVICE_TICK(dc7085)
{
	/*
	 *  If a key is available from the keyboard, add it to the rx queue.
	 *  If other bits are set, an interrupt might need to be caused.
	 */
	struct dc_data *d = (struct dc_data *) extra;
	int avail;

	if (cpu->machine->slow_serial_interrupts_hack_for_linux) {
		/*
		 *  Special hack to prevent Linux from Oopsing. (This makes
		 *  interrupts not come as fast as possible.)
		 */
		if (d->just_transmitted_something) {
			d->just_transmitted_something --;
			return;
		}
	}

	d->regs.dc_csr &= ~CSR_RDONE;

	if ((d->regs.dc_csr & CSR_MSE) && !(d->regs.dc_csr & CSR_TRDY)) {
		int scanner_start = d->tx_scanner;

		/*  Loop until we've checked all 4 channels, or some
			channel was ready to transmit:  */

		do {
			d->tx_scanner = (d->tx_scanner + 1) % 4;

			if (d->regs.dc_tcr & (1 << d->tx_scanner)) {
				d->regs.dc_csr |= CSR_TRDY;
				if (d->regs.dc_csr & CSR_TIE)
					INTERRUPT_ASSERT(d->irq);

				d->regs.dc_csr &= ~CSR_TX_LINE_NUM;
				d->regs.dc_csr |= (d->tx_scanner << 8);
			}
		} while (!(d->regs.dc_csr & CSR_TRDY) &&
		    d->tx_scanner != scanner_start);

		/*  We have to return here. NetBSD can handle both
		    rx and tx interrupts simultaneously, but Ultrix
		    doesn't like that?  */

		if (d->regs.dc_csr & CSR_TRDY)
			return;
	}

	lk201_tick(cpu->machine, &d->lk201);

	avail = d->cur_rx_queue_pos_write != d->cur_rx_queue_pos_read;

	if (avail && (d->regs.dc_csr & CSR_MSE))
		d->regs.dc_csr |= CSR_RDONE;

	if ((d->regs.dc_csr & CSR_RDONE) && (d->regs.dc_csr & CSR_RIE))
		INTERRUPT_ASSERT(d->irq);
}


DEVICE_ACCESS(dc7085)
{
	struct dc_data *d = (struct dc_data *) extra;
	uint64_t idata = 0, odata = 0;
	size_t i;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	/*  Always clear:  */
	d->regs.dc_csr &= ~CSR_CLR;

	switch (relative_addr) {

	case 0x00:	/*  CSR:  Control and Status  */
		if (writeflag == MEM_WRITE) {
			debug("[ dc7085 write to CSR: 0x%04x ]\n", idata);
			idata &= (CSR_TIE | CSR_RIE | CSR_MSE | CSR_CLR
			    | CSR_MAINT);
			d->regs.dc_csr &= ~(CSR_TIE | CSR_RIE | CSR_MSE
			    | CSR_CLR | CSR_MAINT);
			d->regs.dc_csr |= idata;
			if (!(d->regs.dc_csr & CSR_MSE))
				d->regs.dc_csr &= ~(CSR_TRDY | CSR_RDONE);
			goto do_return;
		} else {
			/*  read:  */

			/*  fatal("[ dc7085 read from CSR: (csr = 0x%04x) ]\n",
			    d->regs.dc_csr);  */
			odata = d->regs.dc_csr;
		}
		break;

	case 0x08:	/*  LPR:  */
		if (writeflag == MEM_WRITE) {
			debug("[ dc7085 write to LPR: 0x%04x ]\n", idata);
			d->regs.dc_rbuf_lpr = idata;
			goto do_return;
		} else {
			/*  read:  */
			int avail = d->cur_rx_queue_pos_write !=
			    d->cur_rx_queue_pos_read;
			int ch = 0, lineno = 0;
			/*  debug("[ dc7085 read from RBUF: ");  */
			if (avail) {
				ch = d->rx_queue_char[d->cur_rx_queue_pos_read];
				lineno = d->rx_queue_lineno[
				    d->cur_rx_queue_pos_read];
				d->cur_rx_queue_pos_read++;
				if (d->cur_rx_queue_pos_read == MAX_QUEUE_LEN)
					d->cur_rx_queue_pos_read = 0;
				/*  if (ch >= ' ' && ch < 127)
					debug("'%c'", ch);
				else
					debug("0x%x", ch);
				debug(" for lineno %i ", lineno);  */
			}  /*  else
				debug("empty ");
			debug("]\n");  */
			odata = (avail? RBUF_DVAL:0) |
			    (lineno << RBUF_LINE_NUM_SHIFT) | ch;

			d->regs.dc_csr &= ~CSR_RDONE;
			INTERRUPT_DEASSERT(d->irq);

			d->just_transmitted_something = 4;
		}
		break;

	case 0x10:	/*  TCR:  */
		if (writeflag == MEM_WRITE) {
			/*  fatal("[ dc7085 write to TCR: 0x%04x) ]\n",
			    (int)idata);  */
			d->regs.dc_tcr = idata;
			d->regs.dc_csr &= ~CSR_TRDY;
			INTERRUPT_DEASSERT(d->irq);
			goto do_return;
		} else {
			/*  read:  */
			/*  debug("[ dc7085 read from TCR: (tcr = 0x%04x) ]\n",
			    d->regs.dc_tcr);  */
			odata = d->regs.dc_tcr;
		}
		break;

	case 0x18:	/*  Modem status (R), transmit data (W)  */
		if (writeflag == MEM_WRITE) {
			int line_no = (d->regs.dc_csr >>
			    RBUF_LINE_NUM_SHIFT) & 0x3;
			idata &= 0xff;

			lk201_tx_data(&d->lk201, line_no, idata);

			d->regs.dc_csr &= ~CSR_TRDY;
			INTERRUPT_DEASSERT(d->irq);

			d->just_transmitted_something = 4;
		} else {
			/*  read:  */
			d->regs.dc_msr_tdr |= MSR_DSR2 | MSR_CD2 |
			    MSR_DSR3 | MSR_CD3;
			debug("[ dc7085 read from MSR: (msr_tdr = 0x%04x) ]\n",
			    d->regs.dc_msr_tdr);
			odata = d->regs.dc_msr_tdr;
		}
		break;

	default:
		if (writeflag==MEM_READ) {
			debug("[ dc7085 read from 0x%08lx ]\n",
			    (long)relative_addr);
		} else {
			debug("[ dc7085 write to 0x%08lx:",
			    (long)relative_addr);
			for (i=0; i<len; i++)
				debug(" %02x", data[i]);
			debug(" ]\n");
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

do_return:
	dev_dc7085_tick(cpu, extra);

	return 1;
}


/*
 *  dev_dc7085_init():
 *
 *  Initialize a dc7085 serial controller device. use_fb should be non-zero
 *  if a framebuffer device is used. Channel 0 will then be treated as a
 *  DECstation keyboard, instead of a plain serial console.
 */
int dev_dc7085_init(struct machine *machine, struct memory *mem,
	uint64_t baseaddr, char *irq_path, int use_fb)
{
	struct dc_data *d;

	CHECK_ALLOCATION(d = (struct dc_data *) malloc(sizeof(struct dc_data)));
	memset(d, 0, sizeof(struct dc_data));

	INTERRUPT_CONNECT(irq_path, d->irq);
	d->use_fb = use_fb;

	d->regs.dc_csr = CSR_TRDY | CSR_MSE;
	d->regs.dc_tcr = 0x00;

	d->console_handle = console_start_slave(machine, "DC7085", 1);

	lk201_init(&d->lk201, use_fb, add_to_rx_queue, d->console_handle, d);

	memory_device_register(mem, "dc7085", baseaddr, DEV_DC7085_LENGTH,
	    dev_dc7085_access, d, DM_DEFAULT, NULL);
	machine_add_tickfunction(machine, dev_dc7085_tick, d,
	    DC_TICK_SHIFT);

	return d->console_handle;
}

