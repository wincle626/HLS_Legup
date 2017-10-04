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
 *  COMMENT: PlayStation 2 misc stuff (timer, DMA, interrupts, ...)
 *
 *	offset 0x0000	timer control
 *	offset 0x8000	DMA controller
 *	offset 0xf000	Interrupt register
 *
 *  The 16 normal PS2 interrupts interrupt at MIPS interrupt 2.
 *  The 16 DMA interrupts are connected to MIPS interrupt 3.
 *
 *  SBUS interrupts go via PS2 interrupt 1.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/ee_timerreg.h"
#include "thirdparty/ps2_dmacreg.h"


#define	TICK_STEPS_SHIFT	14

/*  NOTE/TODO: This should be the same as in ps2_gs:  */
#define	DEV_PS2_GIF_FAKE_BASE		0x50000000

#define	N_PS2_DMA_CHANNELS		10
#define	N_PS2_TIMERS			4

struct ps2_data {
	uint32_t	timer_count[N_PS2_TIMERS];
	uint32_t	timer_comp[N_PS2_TIMERS];
	uint32_t	timer_mode[N_PS2_TIMERS];
	uint32_t	timer_hold[N_PS2_TIMERS];
				/*  NOTE: only 0 and 1 are valid  */
	struct interrupt timer_irq[N_PS2_TIMERS];

	uint64_t	dmac_reg[DMAC_REGSIZE / 0x10];
	struct interrupt dmac_irq;		/*  MIPS irq 3  */
	struct interrupt dma_channel2_irq;	/*  irq path of channel 2  */

	uint64_t	other_memory_base[N_PS2_DMA_CHANNELS];

	uint32_t	intr;
	uint32_t	imask;
	uint32_t	sbus_smflg;
	struct interrupt intr_irq;		/*  MIPS irq 2  */
	struct interrupt sbus_irq;		/*  PS2 irq 1  */
};

#define	DEV_PS2_LENGTH		0x10000


void ps2_intr_interrupt_assert(struct interrupt *interrupt)
{
	struct ps2_data *d = (struct ps2_data *) interrupt->extra;
	d->intr |= (1 << interrupt->line);
	if (d->intr & d->imask)
		INTERRUPT_ASSERT(d->intr_irq);
}
void ps2_intr_interrupt_deassert(struct interrupt *interrupt)
{
	struct ps2_data *d = (struct ps2_data *) interrupt->extra;
	d->intr &= ~(1 << interrupt->line);
	if (!(d->intr & d->imask))
		INTERRUPT_DEASSERT(d->intr_irq);
}
void ps2_dmac_interrupt_assert(struct interrupt *interrupt)
{
	struct ps2_data *d = (struct ps2_data *) interrupt->extra;
	d->dmac_reg[0x601] |= (1 << interrupt->line);
	/*  TODO: DMA interrupt mask?  */
	if (d->dmac_reg[0x601] & 0xffff)
		INTERRUPT_ASSERT(d->dmac_irq);
}
void ps2_dmac_interrupt_deassert(struct interrupt *interrupt)
{
	struct ps2_data *d = (struct ps2_data *) interrupt->extra;
	d->dmac_reg[0x601] &= ~(1 << interrupt->line);
	/*  TODO: DMA interrupt mask?  */
	if (!(d->dmac_reg[0x601] & 0xffff))
		INTERRUPT_DEASSERT(d->dmac_irq);
}
void ps2_sbus_interrupt_assert(struct interrupt *interrupt)
{
	/*  Note: sbus irq 0 = mask 0x100, sbus irq 1 = mask 0x400  */
	struct ps2_data *d = (struct ps2_data *) interrupt->extra;
	d->sbus_smflg |= (1 << (8 + interrupt->line * 2));
	/*  TODO: SBUS interrupt mask?  */
	if (d->sbus_smflg != 0)
		INTERRUPT_ASSERT(d->sbus_irq);
}
void ps2_sbus_interrupt_deassert(struct interrupt *interrupt)
{
	/*  Note: sbus irq 0 = mask 0x100, sbus irq 1 = mask 0x400  */
	struct ps2_data *d = (struct ps2_data *) interrupt->extra;
	d->sbus_smflg &= ~(1 << (8 + interrupt->line * 2));
	/*  TODO: SBUS interrupt mask?  */
	if (d->sbus_smflg == 0)
		INTERRUPT_DEASSERT(d->sbus_irq);
}


DEVICE_TICK(ps2)
{
	struct ps2_data *d = (struct ps2_data *) extra;
	int i;

	/*
	 *  Right now this interrupts every now and then.
	 *  The main interrupt in NetBSD should be 100 Hz. TODO.
	 */
	for (i=0; i<N_PS2_TIMERS; i++) {
		/*  Count-up Enable:   TODO: by how much?  */
		if (d->timer_mode[i] & T_MODE_CUE)
			d->timer_count[i] ++;

		if (d->timer_mode[i] & (T_MODE_CMPE | T_MODE_OVFE)) {
			/*  Zero return:  */
			if (d->timer_mode[i] & T_MODE_ZRET)
				d->timer_count[i] = 0;

			INTERRUPT_ASSERT(d->timer_irq[i]);

			/*  timer 1..3 are "single-shot"? TODO  */
			if (i > 0) {
				d->timer_mode[i] &=
				    ~(T_MODE_CMPE | T_MODE_OVFF);
			}
		}
	}
}


DEVICE_ACCESS(ps2)
{
	uint64_t idata = 0, odata = 0;
	int regnr = 0;
	struct ps2_data *d = (struct ps2_data *) extra;
	int timer_nr = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	if (relative_addr >= 0x8000 && relative_addr < 0x8000 + DMAC_REGSIZE) {
		regnr = (relative_addr - 0x8000) / 16;
		if (writeflag == MEM_READ)
			odata = d->dmac_reg[regnr];
		else
			d->dmac_reg[regnr] = idata;
	}

	/*
	 *  Timer control:
	 *  The four timers are at offsets 0, 0x800, 0x1000, and 0x1800.
	 */
	if (relative_addr < TIMER_REGSIZE) {
		/*  0, 1, 2, or 3  */
		timer_nr = (relative_addr & 0x1800) >> 11;
		relative_addr &= (TIMER_OFS-1);
	}

	switch (relative_addr) {
	case 0x0000:	/*  timer count  */
		if (writeflag == MEM_READ) {
			odata = d->timer_count[timer_nr];
			if (timer_nr == 0) {
				/*  :-)  TODO: remove this?  */
				d->timer_count[timer_nr] ++;
			}
			debug("[ ps2: read timer %i count: 0x%llx ]\n",
			    timer_nr, (long long)odata);
		} else {
			d->timer_count[timer_nr] = idata;
			debug("[ ps2: write timer %i count: 0x%llx ]\n",
			    timer_nr, (long long)idata);
		}
		break;
	case 0x0010:	/*  timer mode  */
		if (writeflag == MEM_READ) {
			odata = d->timer_mode[timer_nr];
			debug("[ ps2: read timer %i mode: 0x%llx ]\n",
			    timer_nr, (long long)odata);
		} else {
			d->timer_mode[timer_nr] = idata;
			debug("[ ps2: write timer %i mode: 0x%llx ]\n",
			    timer_nr, (long long)idata);
		}
		break;
	case 0x0020:	/*  timer comp  */
		if (writeflag == MEM_READ) {
			odata = d->timer_comp[timer_nr];
			debug("[ ps2: read timer %i comp: 0x%llx ]\n",
			    timer_nr, (long long)odata);
		} else {
			d->timer_comp[timer_nr] = idata;
			debug("[ ps2: write timer %i comp: 0x%llx ]\n",
			    timer_nr, (long long)idata);
		}
		break;
	case 0x0030:	/*  timer hold  */
		if (writeflag == MEM_READ) {
			odata = d->timer_hold[timer_nr];
			debug("[ ps2: read timer %i hold: 0x%llx ]\n",
			    timer_nr, (long long)odata);
			if (timer_nr >= 2)
				fatal("[ WARNING: ps2: read from non-"
				    "existant timer %i hold register ]\n");
		} else {
			d->timer_hold[timer_nr] = idata;
			debug("[ ps2: write timer %i hold: 0x%llx ]\n",
			    timer_nr, (long long)idata);
			if (timer_nr >= 2)
				fatal("[ WARNING: ps2: write to "
				    "non-existant timer %i hold register ]\n",
				    timer_nr);
		}
		break;

	case 0x8000 + D2_CHCR_REG:
		if (writeflag==MEM_READ) {
			odata = d->dmac_reg[regnr];
			/*  debug("[ ps2: dmac read from D2_CHCR "
			    "(0x%llx) ]\n", (long long)d->dmac_reg[regnr]);  */
		} else {
			/*  debug("[ ps2: dmac write to D2_CHCR, "
			    "data 0x%016llx ]\n", (long long) idata);  */
			if (idata & D_CHCR_STR) {
				int length = d->dmac_reg[D2_QWC_REG/0x10] * 16;
				uint64_t from_addr = d->dmac_reg[
				    D2_MADR_REG/0x10];
				uint64_t to_addr   = d->dmac_reg[
				    D2_TADR_REG/0x10];
				unsigned char *copy_buf;

				debug("[ ps2: dmac [ch2] transfer addr="
				    "0x%016llx len=0x%lx ]\n", (long long)
				    d->dmac_reg[D2_MADR_REG/0x10],
				    (long)length);

				CHECK_ALLOCATION(copy_buf = (unsigned char *) malloc(length));

				cpu->memory_rw(cpu, cpu->mem, from_addr,
				    copy_buf, length, MEM_READ,
				    CACHE_NONE | PHYSICAL);
				cpu->memory_rw(cpu, cpu->mem,
				    d->other_memory_base[DMA_CH_GIF] + to_addr,
				    copy_buf, length, MEM_WRITE,
				    CACHE_NONE | PHYSICAL);
				free(copy_buf);

				/*  Done with the transfer:  */
				d->dmac_reg[D2_QWC_REG/0x10] = 0;
				idata &= ~D_CHCR_STR;

				/*  interrupt DMA channel 2  */
				INTERRUPT_ASSERT(d->dma_channel2_irq);
			} else
				debug("[ ps2: dmac [ch2] stopping "
				    "transfer ]\n");
			d->dmac_reg[regnr] = idata;
			return 1;
		}
		break;

	case 0x8000 + D2_QWC_REG:
	case 0x8000 + D2_MADR_REG:
	case 0x8000 + D2_TADR_REG:
		/*  no debug output  */
		break;

	case 0xe010:	/*  dmac interrupt status (and mask,  */
			/*  the upper 16 bits)  */
		if (writeflag == MEM_WRITE) {
			uint32_t oldmask = d->dmac_reg[regnr] & 0xffff0000;
			/*  Clear out those bits that are set in idata:  */
			d->dmac_reg[regnr] &= ~idata;
			d->dmac_reg[regnr] &= 0xffff;
			d->dmac_reg[regnr] |= oldmask;
			if (((d->dmac_reg[regnr] & 0xffff) &
			    ((d->dmac_reg[regnr]>>16) & 0xffff)) == 0) {
				INTERRUPT_DEASSERT(d->dmac_irq);
			}
		} else {
			/*  Hm... make it seem like the mask bits are (at
			    least as much as) the interrupt assertions:  */
			odata = d->dmac_reg[regnr];
			odata |= (odata << 16);
		}
		break;

	case 0xf000:	/*  interrupt register  */
		if (writeflag == MEM_READ) {
			odata = d->intr;
			debug("[ ps2: read from Interrupt Register:"
			    " 0x%llx ]\n", (long long)odata);

			/*  TODO: This is _NOT_ correct behavior:  */
//			d->intr = 0;
//			INTERRUPT_DEASSERT(d->intr_irq);
		} else {
			debug("[ ps2: write to Interrupt Register: "
			    "0x%llx ]\n", (long long)idata);
			/*  Clear out bits that are set in idata:  */
			d->intr &= ~idata;

			if ((d->intr & d->imask) == 0)
				INTERRUPT_DEASSERT(d->intr_irq);
		}
		break;

	case 0xf010:	/*  interrupt mask  */
		if (writeflag == MEM_READ) {
			odata = d->imask;
			/*  debug("[ ps2: read from Interrupt Mask "
			    "Register: 0x%llx ]\n", (long long)odata);  */
		} else {
			/*  debug("[ ps2: write to Interrupt Mask "
			    "Register: 0x%llx ]\n", (long long)idata);  */
			/*  Note: written value indicates which bits
			    to _toggle_, not which bits to set!  */
			d->imask ^= idata;
		}
		break;

	case 0xf230:	/*  sbus interrupt register?  */
		if (writeflag == MEM_READ) {
			odata = d->sbus_smflg;
			debug("[ ps2: read from SBUS SMFLG:"
			    " 0x%llx ]\n", (long long)odata);
		} else {
			/*  Clear bits on write:  */
			debug("[ ps2: write to SBUS SMFLG:"
			    " 0x%llx ]\n", (long long)idata);
			d->sbus_smflg &= ~idata;
			/*  irq 1 is SBUS  */
			if (d->sbus_smflg == 0)
				INTERRUPT_DEASSERT(d->sbus_irq);
		}
		break;
	default:
		if (writeflag==MEM_READ) {
			debug("[ ps2: read from addr 0x%x: 0x%llx ]\n",
			    (int)relative_addr, (long long)odata);
		} else {
			debug("[ ps2: write to addr 0x%x: 0x%llx ]\n",
			    (int)relative_addr, (long long)idata);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(ps2)
{
	struct ps2_data *d;
	int i;
	struct interrupt templ;
	char n[300];

	CHECK_ALLOCATION(d = (struct ps2_data *) malloc(sizeof(struct ps2_data)));
	memset(d, 0, sizeof(struct ps2_data));

	d->other_memory_base[DMA_CH_GIF] = DEV_PS2_GIF_FAKE_BASE;

	/*  Connect to MIPS irq 2 (interrupt controller) and 3 (dmac):  */
	snprintf(n, sizeof(n), "%s.2", devinit->interrupt_path);
	INTERRUPT_CONNECT(n, d->intr_irq);
	snprintf(n, sizeof(n), "%s.3", devinit->interrupt_path);
	INTERRUPT_CONNECT(n, d->dmac_irq);

	/*
	 *  Register interrupts:
	 *
	 *	16 normal IRQs	(machine[x].cpu[x].ps2_intr.%i)
	 *	16 DMA IRQs	(machine[x].cpu[x].ps2_dmac.%i)
	 *	 2 sbus IRQs	(machine[x].cpu[x].ps2_sbus.%i)
	 */
	for (i=0; i<16; i++) {
		snprintf(n, sizeof(n), "%s.ps2_intr.%i",
		    devinit->interrupt_path, i);
		memset(&templ, 0, sizeof(templ));
		templ.line = i;
		templ.name = n;
		templ.extra = d;
		templ.interrupt_assert = ps2_intr_interrupt_assert;
		templ.interrupt_deassert = ps2_intr_interrupt_deassert;
		interrupt_handler_register(&templ);
	}
	for (i=0; i<16; i++) {
		snprintf(n, sizeof(n), "%s.ps2_dmac.%i",
		    devinit->interrupt_path, i);
		memset(&templ, 0, sizeof(templ));
		templ.line = i;
		templ.name = n;
		templ.extra = d;
		templ.interrupt_assert = ps2_dmac_interrupt_assert;
		templ.interrupt_deassert = ps2_dmac_interrupt_deassert;
		interrupt_handler_register(&templ);
	}
	for (i=0; i<2; i++) {
		snprintf(n, sizeof(n), "%s.ps2_sbus.%i",
		    devinit->interrupt_path, i);
		memset(&templ, 0, sizeof(templ));
		templ.line = i;
		templ.name = n;
		templ.extra = d;
		templ.interrupt_assert = ps2_sbus_interrupt_assert;
		templ.interrupt_deassert = ps2_sbus_interrupt_deassert;
		interrupt_handler_register(&templ);
	}

	/*  Connect to DMA channel 2 irq:  */
	snprintf(n, sizeof(n), "%s.ps2_dmac.2", devinit->interrupt_path);
	INTERRUPT_CONNECT(n, d->dma_channel2_irq);

	/*  Connect to SBUS interrupt, at ps2 interrupt 1:  */
	snprintf(n, sizeof(n), "%s.ps2_intr.1", devinit->interrupt_path);
	INTERRUPT_CONNECT(n, d->sbus_irq);

	/*  Connect to the timers' interrupts:  */
	for (i=0; i<N_PS2_TIMERS; i++) {
		/*  PS2 irq 9 is timer0, etc.  */
		snprintf(n, sizeof(n), "%s.ps2_intr.%i",
		    devinit->interrupt_path, 9 + i);
		INTERRUPT_CONNECT(n, d->timer_irq[i]);
	}

	memory_device_register(devinit->machine->memory, "ps2", devinit->addr,
	    DEV_PS2_LENGTH, dev_ps2_access, d, DM_DEFAULT, NULL);
	machine_add_tickfunction(devinit->machine,
	    dev_ps2_tick, d, TICK_STEPS_SHIFT);

	return 1;
}

