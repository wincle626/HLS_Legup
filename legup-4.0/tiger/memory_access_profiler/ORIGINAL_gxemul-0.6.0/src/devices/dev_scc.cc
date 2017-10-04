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
 *  COMMENT: Serial controller used in some DECsystem and SGI machines
 *
 *  Hm... Same as Z8530? Most of the code in here is written for DECsystem
 *  emulation, though.
 *
 *  NOTE:
 *	Each scc device is responsible for two lines; the first scc device
 *	controls mouse (0) and keyboard (1), and the second device controls
 *	serial ports (2 and 3).
 *
 *  TODO:
 *	Mouse support!!!  (scc0 and scc1 need to cooperate, in order to
 *		emulate the same lk201 behaviour as when using the dc device)
 *	DMA
 *	More correct interrupt support.
 *
 ******************************************************************************
 *	 _____ ___  ____   ___  _ 
 *	|_   _/ _ \|  _ \ / _ \| |
 *	  | || | | | | | | | | | |
 *	  | || |_| | |_| | |_| |_|
 *	  |_| \___/|____/ \___/(_)
 *
 *	Since this is actually a Z8530, it should be merged with dev_z8530.c!
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

#include "thirdparty/sccreg.h"


#define	SCC_TICK_SHIFT		14

#define	N_SCC_PORTS		2
#define	N_SCC_REGS		16
#define	MAX_QUEUE_LEN		1024

/*  #define SCC_DEBUG  */


struct scc_data {
	int		irq_nr;
	int		use_fb;
	int		console_handle;

	int		scc_nr;
	int		addrmul;

	int		register_select_in_progress[N_SCC_PORTS];
	int		register_selected[N_SCC_PORTS];

	unsigned char	scc_register_r[N_SCC_PORTS * N_SCC_REGS];
	unsigned char	scc_register_w[N_SCC_PORTS * N_SCC_REGS];

	unsigned char	rx_queue_char[N_SCC_PORTS * MAX_QUEUE_LEN];
	int		cur_rx_queue_pos_write[N_SCC_PORTS];
	int		cur_rx_queue_pos_read[N_SCC_PORTS];

	struct lk201_data lk201;
};


/*
 *  dev_scc_add_to_rx_queue():
 *
 *  Add a character to the receive queue.
 */
void dev_scc_add_to_rx_queue(void *e, int ch, int portnr)
{ 
	struct scc_data *d = (struct scc_data *) e;
	int scc_nr;

	/*  DC's keyboard port ==> SCC keyboard port  */
	if (portnr == 0)
		portnr = 3;

	scc_nr = portnr / N_SCC_PORTS;
	if (scc_nr != d->scc_nr)
		return;

	portnr &= (N_SCC_PORTS - 1);

        d->rx_queue_char[portnr * MAX_QUEUE_LEN +
	    d->cur_rx_queue_pos_write[portnr]] = ch; 
        d->cur_rx_queue_pos_write[portnr] ++;
        if (d->cur_rx_queue_pos_write[portnr] == MAX_QUEUE_LEN)
                d->cur_rx_queue_pos_write[portnr] = 0;

        if (d->cur_rx_queue_pos_write[portnr] ==
	    d->cur_rx_queue_pos_read[portnr])
                fatal("warning: add_to_rx_queue(): rx_queue overrun!\n");
}


static int rx_avail(struct scc_data *d, int portnr)
{
	return d->cur_rx_queue_pos_write[portnr] !=
	    d->cur_rx_queue_pos_read[portnr];
}


static unsigned char rx_nextchar(struct scc_data *d, int portnr)
{
	unsigned char ch;
	ch = d->rx_queue_char[portnr * MAX_QUEUE_LEN +
	    d->cur_rx_queue_pos_read[portnr]];
	d->cur_rx_queue_pos_read[portnr]++;
	if (d->cur_rx_queue_pos_read[portnr] == MAX_QUEUE_LEN)
		d->cur_rx_queue_pos_read[portnr] = 0;
	return ch;
}


DEVICE_TICK(scc)
{
	struct scc_data *d = (struct scc_data *) extra;
	int i;

	/*  Add keystrokes to the rx queue:  */
	if (d->use_fb == 0 && d->scc_nr == 1) {
		if (console_charavail(d->console_handle))
			dev_scc_add_to_rx_queue(extra, console_readchar(
			    d->console_handle), 2);
	}
	if (d->use_fb == 1 && d->scc_nr == 1)
		lk201_tick(cpu->machine, &d->lk201);

	for (i=0; i<N_SCC_PORTS; i++) {
		d->scc_register_r[i * N_SCC_REGS + SCC_RR0] |= SCC_RR0_TX_EMPTY;
		d->scc_register_r[i * N_SCC_REGS + SCC_RR1] = 0;
		    /*  No receive errors  */

		d->scc_register_r[i * N_SCC_REGS + SCC_RR0] &=
		    ~SCC_RR0_RX_AVAIL;
		if (rx_avail(d, i))
			d->scc_register_r[i * N_SCC_REGS + SCC_RR0] |=
			    SCC_RR0_RX_AVAIL;

		/*
		 *  Interrupts:
		 *  (NOTE: Interrupt enables are always at channel A)
		 */
		if (d->scc_register_w[N_SCC_REGS + SCC_WR9] &
		    SCC_WR9_MASTER_IE) {
			/*  TX interrupts?  */
			if (d->scc_register_w[i * N_SCC_REGS + SCC_WR1] &
			    SCC_WR1_TX_IE) {
				if (d->scc_register_r[i * N_SCC_REGS + SCC_RR3]
				    & SCC_RR3_TX_IP_A ||
				    d->scc_register_r[i * N_SCC_REGS + SCC_RR3]
				    & SCC_RR3_TX_IP_B) {
fatal("TODO: legacy rewrite!\n");
abort();
//					cpu_interrupt(cpu, d->irq_nr);
				}
			}

			/*  RX interrupts?  */
			if (d->scc_register_w[N_SCC_REGS + SCC_WR1] &
			    (SCC_WR1_RXI_FIRST_CHAR | SCC_WR1_RXI_ALL_CHAR)) {
				if (d->scc_register_r[i * N_SCC_REGS + SCC_RR0]
				    & SCC_RR0_RX_AVAIL) {
					if (i == SCC_CHANNEL_A)
						d->scc_register_r[N_SCC_REGS +
						    SCC_RR3] |= SCC_RR3_RX_IP_A;
					else
						d->scc_register_r[N_SCC_REGS +
						    SCC_RR3] |= SCC_RR3_RX_IP_B;
				}

				if (d->scc_register_r[i * N_SCC_REGS + SCC_RR3]
				    & SCC_RR3_RX_IP_A ||
				    d->scc_register_r[i * N_SCC_REGS + SCC_RR3]
				    & SCC_RR3_RX_IP_B) {
fatal("TODO: legacy rewrite!\n");
abort();
//					cpu_interrupt(cpu, d->irq_nr);
				}
			}

			if (d->scc_register_w[N_SCC_REGS + SCC_WR1] &
			    SCC_WR1_DMA_MODE) {
				if (d->scc_register_r[i * N_SCC_REGS + SCC_RR0]
				    & SCC_RR0_RX_AVAIL) {
					if (i == SCC_CHANNEL_A)
						d->scc_register_r[N_SCC_REGS +
						    SCC_RR3] |=
						    SCC_RR3_EXT_IP_A;
					else
						d->scc_register_r[N_SCC_REGS +
						    SCC_RR3] |=
						    SCC_RR3_EXT_IP_B;
				}

				if (d->scc_register_r[i * N_SCC_REGS + SCC_RR3]
				    & SCC_RR3_EXT_IP_A ||
				    d->scc_register_r[i * N_SCC_REGS + SCC_RR3]
				    & SCC_RR3_EXT_IP_B)
{
fatal("TODO: legacy rewrite!\n");
abort();
//					cpu_interrupt(cpu, d->irq_nr);
/*  TODO: huh?  */
//cpu_interrupt(cpu, 8 + 0x02000000);
}
			}
		}
	}
}


/*
 *  dev_scc_dma_func():
 */
int dev_scc_dma_func(struct cpu *cpu, void *extra, uint64_t addr,
	size_t dma_len, int tx)
{
	/*  printf("dev_scc_dma_func(): addr = %08x, len = %i\n",
	    (int)addr, (int)dma_len);  */
	unsigned char word[4];
	struct scc_data *d = (struct scc_data *) extra;
	int n;

	int port = SCC_CHANNEL_A;	/*  TODO  */

	if (tx) {
		do {
			cpu->memory_rw(cpu, cpu->mem, addr, &word[0],
			    sizeof(word), MEM_READ, NO_EXCEPTIONS | PHYSICAL);

			lk201_tx_data(&d->lk201, d->scc_nr * 2 + port, word[1]);
			/*  Loopback:  */
			if (d->scc_register_w[port * N_SCC_REGS + SCC_WR14]
			    & SCC_WR14_LOCAL_LOOPB)
				dev_scc_add_to_rx_queue(d, word[1],
				    d->scc_nr * 2 + port);

			addr += sizeof(word);
		} while ((addr & 0xffc) != 0);

		dev_scc_tick(cpu, extra);
		return 1;
	} else {
		printf("dev_scc_dma_func(): addr = %08x, len = %i\n",
		    (int)addr, (int)dma_len);


/*  TODO: all this is just nonsense  */

		n = 0;
		while (rx_avail(d, port)) {
			word[0] = word[1] = word[2] = word[3] = 0;
			word[0] = word[1] = word[2] = word[3] =
			    rx_nextchar(d, port);
			n++;
			cpu->memory_rw(cpu, cpu->mem, addr, &word[0],
			    sizeof(word), MEM_WRITE, NO_EXCEPTIONS | PHYSICAL);

			addr += sizeof(word);
			/*  Half-page?  */
			if ((addr & 0x7fc) == 0)
				break;
		}
		dev_scc_tick(cpu, extra);
		return n*4;
	}
}


DEVICE_ACCESS(scc)
{
	struct scc_data *d = (struct scc_data *) extra;
	uint64_t idata = 0, odata = 0;
	int port;
	int ultrix_mode = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	/*  relative_addr /= d->addrmul;  */
		/*  See SGI comment below instead.  */
	/*
	 *  SGI writes command to 0x0f, and data to 0x1f.
	 *  (TODO: This works for port nr 0, how about port nr 1?)
	 */
	if ((relative_addr & 0x0f) == 0xf) {
		if (relative_addr == 0x0f)
			relative_addr = 1;
		else
			relative_addr = 5;
	}

	port = relative_addr / 8;
	relative_addr &= 7;

	dev_scc_tick(cpu, extra);

	/*
	 *  Ultrix writes words such as 0x1200 to relative address 0,
	 *  instead of writing the byte 0x12 directly to address 1.
	 */
	if ((relative_addr == 0 || relative_addr == 4) && (idata & 0xff) == 0) {
		ultrix_mode = 1;
		relative_addr ++;
		idata >>= 8;
	}

	switch (relative_addr) {
	case 1:		/*  command  */
		if (writeflag==MEM_READ) {
			odata = d->scc_register_r[port * N_SCC_REGS +
			    d->register_selected[port]];

			if (d->register_selected[port] == SCC_RR3) {
				if (port == SCC_CHANNEL_B)
					fatal("WARNING! scc channel B has "
					    "no RR3\n");

				d->scc_register_r[port * N_SCC_REGS +
				    SCC_RR3] = 0;

fatal("TODO: legacy rewrite!\n");
abort();
//				cpu_interrupt_ack(cpu, d->irq_nr);
			}

#ifdef SCC_DEBUG
			fatal("[ scc: port %i, register %i, read value "
			    "0x%02x ]\n", port, d->register_selected[port],
			    (int)odata);
#endif
			d->register_select_in_progress[port] = 0;
			d->register_selected[port] = 0;
			/*  debug("[ scc: (port %i) read from 0x%08lx ]\n",
			    port, (long)relative_addr);  */
		} else {
			/*  If no register is selected, then select one.
			    Otherwise, write to the selected register.  */
			if (d->register_select_in_progress[port] == 0) {
				d->register_select_in_progress[port] = 1;
				d->register_selected[port] = idata;
				d->register_selected[port] &= (N_SCC_REGS-1);
			} else {
				d->scc_register_w[port * N_SCC_REGS +
				    d->register_selected[port]] = idata;
#ifdef SCC_DEBUG
				fatal("[ scc: port %i, register %i, write "
				    "value 0x%02x ]\n", port,
				    d->register_selected[port], idata);
#endif

				d->scc_register_r[port * N_SCC_REGS +
				    SCC_RR12] = d->scc_register_w[port *
				    N_SCC_REGS + SCC_WR12];
				d->scc_register_r[port * N_SCC_REGS +
				    SCC_RR13] = d->scc_register_w[port *
				    N_SCC_REGS + SCC_WR13];

				d->register_select_in_progress[port] = 0;
				d->register_selected[port] = 0;
			}
		}
		break;
	case 5:		/*  data  */
		if (writeflag==MEM_READ) {
			if (rx_avail(d, port))
				odata = rx_nextchar(d, port);

			/*  TODO:  perhaps only clear the RX part of RR3?  */
			d->scc_register_r[N_SCC_REGS + SCC_RR3] = 0;

fatal("TODO: legacy rewrite!\n");
abort();
//			cpu_interrupt_ack(cpu, d->irq_nr);

//			debug("[ scc: (port %i) read from 0x%08lx: 0x%02x ]\n",
//			    port, (long)relative_addr, (int)odata);
		} else {
			/*  debug("[ scc: (port %i) write to  0x%08lx: "
			    "0x%08x ]\n", port, (long)relative_addr,
			    (int)idata);  */

			/*  Send the character:  */
			lk201_tx_data(&d->lk201, d->scc_nr * 2 + port, idata);

			/*  Loopback:  */
			if (d->scc_register_w[port * N_SCC_REGS + SCC_WR14]
			    & SCC_WR14_LOCAL_LOOPB)
				dev_scc_add_to_rx_queue(d, idata, d->scc_nr
				    * 2 + port);

			/*  TX interrupt:  */
			if (d->scc_register_w[port * N_SCC_REGS + SCC_WR9] &
			    SCC_WR9_MASTER_IE &&
			    d->scc_register_w[port * N_SCC_REGS + SCC_WR1] &
			    SCC_WR1_TX_IE) {
				if (port == SCC_CHANNEL_A)
					d->scc_register_r[N_SCC_REGS + SCC_RR3]
					    |= SCC_RR3_TX_IP_A;
				else
					d->scc_register_r[N_SCC_REGS + SCC_RR3]
					    |= SCC_RR3_TX_IP_B;
			}

			dev_scc_tick(cpu, extra);
		}
		break;
	default:
		if (writeflag==MEM_READ) {
			debug("[ scc: (port %i) read from 0x%08lx ]\n",
			    port, (long)relative_addr);
		} else {
			debug("[ scc: (port %i) write to  0x%08lx: 0x%08x ]\n",
			    port, (long)relative_addr, (int)idata);
		}
	}

	if (ultrix_mode && writeflag == MEM_READ) {
		odata <<= 8;
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


/*
 *  dev_scc_init():
 *
 *	use_fb = non-zero when using graphical console + keyboard
 *	scc_nr = 0 or 1
 *	addmul = 1 in most cases, 8 on SGI?
 */
void *dev_scc_init(struct machine *machine, struct memory *mem,
	uint64_t baseaddr, int irq_nr, int use_fb, int scc_nr, int addrmul)
{
	struct scc_data *d;

	CHECK_ALLOCATION(d = (struct scc_data *) malloc(sizeof(struct scc_data)));
	memset(d, 0, sizeof(struct scc_data));

	d->irq_nr  = irq_nr;
	d->scc_nr  = scc_nr;
	d->use_fb  = use_fb;
	d->addrmul = addrmul;
	d->console_handle = console_start_slave(machine, "SCC", 1);

	lk201_init(&d->lk201, use_fb, dev_scc_add_to_rx_queue,
	    d->console_handle, d);

	memory_device_register(mem, "scc", baseaddr, DEV_SCC_LENGTH,
	    dev_scc_access, d, DM_DEFAULT, NULL);
	machine_add_tickfunction(machine, dev_scc_tick, d, SCC_TICK_SHIFT);

	return (void *) d;
}

