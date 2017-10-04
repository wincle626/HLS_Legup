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
 *  COMMENT: Dreamcast Real-Time Clock
 *
 *  Pretty basic; two 32-bit words at physical addresses 0x00710000 and
 *  0x00710004 hold the high and low 16-bit parts, respectively, of the
 *  system's 32-bit tv_sec value.
 *
 *  The only difference from the raw Unix concept is that the Dreamcast's
 *  clock is based at 1950 instead of 1970.
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


/*  #define debug fatal  */

struct dreamcast_rtc_data {
	int		dummy;
};


DEVICE_ACCESS(dreamcast_rtc)
{
	/*  struct dreamcast_rtc_data *d = extra;  */
	uint64_t idata = 0, odata = 0;
	struct timeval tv;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	switch (relative_addr) {

	case 0:
	case 4:
		if (writeflag == MEM_WRITE)
			debug("[ dreamcast_rtc: Writes are ignored, only "
			    "reads are supported. ]\n");

		gettimeofday(&tv, NULL);

		/*  Offset by 20 years:  */
		odata = tv.tv_sec + 631152000;

		if (relative_addr == 0)
			odata = (odata >> 16) & 0xffff;
		else
			odata &= 0xffff;
		break;

	default:
		if (writeflag == MEM_READ) {
			fatal("[ dreamcast_rtc: read from addr 0x%x ]\n",
			    (int)relative_addr);
		} else {
			fatal("[ dreamcast_rtc: write to addr 0x%x: 0x%x ]\n",
			    (int)relative_addr, (int)idata);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(dreamcast_rtc)
{
	struct dreamcast_rtc_data *d;

	CHECK_ALLOCATION(d = (struct dreamcast_rtc_data *) malloc(sizeof(struct dreamcast_rtc_data)));
	memset(d, 0, sizeof(struct dreamcast_rtc_data));

	memory_device_register(devinit->machine->memory, devinit->name,
	    0x00710000, 0x100, dev_dreamcast_rtc_access, d, DM_DEFAULT, NULL);

	return 1;
}

