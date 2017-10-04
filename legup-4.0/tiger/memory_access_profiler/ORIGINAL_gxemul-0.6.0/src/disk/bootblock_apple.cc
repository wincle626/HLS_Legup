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
 *  Apple bootblock handling.
 *
 *  TODO: This is just a quick hack skeleton. Doesn't really work yet.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "diskimage.h"
#include "misc.h"


/*
 *  apple_load_bootblock():
 *
 *  Try to load a kernel from a disk image with an Apple Partition Table.
 *
 *  TODO: This function uses too many magic offsets and so on; it should be
 *  cleaned up some day. See http://www.awprofessional.com/articles/
 *	article.asp?p=376123&seqNum=3&rl=1  for some info on the Apple
 *  partition format.
 *
 *  Returns 1 on success, 0 on failure.
 */
int apple_load_bootblock(struct machine *m, struct cpu *cpu,
	int disk_id, int disk_type, int *n_loadp, char ***load_namesp)
{
	unsigned char buf[0x8000];
	int res, partnr, n_partitions = 0, n_hfs_partitions = 0;
	uint64_t hfs_start, hfs_length;

	res = diskimage_access(m, disk_id, disk_type, 0, 0x0, buf, sizeof(buf));
	if (!res) {
		fatal("apple_load_bootblock: couldn't read the disk "
		    "image. Aborting.\n");
		return 0;
	}

	partnr = 0;
	do {
		int start, length;
		int ofs = 0x200 * (partnr + 1);
		if (partnr == 0)
			n_partitions = buf[ofs + 7];
		start = ((uint64_t)buf[ofs + 8] << 24) + (buf[ofs + 9] << 16) +
		    (buf[ofs + 10] << 8) + buf[ofs + 11];
		length = ((uint64_t)buf[ofs+12] << 24) + (buf[ofs + 13] << 16) +
		    (buf[ofs + 14] << 8) + buf[ofs + 15];

		debug("partition %i: '%s', type '%s', start %i, length %i\n",
		    partnr, buf + ofs + 0x10, buf + ofs + 0x30,
		    start, length);

		if (strcmp((char *)buf + ofs + 0x30, "Apple_HFS") == 0) {
			n_hfs_partitions ++;
			hfs_start = 512 * start;
			hfs_length = 512 * length;
		}

		/*  Any more partitions?  */
		partnr ++;
	} while (partnr < n_partitions);

	if (n_hfs_partitions == 0) {
		fatal("Error: No HFS partition found! TODO\n");
		return 0;
	}
	if (n_hfs_partitions >= 2) {
		fatal("Error: Too many HFS partitions found! TODO\n");
		return 0;
	}

	return 0;
}


