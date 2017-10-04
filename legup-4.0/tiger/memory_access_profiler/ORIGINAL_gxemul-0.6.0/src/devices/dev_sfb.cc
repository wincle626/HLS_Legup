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
 *  COMMENT: SFB TURBOchannel framebuffer (graphics card)
 *
 *  See include/sfbreg.h (and NetBSD's arch/pmax/dev/sfb.c) for more info.
 *
 *
 *  TODO:  This is not really implemented yet. It "happens" to work with
 *         NetBSD and OpenBSD, but not with Ultrix yet.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "devices.h"
#include "memory.h"
#include "misc.h"

#include "thirdparty/sfbreg.h"


#define	SFB_XSIZE	1280
#define	SFB_YSIZE	1024


/*  #define debug fatal  */


#define	SFB_REG_SIZE	0x80
#define	N_SFB_REGS	(SFB_REG_SIZE / 4)


struct sfb_data {
	struct vfb_data	*vfb_data;

	uint32_t	reg[N_SFB_REGS];
};


DEVICE_ACCESS(sfb)
{
	uint64_t idata = 0, odata = 0;
	struct sfb_data *d = (struct sfb_data *) extra;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	if (writeflag==MEM_READ) {
		odata = d->reg[(relative_addr >> 2) & (N_SFB_REGS - 1)];
		debug("[ sfb: read from addr 0x%x: 0x%llx ]\n",
		    (int)relative_addr, (long long)odata);
	} else {
		d->reg[(relative_addr >> 2) & (N_SFB_REGS - 1)] = idata;
		debug("[ sfb: write to addr 0x%x: 0x%llx ]\n",
		    (int)relative_addr, (long long)idata);
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


/*
 *  dev_sfb_init():
 */
void dev_sfb_init(struct machine *machine, struct memory *mem,
	uint64_t baseaddr, struct vfb_data *fb)
{
	struct sfb_data *d;

	CHECK_ALLOCATION(d = (struct sfb_data *) malloc(sizeof(struct sfb_data)));
	memset(d, 0, sizeof(struct sfb_data));

	d->vfb_data = fb;

	d->reg[(SFB_VHORIZONTAL - SFB_ASIC_OFFSET) / 4] = SFB_XSIZE >> 2;
	d->reg[(SFB_VVERTICAL - SFB_ASIC_OFFSET) / 4]   = SFB_YSIZE;

	memory_device_register(mem, "sfb", baseaddr, SFB_REG_SIZE,
	    dev_sfb_access, d, DM_DEFAULT, NULL);
}

