// C program to parse list.txt for functional ranges

/* TO DO:
 *	- fix error where the function is being split just before a jr $ra
 */

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <math.h>
using namespace std;

typedef struct {
	string label;
	string start;
	string end;
} FUNCTION;

FUNCTION parse_line(string line);
string offsetHex(string hex, int offset);
long int hex2int (string hex);
string int2hex (long int val);
int _h2i(char c);
char _i2h(int i);
string ucase(string str);

int main(int argc, char** argv) {
	vector<FUNCTION> functions;
	ifstream fDump;
	string line = "", last_line;

	if (argc != 2) { printf("Incorrect usage, expected: %s  <file.elf>.\n", argv[0]); exit(1); }

	// disassemble elf file
	system(("mipsel-unknown-elf-objdump -d " + string(argv[1]) + " > " + string(argv[1]) + ".dump").c_str());
	fDump.open((string(argv[1]) + ".dump").c_str(), ios::in|ios::binary);
	
	// parse file.elf.dump
	if (fDump.is_open()) {
		while(!fDump.eof() && line != "Disassembly of section .text:") getline(fDump,line);		// skip past beginning
		getline(fDump,line);
		getline(fDump,line);
		while (!fDump.eof()) {
			if (line.size() > 0) {
				//printf("line = %s\n", line.substr(line.size()-3,2).c_str());
				if (line.substr(line.size()-2,2) == ">:") { //line[0] == '0') {
					functions.push_back(parse_line(line));	// if the line starts with '0', it is the declaration of a function so add a RANGE for it
				}
				last_line = line;
			}
			getline (fDump, line);
		}
	} else {
		printf("File not found! (%s) Quitting.\n", argv[1]);
		exit(1);
	}
	fDump.close();
	
	// now go through the vector of functions (names & start addresses) and correlate end addresses (prev_section.end = cur.start - 4)
	for (int i=1; i<functions.size(); i++) {
		functions[i-1].end = offsetHex(functions[i].start, -4);
	}

	// avoid seg faults by ensuring we found something before continuing
	if (functions.size() == 0) { printf("No functions found! Quitting.\n"); exit(1); }
	
	// remove file extension
	string basename = string(argv[1]);
	basename = basename.substr(0,basename.find_last_of('.'));
	FILE *flist = fopen((basename+".flist").c_str(), "w");
	FILE *flist_labels = fopen((basename+".flist_lab").c_str(), "w");
	FILE *range = fopen((basename+".range").c_str(), "w");
	
	// set last function's end based off end of file
	char buf[10];
	int e;
	system(string(string("mipsel-unknown-elf-objdump ") + basename + string(".elf -h | sed -r 's|^[ ]*[0-9]* \\.text[ ]*([0-9a-fA-F]*).*|\\1|p' -n > last_addr")).c_str());
	FILE *last_addr = fopen("last_addr", "r");
	fscanf(last_addr, "%x", &e);
	fclose(last_addr);
	system("rm last_addr");
	sprintf(&buf[0], "%08X", e + 0x800000 - 4);
	functions.back().end = string(buf);
	
	// print results
	for (int i=0; i<functions.size(); i++) {
		fprintf(flist, "%s\n", functions[i].start.c_str());
		fprintf(flist_labels, "%s\n", functions[i].label.c_str());
		fprintf(range, "%s\n%s\n",  functions[i].start.c_str(), functions[i].end.c_str());
	}
	
	if (functions.size() == 17) {			// this is for the perfect hash code -- seg faults if 17 keys input (no idea why!!)
		int s;
		sscanf(functions.back().end.c_str(),"%x", &s);
		s = s + 1;
		fprintf(flist, "%08X\n", s);
		fprintf(flist_labels, ".\n");
		fprintf(range, "%08X\n%08X\n",  s, s);
	}
	
	/*for (int i=0; i<16-functions.size(); i++) {	// have to have at least 8 functions or hash optimizes away the val ^= (val >> 8)
		string t = functions[functions.size()-1].start;
		t = t.substr(0,t.size()-2)+"1"+_i2h(i);
		fprintf(flist, "%s\n", t.c_str());
		fprintf(flist_labels, ".\n", t.c_str());
	}	*/
	
	fclose(flist);
	fclose(flist_labels);
	fclose(range);

	return 0;
}

FUNCTION parse_line(string line) {
	FUNCTION function;
	
	int x1 = line.find_first_of('<')+1;
	int x2 = line.find_first_of('>');
	function.label = line.substr(x1, x2-x1);
	function.start = ucase(line.substr(0,line.find_first_of(' ')));
	function.end = "";

	return function;
}

string offsetHex(string hex, int offset) {
	return int2hex(hex2int(hex) + offset);
}
	

long int hex2int (string hex) {
	long int ret = 0;

	while (hex.size() > 0) {
		ret = ret*16 + _h2i(hex[0]);
		hex = hex.substr(1);
	}
		
	return ret;
}

string int2hex (long int val) {
	string hex = "";
	
	while (val > 15) {
		hex = _i2h(val % 16) + hex;
		val = floor((double)val/16);
	}
	
	hex = _i2h(val) + hex;
	
	while (hex.size() < 8) hex = "0" + hex;
		
	return hex;
}	
	
int _h2i(char h) {
	switch (h) {
		case '0': return 0;
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		case 'A':
		case 'a': return 10;
		case 'B':
		case 'b': return 11;
		case 'C':
		case 'c': return 12;
		case 'D':
		case 'd': return 13;
		case 'E':
		case 'e': return 14;
		case 'F':
		case 'f': return 15;

		default:
			cout << "h2i value invalid....wtf? " << h << "\n";
			exit(1);
	}
}

char _i2h(int i) {
	switch (i) {
		case 0: return '0';
		case 1: return '1';
		case 2: return '2';
		case 3: return '3';
		case 4: return '4';
		case 5: return '5';
		case 6: return '6';
		case 7: return '7';
		case 8: return '8';
		case 9: return '9';
		case 10: return 'A';
		case 11: return 'B';
		case 12: return 'C';
		case 13: return 'D';
		case 14: return 'E';
		case 15: return 'F';

		default:
			cout << "i2h value invalid....wtf? " << i << "\n";
			exit(1);
	}
}

string ucase(string str) {
	for (int i=0; i<str.size(); i++) str[i] = toupper(str[i]);
	return str;
}
