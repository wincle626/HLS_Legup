#include <fstream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

int NUM_FUNCTIONS;
void output_prof_data(FILE *sdram, char* hash_file, int data_size);

int main(int argc, char** argv) {
	if (argc != 3 && argc != 6) { printf("Incorrect usage.  Expecting <elf_file> <sdram_file> [-p <prof_file> <NUM_FUNCTIONS>].\n"); exit(1); }

	ifstream fHDump;
	string line, name, data = "";
	unsigned long start_addr = 0;
	unsigned long text_end_addr = 0;
	bool isText = true;
	unsigned end_addr = 0;
	bool done_first_section = false;

	// disassemble elf headers
	system(("objdump -h " + string(argv[1]) + " > " + string(argv[1]) + ".header.dump").c_str());
	fHDump.open((string(argv[1]) + ".header.dump").c_str(), ios::in|ios::binary);

	if (fHDump.is_open()) {
		unsigned vma;
		unsigned size;
		// find first section
		while (!fHDump.eof() && line.substr(0,8) != "Idx Name") {
			getline(fHDump, line);
		}
		while (!fHDump.eof()) {
			getline(fHDump, line);
			sscanf(line.c_str(), "%*d %*s %x %x", &size, &vma);
			if (vma != 0) {
				end_addr = size + vma;
			}
			// ignore attributes
			getline(fHDump, line);
		}
	}
	fHDump.close();

	// disassemble elf
	ifstream fDump;
	system(("objdump -s " + string(argv[1]) + " > " + string(argv[1]) + ".dump").c_str());
	fDump.open((string(argv[1]) + ".dump").c_str(), ios::in|ios::binary);

	if (fDump.is_open() && !fDump.fail()) {
		// find first section
		while (!fDump.eof() && line.substr(0,19) != "Contents of section") getline(fDump, line);
			
		// NOTE -- THIS SKIPS BSS!!
		// store section data
		while (!fDump.eof()) {
			if (line.substr(0,19) == "Contents of section") {
				// only store first section
				if (done_first_section) break;
				done_first_section = true;
				
				name = line.substr(21, line.size()-22);

				// get next line so i know the address
				getline(fDump, line);
				sscanf(line.substr(0,7).c_str(), "%lx", &start_addr);
				if (isText) {
					isText = false;
				} else if (start_addr > 0) {
					// add spacing if necessary
					unsigned space = start_addr - data.size() / 2 - 0x800000;
					//printf("  space = %u\n", space);
					while (space-- > 0) data += "00";
				}

				// if the address starts at 0, ignore
				if (start_addr > 0) {
					sscanf(line.substr(0,7).c_str(), "%lx", &text_end_addr);
					//printf("Section Name: %s, Section Start: %lx\n", name.c_str(), start_addr);
					// have to do line processing so the data isn't lost
					for (int k=8; k<44; k++) {
						if (line[k] != ' ') data += line[k];
					}
				}
			} else if (start_addr > 0) {
				sscanf(line.substr(0,7).c_str(), "%lx", &text_end_addr);
				for (int k=8; k<44; k++) {
					if (line[k] != ' ') data += line[k];
				}
			}

			if (!fDump.eof()) getline(fDump, line);
		}
	} else {
		printf("File %s failed to load! Quitting.\n", argv[1]);
		exit(1);
	}

	fDump.close();

	// pad empty sections
	unsigned space = end_addr - data.size() / 2 - 0x800000;
	//while (space-- > 0) data += "00";

	// pad with 0's so that size is a multiple of 4
	if (data.size() % 4 == 2) data += "00";

	FILE *sdram = fopen(argv[2], "w");
	for (int j=0; j<data.size(); j+=4) {
		fprintf(sdram, "%c%c%c%c\n", data[j+2], data[j+3], data[j+0], data[j+1]);
		if (data[j] == '.' || data[j+1] == '.' || data[j+2] == '.' || data[j+3] == '.') {
			printf("failed to produce sdram.dat");
			exit(1);
		}
	}
	
	fclose(sdram);

	return 0;
}
