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
 *  Bootblock handling:
 *
 *	o)  For some machines (e.g. DECstation or the Dreamcast), it is
 *	    possible to load a bootblock from a fixed location on disk, and
 *	    simply execute it in memory.
 *
 *	o)  For booting from generic CDROM ISO9660 images, a filename of
 *	    a file to load must be supplied (the kernel filename). It is
 *	    loaded, possibly gunzipped, and then executed as if it was a
 *	    separate file.
 *
 *  TODO: This module needs some cleanup.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "diskimage.h"
#include "emul.h"
#include "machine.h"
#include "memory.h"

static const char *diskimage_types[] = DISKIMAGE_TYPES;


/*
 *  load_bootblock():
 *
 *  For some emulation modes, it is possible to boot from a harddisk image by
 *  loading a bootblock from a specific disk offset into memory, and executing
 *  that, instead of requiring a separate kernel file.  It is then up to the
 *  bootblock to load a kernel.
 *
 *  Returns 1 on success, 0 on failure.
 */
int load_bootblock(struct machine *m, struct cpu *cpu,
	int *n_loadp, char ***load_namesp)
{
	int boot_disk_id, boot_disk_type = 0, n_blocks, res, readofs,
	    iso_type, retval = 0;
	unsigned char minibuf[0x20];
	unsigned char *bootblock_buf;
	uint64_t bootblock_offset, base_offset;
	uint64_t bootblock_loadaddr, bootblock_pc;

	boot_disk_id = diskimage_bootdev(m, &boot_disk_type);
	if (boot_disk_id < 0)
		return 0;

	base_offset = diskimage_get_baseoffset(m, boot_disk_id, boot_disk_type);

	switch (m->machine_type) {

	case MACHINE_DREAMCAST:
		if (!diskimage_is_a_cdrom(cpu->machine, boot_disk_id,
		    boot_disk_type)) {
			fatal("The Dreamcast emulation mode can only boot"
			    " from CD images, not from other disk types.\n");
			exit(1);
		}

		CHECK_ALLOCATION(bootblock_buf = (unsigned char *) malloc(32768));

		debug("loading Dreamcast IP.BIN from %s id %i\n",
		    diskimage_types[boot_disk_type], boot_disk_id);

		res = diskimage_access(m, boot_disk_id, boot_disk_type,
		    0, base_offset, bootblock_buf, 0x8000);
		if (!res) {
			fatal("Couldn't read the disk image. Aborting.\n");
			return 0;
		}

		if (strncmp((char *)bootblock_buf, "SEGA ", 5) != 0) {
			fatal("This is not a Dreamcast IP.BIN header.\n");
			free(bootblock_buf);
			return 0;
		}

		/*  Store IP.BIN at 0x8c008000, and set entry point.  */
		store_buf(cpu, 0x8c008000, (char *)bootblock_buf, 32768);
		cpu->pc = 0x8c008300;

		/*  Remember the name of the file to boot (1ST_READ.BIN):  */
		if (cpu->machine->boot_kernel_filename == NULL ||
		    cpu->machine->boot_kernel_filename[0] == '\0') {
			int i = 0x60;
			while (i < 0x70) {
				if (bootblock_buf[i] == ' ')
					bootblock_buf[i] = 0;
				i ++;
			}
			CHECK_ALLOCATION(cpu->machine->boot_kernel_filename =
			    strdup((char *)bootblock_buf + 0x60));
		}

		debug("boot filename: %s\n",
		    cpu->machine->boot_kernel_filename);

		free(bootblock_buf);

		break;

	case MACHINE_PMAX:
		/*
		 *  The first few bytes of a disk contains information about
		 *  where the bootblock(s) are located. (These are all 32-bit
		 *  little-endian words.)
		 *
		 *  Offset 0x10 = load address
		 *         0x14 = initial PC value
		 *         0x18 = nr of 512-byte blocks to read
		 *         0x1c = offset on disk to where the bootblocks
		 *                are (in 512-byte units)
		 *         0x20 = nr of blocks to read...
		 *         0x24 = offset...
		 *
		 *  nr of blocks to read and offset are repeated until nr of
		 *  blocks to read is zero.
		 */
		res = diskimage_access(m, boot_disk_id, boot_disk_type, 0, 0,
		    minibuf, sizeof(minibuf));

		bootblock_loadaddr = minibuf[0x10] + (minibuf[0x11] << 8)
		  + (minibuf[0x12] << 16) + ((uint64_t)minibuf[0x13] << 24);

		/*  Convert loadaddr to uncached:  */
		if ((bootblock_loadaddr & 0xf0000000ULL) != 0x80000000 &&
		    (bootblock_loadaddr & 0xf0000000ULL) != 0xa0000000) {
			fatal("\nWARNING! Weird load address 0x%08"PRIx32
			    " for SCSI id %i.\n\n",
			    (uint32_t)bootblock_loadaddr, boot_disk_id);
			if (bootblock_loadaddr == 0) {
				fatal("I'm assuming that this is _not_ a "
				    "DEC bootblock.\nAre you sure you are"
				    " booting from the correct disk?\n");
				exit(1);
			}
		}

		bootblock_loadaddr &= 0x0fffffffULL;
		bootblock_loadaddr |= 0xffffffffa0000000ULL;

		bootblock_pc = minibuf[0x14] + (minibuf[0x15] << 8)
		  + (minibuf[0x16] << 16) + ((uint64_t)minibuf[0x17] << 24);

		bootblock_pc &= 0x0fffffffULL;
		bootblock_pc |= 0xffffffffa0000000ULL;
		cpu->pc = bootblock_pc;

		debug("DEC boot: loadaddr=0x%08"PRIx32", pc=0x%08"PRIx32,
		    (uint32_t) bootblock_loadaddr, (uint32_t) bootblock_pc);

		readofs = 0x18;

		for (;;) {
			res = diskimage_access(m, boot_disk_id, boot_disk_type,
			    0, readofs, minibuf, sizeof(minibuf));
			if (!res) {
				fatal("Couldn't read the disk image. "
				    "Aborting.\n");
				return 0;
			}

			n_blocks = minibuf[0] + (minibuf[1] << 8)
			  + (minibuf[2] << 16) + ((uint64_t)minibuf[3] << 24);

			bootblock_offset = (minibuf[4] + (minibuf[5] << 8) +
			  (minibuf[6]<<16) + ((uint64_t)minibuf[7]<<24)) * 512;

			if (n_blocks < 1)
				break;

			debug(readofs == 0x18? ": %i" : " + %i", n_blocks);

			if (n_blocks * 512 > 65536)
				fatal("\nWARNING! Unusually large bootblock "
				    "(%i bytes)\n\n", n_blocks * 512);

			CHECK_ALLOCATION(bootblock_buf = (unsigned char *) malloc(n_blocks*512));

			res = diskimage_access(m, boot_disk_id, boot_disk_type,
			    0, bootblock_offset, bootblock_buf, n_blocks * 512);
			if (!res) {
				fatal("WARNING: could not load bootblocks from"
				    " disk offset 0x%llx\n",
				    (long long)bootblock_offset);
			}

			store_buf(cpu, bootblock_loadaddr,
			    (char *)bootblock_buf, n_blocks * 512);

			bootblock_loadaddr += 512*n_blocks;
			free(bootblock_buf);
			readofs += 8;
		}

		debug(readofs == 0x18? ": no blocks?\n" : " blocks\n");
		return 1;
	}


	/*
	 *  Try reading a kernel manually from the disk. The code here
	 *  does not rely on machine-dependent boot blocks etc.
	 */
	/*  ISO9660: (0x800 bytes at 0x8000 + base_offset)  */
	CHECK_ALLOCATION(bootblock_buf = (unsigned char *) malloc(0x800));
	res = diskimage_access(m, boot_disk_id, boot_disk_type,
	    0, base_offset + 0x8000, bootblock_buf, 0x800);
	if (!res) {
		fatal("Couldn't read the disk image. Aborting.\n");
		return 0;
	}

	iso_type = 0;
	if (strncmp((char *)bootblock_buf+1, "CD001", 5) == 0)
		iso_type = 1;
	if (strncmp((char *)bootblock_buf+1, "CDW01", 5) == 0)
		iso_type = 2;
	if (strncmp((char *)bootblock_buf+1, "CDROM", 5) == 0)
		iso_type = 3;

	if (iso_type != 0) {
		/*
		 *  If the user specified a kernel name, then load it from
		 *  disk.
		 */
		if (cpu->machine->boot_kernel_filename == NULL ||
		    cpu->machine->boot_kernel_filename[0] == '\0')
			fatal("\nISO9660 filesystem, but no kernel "
			    "specified? (Use the -j option.)\n");
		else
			retval = iso_load_bootblock(m, cpu, boot_disk_id,
			    boot_disk_type, iso_type, bootblock_buf,
			    n_loadp, load_namesp);
	}

	if (retval != 0)
		goto ret_ok;

	/*  Apple parition table:  */
	res = diskimage_access(m, boot_disk_id, boot_disk_type,
	    0, 0x0, bootblock_buf, 0x800);
	if (!res) {
		fatal("Couldn't read the disk image. Aborting.\n");
		return 0;
	}
	if (bootblock_buf[0x000] == 'E' && bootblock_buf[0x001] == 'R' &&
	    bootblock_buf[0x200] == 'P' && bootblock_buf[0x201] == 'M') {
		if (cpu->machine->boot_kernel_filename == NULL ||
		    cpu->machine->boot_kernel_filename[0] == '\0')
			fatal("\nApple partition table, but no kernel "
			    "specified? (Use the -j option.)\n");
		else
			retval = apple_load_bootblock(m, cpu, boot_disk_id,
			    boot_disk_type, n_loadp, load_namesp);
	}

ret_ok:
	free(bootblock_buf);
	return retval;
}


