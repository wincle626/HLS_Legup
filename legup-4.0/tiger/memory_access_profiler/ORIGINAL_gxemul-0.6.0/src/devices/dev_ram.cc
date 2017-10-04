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
 *  COMMENT: A generic RAM (memory) device
 *
 *  Note: This device can also be used to mirror/alias another part of RAM.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>

#include "cpu.h"
#include "devices.h"
#include "machine.h"
#include "memory.h"
#include "misc.h"


/*  #define RAM_DEBUG  */

struct ram_data {
	uint64_t	baseaddress;

	int		mode;
	uint64_t	otheraddress;

	/*  If mode = DEV_RAM_MIRROR:  */
	uint64_t	offset;

	/*  If mode = DEV_RAM_RAM:  */
	unsigned char	*data;
	uint64_t	length;
};


DEVICE_ACCESS(ram)
{
	struct ram_data *d = (struct ram_data *) extra;

#ifdef RAM_DEBUG
	if (writeflag==MEM_READ) {
		debug("[ ram: read from 0x%x, len=%i ]\n",
		    (int)relative_addr, (int)len);
	} else {
		int i;
		debug("[ ram: write to 0x%x:", (int)relative_addr);
		for (i=0; i<len; i++)
			debug(" %02x", data[i]);
		debug(" (len=%i) ]\n", len);
	}
#endif

	switch (d->mode) {

	case DEV_RAM_MIRROR:
		/*  TODO:  how about caches?  */
		return cpu->memory_rw(cpu, mem,
		    d->otheraddress + relative_addr, data, len,
		    writeflag, PHYSICAL);

	case DEV_RAM_RAM:
		if (writeflag == MEM_WRITE) {
			memcpy(&d->data[relative_addr], data, len);

			/*  Invalidate any code translations on a write:  */
			if (cpu->invalidate_code_translation != NULL) {
				cpu->invalidate_code_translation(
				    cpu, d->baseaddress + relative_addr,
				    INVALIDATE_PADDR);
			}
		} else {
			memcpy(data, &d->data[relative_addr], len);
		}
		break;

	default:
		fatal("dev_ram_access(): unknown mode %i\n", d->mode);
		exit(1);
	}

	return 1;
}


/*
 *  dev_ram_init():
 *
 *  Initializes a RAM or mirror device. Things get a bit complicated because
 *  of dyntrans (i.e. mirrored memory ranges should be entered into the
 *  translation arrays just as normal memory and other devices are).
 */
void dev_ram_init(struct machine *machine, uint64_t baseaddr, uint64_t length,
	int mode, uint64_t otheraddress)
{
	struct ram_data *d;
	int flags = DM_DEFAULT, points_to_ram = 1;

	CHECK_ALLOCATION(d = (struct ram_data *) malloc(sizeof(struct ram_data)));
	memset(d, 0, sizeof(struct ram_data));

	if (mode & DEV_RAM_MIGHT_POINT_TO_DEVICES) {
		mode &= ~DEV_RAM_MIGHT_POINT_TO_DEVICES;
		points_to_ram = 0;
	}

	d->mode         = mode;
	d->baseaddress  = baseaddr;
	d->otheraddress = otheraddress;

	switch (d->mode) {

	case DEV_RAM_MIRROR:
		/*
		 *  Calculate the amount that the mirror memory is offset from
		 *  the real (physical) memory. This is used in src/memory_rw.c
		 *  with dyntrans accesses if DM_EMULATED_RAM is set.
		 */
		d->offset = baseaddr - otheraddress;

		/*  Aligned RAM? Then it works with dyntrans.  */
		if (points_to_ram &&
		    (baseaddr & (machine->arch_pagesize-1)) == 0 &&
		    (otheraddress & (machine->arch_pagesize - 1)) == 0 &&
		    (length & (machine->arch_pagesize - 1)) == 0)
			flags |= DM_DYNTRANS_OK | DM_DYNTRANS_WRITE_OK
			    | DM_EMULATED_RAM;

		memory_device_register(machine->memory, "ram [mirror]",
		    baseaddr, length, dev_ram_access, d, flags
		    | DM_READS_HAVE_NO_SIDE_EFFECTS, (unsigned char*) (void *) &d->offset);
		break;

	case DEV_RAM_RAM:
		/*
		 *  Allocate zero-filled RAM using mmap(). If mmap() failed,
		 *  try malloc(), but then memset() must also be called, which
		 *  can be slow for large chunks of memory.
		 */
		d->length = length;
		d->data = (unsigned char *) mmap(NULL, length,
		    PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
		if (d->data == NULL) {
			CHECK_ALLOCATION(d->data = (unsigned char *) malloc(length));
			memset(d->data, 0, length);
		}

		/*  Aligned memory? Then it works with dyntrans.  */
		if ((baseaddr & (machine->arch_pagesize - 1)) == 0 &&
		    (length & (machine->arch_pagesize - 1)) == 0)
			flags |= DM_DYNTRANS_OK | DM_DYNTRANS_WRITE_OK;

		memory_device_register(machine->memory, "ram", baseaddr,
		    d->length, dev_ram_access, d, flags
		    | DM_READS_HAVE_NO_SIDE_EFFECTS, d->data);
		break;

	default:
		fatal("dev_ram_access(): unknown mode %i\n", d->mode);
		exit(1);
	}
}

