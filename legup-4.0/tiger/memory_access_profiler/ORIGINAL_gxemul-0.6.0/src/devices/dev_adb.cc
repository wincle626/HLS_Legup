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
 *  COMMENT: Apple Desktop Bus (ADB) controller
 *
 *  Based on intuition from reverse-engineering NetBSD/macppc source code,
 *  so it probably only works with that OS.
 *
 *  The comment "OK" means that 100% of the functionality used by NetBSD/macppc
 *  is covered.
 *
 *  TODO:
 *	o)  Clean up, don't hardcode values.
 *	o)  Convert into a separate controller, bus, device architecture.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "console.h"
#include "cpu.h"
#include "device.h"
#include "interrupt.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/adb_viareg.h"


#define debug fatal
/*  #define ADB_DEBUG  */


#define	TICK_SHIFT		17
#define	DEV_ADB_LENGTH		0x2000

#define	N_VIA_REGS		0x10
#define	VIA_REG_SHIFT		9

#define	MAX_BUF			100


static const char *via_regname[N_VIA_REGS] = {
	"vBufB", "vBufA", "vDirB", "vDirA",
	"vT1C",  "vT1CH", "vT1L",  "vT1LH",
	"vT2C",  "vT2CH", "vSR",   "vACR",
	"vPCR",  "vIFR",  "vIER",  "(unknown)" };

struct adb_data {
	struct interrupt irq;
	int		int_asserted;

	int		kbd_dev;

	long long	transfer_nr;

	uint8_t		reg[N_VIA_REGS];

	int		cur_output_offset;
	uint8_t		output_buf[MAX_BUF];

	int		cur_input_offset;
	int		cur_input_length;
	uint8_t		input_buf[MAX_BUF];

	int		dir;
	int		int_enable;
	int		ack;		/*  last ack state  */
	int		tip;		/*  transfer in progress  */
};

#define	DIR_INPUT		0
#define	DIR_OUTPUT		1

#define	BUFB_nINTR		0x08
#define	BUFB_ACK		0x10
#define	BUFB_nTIP		0x20
#define	IFR_SR			0x04
#define	IFR_ANY			0x80
#define	ACR_SR_OUT		0x10



DEVICE_TICK(adb)
{
	struct adb_data *d = (struct adb_data *) extra;
	int assert;

	assert = d->reg[vIFR >> VIA_REG_SHIFT] & IFR_ANY;
	if (assert == IFR_ANY && d->int_enable)
		assert = 1;

	if (assert)
		INTERRUPT_ASSERT(d->irq);
	else if (d->int_asserted)
		INTERRUPT_DEASSERT(d->irq);

	d->int_asserted = assert;
}


/*
 *  adb_reset():
 *
 *  Reset registers to default values.
 */
static void adb_reset(struct adb_data *d)
{
	d->kbd_dev = 2;

	memset(d->reg, 0, sizeof(d->reg));
	d->reg[vBufB >> VIA_REG_SHIFT] = BUFB_nINTR | BUFB_nTIP;

	d->cur_output_offset = 0;
	memset(d->output_buf, 0, sizeof(d->output_buf));

	d->dir = 0;
	d->int_enable = 0;
	d->ack = 0;
	d->tip = 0;
}


/*
 *  adb_process_cmd():
 *
 *  This function should be called whenever a complete ADB command has been
 *  received.
 */
static void adb_process_cmd(struct cpu *cpu, struct adb_data *d)
{
	int i, reg, dev;

	debug("[ adb: COMMAND:");
	for (i=0; i<d->cur_output_offset; i++)
		debug(" %02x", d->output_buf[i]);
	debug(" ]\n");

	if (d->cur_output_offset < 2) {
		fatal("[ adb: WEIRD output length: %i ]\n",
		    d->cur_output_offset);
		exit(1);
	}

	switch (d->output_buf[0]) {

	case 0:	/*  ADB commands:  */
		if (d->output_buf[1] == 0x00) {
			/*  Reset.  */
			return;
		}
		if ((d->output_buf[1] & 0x0c) == 0x0c) {
			/*  ADBTALK:  */
			reg = d->output_buf[1] & 3;
			dev = d->output_buf[1] >> 4;
			fatal("dev=%i reg=%i\n", dev, reg);
			/*  Default values: nothing here  */
			d->input_buf[0] = 0x00;
			d->input_buf[1] = 0x00;
			d->input_buf[2] = d->output_buf[1];
			d->cur_input_length = 3;
			if (dev == d->kbd_dev) {
				/*  Keyboard.  */
				d->input_buf[0] = 0x01;
				d->input_buf[1] = 0x01;
				d->input_buf[2] = d->output_buf[1];
				d->input_buf[3] = 0x01;
				d->input_buf[4] = 0x01;
				d->cur_input_length = 5;
			}
		} else if ((d->output_buf[1] & 0x0c) == 0x08) {
			int new_dev_pos = d->output_buf[2] & 15;
			/*  ADBLISTEN:  */
			if ((d->output_buf[1] >> 4) != d->kbd_dev) {
				fatal("[ adb: ADBLISTEN not to kbd ]\n");
				exit(1);
			}
			if (d->output_buf[3] != 0xfe ||
			    (d->output_buf[2] & 0xf0) != 0x60) {
				fatal("[ adb: unknown ADBLISTEN ]\n");
				exit(1);
			}
			/*  Move device.  */
			d->kbd_dev = new_dev_pos;
		} else {
			fatal("[ adb: unknown ADB command? ]\n");
			exit(1);
		}
		break;

	case 1:	/*  PRAM/RTC:  */
		if (d->cur_output_offset == 3 &&
		    d->output_buf[1] == 0x01 &&
		    d->output_buf[2] == 0x01) {
			/*  Autopoll:  */
			d->input_buf[0] = 0x00;
			d->input_buf[1] = 0x00;
			d->input_buf[2] = d->output_buf[1];
			d->cur_input_length = 3;
		} else if (d->cur_output_offset == 2 &&
		    d->output_buf[1] == 0x03) {
			/*  Read RTC date/time:  */
			struct timeval tv;
			gettimeofday(&tv, NULL);
			d->input_buf[0] = tv.tv_sec >> 24;
			d->input_buf[1] = tv.tv_sec >> 16;
			d->input_buf[2] = tv.tv_sec >>  8;
			d->input_buf[3] = tv.tv_sec;
			d->cur_input_length = 4;
		} else if (d->cur_output_offset == 2 &&
		    d->output_buf[1] == 0x11) {
			/*  Reboot.  */
			fatal("[ adb: reboot. TODO: make this nicer ]\n");
			exit(1);
		} else {
			fatal("[ adb: UNIMPLEMENTED PRAM/RTC command ]\n");
			exit(1);
		}
		break;

	default:fatal("[ adb: UNKNOWN command type 0x%02x ]\n",
		    d->output_buf[0]);
		exit(1);
	}

	d->reg[vBufB >> VIA_REG_SHIFT] &= ~BUFB_nINTR;
	d->reg[vIFR >> VIA_REG_SHIFT] |= IFR_ANY | IFR_SR;
	d->reg[vSR >> VIA_REG_SHIFT] = 0x00;	/*  Dummy.  */
}


/*
 *  adb_transfer():
 *
 *  This function should be called whenever a new transfer is started, a
 *  transfer is finished, or when the next byte in a transfer should be
 *  sent/received.
 */
static void adb_transfer(struct cpu *cpu, struct adb_data *d, int state_change)
{
	unsigned char c = 0x00;

	if (state_change) {
		if (d->tip == 0) {
			debug("[ adb: transfer #%lli done ]\n",
			    (long long)d->transfer_nr);
			if (d->cur_output_offset > 0)
				adb_process_cmd(cpu, d);
			d->transfer_nr ++;
			return;
		}
		debug("[ adb: starting transfer #%lli: %s ]\n", (long long)
		    d->transfer_nr, d->dir == DIR_INPUT? "INPUT" : "OUTPUT");
		d->cur_input_offset = d->cur_output_offset = 0;
	}

	debug("[ adb: transfer #%lli: ", (long long)d->transfer_nr);

	switch (d->dir) {

	case DIR_INPUT:
		if (d->cur_input_offset >= d->cur_input_length)
			fatal("[ adb: INPUT beyond end of data? ]\n");
		else
			c = d->input_buf[d->cur_input_offset ++];
		debug("input 0x%02x", c);
		d->reg[vSR >> VIA_REG_SHIFT] = c;
		d->reg[vIFR >> VIA_REG_SHIFT] |= IFR_ANY | IFR_SR;
		if (d->cur_input_offset >= d->cur_input_length)
			d->reg[vBufB >> VIA_REG_SHIFT] |= BUFB_nINTR;
		break;

	case DIR_OUTPUT:
		c = d->reg[vSR >> VIA_REG_SHIFT];
		debug("output 0x%02x", c);
		d->reg[vIFR >> VIA_REG_SHIFT] |= IFR_ANY | IFR_SR;
		d->reg[vBufB >> VIA_REG_SHIFT] |= BUFB_nINTR;
		d->output_buf[d->cur_output_offset ++] = c;
		break;
	}

	debug(" ]\n");
}


DEVICE_ACCESS(adb)
{
	uint64_t idata = 0, odata = 0;
	struct adb_data *d = (struct adb_data *) extra;
	uint8_t old = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

#ifdef ADB_DEBUG
	if ((relative_addr & ((1 << VIA_REG_SHIFT) - 1)) != 0)
		fatal("[ adb: %s non-via register? offset 0x%x ]\n",
		    writeflag == MEM_READ? "read from" : "write to",
		    (int)relative_addr);
	else if (writeflag == MEM_READ)
		fatal("[ adb: read from %s: 0x%02x ]\n",
		    via_regname[relative_addr >> VIA_REG_SHIFT],
		    (int)d->reg[relative_addr >> VIA_REG_SHIFT]);
	else
		fatal("[ adb: write to %s: 0x%02x ]\n", via_regname[
		    relative_addr >> VIA_REG_SHIFT], (int)idata);
#endif

	if (writeflag == MEM_READ)
		odata = d->reg[relative_addr >> VIA_REG_SHIFT];
	else {
		old = d->reg[relative_addr >> VIA_REG_SHIFT];
		switch (relative_addr) {
		case vIFR:
			/*
			 *  vIFR is write-ones-to-clear, and the highest bit
			 *  (IFR_ANY) is set if any of the lower bits are set.
			 */
			d->reg[relative_addr >> VIA_REG_SHIFT] &= ~(idata|0x80);
			if (d->reg[relative_addr >> VIA_REG_SHIFT] & 0x7f)
				d->reg[relative_addr >> VIA_REG_SHIFT] |= 0x80;
			break;
		default:
			d->reg[relative_addr >> VIA_REG_SHIFT] = idata;
		}
	}

	switch (relative_addr) {

	case vBufB:
		/*  OK  */
		if (writeflag == MEM_WRITE) {
			int old_tip = d->tip;
			int old_ack = d->ack;
			if (idata & BUFB_nINTR)
				idata &= ~BUFB_nINTR;
			d->ack = 0;
			if (idata & BUFB_ACK) {
				idata &= ~BUFB_ACK;
				d->ack = 1;
			}
			d->tip = 1;
			if (idata & BUFB_nTIP) {
				idata &= ~BUFB_nTIP;
				d->tip = 0;
			}
			if (idata != 0)
				fatal("[ adb: WARNING! UNIMPLEMENTED bits in"
				    " vBufB: 0x%02x ]\n", (int)idata);
			if (old_tip != d->tip)
				adb_transfer(cpu, d, 1);
			else if (old_ack != d->ack)
				adb_transfer(cpu, d, 0);
		}
		break;

	case vDirB:
		break;

	case vSR:
		/*  Clear the SR interrupt flag, if set:  */
		d->reg[vIFR >> VIA_REG_SHIFT] &= ~IFR_SR;
		break;

	case vACR:
		/*  OK  */
		if (writeflag == MEM_WRITE) {
			if (idata & ACR_SR_OUT)
				d->dir = DIR_OUTPUT;
			else
				d->dir = DIR_INPUT;
		}
		break;

	case vIFR:
		/*  OK  */
		break;

	case vIER:
		/*  OK  */
		if (writeflag == MEM_WRITE) {
			d->int_enable = idata & 0x80? 1 : 0;
			if (idata != 0x04 && idata != 0x84)
				fatal("[ adb: WARNING! vIER value 0x%x is"
				    " UNKNOWN ]\n", (int)idata);
		}
		break;

	default:if ((relative_addr & ((1 << VIA_REG_SHIFT) - 1)) != 0)
			fatal("[ adb: %s non-via register? offset 0x%x ]\n",
			    writeflag == MEM_READ? "read from" : "write to",
			    (int)relative_addr);
		else if (writeflag == MEM_READ)
			fatal("[ adb: READ from UNIMPLEMENTED %s ]\n",
			    via_regname[relative_addr >> VIA_REG_SHIFT]);
		else
			fatal("[ adb: WRITE to UNIMPLEMENTED %s: 0x%x ]\n",
			    via_regname[relative_addr >> VIA_REG_SHIFT],
			    (int)idata);
		exit(1);
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(adb)
{
	struct adb_data *d;

	CHECK_ALLOCATION(d = (struct adb_data *) malloc(sizeof(struct adb_data)));
	memset(d, 0, sizeof(struct adb_data));

	INTERRUPT_CONNECT(devinit->interrupt_path, d->irq);

	adb_reset(d);

	memory_device_register(devinit->machine->memory, devinit->name,
	    devinit->addr, DEV_ADB_LENGTH, dev_adb_access, d, DM_DEFAULT, NULL);
	machine_add_tickfunction(devinit->machine, dev_adb_tick, d,
	    TICK_SHIFT);

	return 1;
}

