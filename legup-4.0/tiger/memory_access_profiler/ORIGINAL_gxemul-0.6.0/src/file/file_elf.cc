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
 *  COMMENT: ELF file support
 */

/*  Note: Included from file.c.  */


#include "thirdparty/exec_elf.h"

/*  ELF machine types as strings: (same as exec_elf.h)  */
#define N_ELF_MACHINE_TYPES     89
static const char *elf_machine_type[N_ELF_MACHINE_TYPES] = {
        "NONE", "M32", "SPARC", "386",				/*  0..3  */
        "68K", "88K", "486", "860",				/*  4..7  */  
        "MIPS", "S370", "MIPS_RS3_LE", "RS6000",		/*  8..11  */
        "unknown12", "unknown13", "unknown14", "PARISC",	/*  12..15  */
        "NCUBE", "VPP500", "SPARC32PLUS", "960",		/*  16..19  */
        "PPC", "PPC64", "unknown22", "unknown23",		/*  20..23  */
        "unknown24", "unknown25", "unknown26", "unknown27",	/*  24..27  */
        "unknown28", "unknown29", "unknown30", "unknown31",	/*  28..31  */
        "unknown32", "unknown33", "unknown34", "unknown35",	/*  32..35  */
        "V800", "FR20", "RH32", "RCE",				/*  36..39  */
        "ARM", "ALPHA", "SH", "SPARCV9",			/*  40..43  */
        "TRICORE", "ARC", "H8_300", "H8_300H",			/*  44..47  */
        "H8S", "H8_500", "IA_64", "MIPS_X",			/*  48..51  */
        "COLDFIRE", "68HC12", "unknown54", "unknown55",		/*  52..55  */
        "unknown56", "unknown57", "unknown58", "unknown59",	/*  56..59  */
        "unknown60", "unknown61", "AMD64", "unknown63",		/*  60..63  */
        "unknown64", "unknown65", "unknown66", "unknown67",	/*  64..67  */
        "unknown68", "unknown69", "unknown70", "unknown71",	/*  68..71  */
        "unknown72", "unknown73", "unknown74", "unknown75",	/*  72..75  */
        "unknown76", "unknown77", "unknown78", "unknown79",	/*  76..79  */
        "unknown80", "unknown81", "unknown82", "AVR",		/*  80..83  */
        "unknown84", "unknown85", "unknown86", "unknown87",	/*  84..87  */
        "M32R"							/*  88      */
};


/*
 *  file_load_elf():
 *
 *  Loads an ELF image into the emulated memory.  The entry point (read from
 *  the ELF header) and the initial value of the gp register (read from the
 *  ELF symbol table) are stored in the specified CPU's registers.
 *
 *  This is pretty heavy stuff, but is needed because of the heaviness of
 *  ELF files. :-/   Hopefully it will be able to recognize most valid ELFs.
 */
static void file_load_elf(struct machine *m, struct memory *mem,
	char *filename, uint64_t *entrypointp, int arch, uint64_t *gpp,
	int *byte_order, uint64_t *tocp)
{
	Elf32_Ehdr hdr32;
	Elf64_Ehdr hdr64;
	FILE *f;
	uint64_t eentry;
	int len, i, ok;
	int elf64, encoding, eflags;
	int etype, emachine;
	int ephnum, ephentsize, eshnum, eshentsize;
	off_t ephoff, eshoff;
	Elf32_Phdr phdr32;
	Elf64_Phdr phdr64;
	Elf32_Shdr shdr32;
	Elf64_Shdr shdr64;
	Elf32_Sym sym32;
	Elf64_Sym sym64;
	int ofs;
	int chunk_len = 1024, align_len;
	char *symbol_strings = NULL;
	size_t symbol_length = 0;
	const char *s;
	int n_symbols = 0;
	Elf32_Sym *symbols_sym32 = NULL;
	Elf64_Sym *symbols_sym64 = NULL;

	f = fopen(filename, "r");
	if (f == NULL) {
		perror(filename);
		exit(1);
	}

	len = fread(&hdr32, 1, sizeof(Elf32_Ehdr), f);
	if (len < (signed int)sizeof(Elf32_Ehdr)) {
		fprintf(stderr, "%s: not an ELF file image\n", filename);
		exit(1);
	}

	if (memcmp(&hdr32.e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0) {
		fprintf(stderr, "%s: not an ELF file image\n", filename);
		exit(1);
	}

	switch (hdr32.e_ident[EI_CLASS]) {
	case ELFCLASS32:
		elf64 = 0;
		break;
	case ELFCLASS64:
		elf64 = 1;
		fseek(f, 0, SEEK_SET);
		len = fread(&hdr64, 1, sizeof(Elf64_Ehdr), f);
		if (len < (signed int)sizeof(Elf64_Ehdr)) {
			fprintf(stderr, "%s: not an ELF64 file image\n",
			    filename);
			exit(1);
		}
		break;
	default:
		fprintf(stderr, "%s: unknown ELF class '%i'\n",
		    filename, hdr32.e_ident[EI_CLASS]);
		exit(1);
	}

	encoding = hdr32.e_ident[EI_DATA];
	if (encoding != ELFDATA2LSB && encoding != ELFDATA2MSB) {
		fprintf(stderr, "%s: unknown data encoding '%i'\n",
		    filename, hdr32.e_ident[EI_DATA]);
		exit(1);
	}

	if (elf64) {
		unencode(etype,      &hdr64.e_type,      Elf64_Quarter);
		unencode(eflags,     &hdr64.e_flags,     Elf64_Half);
		unencode(emachine,   &hdr64.e_machine,   Elf64_Quarter);
		unencode(eentry,     &hdr64.e_entry,     Elf64_Addr);
		unencode(ephnum,     &hdr64.e_phnum,     Elf64_Quarter);
		unencode(ephentsize, &hdr64.e_phentsize, Elf64_Quarter);
		unencode(ephoff,     &hdr64.e_phoff,     Elf64_Off);
		unencode(eshnum,     &hdr64.e_shnum,     Elf64_Quarter);
		unencode(eshentsize, &hdr64.e_shentsize, Elf64_Quarter);
		unencode(eshoff,     &hdr64.e_shoff,     Elf64_Off);
		if (ephentsize != sizeof(Elf64_Phdr)) {
			fprintf(stderr, "%s: incorrect phentsize? %i, should "
			    "be %i\nPerhaps this is a dynamically linked "
			    "binary (which isn't supported yet).\n", filename,
			    (int)ephentsize, (int)sizeof(Elf64_Phdr));
			exit(1);
		}
		if (eshentsize != sizeof(Elf64_Shdr)) {
			fprintf(stderr, "%s: incorrect shentsize? %i, should "
			    "be %i\nPerhaps this is a dynamically linked "
			    "binary (which isn't supported yet).\n", filename,
			    (int)eshentsize, (int)sizeof(Elf64_Shdr));
			exit(1);
		}
	} else {
		unencode(etype,      &hdr32.e_type,      Elf32_Half);
		unencode(eflags,     &hdr32.e_flags,     Elf32_Word);
		unencode(emachine,   &hdr32.e_machine,   Elf32_Half);
		unencode(eentry,     &hdr32.e_entry,     Elf32_Addr);
		unencode(ephnum,     &hdr32.e_phnum,     Elf32_Half);
		unencode(ephentsize, &hdr32.e_phentsize, Elf32_Half);
		unencode(ephoff,     &hdr32.e_phoff,     Elf32_Off);
		unencode(eshnum,     &hdr32.e_shnum,     Elf32_Half);
		unencode(eshentsize, &hdr32.e_shentsize, Elf32_Half);
		unencode(eshoff,     &hdr32.e_shoff,     Elf32_Off);
		if (ephentsize != sizeof(Elf32_Phdr)) {
			fprintf(stderr, "%s: incorrect phentsize? %i, should "
			    "be %i\nPerhaps this is a dynamically linked "
			    "binary (which isn't supported yet).\n", filename,
			    (int)ephentsize, (int)sizeof(Elf32_Phdr));
			exit(1);
		}
		if (eshentsize != sizeof(Elf32_Shdr)) {
			fprintf(stderr, "%s: incorrect shentsize? %i, should "
			    "be %i\nPerhaps this is a dynamically linked "
			    "binary (which isn't supported yet).\n", filename,
			    (int)eshentsize, (int)sizeof(Elf32_Shdr));
			exit(1);
		}
	}

	if ( etype != ET_EXEC ) {
		fprintf(stderr, "%s is not an ELF Executable file, type = %i\n",
		    filename, etype);
		exit(1);
	}

	ok = 0;
	switch (arch) {
	/*case ARCH_ALPHA:
		switch (emachine) {
		case EM_ALPHA:
		case -28634:
			ok = 1;
		}
		break;*/
	case ARCH_ARM:
		switch (emachine) {
		case EM_ARM:
			ok = 1;
		}
		break;
	/*  case ARCH_AVR:
		switch (emachine) {
		case EM_AVR:
			ok = 1;
		}
		break;
	case ARCH_AVR32:
		switch (emachine) {
		case 6317:
			ok = 1;
		}
		break;
	case ARCH_HPPA:
		switch (emachine) {
		case EM_PARISC:
			ok = 1;
		}
		break;
	case ARCH_I960:
		switch (emachine) {
		case EM_960:
			ok = 1;
		}
		break;
	case ARCH_IA64:
		switch (emachine) {
		case EM_IA_64:
			ok = 1;
		}
		break;
	case ARCH_M68K:
		switch (emachine) {
		case EM_68K:
			ok = 1;
		}
		break;
	case ARCH_M32R:
		switch (emachine) {
		case EM_M32R:
			ok = 1;
		}
		break;*/
	case ARCH_MIPS:
		switch (emachine) {
		case EM_MIPS:
		case EM_MIPS_RS3_LE:
			ok = 1;
		}
		break;
	case ARCH_PPC:
		switch (emachine) {
		case EM_PPC:
		case EM_PPC64:
			ok = 1;
		}
		break;
	case ARCH_SH:
		switch (emachine) {
		case EM_SH:
			ok = 1;
		}
		break;
	/*case ARCH_SPARC:
		switch (emachine) {
		case EM_SPARC:
		case EM_SPARCV9:
			ok = 1;
		}
		break;
	case ARCH_X86:
		switch (emachine) {
		case EM_386:
		case EM_486:
			*tocp = 1;
			ok = 1;
			break;
		case EM_AMD64:
			*tocp = 2;
			ok = 1;
			break;
		}
		break;  */
	default:
		fatal("file.c: INTERNAL ERROR: Unimplemented arch!\n");
	}
	if (!ok) {
		fprintf(stderr, "%s: this is a ", filename);
		if (emachine >= 0 && emachine < N_ELF_MACHINE_TYPES)
			fprintf(stderr, "%s", elf_machine_type[emachine]);
		else
			fprintf(stderr, "machine type '%i'", emachine);
		fprintf(stderr, " ELF binary!\n");
		exit(1);
	}

	s = "entry point";
	if (elf64 && arch == ARCH_PPC)
		s = "function descriptor at";

	debug("ELF%i %s, %s 0x", elf64? 64 : 32,
	    encoding == ELFDATA2LSB? "LSB (LE)" : "MSB (BE)", s);

	if (elf64)
		debug("%016"PRIx64"\n", (uint64_t) eentry);
	else
		debug("%08"PRIx32"\n", (uint32_t) eentry);

	/*
	 *  SH64: 32-bit instruction encoding?
	 */
	if (arch == ARCH_SH && (eentry & 1)) {
		fatal("SH64: 32-bit instruction encoding: TODO\n");
		/*  m->cpus[0]->cd.sh.compact = 0;  */
		m->cpus[0]->cd.sh.cpu_type.bits = 64;
		exit(1);
	}

	/*  Read the program headers:  */

	for (i=0; i<ephnum; i++) {
		int p_type;
		uint64_t p_offset;
		uint64_t p_vaddr;
		uint64_t p_paddr;
		uint64_t p_filesz;
		uint64_t p_memsz;
		int p_flags;
		int p_align;
		int allRead;

		fseek(f, ephoff + i * ephentsize, SEEK_SET);

		if (elf64) {
			allRead = fread(&phdr64, 1, sizeof(Elf64_Phdr), f) == sizeof(Elf64_Phdr);
			unencode(p_type,    &phdr64.p_type,    Elf64_Half);
			unencode(p_flags,   &phdr64.p_flags,   Elf64_Half);
			unencode(p_offset,  &phdr64.p_offset,  Elf64_Off);
			unencode(p_vaddr,   &phdr64.p_vaddr,   Elf64_Addr);
			unencode(p_paddr,   &phdr64.p_paddr,   Elf64_Addr);
			unencode(p_filesz,  &phdr64.p_filesz,  Elf64_Xword);
			unencode(p_memsz,   &phdr64.p_memsz,   Elf64_Xword);
			unencode(p_align,   &phdr64.p_align,   Elf64_Xword);
		} else {
			allRead = fread(&phdr32, 1, sizeof(Elf32_Phdr), f) == sizeof(Elf32_Phdr);
			unencode(p_type,    &phdr32.p_type,    Elf32_Word);
			unencode(p_offset,  &phdr32.p_offset,  Elf32_Off);
			unencode(p_vaddr,   &phdr32.p_vaddr,   Elf32_Addr);
			unencode(p_paddr,   &phdr32.p_paddr,   Elf32_Addr);
			unencode(p_filesz,  &phdr32.p_filesz,  Elf32_Word);
			unencode(p_memsz,   &phdr32.p_memsz,   Elf32_Word);
			unencode(p_flags,   &phdr32.p_flags,   Elf32_Word);
			unencode(p_align,   &phdr32.p_align,   Elf32_Word);
		}

		if (!allRead) {
			fprintf(stderr, "Could not read Phdr from %s. Aborting.\n", filename);
			exit(1);
		}

		/*
		 *  Hack for loading PPC kernels that are linked to high
		 *  addresses. (This requires enabling of instruction and
		 *  data virtual address translation.)
		 */
		if (arch == ARCH_PPC) {
			if ( (elf64 && (p_vaddr >> 60) != 0) ||
			    (!elf64 && (p_vaddr >> 28) != 0) )
				m->cpus[m->bootstrap_cpu]->
				    cd.ppc.msr |= PPC_MSR_IR | PPC_MSR_DR;
		}

		if (p_memsz != 0 && (p_type == PT_LOAD ||
		    (p_type & PF_MASKPROC) == PT_MIPS_REGINFO)) {
			debug("chunk %i (", i);
			if (p_type == PT_LOAD)
				debug("load");
			else
				debug("0x%08"PRIx32, (uint32_t) p_type);

			debug(") @ 0x%"PRIx64", vaddr 0x", (uint64_t) p_offset);

			if (elf64)
				debug("%016"PRIx64, (uint64_t) p_vaddr);
			else
				debug("%08"PRIx32, (uint32_t) p_vaddr);

			debug(" len=0x%"PRIx64"\n", (uint64_t) p_memsz);

			if (p_vaddr != p_paddr) {
				if (elf64)
					debug("NOTE: vaddr (0x%"PRIx64") and "
					    "paddr (0x%"PRIx64") differ; using "
					    "vaddr\n", (uint64_t) p_vaddr,
					    (uint64_t) p_paddr);
				else
					debug("NOTE: vaddr (0x%08"PRIx32") and "
					    "paddr (0x%08"PRIx32") differ; usin"
					    "g vaddr\n", (uint32_t) p_vaddr,
					    (uint32_t)p_paddr);
			}

			if (p_memsz < p_filesz) {
				fprintf(stderr, "%s: memsz < filesz. TODO: how"
				    " to handle this? memsz=%016"PRIx64
				    " filesz=%016"PRIx64"\n", filename,
				    (uint64_t) p_memsz, (uint64_t) p_filesz);
				exit(1);
			}

			fseek(f, p_offset, SEEK_SET);
			align_len = 1;
			if ((p_vaddr & 0xf)==0)		align_len = 0x10;
			if ((p_vaddr & 0x3f)==0)	align_len = 0x40;
			if ((p_vaddr & 0xff)==0)	align_len = 0x100;
			if ((p_vaddr & 0xfff)==0)	align_len = 0x1000;
			if ((p_vaddr & 0x3fff)==0)	align_len = 0x4000;
			if ((p_vaddr & 0xffff)==0)	align_len = 0x10000;
			ofs = 0;  len = chunk_len = align_len;
			while (ofs < (int64_t)p_filesz && len==chunk_len) {
				unsigned char *ch;
				int i = 0;

				CHECK_ALLOCATION(ch = (unsigned char *) malloc(chunk_len));

				/*  Switch to larger size, if possible:  */
				if (align_len < 0x10000 &&
				    ((p_vaddr + ofs) & 0xffff)==0) {
					align_len = 0x10000;
					len = chunk_len = align_len;
					free(ch);
					CHECK_ALLOCATION(ch=(unsigned char *)malloc(chunk_len));
				} else if (align_len < 0x1000 &&
				    ((p_vaddr + ofs) & 0xfff)==0) {
					align_len = 0x1000;
					len = chunk_len = align_len;
					free(ch);
					CHECK_ALLOCATION(ch=(unsigned char *)malloc(chunk_len));
				}

				len = fread(&ch[0], 1, chunk_len, f);
				if (ofs + len > (int64_t)p_filesz)
					len = p_filesz - ofs;

				while (i < len) {
					size_t len_to_copy;
					len_to_copy = (i + align_len) <= len?
					    align_len : len - i;
					m->cpus[0]->memory_rw(m->cpus[0], mem,
					    p_vaddr + ofs, &ch[i], len_to_copy,
					    MEM_WRITE, NO_EXCEPTIONS);
					ofs += align_len;
					i += align_len;
				}

				free(ch);
			}
		}
	}

	/*
	 *  Read the section headers to find the address of the _gp
	 *  symbol (for MIPS):
	 */

	for (i=0; i<eshnum; i++) {
		int sh_name, sh_type, sh_flags, sh_link, sh_info, sh_entsize;
		uint64_t sh_addr, sh_size, sh_addralign;
		off_t sh_offset;
		int n_entries;	/*  for reading the symbol / string tables  */

		/*  debug("section header %i at %016"PRIx64"\n", i,
		    (uint64_t) eshoff+i*eshentsize);  */

		fseek(f, eshoff + i * eshentsize, SEEK_SET);

		if (elf64) {
			len = fread(&shdr64, 1, sizeof(Elf64_Shdr), f);
			if (len != sizeof(Elf64_Shdr)) {
				fprintf(stderr, "couldn't read header\n");
				exit(1);
			}
			unencode(sh_name,    &shdr64.sh_name, Elf64_Half);
			unencode(sh_type,    &shdr64.sh_type, Elf64_Half);
			unencode(sh_flags,   &shdr64.sh_flags, Elf64_Xword);
			unencode(sh_addr,    &shdr64.sh_addr, Elf64_Addr);
			unencode(sh_offset,  &shdr64.sh_offset, Elf64_Off);
			unencode(sh_size,    &shdr64.sh_size, Elf64_Xword);
			unencode(sh_link,    &shdr64.sh_link, Elf64_Half);
			unencode(sh_info,    &shdr64.sh_info, Elf64_Half);
			unencode(sh_addralign, &shdr64.sh_addralign,
			    Elf64_Xword);
			unencode(sh_entsize, &shdr64.sh_entsize, Elf64_Xword);
		} else {
			len = fread(&shdr32, 1, sizeof(Elf32_Shdr), f);
			if (len != sizeof(Elf32_Shdr)) {
				fprintf(stderr, "couldn't read header\n");
				exit(1);
			}
			unencode(sh_name,      &shdr32.sh_name,    Elf32_Word);
			unencode(sh_type,      &shdr32.sh_type,    Elf32_Word);
			unencode(sh_flags,     &shdr32.sh_flags,   Elf32_Word);
			unencode(sh_addr,      &shdr32.sh_addr,    Elf32_Addr);
			unencode(sh_offset,    &shdr32.sh_offset,  Elf32_Off);
			unencode(sh_size,      &shdr32.sh_size,    Elf32_Word);
			unencode(sh_link,      &shdr32.sh_link,    Elf32_Word);
			unencode(sh_info,      &shdr32.sh_info,    Elf32_Word);
			unencode(sh_addralign, &shdr32.sh_addralign,Elf32_Word);
			unencode(sh_entsize,   &shdr32.sh_entsize, Elf32_Word);
		}

		/*  debug("sh_name=%04lx, sh_type=%08lx, sh_flags=%08lx"
		    " sh_size=%06lx sh_entsize=%03lx\n",
		    (long)sh_name, (long)sh_type, (long)sh_flags,
		    (long)sh_size, (long)sh_entsize);  */

		/*  Perhaps it is bad to reuse sh_entsize like this?  TODO  */
		if (elf64)
			sh_entsize = sizeof(Elf64_Sym);
		else
			sh_entsize = sizeof(Elf32_Sym);

		if (sh_type == SHT_SYMTAB) {
			size_t len;
			n_entries = sh_size / sh_entsize;

			fseek(f, sh_offset, SEEK_SET);

			if (elf64) {
				if (symbols_sym64 != NULL)
					free(symbols_sym64);

				CHECK_ALLOCATION(symbols_sym64 = (Elf64_Sym *)
				    malloc(sh_size));

				len = fread(symbols_sym64, 1, sh_entsize *
				    n_entries, f);
			} else {
				if (symbols_sym32 != NULL)
					free(symbols_sym32);

				CHECK_ALLOCATION(symbols_sym32 = (Elf32_Sym *)
				    malloc(sh_size));

				len = fread(symbols_sym32, 1,
				    sh_entsize * n_entries, f);
			}

			if (len != sh_size) {
				fprintf(stderr, "could not read symbols from "
				    "%s\n", filename);
				exit(1);
			}

			debug("%i symbol entries at 0x%"PRIx64"\n",
			    (int) n_entries, (uint64_t) sh_offset);

			n_symbols = n_entries;
		}

		/*
		 *  TODO:  This is incorrect, there may be several strtab
		 *         sections.
		 *
		 *  For now, the simple/stupid guess that the largest string
		 *  table is the one to use seems to be good enough.
		 */

		if (sh_type == SHT_STRTAB && sh_size > symbol_length) {
			size_t len;

			if (symbol_strings != NULL)
				free(symbol_strings);

			CHECK_ALLOCATION(symbol_strings = (char *) malloc(sh_size + 1));

			fseek(f, sh_offset, SEEK_SET);
			len = fread(symbol_strings, 1, sh_size, f);
			if (len != sh_size) {
				fprintf(stderr, "could not read symbols from "
				    "%s\n", filename);
				exit(1);
			}

			debug("%i bytes of symbol strings at 0x%"PRIx64"\n",
			    (int) sh_size, (uint64_t) sh_offset);

			symbol_strings[sh_size] = '\0';
			symbol_length = sh_size;
		}
	}

	fclose(f);

	/*  Decode symbols:  */
	if (symbol_strings != NULL) {
		for (i=0; i<n_symbols; i++) {
			uint64_t st_name, addr, size;
			int st_info;

			if (elf64) {
				sym64 = symbols_sym64[i];
				unencode(st_name, &sym64.st_name,  Elf64_Half);
				unencode(st_info, &sym64.st_info,  Elf_Byte);
				unencode(addr,    &sym64.st_value, Elf64_Addr);
				unencode(size,    &sym64.st_size,  Elf64_Xword);
			} else {
				sym32 = symbols_sym32[i];
				unencode(st_name, &sym32.st_name,  Elf32_Word);
				unencode(st_info, &sym32.st_info,  Elf_Byte);
				unencode(addr,    &sym32.st_value, Elf32_Word);
				unencode(size,    &sym32.st_size, Elf32_Word);
			}

			/*  debug("symbol info=0x%02x addr=0x%016"PRIx64
			    " (%i) '%s'\n", st_info, (uint64_t) addr,
			    st_name, symbol_strings + st_name);  */

			if (size == 0)
				size ++;

			if (addr != 0) /* && ((st_info >> 4) & 0xf)
			    >= STB_GLOBAL) */ {
				/*  debug("symbol info=0x%02x addr=0x%016"PRIx64
				    " '%s'\n", st_info, (uint64_t) addr,
				    symbol_strings + st_name);  */
				add_symbol_name(&m->symbol_context,
				    addr, size, symbol_strings + st_name,
				    0, -1);
			}

			if (strcmp(symbol_strings + st_name, "_gp") == 0) {
				debug("found _gp address: 0x");
				if (elf64)
					debug("%016"PRIx64"\n", (uint64_t)addr);
				else
					debug("%08"PRIx32"\n", (uint32_t)addr);
				*gpp = addr;
			}
		}
	}

	*entrypointp = eentry;

	if (encoding == ELFDATA2LSB)
		*byte_order = EMUL_LITTLE_ENDIAN;
	else
		*byte_order = EMUL_BIG_ENDIAN;

	if (elf64 && arch == ARCH_PPC) {
		/*
		 *  Special case for 64-bit PPC ELFs:
		 *
		 *  The ELF starting symbol points to a ".opd" section
		 *  which contains a function descriptor:
		 *
		 *      uint64_t  start;
		 *      uint64_t  toc_base;
		 *      uint64_t  something_else;       (?)
		 */
		int res;
		unsigned char b[sizeof(uint64_t)];
		uint64_t toc_base;

		debug("PPC64: ");

		res = m->cpus[0]->memory_rw(m->cpus[0], mem, eentry, b,
		    sizeof(b), MEM_READ, NO_EXCEPTIONS);
		if (!res)
			debug(" [WARNING: could not read memory?] ");

		/*  PPC are always big-endian:  */
		*entrypointp = ((uint64_t)b[0] << 56) +
		    ((uint64_t)b[1] << 48) + ((uint64_t)b[2] << 40) +
		    ((uint64_t)b[3] << 32) + ((uint64_t)b[4] << 24) +
		    ((uint64_t)b[5] << 16) + ((uint64_t)b[6] << 8) +
		    (uint64_t)b[7];

		res = m->cpus[0]->memory_rw(m->cpus[0], mem, eentry + 8,
		    b, sizeof(b), MEM_READ, NO_EXCEPTIONS);
		if (!res)
			fatal(" [WARNING: could not read memory?] ");

		toc_base = ((uint64_t)b[0] << 56) +
		    ((uint64_t)b[1] << 48) + ((uint64_t)b[2] << 40) +
		    ((uint64_t)b[3] << 32) + ((uint64_t)b[4] << 24) +
		    ((uint64_t)b[5] << 16) + ((uint64_t)b[6] << 8) +
		    (uint64_t)b[7];

		debug("entrypoint 0x%016"PRIx64", toc_base 0x%016"PRIx64"\n",
		    (uint64_t) *entrypointp, (uint64_t) toc_base);
		if (tocp != NULL)
			*tocp = toc_base;
	}

	n_executables_loaded ++;
}

