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
 *  COMMENT: Mach-O file support
 */

/*  Note: Included from file.c.  */


/*
 *  file_load_macho():
 *
 *  Loads a Mach-O binary image into the emulated memory. The entry point
 *  is stored in the specified CPU's registers.
 *
 *  TODO:
 *
 *	o)  Almost everything.
 *
 *	o)  I haven't had time to look into whether Apple's open source
 *	    license is BSD-compatible or not. Perhaps it would be possible
 *	    to use a header file containing symbolic names, and not use
 *	    hardcoded values.
 */
static void file_load_macho(struct machine *m, struct memory *mem,
	char *filename, uint64_t *entrypointp, int arch, int *byte_orderp,
	int is_64bit, int is_reversed)
{
	FILE *f;
	uint64_t entry = 0;
	int entry_set = 0;
	int encoding = ELFDATA2MSB;
	unsigned char buf[65536];
	char *symbols, *strings;
	uint32_t cputype, cpusubtype, filetype, ncmds, sizeofcmds, flags;
	uint64_t vmaddr, vmsize, fileoff, filesize;
	int cmd_type, cmd_len, i, flavor;
	int32_t symoff, nsyms, stroff, strsize;
	size_t len, pos;

	if (m->cpus[0]->byte_order == EMUL_BIG_ENDIAN)
		encoding = ELFDATA2MSB;

	f = fopen(filename, "r");
	if (f == NULL) {
		perror(filename);
		exit(1);
	}

	if (is_64bit) {
		fatal("TODO: 64-bit Mach-O. Not supported yet.\n");
		exit(1);
	}
	if (is_reversed) {
		fatal("TODO: Reversed-endianness. Not supported yet.\n");
		exit(1);
	}

	len = fread(buf, 1, sizeof(buf), f);
	if (len < 100) {
		fatal("Bad Mach-O file?\n");
		exit(1);
	}

	unencode(cputype,    &buf[4], uint32_t);
	unencode(cpusubtype, &buf[8], uint32_t);
	unencode(filetype,   &buf[12], uint32_t);
	unencode(ncmds,      &buf[16], uint32_t);
	unencode(sizeofcmds, &buf[20], uint32_t);
	unencode(flags,      &buf[24], uint32_t);

	/*  debug("cputype=0x%x cpusubtype=0x%x filetype=0x%x\n",
	    cputype, cpusubtype, filetype);
	    debug("ncmds=%i sizeofcmds=0x%08x flags=0x%08x\n",
	    ncmds, sizeofcmds, flags);  */

	/*
	 *  Compare to "normal" values.
	 *  NOTE/TODO: These were for a Darwin (Macintosh PPC) kernel.
	 */
	if (cputype != 0x12) {
		fatal("Error: Unimplemented cputype 0x%x\n", cputype);
		exit(1);
	}
	if (cpusubtype != 0) {
		fatal("Error: Unimplemented cpusubtype 0x%x\n", cpusubtype);
		exit(1);
	}
	/*  Filetype 2 means an executable image.  */
	if (filetype != 2) {
		fatal("Error: Unimplemented filetype 0x%x\n", filetype);
		exit(1);
	}
	if (!(flags & 1)) {
		fatal("Error: File has 'undefined references'. Cannot"
		    " be executed.\n", flags);
		exit(1);
	}

	/*  I've only encountered flags == 1 so far.  */
	if (flags != 1) {
		fatal("Error: Unimplemented flags 0x%x\n", flags);
		exit(1);
	}

	/*
	 *  Read all load commands:
	 */
	pos = is_64bit? 32 : 28;
	cmd_type = 0;
	do {
		/*  Read command type and length:  */
		unencode(cmd_type, &buf[pos], uint32_t);
		unencode(cmd_len,  &buf[pos+4], uint32_t);

#if 0
		debug("cmd %i, len=%i\n", cmd_type, cmd_len);
		for (i=8; i<cmd_len; i++) {
			unsigned char ch = buf[pos+i];
			if (ch >= ' ' && ch < 127)
				debug("%c", ch);
			else
				debug(".");
		}
#endif
		switch (cmd_type) {
		case 1:	/*  LC_SEGMENT  */
			debug("seg ");
			for (i=0; i<16; i++) {
				if (buf[pos + 8 + i] == 0)
					break;
				debug("%c", buf[pos + 8 + i]);
			}
			unencode(vmaddr,   &buf[pos+8+16+0], uint32_t);
			unencode(vmsize,   &buf[pos+8+16+4], uint32_t);
			unencode(fileoff,  &buf[pos+8+16+8], uint32_t);
			unencode(filesize, &buf[pos+8+16+12], uint32_t);
			debug(": vmaddr=0x%x size=0x%x fileoff=0x%x",
			    (int)vmaddr, (int)vmsize, (int)fileoff);

			if (filesize == 0) {
				debug("\n");
				break;
			}

			fseek(f, fileoff, SEEK_SET);

			/*  Load data from the file:  */
			while (filesize != 0) {
				unsigned char buf[32768];
				ssize_t len = filesize > sizeof(buf) ?
				    sizeof(buf) : filesize;
				len = fread(buf, 1, len, f);

				/*  printf("fread len=%i vmaddr=%x buf[0..]="
				    "%02x %02x %02x\n", (int)len, (int)vmaddr,
				    buf[0], buf[1], buf[2]);  */

				if (len > 0) {
					int len2 = 0;
					uint64_t vaddr1 = vmaddr &
					    ((1 << BITS_PER_MEMBLOCK) - 1);
					uint64_t vaddr2 = (vmaddr +
					    len) & ((1 << BITS_PER_MEMBLOCK)-1);
					if (vaddr2 < vaddr1) {
						len2 = len - vaddr2;
						m->cpus[0]->memory_rw(m->cpus[
						    0], mem, vmaddr, &buf[0],
						    len2, MEM_WRITE,
						    NO_EXCEPTIONS);
					}
					m->cpus[0]->memory_rw(m->cpus[0], mem,
					    vmaddr + len2, &buf[len2], len-len2,
					    MEM_WRITE, NO_EXCEPTIONS);
				} else {
					fprintf(stderr, "error reading\n");
					exit(1);
				}

				vmaddr += len;
				filesize -= len;
			}

			debug("\n");
			break;

		case 2:	/*  LC_SYMTAB  */
			unencode(symoff,  &buf[pos+8], uint32_t);
			unencode(nsyms,   &buf[pos+12], uint32_t);
			unencode(stroff,  &buf[pos+16], uint32_t);
			unencode(strsize, &buf[pos+20], uint32_t);
			debug("symtable: %i symbols @ 0x%x (strings at "
			    "0x%x)\n", nsyms, symoff, stroff);

			CHECK_ALLOCATION(symbols = (char *) malloc(12 * nsyms));
			fseek(f, symoff, SEEK_SET);
			if (fread(symbols, 1, 12 * nsyms, f) != (size_t) 12*nsyms) {
				fprintf(stderr, "could not read symbols from %s\n", filename);
				exit(1);
			}

			CHECK_ALLOCATION(strings = (char *) malloc(strsize));
			fseek(f, stroff, SEEK_SET);
			if (fread(strings, 1, strsize, f) != (size_t) strsize) {
				fprintf(stderr, "could not read symbol strings from %s\n", filename);
				exit(1);
			}

			for (i=0; i<nsyms; i++) {
				int n_strx, n_type, n_sect, n_desc;
				uint32_t n_value;
				unencode(n_strx,  &symbols[i*12+0], int32_t);
				unencode(n_type,  &symbols[i*12+4], uint8_t);
				unencode(n_sect,  &symbols[i*12+5], uint8_t);
				unencode(n_desc,  &symbols[i*12+6], int16_t);
				unencode(n_value, &symbols[i*12+8], uint32_t);
				/*  debug("%i: strx=%i type=%i sect=%i desc=%i"
				    " value=0x%x\n", i, n_strx, n_type,
				    n_sect, n_desc, n_value);  */
				add_symbol_name(&m->symbol_context,
				    n_value, 0, strings + n_strx, 0, -1);
			}

			free(symbols);
			free(strings);
			break;

		case 5:	debug("unix thread context: ");
			/*  See http://cvs.sf.net/viewcvs.py/hte/
			    HT%20Editor/machostruc.h or similar for details
			    on the thread struct.  */
			unencode(flavor, &buf[pos+8], uint32_t);
			if (flavor != 1) {
				fatal("unimplemented flavor %i\n", flavor);
				exit(1);
			}

			if (arch != ARCH_PPC) {
				fatal("non-PPC arch? TODO\n");
				exit(1);
			}

			unencode(entry, &buf[pos+16], uint32_t);
			entry_set = 1;
			debug("pc=0x%x\n", (int)entry);

			for (i=1; i<40; i++) {
				uint32_t x;
				unencode(x, &buf[pos+16+i*4], uint32_t);
				if (x != 0) {
					fatal("Entry nr %i in the Mach-O"
					    " thread struct is non-zero"
					    " (0x%x). This is not supported"
					    " yet. TODO\n", i, x);
					exit(1);
				}
			}
			break;

		default:fatal("WARNING! Unimplemented load command %i!\n",
			    cmd_type);
		}

		pos += cmd_len;
	} while (pos < sizeofcmds && cmd_type != 0);

	fclose(f);

	if (!entry_set) {
		fatal("No entry point? Aborting.\n");
		exit(1);
	}

	*entrypointp = entry;

	if (encoding == ELFDATA2LSB)
		*byte_orderp = EMUL_LITTLE_ENDIAN;
	else
		*byte_orderp = EMUL_BIG_ENDIAN;

	n_executables_loaded ++;
}

