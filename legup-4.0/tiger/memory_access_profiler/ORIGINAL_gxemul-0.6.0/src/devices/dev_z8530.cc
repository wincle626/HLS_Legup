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
 *  COMMENT: Zilog Z8530 "zs" serial controller
 *
 *  Features:
 *	o)  Two channels, 0 = "channel B", 1 = "channel A".
 *	    Normally, only channel B is in use.
 *
 *  This is a work in progress... TODOs include:
 *	o)  Implement more of the register set.
 *	o)  Verify that it works with other guest OSes than NetBSD and OpenBSD.
 *	o)  Implement DMA!
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

#include "thirdparty/z8530reg.h"


/*  #define debug fatal  */

#define	ZS_TICK_SHIFT		14
#define	ZS_N_REGS		16
#define	ZS_N_CHANNELS		2
#define	DEV_Z8530_LENGTH	4

struct z8530_data {
	struct interrupt irq;
	int		irq_asserted;
	int		addr_mult;

	int		console_handle[ZS_N_CHANNELS];
	int		reg_select[ZS_N_CHANNELS];
	uint8_t		rr[ZS_N_CHANNELS][ZS_N_REGS];
	uint8_t		wr[ZS_N_CHANNELS][ZS_N_REGS];
};


/*
 *  check_incoming():
 *
 *  Sets the RX interrupt flag for ports B and A, if something there is input
 *  available on port 0 or 1, respectively.
 */
static void check_incoming(struct cpu *cpu, struct z8530_data *d)
{
	if (console_charavail(d->console_handle[0])) {
		d->rr[1][3] |= ZSRR3_IP_B_RX;
		d->rr[0][0] |= ZSRR0_RX_READY;
	}
	if (console_charavail(d->console_handle[1])) {
		d->rr[1][3] |= ZSRR3_IP_A_RX;
		d->rr[1][0] |= ZSRR0_RX_READY;
	}
}


DEVICE_TICK(z8530)
{
	/*  Generate transmit and receive interrupts at regular intervals.  */
	struct z8530_data *d = (struct z8530_data *) extra;
	int asserted = 0;

	if (d->rr[1][3] & ZSRR3_IP_B_TX && d->wr[0][1] & ZSWR1_TIE)
		asserted = 1;
	if (d->rr[1][3] & ZSRR3_IP_A_TX && d->wr[1][1] & ZSWR1_TIE)
		asserted = 1;

	d->rr[1][3] &= ~(ZSRR3_IP_B_RX | ZSRR3_IP_A_RX);
	if (!asserted)
		check_incoming(cpu, d);

	if (d->rr[1][3] & ZSRR3_IP_B_RX && (d->wr[0][1]&0x18) != ZSWR1_RIE_NONE)
		asserted = 1;
	if (d->rr[1][3] & ZSRR3_IP_A_RX && (d->wr[1][1]&0x18) != ZSWR1_RIE_NONE)
		asserted = 1;

	if (!(d->wr[1][9] & ZSWR9_MASTER_IE))
		asserted = 0;

	if (asserted)
		INTERRUPT_ASSERT(d->irq);

	if (d->irq_asserted && !asserted)
		INTERRUPT_DEASSERT(d->irq);

	d->irq_asserted = asserted;
}


DEVICE_ACCESS(z8530)
{
	struct z8530_data *d = (struct z8530_data *) extra;
	uint64_t idata = 0, odata = 0;
	int port_nr;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	/*  Both ports are always ready to transmit:  */
	d->rr[0][0] |= ZSRR0_TX_READY | ZSRR0_DCD | ZSRR0_CTS;
	d->rr[1][0] |= ZSRR0_TX_READY | ZSRR0_DCD | ZSRR0_CTS;

	relative_addr /= d->addr_mult;

	port_nr = (relative_addr / 2) % ZS_N_CHANNELS;
	relative_addr &= 1;

	if (relative_addr == 0) {
		/*  Register access:  */
		if (writeflag == MEM_READ) {
			odata = d->rr[port_nr][d->reg_select[port_nr]];
			debug("[ z8530: read from port %i reg %2i: "
			    "0x%02x ]\n", port_nr, d->reg_select[
			    port_nr], (int)odata);
			d->reg_select[port_nr] = 0;
		} else {
			if (d->reg_select[port_nr] == 0) {
				if (idata < 16)
					d->reg_select[port_nr] = idata & 15;
				else
					d->reg_select[port_nr] = idata & 7;
				switch (idata & 0xf8) {
				case ZSWR0_CLR_INTR:	/*  Interrupt ack:  */
					d->rr[1][3] = 0;
					break;
				}
			} else {
				d->wr[port_nr][d->reg_select[port_nr]] = idata;
				switch (d->reg_select[port_nr]) {
				default:debug("[ z8530: write to  port %i reg "
					    "%2i: 0x%02x ]\n", port_nr, d->
					    reg_select[port_nr], (int)idata);
				}
				d->reg_select[port_nr] = 0;
			}
		}
	} else {
		/*  Data access:  */
		if (writeflag == MEM_READ) {
			int x = console_readchar(d->console_handle[port_nr]);
			d->rr[port_nr][0] &= ~ZSRR0_RX_READY;
			odata = x < 0? 0 : x;
		} else {
			idata &= 255;
			if (idata != 0)
				console_putchar(d->console_handle[port_nr],
				    idata);
			if (1 /* d->wr[port_nr][1] & ZSWR1_TIE */) {
				if (port_nr == 0)
					d->rr[1][3] |= ZSRR3_IP_B_TX;
				else
					d->rr[1][3] |= ZSRR3_IP_A_TX;
			}
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	dev_z8530_tick(cpu, extra);

	return 1;
}


DEVINIT(z8530)
{
	struct z8530_data *d;
	char tmp[100];

	CHECK_ALLOCATION(d = (struct z8530_data *) malloc(sizeof(struct z8530_data)));
	memset(d, 0, sizeof(struct z8530_data));

	d->addr_mult  = devinit->addr_mult;

	INTERRUPT_CONNECT(devinit->interrupt_path, d->irq);

	snprintf(tmp, sizeof(tmp), "%s [ch-b]", devinit->name);
	d->console_handle[0] = console_start_slave(devinit->machine, tmp,
	    devinit->in_use);
	snprintf(tmp, sizeof(tmp), "%s [ch-a]", devinit->name);
	d->console_handle[1] = console_start_slave(devinit->machine, tmp, 0);

	if (devinit->name2 != NULL && devinit->name2[0])
		snprintf(tmp, sizeof(tmp), "%s [%s]", devinit->name,
		    devinit->name2);
	else
		snprintf(tmp, sizeof(tmp), "%s", devinit->name);

	memory_device_register(devinit->machine->memory, tmp, devinit->addr,
	    DEV_Z8530_LENGTH * d->addr_mult, dev_z8530_access, d, DM_DEFAULT,
	    NULL);

	machine_add_tickfunction(devinit->machine, dev_z8530_tick, d,
	    ZS_TICK_SHIFT);

	devinit->return_ptr = (void *)(size_t) d->console_handle[0];

	return 1;
}

