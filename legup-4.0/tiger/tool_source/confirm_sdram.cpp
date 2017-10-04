#include <string>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#define min(x,y) (x<y ? x : y)
using namespace std;

int h2i(string h);
int _h2i(char h);


int main(int argc, char** argv) {
	if (argc != 3) { printf("Incorrect usage: Expected %s <.elf> <.sdram>\n", argv[0]); exit(1); }
	
	ifstream dump;
	string line, data;
	int address;
	vector<string> sdram;
	bool correct = true;
	
	// load sdram.dat file into vector
	dump.open(argv[2], ios::in|ios::binary);
	getline(dump, line);
	while(!dump.eof()) {
		sdram.push_back(line);
		getline(dump, line);
	}
	dump.close();
	
	// disassemble elf headers
	system(("objdump -s " + string(argv[1]) + " > " + string(argv[1]) + ".dump").c_str());
	dump.open((string(argv[1]) + ".dump").c_str(), ios::in|ios::binary);
	
	// go through each line, grab address, see if first few bytes are correct in sdram
	getline(dump, line);
	while (!dump.eof()) {
		if (line[0] == ' ' && line.find_first_of(" ", 1) == 7) {
			sscanf(line.substr(2, 5).c_str(), "%x", &address);	// don't want leading 8
			data = line.substr(8, 8);
			string equiv = data.substr(2,2) + data.substr(0,2) + data.substr(6,2) + data.substr(4,2);
			
			int data_size = min(data.find_first_of(" "), data.size());
			
			int sdram_offset = address % 16;
			int sdram_pos = (address-sdram_offset)/2;
			string sdram_combined = "";
			for (int i=0; i<10; i++) sdram_combined += sdram[sdram_pos+i].substr(2,2) + sdram[sdram_pos+i].substr(0,2);
			sdram_combined = sdram_combined.substr(sdram_offset*2, data_size);
			
			if (data.substr(0,data_size) != sdram_combined) {
				printf("sdram.dat file corrupted! Trace:\n");
				printf("\tdump line: %s\n", line.c_str());
				printf("\tdump equiv: %s\n", equiv.c_str());
				printf("\tsdram:      %s (line %d)\n", sdram_combined.c_str(), sdram_pos);
				correct = false;
			}		
		}
		
		getline(dump, line);
	}
	dump.close();
	
	if (correct) {
		printf("sdram.dat file confirmed!\n");
	}
	
	return 0;
}
