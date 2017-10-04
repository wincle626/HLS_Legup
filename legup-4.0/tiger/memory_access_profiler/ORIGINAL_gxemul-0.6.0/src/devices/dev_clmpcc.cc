/*
 *  Copyright (C) 2007-2009  Anders Gavare.  All rights reserved.
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
 *  COMMENT: Cirrus Logic 4-Channel Communications Controller (CD2400/CD2401)
 *
 *  Used by OpenBSD/mvme88k.
 *
 *  Works so far:
 *	TX/RX interrupts (happy case).
 *
 *  TODO:
 *	Multiple channels
 *	DMA?
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "console.h"
#include "cpu.h"
#include "device.h"
#include "emul.h"
#include "interrupt.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/mvme_pcctworeg.h"
#include "thirdparty/clmpccreg.h"


#define debug fatal

#define	CLMPCC_LEN		0x200
#define	DEV_CLMPCC_TICK_SHIFT	16

struct clmpcc_data {
	unsigned char	reg[CLMPCC_LEN];

	int		console_handle;

	/*  Interrupt pins on the PCC2 controller:  */
	struct interrupt irq_scc_rxe;
	struct interrupt irq_scc_m;
	struct interrupt irq_scc_tx;
	struct interrupt irq_scc_rx;
};


static void reassert_interrupts(struct clmpcc_data *d)
{
	int rxintr = 0;

	if (console_charavail(d->console_handle))
		rxintr = 1;

	if (rxintr)
		INTERRUPT_ASSERT(d->irq_scc_rx);
	else
		INTERRUPT_DEASSERT(d->irq_scc_rx);

	/*  TODO: Hack/experiment for now...  */
	if ((d->reg[CLMPCC_REG_IER] & 3) != 0)
		INTERRUPT_ASSERT(d->irq_scc_tx);
	else
		INTERRUPT_DEASSERT(d->irq_scc_tx);
}


DEVICE_TICK(clmpcc)
{
	struct clmpcc_data *d = (struct clmpcc_data *) extra;
	reassert_interrupts(d);
}


DEVICE_ACCESS(clmpcc)
{
	struct clmpcc_data *d = (struct clmpcc_data *) extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE) {
		idata = memory_readmax64(cpu, data, len);
		d->reg[relative_addr] = idata;
	}

	if (writeflag == MEM_READ)
		odata = d->reg[relative_addr];

	switch (relative_addr) {

	case 0:	/*  Used by OpenBSD/mvme88k when probing...  */
		break;

	case CLMPCC_REG_TPR:	/*  Timer Period Register  */
		break;

	case CLMPCC_REG_CAR:	/*  Channel Access Register  */
		/*  debug("[ clmpcc: selecting channel %i ]\n",
		    (int) idata);  */
		break;

	case CLMPCC_REG_CCR:	/*  Channel Command Register:  */
		odata = 0;
		break;

	case CLMPCC_REG_CMR:	/*  Channel Mode Register  */
	case CLMPCC_REG_COR1:	/*  Channel Option Register #1  */
	case CLMPCC_REG_COR2:	/*  Channel Option Register #2  */
	case CLMPCC_REG_COR3:	/*  Channel Option Register #3  */
	case CLMPCC_REG_COR4:	/*  Channel Option Register #4  */
	case CLMPCC_REG_COR5:	/*  Channel Option Register #5  */
	case CLMPCC_REG_COR6:	/*  Channel Option Register #6  */
	case CLMPCC_REG_COR7:	/*  Channel Option Register #7  */
	case CLMPCC_REG_SCHR1:	/*  Special Character Register #1  */
	case CLMPCC_REG_SCHR2:	/*  Special Character Register #2  */
	case CLMPCC_REG_SCHR3:	/*  Special Character Register #3  */
	case CLMPCC_REG_SCHR4:	/*  Special Character Register #4  */
	case CLMPCC_REG_SCRl:	/*  Special Character Range (low)  */
	case CLMPCC_REG_SCRh:	/*  Special Character Range (high)  */
	case CLMPCC_REG_LNXT:	/*  LNext Character  */
		break;

	case CLMPCC_REG_STCR:	/*  Special Transmit Command Register  */
		if (writeflag == MEM_WRITE) {
			if (idata == 0x0b) {
				if (d->reg[CLMPCC_REG_CAR] == 0)
					console_putchar(d->console_handle,
					    d->reg[CLMPCC_REG_SCHR3]);
				else
					fatal("[ clmpcc: TODO: transmit "
					    "to channel, CAR!=0 ]\n");

				/*  Command done:  */
				d->reg[CLMPCC_REG_STCR] = 0x00;
			} else {
				fatal("clmpcc: unimplemented STCR byte "
				    "0x%02x\n", (int) idata);
				exit(1);
			}
		}
		break;

	case CLMPCC_REG_RBPR:	/*  Receive Baud Rate Period Register  */
	case CLMPCC_REG_TBPR:	/*  Transmit Baud Rate Period Register  */
	case CLMPCC_REG_RCOR:	/*  Receive Clock Options Register  */
	case CLMPCC_REG_TCOR:	/*  Transmit Clock Options Register  */
		break;

	case CLMPCC_REG_MSVR_RTS:/* Modem Signal Value Register, RTS  */
	case CLMPCC_REG_MSVR_DTR:/* Modem Signal Value Register, DTR  */
		break;

	case CLMPCC_REG_RTPRl:	/*  Receive Timeout Period Register (low)  */
	case CLMPCC_REG_RTPRh:	/*  Receive Timeout Period Register (high)  */
		break;

	case CLMPCC_REG_IER:	/*  Interrupt Enable Register  */
	case CLMPCC_REG_LIVR:	/*  Local Interrupt Vector Register  */
	case CLMPCC_REG_RPILR:	/*  Rx Priority Interrupt Level Register  */
	case CLMPCC_REG_TPILR:	/*  Tx Priority Interrupt Level Register  */
	case CLMPCC_REG_MPILR:	/*  Modem Priority Interrupt Level Register  */
		reassert_interrupts(d);
		break;

	case CLMPCC_REG_RISRl:	/*  Receive Interrupt Status Reg (low)  */
		odata = 0x00;
		break;

	case CLMPCC_REG_TIR:	/*  Transmit Interrupt Register  */
		odata = 0x40;	/*  see openbsd's cl_txintr  */
		break;

	case CLMPCC_REG_RIR:	/*  Rx Interrupt Register  */
		odata = 0x00;
		if (console_charavail(d->console_handle))
			odata = 0xc0;
		break;

	case CLMPCC_REG_LICR:	/*  Local Interrupt Status Register  */
		odata = 0;	/*  channel nr << 2  */
		break;

	case CLMPCC_REG_RFOC:	/*  Rx FIFO Output Count  */
		odata = 0;
		if (console_charavail(d->console_handle))
			odata = 1;
		break;

	case CLMPCC_REG_TFTC:	/*  Tx FIFO Transfer Count  */
		/*
		 *  0x1f is low enough to allow OpenBSD/mvme88k's ramdisk
		 *  kernel to run (bsd.rd), but not the default kernel (bsd).
		 *  Lowering it to 0x0f seems to work.
		 */
		odata = 0x0f;
		break;

	case CLMPCC_REG_TEOIR:	/*  Tx End of Interrupt Register  */
	case CLMPCC_REG_REOIR:	/*  Rx End of Interrupt Register  */
		/*  TODO: Do something more realistic?  */
		INTERRUPT_DEASSERT(d->irq_scc_tx);
		INTERRUPT_DEASSERT(d->irq_scc_rx);
		break;

	case CLMPCC_REG_TDR:
		if (writeflag == MEM_WRITE) {
			if (d->reg[CLMPCC_REG_CAR] == 0)
				console_putchar(d->console_handle, idata);
			else
				fatal("[ clmpcc: TODO: transmit "
				    "to channel, CAR!=0 ]\n");
		} else {
			odata = console_readchar(d->console_handle);
		}
		break;

	default:if (writeflag == MEM_READ)
			fatal("[ clmpcc: unimplemented READ from offset 0x%x ]"
			    "\n", (int)relative_addr);
		else
			fatal("[ clmpcc: unimplemented WRITE to offset 0x%x: "
			    "0x%x ]\n", (int)relative_addr, (int)idata);
		exit(1);
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(clmpcc)
{
	struct clmpcc_data *d;
	char tmpstr[100];

	CHECK_ALLOCATION(d = (struct clmpcc_data *) malloc(sizeof(struct clmpcc_data)));
	memset(d, 0, sizeof(struct clmpcc_data));

	d->console_handle =
	    console_start_slave(devinit->machine, devinit->name2 != NULL?
	    devinit->name2 : devinit->name, devinit->in_use);

	/*
	 *  Connect to the PCC2's interrupt pins:
	 *
	 *  The supplied interrupt_path is something like
	 *  "machine[0].cpu[0].pcc2".
	 *
	 *  We want to use "machine[0].cpu[0].pcc2.x", where x is
	 *  0xc, 0xd, 0xe, and 0xf (PCC2V_SCC_xxx).
	 */
	snprintf(tmpstr, sizeof(tmpstr), "%s.%i",
	    devinit->interrupt_path, PCC2V_SCC_RXE);
	INTERRUPT_CONNECT(tmpstr, d->irq_scc_rxe);
	snprintf(tmpstr, sizeof(tmpstr), "%s.%i",
	    devinit->interrupt_path, PCC2V_SCC_M);
	INTERRUPT_CONNECT(tmpstr, d->irq_scc_m);
	snprintf(tmpstr, sizeof(tmpstr), "%s.%i",
	    devinit->interrupt_path, PCC2V_SCC_TX);
	INTERRUPT_CONNECT(tmpstr, d->irq_scc_tx);
	snprintf(tmpstr, sizeof(tmpstr), "%s.%i",
	    devinit->interrupt_path, PCC2V_SCC_RX);
	INTERRUPT_CONNECT(tmpstr, d->irq_scc_rx);

	memory_device_register(devinit->machine->memory, devinit->name,
	    devinit->addr, CLMPCC_LEN, dev_clmpcc_access, (void *)d,
	    DM_DEFAULT, NULL);

	machine_add_tickfunction(devinit->machine,
	    dev_clmpcc_tick, d, DEV_CLMPCC_TICK_SHIFT);

	/*
	 *  NOTE:  Ugly cast into a pointer, because this is a convenient way
	 *         to return the console handle to code in src/machines/.
	 */
	devinit->return_ptr = (void *)(size_t)d->console_handle;

	return 1;
}

