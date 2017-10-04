/*
 *  Copyright (C) 2005-2009  Anders Gavare.  All rights reserved.
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
 *  COMMENT: DEC 21143 "Tulip" ethernet controller
 *
 *  Implemented from Intel document 278074-001 ("21143 PC/CardBus 10/100Mb/s
 *  Ethernet LAN Controller") and by reverse-engineering OpenBSD and NetBSD
 *  sources.
 *
 *  This device emulates several sub-components:
 *
 *	21143:	This is the actual ethernet controller.
 *
 *	MII:	The "physical" network interface.
 *
 *	SROM:	A ROM area containing setting such as which MAC address to
 *		use, and info about the MII.
 *
 *
 *  TODO:
 *	o)  Handle _writes_ to MII registers.
 *	o)  Make it work with modern Linux kernels (as a guest OS).
 *	o)  Endianness for descriptors? If necessary.
 *	o)  Actually handle the "Setup" packet.
 *	o)  MAC filtering on incoming packets.
 *	o)  Don't hardcode as many values.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "devices.h"
#include "emul.h"
#include "interrupt.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"
#include "net.h"

#include "thirdparty/mii.h"
#include "thirdparty/tulipreg.h"


/*  #define debug fatal  */

#define	DEC21143_TICK_SHIFT		16

#define	N_REGS			32
#define	ROM_WIDTH		6

struct dec21143_data {
	struct interrupt irq;
	int		irq_was_asserted;

	/*  PCI:  */
	int		pci_little_endian;

	/*  Ethernet address, and a network which we are connected to:  */
	uint8_t		mac[6];
	struct net	*net;

	/*  SROM emulation:  */
	uint8_t		srom[1 << (ROM_WIDTH + 1)];
	int		srom_curbit;
	int		srom_opcode;
	int		srom_opcode_has_started;
	int		srom_addr;

	/*  MII PHY emulation:  */
	uint16_t	mii_phy_reg[MII_NPHY * 32];
	int		mii_state;
	int		mii_bit;
	int		mii_opcode;
	int		mii_phyaddr;
	int		mii_regaddr;

	/*  21143 registers:  */
	uint32_t	reg[N_REGS];

	/*  Internal TX state:  */
	uint64_t	cur_tx_addr;
	unsigned char	*cur_tx_buf;
	int		cur_tx_buf_len;
	int		tx_idling;
	int		tx_idling_threshold;

	/*  Internal RX state:  */
	uint64_t	cur_rx_addr;
	unsigned char	*cur_rx_buf;
	int		cur_rx_buf_len;
	int		cur_rx_offset;
};


/*  Internal states during MII data stream decode:  */
#define	MII_STATE_RESET				0
#define	MII_STATE_START_WAIT			1
#define	MII_STATE_READ_OP			2
#define	MII_STATE_READ_PHYADDR_REGADDR		3
#define	MII_STATE_A				4
#define	MII_STATE_D				5
#define	MII_STATE_IDLE				6


/*
 *  dec21143_rx():
 *
 *  Receive a packet. (If there is no current packet, then check for newly
 *  arrived ones. If the current packet couldn't be fully transfered the
 *  last time, then continue on that packet.)
 */
int dec21143_rx(struct cpu *cpu, struct dec21143_data *d)
{
	uint64_t addr = d->cur_rx_addr, bufaddr;
	unsigned char descr[16];
	uint32_t rdes0, rdes1, rdes2, rdes3;
	int bufsize, buf1_size, buf2_size, i, writeback_len = 4, to_xfer;

	/*  No current packet? Then check for new ones.  */
	if (d->cur_rx_buf == NULL) {
		/*  Nothing available? Then abort.  */
		if (!net_ethernet_rx_avail(d->net, d))
			return 0;

		/*  Get the next packet into our buffer:  */
		net_ethernet_rx(d->net, d, &d->cur_rx_buf,
		    &d->cur_rx_buf_len);

		/*  Append a 4 byte CRC:  */
		d->cur_rx_buf_len += 4;
		CHECK_ALLOCATION(d->cur_rx_buf = (unsigned char *) realloc(d->cur_rx_buf,
		    d->cur_rx_buf_len));

		/*  Well... the CRC is just zeros, for now.  */
		memset(d->cur_rx_buf + d->cur_rx_buf_len - 4, 0, 4);

		d->cur_rx_offset = 0;
	}

	/*  fatal("{ dec21143_rx: base = 0x%08x }\n", (int)addr);  */
	addr &= 0x7fffffff;

	if (!cpu->memory_rw(cpu, cpu->mem, addr, descr, sizeof(uint32_t),
	    MEM_READ, PHYSICAL | NO_EXCEPTIONS)) {
		fatal("[ dec21143_rx: memory_rw failed! ]\n");
		return 0;
	}

	rdes0 = descr[0] + (descr[1]<<8) + (descr[2]<<16) + (descr[3]<<24);

	/*  Only use descriptors owned by the 21143:  */
	if (!(rdes0 & TDSTAT_OWN)) {
		d->reg[CSR_STATUS/8] |= STATUS_RU;
		return 0;
	}

	if (!cpu->memory_rw(cpu, cpu->mem, addr + sizeof(uint32_t), descr +
	    sizeof(uint32_t), sizeof(uint32_t) * 3, MEM_READ, PHYSICAL |
	    NO_EXCEPTIONS)) {
		fatal("[ dec21143_rx: memory_rw failed! ]\n");
		return 0;
	}

	rdes1 = descr[4] + (descr[5]<<8) + (descr[6]<<16) + (descr[7]<<24);
	rdes2 = descr[8] + (descr[9]<<8) + (descr[10]<<16) + (descr[11]<<24);
	rdes3 = descr[12] + (descr[13]<<8) + (descr[14]<<16) + (descr[15]<<24);

	buf1_size = rdes1 & TDCTL_SIZE1;
	buf2_size = (rdes1 & TDCTL_SIZE2) >> TDCTL_SIZE2_SHIFT;
	bufaddr = buf1_size? rdes2 : rdes3;
	bufsize = buf1_size? buf1_size : buf2_size;

	d->reg[CSR_STATUS/8] &= ~STATUS_RS;

	if (rdes1 & TDCTL_ER)
		d->cur_rx_addr = d->reg[CSR_RXLIST / 8];
	else {
		if (rdes1 & TDCTL_CH)
			d->cur_rx_addr = rdes3;
		else
			d->cur_rx_addr += 4 * sizeof(uint32_t);
	}

	debug("{ RX (%llx): 0x%08x 0x%08x 0x%x 0x%x: buf %i bytes at 0x%x }\n",
	    (long long)addr, rdes0, rdes1, rdes2, rdes3, bufsize, (int)bufaddr);
	bufaddr &= 0x7fffffff;

	/*  Turn off all status bits, and give up ownership:  */
	rdes0 = 0x00000000;

	to_xfer = d->cur_rx_buf_len - d->cur_rx_offset;
	if (to_xfer > bufsize)
		to_xfer = bufsize;

	/*  DMA bytes from the packet into emulated physical memory:  */
	for (i=0; i<to_xfer; i++) {
		cpu->memory_rw(cpu, cpu->mem, bufaddr + i,
		    d->cur_rx_buf + d->cur_rx_offset + i, 1, MEM_WRITE,
		    PHYSICAL | NO_EXCEPTIONS);
		/*  fatal(" %02x", d->cur_rx_buf[d->cur_rx_offset + i]);  */
	}

	/*  Was this the first buffer in a frame? Then mark it as such.  */
	if (d->cur_rx_offset == 0)
		rdes0 |= TDSTAT_Rx_FS;

	d->cur_rx_offset += to_xfer;

	/*  Frame completed?  */
	if (d->cur_rx_offset >= d->cur_rx_buf_len) {
		rdes0 |= TDSTAT_Rx_LS;

		/*  Set the frame length:  */
		rdes0 |= (d->cur_rx_buf_len << 16) & TDSTAT_Rx_FL;

		/*  Frame too long? (1518 is max ethernet frame length)  */
		if (d->cur_rx_buf_len > 1518)
			rdes0 |= TDSTAT_Rx_TL;

		/*  Cause a receiver interrupt:  */
		d->reg[CSR_STATUS/8] |= STATUS_RI;

		free(d->cur_rx_buf);
		d->cur_rx_buf = NULL;
		d->cur_rx_buf_len = 0;
	}

	/*  Descriptor writeback:  */
	descr[ 0] = rdes0;       descr[ 1] = rdes0 >> 8;
	descr[ 2] = rdes0 >> 16; descr[ 3] = rdes0 >> 24;
	if (writeback_len > 1) {
		descr[ 4] = rdes1;       descr[ 5] = rdes1 >> 8;
		descr[ 6] = rdes1 >> 16; descr[ 7] = rdes1 >> 24;
		descr[ 8] = rdes2;       descr[ 9] = rdes2 >> 8;
		descr[10] = rdes2 >> 16; descr[11] = rdes2 >> 24;
		descr[12] = rdes3;       descr[13] = rdes3 >> 8;
		descr[14] = rdes3 >> 16; descr[15] = rdes3 >> 24;
	}

	if (!cpu->memory_rw(cpu, cpu->mem, addr, descr, sizeof(uint32_t)
	    * writeback_len, MEM_WRITE, PHYSICAL | NO_EXCEPTIONS)) {
		fatal("[ dec21143_rx: memory_rw failed! ]\n");
		return 0;
	}

	return 1;
}


/*
 *  dec21143_tx():
 *
 *  Transmit a packet, if the guest OS has marked a descriptor as containing
 *  data to transmit.
 */
int dec21143_tx(struct cpu *cpu, struct dec21143_data *d)
{
	uint64_t addr = d->cur_tx_addr, bufaddr;
	unsigned char descr[16];
	uint32_t tdes0, tdes1, tdes2, tdes3;
	int bufsize, buf1_size, buf2_size, i;

	addr &= 0x7fffffff;

	if (!cpu->memory_rw(cpu, cpu->mem, addr, descr, sizeof(uint32_t),
	    MEM_READ, PHYSICAL | NO_EXCEPTIONS)) {
		fatal("[ dec21143_tx: memory_rw failed! ]\n");
		return 0;
	}

	tdes0 = descr[0] + (descr[1]<<8) + (descr[2]<<16) + (descr[3]<<24);

	/*  fatal("{ dec21143_tx: base=0x%08x, tdes0=0x%08x }\n",
	    (int)addr, (int)tdes0);  */

	/*  Only process packets owned by the 21143:  */
	if (!(tdes0 & TDSTAT_OWN)) {
		if (d->tx_idling > d->tx_idling_threshold) {
			d->reg[CSR_STATUS/8] |= STATUS_TU;
			d->tx_idling = 0;
		} else
			d->tx_idling ++;
		return 0;
	}

	if (!cpu->memory_rw(cpu, cpu->mem, addr + sizeof(uint32_t), descr +
	    sizeof(uint32_t), sizeof(uint32_t) * 3, MEM_READ, PHYSICAL |
	    NO_EXCEPTIONS)) {
		fatal("[ dec21143_tx: memory_rw failed! ]\n");
		return 0;
	}

	tdes1 = descr[4] + (descr[5]<<8) + (descr[6]<<16) + (descr[7]<<24);
	tdes2 = descr[8] + (descr[9]<<8) + (descr[10]<<16) + (descr[11]<<24);
	tdes3 = descr[12] + (descr[13]<<8) + (descr[14]<<16) + (descr[15]<<24);

	buf1_size = tdes1 & TDCTL_SIZE1;
	buf2_size = (tdes1 & TDCTL_SIZE2) >> TDCTL_SIZE2_SHIFT;
	bufaddr = buf1_size? tdes2 : tdes3;
	bufsize = buf1_size? buf1_size : buf2_size;

	d->reg[CSR_STATUS/8] &= ~STATUS_TS;

	if (tdes1 & TDCTL_ER)
		d->cur_tx_addr = d->reg[CSR_TXLIST / 8];
	else {
		if (tdes1 & TDCTL_CH)
			d->cur_tx_addr = tdes3;
		else
			d->cur_tx_addr += 4 * sizeof(uint32_t);
	}

	/*
	fatal("{ TX (%llx): 0x%08x 0x%08x 0x%x 0x%x: buf %i bytes at 0x%x }\n",
	  (long long)addr, tdes0, tdes1, tdes2, tdes3, bufsize, (int)bufaddr);
	*/
	bufaddr &= 0x7fffffff;

	/*  Assume no error:  */
	tdes0 &= ~ (TDSTAT_Tx_UF | TDSTAT_Tx_EC | TDSTAT_Tx_LC
	    | TDSTAT_Tx_NC | TDSTAT_Tx_LO | TDSTAT_Tx_TO | TDSTAT_ES);

	if (tdes1 & TDCTL_Tx_SET) {
		/*
		 *  Setup Packet.
		 *
		 *  TODO. For now, just ignore it, and pretend it worked.
		 */
		/*  fatal("{ TX: setup packet }\n");  */
		if (bufsize != 192)
			fatal("[ dec21143: setup packet len = %i, should be"
			    " 192! ]\n", (int)bufsize);
		if (tdes1 & TDCTL_Tx_IC)
			d->reg[CSR_STATUS/8] |= STATUS_TI;
		/*  New descriptor values, according to the docs:  */
		tdes0 = 0x7fffffff; tdes1 = 0xffffffff;
		tdes2 = 0xffffffff; tdes3 = 0xffffffff;
	} else {
		/*
		 *  Data Packet.
		 */
		/*  fatal("{ TX: data packet: ");  */
		if (tdes1 & TDCTL_Tx_FS) {
			/*  First segment. Let's allocate a new buffer:  */
			/*  fatal("new frame }\n");  */

			CHECK_ALLOCATION(d->cur_tx_buf = (unsigned char *) malloc(bufsize));
			d->cur_tx_buf_len = 0;
		} else {
			/*  Not first segment. Increase the length of
			    the current buffer:  */
			/*  fatal("continuing last frame }\n");  */

			if (d->cur_tx_buf == NULL)
				fatal("[ dec21143: WARNING! tx: middle "
				    "segment, but no first segment?! ]\n");

			CHECK_ALLOCATION(d->cur_tx_buf = (unsigned char *) realloc(d->cur_tx_buf,
			    d->cur_tx_buf_len + bufsize));
		}

		/*  "DMA" data from emulated physical memory into the buf:  */
		for (i=0; i<bufsize; i++) {
			cpu->memory_rw(cpu, cpu->mem, bufaddr + i,
			    d->cur_tx_buf + d->cur_tx_buf_len + i, 1, MEM_READ,
			    PHYSICAL | NO_EXCEPTIONS);
			/*  fatal(" %02x", d->cur_tx_buf[
			    d->cur_tx_buf_len + i]);  */
		}

		d->cur_tx_buf_len += bufsize;

		/*  Last segment? Then actually transmit it:  */
		if (tdes1 & TDCTL_Tx_LS) {
			/*  fatal("{ TX: data frame complete. }\n");  */
			if (d->net != NULL) {
				net_ethernet_tx(d->net, d, d->cur_tx_buf,
				    d->cur_tx_buf_len);
			} else {
				static int warn = 0;
				if (!warn)
					fatal("[ dec21143: WARNING! Not "
					    "connected to a network! ]\n");
				warn = 1;
			}

			free(d->cur_tx_buf);
			d->cur_tx_buf = NULL;
			d->cur_tx_buf_len = 0;

			/*  Interrupt, if Tx_IC is set:  */
			if (tdes1 & TDCTL_Tx_IC)
				d->reg[CSR_STATUS/8] |= STATUS_TI;
		}

		/*  We are done with this segment.  */
		tdes0 &= ~TDSTAT_OWN;
	}

	/*  Error summary:  */
	if (tdes0 & (TDSTAT_Tx_UF | TDSTAT_Tx_EC | TDSTAT_Tx_LC
	    | TDSTAT_Tx_NC | TDSTAT_Tx_LO | TDSTAT_Tx_TO))
		tdes0 |= TDSTAT_ES;

	/*  Descriptor writeback:  */
	descr[ 0] = tdes0;       descr[ 1] = tdes0 >> 8;
	descr[ 2] = tdes0 >> 16; descr[ 3] = tdes0 >> 24;
	descr[ 4] = tdes1;       descr[ 5] = tdes1 >> 8;
	descr[ 6] = tdes1 >> 16; descr[ 7] = tdes1 >> 24;
	descr[ 8] = tdes2;       descr[ 9] = tdes2 >> 8;
	descr[10] = tdes2 >> 16; descr[11] = tdes2 >> 24;
	descr[12] = tdes3;       descr[13] = tdes3 >> 8;
	descr[14] = tdes3 >> 16; descr[15] = tdes3 >> 24;

	if (!cpu->memory_rw(cpu, cpu->mem, addr, descr, sizeof(uint32_t)
	    * 4, MEM_WRITE, PHYSICAL | NO_EXCEPTIONS)) {
		fatal("[ dec21143_tx: memory_rw failed! ]\n");
		return 0;
	}

	return 1;
}


DEVICE_TICK(dec21143)
{
	struct dec21143_data *d = (struct dec21143_data *) extra;
	int asserted;

	if (d->reg[CSR_OPMODE / 8] & OPMODE_ST)
		while (dec21143_tx(cpu, d))
			;

	if (d->reg[CSR_OPMODE / 8] & OPMODE_SR)
		while (dec21143_rx(cpu, d))
			;

	/*  Normal and Abnormal interrupt summary:  */
	d->reg[CSR_STATUS / 8] &= ~(STATUS_NIS | STATUS_AIS);
	if (d->reg[CSR_STATUS / 8] & 0x00004845)
		d->reg[CSR_STATUS / 8] |= STATUS_NIS;
	if (d->reg[CSR_STATUS / 8] & 0x0c0037ba)
		d->reg[CSR_STATUS / 8] |= STATUS_AIS;

	asserted = d->reg[CSR_STATUS / 8] & d->reg[CSR_INTEN / 8] & 0x0c01ffff;

	if (asserted)
		INTERRUPT_ASSERT(d->irq);
	if (!asserted && d->irq_was_asserted)
		INTERRUPT_DEASSERT(d->irq);

	/*  Remember assertion flag:  */
	d->irq_was_asserted = asserted;
}


/*
 *  mii_access():
 *
 *  This function handles accesses to the MII. Data streams seem to be of the
 *  following format:
 *
 *      vv---- starting delimiter
 *  ... 01 xx yyyyy zzzzz a[a] dddddddddddddddd
 *         ^---- I am starting with mii_bit = 0 here
 *
 *  where x = opcode (10 = read, 01 = write)
 *        y = PHY address
 *        z = register address
 *        a = on Reads: ACK bit (returned, should be 0)
 *            on Writes: _TWO_ dummy bits (10)
 *        d = 16 bits of data (MSB first)
 */
static void mii_access(struct cpu *cpu, struct dec21143_data *d,
	uint32_t oldreg, uint32_t idata)
{
	int obit, ibit = 0;
	uint16_t tmp;

	/*  Only care about data during clock cycles:  */
	if (!(idata & MIIROM_MDC))
		return;

	if (idata & MIIROM_MDC && oldreg & MIIROM_MDC)
		return;

	/*  fatal("[ mii_access(): 0x%08x ]\n", (int)idata);  */

	if (idata & MIIROM_BR) {
		fatal("[ mii_access(): MIIROM_BR: TODO ]\n");
		return;
	}

	obit = idata & MIIROM_MDO? 1 : 0;

	if (d->mii_state >= MII_STATE_START_WAIT &&
	    d->mii_state <= MII_STATE_READ_PHYADDR_REGADDR &&
	    idata & MIIROM_MIIDIR)
		fatal("[ mii_access(): bad dir? ]\n");

	switch (d->mii_state) {

	case MII_STATE_RESET:
		/*  Wait for a starting delimiter (0 followed by 1).  */
		if (obit)
			return;
		if (idata & MIIROM_MIIDIR)
			return;
		/*  fatal("[ mii_access(): got a 0 delimiter ]\n");  */
		d->mii_state = MII_STATE_START_WAIT;
		d->mii_opcode = 0;
		d->mii_phyaddr = 0;
		d->mii_regaddr = 0;
		break;

	case MII_STATE_START_WAIT:
		/*  Wait for a starting delimiter (0 followed by 1).  */
		if (!obit)
			return;
		if (idata & MIIROM_MIIDIR) {
			d->mii_state = MII_STATE_RESET;
			return;
		}
		/*  fatal("[ mii_access(): got a 1 delimiter ]\n");  */
		d->mii_state = MII_STATE_READ_OP;
		d->mii_bit = 0;
		break;

	case MII_STATE_READ_OP:
		if (d->mii_bit == 0) {
			d->mii_opcode = obit << 1;
			/*  fatal("[ mii_access(): got first opcode bit "
			    "(%i) ]\n", obit);  */
		} else {
			d->mii_opcode |= obit;
			/*  fatal("[ mii_access(): got opcode = %i ]\n",
			    d->mii_opcode);  */
			d->mii_state = MII_STATE_READ_PHYADDR_REGADDR;
		}
		d->mii_bit ++;
		break;

	case MII_STATE_READ_PHYADDR_REGADDR:
		/*  fatal("[ mii_access(): got phy/reg addr bit nr %i (%i)"
		    " ]\n", d->mii_bit - 2, obit);  */
		if (d->mii_bit <= 6)
			d->mii_phyaddr |= obit << (6-d->mii_bit);
		else
			d->mii_regaddr |= obit << (11-d->mii_bit);
		d->mii_bit ++;
		if (d->mii_bit >= 12) {
			/*  fatal("[ mii_access(): phyaddr=0x%x regaddr=0x"
			    "%x ]\n", d->mii_phyaddr, d->mii_regaddr);  */
			d->mii_state = MII_STATE_A;
		}
		break;

	case MII_STATE_A:
		switch (d->mii_opcode) {
		case MII_COMMAND_WRITE:
			if (d->mii_bit >= 13)
				d->mii_state = MII_STATE_D;
			break;
		case MII_COMMAND_READ:
			ibit = 0;
			d->mii_state = MII_STATE_D;
			break;
		default:debug("[ mii_access(): UNIMPLEMENTED MII opcode "
			    "%i (probably just a bug in GXemul's "
			    "MII data stream handling) ]\n", d->mii_opcode);
			d->mii_state = MII_STATE_RESET;
		}
		d->mii_bit ++;
		break;

	case MII_STATE_D:
		switch (d->mii_opcode) {
		case MII_COMMAND_WRITE:
			if (idata & MIIROM_MIIDIR)
				fatal("[ mii_access(): write: bad dir? ]\n");
			obit = obit? (0x8000 >> (d->mii_bit - 14)) : 0;
			tmp = d->mii_phy_reg[(d->mii_phyaddr << 5) +
			    d->mii_regaddr] | obit;
			if (d->mii_bit >= 29) {
				d->mii_state = MII_STATE_IDLE;
				debug("[ mii_access(): WRITE to phyaddr=0x%x "
				    "regaddr=0x%x: 0x%04x ]\n", d->mii_phyaddr,
				    d->mii_regaddr, tmp);
			}
			break;
		case MII_COMMAND_READ:
			if (!(idata & MIIROM_MIIDIR))
				break;
			tmp = d->mii_phy_reg[(d->mii_phyaddr << 5) +
			    d->mii_regaddr];
			if (d->mii_bit == 13)
				debug("[ mii_access(): READ phyaddr=0x%x "
				    "regaddr=0x%x: 0x%04x ]\n", d->mii_phyaddr,
				    d->mii_regaddr, tmp);
			ibit = tmp & (0x8000 >> (d->mii_bit - 13));
			if (d->mii_bit >= 28)
				d->mii_state = MII_STATE_IDLE;
			break;
		}
		d->mii_bit ++;
		break;

	case MII_STATE_IDLE:
		d->mii_bit ++;
		if (d->mii_bit >= 31)
			d->mii_state = MII_STATE_RESET;
		break;
	}

	d->reg[CSR_MIIROM / 8] &= ~MIIROM_MDI;
	if (ibit)
		d->reg[CSR_MIIROM / 8] |= MIIROM_MDI;
}


/*
 *  srom_access():
 *
 *  This function handles reads from the Ethernet Address ROM. This is not a
 *  100% correct implementation, as it was reverse-engineered from OpenBSD
 *  sources; it seems to work with OpenBSD, NetBSD, and Linux, though.
 *
 *  Each transfer (if I understood this correctly) is of the following format:
 *
 *	1xx yyyyyy zzzzzzzzzzzzzzzz
 *
 *  where 1xx    = operation (6 means a Read),
 *        yyyyyy = ROM address
 *        zz...z = data
 *
 *  y and z are _both_ read and written to at the same time; this enables the
 *  operating system to sense the number of bits in y (when reading, all y bits
 *  are 1 except the last one).
 */
static void srom_access(struct cpu *cpu, struct dec21143_data *d,
	uint32_t oldreg, uint32_t idata)
{
	int obit, ibit;

	/*  debug("CSR9 WRITE! 0x%08x\n", (int)idata);  */

	/*  New selection? Then reset internal state.  */
	if (idata & MIIROM_SR && !(oldreg & MIIROM_SR)) {
		d->srom_curbit = 0;
		d->srom_opcode = 0;
		d->srom_opcode_has_started = 0;
		d->srom_addr = 0;
	}

	/*  Only care about data during clock cycles:  */
	if (!(idata & MIIROM_SROMSK))
		return;

	obit = 0;
	ibit = idata & MIIROM_SROMDI? 1 : 0;
	/*  debug("CLOCK CYCLE! (bit %i): ", d->srom_curbit);  */

	/*
	 *  Linux sends more zeroes before starting the actual opcode, than
	 *  OpenBSD and NetBSD. Hopefully this is correct. (I'm just guessing
	 *  that all opcodes should start with a 1, perhaps that's not really
	 *  the case.)
	 */
	if (!ibit && !d->srom_opcode_has_started)
		return;

	if (d->srom_curbit < 3) {
		d->srom_opcode_has_started = 1;
		d->srom_opcode <<= 1;
		d->srom_opcode |= ibit;
		/*  debug("opcode input '%i'\n", ibit);  */
	} else {
		switch (d->srom_opcode) {
		case TULIP_SROM_OPC_READ:
			if (d->srom_curbit < ROM_WIDTH + 3) {
				obit = d->srom_curbit < ROM_WIDTH + 2;
				d->srom_addr <<= 1;
				d->srom_addr |= ibit;
			} else {
				uint16_t romword = d->srom[d->srom_addr*2]
				    + (d->srom[d->srom_addr*2+1] << 8);
				if (d->srom_curbit == ROM_WIDTH + 3)
					debug("[ dec21143: ROM read from offset"
					    " 0x%03x: 0x%04x ]\n",
					    d->srom_addr, romword);
				obit = romword & (0x8000 >>
				    (d->srom_curbit - ROM_WIDTH - 3))? 1 : 0;
			}
			break;
		default:fatal("[ dec21243: unimplemented SROM/EEPROM "
			    "opcode %i ]\n", d->srom_opcode);
		}
		d->reg[CSR_MIIROM / 8] &= ~MIIROM_SROMDO;
		if (obit)
			d->reg[CSR_MIIROM / 8] |= MIIROM_SROMDO;
		/*  debug("input '%i', output '%i'\n", ibit, obit);  */
	}

	d->srom_curbit ++;

	/*
	 *  Done opcode + addr + data? Then restart. (At least NetBSD does
	 *  sequential reads without turning selection off and then on.)
	 */
	if (d->srom_curbit >= 3 + ROM_WIDTH + 16) {
		d->srom_curbit = 0;
		d->srom_opcode = 0;
		d->srom_opcode_has_started = 0;
		d->srom_addr = 0;
	}
}


/*
 *  dec21143_reset():
 *
 *  Set the 21143 registers, SROM, and MII data to reasonable values.
 */
static void dec21143_reset(struct cpu *cpu, struct dec21143_data *d)
{
	int leaf;

	if (d->cur_rx_buf != NULL)
		free(d->cur_rx_buf);
	if (d->cur_tx_buf != NULL)
		free(d->cur_tx_buf);
	d->cur_rx_buf = d->cur_tx_buf = NULL;

	memset(d->reg, 0, sizeof(uint32_t) * N_REGS);
	memset(d->srom, 0, sizeof(d->srom));
	memset(d->mii_phy_reg, 0, sizeof(d->mii_phy_reg));

	/*  Register values at reset, according to the manual:  */
	d->reg[CSR_BUSMODE / 8] = 0xfe000000;	/*  csr0   */
	d->reg[CSR_MIIROM  / 8] = 0xfff483ff;	/*  csr9   */
	d->reg[CSR_SIACONN / 8] = 0xffff0000;	/*  csr13  */
	d->reg[CSR_SIATXRX / 8] = 0xffffffff;	/*  csr14  */
	d->reg[CSR_SIAGEN  / 8] = 0x8ff00000;	/*  csr15  */

	d->tx_idling_threshold = 10;
	d->cur_rx_addr = d->cur_tx_addr = 0;

	/*  Version (= 1) and Chip count (= 1):  */
	d->srom[TULIP_ROM_SROM_FORMAT_VERION] = 1;
	d->srom[TULIP_ROM_CHIP_COUNT] = 1;

	/*  Set the MAC address:  */
	memcpy(d->srom + TULIP_ROM_IEEE_NETWORK_ADDRESS, d->mac, 6);

	leaf = 30;
	d->srom[TULIP_ROM_CHIPn_DEVICE_NUMBER(0)] = 0;
	d->srom[TULIP_ROM_CHIPn_INFO_LEAF_OFFSET(0)] = leaf & 255;
	d->srom[TULIP_ROM_CHIPn_INFO_LEAF_OFFSET(0)+1] = leaf >> 8;

	d->srom[leaf+TULIP_ROM_IL_SELECT_CONN_TYPE] = 0; /*  Not used?  */
	d->srom[leaf+TULIP_ROM_IL_MEDIA_COUNT] = 2;
	leaf += TULIP_ROM_IL_MEDIAn_BLOCK_BASE;

	d->srom[leaf] = 7;	/*  descriptor length  */
	d->srom[leaf+1] = TULIP_ROM_MB_21142_SIA;
	d->srom[leaf+2] = TULIP_ROM_MB_MEDIA_100TX;
	/*  here comes 4 bytes of GPIO control/data settings  */
	leaf += d->srom[leaf];

	d->srom[leaf] = 15;	/*  descriptor length  */
	d->srom[leaf+1] = TULIP_ROM_MB_21142_MII;
	d->srom[leaf+2] = 0;	/*  PHY nr  */
	d->srom[leaf+3] = 0;	/*  len of select sequence  */
	d->srom[leaf+4] = 0;	/*  len of reset sequence  */
	/*  5,6, 7,8, 9,10, 11,12, 13,14 = unused by GXemul  */
	leaf += d->srom[leaf];

	/*  MII PHY initial state:  */
	d->mii_state = MII_STATE_RESET;

	/*  PHY #0:  */
	d->mii_phy_reg[MII_BMSR] = BMSR_100TXFDX | BMSR_10TFDX |
	    BMSR_ACOMP | BMSR_ANEG | BMSR_LINK;
}


DEVICE_ACCESS(dec21143)
{
	struct dec21143_data *d = (struct dec21143_data *) extra;
	uint64_t idata = 0, odata = 0;
	uint32_t oldreg = 0;
	int regnr = relative_addr >> 3;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len | d->pci_little_endian);

	if ((relative_addr & 7) == 0 && regnr < N_REGS) {
		if (writeflag == MEM_READ) {
			odata = d->reg[regnr];
		} else {
			oldreg = d->reg[regnr];
			switch (regnr) {
			case CSR_STATUS / 8:	/*  Zero-on-write  */
				d->reg[regnr] &= ~(idata & 0x0c01ffff);
				break;
			case CSR_MISSED / 8:	/*  Read only  */
				break;
			default:d->reg[regnr] = idata;
			}
		}
	} else
		fatal("[ dec21143: WARNING! unaligned access (0x%x) ]\n",
		    (int)relative_addr);

	switch (relative_addr) {

	case CSR_BUSMODE:	/*  csr0  */
		if (writeflag == MEM_WRITE) {
			/*  Software reset takes effect immediately.  */
			if (idata & BUSMODE_SWR) {
				dec21143_reset(cpu, d);
				idata &= ~BUSMODE_SWR;
			}
		}
		break;

	case CSR_TXPOLL:	/*  csr1  */
		if (writeflag == MEM_READ)
			fatal("[ dec21143: UNIMPLEMENTED READ from "
			    "txpoll ]\n");
		d->tx_idling = d->tx_idling_threshold;
		dev_dec21143_tick(cpu, extra);
		break;

	case CSR_RXPOLL:	/*  csr2  */
		if (writeflag == MEM_READ)
			fatal("[ dec21143: UNIMPLEMENTED READ from "
			    "rxpoll ]\n");
		dev_dec21143_tick(cpu, extra);
		break;

	case CSR_RXLIST:	/*  csr3  */
		if (writeflag == MEM_WRITE) {
			debug("[ dec21143: setting RXLIST to 0x%x ]\n",
			    (int)idata);
			if (idata & 0x3)
				fatal("[ dec21143: WARNING! RXLIST not aligned"
				    "? (0x%llx) ]\n", (long long)idata);
			idata &= ~0x3;
			d->cur_rx_addr = idata;
		}
		break;

	case CSR_TXLIST:	/*  csr4  */
		if (writeflag == MEM_WRITE) {
			debug("[ dec21143: setting TXLIST to 0x%x ]\n",
			    (int)idata);
			if (idata & 0x3)
				fatal("[ dec21143: WARNING! TXLIST not aligned"
				    "? (0x%llx) ]\n", (long long)idata);
			idata &= ~0x3;
			d->cur_tx_addr = idata;
		}
		break;

	case CSR_STATUS:	/*  csr5  */
	case CSR_INTEN:		/*  csr7  */
		if (writeflag == MEM_WRITE) {
			/*  Recalculate interrupt assertion.  */
			dev_dec21143_tick(cpu, extra);
		}
		break;

	case CSR_OPMODE:	/*  csr6:  */
		if (writeflag == MEM_WRITE) {
			if (idata & 0x02000000) {
				/*  A must-be-one bit.  */
				idata &= ~0x02000000;
			}
			if (idata & OPMODE_ST) {
				idata &= ~OPMODE_ST;
			} else {
				/*  Turned off TX? Then idle:  */
				d->reg[CSR_STATUS/8] |= STATUS_TPS;
			}
			if (idata & OPMODE_SR) {
				idata &= ~OPMODE_SR;
			} else {
				/*  Turned off RX? Then go to stopped state:  */
				d->reg[CSR_STATUS/8] &= ~STATUS_RS;
			}
			idata &= ~(OPMODE_HBD | OPMODE_SCR | OPMODE_PCS
			    | OPMODE_PS | OPMODE_SF | OPMODE_TTM | OPMODE_FD);
			if (idata & OPMODE_PNIC_IT) {
				idata &= ~OPMODE_PNIC_IT;
				d->tx_idling = d->tx_idling_threshold;
			}
			if (idata != 0) {
				fatal("[ dec21143: UNIMPLEMENTED OPMODE bits"
				    ": 0x%08x ]\n", (int)idata);
			}
			dev_dec21143_tick(cpu, extra);
		}
		break;

	case CSR_MISSED:	/*  csr8  */
		break;

	case CSR_MIIROM:	/*  csr9  */
		if (writeflag == MEM_WRITE) {
			if (idata & MIIROM_MDC)
				mii_access(cpu, d, oldreg, idata);
			else
				srom_access(cpu, d, oldreg, idata);
		}
		break;

	case CSR_SIASTAT:	/*  csr12  */
		/*  Auto-negotiation status = Good.  */
		odata = SIASTAT_ANS_FLPGOOD;
		break;

	case CSR_SIATXRX:	/*  csr14  */
		/*  Auto-negotiation Enabled  */
		odata = SIATXRX_ANE;
		break;

	case CSR_SIACONN:	/*  csr13  */
	case CSR_SIAGEN:	/*  csr15  */
		/*  Don't print warnings for these, for now.  */
		break;

	default:if (writeflag == MEM_READ)
			fatal("[ dec21143: read from unimplemented 0x%02x ]\n",
			    (int)relative_addr);
		else
			fatal("[ dec21143: write to unimplemented 0x%02x: "
			    "0x%02x ]\n", (int)relative_addr, (int)idata);
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len | d->pci_little_endian, odata);

	return 1;
}


DEVINIT(dec21143)
{
	struct dec21143_data *d;
	char name2[100];

	CHECK_ALLOCATION(d = (struct dec21143_data *) malloc(sizeof(struct dec21143_data)));
	memset(d, 0, sizeof(struct dec21143_data));

	INTERRUPT_CONNECT(devinit->interrupt_path, d->irq);
	d->pci_little_endian = devinit->pci_little_endian;

	net_generate_unique_mac(devinit->machine, d->mac);
	net_add_nic(devinit->machine->emul->net, d, d->mac);
	d->net = devinit->machine->emul->net;

	dec21143_reset(devinit->machine->cpus[0], d);

	snprintf(name2, sizeof(name2), "%s [%02x:%02x:%02x:%02x:%02x:%02x]",
	    devinit->name, d->mac[0], d->mac[1], d->mac[2], d->mac[3],
	    d->mac[4], d->mac[5]);

	memory_device_register(devinit->machine->memory, name2,
	    devinit->addr, 0x100, dev_dec21143_access, d, DM_DEFAULT, NULL);

	machine_add_tickfunction(devinit->machine,
	    dev_dec21143_tick, d, DEC21143_TICK_SHIFT);

	/*
	 *  NetBSD/cats uses memory accesses, OpenBSD/cats uses I/O registers.
	 *  Let's make a mirror from the memory range to the I/O range:
	 */
	dev_ram_init(devinit->machine, devinit->addr2, 0x100, DEV_RAM_MIRROR
	    | DEV_RAM_MIGHT_POINT_TO_DEVICES, devinit->addr);

	return 1;
}

