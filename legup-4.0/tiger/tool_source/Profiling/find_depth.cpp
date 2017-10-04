#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
using namespace std;

int main(int argc, char** argv) {
	string line = "";
	ifstream src;
	int max_depth, cur_depth;
	
	if (argc != 2) { printf("wrong number of arguments!\n"); exit(1); }
	
	max_depth = 0;
	src.open(argv[1], ios::in);
	if (src.is_open()) {
		while(!src.eof() && line[0] != '<') getline(src,line);		// find main function
		getline(src,line);
		while (!src.eof() && line[0] == ' ') {						// go until end
			cur_depth = line.find('<') / 2;
			if (cur_depth > max_depth) max_depth = cur_depth;
			getline(src,line);
		}
	}
	
	printf("Max depth for %s = %d\n", argv[1], max_depth);
	
	return 0;
}