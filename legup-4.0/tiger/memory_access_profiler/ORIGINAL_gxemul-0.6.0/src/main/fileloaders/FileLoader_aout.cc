/*
 *  Copyright (C) 2009-2010  Anders Gavare.  All rights reserved.
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
#include "FileLoader_aout.h"

#include "thirdparty/exec_aout.h"

struct aout_symbol {
	uint32_t	strindex;
	uint32_t	type;
	uint32_t	addr;
};


FileLoader_aout::FileLoader_aout(const string& filename)
	: FileLoaderImpl(filename)
{
}


string FileLoader_aout::DetectFileType(unsigned char *buf, size_t buflen, float& matchness) const
{
	matchness = 0.9;

	if (buf[0]==0x00 && buf[1]==0x8b && buf[2]==0x01 && buf[3]==0x07) {
		/*  MIPS a.out  */
		return "a.out_MIPS";
	}
	if (buf[0]==0x00 && buf[1]==0x87 && buf[2]==0x01 && buf[3]==0x08) {
		/*  M68K a.out  */
		// AOUT_FLAG_VADDR_ZERO_HACK for OpenBSD/mac68k
		return "a.out_M68K_vaddr0";
	}
// Luna88k a.out for OpenBSD is _almost_ like for MVME88K... just a difference
// of entry point (?)
//	if (buf[0]==0x00 && buf[1]==0x99 && buf[2]==0x01 && buf[3]==0x07) {
//		/*  OpenBSD/luna88k a.out  */
//		return "a.out_M88K_fromBeginningEntryAfterHeader";
//	}
	if (buf[0]==0x00 && buf[1]==0x99 && buf[2]==0x01 && buf[3]==0x0b) {
		/*  OpenBSD/M88K a.out  */
		return "a.out_M88K_fromBeginning";
	}
	if (buf[0]==0x00 && buf[1]==0x8f && buf[2]==0x01 && buf[3]==0x0b) {
		/*  ARM a.out  */
		return "a.out_ARM_fromBeginning";
	}
	if (buf[0]==0x00 && buf[1]==0x86 && buf[2]==0x01 && buf[3]==0x0b) {
		/*  i386 a.out (old OpenBSD and NetBSD etc)  */
		return "a.out_i386_fromBeginning";
	}
	if (buf[0]==0x01 && buf[1]==0x03 && buf[2]==0x01 && buf[3]==0x07) {
		/*  SPARC a.out (old 32-bit NetBSD etc)  */
		return "a.out_SPARC_noSizes";
	}
	if (buf[0]==0x00 && buf[2]==0x00 && buf[8]==0x7a && buf[9]==0x75) {
		/*  DEC OSF1 on MIPS:  */
		return "a.out_MIPS_osf1";
	}

	matchness = 0.0;
	return "";
}


static uint32_t unencode32(uint8_t *p, Endianness endianness)
{
	uint32_t res;
	
	if (endianness == BigEndian)
		res = p[3] + (p[2] << 8) + (p[1] << 16) + (p[0] << 24);
	else
		res = p[0] + (p[1] << 8) + (p[2] << 16) + (p[3] << 24);

	return res;
}


bool FileLoader_aout::LoadIntoComponent(refcount_ptr<Component> component, ostream& messages) const
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

	unsigned char buf[65536];

	memset(buf, 0, sizeof(buf));
	file.seekg(0, std::ios_base::end);
	uint64_t totalSize = file.tellg();
	file.seekg(0, std::ios_base::beg);
	file.read((char *)buf, totalSize < sizeof(buf)? totalSize : sizeof(buf));
	size_t amountRead = file.gcount();

	float matchness;
	string format = DetectFileType(buf, amountRead, matchness);

	if (format == "") {
		messages << "Unknown a.out format.\n";
		return false;
	}

	file.seekg(0, std::ios_base::beg);

	Endianness endianness = BigEndian;
	StateVariable* var = component->GetVariable("bigendian");
	if (var == NULL) {
		messages << "Target does not have the 'bigendian' variable,"
		    " which is needed to\n"
		    "load a.out files.\n";
		return false;
	}

	endianness = var->ToInteger() == 0? LittleEndian : BigEndian;

	struct exec aout_header;
	uint32_t entry, datasize, textsize;
	int32_t symbsize = 0;
	uint32_t vaddr, total_len;

	if (format.substr(format.length()-5, 5) == "_osf1") {
		file.read((char *)buf, 32);
		if (file.gcount() != 32) {
			messages << "The file is too small to be an OSF1 a.out.\n";
			return false;
		}

		vaddr = unencode32(buf + 16, endianness);
		entry = unencode32(buf + 20, endianness);

		symbsize = 0;
		file.seekg(0, std::ios_base::end);
		/*  This is of course wrong, but should work anyway:  */
		textsize = (uint32_t) file.tellg() - 512;
		datasize = 0;
		file.seekg(512, std::ios_base::beg);
	} else if (format.substr(format.length()-8, 8) == "_noSizes") {
		file.seekg(0, std::ios_base::end);
		textsize = (uint32_t) file.tellg() - 32;
		datasize = 0;
		vaddr = entry = 0;
		file.seekg(32, std::ios_base::beg);
	} else {
		file.read((char *)&aout_header, sizeof(aout_header));
		if (file.gcount() != sizeof(aout_header)) {
			messages << "The file is too small to be an a.out.\n";
			return false;
		}

		entry = unencode32((unsigned char*)&aout_header.a_entry, endianness);

		vaddr = entry;

		if (format.substr(format.length()-7, 7) == "_vaddr0")
			vaddr = 0;

		textsize = unencode32((unsigned char*)&aout_header.a_text, endianness);
		datasize = unencode32((unsigned char*)&aout_header.a_data, endianness);
		symbsize = unencode32((unsigned char*)&aout_header.a_syms, endianness);
	}

	if (format.substr(format.length()-14, 14) == "_fromBeginning") {
		file.seekg(0, std::ios_base::beg);
		vaddr &= ~0xfff;
	}

	messages.flags(std::ios::hex);
	messages << "a.out: entry point 0x";
	messages << setw(8) << setfill('0') << (uint32_t) entry << "\n";

	messages.flags(std::ios::dec);
	messages << "text + data = " << textsize << " + " << datasize << " bytes\n";

	// Load text and data:
	total_len = textsize + datasize;
	while (total_len != 0) {
		int len = total_len > sizeof(buf) ? sizeof(buf) : total_len;
		file.read((char *)buf, len);
		len = file.gcount();
		if (len < 1)
			break;

		// Write to the bus, one byte at a time.
		for (int k=0; k<len; ++k) {
			bus->AddressSelect(vaddr);
			if (!bus->WriteData(buf[k])) {
				messages.flags(std::ios::hex);
				messages << "Failed to write data to virtual "
				    "address 0x" << vaddr << "\n";
				return false;
			}
			
			++ vaddr;
		}

		total_len -= len;
	}

	if (total_len != 0) {
		messages << "Failed to read the entire file.\n";
		return false;
	}

	SymbolRegistry* symbolRegistry = NULL;
	CPUComponent* cpu = component->AsCPUComponent();
	if (cpu != NULL)
		symbolRegistry = &cpu->GetSymbolRegistry();

	// Symbols:
	if (symbolRegistry != NULL && symbsize > 0) {
		messages.flags(std::ios::dec);
		messages << "symbols: " << symbsize << " bytes at 0x";
		messages.flags(std::ios::hex);
		messages << file.tellg() << "\n";

		vector<char> symbolData;
		symbolData.resize(symbsize);
		file.read(&symbolData[0], symbsize);
		if (file.gcount() != symbsize) {
			messages << "Failed to read all symbols.\n";
			return false;
		}

		off_t oldpos = file.tellg();
		file.seekg(0, std::ios_base::end);
		int strings_len = file.tellg() - oldpos;
		file.seekg(oldpos, std::ios_base::beg);

		messages.flags(std::ios::dec);
		messages << "strings: " << strings_len << " bytes at 0x";
		messages.flags(std::ios::hex);
		messages << file.tellg() << "\n";

		vector<char> symbolStrings;
		// Note: len + 1 for a nul terminator, for safety.
		symbolStrings.resize(strings_len + 1);
		file.read(&symbolStrings[0], strings_len);
		if (file.gcount() != strings_len) {
			messages << "Failed to read all strings.\n";
			return false;
		}

		assert(sizeof(struct aout_symbol) == 12);

		int nsymbols = 0;

		struct aout_symbol* aout_symbol_ptr = (struct aout_symbol *) &symbolData[0];
		int n_symbols = symbsize / sizeof(struct aout_symbol);
		for (int i = 0; i < n_symbols; i++) {
			uint32_t index = unencode32((unsigned char*)&aout_symbol_ptr[i].strindex, endianness);
			uint32_t type  = unencode32((unsigned char*)&aout_symbol_ptr[i].type, endianness);
			uint32_t addr  = unencode32((unsigned char*)&aout_symbol_ptr[i].addr, endianness);

			// TODO: These bits probably mean different things for
			// different a.out formats. For OpenBSD/m88k at least,
			// this bit (0x01000000) seems to mean "a normal symbol".
			if (!(type & 0x01000000))
				continue;

			// ... and the rectangle drawing demo says
			// "_my_memset" at addr 1020, type 5020000
			// "rectangles_m88k_O2.o" at addr 1020, type 1f000000
			if ((type & 0x1f000000) == 0x1f000000)
				continue;

			if (index >= (uint32_t)strings_len) {
				messages << "symbol " << i << " has invalid string index\n";
				continue;
			}

			string symbol = ((char*) &symbolStrings[0]) + index;
			if (symbol == "")
				continue;

			// messages << "\"" << symbol << "\" at addr " << addr
			//   << ", type " << type << "\n";

			// Add this symbol to the symbol registry:
			symbolRegistry->AddSymbol(symbol, addr);
			++ nsymbols;
		}

		messages.flags(std::ios::dec);
		messages << nsymbols << " symbols read\n";
	}

	// Set the CPU's entry point.
	stringstream ss;
	ss << (int64_t)(int32_t)entry;
	component->SetVariableValue("pc", ss.str());

	return true;
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

#include "ComponentFactory.h"

static void Test_FileLoader_aout_Constructor()
{
	FileLoader_aout aoutLoader("test/FileLoader_A.OUT_M88K");
}

UNITTESTS(FileLoader_aout)
{
	UNITTEST(Test_FileLoader_aout_Constructor);

	// TODO
}

#endif
