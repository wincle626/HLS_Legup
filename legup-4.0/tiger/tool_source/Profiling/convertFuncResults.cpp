#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <math.h>
using namespace std;

int NUM_FUNCTIONS;

typedef struct {	// ADDRESS HASH
	long int V1;
	int A1, A2;
	int B1, B2;
	int *tab;
	
	void init(char *fname) {
		tab = (int*)malloc(sizeof(int)*(NUM_FUNCTIONS+1));
		FILE *fHash = fopen(fname, "r");
		if (fHash == NULL) { printf("Hash file (%s) not found!\n", fname); exit(1); }
		
		// Read/parse data from .prof file as 8+N bytes of information (TAB,V1,A1,A2,B1,B2)
		fscanf(fHash, "tab[] = {");
		for (int i=0; i<NUM_FUNCTIONS; i++) fscanf(fHash, "%d,", &tab[i]);
		tab[NUM_FUNCTIONS] = '\0';	// so we can use it as a string to output
		fscanf(fHash, "}\nV1 = 0x%lx\n", &V1);
		fscanf(fHash, "A1 = %d\n", &A1);
		fscanf(fHash, "A2 = %d\n", &A2);
		fscanf(fHash, "B1 = %d\n", &B1);
		fscanf(fHash, "B2 = 0x%x\n", &B2);
		
		if (1) {
			printf("tab = { %d", tab[0]);
			for (int i=1; i<NUM_FUNCTIONS; i++) printf(", %d", tab[i]);
			printf(" }\n");
			printf("V1 = 0x%x, A1 = %d, A2 = %d, B1 = %d, B2 = 0x%x\n", V1, A1, A2, B1, B2);
		}
		
		fclose(fHash);
	}
	
	#define dprintf(...) // printf
	int doHash (unsigned int val) {
		unsigned int a, b, rsl;
		
		val &= 0xFFFFFF;	// only take bottom 26 bits b/c thats all PC is
		dprintf("------\nval = %x\n", val);
		val += V1;				dprintf("val = %x\n", val);
		val += (val << 8);		dprintf("val = %x\n", val);
		val ^= (val >> 4);		dprintf("val = %x\n", val);
		b = (val >> B1) & B2;	dprintf("b = %x\n", b);
		
		// fix C++ issue where shifting left 32 bits does nothing
		if (A1 != 32) 	a = (val + (val << A1)) >> A2;
		else		  	a = val >> A2;
		dprintf("a = %x ---- %x\n", a, (val + (val << A1)));
		dprintf("tab[b] = %x\n", tab[b]);
		rsl = (a^tab[b]);		dprintf("rsl = %x\n", rsl);
		rsl &= NUM_FUNCTIONS-1;
		
		return rsl;
	}
	
} ADDRESS_HASH;


int main (int argc, char** argv) {
	fstream fResults;
	string line;
	
	int func_num, func_count;
	ADDRESS_HASH hash;

	if (argc != 7) { printf("Incorrect usage, expected: %s  <transcript> <benchmark.flist> <benchmark.flist_lab> <benchmark.hash> <prof_type (v/s)> <NUM_FUNCTIONS>.\n", argv[0]); exit(1); }

	sscanf(argv[6], "%d", &NUM_FUNCTIONS);	// put num_functions to global variable
	fResults.open(argv[1], ios::in|ios::binary);
	int *funcs = (int*)malloc(sizeof(int)*NUM_FUNCTIONS);
	hash.init(argv[4]);

	// parse prof_results
	if (fResults.is_open()) {
		while (!fResults.eof() && line.find("# Profile Results:")==string::npos) getline (fResults, line);
		
		func_num = 0;
		while (!fResults.eof() && line.find("# Done")==string::npos) {
			getline (fResults, line);
			if (line.size() > 0) {		
				sscanf(line.c_str(), "# %d %d", &func_num, &func_count);
				//int s = line.find_first_of(" ", 2);
				//sscanf(line.substr(s).c_str(), "%d", &func_count);
				funcs[func_num] = func_count;
				func_num++;
			}
		}
	} else {
		printf("File not found! (%s) Quitting.\n", argv[1]);
		exit(1);
	}
	fResults.close();

	// find largest function label
	ifstream fFList_lab;
	fFList_lab.open(argv[3], ios::in|ios::binary);	// ${BENCHMARK}.flist_lab
	string label;
	int max_label_len = 0;
	while (!fFList_lab.eof()) {
		getline(fFList_lab, label);
		if (label.size() > max_label_len) max_label_len = label.size();
	}
	fFList_lab.close();
	
	// return data -- load filelist and hash to get results
	FILE *fFList = fopen(argv[2], "r");				// ${BENCHMARK}.flist
	fFList_lab.open(argv[3], ios::in|ios::binary);	// ${BENCHMARK}.flist_lab
	int f_addr, f_num, f_cnt=0;
		
	if (fFList_lab.is_open()) {
		while (!fFList_lab.eof()) {
			fscanf(fFList, "%x\n", &f_addr);
			getline(fFList_lab, label);
			
			if (label == "") break;
			f_num = (argv[5][0]=='v') ? hash.doHash(f_addr) : f_cnt++;
			
			//printf("%20s [0x%8x][%d] ==> %d\n", label.c_str(), f_addr, f_num, storage.storage[f_num]);		
			printf("%*s [%x -> %2d] = %5d\n", max_label_len, label.c_str(), f_addr, f_num, funcs[f_num]);		
		}
	} else {
		printf("Error - %s doesn't exist!\n", argv[3]);
	}
}
