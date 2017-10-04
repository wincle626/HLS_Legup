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
 *  COMMENT: ECOFF file support
 */

/*  Note: Included from file.c.  */


#include "thirdparty/exec_ecoff.h"


/*  Special symbol format used by Microsoft-ish COFF files:  */
struct ms_sym {
	unsigned char	name[8];
	unsigned char	value[4];
	unsigned char	section[2];
	unsigned char	type[2];   
	unsigned char	storage_class;
	unsigned char	n_aux_syms;   
};


/*
 *  file_load_ecoff():
 *
 *  Loads an ecoff binary image into the emulated memory.  The entry point
 *  (read from the ecoff header) is stored in the specified CPU's registers.
 */
static void file_load_ecoff(struct machine *m, struct memory *mem,
	char *filename, uint64_t *entrypointp,
	int arch, uint64_t *gpp, int *byte_orderp)
{
	struct ecoff_exechdr exechdr;
	int f_magic, f_nscns, f_nsyms;
	int a_magic;
	off_t f_symptr, a_tsize, a_dsize, a_bsize;
	uint64_t a_entry, a_tstart, a_dstart, a_bstart, a_gp, end_addr=0;
	const char *format_name;
	struct ecoff_scnhdr scnhdr;
	FILE *f;
	int len, secn, total_len, chunk_size;
	int encoding = ELFDATA2LSB;	/*  Assume little-endian. See below  */
	int program_byte_order = -1;
	unsigned char buf[8192];

	f = fopen(filename, "r");
	if (f == NULL) {
		perror(filename);
		exit(1);
	}

	len = fread(&exechdr, 1, sizeof(exechdr), f);
	if (len != sizeof(exechdr)) {
		fprintf(stderr, " not a complete ecoff image\n");
		exit(1);
	}

	/*
	 *  The following code looks a bit ugly, but it should work. The ECOFF
	 *  16-bit magic value seems to be stored in MSB byte order for
	 *  big-endian binaries, and LSB byte order for little-endian binaries.
	 *
	 *  The unencode() assumes little-endianness by default.
	 */
	unencode(f_magic, &exechdr.f.f_magic, uint16_t);
	switch (f_magic) {
	case ((ECOFF_MAGIC_MIPSEB & 0xff) << 8) +
	    ((ECOFF_MAGIC_MIPSEB >> 8) & 0xff):
		format_name = "MIPS1 BE";
		encoding = ELFDATA2MSB;
		break;
	case ECOFF_MAGIC_MIPSEB:
		/*  NOTE: Big-endian header, little-endian code!  */
		format_name = "MIPS1 BE-LE";
		encoding = ELFDATA2MSB;
		program_byte_order = ELFDATA2LSB;
		break;
	case ECOFF_MAGIC_MIPSEL:
		format_name = "MIPS1 LE";
		encoding = ELFDATA2LSB;
		break;
	case ((ECOFF_MAGIC_MIPSEB2 & 0xff) << 8) +
	    ((ECOFF_MAGIC_MIPSEB2 >> 8) & 0xff):
		format_name = "MIPS2 BE";
		encoding = ELFDATA2MSB;
		break;
	case ECOFF_MAGIC_MIPSEL2:
		format_name = "MIPS2 LE";
		encoding = ELFDATA2LSB;
		break;
	case ((ECOFF_MAGIC_MIPSEB3 & 0xff) << 8) +
	    ((ECOFF_MAGIC_MIPSEB3 >> 8) & 0xff):
		format_name = "MIPS3 BE";
		encoding = ELFDATA2MSB;
		break;
	case ECOFF_MAGIC_MIPSEL3:
		format_name = "MIPS3 LE";
		encoding = ELFDATA2LSB;
		break;
	default:
		fprintf(stderr, "%s: unimplemented ECOFF format, magic = "
		    "0x%04x\n", filename, (int)f_magic);
		exit(1);
	}

	/*  Read various header information:  */
	unencode(f_nscns,  &exechdr.f.f_nscns,  uint16_t);
	unencode(f_symptr, &exechdr.f.f_symptr, uint32_t);
	unencode(f_nsyms,  &exechdr.f.f_nsyms,  uint32_t);
	debug("ECOFF, %s, %i sections, %i symbols @ 0x%lx\n",
	    format_name, f_nscns, f_nsyms, (long)f_symptr);

	unencode(a_magic, &exechdr.a.magic, uint16_t);
	unencode(a_tsize, &exechdr.a.tsize, uint32_t);
	unencode(a_dsize, &exechdr.a.dsize, uint32_t);
	unencode(a_bsize, &exechdr.a.bsize, uint32_t);
	debug("magic 0x%04x, tsize 0x%x, dsize 0x%x, bsize 0x%x\n",
	    a_magic, (int)a_tsize, (int)a_dsize, (int)a_bsize);

	unencode(a_tstart, &exechdr.a.text_start, uint32_t);
	unencode(a_dstart, &exechdr.a.data_start, uint32_t);
	unencode(a_bstart, &exechdr.a.bss_start,  uint32_t);
	debug("text @ 0x%08x, data @ 0x%08x, bss @ 0x%08x\n",
	    (int)a_tstart, (int)a_dstart, (int)a_bstart);

	unencode(a_entry, &exechdr.a.entry,    uint32_t);
	unencode(a_gp,    &exechdr.a.gp_value, uint32_t);
	debug("entrypoint 0x%08x, gp = 0x%08x\n",
	    (int)a_entry, (int)a_gp);

	/*
	 *  Special hack for a MACH/pmax kernel, I don't know how applicable
	 *  this is for other files:
	 *  there are no sections (!), and a_magic = 0x0108 instead of
	 *  0x0107 as it is on most other (E)COFF files I've seen.
	 *
	 *  Then load everything after the header to the text start address.
	 */
	if (f_nscns == 0 && a_magic == 0x108) {
		uint64_t where = a_tstart;
		total_len = 0;
		fseek(f, 0x50, SEEK_SET);
		while (!feof(f)) {
			chunk_size = 256;
			len = fread(buf, 1, chunk_size, f);

			if (len > 0)
				m->cpus[0]->memory_rw(m->cpus[0], mem, where,
				    &buf[0], len, MEM_WRITE, NO_EXCEPTIONS);
			where += len;
			total_len += len;
		}
		debug("MACH/pmax hack (!), read 0x%x bytes\n", total_len);
	}

	/*  Go through all the section headers:  */
	for (secn=0; secn<f_nscns; secn++) {
		off_t s_scnptr, s_relptr, s_lnnoptr, oldpos;
		int s_nreloc, s_nlnno, s_flags;
		int s_size;
		unsigned int i;
		uint64_t s_paddr, s_vaddr;

		/*  Read a section header:  */
		len = fread(&scnhdr, 1, sizeof(scnhdr), f);
		if (len != sizeof(scnhdr)) {
			fprintf(stderr, "%s: incomplete section "
			    "header %i\n", filename, secn);
			exit(1);
		}

		/*  Show the section name:  */
		debug("section ");
		for (i=0; i<sizeof(scnhdr.s_name); i++)
			if (scnhdr.s_name[i] >= 32 && scnhdr.s_name[i] < 127)
				debug("%c", scnhdr.s_name[i]);
			else
				break;
		debug(" (");

		unencode(s_paddr,   &scnhdr.s_paddr,   uint32_t);
		unencode(s_vaddr,   &scnhdr.s_vaddr,   uint32_t);
		unencode(s_size,    &scnhdr.s_size,    uint32_t);
		unencode(s_scnptr,  &scnhdr.s_scnptr,  uint32_t);
		unencode(s_relptr,  &scnhdr.s_relptr,  uint32_t);
		unencode(s_lnnoptr, &scnhdr.s_lnnoptr, uint32_t);
		unencode(s_nreloc,  &scnhdr.s_nreloc,  uint16_t);
		unencode(s_nlnno,   &scnhdr.s_nlnno,   uint16_t);
		unencode(s_flags,   &scnhdr.s_flags,   uint32_t);

		debug("0x%x @ 0x%08x, offset 0x%lx, flags 0x%x)\n",
		    (int)s_size, (int)s_vaddr, (long)s_scnptr, (int)s_flags);

		end_addr = s_vaddr + s_size;

		if (s_relptr != 0) {
			/*
			 *  TODO: Read this url, or similar:
			 *  http://www.iecc.com/linker/linker07.html
			 */
			fprintf(stderr, "%s: relocatable code/data in "
			    "section nr %i: not yet implemented\n",
			    filename, secn);
			exit(1);
		}

		/*  Loadable? Then load the section:  */
		if (s_scnptr != 0 && s_size != 0 &&
		    s_vaddr != 0 && !(s_flags & 0x02)) {
			/*  Remember the current file offset:  */
			oldpos = ftello(f);

			/*  Load the section into emulated memory:  */
			fseek(f, s_scnptr, SEEK_SET);
			total_len = 0;
			chunk_size = 1;
			if ((s_vaddr & 0xf) == 0)	chunk_size = 0x10;
			if ((s_vaddr & 0xff) == 0)	chunk_size = 0x100;
			if ((s_vaddr & 0xfff) == 0)	chunk_size = 0x1000;
			while (total_len < s_size) {
				len = chunk_size;
				if (total_len + len > s_size)
					len = s_size - total_len;
				len = fread(buf, 1, chunk_size, f);
				if (len == 0) {
					debug("!!! total_len = %i, "
					    "chunk_size = %i, len = %i\n",
					    total_len, chunk_size, len);
					break;
				}

				m->cpus[0]->memory_rw(m->cpus[0], mem, s_vaddr,
				    &buf[0], len, MEM_WRITE, NO_EXCEPTIONS);
				s_vaddr += len;
				total_len += len;
			}

			/*  Return to position inside the section headers:  */
			fseek(f, oldpos, SEEK_SET);
		}
	}

	if (f_symptr != 0 && f_nsyms != 0) {
		struct ecoff_symhdr symhdr;
		int sym_magic, iextMax, issExtMax, issMax, crfd;
		off_t cbRfdOffset, cbExtOffset, cbSsExtOffset, cbSsOffset;
		char *symbol_data;
		struct ecoff_extsym *extsyms;
		int nsymbols, sym_nr;

		fseek(f, f_symptr, SEEK_SET);

		len = fread(&symhdr, 1, sizeof(symhdr), f);
		if (len != sizeof(symhdr)) {
			fprintf(stderr, "%s: not a complete "
			    "ecoff image: symhdr broken\n", filename);
			exit(1);
		}

		unencode(sym_magic,     &symhdr.magic,         uint16_t);
		unencode(crfd,          &symhdr.crfd,          uint32_t);
		unencode(cbRfdOffset,   &symhdr.cbRfdOffset,   uint32_t);
		unencode(issMax,        &symhdr.issMax,        uint32_t);
		unencode(cbSsOffset,    &symhdr.cbSsOffset,    uint32_t);
		unencode(issExtMax,     &symhdr.issExtMax,     uint32_t);
		unencode(cbSsExtOffset, &symhdr.cbSsExtOffset, uint32_t);
		unencode(iextMax,       &symhdr.iextMax,       uint32_t);
		unencode(cbExtOffset,   &symhdr.cbExtOffset,   uint32_t);

		if (sym_magic != MIPS_MAGIC_SYM) {
			unsigned char *ms_sym_buf;
			struct ms_sym *sym;
			int n_real_symbols = 0;

			debug("bad symbol magic, assuming Microsoft format: ");

			/*
			 *  See http://www.lisoleg.net/lisoleg/elfandlib/
			 *    Microsoft%20Portable%20Executable%20COFF%20For
			 *    mat%20Specification.txt
			 *  for more details.
			 */
			CHECK_ALLOCATION(ms_sym_buf = (unsigned char *)
			    malloc(sizeof(struct ms_sym) * f_nsyms));
			fseek(f, f_symptr, SEEK_SET);
			len = fread(ms_sym_buf, 1,
			    sizeof(struct ms_sym) * f_nsyms, f);
			sym = (struct ms_sym *) ms_sym_buf;
			for (sym_nr=0; sym_nr<f_nsyms; sym_nr++) {
				char name[300];
				uint32_t v, t, altname;
				/*  debug("sym %5i: '", sym_nr);
				for (i=0; i<8 && sym->name[i]; i++)
					debug("%c", sym->name[i]);  */
				v = sym->value[0] + (sym->value[1] << 8)
				    + (sym->value[2] << 16)
				    + ((uint64_t)sym->value[3] << 24);
				altname = sym->name[4] + (sym->name[5] << 8)
				    + (sym->name[6] << 16)
				    + ((uint64_t)sym->name[3] << 24);
				t = (sym->type[1] << 8) + sym->type[0];
				/*  TODO: big endian COFF?  */
				/*  debug("' value=0x%x type=0x%04x", v, t);  */

				if (t == 0x20 && sym->name[0]) {
					memcpy(name, sym->name, 8);
					name[8] = '\0';
					add_symbol_name(&m->symbol_context,
					    v, 0, name, 0, -1);
					n_real_symbols ++;
				} else if (t == 0x20 && !sym->name[0]) {
					off_t ofs;
					ofs = f_symptr + altname +
					    sizeof(struct ms_sym) * f_nsyms;
					fseek(f, ofs, SEEK_SET);
					if (fread(name, 1, sizeof(name), f) != sizeof(name)) {
						fprintf(stderr, "error reading symbol from %s\n", filename);
						exit(1);
					}
					name[sizeof(name)-1] = '\0';
					/*  debug(" [altname=0x%x '%s']",
					    altname, name);  */
					add_symbol_name(&m->symbol_context,
					    v, 0, name, 0, -1);
					n_real_symbols ++;
				}


				if (sym->n_aux_syms) {
					int n = sym->n_aux_syms;
					/*  debug(" aux='");  */
					while (n-- > 0) {
						sym ++; sym_nr ++;
						/*  for (i=0; i<8 &&
						    sym->name[i]; i++)
							debug("%c",
							    sym->name[i]);  */
					}
					/*  debug("'");  */
				}
				/*  debug("\n");  */
				sym ++;
			}

			debug("%i symbols\n", n_real_symbols);
			free(ms_sym_buf);

			goto skip_normal_coff_symbols;
		}

		debug("symbol header: magic = 0x%x\n", sym_magic);

		debug("%i symbols @ 0x%08x (strings @ 0x%08x)\n",
		    iextMax, cbExtOffset, cbSsExtOffset);

		CHECK_ALLOCATION(symbol_data = (char *) malloc(issExtMax + 2));
		memset(symbol_data, 0, issExtMax + 2);
		fseek(f, cbSsExtOffset, SEEK_SET);
		if (fread(symbol_data, 1, issExtMax + 1, f) != (size_t) issExtMax+1) {
			fprintf(stderr, "error reading symbol data from %s\n", filename);
			exit(1);
		}

		nsymbols = iextMax;

		CHECK_ALLOCATION(extsyms = (struct ecoff_extsym *)
		    malloc(iextMax * sizeof(struct ecoff_extsym)));
		memset(extsyms, 0, iextMax * sizeof(struct ecoff_extsym));
		fseek(f, cbExtOffset, SEEK_SET);
		if (fread(extsyms, 1, iextMax * sizeof(struct ecoff_extsym), f) !=
		    iextMax * sizeof(struct ecoff_extsym)) {
			fprintf(stderr, "error reading extsyms from %s\n", filename);
			exit(1);
		}

		/*  Unencode the strindex and value first:  */
		for (sym_nr=0; sym_nr<nsymbols; sym_nr++) {
			uint64_t value, strindex;

			unencode(strindex, &extsyms[sym_nr].es_strindex,
			    uint32_t);
			unencode(value, &extsyms[sym_nr].es_value, uint32_t);

			extsyms[sym_nr].es_strindex = strindex;
			extsyms[sym_nr].es_value    = value;
		}

		for (sym_nr=0; sym_nr<nsymbols; sym_nr++) {
			/*  debug("symbol%6i: 0x%08x = %s\n",
			    sym_nr, (int)extsyms[sym_nr].es_value,
			    symbol_data + extsyms[sym_nr].es_strindex);  */

			add_symbol_name(&m->symbol_context,
			    extsyms[sym_nr].es_value, 0,
			    symbol_data + extsyms[sym_nr].es_strindex, 0, -1);
		}

		free(extsyms);
		free(symbol_data);

skip_normal_coff_symbols:
		;
	}

	fclose(f);

	*entrypointp = a_entry;
	*gpp = a_gp;
	m->file_loaded_end_addr = end_addr;

	if (program_byte_order != -1)
		encoding = program_byte_order;

	if (encoding == ELFDATA2LSB)
		*byte_orderp = EMUL_LITTLE_ENDIAN;
	else
		*byte_orderp = EMUL_BIG_ENDIAN;

	n_executables_loaded ++;
}

