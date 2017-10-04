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
 *  COMMENT: Brooktree BT431, used by TURBOchannel graphics cards
 *
 *  TODO.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "devices.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/bt431reg.h"


struct bt431_data {
	uint32_t	bt431_reg[DEV_BT431_NREGS];

	unsigned char	cur_addr_hi;
	unsigned char	cur_addr_lo;

	int		planes;
	int		cursor_on;
	int		cursor_x;
	int		cursor_y;
	int		cursor_xsize;
	int		cursor_ysize;

	struct vfb_data *vfb_data;
};


DEVICE_ACCESS(bt431)
{
	struct bt431_data *d = (struct bt431_data *) extra;
	uint64_t idata = 0, odata = 0;
	int btaddr;
#if 0
	int on, new_cursor_x, new_cursor_y;
#endif

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	btaddr = ((d->cur_addr_hi << 8) + d->cur_addr_lo) % DEV_BT431_NREGS;

/*  TODO  */

	/*  Read from/write to the bt431:  */
	switch (relative_addr) {
	case 0x00:		/*  Low byte of address:  */
		if (writeflag == MEM_WRITE) {
			debug("[ bt431: write to Low Address Byte, "
			    "0x%02x ]\n", (int)idata);
			d->cur_addr_lo = idata;
		} else {
			odata = d->cur_addr_lo;
			debug("[ bt431: read from Low Address Byte: "
			    "0x%0x ]\n", (int)odata);
		}
		break;
	case 0x04:		/*  High byte of address:  */
		if (writeflag == MEM_WRITE) {
			debug("[ bt431: write to High Address Byte, "
			    "0x%02x ]\n", (int)idata);
			d->cur_addr_hi = idata;
		} else {
			odata = d->cur_addr_hi;
			debug("[ bt431: read from High Address Byte: "
			    "0x%0x ]\n", (int)odata);
		}
		break;
	case 0x08:		/*  Register access:  */
		if (writeflag == MEM_WRITE) {
			debug("[ bt431: write to BT431 register 0x%04x, "
			    "value 0x%02x ]\n", btaddr, (int)idata);
			d->bt431_reg[btaddr] = idata;

#if 0
			/*  Write to cursor bitmap:  */
			if (btaddr >= 0x400)
				bt431_sync_xysize(d);
#endif
		} else {
			odata = d->bt431_reg[btaddr];
			debug("[ bt431: read from BT431 register 0x%04x, "
			    "value 0x%02x ]\n", btaddr, (int)odata);
		}

		/*  Go to next register:  */
		d->cur_addr_lo ++;
		if (d->cur_addr_lo == 0)
			d->cur_addr_hi ++;
		break;
	default:
		if (writeflag == MEM_WRITE) {
			debug("[ bt431: unimplemented write to address "
			    "0x%x, data=0x%02x ]\n", (int)relative_addr,
			    (int)idata);
		} else {
			debug("[ bt431: unimplemented read from address "
			    "0x%x ]\n", (int)relative_addr);
		}
	}

#if 0

TODO: This is from bt459!

	/*  NetBSD uses 370,37 as magic values.  */
	new_cursor_x = (d->bt431_reg[BT431_REG_CXLO] & 255) +
	    ((d->bt431_reg[BT431_REG_CXHI] & 255) << 8) - 370;
	new_cursor_y = (d->bt431_reg[BT431_REG_CYLO] & 255) +
	    ((d->bt431_reg[BT431_REG_CYHI] & 255) << 8) - 37;

	/*  TODO: what do the bits in the CCR do?  */
	on = d->bt431_reg[BT431_REG_CCR] ? 1 : 0;

on = 1;

	if (new_cursor_x != d->cursor_x || new_cursor_y != d->cursor_y ||
	    on != d->cursor_on) {
		int ysize_mul = 1;

		d->cursor_x = new_cursor_x;
		d->cursor_y = new_cursor_y;
		d->cursor_on = on;

		/*
		 *  Ugly hack for Ultrix:
		 *
		 *  Ultrix and NetBSD assume that the cursor works differently.
		 *  Ultrix uses the 370,38 coordinates, but draws the cursor
		 *  upwards. NetBSD draws it downwards.  Ultrix also makes the
		 *  cursor smaller (?).
		 *
		 *  TODO:  This actually depends on which ultrix kernel you use.
		 *  Clearly, the BT459 emulation is not implemented well
		 *  enough yet.
		 *
		 *  TODO:  Find out why? Is it because of special BT459
		 *  commands?
		 *
		 *  TODO: Is the above text even valid anymore? :-)
		 */
		if (!(d->bt431_reg[BT431_REG_CCR] & 1)) {
/*			ysize_mul = 4; */
			d->cursor_y += 5 - (d->cursor_ysize * ysize_mul);
		}

		debug("[ bt431: cursor = %03i,%03i ]\n", d->cursor_x,
		    d->cursor_y);
		dev_fb_setcursor(d->vfb_data, d->cursor_x, d->cursor_y, on,
		    d->cursor_xsize, d->cursor_ysize * ysize_mul);
	}
#endif

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


/*
 *  dev_bt431_init():
 */
void dev_bt431_init(struct memory *mem, uint64_t baseaddr,
	struct vfb_data *vfb_data, int planes)
{
	struct bt431_data *d;

	CHECK_ALLOCATION(d = (struct bt431_data *) malloc(sizeof(struct bt431_data)));
	memset(d, 0, sizeof(struct bt431_data));

	d->vfb_data     = vfb_data;
	d->planes       = planes;
	d->cursor_x     = -1;
	d->cursor_y     = -1;
	d->cursor_xsize = d->cursor_ysize = 8;	/*  anything  */

	memory_device_register(mem, "bt431", baseaddr, DEV_BT431_LENGTH,
	    dev_bt431_access, (void *)d, DM_DEFAULT, NULL);
}

