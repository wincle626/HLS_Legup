#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <math.h>
using namespace std;

int h2i(char h) {
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
		default : cout << "h2i value invalid....wtf? " << h << "\n";	exit(1);
	}
}

#define PC_LEN 7
int parse_pc(string line) {
	char char_pc[PC_LEN];
	int ret_pc = 0;

	sscanf(line.c_str(), "# pc = %s", char_pc);
	for (int k=0; k<PC_LEN; k++) {
		ret_pc = 16*ret_pc + h2i(char_pc[k]);
	}

	return ret_pc;
}

int main(int argc, char** argv) {
	/* ALGORITHM
	 * - read in list.txt file
	 * - parse and put to vector/array
	 * - read in pc file line by line
	 *		- parse line to find PC value
	 *		- lookup PC value from list.txt vector
	 *		- append line at end and put to new file
	 */

	ifstream fList, fPc;
	vector<string> pc_lookup;
	string line;
	int pc;


	if (argc != 3) { cout << "Incorrect usage: Expected <list file>, <pc file>. Quitting.\n"; exit(1); }

	// parse list.txt
	fList.open(argv[1], ios::in);
	if (fList.is_open()) {
		while(line != "00000000 <.text>:") getline(fList,line);		// skip past beginning

		getline(fList,line);
		while (!fList.eof()) {
			pc_lookup.push_back(line);
			getline (fList, line);
		}
	} else {
		printf("File not found! (%s) Quitting.\n", argv[1]);
		exit(1);
	}
	fList.close();

	fPc.open(argv[2], ios::in);
	if (fPc.is_open()) {
		while (line != "# pc = 0000000") getline(fPc, line);		// skip modelsim compilation output

		while (!fPc.eof()) {
			if (line.substr(0,7) == "# pc = ") {
				pc = parse_pc(line);
				cout << line << "\t\t\t\t" << pc_lookup[(pc/4)] << "\n";
			} else {
				cout << line << "\n";
			}
			getline(fPc, line);
		}
	} else {
		printf("File not found! (%s) Quitting.\n", argv[1]);
		exit(1);
	}
	fPc.close();

	return 0;
}