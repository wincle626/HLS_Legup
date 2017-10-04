#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <fstream>
#include <stdlib.h>
using namespace std;



// C program to convert elf file to mif file
int main(int argc, char** argv) {
	// ensure correct usage
	if (argc < 3 || (argc == 4 && string(argv[3]) != "--output-depth")) {
		printf("Correct usage: %s <ELF File> <MIF file> [--output-depth]\n", argv[0]);
		exit(1);
	}
	
	// variables
	FILE * mif = fopen(argv[2], "w");
	//mif = stdout;
	
	// make sure files exist
	if (mif == NULL) {
		fprintf(stderr, "Error: Can't open out file (%s)!\n", argv[2]);
		exit(1);
	}
	
	// disassemble elf
	ifstream fDump;
	system(("mipsel-unknown-elf-objdump -d " + string(argv[1]) + " > " + string(argv[1]) + ".src").c_str());
	fDump.open((string(argv[1]) + ".src").c_str(), ios::in|ios::binary);
	if (!fDump.is_open()) {
		fprintf(stderr, "Error: Can't open input file (%s)!\n", argv[1]);
		exit(1);
	}
	
	// parse elf dump file
	string line;
	int instr;
	vector<int> instrs;
	
	// find first section
	while (!fDump.eof() && line.substr(0,29) != "Disassembly of section .text:") getline(fDump, line);
	// skip next two lines (blank then main)
	getline(fDump, line); getline(fDump, line); getline(fDump, line);
	
	// store section data
	while (!fDump.eof() && line != "") {
		sscanf(line.c_str(), "%*x: %x", &instr);
		instrs.push_back(instr);
		getline(fDump, line);
	}
	
	// print MIF header
	fprintf(mif, "WIDTH=32;\n");
	fprintf(mif, "DEPTH=%d;\n", instrs.size()-1);
	fprintf(mif, "ADDRESS_RADIX=HEX;\n");
	fprintf(mif, "DATA_RADIX=HEX;\n");
	fprintf(mif, "CONTENT BEGIN\n");
	
	// print MIF data
	for (int i=0; i<instrs.size()-1; i++) { // size-1 b/c we push_back the EOF line
		fprintf(mif, "%08X : %08X;\n", i, instrs[i]);
	}
	
	// print MIF footer
	fprintf(mif, "END;\n");
	
	fDump.close();
	fclose(mif);
	
	// if --output-depth specified, output the depth
	if (argc == 4) printf("%d\n", instrs.size()-1);
	return 0;
}
	