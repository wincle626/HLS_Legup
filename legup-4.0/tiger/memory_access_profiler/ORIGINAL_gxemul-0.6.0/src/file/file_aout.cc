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
 *  COMMENT: a.out executable file support
 */

/*  Note: Included from file.c.  */


#include "thirdparty/exec_aout.h"


#define	AOUT_FLAG_DECOSF1		1
#define	AOUT_FLAG_FROM_BEGINNING	2
#define	AOUT_FLAG_VADDR_ZERO_HACK	4
#define	AOUT_FLAG_NO_SIZES		8

struct aout_symbol {
	uint32_t	strindex;
	uint32_t	type;
	uint32_t	addr;
};


/*
 *  file_load_aout():
 *
 *  Loads an a.out binary image into the emulated memory.  The entry point
 *  (read from the a.out header) is stored in the specified CPU's registers.
 *
 *  TODO:  This has to be rewritten / corrected to support multiple a.out
 *         formats, where text/data are aligned differently.
 */
static void file_load_aout(struct machine *m, struct memory *mem,
	char *filename, int flags,
	uint64_t *entrypointp, int arch, int *byte_orderp)
{
	struct exec aout_header;
	FILE *f;
	int len;
	int encoding = ELFDATA2LSB;
	uint32_t entry, datasize, textsize;
	int32_t symbsize = 0;
	uint32_t vaddr, total_len;
	unsigned char buf[65536];
	unsigned char *syms;

	if (m->cpus[0]->byte_order == EMUL_BIG_ENDIAN)
		encoding = ELFDATA2MSB;

	f = fopen(filename, "r");
	if (f == NULL) {
		perror(filename);
		exit(1);
	}

	if (flags & AOUT_FLAG_DECOSF1) {
		if (fread(&buf, 1, 32, f) != 32) {
			perror(filename);
			exit(1);
		}
		vaddr = buf[16] + (buf[17] << 8) +
		    (buf[18] << 16) + ((uint64_t)buf[19] << 24);
		entry = buf[20] + (buf[21] << 8) +
		    (buf[22] << 16) + ((uint64_t)buf[23] << 24);
		debug("OSF1 a.out, load address 0x%08lx, "
		    "entry point 0x%08x\n", (long)vaddr, (long)entry);
		symbsize = 0;
		fseek(f, 0, SEEK_END);
		/*  This is of course wrong, but should work anyway:  */
		textsize = ftello(f) - 512;
		datasize = 0;
		fseek(f, 512, SEEK_SET);
	} else if (flags & AOUT_FLAG_NO_SIZES) {
		fseek(f, 0, SEEK_END);
		textsize = ftello(f) - 32;
		datasize = 0;
		vaddr = entry = 0;
		fseek(f, 32, SEEK_SET);
	} else {
		len = fread(&aout_header, 1, sizeof(aout_header), f);
		if (len != sizeof(aout_header)) {
			fprintf(stderr, "%s: not a complete a.out image\n",
			    filename);
			exit(1);
		}

		unencode(entry, &aout_header.a_entry, uint32_t);
		debug("a.out, entry point 0x%08lx\n", (long)entry);
		vaddr = entry;

		if (flags & AOUT_FLAG_VADDR_ZERO_HACK)
			vaddr = 0;

		unencode(textsize, &aout_header.a_text, uint32_t);
		unencode(datasize, &aout_header.a_data, uint32_t);
		debug("text + data = %i + %i bytes\n", textsize, datasize);

		unencode(symbsize, &aout_header.a_syms, uint32_t);
	}

	if (flags & AOUT_FLAG_FROM_BEGINNING) {
		fseek(f, 0, SEEK_SET);
		vaddr &= ~0xfff;
	}

	/*  Load text and data:  */
	total_len = textsize + datasize;
	while (total_len != 0) {
		len = total_len > sizeof(buf) ? sizeof(buf) : total_len;
		len = fread(buf, 1, len, f);

		/*  printf("fread len=%i vaddr=%x buf[0..]=%02x %02x %02x\n",
		    (int)len, (int)vaddr, buf[0], buf[1], buf[2]);  */

		if (len > 0) {
			int len2 = 0;
			uint64_t vaddr1 = vaddr &
			    ((1 << BITS_PER_MEMBLOCK) - 1);
			uint64_t vaddr2 = (vaddr +
			    len) & ((1 << BITS_PER_MEMBLOCK) - 1);
			if (vaddr2 < vaddr1) {
				len2 = len - vaddr2;
				m->cpus[0]->memory_rw(m->cpus[0], mem, vaddr,
				    &buf[0], len2, MEM_WRITE, NO_EXCEPTIONS);
			}
			m->cpus[0]->memory_rw(m->cpus[0], mem, vaddr + len2,
			    &buf[len2], len-len2, MEM_WRITE, NO_EXCEPTIONS);
		} else {
			if (flags & AOUT_FLAG_DECOSF1)
				break;
			else {
				fprintf(stderr, "could not read from %s,"
				    " wanted to read %i bytes\n", filename,
				    (int) total_len);
				exit(1);
			}
		}

		vaddr += len;
		total_len -= len;
	}

	if (symbsize != 0) {
		struct aout_symbol *aout_symbol_ptr;
		int i, n_symbols;
		uint32_t type, addr, str_index;
		uint32_t strings_len;
		char *string_symbols;
		off_t oldpos;

		debug("symbols: %i bytes @ 0x%x\n", symbsize, (int)ftello(f));
		CHECK_ALLOCATION(syms = (unsigned char *) malloc(symbsize));
		len = fread(syms, 1, symbsize, f);
		if (len != symbsize) {
			fprintf(stderr, "error reading symbols from %s\n",
			    filename);
			exit(1);
		}

		oldpos = ftello(f);
		fseek(f, 0, SEEK_END);
		strings_len = ftello(f) - oldpos;
		fseek(f, oldpos, SEEK_SET);
		debug("strings: %i bytes @ 0x%x\n", strings_len,(int)ftello(f));
		CHECK_ALLOCATION(string_symbols = (char *) malloc(strings_len));
		if (fread(string_symbols, 1, strings_len, f) != strings_len) {
			fprintf(stderr, "Could not read symbols from %s?\n", filename);
			perror("fread");
			exit(1);
		}

		aout_symbol_ptr = (struct aout_symbol *) syms;
		n_symbols = symbsize / sizeof(struct aout_symbol);
		i = 0;
		while (i < n_symbols) {
			unencode(str_index, &aout_symbol_ptr[i].strindex,
			    uint32_t);
			unencode(type, &aout_symbol_ptr[i].type, uint32_t);
			unencode(addr, &aout_symbol_ptr[i].addr, uint32_t);

			/*  debug("symbol type 0x%04x @ 0x%08x: %s\n",
			    type, addr, string_symbols + str_index);  */

			if (type != 0 && addr != 0)
				add_symbol_name(&m->symbol_context,
				    addr, 0, string_symbols + str_index, 0, -1);
			i++;
		}

		free(string_symbols);
		free(syms);
	}

	fclose(f);

	*entrypointp = (int32_t)entry;

	if (encoding == ELFDATA2LSB)
		*byte_orderp = EMUL_LITTLE_ENDIAN;
	else
		*byte_orderp = EMUL_BIG_ENDIAN;

	n_executables_loaded ++;
}

