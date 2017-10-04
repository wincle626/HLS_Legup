#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <math.h>
using namespace std;

typedef struct {
	int lo;
	int hi;
	int count;
	string label;
} RANGE;

int main (int argc, char** argv) {
	if (argc != 5) { printf("Incorrect usage, expected: %s  <bench.flist> <bench.flist_lab> <bench.pc_trace> <bench.elf>.\n", argv[0]); exit(1); }
	
	vector<RANGE> ranges;
	RANGE tRange;
	tRange.count = 0;	// initialize all counters
	
	// read in flist file (only lo addresses are stored)
	FILE *flist = fopen(argv[1], "r");
	if (flist == NULL) { printf("File %s not found!\n", argv[1]); exit(1); }
	int line;
	while (fscanf(flist, "%x", &line) != EOF) {
		//printf("line = %x\n", line);
		tRange.lo = line;
		ranges.push_back(tRange);
	}
	fclose(flist);
	
	// infer hi addresses
	for (int i=0; i<ranges.size()-1; i++) {
		ranges[i].hi = ranges[i+1].lo-4;
	}

	// get end address from dump
	system(string(string("mipsel-unknown-elf-objdump -h ") + string(argv[4]) + string(" | grep '\\.text' | sed -r 's|.*text[ ]*([^ ]*).*|\\1|' > hi_addr.temp")).c_str());
	flist = fopen("hi_addr.temp", "r");
	fscanf(flist, "%x\n", &line);
	ranges[ranges.size()-1].hi = (line + ranges[0].lo);// add to start address b/c its just a size
	fclose(flist);
	
	// read in flist_lab file
	flist = fopen(argv[2], "r");
	char tLabel[100];
	int j = 0;
	while (fscanf(flist, "%s", &tLabel[0]) != EOF) {
		ranges[j++].label = string(tLabel);
	}
	fclose(flist);
	
	// test output
	//for (int i=0; i<ranges.size(); i++) { printf("ranges[%d] (%30s): %x --> %x\n", i, ranges[i].label.c_str(), ranges[i].lo, ranges[i].hi); }
	
	// read through pc_trace file, increment counters
	FILE *trace = fopen(argv[3], "r");
	while (fscanf(trace, "%x", &line) != EOF) {
		for (int i=0; i<ranges.size(); i++) {
			if (line >= ranges[i].lo && line <= ranges[i].hi) {
				ranges[i].count++;
				break;
			}
		}
	}
	
	// return results
	for (int i=0; i<ranges.size(); i++) {
		printf("%s\t%d\n", ranges[i].label.c_str(), ranges[i].count);
	}
	return 0;
}
