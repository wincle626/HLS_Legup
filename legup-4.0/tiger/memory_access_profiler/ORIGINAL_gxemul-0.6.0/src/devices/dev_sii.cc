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
 *  COMMENT: SII SCSI controller, used in some DECstation systems
 *
 *  TODO:  This is huge and ugly. Fix this.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "devices.h"
#include "interrupt.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/siireg.h"


#define	SII_TICK_SHIFT		14


struct sii_data {
	struct interrupt irq;

	uint64_t	buf_start;
	uint64_t	buf_end;

	int		connected;
	int		connected_to_id;

	int		register_choice;
	SIIRegs		siiregs;
	uint16_t	*regs;
};


/*
 *  combine_sii_bits():
 *
 *  Combines some bits of CSTAT and DSTAT that are connected.
 */
void combine_sii_bits(struct sii_data *d)
{
	int ci, di;

	di = ((d->siiregs.dstat & SII_MIS) |
	      (d->siiregs.dstat & SII_IBF) |
	      (d->siiregs.dstat & SII_TBE) |
	      (d->siiregs.dstat & SII_DNE))==0? 0 : SII_DI;

	ci = ((d->siiregs.cstat & SII_RST) |
	      (d->siiregs.cstat & SII_BER) |
	      (d->siiregs.cstat & SII_OBC) |
	      (d->siiregs.cstat & SII_BUF) |
	      (d->siiregs.cstat & SII_LDN) |
	      (d->siiregs.cstat & SII_SCH))==0? 0 : SII_CI;

	d->siiregs.cstat &= ~(SII_CI | SII_DI);
	d->siiregs.dstat &= ~(SII_CI | SII_DI);

	d->siiregs.cstat |= (ci | di);
	d->siiregs.dstat |= (ci | di);
}


DEVICE_TICK(sii)
{
	struct sii_data *d = (struct sii_data *) extra;

	/*  ?  */
	d->siiregs.dstat = (d->siiregs.dstat & ~0x7)
	    | ((d->siiregs.dstat + 1) & 0x7);

	/*  SCSI Commands:  */

	if (d->siiregs.comm & SII_CHRESET) {		/*  (I,T,D)  */
		/*  debug("[ sii: command TODO: CHRESET ]\n");  */
	}

	if (d->siiregs.comm & SII_DISCON) {		/*  (I,T,D)  */
		/*  debug("[ sii: command TODO: DISCON ]\n");  */
		d->siiregs.cstat &= ~SII_CON;	/*  Connected  */

		if (d->connected) {
			d->siiregs.cstat |= SII_SCH;	/*  State change  */
			d->connected = 0;
		}

		d->siiregs.cstat &= ~SII_SIP;  	/*  Selection in progress  */
		d->siiregs.comm  &= ~SII_DISCON;
	}

	if (d->siiregs.comm & SII_REQDATA) {	/*  (T)  */
		/*  debug("[ sii: command TODO: REQDATA ]\n");  */
	}

	if (d->siiregs.comm & SII_SELECT) {		/*  (D)  */
		/*  debug("[ sii: command SELECT ]\n");  */
		d->siiregs.comm &= ~SII_SELECT;

		/*  slcsr contains the other target's id  */
		d->siiregs.cstat |= SII_SIP;	/*  Selection in progress  */
		d->connected = 0;
		d->connected_to_id = 0;

		/*  Is the target available for selection?
		    TODO: make this nicer  */
#if 0
		if ((d->siiregs.slcsr & 7) == 0) {
			d->siiregs.cstat |=  SII_CON;	/*  Connected  */
			d->siiregs.cstat |=  SII_SCH;	/*  State change  */
			d->siiregs.cstat &= ~SII_SIP;	/*  Sel. in progress  */

			d->connected = 1;
			d->connected_to_id = 0;
		}
#endif
	}

	if (d->siiregs.comm & SII_INXFER
	    && (d->siiregs.comm & 0x70) == (d->siiregs.cstat & 0x70) &&
	    (d->siiregs.comm & 0x03) == (d->siiregs.dstat & 0x03) &&
	    !(d->siiregs.cstat & SII_SIP)) {	/*  (I,T)  */
		debug("[ sii: command INXFER to scsiid=%i ]\n",
		    d->siiregs.slcsr);
		if (d->siiregs.comm & SII_DMA)
			debug("[ sii DMA: TODO ]\n");
		else {
			debug("[ sii: transmitting byte 0x%02x using "
			    "PIO mode ]\n", d->siiregs.data);
			d->siiregs.comm &= ~SII_INXFER;

/*			d->siiregs.dstat |= SII_DNE; */
			    /*  Done, only for DMA?  */
			d->siiregs.dstat |= SII_TBE;	/*  Buffer empty?  */
		}
	}

	combine_sii_bits(d);

	if (d->siiregs.csr & SII_IE && d->siiregs.cstat & (SII_CI | SII_DI))
		INTERRUPT_ASSERT(d->irq);
	else
		INTERRUPT_DEASSERT(d->irq);
}


DEVICE_ACCESS(sii)
{
	uint64_t idata = 0, odata = 0;
	int regnr;
	struct sii_data *d = (struct sii_data *) extra;

	if (relative_addr & 3) {
		debug("[ sii relative_addr = 0x%x !!! ]\n",
		    (int) relative_addr);
		return 0;
	}

	dev_sii_tick(cpu, extra);

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	regnr = relative_addr / 2;
	odata = d->regs[regnr];

	switch (relative_addr) {
	case 0x00:				/*  SII_SDB: Diagnostic  */
		if (writeflag == MEM_READ) {
			debug("[ sii: read from SDB (data=0x%04x) ]\n",
			    d->regs[regnr]);
		} else {
			debug("[ sii: write to  SDB (data=0x%04x) ]\n",
			    (int)idata);
			d->regs[regnr] = idata;
			return 1;
		}
		break;
	case 0x0c:				/*  SII_CSR: Control/status  */
		if (writeflag == MEM_READ) {
			debug("[ sii: read from CSR (data=0x%04x) ]\n",
			    d->regs[regnr]);
		} else {
			debug("[ sii: write to  CSR (data=0x%04x: %s %s "
			    "%s %s %s) ]\n", (int)idata,
			    idata & SII_HPM? "HPM" : "!hpm",
			    idata & SII_RSE? "RSE" : "!rse",
			    idata & SII_SLE? "SLE" : "!sle",
			    idata & SII_PCE? "PCE" : "!pce",
			    idata & SII_IE?  "IE"  : "!ie");
			d->regs[regnr] = idata;
			return 1;
		}
		break;
	case 0x10:				/*  SII_ID: SCSI ID  */
		if (writeflag == MEM_READ) {
			debug("[ sii: read from ID (data=0x%04x) ]\n",
			    d->regs[regnr]);
		} else {
			debug("[ sii: write to  ID (data=0x%04x: scsi id %i)"
			    " ]\n", (int)idata, (int)(idata & 7));
			if (!(idata & SII_ID_IO))
				debug("WARNING: sii ID bit SII_ID_IO not "
				    "set on write!\n");
			idata &= ~SII_ID_IO;
			if ((idata & ~0x7) != 0)
				debug("WARNING: sii ID bits that should "
				    "be zero are not zero!\n");
			idata &= 0x7;
			d->regs[regnr] = idata & 0x7;
			return 1;
		}
		break;
	case 0x14:			/*  SII_SLCSR: Selector control  */
		if (writeflag == MEM_READ) {
			debug("[ sii: read from SLCSR (data=0x%04x: "
			    "scsi_id=%i) ]\n", d->regs[regnr],
			    d->regs[regnr] & 7);
		} else {
			debug("[ sii: write to  SLCSR (data=0x%04x: "
			    "scsi_id=%i) ]\n", (int)idata, (int)(idata & 7));
			if ((idata & ~0x7) != 0)
				debug("WARNING: sii SLCSR bits that should "
				    "be zero are not zero!\n");
			idata &= 0x7;
			d->regs[regnr] = idata & 0x7;
			return 1;
		}
		break;
	case 0x18:		/*  SII_DESTAT: Selection detector status  */
		if (writeflag == MEM_READ) {
			/*  TODO: set DESTAT from somewhere else?  */
			debug("[ sii: read from DESTAT (data=0x%04x: "
			    "scsi_id=%i) ]\n", d->regs[regnr],
			    d->regs[regnr] & 7);
		} else {
			debug("[ sii: write to  DESTAT (data=0x%04x: "
			    "scsi_id=%i) ]\n", (int)idata, (int)(idata & 7));
			debug("WARNING: sii DESTAT is read-only!\n");
			return 1;
		}
		break;
	case 0x20:				/*  SII_DATA: Data register  */
		if (writeflag == MEM_READ) {
			/*  TODO  */
			debug("[ sii: read from DATA (data=0x%04x) ]\n",
			    d->regs[regnr]);
		} else {
			/*  TODO  */
			debug("[ sii: write to  DATA (data=0x%04x) ]\n",
			    (int)idata);
			idata &= 0xff;
			d->regs[regnr] = idata;
			return 1;
		}
		break;
	case 0x24:				/*  SII_DMCTRL: DMA control  */
		if (writeflag == MEM_READ) {
			debug("[ sii: read from DMCTRL (data=0x%04x) ]\n",
			    d->regs[regnr]);
		} else {
			debug("[ sii: write to  DMCTRL (data=0x%04x: %s) ]\n",
			    (int)idata, (idata & 3)==0? "async" : "sync");
			if ((idata & ~0x3) != 0)
				debug("WARNING: sii DMCTRL bits that "
				    "should be zero are not zero!\n");
			idata &= 0x3;
			d->regs[regnr] = idata;
			return 1;
		}
		break;
	case 0x48:			/*  SII_CSTAT: Connection status  */
		if (writeflag == MEM_READ) {
			debug("[ sii: read from CSTAT (data=0x%04x) ]\n",
			    d->regs[regnr]);
		} else {
			debug("[ sii: write to  CSTAT (data=0x%04x) ]\n",
			    (int)idata);

			/*  readonly / writeoncetoclear bits according
			    to page 21 in the DS3100 manual:  */
			if (idata & (1<<13)) {
				idata &= ~(1<<13); d->regs[regnr] &= ~(1<<13);
			}
			if (idata & (1<<12)) {
				idata &= ~(1<<12); d->regs[regnr] &= ~(1<<12);
			}
			if (idata & (1<<11)) {
				/*  is this actually write-1-to-clear?  */
				idata &= ~(1<<11); d->regs[regnr] &= ~(1<<11);
			}
			if (idata & (1<<9)) {
				/*  ?  */
				idata &= ~(1<<9); d->regs[regnr] &= ~(1<<9);
			}
			if (idata & (1<<8)) {
				/*  ?  */
				idata &= ~(1<<8); d->regs[regnr] &= ~(1<<8);
			}
			if (idata & (1<<7)) {
				idata &= ~(1<<7); d->regs[regnr] &= ~(1<<7);
			}
			if (idata & (1<<3)) {
				idata &= ~(1<<3); d->regs[regnr] &= ~(1<<3);
			}

			/*  Read-only bits are taken from the old register:  */
			idata &= ~0x3bf7;
			idata |= d->regs[regnr] & 0x3bf7;

			d->regs[regnr] = idata;
			return 1;
		}
		break;
	case 0x4c:			/*  SII_DSTAT: Data transfer status  */
		if (writeflag == MEM_READ) {
			debug("[ sii: read from DSTAT (data=0x%04x) ]\n",
			    d->regs[regnr]);
		} else {
			debug("[ sii: write to  DSTAT (data=0x%04x) ]\n",
			    (int)idata);

			/*  readonly / writeoncetoclear bits
			    according to page 22 in the DS3100 manual:  */
			if (idata & (1<<13)) {
				idata &= ~(1<<13); d->regs[regnr] &= ~(1<<13);
			}
			if (idata & (1<<11)) {
				/*  is this write-1-to-clear?  */
				idata &= ~(1<<11); d->regs[regnr] &= ~(1<<11);
			}
			if (idata & (1<<10)) {
				/*  is this write-1-to-clear?  */
				idata &= ~(1<<10); d->regs[regnr] &= ~(1<<10);
			}
			if (idata & (1<<4)) {
				/*  is this write-1-to-clear?  */
				idata &= ~(1<<4); d->regs[regnr] &= ~(1<<4);
			}
			if (idata & (1<<3)) {
				idata &= ~(1<<3); d->regs[regnr] &= ~(1<<3);
			}

			/*  Read-only bits are taken from the old register:  */
			idata &= ~0x0c17;
			idata |= d->regs[regnr] & 0x0c17;

			d->regs[regnr] = idata;
			return 1;
		}
		break;
	case 0x50:				/*  SII_COMM: Command  */
		if (writeflag == MEM_READ) {
			debug("[ sii: read from COMM (data=0x%04x) ]\n",
			    d->regs[regnr]);
		} else {
			debug("[ sii: write to  COMM (data=0x%04x: %s %s "
			    "%s command=0x%02x rest=0x%02x) ]\n", (int)idata,
			    idata & SII_DMA?    "DMA" : "!dma",
			    idata & SII_DO_RST? "RST" : "!rst",
			    idata & SII_RSL?    "RSL" : "!rsl",
			    /*  command, 5 bits:  */
			    (int)((idata >> 7) & 0x1f),
			    /*  rest, 7 bits:  */
			    (int)(idata & 0x3f));

			if (idata & SII_DO_RST) {
				/*  Reset: TODO  */
			}

			idata &= ~SII_DO_RST;
			d->regs[regnr] = idata;

			dev_sii_tick(cpu, extra);
			return 1;
		}
		break;
	case 0x54:			/*  SII_DICTRL: Diagnostics control  */
		if (writeflag == MEM_READ) {
			debug("[ sii: read from DICTRL (data=0x%04x) ]\n",
			    d->regs[regnr]);
		} else {
			debug("[ sii: write to  DICTRL (data=0x%04x: "
			    "port=%s) ]\n", (int)idata,
			    idata & SII_PRE? "enabled" : "disabled");
			if ((idata & ~0xf) != 0)
				debug("WARNING: sii DICTRL bits that "
				    "should be zero are not zero!\n");
			d->regs[regnr] = idata;
			return 1;
		}
		break;
	default:
		if (writeflag==MEM_READ) {
			debug("[ sii: read from %08lx (data=0x%04x) ]\n",
			    (long)relative_addr, d->regs[regnr]);
		} else {
			debug("[ sii: write to  %08lx (data=0x%04x) ]\n",
			    (long)relative_addr, (int)idata);
			d->regs[regnr] = idata;
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


void dev_sii_init(struct machine *machine, struct memory *mem,
	uint64_t baseaddr, uint64_t buf_start, uint64_t buf_end,
	char *irq_path)
{
	struct sii_data *d;

	CHECK_ALLOCATION(d = (struct sii_data *) malloc(sizeof(struct sii_data)));
	memset(d, 0, sizeof(struct sii_data));

	INTERRUPT_CONNECT(irq_path, d->irq);
	d->buf_start = buf_start;
	d->buf_end   = buf_end;
	d->regs      = (uint16_t *) &d->siiregs;

	memory_device_register(mem, "sii", baseaddr, DEV_SII_LENGTH,
	    dev_sii_access, (void *)d, DM_DEFAULT, NULL);

	machine_add_tickfunction(machine, dev_sii_tick, d,
	    SII_TICK_SHIFT);
}

