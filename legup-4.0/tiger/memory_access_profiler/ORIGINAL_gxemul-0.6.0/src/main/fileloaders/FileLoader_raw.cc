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
#include "FileLoader_raw.h"


// This is basically strtoull(), but it needs to be explicitly implemented
// since some systems lack it. (Also, compiling with GNU C++ in ANSI mode
// does not work with strtoull.)
static uint64_t parse_number(const char* str, bool& error)
{
	int base = 10;
	uint64_t result = 0;
	bool negative = false;

	error = false;

	if (str == NULL)
		return 0;

	while (*str == ' ')
		++str;

	if (*str == '-') {
		negative = true;
		++str;
	}

	while ((*str == 'x' || *str == 'X') || (*str >= '0' && *str <= '9')
	    || (*str >= 'a' && *str <= 'f') || (*str >= 'A' && *str <= 'F')) {
		if (*str == 'x' || *str == 'X') {
			base = 16;
		} else {
			int n = *str - '0';
			if (*str >= 'a' && *str <= 'f')
				n = *str - 'a' + 10;
			if (*str >= 'A' && *str <= 'F')
				n = *str - 'A' + 10;
			result = result * base + n;
		}
		++str;
	}

	if (*str)
		error = true;

	if (negative)
		return -result;
	else
		return result;
}


FileLoader_raw::FileLoader_raw(const string& filename)
	: FileLoaderImpl(filename)
{
}


static vector<string> SplitStringIntoVector(const string &str, const char splitter)
{
	// This is slow and hackish, but works.
	vector<string> strings;
	string word;

	for (size_t i=0, n=str.length(); i<n; i++) {
		char ch = str[i];
		if (ch == splitter) {
			strings.push_back(word);
			word = "";
		} else {
			word += ch;
		}
	}

	strings.push_back(word);

	return strings;
}


string FileLoader_raw::DetectFileType(unsigned char *buf, size_t buflen, float& matchness) const
{
	matchness = 0.0;

	vector<string> parts = SplitStringIntoVector(Filename(), ':');

	// Possible variants:
	//
	// filename			<-- NOT a raw file
	// raw:vaddr:filename
	// raw:vaddr:skiplen:filename
	// raw:vaddr:skiplen:initialpc:filename  e.g. raw:0xbfc00000:0x100:0xbfc00884:rom.bin
	if (parts.size() < 3)
		return "";

	if (parts[0] != "raw")
		return "";

	matchness = 1.0;
	return "raw";
}


bool FileLoader_raw::LoadIntoComponent(refcount_ptr<Component> component, ostream& messages) const
{
	AddressDataBus* bus = component->AsAddressDataBus();
	if (bus == NULL) {
		messages << "Target is not an AddressDataBus.\n";
		return false;
	}

	// Possible variants:
	//
	// raw:vaddr:filename
	// raw:vaddr:skiplen:filename
	// raw:vaddr:skiplen:initialpc:filename  e.g. 0xbfc00000:0x100:0xbfc00884:rom.bin
	vector<string> parts = SplitStringIntoVector(Filename(), ':');
	if (parts.size() < 3 || parts.size() > 5) {
		messages << "Syntax is raw:vaddr:[skiplen:[initialpc:]]filename.\n";
		return false;
	}

	string strvaddr = parts[1];
	string strskiplen = (parts.size() >= 4)? parts[2] : "";
	string strinitialpc = (parts.size() >= 5)? parts[3] : "";
	string fname = parts[parts.size() - 1];

	bool error;
	uint64_t vaddr = parse_number(strvaddr.c_str(), error);
	if (error) {
		messages << "could not parse vaddr.\n";
		return false;
	}

	uint64_t skiplen = 0;
	if (strskiplen != "") {
		skiplen = parse_number(strskiplen.c_str(), error);
		if (error) {
			messages << "could not parse skiplen\n";
			return false;
		}
	}

	uint64_t initialpc = vaddr;
	if (strinitialpc != "") {
		initialpc = parse_number(strinitialpc.c_str(), error);
		if (error) {
			messages << "could not parse initialpc\n";
			return false;
		}
	}

	ifstream file(fname.c_str());
	if (!file.is_open()) {
		messages << "Unable to read file.\n";
		return false;
	}

	file.seekg(0, std::ios_base::end);
	off_t totalSize = file.tellg();
	file.seekg(skiplen, std::ios_base::beg);
	totalSize -= skiplen;

	// Read everything:
	vector<char> data;
	data.resize(totalSize);
	file.read(&data[0], totalSize);
	if (file.gcount() != totalSize) {
		messages << "failed to read the whole file\n";
		return false;
	}

	messages.flags(std::ios::hex);
	messages << "Raw file: entry point 0x" << initialpc << "\n";
	messages << "loadable chunk";
	if (skiplen != 0) {
		messages.flags(std::ios::dec);
		messages << " at offset " << skiplen;
	}

	messages.flags(std::ios::hex);
	messages << ": vaddr 0x" << vaddr;

	messages.flags(std::ios::dec);
	messages << ", " << totalSize << " bytes\n";

	// Write to the bus, one byte at a time.
	for (size_t k=0; k<(size_t)totalSize; ++k) {
		bus->AddressSelect(vaddr);
		if (!bus->WriteData(data[k])) {
			messages.flags(std::ios::hex);
			messages << "Failed to write data to "
			    "virtual address 0x" << vaddr << "\n";
			return false;
		}

		++ vaddr;
	}

	// Set the CPU's entry point.
	stringstream ss;
	ss << initialpc;
	component->SetVariableValue("pc", ss.str());

	return true;
}


/*****************************************************************************/


#ifdef WITHUNITTESTS

#include "ComponentFactory.h"

static void Test_FileLoader_raw_Constructor()
{
	// TODO: haha, better test.
	FileLoader_raw rawLoader("test/FileLoader_A.OUT_M88K");
}

UNITTESTS(FileLoader_raw)
{
	UNITTEST(Test_FileLoader_raw_Constructor);

	// TODO
}

#endif
