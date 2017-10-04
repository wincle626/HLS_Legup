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
 *  COMMENT: PlayStation 2 graphics system
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "device.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"


#define	DEV_PS2_GS_LENGTH		0x2000

/*  NOTE/TODO: This should be the same as in ps2_stuff:  */
#define	DEV_PS2_GIF_FAKE_BASE		0x50000000

#define	N_GS_REGS	0x108

struct gs_data {
	uint64_t	reg[N_GS_REGS];		/*  GS registers  */
};

#define	GS_S_PMODE_REG		0x00
#define	GS_S_SMODE1_REG		0x01
#define	GS_S_SMODE2_REG		0x02
#define	GS_S_SRFSH_REG		0x03
#define	GS_S_SYNCH1_REG		0x04
#define	GS_S_SYNCH2_REG		0x05
#define	GS_S_SYNCV_REG		0x06
#define	GS_S_DISPFB1_REG	0x07
#define	GS_S_DISPLAY1_REG	0x08
#define	GS_S_DISPFB2_REG	0x09
#define	GS_S_DISPLAY2_REG	0x0a
#define	GS_S_EXTBUF_REG0	0x0b
#define	GS_S_EXTDATA_REG	0x0c
#define	GS_S_EXTWRITE_REG	0x0d
#define	GS_S_BGCOLOR_REG	0x0e

//static const char *gs_reg_names[15] = {
//	"PMODE", "SMODE1", "SMODE2", "SRFSH",
//	"SYNCH1", "SYNCH2", "SYNCV", "DISPFB1",
//	"DISPLAY1", "DISPFB2", "DISPLAY2", "EXTBUF",
//	"EXTDATA", "EXTWRITE", "BGCOLOR"  };

#define	GS_S_CSR_REG		0x100
#define	GS_S_IMR_REG		0x101
#define	GS_S_BUSDIR_REG		0x104
#define	GS_S_SIGLBLID_REG	0x108

//static const char *gs_reg_high_names[4] = {
//	"CSR", "IMR", "BUSDIR", "SIGLBLID"
//	};


DEVICE_ACCESS(ps2_gs)
{
	struct gs_data *d = (struct gs_data *) extra;
	uint64_t idata = 0, odata = 0;
	size_t i;
	int regnr;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	regnr = relative_addr / 16;
	if (relative_addr & 0xf) {
		debug("[ gs unaligned access, addr 0x%x ]\n",
		    (int)relative_addr);
		return 0;
	}

	/*  Empty the GS FIFO:  */
	d->reg[GS_S_CSR_REG] &= ~(3 << 14);
	d->reg[GS_S_CSR_REG] |=  (1 << 14);

	switch (relative_addr) {
	default:
		if (writeflag==MEM_READ) {
			debug("[ gs read from addr 0x%x ]\n",
			    (int)relative_addr);
			odata = d->reg[regnr];
		} else {
			debug("[ gs write to addr 0x%x:", (int)relative_addr);
			for (i=0; i<len; i++)
				debug(" %02x", data[i]);
			debug(" ]\n");
			d->reg[regnr] = idata;
			return 1;
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


/*
 *  devinit_ps2_gs():
 *
 *  Initialize the Playstation 2 graphics system. This is a bit tricky.
 *  'gs' is the memory mapped device, as seen by the main processor.
 *  'gif' is another thing which has its own memory.  DMA is used to
 *  transfer stuff to the gif from the main memory.
 *  There is also a framebuffer, which is separated from the main
 *  memory.
 *
 *  TODO:  Make this clearer.
 */
DEVINIT(ps2_gs)
{
	struct gs_data *d;
	char str[100];

	CHECK_ALLOCATION(d = (struct gs_data *) malloc(sizeof(struct gs_data)));
	memset(d, 0, sizeof(struct gs_data));

	snprintf(str, sizeof(str) - 1, "ps2_gif addr=0x%llx",
	    (long long)DEV_PS2_GIF_FAKE_BASE);
	device_add(devinit->machine, str);

	memory_device_register(devinit->machine->memory, devinit->name,
	    devinit->addr, DEV_PS2_GS_LENGTH, dev_ps2_gs_access, d,
	    DM_DEFAULT, NULL);

	return 1;
}

