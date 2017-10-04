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
 *  COMMENT: System Support Chip serial controller
 *
 *  Serial controller on DECsystem 5400 and 5800.
 *  Known as System Support Chip on VAX 3600 (KA650).
 *
 *  Described around page 80 in the kn210tm1.pdf.
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


#define	RX_INT_ENABLE	0x40
#define	RX_AVAIL	0x80
#define	TX_INT_ENABLE	0x40
#define	TX_READY	0x80

#define	SSC_TICK_SHIFT	14

/*
 *  _TXRX is for debugging putchar/getchar. The other
 *  one is more general.
 */
/*  #define SSC_DEBUG_TXRX  */
#define SSC_DEBUG

struct ssc_data {
	int		console_handle;
	int		use_fb;

	int		rx_ctl;
	int		tx_ctl;

	struct interrupt irq;
};


DEVICE_TICK(ssc)
{
	struct ssc_data *d = (struct ssc_data *) extra;

	d->tx_ctl |= TX_READY;	/*  transmitter always ready  */

	d->rx_ctl &= ~RX_AVAIL;
	if (console_charavail(d->console_handle))
		d->rx_ctl |= RX_AVAIL;

	/*  rx interrupts enabled, and char avail?  */
	if (d->rx_ctl & RX_INT_ENABLE && d->rx_ctl & RX_AVAIL) {
		/*  TODO:  This is for 5800 only!  */
		unsigned char txvector = 0xf8;
		cpu->memory_rw(cpu, cpu->mem, 0x40000050, &txvector,
		    1, MEM_WRITE, NO_EXCEPTIONS | PHYSICAL);
		INTERRUPT_ASSERT(d->irq);
	}

	/*  tx interrupts enabled?  */
	if (d->tx_ctl & TX_INT_ENABLE) {
		/*  TODO:  This is for 5800 only!  */
		unsigned char txvector = 0xfc;
		cpu->memory_rw(cpu, cpu->mem, 0x40000050, &txvector,
		    1, MEM_WRITE, NO_EXCEPTIONS | PHYSICAL);
		INTERRUPT_ASSERT(d->irq);
	}
}


DEVICE_ACCESS(ssc)
{
	uint64_t idata = 0, odata = 0;
	struct ssc_data *d = (struct ssc_data *) extra;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	dev_ssc_tick(cpu, extra);

	switch (relative_addr) {
	case 0x0080:	/*  receive status  */
		if (writeflag==MEM_READ) {
			odata = d->rx_ctl;
#ifdef SSC_DEBUG_TXRX
			debug("[ ssc: read from 0x%08lx: 0x%02x ]\n",
			    (long)relative_addr, (int)odata);
#endif
		} else {
			d->rx_ctl = idata;

			INTERRUPT_DEASSERT(d->irq);

#ifdef SSC_DEBUG_TXRX
			debug("[ ssc: write to  0x%08lx: 0x%02x ]\n",
			    (long)relative_addr, (int)idata);
#endif
		}

		break;
	case 0x0084:	/*  receive data  */
		if (writeflag==MEM_READ) {
#ifdef SSC_DEBUG_TXRX
			debug("[ ssc: read from 0x%08lx ]\n",
			    (long)relative_addr);
#endif
			if (console_charavail(d->console_handle))
				odata = console_readchar(d->console_handle);
		} else {
#ifdef SSC_DEBUG_TXRX
			debug("[ ssc: write to 0x%08lx: 0x%02x ]\n",
			    (long)relative_addr, (int)idata);
#endif
		}

		break;
	case 0x0088:	/*  transmit status  */
		if (writeflag==MEM_READ) {
			odata = d->tx_ctl;
#ifdef SSC_DEBUG_TXRX
			debug("[ ssc: read from 0x%08lx: 0x%04x ]\n",
			    (long)relative_addr, (int)odata);
#endif
		} else {
			d->tx_ctl = idata;

			INTERRUPT_DEASSERT(d->irq);

#ifdef SSC_DEBUG_TXRX
			debug("[ ssc: write to  0x%08lx: 0x%02x ]\n",
			    (long)relative_addr, (int)idata);
#endif
		}

		break;
	case 0x008c:	/*  transmit data  */
		if (writeflag==MEM_READ) {
			debug("[ ssc: read from 0x%08lx ]\n",
			    (long)relative_addr);
		} else {
			/*  debug("[ ssc: write to 0x%08lx: 0x%02x ]\n",
			    (long)relative_addr, (int)idata);  */
			console_putchar(d->console_handle, idata);
		}

		break;
	case 0x0100:
		if (writeflag==MEM_READ) {
			odata = 128;
#ifdef SSC_DEBUG_TXRX
			debug("[ ssc: read from 0x%08lx: 0x%08lx ]\n",
			    (long)relative_addr, (long)odata);
#endif
		} else {
#ifdef SSC_DEBUG_TXRX
			debug("[ ssc: write to  0x%08lx: 0x%08x ]\n",
			    (long)relative_addr, idata);
#endif
		}

		break;
	case 0x0108:
		if (writeflag==MEM_READ) {
			debug("[ ssc: read from 0x%08lx ]\n",
			    (long)relative_addr);
		} else {
#ifdef SSC_DEBUG
			debug("[ ssc: write to  0x%08lx: 0x%08x ]\n",
			    (long)relative_addr, (int)idata);
#endif
		}

		break;
	default:
		if (writeflag==MEM_READ) {
			debug("[ ssc: read from 0x%08lx ]\n",
			    (long)relative_addr);
		} else {
			debug("[ ssc: write to  0x%08lx: 0x%08x ]\n",
			    (long)relative_addr, (int)idata);
		}
	}

	dev_ssc_tick(cpu, extra);

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


void dev_ssc_init(struct machine *machine, struct memory *mem,
	uint64_t baseaddr, const char *irq_path, int use_fb)
{
	struct ssc_data *d;

	CHECK_ALLOCATION(d = (struct ssc_data *) malloc(sizeof(struct ssc_data)));
	memset(d, 0, sizeof(struct ssc_data));

	d->use_fb = use_fb;
	d->console_handle = console_start_slave(machine, "SSC", 1);

	INTERRUPT_CONNECT(irq_path, d->irq);

	memory_device_register(mem, "ssc", baseaddr, DEV_SSC_LENGTH,
	    dev_ssc_access, d, DM_DEFAULT, NULL);

	machine_add_tickfunction(machine, dev_ssc_tick, d, SSC_TICK_SHIFT);
}

