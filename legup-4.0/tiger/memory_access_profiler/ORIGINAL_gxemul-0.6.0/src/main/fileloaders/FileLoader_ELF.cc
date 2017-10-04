/*
 *  Copyright (C) 2008-2010  Anders Gavare.  All rights reserved.
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
 */

#include <assert.h>
#include <string.h>
#include <fstream>
#include <iomanip>

using std::setw;
using std::setfill;
using std::ifstream;

#include "AddressDataBus.h"
#include "components/CPUComponent.h"
#include "FileLoader_ELF.h"

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


FileLoader_ELF::FileLoader_ELF(const string& filename)
	: FileLoaderImpl(filename)
{
}


string FileLoader_ELF::DetectFileType(unsigned char *buf, size_t buflen, float& matchness) const
{
	if (buflen < sizeof(Elf32_Ehdr))
		return "";

	// Note: The e_ident part of the 32-bit and the 64-bit variants have
	// the same layout, so it is safe to only check the 32-bit variant here.
	Elf32_Ehdr* elf32_ehdr = (Elf32_Ehdr*) buf;
	if (elf32_ehdr->e_ident[EI_MAG0] == ELFMAG0 &&
	    elf32_ehdr->e_ident[EI_MAG1] == ELFMAG1 &&
	    elf32_ehdr->e_ident[EI_MAG2] == ELFMAG2 &&
	    elf32_ehdr->e_ident[EI_MAG3] == ELFMAG3) {
		// We are here if this is either an ELF32 or ELF64.
		int elfClass = elf32_ehdr->e_ident[EI_CLASS];

		matchness = 1.0;
		if (elfClass == ELFCLASS32)
			return "ELF32";
		if (elfClass == ELFCLASS64)
			return "ELF64";

		matchness = 0.0;
		stringstream ss;
		ss << "ELF Unknown class " << elfClass;
		return ss.str();
	}

	return "";
}


bool FileLoader_ELF::LoadIntoComponent(refcount_ptr<Component> component, ostream& messages) const
{
	AddressDataBus* bus = component->AsAddressDataBus();
	if (bus == NULL) {
		messages << "Target is not an AddressDataBus.\n";
		return false;
	}

	ifstream file(Filename().c_str());
	if (!file.is_open()) {
		messages << "Unable to read file.\n";
		return false;
	}

	char buf[64];

	// buf must be large enough for the largest possible header we wish
	// to examine to fit.
	assert(sizeof(buf) >= sizeof(Elf32_Ehdr));
	assert(sizeof(buf) >= sizeof(Elf64_Ehdr));

	memset(buf, 0, sizeof(buf));
	file.read(buf, sizeof(buf));

	// Note: The e_ident part of the 32-bit and the 64-bit variants have
	// the same layout, so it is safe to only use the 32-bit variant here.
	Elf32_Ehdr* ehdr32 = (Elf32_Ehdr*) buf;
	Elf64_Ehdr* ehdr64 = (Elf64_Ehdr*) buf;
	if (ehdr32->e_ident[EI_MAG0] != ELFMAG0 ||
	    ehdr32->e_ident[EI_MAG1] != ELFMAG1 ||
	    ehdr32->e_ident[EI_MAG2] != ELFMAG2 ||
	    ehdr32->e_ident[EI_MAG3] != ELFMAG3) {
		messages << "Not an ELF.\n";
		return false;
	}

	int elfClass = ehdr32->e_ident[EI_CLASS];
	int elfDataEncoding = ehdr32->e_ident[EI_DATA];
	int elfVersion = ehdr32->e_ident[EI_VERSION];

	if (elfClass != ELFCLASS32 && elfClass != ELFCLASS64) {
		messages << "Unknown ELF class.\n";
		return false;
	}
	if (elfDataEncoding != ELFDATA2LSB && elfDataEncoding != ELFDATA2MSB) {
		messages << "Unknown ELF data encoding.\n";
		return false;
	}
	if (elfVersion != EV_CURRENT) {
		messages << "Unknown ELF version.\n";
		return false;
	}

	bool elf32 = elfClass == ELFCLASS32;

#define ELF_HEADER_VAR(hdr32,hdr64,type,name) type name = elf32? hdr32->name  \
							       : hdr64->name; \
	if (elfDataEncoding == ELFDATA2LSB) {				      \
		int size = elf32? sizeof(hdr32->name) : sizeof(hdr64->name);  \
		switch (size) {						      \
		case 2:	name = LE16_TO_HOST(name); break;		      \
		case 4:	name = LE32_TO_HOST(name); break;		      \
		case 8:	name = LE64_TO_HOST(name); break;		      \
		}							      \
	} else {							      \
		int size = elf32? sizeof(hdr32->name) : sizeof(hdr64->name);  \
		switch (size) {						      \
		case 2:	name = BE16_TO_HOST(name); break;		      \
		case 4:	name = BE32_TO_HOST(name); break;		      \
		case 8:	name = BE64_TO_HOST(name); break;		      \
		}							      \
	}

	ELF_HEADER_VAR(ehdr32, ehdr64, uint64_t, e_type);

	if (e_type != ET_EXEC) {
		messages << "ELF file is not an Executable.\n";
		return false;
	}

	ELF_HEADER_VAR(ehdr32, ehdr64, uint64_t, e_entry);
	ELF_HEADER_VAR(ehdr32, ehdr64, uint64_t, e_machine);

	ELF_HEADER_VAR(ehdr32, ehdr64, uint64_t, e_phoff);
	ELF_HEADER_VAR(ehdr32, ehdr64, uint64_t, e_phentsize);
	ELF_HEADER_VAR(ehdr32, ehdr64, uint64_t, e_phnum);

	ELF_HEADER_VAR(ehdr32, ehdr64, uint64_t, e_shoff);
	ELF_HEADER_VAR(ehdr32, ehdr64, uint64_t, e_shentsize);
	ELF_HEADER_VAR(ehdr32, ehdr64, uint64_t, e_shnum);

	size_t expectedPhentSize = (elf32? sizeof(Elf32_Phdr) : sizeof(Elf64_Phdr));
	if (e_phentsize != expectedPhentSize) {
		messages << "Incorrect ELF phentsize? " << e_phentsize << ", should "
		    "be " << expectedPhentSize << "\n"
		    "Perhaps this is a dynamically linked "
		    "binary (which isn't supported yet).\n";
		return false;
	}

	size_t expectedShentSize = (elf32? sizeof(Elf32_Shdr) : sizeof(Elf64_Shdr));
	if (e_shentsize != expectedShentSize) {
		messages << "Incorrect ELF shentsize? " << e_shentsize << ", should "
		    "be " << expectedShentSize << "\n"
		    "Perhaps this is a dynamically linked "
		    "binary (which isn't supported yet).\n";
		return false;
	}

	if (e_machine >= 0 && e_machine < N_ELF_MACHINE_TYPES)
		messages << elf_machine_type[e_machine];
	else
		messages << "machine type '" << e_machine << "'";
	messages << " ELF" << (elf32? 32 : 64) << " ";

	messages << (elfDataEncoding == ELFDATA2LSB? "LSB (LE)" : "MSB (BE)") << ": ";

	if (!elf32 && (e_machine == EM_PPC || e_machine == EM_PPC64))
		messages << "PPC function descriptor at";
	else
		messages << "entry point";

	messages << " 0x";
	messages.flags(std::ios::hex);

	// Special case for MIPS: 32-bit addresses are sign-extended.
	if (e_machine == EM_MIPS && elf32)
		e_entry = (int32_t) e_entry;

	uint64_t display_entry_point = e_entry;

	// MIPS16 encoding (16-bit words) is indicated by the lowest bit of the PC.
	bool mips16 = false;
	if (e_machine == EM_MIPS && (e_entry & 1)) {
		display_entry_point &= ~1;
		mips16 = true;
	}

	// SHmedia (SH64) 32-bit encoding is indicated by the lowest bit of the PC.
	bool shmedia = false;
	if (e_machine == EM_SH && (e_entry & 1)) {
		display_entry_point &= ~1;
		shmedia = true;
	}

	if (elf32)
		messages << setw(8) << setfill('0') << (uint32_t) display_entry_point;
	else
		messages << setw(16) << setfill('0') << (uint64_t) display_entry_point;

	if (mips16)
		messages << " (MIPS16 encoding)";

	if (shmedia)
		messages << " (SHmedia encoding)";

	messages << "\n";

	// PROGRAM HEADERS
	size_t i;
	for (i=0; i<e_phnum; ++i) {
		// Load Phdr number i:
		file.seekg(e_phoff + i * e_phentsize, std::ios::beg);

		char phdr_buf[64];
		assert(sizeof(phdr_buf) >= sizeof(Elf32_Phdr));
		assert(sizeof(phdr_buf) >= sizeof(Elf64_Phdr));
		Elf32_Phdr* phdr32 = (Elf32_Phdr*) phdr_buf;
		Elf64_Phdr* phdr64 = (Elf64_Phdr*) phdr_buf;

		memset(phdr_buf, 0, sizeof(phdr_buf));

		int toRead = elf32? sizeof(Elf32_Phdr) : sizeof(Elf64_Phdr);
		file.read(phdr_buf, toRead);
		if (file.gcount() != toRead) {
			messages << "Unable to read Phdr.\n";
			return false;
		}
		
		ELF_HEADER_VAR(phdr32, phdr64, uint64_t, p_type);
		ELF_HEADER_VAR(phdr32, phdr64, uint64_t, p_offset);
		ELF_HEADER_VAR(phdr32, phdr64, uint64_t, p_vaddr);
		ELF_HEADER_VAR(phdr32, phdr64, uint64_t, p_filesz);
		ELF_HEADER_VAR(phdr32, phdr64, uint64_t, p_memsz);

		// Skip non-loadable program segments:		
		if (p_type != PT_LOAD)
			continue;

		if (p_memsz < p_filesz) {
			messages << "memsz < filesz. TODO: how"
			    " to handle this? memsz = " << p_memsz <<
			    ", filesz = " << p_filesz << "\n";
			return false;
		}

		// Special case for MIPS: 32-bit addresses are sign-extended.
		if (e_machine == EM_MIPS && elf32)
			p_vaddr = (int32_t) p_vaddr;

		messages.flags(std::ios::hex);
		messages << "loadable chunk: vaddr 0x";

		if (elf32)
			messages << setw(8) << setfill('0') << (uint32_t) p_vaddr;
		else
			messages << setw(16) << setfill('0') << (uint64_t) p_vaddr;

		messages.flags(std::ios::dec);
		messages << ", " << p_filesz << " bytes\n";

		file.seekg(p_offset, std::ios::beg);
		char databuf[65536];
		uint64_t bytesRead = 0;
		uint64_t vaddrToWriteTo = p_vaddr;

		while (bytesRead < p_filesz) {
			int sizeToRead = sizeof(databuf);
			if (sizeToRead + bytesRead > p_filesz)
				sizeToRead = p_filesz - bytesRead;

			assert(sizeToRead != 0);
			memset(databuf, 0, sizeToRead);

			file.read(databuf, sizeToRead);
			int bytesReadThisTime = file.gcount();
			bytesRead += bytesReadThisTime;

			// Write to the bus, one byte at a time.
			for (int k=0; k<bytesReadThisTime; ++k) {
				bus->AddressSelect(vaddrToWriteTo);
				if (!bus->WriteData(databuf[k])) {
					messages.flags(std::ios::hex);
					messages << "Failed to write data to "
					    "virtual address 0x"
					    << vaddrToWriteTo << "\n";
					return false;
				}

				++ vaddrToWriteTo;
			}
		}
	}

	// SECTION HEADERS
	vector<char> symtab;
	vector<char> symstrings;
	for (i=0; i<e_shnum; ++i) {
		// Load Shdr number i:
		file.seekg(e_shoff + i * e_shentsize, std::ios::beg);

		char shdr_buf[64];
		assert(sizeof(shdr_buf) >= sizeof(Elf32_Shdr));
		assert(sizeof(shdr_buf) >= sizeof(Elf64_Shdr));
		Elf32_Shdr* shdr32 = (Elf32_Shdr*) shdr_buf;
		Elf64_Shdr* shdr64 = (Elf64_Shdr*) shdr_buf;

		memset(shdr_buf, 0, sizeof(shdr_buf));
		
		int toRead = elf32? sizeof(Elf32_Shdr) : sizeof(Elf64_Shdr);
		file.read(shdr_buf, toRead);
		if (file.gcount() != toRead) {
			messages << "Unable to read Shdr.\n";
			return false;
		}

		ELF_HEADER_VAR(shdr32, shdr64, uint64_t, sh_name);
		ELF_HEADER_VAR(shdr32, shdr64, uint64_t, sh_type);
		ELF_HEADER_VAR(shdr32, shdr64, uint64_t, sh_flags);
		ELF_HEADER_VAR(shdr32, shdr64, uint64_t, sh_addr);
		ELF_HEADER_VAR(shdr32, shdr64, uint64_t, sh_offset);
		ELF_HEADER_VAR(shdr32, shdr64, uint64_t, sh_size);
		ELF_HEADER_VAR(shdr32, shdr64, uint64_t, sh_link);
		ELF_HEADER_VAR(shdr32, shdr64, uint64_t, sh_info);
		ELF_HEADER_VAR(shdr32, shdr64, uint64_t, sh_addralign);
		ELF_HEADER_VAR(shdr32, shdr64, uint64_t, sh_entsize);

		if (sh_type == SHT_SYMTAB) {
			if (symtab.size() > 0) {
				messages << "symtab: another symtab already loaded? TODO\n";
				return false;
			}
			
			int entrySize = elf32? sizeof(Elf32_Sym) : sizeof(Elf64_Sym);
			int nEntries = sh_size / entrySize;
			
			messages.flags(std::ios::dec);
			messages << "symtab: " << nEntries << " symbols at 0x";
			messages.flags(std::ios::hex);
			messages << sh_offset << "\n";
			
			symtab.resize(sh_size);
			file.seekg(sh_offset, std::ios::beg);
			file.read(&symtab[0], sh_size);
			if ((uint64_t) file.gcount() != sh_size) {
				messages << "Failed to read all " << sh_size << " symbol bytes.\n";
				return false;
			}
		}
		
		// TODO/HACK: Figure out which strtab to use. For now, simply
		// use the largest one!
		if (sh_type == SHT_STRTAB && sh_size > symstrings.size()) {
			messages.flags(std::ios::dec);
			messages << "strtab: " << sh_size << " bytes at 0x";
			messages.flags(std::ios::hex);
			messages << sh_offset << "\n";

			symstrings.resize(sh_size);
			file.seekg(sh_offset, std::ios::beg);
			file.read(&symstrings[0], sh_size);
			if ((uint64_t) file.gcount() != sh_size) {
				messages << "Failed to read all " << sh_size << " symbol string bytes.\n";
				return false;
			}
		}
	}

	SymbolRegistry* symbolRegistry = NULL;
	CPUComponent* cpu = component->AsCPUComponent();
	if (cpu != NULL)
		symbolRegistry = &cpu->GetSymbolRegistry();

	// Symbols
	if (symbolRegistry != NULL && symtab.size() > 0 && symstrings.size() > 0) {
		int entrySize = elf32? sizeof(Elf32_Sym) : sizeof(Elf64_Sym);
		int nEntries = symtab.size() / entrySize;

		// For safety:
		symstrings.resize(symstrings.size() + 1);
		symstrings[symstrings.size() - 1] = '\0';

		int nsymbols = 0;
		
		messages.flags(std::ios::hex);

		for (int j=0; j<nEntries; j++) {
			size_t p = j * entrySize;
			Elf32_Sym* sym32 = (Elf32_Sym*) &symtab[p];
			Elf64_Sym* sym64 = (Elf64_Sym*) &symtab[p];

			ELF_HEADER_VAR(sym32, sym64, uint64_t, st_name);
			ELF_HEADER_VAR(sym32, sym64, uint64_t, st_info);
			ELF_HEADER_VAR(sym32, sym64, uint64_t, st_value);
			ELF_HEADER_VAR(sym32, sym64, uint64_t, st_size);

			int bind = elf32? ELF32_ST_BIND(st_info) : ELF64_ST_BIND(st_info);
			if (bind == STB_LOCAL)
				continue;

			if (st_size == 0)
				st_size ++;

			if (st_name >= symstrings.size() - 1) {
				messages << "symbol pointer mismatch?\n";
				continue;
			}

			string symbol = &symstrings[st_name];

			// Special case for MIPS: 32-bit addresses are sign-extended.
			if (e_machine == EM_MIPS && elf32)
				st_value = (int32_t) st_value;

			// Special case for MIPS: _gp symbol initiates the GP register.
			if (e_machine == EM_MIPS && symbol == "_gp") {
				messages << "found _gp address: 0x";
				if (elf32)
					messages << setw(8) << setfill('0') << (uint32_t) st_value << "\n";
				else
					messages << setw(16) << setfill('0') << (uint64_t) st_value << "\n";

				stringstream ss;
				ss << st_value;
				component->SetVariableValue("gp", ss.str());
			}

			// FOR DEBUGGING ONLY:
			// messages << "symbol name '" << symbol << "', addr 0x" <<
			//    st_value << ", size 0x" << st_size << "\n";

			// Add this symbol to the symbol registry:
			symbolRegistry->AddSymbol(symbol, st_value);
			++ nsymbols;
		}

		messages.flags(std::ios::dec);
		messages << nsymbols << " symbols read\n";
	}

	// Set the CPU's entry point.

#if 0
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
#endif

	stringstream ss;
	ss << e_entry;
	component->SetVariableValue("pc", ss.str());
	component->SetVariableValue("bigendian",
	    elfDataEncoding == ELFDATA2LSB? "false" : "true");

	return true;
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

#include "ComponentFactory.h"

static void Test_FileLoader_ELF_Constructor()
{
	FileLoader_ELF elfLoader("test/FileLoader_ELF_MIPS");
}

UNITTESTS(FileLoader_ELF)
{
	UNITTEST(Test_FileLoader_ELF_Constructor);

	// TODO
}

#endif
