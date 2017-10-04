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
 *  COMMENT: Fujitsu MB8696x Ethernet interface (used in the Dreamcast)
 *
 *  Used as the LAN adapter (MB86967) in the Dreamcast machine mode.
 *
 *
 *  Note: This is just a bogus module so far.
 *
 *  (TODO: "Reverse engineer" more of NetBSD's mb86960.c to implement this.)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "cpu.h"
#include "device.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"
#include "net.h"

#include "thirdparty/mb86960reg.h"


#ifdef UNSTABLE_DEVEL
#define	MB8696X_DEBUG
#endif

#ifdef MB8696X_DEBUG
#define debug fatal
#endif

struct mb8696x_data {
	int		addr_mult;

	/*
	 *  Registers:
	 *
	 *  reg contains 32 registers. However, registers 8..15 are bank-
	 *  switched, based on a setting in FE_DLCR7. mar_8_15 are "multicast
	 *  address registers", and bmpr_8_15 are "buffer memory port
	 *  registers".
	 */
	uint8_t		reg[MB8696X_NREGS];
	uint8_t		mar_8_15[8];
	uint8_t		bmpr_8_15[8];

	/*  EEPROM contents and internal state during a read operation:  */
	uint8_t		eeprom[FE_EEPROM_SIZE];
	int		eeprom_state;
	uint8_t		eeprom_bit_count;
	uint8_t		eeprom_command;
	uint16_t	eeprom_data;
};

#define	EEPROM_STATE_NOTHING	0
#define	EEPROM_STATE_READY	1	/*  Waiting for start bit  */
#define	EEPROM_STATE_COMMAND	2	/*  Waiting for 8 command bits  */
#define	EEPROM_STATE_READ	3


DEVICE_ACCESS(mb8696x)
{
	struct mb8696x_data *d = (struct mb8696x_data *) extra;
	uint64_t idata = 0, odata = 0;
	uint8_t *reg_ptr;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	relative_addr /= d->addr_mult;

	/*
	 *  Default result on reads:  (Note special treatment of banked regs.)
	 */
	reg_ptr = &d->reg[relative_addr];
	if (relative_addr >= 8 && relative_addr <= 15 &&
	    (d->reg[FE_DLCR7] & FE_D7_RBS) == FE_D7_RBS_MAR)
		reg_ptr = &d->mar_8_15[relative_addr - 8];
	if (relative_addr >= 8 && relative_addr <= 15 &&
	    (d->reg[FE_DLCR7] & FE_D7_RBS) == FE_D7_RBS_BMPR)
		reg_ptr = &d->bmpr_8_15[relative_addr - 8];

	odata = *reg_ptr;


	switch (relative_addr) {

	case FE_DLCR0:	/*  TX interrupt status  */
	case FE_DLCR1:	/*  RX interrupt status  */
		/*  Write-1-to-clear:  */
		if (writeflag == MEM_WRITE)
			(*reg_ptr) &= ~idata;

/*  TODO: reassert interrupts  */

		break;

	case FE_DLCR2:	/*  TX interrupt control  */
	case FE_DLCR3:	/*  RX interrupt control  */
		if (writeflag == MEM_WRITE)
			(*reg_ptr) = idata;

/*  TODO: reassert interrupts  */

		break;

	case FE_DLCR6:
	case FE_DLCR8:	/*  Ethernet addr byte 0  */
	case FE_DLCR9:	/*  Ethernet addr byte 1  */
	case FE_DLCR10:	/*  Ethernet addr byte 2  */
	case FE_DLCR11:	/*  Ethernet addr byte 3  */
	case FE_DLCR12:	/*  Ethernet addr byte 4  */
	case FE_DLCR13:	/*  Ethernet addr byte 5  */
		if (writeflag == MEM_WRITE)
			(*reg_ptr) = idata;
		break;

	case FE_DLCR7:
		if (writeflag == MEM_WRITE) {
			/*  Identification cannot be overwritten:  */
			(*reg_ptr) &= FE_D7_IDENT;
			(*reg_ptr) |= (idata & ~FE_D7_IDENT);
		}
		break;

	case FE_BMPR16:
		/*  EEPROM control  */
		if (writeflag == MEM_WRITE) {
			if (idata & ~(FE_B16_DOUT | FE_B16_DIN |
			    FE_B16_SELECT | FE_B16_CLOCK)) {
				fatal("mb8696x: UNIMPLEMENTED bits when "
				    "writing to FE_BMPR16: 0x%02x\n",
				    (int)idata);
				exit(1);
			}

			/*  Dropped out of select state?  */
			if (!(idata & FE_B16_SELECT))
				d->eeprom_state = EEPROM_STATE_NOTHING;

			/*  Switching to select state?  */
			if (!((*reg_ptr) & FE_B16_SELECT) &&
			    idata & FE_B16_SELECT)
				d->eeprom_state = EEPROM_STATE_READY;

			/*  Bit clock?  */
			if (!((*reg_ptr) & FE_B16_CLOCK) &&
			    idata & FE_B16_CLOCK) {
				int bit = d->reg[FE_BMPR17] & FE_B17_DATA? 1:0;
				switch (d->eeprom_state) {
				case EEPROM_STATE_READY:
					d->eeprom_state = EEPROM_STATE_COMMAND;
					d->eeprom_bit_count = 0;
					break;
				case EEPROM_STATE_COMMAND:
					d->eeprom_bit_count ++;
					d->eeprom_command <<= 1;
					d->eeprom_command |= bit;
					if (d->eeprom_bit_count == 8) {
						int addr = (d->eeprom_command
						    & 0x7f) << 1;
						/*  printf("COMMAND=%08x\n",
						    d->eeprom_command);  */
						if (!(d->eeprom_command&0x80)) {
							fatal("WRITES to the "
							    "EEPROM are not yet"
							    " implemented.\n");
							exit(1);
						}
						/*  This is a read command.  */
						d->eeprom_bit_count = 0;
						d->eeprom_state =
						    EEPROM_STATE_READ;
						d->eeprom_data = d->eeprom[addr]
						    * 256 + d->eeprom[addr+1];
					}
					break;
				case EEPROM_STATE_READ:
					d->reg[FE_BMPR17] = 0;
					if (d->eeprom_data & 0x8000)
						d->reg[FE_BMPR17] = FE_B17_DATA;
					d->eeprom_data <<= 1;
					d->eeprom_bit_count ++;
					if (d->eeprom_bit_count > 16)
						fatal("[ WARNING: more than 16"
						    " bits of EEPROM data "
						    "read? ]\n");
					break;
				}
			}

			(*reg_ptr) = idata;
		}
		break;

	case FE_BMPR17:
		/*  EEPROM data  */
		if (writeflag == MEM_WRITE) {
			if (idata & ~FE_B17_DATA) {
				fatal("mb8696x: UNIMPLEMENTED bits when "
				    "writing to FE_BMPR17: 0x%02x\n",
				    (int)idata);
				exit(1);
			}
			(*reg_ptr) = idata;
		}
		break;

	default:
		{
			const char *bank = "";
			if ((d->reg[FE_DLCR7] & FE_D7_RBS) == FE_D7_RBS_MAR)
				bank = " (bank MAR)";
			if ((d->reg[FE_DLCR7] & FE_D7_RBS) == FE_D7_RBS_BMPR)
				bank = " (bank BMPR)";
			if (writeflag == MEM_READ) {
				fatal("[ mb8696x: read from UNIMPLEMENTED reg "
				    "%i%s ]\n", (int)relative_addr, bank);
			} else {
				fatal("[ mb8696x: write to UNIMPLEMENTED reg "
				    "%i%s: 0x%02x ]\n", (int)relative_addr,
				    bank, (int)idata);
			}

#ifdef MB8696X_DEBUG
			exit(1);
#endif
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(mb8696x)
{
	struct mb8696x_data *d;

	CHECK_ALLOCATION(d = (struct mb8696x_data *) malloc(sizeof(struct mb8696x_data)));
	memset(d, 0, sizeof(struct mb8696x_data));

	d->addr_mult = devinit->addr_mult;

	memory_device_register(devinit->machine->memory, devinit->name,
	    devinit->addr, MB8696X_NREGS * d->addr_mult, dev_mb8696x_access, d,
	    DM_DEFAULT, NULL);

	/*  NetBSD/dreamcast expects ident = 86967.  */
	d->reg[FE_DLCR7] = FE_D7_IDENT_86967;

	/*
	 *  Generate the MAC address, both in the first 6 bytes of the
	 *  EEPROM, and in DLCR8..13:
	 */
	net_generate_unique_mac(devinit->machine, &d->eeprom[0]);
	memcpy(&d->reg[FE_DLCR8], &d->eeprom[0], 6);

	return 1;
}

