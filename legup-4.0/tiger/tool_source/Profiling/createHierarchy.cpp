#include <fstream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <set>

using namespace std;

#include "numFunctions.h"

map< string, set<string> > functions;	// map< function_name, set<jal> >

void output_hierarchy (string cur_function, string space) {
	set<string>::iterator si;
	
	printf("%s%s\n", space.c_str(), cur_function.c_str());
	for (si=functions[cur_function].begin(); si!=functions[cur_function].end(); si++) {
		output_hierarchy(*si, (space + ". "));
	}
}

int main(int argc, char** argv) {
	if (argc != 2) { printf("Incorrect usage.  Expecting: %s <benchmark.src>\n", argv[0]); exit(1); }
	
	ifstream fDump;
	string cur_func_name;
	char cur_str[50];
	string line;
	int x1, x2;

	
	// parse benchmark.src
	fDump.open(argv[1], ios::in|ios::binary);
	if (fDump.is_open()) {
		while(!fDump.eof() && line != "Disassembly of section .text:") getline(fDump,line);		// skip past beginning
		getline(fDump,line);
		getline(fDump,line);
		while (!fDump.eof()) {
			x1 = line.find_first_of("<")+1;
			x2 = line.find_first_of(">");
			cur_func_name = line.substr(x1,x2-x1);
			//printf("cur_func_name = %s\n", cur_func_name.c_str());
			functions[cur_func_name].clear();
			
			while (!fDump.eof() && line.size() > 0) {		
				if (line.find("jal") != string::npos) {
					x1 = line.find_first_of("<")+1;
					x2 = line.find_first_of(">");
					//printf ("inserting %s\n", line.substr(x1, x2-x1).c_str());
					functions[cur_func_name].insert(line.substr(x1, x2-x1));
				}
				getline (fDump, line);
			}
			getline (fDump, line);
		}
	} else {
		printf("File not found! (%s) Quitting.\n", argv[1]);
		exit(1);
	}
	fDump.close();
	
	// output hierarchy
	output_hierarchy ("main", "");	
	
	return 0;
}
