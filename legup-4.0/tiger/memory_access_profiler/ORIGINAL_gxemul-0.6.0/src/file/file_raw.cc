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
 *  COMMENT: Raw file support
 *
 *  (Simply loads raw data into emulated memory space).
 */

/*  Note: Included from file.c.  */


/*
 *  file_load_raw():
 *
 *  Loads a raw binary into emulated memory. The filename should be
 *  of the following form:     loadaddress:filename
 *  or    loadaddress:skiplen:filename
 *  or    loadaddress:skiplen:pc:filename
 */
static void file_load_raw(struct machine *m, struct memory *mem,
	char *filename, uint64_t *entrypointp)
{
	FILE *f;
	int len, sign3264;
	unsigned char buf[16384];
	uint64_t entry, loadaddr, vaddr, skip = 0;
	char *p, *p2;

	/*  Special case for 32-bit MIPS:  */
	sign3264 = 0;
	if (m->arch == ARCH_MIPS && m->cpus[0]->is_32bit)
		sign3264 = 1;

	p = strchr(filename, ':');
	if (p == NULL) {
		fprintf(stderr, "\n");
		perror(filename);
		exit(1);
	}

	loadaddr = vaddr = entry = strtoull(filename, NULL, 0);
	p2 = p+1;

	/*  A second value? That's the optional skip value  */
	p = strchr(p2, ':');
	if (p != NULL) {
		skip = strtoull(p2, NULL, 0);
		p = p+1;
		/*  A third value? That's the initial pc:  */
		if (strchr(p, ':') != NULL) {
			entry = strtoull(p, NULL, 0);
			p = strchr(p, ':') + 1;
		}
	} else
		p = p2;

	if (sign3264) {
		loadaddr = (int64_t)(int32_t)loadaddr;
		entry = (int64_t)(int32_t)entry;
		vaddr = (int64_t)(int32_t)vaddr;
		skip = (int64_t)(int32_t)skip;
	}

	f = fopen(strrchr(filename, ':')+1, "r");
	if (f == NULL) {
		perror(p);
		exit(1);
	}

	fseek(f, skip, SEEK_SET);

	/*  Load file contents:  */
	while (!feof(f)) {
		size_t to_read = sizeof(buf);

		/*  If vaddr isn't buf-size aligned, then start with a
		    smaller buffer:  */
		if (vaddr & (sizeof(buf) - 1))
			to_read = sizeof(buf) - (vaddr & (sizeof(buf)-1));

		len = fread(buf, 1, to_read, f);

		if (len > 0)
			m->cpus[0]->memory_rw(m->cpus[0], mem, vaddr, &buf[0],
			    len, MEM_WRITE, NO_EXCEPTIONS);

		vaddr += len;
	}

	debug("RAW: 0x%"PRIx64" bytes @ 0x%08"PRIx64,
	    (uint64_t) (ftello(f) - skip), (uint64_t) loadaddr);

	if (skip != 0)
		debug(" (0x%"PRIx64" bytes of header skipped)",
		    (uint64_t) skip);

	debug("\n");

	fclose(f);

	*entrypointp = entry;

	n_executables_loaded ++;
}

