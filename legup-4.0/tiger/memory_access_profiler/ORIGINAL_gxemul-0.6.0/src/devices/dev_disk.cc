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
 *  COMMENT: A simple disk controller device, for the test machines
 *
 *  Basic "disk" device. This is a simple test device which can be used to
 *  read and write data from disk devices.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "device.h"
#include "diskimage.h"
#include "emul.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"

#include "testmachine/dev_disk.h"


struct disk_data {
	uint64_t	offset;
	int		disk_id;
	int		command;
	int		status;
	unsigned char	*buf;
};


DEVICE_ACCESS(disk_buf)
{
	struct disk_data *d = (struct disk_data *) extra;

	if (writeflag == MEM_WRITE)
		memcpy(d->buf + relative_addr, data, len);
	else
		memcpy(data, d->buf + relative_addr, len);

	return 1;
}


DEVICE_ACCESS(disk)
{
	struct disk_data *d = (struct disk_data *) extra;
	uint64_t idata = 0, odata = 0;

	if (writeflag == MEM_WRITE)
		idata = memory_readmax64(cpu, data, len);

	switch (relative_addr) {

	case DEV_DISK_OFFSET:
		if (writeflag == MEM_READ) {
			odata = d->offset;
		} else {
			d->offset = idata;
		}
		break;

	case DEV_DISK_OFFSET_HIGH32:
		if (writeflag == MEM_READ) {
			odata = d->offset >> 32;
		} else {
			d->offset = (uint32_t)d->offset | (idata << 32);
		}
		break;

	case DEV_DISK_ID:
		if (writeflag == MEM_READ) {
			odata = d->disk_id;
		} else {
			d->disk_id = idata;
		}
		break;

	case DEV_DISK_START_OPERATION:
		if (writeflag == MEM_READ) {
			odata = d->command;
		} else {
			d->command = idata;
			switch (d->command) {
			case 0:	d->status = diskimage_access(cpu->machine,
				     d->disk_id, DISKIMAGE_IDE, 0,
				     d->offset, d->buf, 512);
				break;
			case 1:	d->status = diskimage_access(cpu->machine,
				     d->disk_id, DISKIMAGE_IDE, 1,
				     d->offset, d->buf, 512);
				break;
			}
		}
		break;

	case DEV_DISK_STATUS:
		if (writeflag == MEM_READ) {
			odata = d->status;
		} else {
			d->status = idata;
		}
		break;

	default:if (writeflag == MEM_WRITE) {
			fatal("[ disk: unimplemented write to "
			    "offset 0x%x: data=0x%x ]\n", (int)
			    relative_addr, (int)idata);
		} else {
			fatal("[ disk: unimplemented read from "
			    "offset 0x%x ]\n", (int)relative_addr);
		}
	}

	if (writeflag == MEM_READ)
		memory_writemax64(cpu, data, len, odata);

	return 1;
}


DEVINIT(disk)
{
	struct disk_data *d;
	size_t nlen;
	char *n1, *n2;
                 
	CHECK_ALLOCATION(d = (struct disk_data *) malloc(sizeof(struct disk_data)));
	memset(d, 0, sizeof(struct disk_data));

	nlen = strlen(devinit->name) + 30;
	CHECK_ALLOCATION(n1 = (char *) malloc(nlen));
	CHECK_ALLOCATION(n2 = (char *) malloc(nlen));

	CHECK_ALLOCATION(d->buf = (unsigned char *) malloc(devinit->machine->arch_pagesize));
	memset(d->buf, 0, devinit->machine->arch_pagesize);

	snprintf(n1, nlen, "%s [control]", devinit->name);
	snprintf(n2, nlen, "%s [data buffer]", devinit->name);

	memory_device_register(devinit->machine->memory, n1,
	    devinit->addr, DEV_DISK_BUFFER, dev_disk_access, (void *)d,
	    DM_DEFAULT, NULL);

	memory_device_register(devinit->machine->memory, n2,
	    devinit->addr + DEV_DISK_BUFFER,
	    devinit->machine->arch_pagesize, dev_disk_buf_access,
	    (void *)d, DM_DYNTRANS_OK | DM_DYNTRANS_WRITE_OK |
	    DM_READS_HAVE_NO_SIDE_EFFECTS, d->buf);

	return 1;
}

