/*
 *  Copyright (C) 2006-2009  Anders Gavare.  All rights reserved.
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
 *  COMMENT: SH4-specific memory mapped registers (0xf0000000 - 0xffffffff)
 *
 *  TODO: Among other things:
 *
 *	x)  Interrupt masks (msk register stuff).
 *	x)  BSC (Bus state controller).
 *	x)  DMA
 *	x)  UBC
 *	x)  ...
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bus_pci.h"
#include "console.h"
#include "cpu.h"
#include "device.h"
#include "devices.h"
#include "interrupt.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"
#include "sh4_dmacreg.h"
#include "timer.h"

#include "thirdparty/sh4_bscreg.h"
#include "thirdparty/sh4_cache.h"
#include "thirdparty/sh4_exception.h"
#include "thirdparty/sh4_intcreg.h"
#include "thirdparty/sh4_mmu.h"
#include "thirdparty/sh4_pcicreg.h"
#include "thirdparty/sh4_rtcreg.h"
#include "thirdparty/sh4_scifreg.h"
#include "thirdparty/sh4_scireg.h"
#include "thirdparty/sh4_tmureg.h"


#define	SH4_REG_BASE		0xff000000
#define	SH4_TICK_SHIFT		14
#define	N_SH4_TIMERS		3

/*  PCI stuff:  */
#define	N_PCIC_REGS			(0x224 / sizeof(uint32_t))
#define	N_PCIC_IRQS			16
#define	PCIC_REG(addr)			((addr - SH4_PCIC) / sizeof(uint32_t))
#define	PCI_VENDOR_HITACHI		0x1054
#define	PCI_PRODUCT_HITACHI_SH7751	0x3505
#define	PCI_PRODUCT_HITACHI_SH7751R	0x350e   

#define	SCIF_TX_FIFO_SIZE	16
#define	SCIF_DELAYED_TX_VALUE	2	/*  2 to be safe, 1 = fast but buggy  */

/*  General-purpose I/O stuff:  */
#define	SH4_PCTRA		0xff80002c
#define	SH4_PDTRA		0xff800030
#define	SH4_PCTRB		0xff800040
#define	SH4_PDTRB		0xff800044
#define	SH4_GPIOIC		0xff800048

#ifdef UNSTABLE_DEVEL
#define SH4_DEGUG
/*  #define debug fatal  */
#endif

struct sh4_data {
	/*  Store Queues:  */
	uint8_t		sq[32 * 2];

	/*  SCIF (Serial controller):  */
	uint16_t	scif_smr;
	uint8_t		scif_brr;
	uint16_t	scif_scr;
	uint16_t	scif_ssr;
	uint16_t	scif_fcr;
	uint16_t	scif_lsr;
	int		scif_delayed_tx;
	int		scif_console_handle;
	uint8_t		scif_tx_fifo[SCIF_TX_FIFO_SIZE + 1];
	size_t		scif_tx_fifo_cursize;
	struct interrupt scif_tx_irq;
	struct interrupt scif_rx_irq;
	int		scif_tx_irq_asserted;
	int		scif_rx_irq_asserted;

	/*  Bus State Controller:  */
	uint32_t	bsc_bcr1;
	uint16_t	bsc_bcr2;
	uint32_t	bsc_wcr1;
	uint32_t	bsc_wcr2;
	uint32_t	bsc_mcr;
	uint16_t	bsc_rtcsr;
	uint16_t	bsc_rtcor;
	uint16_t	bsc_rfcr;

	/*  GPIO:  */
	uint32_t	pctra;		/*  Port Control Register A  */
	uint32_t	pdtra;		/*  Port Data Register A  */
	uint32_t	pctrb;		/*  Port Control Register B  */
	uint32_t	pdtrb;		/*  Port Data Register B  */

	/*  PCIC (PCI controller):  */
	struct pci_data	*pci_data;
	struct interrupt cpu_pcic_interrupt[N_PCIC_IRQS];
	uint32_t	pcic_reg[N_PCIC_REGS];

	/*  SCI (serial interface):  */
	int		sci_bits_outputed;
	int		sci_bits_read;
	uint8_t		sci_scsptr;
	uint8_t		sci_curbyte;
	uint8_t		sci_cur_addr;

	/*  SD-RAM:  */
	uint16_t	sdmr2;
	uint16_t	sdmr3;

	/*  Timer Management Unit:  */
	struct timer	*sh4_timer;
	struct interrupt timer_irq[4];
	uint32_t	tocr;
	uint32_t	tstr;
	uint32_t	tcnt[N_SH4_TIMERS];
	uint32_t	tcor[N_SH4_TIMERS];
	uint32_t	tcr[N_SH4_TIMERS];
	int		timer_interrupts_pending[N_SH4_TIMERS];
	double		timer_hz[N_SH4_TIMERS];

	/*  RTC:  */
	uint32_t	rtc_reg[14];	/*  Excluding rcr1 and 2  */
	uint8_t		rtc_rcr1;
};


#define	SH4_PSEUDO_TIMER_HZ	110.0


/*
 *  sh4_timer_tick():
 *
 *  This function is called SH4_PSEUDO_TIMER_HZ times per real-world second.
 *  Its job is to update the SH4 timer counters, and if necessary, increase
 *  the number of pending interrupts.
 *
 *  Also, RAM Refresh is also faked here.
 */
static void sh4_timer_tick(struct timer *t, void *extra)
{
	struct sh4_data *d = (struct sh4_data *) extra;
	int i;

	/*  Fake RAM refresh:  */
	d->bsc_rfcr ++;
	if (d->bsc_rtcsr & (RTCSR_CMIE | RTCSR_OVIE)) {
		fatal("sh4: RTCSR_CMIE | RTCSR_OVIE: TODO\n");
		/*  TODO: Implement refresh interrupts etc.  */
		exit(1);
	}

	/*  Timer interrupts:  */
	for (i=0; i<N_SH4_TIMERS; i++) {
		int32_t old = d->tcnt[i];

		/*  printf("tcnt[%i] = %08x   tcor[%i] = %08x\n",
		    i, d->tcnt[i], i, d->tcor[i]);  */

		/*  Only update timers that are currently started:  */
		if (!(d->tstr & (TSTR_STR0 << i)))
			continue;

		/*  Update the current count:  */
		d->tcnt[i] -= (uint32_t) (d->timer_hz[i] / SH4_PSEUDO_TIMER_HZ);

		/*  Has the timer underflowed?  */
		if ((int32_t)d->tcnt[i] < 0 && old >= 0) {
			d->tcr[i] |= TCR_UNF;

			if (d->tcr[i] & TCR_UNIE)
				d->timer_interrupts_pending[i] ++;

			/*
			 *  Set tcnt[i] to tcor[i]. Note: Since this function
			 *  is only called now and then, adding tcor[i] to
			 *  tcnt[i] produces more correct values for long
			 *  running timers.
			 */
			d->tcnt[i] += d->tcor[i];

			/*  At least make sure that tcnt is non-negative...  */
			if ((int32_t)d->tcnt[i] < 0)
				d->tcnt[i] = 0;
		}
	}
}


static void sh4_pcic_interrupt_assert(struct interrupt *interrupt)
{
	struct sh4_data *d = (struct sh4_data *) interrupt->extra;
	INTERRUPT_ASSERT(d->cpu_pcic_interrupt[interrupt->line]);
}
static void sh4_pcic_interrupt_deassert(struct interrupt *interrupt)
{
	struct sh4_data *d = (struct sh4_data *) interrupt->extra;
	INTERRUPT_DEASSERT(d->cpu_pcic_interrupt[interrupt->line]);
}


static void scif_reassert_interrupts(struct sh4_data *d)
{
	int old_tx_asserted = d->scif_tx_irq_asserted;
	int old_rx_asserted = d->scif_rx_irq_asserted;

	d->scif_rx_irq_asserted =
	    d->scif_scr & SCSCR2_RIE && d->scif_ssr & SCSSR2_DR;

	if (d->scif_rx_irq_asserted && !old_rx_asserted)
		INTERRUPT_ASSERT(d->scif_rx_irq);
	else if (!d->scif_rx_irq_asserted && old_rx_asserted)
		INTERRUPT_DEASSERT(d->scif_rx_irq);

	d->scif_tx_irq_asserted =
	    d->scif_scr & SCSCR2_TIE &&
	    d->scif_ssr & (SCSSR2_TDFE | SCSSR2_TEND);

	if (d->scif_tx_irq_asserted && !old_tx_asserted)
		INTERRUPT_ASSERT(d->scif_tx_irq);
	else if (!d->scif_tx_irq_asserted && old_tx_asserted)
		INTERRUPT_DEASSERT(d->scif_tx_irq);
}


DEVICE_TICK(sh4)
{
	struct sh4_data *d = (struct sh4_data *) extra;
	unsigned int i;

	/*
	 *  Serial controller interrupts:
	 *
	 *  RX: Cause interrupt if any char is available.
	 *  TX: Send entire TX FIFO contents, and interrupt.
	 */
	if (console_charavail(d->scif_console_handle))
		d->scif_ssr |= SCSSR2_DR;
	else
		d->scif_ssr &= ~SCSSR2_DR;

	if (d->scif_delayed_tx) {
		if (--d->scif_delayed_tx == 0) {
			/*  Send TX FIFO contents:  */
			for (i=0; i<d->scif_tx_fifo_cursize; i++)
				console_putchar(d->scif_console_handle,
				    d->scif_tx_fifo[i]);

			/*  Clear FIFO:  */
			d->scif_tx_fifo_cursize = 0;

			/*  Done sending; cause a transmit end interrupt:  */
			d->scif_ssr |= SCSSR2_TDFE | SCSSR2_TEND;
		}
	}

	scif_reassert_interrupts(d);

	/*  Timer interrupts:  */
	for (i=0; i<N_SH4_TIMERS; i++)
		if (d->timer_interrupts_pending[i] > 0) {
			INTERRUPT_ASSERT(d->timer_irq[i]);
			d->tcr[i] |= TCR_UNF;
		}
}


/*
 *  sh4_dmac_transfer():
 *
 *  Called whenever a DMA transfer is to be executed.
 *  Clears the lowest bit of the corresponding channel's CHCR when done.
 */
void sh4_dmac_transfer(struct cpu *cpu, struct sh4_data *d, int channel)
{
	/*  According to the SH7760 manual, bits 31..29 are ignored in  */
	/*  both the SAR and DAR.  */
	uint32_t sar = cpu->cd.sh.dmac_sar[channel] & 0x1fffffff;
	uint32_t dar = cpu->cd.sh.dmac_dar[channel] & 0x1fffffff;
	uint32_t count = cpu->cd.sh.dmac_tcr[channel] & 0x1fffffff;
	uint32_t chcr = cpu->cd.sh.dmac_chcr[channel];
	int transmit_size = 1;
	int src_delta = 0, dst_delta = 0;
	int cause_interrupt = chcr & CHCR_IE;

	/*  DMAC not enabled? Then just return.  */
	if (!(chcr & CHCR_TD))
		return;

	/*  Transfer End already set? Then don't transfer again.  */
	if (chcr & CHCR_TE)
		return;

	/*  Special case: 0 means 16777216:  */
	if (count == 0)
		count = 16777216;

	switch (chcr & CHCR_TS) {
	case CHCR_TS_8BYTE: transmit_size = 8; break;
	case CHCR_TS_1BYTE: transmit_size = 1; break;
	case CHCR_TS_2BYTE: transmit_size = 2; break;
	case CHCR_TS_4BYTE: transmit_size = 4; break;
	case CHCR_TS_32BYTE: transmit_size = 32; break;
	default: fatal("Unimplemented transmit size?! CHCR[%i] = 0x%08x\n",
	    channel, chcr);
	exit(1);
	}

	switch (chcr & CHCR_DM) {
	case CHCR_DM_FIXED:       dst_delta = 0; break;
	case CHCR_DM_INCREMENTED: dst_delta = 1; break;
	case CHCR_DM_DECREMENTED: dst_delta = -1; break;
	default: fatal("Unimplemented destination delta?! CHCR[%i] = 0x%08x\n",
	    channel, chcr);
	exit(1);
	}

	switch (chcr & CHCR_SM) {
	case CHCR_SM_FIXED:       src_delta = 0; break;
	case CHCR_SM_INCREMENTED: src_delta = 1; break;
	case CHCR_SM_DECREMENTED: src_delta = -1; break;
	default: fatal("Unimplemented source delta?! CHCR[%i] = 0x%08x\n",
	    channel, chcr);
	exit(1);
	}

	src_delta *= transmit_size;
	dst_delta *= transmit_size;

#ifdef SH4_DEGUG
	fatal("|SH4 DMA transfer, channel %i\n", channel);
	fatal("|Source addr:      0x%08x (delta %i)\n", (int) sar, src_delta);
	fatal("|Destination addr: 0x%08x (delta %i)\n", (int) dar, dst_delta);
	fatal("|Count:            0x%08x\n", (int) count);
	fatal("|Transmit size:    0x%08x\n", (int) transmit_size);
	fatal("|Interrupt:        %s\n", cause_interrupt? "yes" : "no");
#endif

	switch (chcr & CHCR_RS) {
	case 0x200:
		/*
		 *  Single Address Mode
		 *  External Address Space => external device
		 */

		/*  Note: No transfer is done here! It is up to the
		    external device to do the transfer itself!  */
		break;

	default:fatal("Unimplemented SH4 RS DMAC: 0x%08x\n",
		    (int) (chcr & CHCR_RS));
		exit(1);
	}

	if (cause_interrupt) {
		fatal("TODO: sh4 dmac interrupt!\n");
		exit(1);
	}
}


/*
 *  sh4_sci_cmd():
 *
 *  Handle a SCI command byte.
 *
 *  Bit:   Meaning:
 *   7      Ignored (usually 1?)
 *   6      0=Write, 1=Read
 *   5      AD: Address transfer
 *   4      DT: Data transfer
 *   3..0   Data or address bits
 */
static void sh4_sci_cmd(struct sh4_data *d, struct cpu *cpu)
{
	uint8_t cmd = d->sci_curbyte;
	int writeflag = cmd & 0x40? 0 : 1;
	int address_transfer;

	/*  fatal("[ CMD BYTE %02x ]\n", cmd);  */

	if (!(cmd & 0x80)) {
		fatal("SCI cmd bit 7 not set? TODO\n");
		exit(1);
	}

	if ((cmd & 0x30) == 0x20)
		address_transfer = 1;
	else if ((cmd & 0x30) == 0x10)
		address_transfer = 0;
	else {
		fatal("SCI: Neither data nor address transfer? TODO\n");
		exit(1);
	}

	if (address_transfer)
		d->sci_cur_addr = cmd & 0x0f;

	if (!writeflag) {
		/*  Read data from the current address:  */
		uint8_t data_byte;

		cpu->memory_rw(cpu, cpu->mem, SCI_DEVICE_BASE + d->sci_cur_addr,
		    &data_byte, 1, MEM_READ, PHYSICAL);

		debug("[ SCI: read addr=%x data=%x ]\n",
		    d->sci_cur_addr, data_byte);

		d->sci_curbyte = data_byte;

		/*  Set bit 7 right away:  */
		d->sci_scsptr &= ~SCSPTR_SPB1DT;
		if (data_byte & 0x80)
			d->sci_scsptr |= SCSPTR_SPB1DT;
	}

	if (writeflag && !address_transfer) {
		/*  Write the 4 data bits to the current address:  */
		uint8_t data_byte = cmd & 0x0f;

		debug("[ SCI: write addr=%x data=%x ]\n",
		    d->sci_cur_addr, data_byte);

		cpu->memory_rw(cpu, cpu->mem, SCI_DEVICE_BASE + d->sci_cur_addr,
		    &data_byte, 1, MEM_WRITE, PHYSICAL);
	}
}


/*
 *  sh4_sci_access():
 *
 *  Reads or writes a bit via the SH4's serial interface. If writeflag is
 *  non-zero, input is used. If writeflag is zero, a bit is outputed as
 *  the return value from this function.
 */
static uint8_t sh4_sci_access(struct sh4_data *d, struct cpu *cpu,
	int writeflag, uint8_t input)
{
	if (writeflag) {
		/*  WRITE:  */
		int clockpulse;
		uint8_t old = d->sci_scsptr;
		d->sci_scsptr = input;

		/*
		 *  Clock pulse (SCSPTR_SPB0DT going from 0 to 1,
		 *  when SCSPTR_SPB0IO was already set):
		 */
		clockpulse = old & SCSPTR_SPB0IO &&
		    d->sci_scsptr & SCSPTR_SPB0DT &&
		    !(old & SCSPTR_SPB0DT);

		if (!clockpulse)
			return 0;

		/*  Are we in output or input mode?  */
		if (d->sci_scsptr & SCSPTR_SPB1IO) {
			/*  Output:  */
			int bit = d->sci_scsptr & SCSPTR_SPB1DT? 1 : 0;
			d->sci_curbyte <<= 1;
			d->sci_curbyte |= bit;
			d->sci_bits_outputed ++;
			if (d->sci_bits_outputed == 8) {
				/*  4 control bits and 4 address/data bits have
				    been written.  */
				sh4_sci_cmd(d, cpu);
				d->sci_bits_outputed = 0;
			}
		} else {
			/*  Input:  */
			int bit;
			d->sci_bits_read ++;
			d->sci_bits_read &= 7;

			bit = d->sci_curbyte & (0x80 >> d->sci_bits_read);

			d->sci_scsptr &= ~SCSPTR_SPB1DT;
			if (bit)
				d->sci_scsptr |= SCSPTR_SPB1DT;
		}

		/*  Return (value doesn't matter).  */
		return 0;
	} else {
		/*  READ:  */
		return d->sci_scsptr;
	}
}


DEVICE_ACCESS(sh4_itlb_aa)
{
	uint64_t idata = 0, odata = 0;
	int e = (relative_addr & SH4_ITLB_E_MASK) >> SH4_ITLB_E_SHIFT;

	if (writeflag == MEM_WRITE) {
		int safe_to_invalidate = 0;
		uint32_t old_hi = cpu->cd.sh.itlb_hi[e];
		if ((cpu->cd.sh.itlb_lo[e] & SH4_PTEL_SZ_MASK)==SH4_PTEL_SZ_4K)
			safe_to_invalidate = 1;

		idata = memory_readmax64(cpu, data, len);
		cpu->cd.sh.itlb_hi[e] &=
		    ~(SH4_PTEH_VPN_MASK | SH4_PTEH_ASID_MASK);
		cpu->cd.sh.itlb_hi[e] |= (idata &
		    (SH4_ITLB_AA_VPN_MASK | SH4_ITLB_AA_ASID_MASK));
		cpu->cd.sh.itlb_lo[e] &= ~SH4_PTEL_V;
		if (idata & SH4_ITLB_AA_V)
			cpu->cd.sh.itlb_lo[e] |= SH4_PTEL_V;

		/*  Invalidate if this ITLB entry previously belonged to the
		    currently running process, or if it was shared:  */
		if (cpu->cd.sh.ptel & SH4_PTEL_SH ||
		    (old_hi & SH4_ITLB_AA_ASID_MASK) ==
		    (cpu->cd.sh.pteh & SH4_PTEH_ASID_MASK)) {
			if (safe_to_invalidate)
				cpu->invalidate_translation_caches(cpu,
				    old_hi & ~0xfff, INVALIDATE_VADDR);
			else
				cpu->invalidate_translation_caches(cpu,
				    0, INVALIDATE_ALL);
		}
	} else {
		odata = cpu->cd.sh.itlb_hi[e] &
		    (SH4_ITLB_AA_VPN_MASK | SH4_ITLB_AA_ASID_MASK);
		if (cpu->cd.sh.itlb_lo[e] & SH4_PTEL_V)
			odata |= SH4_ITLB_AA_V;
		memory_writemax64(cpu, data, len, odata);
	}

	return 1;
}


DEVICE_ACCESS(sh4_itlb_da1)
{
	uint32_t mask = SH4_PTEL_SH | SH4_PTEL_C | SH4_PTEL_SZ_MASK |
	    SH4_PTEL_PR_MASK | SH4_PTEL_V | 0x1ffffc00;
	uint64_t idata = 0, odata = 0;
	int e = (relative_addr & SH4_ITLB_E_MASK) >> SH4_ITLB_E_SHIFT;

	if (relative_addr & 0x800000) {
		fatal("sh4_itlb_da1: TODO: da2 area\n");
		exit(1);
	}

	if (writeflag == MEM_WRITE) {
		uint32_t old_lo = cpu->cd.sh.itlb_lo[e];
		int safe_to_invalidate = 0;
		if ((cpu->cd.sh.itlb_lo[e] & SH4_PTEL_SZ_MASK)==SH4_PTEL_SZ_4K)
			safe_to_invalidate = 1;

		idata = memory_readmax64(cpu, data, len);
		cpu->cd.sh.itlb_lo[e] &= ~mask;
		cpu->cd.sh.itlb_lo[e] |= (idata & mask);

		/*  Invalidate if this ITLB entry belongs to the
		    currently running process, or if it was shared:  */
		if (old_lo & SH4_PTEL_SH ||
		    (cpu->cd.sh.itlb_hi[e] & SH4_ITLB_AA_ASID_MASK) ==
		    (cpu->cd.sh.pteh & SH4_PTEH_ASID_MASK)) {
			if (safe_to_invalidate)
				cpu->invalidate_translation_caches(cpu,
				    cpu->cd.sh.itlb_hi[e] & ~0xfff,
				    INVALIDATE_VADDR);
			else
				cpu->invalidate_translation_caches(cpu,
				    0, INVALIDATE_ALL);
		}
	} else {
		odata = cpu->cd.sh.itlb_lo[e] & mask;
		memory_writemax64(cpu, data, len, odata);
	}

	return 1;
}


DEVICE_ACCESS(sh4_utlb_aa)
{
	uint64_t idata = 0, odata = 0;
	int i, e = (relative_addr & SH4_UTLB_E_MASK) >> SH4_UTLB_E_SHIFT;
	int a = relative_addr & SH4_UTLB_A;

	if (writeflag == MEM_WRITE) {
		int n_hits = 0;
		int safe_to_invalidate = 0;
		uint32_t vaddr_to_invalidate = 0;

		idata = memory_readmax64(cpu, data, len);
		if (a) {
			for (i=-SH_N_ITLB_ENTRIES; i<SH_N_UTLB_ENTRIES; i++) {
				uint32_t lo, hi;
				uint32_t mask = 0xfffff000;
				int sh;

				if (i < 0) {
					lo = cpu->cd.sh.itlb_lo[
					    i + SH_N_ITLB_ENTRIES];
					hi = cpu->cd.sh.itlb_hi[
					    i + SH_N_ITLB_ENTRIES];
				} else {
					lo = cpu->cd.sh.utlb_lo[i];
					hi = cpu->cd.sh.utlb_hi[i];
				}

				sh = lo & SH4_PTEL_SH;
				if (!(lo & SH4_PTEL_V))
					continue;

				switch (lo & SH4_PTEL_SZ_MASK) {
				case SH4_PTEL_SZ_1K:  mask = 0xfffffc00; break;
				case SH4_PTEL_SZ_64K: mask = 0xffff0000; break;
				case SH4_PTEL_SZ_1M:  mask = 0xfff00000; break;
				}

				if ((hi & mask) != (idata & mask))
					continue;

				if ((lo & SH4_PTEL_SZ_MASK) ==
				    SH4_PTEL_SZ_4K) {
					safe_to_invalidate = 1;
					vaddr_to_invalidate = hi & mask;
				}

				if (!sh && (hi & SH4_PTEH_ASID_MASK) !=
				    (cpu->cd.sh.pteh & SH4_PTEH_ASID_MASK))
					continue;

				if (i < 0) {
					cpu->cd.sh.itlb_lo[i +
					    SH_N_ITLB_ENTRIES] &= ~SH4_PTEL_V;
					if (idata & SH4_UTLB_AA_V)
						cpu->cd.sh.itlb_lo[
						    i+SH_N_ITLB_ENTRIES] |=
						    SH4_PTEL_V;
				} else {
					cpu->cd.sh.utlb_lo[i] &=
					    ~(SH4_PTEL_D | SH4_PTEL_V);
					if (idata & SH4_UTLB_AA_D)
						cpu->cd.sh.utlb_lo[i] |=
						    SH4_PTEL_D;
					if (idata & SH4_UTLB_AA_V)
						cpu->cd.sh.utlb_lo[i] |=
						    SH4_PTEL_V;
				}

				if (i >= 0)
					n_hits ++;
			}

			if (n_hits > 1)
				sh_exception(cpu,
				    EXPEVT_RESET_TLB_MULTI_HIT, 0, 0);
		} else {
			if ((cpu->cd.sh.utlb_lo[e] & SH4_PTEL_SZ_MASK) ==
			    SH4_PTEL_SZ_4K) {
				safe_to_invalidate = 1;
				vaddr_to_invalidate =
				    cpu->cd.sh.utlb_hi[e] & ~0xfff;
			}

			cpu->cd.sh.utlb_hi[e] &=
			    ~(SH4_PTEH_VPN_MASK | SH4_PTEH_ASID_MASK);
			cpu->cd.sh.utlb_hi[e] |= (idata &
			    (SH4_UTLB_AA_VPN_MASK | SH4_UTLB_AA_ASID_MASK));

			cpu->cd.sh.utlb_lo[e] &= ~(SH4_PTEL_D | SH4_PTEL_V);
			if (idata & SH4_UTLB_AA_D)
				cpu->cd.sh.utlb_lo[e] |= SH4_PTEL_D;
			if (idata & SH4_UTLB_AA_V)
				cpu->cd.sh.utlb_lo[e] |= SH4_PTEL_V;
		}

		if (safe_to_invalidate)
			cpu->invalidate_translation_caches(cpu,
			    vaddr_to_invalidate, INVALIDATE_VADDR);
		else
			cpu->invalidate_translation_caches(cpu, 0,
			    INVALIDATE_ALL);
	} else {
		odata = cpu->cd.sh.utlb_hi[e] &
		    (SH4_UTLB_AA_VPN_MASK | SH4_UTLB_AA_ASID_MASK);
		if (cpu->cd.sh.utlb_lo[e] & SH4_PTEL_D)
			odata |= SH4_UTLB_AA_D;
		if (cpu->cd.sh.utlb_lo[e] & SH4_PTEL_V)
			odata |= SH4_UTLB_AA_V;
		memory_writemax64(cpu, data, len, odata);
	}

	return 1;
}


DEVICE_ACCESS(sh4_utlb_da1)
{
	uint32_t mask = SH4_PTEL_WT | SH4_PTEL_SH | SH4_PTEL_D | SH4_PTEL_C
	    | SH4_PTEL_SZ_MASK | SH4_PTEL_PR_MASK | SH4_PTEL_V | 0x1ffffc00;
	uint64_t idata = 0, odata = 0;
	int e = (relative_addr & SH4_UTLB_E_MASK) >> SH4_UTLB_E_SHIFT;

	if (relative_addr & 0x800000) {
		fatal("sh4_utlb_da1: TODO: da2 area\n");
		exit(1);
	}

	if (writeflag == MEM_WRITE) {
		uint32_t old_lo = cpu->cd.sh.utlb_lo[e];
		int safe_to_invalidate = 0;
		if ((cpu->cd.sh.utlb_lo[e] & SH4_PTEL_SZ_MASK)==SH4_PTEL_SZ_4K)
			safe_to_invalidate = 1;

		idata = memory_readmax64(cpu, data, len);
		cpu->cd.sh.utlb_lo[e] &= ~mask;
		cpu->cd.sh.utlb_lo[e] |= (idata & mask);

		/*  Invalidate if this UTLB entry belongs to the
		    currently running process, or if it was shared:  */
		if (old_lo & SH4_PTEL_SH ||
		    (cpu->cd.sh.utlb_hi[e] & SH4_ITLB_AA_ASID_MASK) ==
		    (cpu->cd.sh.pteh & SH4_PTEH_ASID_MASK)) {
			if (safe_to_invalidate)
				cpu->invalidate_translation_caches(cpu,
				    cpu->cd.sh.utlb_hi[e] & ~0xfff,
				    INVALIDATE_VADDR);
			else
				cpu->invalidate_translation_caches(cpu,
				    0, INVALIDATE_ALL);
		}
	} else {
		odata = cpu->cd.sh.utlb_lo[e] & mask;
		memory_writemax64(cpu, data, len, odata);
	}

	return 1;
}


DEVICE_ACCESS(sh4_pcic)
{
	struct sh4_data *d = (struct sh4_data *) extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	relative_addr += SH4_PCIC;

	/*  Register read/write:  */
	if (writeflag == MEM_WRITE)
		d->pcic_reg[PCIC_REG(relative_addr)] = idata;
	else
		odata = d->pcic_reg[PCIC_REG(relative_addr)];

	/*  Special cases:  */

	switch (relative_addr) {

	case SH4_PCICONF0:
		if (writeflag == MEM_WRITE) {
			fatal("[ sh4_pcic: TODO: Write to SH4_PCICONF0? ]\n");
			exit(1);
		} else {
			if (strcmp(cpu->cd.sh.cpu_type.name, "SH7751") == 0) {
				odata = PCI_ID_CODE(PCI_VENDOR_HITACHI,
				    PCI_PRODUCT_HITACHI_SH7751);
			} else if (strcmp(cpu->cd.sh.cpu_type.name,
			    "SH7751R") == 0) {
				odata = PCI_ID_CODE(PCI_VENDOR_HITACHI,
				    PCI_PRODUCT_HITACHI_SH7751R);
			} else {
				fatal("sh4_pcic: TODO: PCICONF0 read for"
				    " unimplemented CPU type?\n");
				exit(1);
			}
		}
		break;

	case SH4_PCICONF1:
	case SH4_PCICONF2:
	case SH4_PCICR:
	case SH4_PCIBCR1:
	case SH4_PCIBCR2:
	case SH4_PCIBCR3:
	case SH4_PCIWCR1:
	case SH4_PCIWCR2:
	case SH4_PCIWCR3:
	case SH4_PCIMCR:
		break;

	case SH4_PCICONF5:
		/*  Hardcoded to what OpenBSD/landisk uses:  */
		if (writeflag == MEM_WRITE && idata != 0xac000000) {
			fatal("sh4_pcic: SH4_PCICONF5 unknown value"
			    " 0x%"PRIx32"\n", (uint32_t) idata);
			exit(1);
		}
		break;

	case SH4_PCICONF6:
		/*  Hardcoded to what OpenBSD/landisk uses:  */
		if (writeflag == MEM_WRITE && idata != 0x8c000000) {
			fatal("sh4_pcic: SH4_PCICONF6 unknown value"
			    " 0x%"PRIx32"\n", (uint32_t) idata);
			exit(1);
		}
		break;

	case SH4_PCILSR0:
		/*  Hardcoded to what OpenBSD/landisk uses:  */
		if (writeflag == MEM_WRITE && idata != ((64 - 1) << 20)) {
			fatal("sh4_pcic: SH4_PCILSR0 unknown value"
			    " 0x%"PRIx32"\n", (uint32_t) idata);
			exit(1);
		}
		break;

	case SH4_PCILAR0:
		/*  Hardcoded to what OpenBSD/landisk uses:  */
		if (writeflag == MEM_WRITE && idata != 0xac000000) {
			fatal("sh4_pcic: SH4_PCILAR0 unknown value"
			    " 0x%"PRIx32"\n", (uint32_t) idata);
			exit(1);
		}
		break;

	case SH4_PCILSR1:
		/*  Hardcoded to what OpenBSD/landisk uses:  */
		if (writeflag == MEM_WRITE && idata != ((64 - 1) << 20)) {
			fatal("sh4_pcic: SH4_PCILSR1 unknown value"
			    " 0x%"PRIx32"\n", (uint32_t) idata);
			exit(1);
		}
		break;

	case SH4_PCILAR1:
		/*  Hardcoded to what OpenBSD/landisk uses:  */
		if (writeflag == MEM_WRITE && idata != 0xac000000) {
			fatal("sh4_pcic: SH4_PCILAR1 unknown value"
			    " 0x%"PRIx32"\n", (uint32_t) idata);
			exit(1);
		}
		break;

	case SH4_PCIMBR:
		if (writeflag == MEM_WRITE && idata != SH4_PCIC_MEM) {
			fatal("sh4_pcic: PCIMBR set to 0x%"PRIx32", not"
			    " 0x%"PRIx32"? TODO\n", (uint32_t) idata,
			    (uint32_t) SH4_PCIC_MEM);
			exit(1);
		}
		break;

	case SH4_PCIIOBR:
		if (writeflag == MEM_WRITE && idata != SH4_PCIC_IO) {
			fatal("sh4_pcic: PCIIOBR set to 0x%"PRIx32", not"
			    " 0x%"PRIx32"? TODO\n", (uint32_t) idata,
			    (uint32_t) SH4_PCIC_IO);
			exit(1);
		}
		break;

	case SH4_PCIPAR:
		/*  PCI bus access Address Register:  */
		{
			int bus  = (idata >> 16) & 0xff;
			int dev  = (idata >> 11) & 0x1f;
			int func = (idata >>  8) &    7;
			int reg  =  idata        & 0xff;
			bus_pci_setaddr(cpu, d->pci_data, bus, dev, func, reg);
		}
		break;

	case SH4_PCIPDR:
		/*  PCI bus access Data Register:  */
		bus_pci_data_access(cpu, d->pci_data, writeflag == MEM_READ?
		    &odata : &idata, len, writeflag);
		break;

	default:if (writeflag == MEM_READ) {
			fatal("[ sh4_pcic: read from addr 0x%x: TODO ]\n",
			    (int)relative_addr);
		} else {
			fatal("[ sh4_pcic: write to addr 0x%x: 0x%x: TODO ]\n",
			    (int)relative_addr, (int)idata);
		}
		exit(1);
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVICE_ACCESS(sh4_sq)
{
	struct sh4_data *d = (struct sh4_data *) extra;
	size_t i;

	if (writeflag == MEM_WRITE) {
		for (i=0; i<len; i++)
			d->sq[(relative_addr + i) % sizeof(d->sq)] = data[i];
	} else {
		for (i=0; i<len; i++)
			data[i] = d->sq[(relative_addr + i) % sizeof(d->sq)];
	}

	return 1;
}


DEVICE_ACCESS(sh4)
{
	struct sh4_data *d = (struct sh4_data *) extra;
	uint64_t idata = 0, odata = 0;
	int timer_nr = 0, dma_channel = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	relative_addr += SH4_REG_BASE;

	/*  SD-RAM access uses address only:  */
	if (relative_addr >= 0xff900000 && relative_addr <= 0xff97ffff) {
		/*  Possibly not 100% correct... TODO  */
		int v = (relative_addr >> 2) & 0xffff;
		if (relative_addr & 0x00040000)
			d->sdmr3 = v;
		else
			d->sdmr2 = v;
		debug("[ sh4: sdmr%i set to 0x%04"PRIx16" ]\n",
		    relative_addr & 0x00040000? 3 : 2, v);
		return 1;
	}


	switch (relative_addr) {

	/*************************************************/

	case SH4_PVR_ADDR:
		odata = cpu->cd.sh.cpu_type.pvr;
		break;

	case SH4_PRR_ADDR:
		odata = cpu->cd.sh.cpu_type.prr;
		break;

	case SH4_PTEH:
		if (writeflag == MEM_READ)
			odata = cpu->cd.sh.pteh;
		else {
			unsigned int old_asid = cpu->cd.sh.pteh
			    & SH4_PTEH_ASID_MASK;
			cpu->cd.sh.pteh = idata;

			if ((idata & SH4_PTEH_ASID_MASK) != old_asid) {
				/*
				 *  TODO: Don't invalidate everything,
				 *  only those pages that belonged to the
				 *  old asid.
				 */
				cpu->invalidate_translation_caches(
				    cpu, 0, INVALIDATE_ALL);
			}
		}
		break;

	case SH4_PTEL:
		if (writeflag == MEM_READ)
			odata = cpu->cd.sh.ptel;
		else
			cpu->cd.sh.ptel = idata;
		break;

	case SH4_TTB:
		if (writeflag == MEM_READ)
			odata = cpu->cd.sh.ttb;
		else
			cpu->cd.sh.ttb = idata;
		break;

	case SH4_TEA:
		if (writeflag == MEM_READ)
			odata = cpu->cd.sh.tea;
		else
			cpu->cd.sh.tea = idata;
		break;

	case SH4_PTEA:
		if (writeflag == MEM_READ)
			odata = cpu->cd.sh.ptea;
		else
			cpu->cd.sh.ptea = idata;
		break;

	case SH4_MMUCR:
		if (writeflag == MEM_READ) {
			odata = cpu->cd.sh.mmucr;
		} else {
			if (idata & SH4_MMUCR_TI) {
				/*  TLB invalidate.  */
				int i;
				for (i = 0; i < SH_N_ITLB_ENTRIES; i++)
					cpu->cd.sh.itlb_lo[i] &=
					    ~SH4_PTEL_V;

				for (i = 0; i < SH_N_UTLB_ENTRIES; i++)
					cpu->cd.sh.utlb_lo[i] &=
					    ~SH4_PTEL_V;

				cpu->invalidate_translation_caches(cpu,
				    0, INVALIDATE_ALL);

				/*  The TI bit should always read as 0.  */
				idata &= ~SH4_MMUCR_TI;
			}

			cpu->cd.sh.mmucr = idata;
		}
		break;

	case SH4_CCR:
		if (writeflag == MEM_READ) {
			odata = cpu->cd.sh.ccr;
		} else {
			cpu->cd.sh.ccr = idata;
		}
		break;

	case SH4_QACR0:
		if (writeflag == MEM_READ) {
			odata = cpu->cd.sh.qacr0;
		} else {
			cpu->cd.sh.qacr0 = idata;
		}
		break;

	case SH4_QACR1:
		if (writeflag == MEM_READ) {
			odata = cpu->cd.sh.qacr1;
		} else {
			cpu->cd.sh.qacr1 = idata;
		}
		break;

	case SH4_TRA:
		if (writeflag == MEM_READ)
			odata = cpu->cd.sh.tra;
		else
			cpu->cd.sh.tra = idata;
		break;

	case SH4_EXPEVT:
		if (writeflag == MEM_READ)
			odata = cpu->cd.sh.expevt;
		else
			cpu->cd.sh.expevt = idata;
		break;

	case SH4_INTEVT:
		if (writeflag == MEM_READ)
			odata = cpu->cd.sh.intevt;
		else
			cpu->cd.sh.intevt = idata;
		break;


	/********************************/
	/*  UBC: User Break Controller  */

	case 0xff200008:    /*  SH4_BBRA  */
		/*  TODO  */
		break;


	/********************************/
	/*  TMU: Timer Management Unit  */

	case SH4_TOCR:
		/*  Timer Output Control Register  */
		if (writeflag == MEM_WRITE) {
			d->tocr = idata;
			if (idata & TOCR_TCOE)
				fatal("[ sh4 timer: TCOE not yet "
				    "implemented ]\n");
		} else {
			odata = d->tocr;
		}
		break;

	case SH4_TSTR:
		/*  Timer Start Register  */
		if (writeflag == MEM_READ) {
			odata = d->tstr;
		} else {
			if (idata & 1 && !(d->tstr & 1))
				debug("[ sh4 timer: starting timer 0 ]\n");
			if (idata & 2 && !(d->tstr & 2))
				debug("[ sh4 timer: starting timer 1 ]\n");
			if (idata & 4 && !(d->tstr & 4))
				debug("[ sh4 timer: starting timer 2 ]\n");
			if (!(idata & 1) && d->tstr & 1)
				debug("[ sh4 timer: stopping timer 0 ]\n");
			if (!(idata & 2) && d->tstr & 2)
				debug("[ sh4 timer: stopping timer 1 ]\n");
			if (!(idata & 4) && d->tstr & 4)
				debug("[ sh4 timer: stopping timer 2 ]\n");
			d->tstr = idata;
		}
		break;

	case SH4_TCOR2:
		timer_nr ++;
	case SH4_TCOR1:
		timer_nr ++;
	case SH4_TCOR0:
		/*  Timer Constant Register  */
		if (writeflag == MEM_READ)
			odata = d->tcor[timer_nr];
		else
			d->tcor[timer_nr] = idata;
		break;

	case SH4_TCNT2:
		timer_nr ++;
	case SH4_TCNT1:
		timer_nr ++;
	case SH4_TCNT0:
		/*  Timer Counter Register  */
		if (writeflag == MEM_READ)
			odata = d->tcnt[timer_nr];
		else
			d->tcnt[timer_nr] = idata;
		break;

	case SH4_TCR2:
		timer_nr ++;
	case SH4_TCR1:
		timer_nr ++;
	case SH4_TCR0:
		/*  Timer Control Register  */
		if (writeflag == MEM_READ) {
			odata = d->tcr[timer_nr];
		} else {
			if (cpu->cd.sh.pclock == 0) {
				fatal("INTERNAL ERROR: pclock must be set"
				    " for this machine. Aborting.\n");
				exit(1);
			}

			switch (idata & 3) {
			case TCR_TPSC_P4:
				d->timer_hz[timer_nr] = cpu->cd.sh.pclock/4.0;
				break;
			case TCR_TPSC_P16:
				d->timer_hz[timer_nr] = cpu->cd.sh.pclock/16.0;
				break;
			case TCR_TPSC_P64:
				d->timer_hz[timer_nr] = cpu->cd.sh.pclock/64.0;
				break;
			case TCR_TPSC_P256:
				d->timer_hz[timer_nr] = cpu->cd.sh.pclock/256.0;
				break;
			}

			debug("[ sh4 timer %i clock set to %f Hz ]\n",
			    timer_nr, d->timer_hz[timer_nr]);

			if (idata & (TCR_ICPF | TCR_ICPE1 | TCR_ICPE0 |
			    TCR_CKEG1 | TCR_CKEG0 | TCR_TPSC2)) {
				fatal("Unimplemented SH4 timer control"
				    " bits: 0x%08"PRIx32". Aborting.\n",
				    (int) idata);
				exit(1);
			}

			INTERRUPT_DEASSERT(d->timer_irq[timer_nr]);

			if (d->tcr[timer_nr] & TCR_UNF && !(idata & TCR_UNF)) {
				if (d->timer_interrupts_pending[timer_nr] > 0)
					d->timer_interrupts_pending[timer_nr]--;
			}

			d->tcr[timer_nr] = idata;
		}
		break;


	/*************************************************/
	/*  DMAC: DMA Controller                         */
	/*  4 channels on SH7750                         */
	/*  8 channels on SH7760                         */

	case SH4_SAR7:	dma_channel ++;
	case SH4_SAR6:	dma_channel ++;
	case SH4_SAR5:	dma_channel ++;
	case SH4_SAR4:	dma_channel ++;
	case SH4_SAR3:	dma_channel ++;
	case SH4_SAR2:	dma_channel ++;
	case SH4_SAR1:	dma_channel ++;
	case SH4_SAR0:	dma_channel ++;
		if (writeflag == MEM_READ)
			odata = cpu->cd.sh.dmac_sar[dma_channel];
		else
			cpu->cd.sh.dmac_sar[dma_channel] = idata;
		break;

	case SH4_DAR7:	dma_channel ++;
	case SH4_DAR6:	dma_channel ++;
	case SH4_DAR5:	dma_channel ++;
	case SH4_DAR4:	dma_channel ++;
	case SH4_DAR3:	dma_channel ++;
	case SH4_DAR2:	dma_channel ++;
	case SH4_DAR1:	dma_channel ++;
	case SH4_DAR0:	dma_channel ++;
		if (writeflag == MEM_READ)
			odata = cpu->cd.sh.dmac_dar[dma_channel];
		else
			cpu->cd.sh.dmac_dar[dma_channel] = idata;
		break;

	case SH4_DMATCR7: dma_channel ++;
	case SH4_DMATCR6: dma_channel ++;
	case SH4_DMATCR5: dma_channel ++;
	case SH4_DMATCR4: dma_channel ++;
	case SH4_DMATCR3: dma_channel ++;
	case SH4_DMATCR2: dma_channel ++;
	case SH4_DMATCR1: dma_channel ++;
	case SH4_DMATCR0: dma_channel ++;
		if (writeflag == MEM_READ)
			odata = cpu->cd.sh.dmac_tcr[dma_channel] & 0x00ffffff;
		else {
			if (idata & ~0x00ffffff) {
				fatal("[ SH4 DMA: Attempt to set top 8 "
				    "bits of the count register? 0x%08"
				    PRIx32" ]\n", (uint32_t) idata);
				exit(1);
			}

			cpu->cd.sh.dmac_tcr[dma_channel] = idata;
		}
		break;

	case SH4_CHCR7:	dma_channel ++;
	case SH4_CHCR6:	dma_channel ++;
	case SH4_CHCR5:	dma_channel ++;
	case SH4_CHCR4:	dma_channel ++;
	case SH4_CHCR3:	dma_channel ++;
	case SH4_CHCR2:	dma_channel ++;
	case SH4_CHCR1:	dma_channel ++;
	case SH4_CHCR0:	dma_channel ++;
		if (writeflag == MEM_READ) {
			odata = cpu->cd.sh.dmac_chcr[dma_channel];
		} else {
			/*  CHCR_CHSET always reads back as 0:  */
			idata &= ~CHCR_CHSET;

			cpu->cd.sh.dmac_chcr[dma_channel] = idata;

			/*  Perform a transfer?  */
			if (idata & CHCR_TD)
				sh4_dmac_transfer(cpu, d, dma_channel);
		}
		break;


	/*************************************************/
	/*  BSC: Bus State Controller                    */

	case SH4_BCR1:
		if (writeflag == MEM_WRITE)
			d->bsc_bcr1 = idata & 0x033efffd;
		else {
			odata = d->bsc_bcr1;
			if (cpu->byte_order == EMUL_LITTLE_ENDIAN)
				odata |= BCR1_LITTLE_ENDIAN;
		}
		break;

	case SH4_BCR2:
		if (len != sizeof(uint16_t)) {
			fatal("Non-16-bit SH4_BCR2 access?\n");
			exit(1);
		}
		if (writeflag == MEM_WRITE)
			d->bsc_bcr2 = idata & 0x3ffd;
		else
			odata = d->bsc_bcr2;
		break;

	case SH4_WCR1:
		if (writeflag == MEM_WRITE)
			d->bsc_wcr1 = idata & 0x77777777;
		else
			odata = d->bsc_wcr1;
		break;

	case SH4_WCR2:
		if (writeflag == MEM_WRITE)
			d->bsc_wcr2 = idata & 0xfffeefff;
		else
			odata = d->bsc_wcr2;
		break;

	case SH4_MCR:
		if (writeflag == MEM_WRITE)
			d->bsc_mcr = idata & 0xf8bbffff;
		else
			odata = d->bsc_mcr;
		break;

	case SH4_RTCSR:
		/*
		 *  Refresh Time Control/Status Register. Called RTCSR in
		 *  NetBSD, but RTSCR in the SH7750 manual?
		 */
		if (writeflag == MEM_WRITE) {
			idata &= 0x00ff;
			if (idata & RTCSR_CMF) {
				idata = (idata & ~RTCSR_CMF)
				    | (d->bsc_rtcsr & RTCSR_CMF);
			}
			d->bsc_rtcsr = idata & 0x00ff;
		} else
			odata = d->bsc_rtcsr;
		break;

	case SH4_RTCOR:
		/*  Refresh Time Constant Register (8 bits):  */
		if (writeflag == MEM_WRITE)
			d->bsc_rtcor = idata & 0x00ff;
		else
			odata = d->bsc_rtcor & 0x00ff;
		break;

	case SH4_RFCR:
		/*  Refresh Count Register (10 bits):  */
		if (writeflag == MEM_WRITE)
			d->bsc_rfcr = idata & 0x03ff;
		else
			odata = d->bsc_rfcr & 0x03ff;
		break;


	/*******************************************/
	/*  GPIO:  General-purpose I/O controller  */

	case SH4_PCTRA:
		if (writeflag == MEM_WRITE)
			d->pctra = idata;
		else
			odata = d->pctra;
		break;

	case SH4_PDTRA:
		if (writeflag == MEM_WRITE) {
			debug("[ sh4: pdtra: write: TODO ]\n");
			d->pdtra = idata;
		} else {
			debug("[ sh4: pdtra: read: TODO ]\n");
			odata = d->pdtra;
odata = random();
		}
		break;

	case SH4_PCTRB:
		if (writeflag == MEM_WRITE)
			d->pctrb = idata;
		else
			odata = d->pctrb;
		break;

	case SH4_PDTRB:
		if (writeflag == MEM_WRITE) {
			debug("[ sh4: pdtrb: write: TODO ]\n");
			d->pdtrb = idata;
		} else {
			debug("[ sh4: pdtrb: read: TODO ]\n");
			odata = d->pdtrb;
		}
		break;


	/****************************/
	/*  SCI:  Serial Interface  */

	case SHREG_SCSPTR:
		odata = sh4_sci_access(d, cpu,
		    writeflag == MEM_WRITE? 1 : 0, idata);

		/*
		 *  TODO
		 *
		 *  Find out the REAL way to make OpenBSD/landisk 4.1 run
		 *  in a stable manner! This is a SUPER-UGLY HACK which
		 *  just side-steps the real bug.
		 *
		 *  NOTE:  Snapshots of OpenBSD/landisk _after_ 4.1 seem
		 *  to work WITHOUT this hack, but NOT with it!
		 */
		cpu->invalidate_translation_caches(cpu, 0, INVALIDATE_ALL);

		break;


	/*********************************/
	/*  INTC:  Interrupt Controller  */

	case SH4_ICR:
		if (writeflag == MEM_WRITE) {
			if (idata & 0x80) {
				fatal("SH4 INTC: IRLM not yet "
				    "supported. TODO\n");
				exit(1);
			}
		}
		break;

	case SH4_IPRA:
		if (writeflag == MEM_READ)
			odata = cpu->cd.sh.intc_ipra;
		else {
			cpu->cd.sh.intc_ipra = idata;
			sh_update_interrupt_priorities(cpu);
		}
		break;

	case SH4_IPRB:
		if (writeflag == MEM_READ)
			odata = cpu->cd.sh.intc_iprb;
		else {
			cpu->cd.sh.intc_iprb = idata;
			sh_update_interrupt_priorities(cpu);
		}
		break;

	case SH4_IPRC:
		if (writeflag == MEM_READ)
			odata = cpu->cd.sh.intc_iprc;
		else {
			cpu->cd.sh.intc_iprc = idata;
			sh_update_interrupt_priorities(cpu);
		}
		break;

	case SH4_IPRD:
		if (writeflag == MEM_READ)
			odata = cpu->cd.sh.intc_iprd;
		else {
			cpu->cd.sh.intc_iprd = idata;
			sh_update_interrupt_priorities(cpu);
		}
		break;

	case SH4_INTPRI00:
		if (writeflag == MEM_READ)
			odata = cpu->cd.sh.intc_intpri00;
		else {
			cpu->cd.sh.intc_intpri00 = idata;
			sh_update_interrupt_priorities(cpu);
		}
		break;

	case SH4_INTPRI00 + 4:
		if (writeflag == MEM_READ)
			odata = cpu->cd.sh.intc_intpri04;
		else {
			cpu->cd.sh.intc_intpri04 = idata;
			sh_update_interrupt_priorities(cpu);
		}
		break;

	case SH4_INTPRI00 + 8:
		if (writeflag == MEM_READ)
			odata = cpu->cd.sh.intc_intpri08;
		else {
			cpu->cd.sh.intc_intpri08 = idata;
			sh_update_interrupt_priorities(cpu);
		}
		break;

	case SH4_INTPRI00 + 0xc:
		if (writeflag == MEM_READ)
			odata = cpu->cd.sh.intc_intpri0c;
		else {
			cpu->cd.sh.intc_intpri0c = idata;
			sh_update_interrupt_priorities(cpu);
		}
		break;

	case SH4_INTMSK00:
		/*  Note: Writes can only set bits, not clear them.  */
		if (writeflag == MEM_READ)
			odata = cpu->cd.sh.intc_intmsk00;
		else
			cpu->cd.sh.intc_intmsk00 |= idata;
		break;

	case SH4_INTMSK00 + 4:
		/*  Note: Writes can only set bits, not clear them.  */
		if (writeflag == MEM_READ)
			odata = cpu->cd.sh.intc_intmsk04;
		else
			cpu->cd.sh.intc_intmsk04 |= idata;
		break;

	case SH4_INTMSKCLR00:
		/*  Note: Writes can only clear bits, not set them.  */
		if (writeflag == MEM_WRITE)
			cpu->cd.sh.intc_intmsk00 &= ~idata;
		break;

	case SH4_INTMSKCLR00 + 4:
		/*  Note: Writes can only clear bits, not set them.  */
		if (writeflag == MEM_WRITE)
			cpu->cd.sh.intc_intmsk04 &= ~idata;
		break;


	/*************************************************/
	/*  SCIF: Serial Controller Interface with FIFO  */

	case SH4_SCIF_BASE + SCIF_SMR:
		if (writeflag == MEM_WRITE) {
			d->scif_smr = idata;
		} else {
			odata = d->scif_smr;
		}
		break;

	case SH4_SCIF_BASE + SCIF_BRR:
		if (writeflag == MEM_WRITE) {
			d->scif_brr = idata;
		} else {
			odata = d->scif_brr;
		}
		break;

	case SH4_SCIF_BASE + SCIF_SCR:
		if (writeflag == MEM_WRITE) {
			d->scif_scr = idata;
			scif_reassert_interrupts(d);
		} else {
			odata = d->scif_scr;
		}
		break;

	case SH4_SCIF_BASE + SCIF_FTDR:
		if (writeflag == MEM_WRITE) {
			/*  Add to TX fifo:  */
			if (d->scif_tx_fifo_cursize >=
			    sizeof(d->scif_tx_fifo)) {
				fatal("[ SCIF TX fifo overrun! ]\n");
				d->scif_tx_fifo_cursize = 0;
			}

			d->scif_tx_fifo[d->scif_tx_fifo_cursize++] = idata;
			d->scif_delayed_tx = SCIF_DELAYED_TX_VALUE;
		}
		break;

	case SH4_SCIF_BASE + SCIF_SSR:
		if (writeflag == MEM_READ) {
			odata = d->scif_ssr;
		} else {
			d->scif_ssr = idata;
			scif_reassert_interrupts(d);
		}
		break;

	case SH4_SCIF_BASE + SCIF_FRDR:
		{
			int x = console_readchar(d->scif_console_handle);
			if (x == 13)
				x = 10;
			odata = x < 0? 0 : x;
			if (console_charavail(d->scif_console_handle))
				d->scif_ssr |= SCSSR2_DR;
			else
				d->scif_ssr &= ~SCSSR2_DR;
			scif_reassert_interrupts(d);
		}
		break;

	case SH4_SCIF_BASE + SCIF_FCR:
		if (writeflag == MEM_WRITE) {
			d->scif_fcr = idata;
		} else {
			odata = d->scif_fcr;
		}
		break;

	case SH4_SCIF_BASE + SCIF_LSR:
		/*  TODO: Implement all bits.  */
		odata = 0;
		break;

	case SH4_SCIF_BASE + SCIF_FDR:
		/*  Nr of bytes in the TX and RX fifos, respectively:  */
		odata = (console_charavail(d->scif_console_handle)? 1 : 0)
		    + (d->scif_tx_fifo_cursize << 8);
		break;


	/*************************************************/

	case SH4_RSECCNT:
	case SH4_RMINCNT:
	case SH4_RHRCNT:
	case SH4_RWKCNT:
	case SH4_RDAYCNT:
	case SH4_RMONCNT:
	case SH4_RYRCNT:
	case SH4_RSECAR:
	case SH4_RMINAR:
	case SH4_RHRAR:
	case SH4_RWKAR:
	case SH4_RDAYAR:
	case SH4_RMONAR:
		if (writeflag == MEM_WRITE) {
			d->rtc_reg[(relative_addr - 0xffc80000) / 4] = idata;
		} else {
			/*  TODO: Update rtc_reg based on host's date/time.  */
			odata = d->rtc_reg[(relative_addr - 0xffc80000) / 4];
		}
		break;

	case SH4_RCR1:
		if (writeflag == MEM_READ)
			odata = d->rtc_rcr1;
		else {
			d->rtc_rcr1 = idata;
			if (idata & 0x18) {
				fatal("SH4: TODO: RTC interrupt enable\n");
				exit(1);
			}
		}
		break;


	/*************************************************/

	default:if (writeflag == MEM_READ) {
			fatal("[ sh4: read from addr 0x%x ]\n",
			    (int)relative_addr);
		} else {
			fatal("[ sh4: write to addr 0x%x: 0x%x ]\n",
			    (int)relative_addr, (int)idata);
		}
#ifdef SH4_DEGUG
		/*  exit(1);  */
#endif
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(sh4)
{
	char tmp[200], n[200];
	int i;
	struct machine *machine = devinit->machine;
	struct sh4_data *d;

	CHECK_ALLOCATION(d = (struct sh4_data *) malloc(sizeof(struct sh4_data)));
	memset(d, 0, sizeof(struct sh4_data));


	/*
	 *  Main SH4 device, and misc memory stuff:
	 */

	memory_device_register(machine->memory, devinit->name,
	    SH4_REG_BASE, 0x01000000, dev_sh4_access, d, DM_DEFAULT, NULL);

	/*  On-chip RAM/cache:  */
	dev_ram_init(machine, 0x1e000000, 0x8000, DEV_RAM_RAM, 0x0);

	/*  0xe0000000: Store queues:  */
	memory_device_register(machine->memory, "sh4_sq",
	    0xe0000000, 0x04000000, dev_sh4_sq_access, d, DM_DEFAULT, NULL);


	/*
	 *  SCIF (Serial console):
	 */

	d->scif_console_handle = console_start_slave(devinit->machine,
	    "SH4 SCIF", 1);

	snprintf(tmp, sizeof(tmp), "%s.irq[0x%x]",
	    devinit->interrupt_path, SH4_INTEVT_SCIF_RXI);
	INTERRUPT_CONNECT(tmp, d->scif_rx_irq);
	snprintf(tmp, sizeof(tmp), "%s.irq[0x%x]",
	    devinit->interrupt_path, SH4_INTEVT_SCIF_TXI);
	INTERRUPT_CONNECT(tmp, d->scif_tx_irq);


	/*
	 *  Caches (fake):
 	 *
	 *  0xf0000000	SH4_CCIA	I-Cache address array
	 *  0xf1000000	SH4_CCID	I-Cache data array
	 *  0xf4000000	SH4_CCDA	D-Cache address array
	 *  0xf5000000	SH4_CCDD	D-Cache data array
	 *
	 *  TODO: Implement more correct cache behaviour?
	 */

	dev_ram_init(machine, SH4_CCIA, SH4_ICACHE_SIZE * 2, DEV_RAM_RAM, 0x0);
	dev_ram_init(machine, SH4_CCID, SH4_ICACHE_SIZE,     DEV_RAM_RAM, 0x0);
	dev_ram_init(machine, SH4_CCDA, SH4_DCACHE_SIZE * 2, DEV_RAM_RAM, 0x0);
	dev_ram_init(machine, SH4_CCDD, SH4_DCACHE_SIZE,     DEV_RAM_RAM, 0x0);

	/*  0xf2000000	SH4_ITLB_AA  */
	memory_device_register(machine->memory, "sh4_itlb_aa", SH4_ITLB_AA,
	    0x01000000, dev_sh4_itlb_aa_access, d, DM_DEFAULT, NULL);

	/*  0xf3000000	SH4_ITLB_DA1  */
	memory_device_register(machine->memory, "sh4_itlb_da1", SH4_ITLB_DA1,
	    0x01000000, dev_sh4_itlb_da1_access, d, DM_DEFAULT, NULL);

	/*  0xf6000000	SH4_UTLB_AA  */
	memory_device_register(machine->memory, "sh4_utlb_aa", SH4_UTLB_AA,
	    0x01000000, dev_sh4_utlb_aa_access, d, DM_DEFAULT, NULL);

	/*  0xf7000000	SH4_UTLB_DA1  */
	memory_device_register(machine->memory, "sh4_utlb_da1", SH4_UTLB_DA1,
	    0x01000000, dev_sh4_utlb_da1_access, d, DM_DEFAULT, NULL);


	/*
	 *  PCIC (PCI controller) at 0xfe200000:
	 */

	memory_device_register(machine->memory, "sh4_pcic", SH4_PCIC,
	    N_PCIC_REGS * sizeof(uint32_t), dev_sh4_pcic_access, d,
	    DM_DEFAULT, NULL);

	/*  Initial PCI control register contents:  */
	d->bsc_bcr2 = BCR2_PORTEN;
	d->pcic_reg[PCIC_REG(SH4_PCICONF2)] = PCI_CLASS_CODE(PCI_CLASS_BRIDGE,
	    PCI_SUBCLASS_BRIDGE_HOST, 0);

	/*  Register 16 PCIC interrupts:  */
	for (i=0; i<N_PCIC_IRQS; i++) {
		struct interrupt templ;
		snprintf(n, sizeof(n), "%s.pcic.%i",
		    devinit->interrupt_path, i);
		memset(&templ, 0, sizeof(templ));
		templ.line = i;
		templ.name = n;
		templ.extra = d;
		templ.interrupt_assert = sh4_pcic_interrupt_assert;
		templ.interrupt_deassert = sh4_pcic_interrupt_deassert;
		interrupt_handler_register(&templ);

		snprintf(tmp, sizeof(tmp), "%s.irq[0x%x]",
		    devinit->interrupt_path, SH4_INTEVT_IRQ0 + 0x20 * i);
		INTERRUPT_CONNECT(tmp, d->cpu_pcic_interrupt[i]);
	}

	/*  Register the PCI bus:  */
	snprintf(tmp, sizeof(tmp), "%s.pcic", devinit->interrupt_path);
	d->pci_data = bus_pci_init(
	    devinit->machine,
	    tmp,			/*  pciirq  */
	    0,				/*  pci device io offset  */
	    0,				/*  pci device mem offset  */
	    SH4_PCIC_IO,		/*  PCI portbase  */
	    SH4_PCIC_MEM,		/*  PCI membase  */
	    tmp,			/*  PCI irqbase  */
	    0x00000000,			/*  ISA portbase  */
	    0x00000000,			/*  ISA membase  */
	    "TODOisaIrqBase");		/*  ISA irqbase  */

	/*  Return PCI bus pointer, to allow per-machine devices
	    to be added later:  */
	devinit->return_ptr = d->pci_data;


	/*
	 *  Timer:
	 */

	d->sh4_timer = timer_add(SH4_PSEUDO_TIMER_HZ, sh4_timer_tick, d);
	machine_add_tickfunction(devinit->machine, dev_sh4_tick, d,
	    SH4_TICK_SHIFT);

	/*  Initial Timer values, according to the SH7750 manual:  */
	d->tcor[0] = 0xffffffff; d->tcnt[0] = 0xffffffff;
	d->tcor[1] = 0xffffffff; d->tcnt[1] = 0xffffffff;
	d->tcor[2] = 0xffffffff; d->tcnt[2] = 0xffffffff;

	snprintf(tmp, sizeof(tmp), "machine[0].cpu[0].irq[0x%x]",
	    SH_INTEVT_TMU0_TUNI0);
	if (!interrupt_handler_lookup(tmp, &d->timer_irq[0])) {
		fatal("Could not find interrupt '%s'.\n", tmp);
		exit(1);
	}
	snprintf(tmp, sizeof(tmp), "machine[0].cpu[0].irq[0x%x]",
	    SH_INTEVT_TMU1_TUNI1);
	if (!interrupt_handler_lookup(tmp, &d->timer_irq[1])) {
		fatal("Could not find interrupt '%s'.\n", tmp);
		exit(1);
	}
	snprintf(tmp, sizeof(tmp), "machine[0].cpu[0].irq[0x%x]",
	    SH_INTEVT_TMU2_TUNI2);
	if (!interrupt_handler_lookup(tmp, &d->timer_irq[2])) {
		fatal("Could not find interrupt '%s'.\n", tmp);
		exit(1);
	}


	/*
	 *  Bus State Controller initial values, according to the
	 *  SH7760 manual:
	 */

	d->bsc_bcr2 = 0x3ffc;
	d->bsc_wcr1 = 0x77777777;
	d->bsc_wcr2 = 0xfffeefff;


	return 1;
}

