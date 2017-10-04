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
 *  COMMENT: SREC file support
 */

/*  Note: Included from file.c.  */


/*
 *  file_load_srec():
 *
 *  Loads a Motorola SREC file into emulated memory. Description of the SREC
 *  file format can be found at here:
 *
 *      http://www.ndsu.nodak.edu/instruct/tareski/373f98/notes/srecord.htm
 *  or  http://www.amelek.gda.pl/avr/uisp/srecord.htm
 */
static void file_load_srec(struct machine *m, struct memory *mem,
	char *filename, uint64_t *entrypointp)
{
	FILE *f;
	unsigned char buf[516];
	unsigned char bytes[270];
	uint64_t entry = 0, vaddr = 0;
	int i, j, count;
	char ch;
	int buf_len, data_start = 0;
	int entry_set = 0;
	int warning = 0;
	int warning_len = 0;
	int total_bytes_loaded = 0;

	f = fopen(filename, "r");
	if (f == NULL) {
		perror(filename);
		exit(1);
	}

	/*  Load file contents:  */
	while (!feof(f)) {
		memset(buf, 0, sizeof(buf));
		if (fgets((char *)buf, sizeof(buf)-1, f) == NULL ||
		    buf[0] == 0 || buf[0]=='\r' || buf[0]=='\n')
			continue;

		if (buf[0] != 'S') {
			if (!warning)
				debug("WARNING! non-S-record found\n");
			warning = 1;
			continue;
		}

		buf_len = strlen((char *)buf);

		if (buf_len < 10) {
			if (!warning_len)
				debug("WARNING! invalid S-record found\n");
			warning_len = 1;
			continue;
		}

		/*
		 *  Stype count address data checksum
		 *  01    23    4..     ..   (last 2 bytes)
		 *
		 *  TODO: actually check the checksum
		 */

		j = 0;
		for (i=1; i<buf_len; i++) {
			if (buf[i]>='a' && buf[i]<='f')
				buf[i] += 10 - 'a';
			else if (buf[i] >= 'A' && buf[i] <= 'F')
				buf[i] += 10 - 'A';
			else if (buf[i] >= '0' && buf[i] <= '9')
				buf[i] -= '0';
			else if (buf[i] == '\r' || buf[i] == '\n') {
			} else
				fatal("invalid characters '%c' in S-record\n",
				    buf[i]);

			if (i >= 4) {
				if (i & 1)
					bytes[j++] += buf[i];
				else
					bytes[j] = buf[i] * 16;
			}
		}

		count = buf[2] * 16 + buf[3];
		/*  debug("count=%i j=%i\n", count, j);  */
		/*  count is j - 1.  */

		switch (buf[1]) {
		case 0:
			debug("SREC \"");
			for (i=2; i<count-1; i++) {
				ch = bytes[i];
				if (ch >= ' ' && ch < 127)
					debug("%c", ch);
				else
					debug("?");
			}
			debug("\"\n");
			break;
		case 1:
		case 2:
		case 3:
			/*  switch again, to get the load address:  */
			switch (buf[1]) {
			case 1:	data_start = 2;
				vaddr = (bytes[0] << 8) + bytes[1];
				break;
			case 2:	data_start = 3;
				vaddr = (bytes[0] << 16) + (bytes[1] << 8) +
				    bytes[2];
				break;
			case 3:	data_start = 4;
				vaddr = ((uint64_t)bytes[0] << 24) +
				    (bytes[1] << 16) + (bytes[2]<<8) + bytes[3];
			}
			m->cpus[0]->memory_rw(m->cpus[0], mem, vaddr,
			    &bytes[data_start], count - 1 - data_start,
			    MEM_WRITE, NO_EXCEPTIONS);
			total_bytes_loaded += count - 1 - data_start;
			break;
		case 7:
		case 8:
		case 9:
			/*  switch again, to get the entry point:  */
			switch (buf[1]) {
			case 7:	entry = ((uint64_t)bytes[0] << 24) +
				    (bytes[1] << 16) + (bytes[2]<<8) + bytes[3];
				break;
			case 8:	entry = (bytes[0] << 16) + (bytes[1] << 8) +
				    bytes[2];
				break;
			case 9:	entry = (bytes[0] << 8) + bytes[1];
				break;
			}
			entry_set = 1;
			debug("entry point 0x%08x\n", (unsigned int)entry);
			break;
		default:
			debug("unimplemented S-record type %i\n", buf[1]);
		}
	}

	debug("0x%x bytes loaded\n", total_bytes_loaded);

	fclose(f);

	if (!entry_set)
		debug("WARNING! no entrypoint found!\n");
	else
		*entrypointp = entry;

	n_executables_loaded ++;
}

