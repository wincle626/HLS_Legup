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
 *  COMMENT: Framebuffer controller device (control's dev_fb in test machines)
 *
 *  A "framebuffer control" device. It can be used to manipulate the
 *  framebuffer device in testmachines.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "devices.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "testmachine/dev_fb.h"


struct fbctrl_data {
	struct vfb_data *vfb_data;
	int		current_port;
	int		port[DEV_FBCTRL_NPORTS];
};


/*
 *  fbctrl_command():
 *
 *  Execute a framebuffer control command.
 */
static void fbctrl_command(struct cpu *cpu, struct fbctrl_data *d)
{
	int cmd = d->port[DEV_FBCTRL_PORT_COMMAND];
	int x1, y1, i;
	struct machine *machine;

	switch (cmd) {

	case DEV_FBCTRL_COMMAND_NOP:
		break;

	case DEV_FBCTRL_COMMAND_SET_RESOLUTION:
		/*
		 *  Change the framebuffer resolution to X1 x Y1 pixels.
		 */
		x1 = d->port[DEV_FBCTRL_PORT_X1];
		y1 = d->port[DEV_FBCTRL_PORT_Y1];
		debug("[ dev_fbctrl: changing resolution to %i,%i ]\n", x1, y1);

		dev_fb_resize(d->vfb_data, x1, y1);

		/*  Remember to invalidate all translations for anyone
		    who might have used the old framebuffer:  */
		machine = cpu->machine;
		for (i=0; i<machine->ncpus; i++)
			machine->cpus[i]->invalidate_translation_caches(
			    machine->cpus[i], 0, INVALIDATE_ALL);
		break;

	case DEV_FBCTRL_COMMAND_GET_RESOLUTION:
		d->port[DEV_FBCTRL_PORT_X1] = d->vfb_data->xsize;
		d->port[DEV_FBCTRL_PORT_Y1] = d->vfb_data->ysize;
		break;

	/*  TODO: Block copy and fill.  */

	default:fatal("fbctrl_command: Unimplemented command %i.\n", cmd);
		exit(1);
	}
}


DEVICE_ACCESS(fbctrl)
{
	struct fbctrl_data *d = (struct fbctrl_data *) extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
	        idata = memory_readmax64(cpu, data, len);

	switch (relative_addr) {

	case DEV_FBCTRL_PORT:
		if (writeflag == MEM_READ)
			odata = d->current_port;
		else {
			d->current_port = idata;
			if (idata >= DEV_FBCTRL_NPORTS)
				fatal("[ WARNING: fbctrl port number is out"
				    " of range! ]\n");
		}
		break;

	case DEV_FBCTRL_DATA:
		if (d->current_port < 0 || d->current_port
		    >= DEV_FBCTRL_NPORTS) {
			fatal("[ fbctrl port number is out of range! ]\n");
			exit(1);
		}

		if (writeflag == MEM_READ)
			odata = d->port[d->current_port];
		else {
			d->port[d->current_port] = idata;
			if (d->current_port == DEV_FBCTRL_PORT_COMMAND)
				fbctrl_command(cpu, d);
		}

		break;

	default:
		fatal("[ dev_fbctrl: unimplemented relative addr 0x%x ]\n",
		    (int)relative_addr);
		exit(1);
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(fbctrl)
{
	struct fbctrl_data *d;

	CHECK_ALLOCATION(d = (struct fbctrl_data *) malloc(sizeof(struct fbctrl_data)));
	memset(d, 0, sizeof(struct fbctrl_data));

	d->vfb_data = dev_fb_init(devinit->machine, devinit->machine->memory,
	    DEV_FB_ADDRESS, VFB_GENERIC, 640, 480, 640, DEV_FBCTRL_MAXY(640),
	    24, "generic");

	memory_device_register(devinit->machine->memory, devinit->name,
	    devinit->addr, DEV_FBCTRL_LENGTH, dev_fbctrl_access, d,
	    DM_DEFAULT, NULL);

	return 1;
}

