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
 *  COMMENT: Realtek 8139 ethernet controller
 *
 *  TODO: Pretty much everything.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "emul.h"
#include "interrupt.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"
#include "net.h"

#include "thirdparty/rtl81x9reg.h"


#define	DEV_RTL8139C_LENGTH	0x100
#define	EEPROM_SIZE		0x100

struct rtl8139c_data {
	struct interrupt	irq;
	unsigned char		macaddr[6];

	/*  Registers:  */
	uint8_t			rl_command;
	uint8_t			rl_eecmd;

	/*  EEPROM:  */
	int			eeprom_address_width;
	int			eeprom_selected;
	int8_t			eeprom_cur_cmd_bit;
	uint16_t		eeprom_cur_cmd;
	uint16_t		eeprom_cur_data;
	uint16_t		eeprom_reg[EEPROM_SIZE];
};


/*
 *  eeprom_clk():
 *
 *  Called whenever the eeprom CLK bit is toggled from 0 to 1.
 */
static void eeprom_clk(struct rtl8139c_data *d)
{
	int data_in = d->rl_eecmd & RL_EE_DATAIN? 1 : 0;

	if (d->eeprom_cur_cmd_bit < d->eeprom_address_width + 4) {
		d->eeprom_cur_cmd <<= 1;
		d->eeprom_cur_cmd |= data_in;
	}

	if (d->eeprom_cur_cmd_bit == d->eeprom_address_width + 3) {
		int cmd = d->eeprom_cur_cmd >> d->eeprom_address_width;
		int addr = d->eeprom_cur_cmd & ((1<<d->eeprom_address_width)-1);

		debug("[ rtl8139c eeprom cmd=0x%x addr=0x%02x ]\n", cmd, addr);

		switch (cmd) {

		case RL_9346_READ:
			d->eeprom_cur_data = d->eeprom_reg[addr % EEPROM_SIZE];
			break;

		default:fatal("[ rtl8139c eeprom: only the read command has"
			    " been implemented. sorry. ]\n");
			exit(1);
		}
	}

	/*  Data output: (Note: Only the READ command has been implemented.)  */
	if (d->eeprom_cur_cmd_bit >= d->eeprom_address_width + 4) {
		int cur_out_bit = d->eeprom_cur_cmd_bit -
		    (d->eeprom_address_width + 4);
		int bit = d->eeprom_cur_data & (1 << (15-cur_out_bit));

		if (bit)
			d->rl_eecmd |= RL_EE_DATAOUT;
		else
			d->rl_eecmd &= ~RL_EE_DATAOUT;
	}

	d->eeprom_cur_cmd_bit ++;

	if (d->eeprom_cur_cmd_bit >= d->eeprom_address_width + 4 + 16) {
		d->eeprom_cur_cmd = 0;
		d->eeprom_cur_cmd_bit = 0;
	}
}


DEVICE_ACCESS(rtl8139c)
{
	struct rtl8139c_data *d = (struct rtl8139c_data *) extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	switch (relative_addr) {

	case RL_COMMAND:
		if (writeflag == MEM_WRITE) {
			if (idata & RL_CMD_RESET) {
				/*  Reset. TODO  */

				/*  ... and then clear the reset bit:  */
				idata &= ~RL_CMD_RESET;
			}

			d->rl_command = idata;
		} else {
			odata = d->rl_command;
		}
		break;

	case RL_EECMD:
		if (writeflag == MEM_WRITE) {
			uint8_t old = d->rl_eecmd;
			d->rl_eecmd = idata;

			if (!d->eeprom_selected && d->rl_eecmd & RL_EE_SEL) {
				/*  Reset eeprom cmd bit state:  */
				d->eeprom_cur_cmd = 0;
				d->eeprom_cur_cmd_bit = 0;
			}
			d->eeprom_selected = d->rl_eecmd & RL_EE_SEL;

			if (idata & RL_EE_CLK && !(old & RL_EE_CLK))
				eeprom_clk(d);
		} else {
			odata = d->rl_eecmd;
		}
		break;

	case 0x82:
		/*  Unknown address, but OpenBSD's re driver writes
		    a 0x01 to this address, in re_reset().  */
		if (writeflag == MEM_WRITE) {
			if (idata != 0x01) {
				fatal("rtl8139c: unimplemented write to"
				    " register 0x82.\n");
				exit(1);
			}
		}
		break;

	default:if (writeflag == MEM_WRITE) {
			fatal("[ rtl8139c: unimplemented write to "
			    "offset 0x%x: data=0x%x ]\n", (int)
			    relative_addr, (int)idata);
		} else {
			fatal("[ rtl8139c: unimplemented read from "
			    "offset 0x%x ]\n", (int)relative_addr);
		}
		exit(1);
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(rtl8139c)
{
	char *name2;
	size_t nlen = 100;
	struct rtl8139c_data *d;

	CHECK_ALLOCATION(d = (struct rtl8139c_data *) malloc(sizeof(struct rtl8139c_data)));
	memset(d, 0, sizeof(struct rtl8139c_data));

	INTERRUPT_CONNECT(devinit->interrupt_path, d->irq);

	net_generate_unique_mac(devinit->machine, d->macaddr);

	/*  TODO: eeprom address width = 6 on 8129?  */
	d->eeprom_address_width = 8;
	d->eeprom_reg[0] = 0x8139;
	d->eeprom_reg[7] = d->macaddr[0] + (d->macaddr[1] << 8);
	d->eeprom_reg[8] = d->macaddr[2] + (d->macaddr[3] << 8);
	d->eeprom_reg[9] = d->macaddr[4] + (d->macaddr[5] << 8);

	CHECK_ALLOCATION(name2 = (char *) malloc(nlen));
	snprintf(name2, nlen, "%s [%02x:%02x:%02x:%02x:%02x:%02x]",
	    devinit->name, d->macaddr[0], d->macaddr[1], d->macaddr[2],
	    d->macaddr[3], d->macaddr[4], d->macaddr[5]);

	memory_device_register(devinit->machine->memory, name2,
	    devinit->addr, DEV_RTL8139C_LENGTH, dev_rtl8139c_access, (void *)d,
	    DM_DEFAULT, NULL);

	net_add_nic(devinit->machine->emul->net, d, d->macaddr);

	return 1;
}

