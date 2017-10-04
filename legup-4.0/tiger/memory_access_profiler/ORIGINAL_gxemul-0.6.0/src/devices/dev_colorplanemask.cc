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
 *  COMMENT: Color plane mask, used in the DECstation 3100 machine
 *
 *  Just a one-byte thingy, but the way things work now this has to
 *  be a separate device. :-/
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "devices.h"
#include "memory.h"
#include "misc.h"


struct colorplanemask_data {
	unsigned char	*color_plane_mask;
};


DEVICE_ACCESS(colorplanemask)
{
	struct colorplanemask_data *d = (struct colorplanemask_data *) extra;

	switch (relative_addr) {

	case 0x00:
		if (writeflag == MEM_WRITE) {
			*d->color_plane_mask = data[0];
		} else {
			debug("[ colorplanemask: read isn't actually "
			    "supported ]\n");
			data[0] = *d->color_plane_mask;
		}
		break;

	default:
		if (writeflag == MEM_WRITE) {
			debug("[ colorplanemask: unimplemented write "
			    "to address 0x%x, data=0x%02x ]\n",
			    (int)relative_addr, data[0]);
		} else {
			debug("[ colorplanemask: unimplemented read "
			    "from address 0x%x ]\n", (int)relative_addr);
		}
	}

	/*  Pretend it was ok:  */
	return 1;
}


/*
 *  dev_colorplanemask_init():
 */
void dev_colorplanemask_init(struct memory *mem, uint64_t baseaddr,
	unsigned char *color_plane_mask)
{
	struct colorplanemask_data *d;

	CHECK_ALLOCATION(d = (struct colorplanemask_data *)
	    malloc(sizeof(struct colorplanemask_data)));
	memset(d, 0, sizeof(struct colorplanemask_data));

	d->color_plane_mask = color_plane_mask;

	memory_device_register(mem, "colorplanemask", baseaddr,
	    DEV_COLORPLANEMASK_LENGTH, dev_colorplanemask_access,
	    (void *)d, DM_DEFAULT, NULL);
}

