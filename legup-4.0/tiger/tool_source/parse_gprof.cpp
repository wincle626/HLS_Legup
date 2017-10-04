#include <fstream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

using namespace std;
 
int main(int argc, char** argv) {
	if (argc != 2) { printf("Incorrect usage.  Expecting <file.gprof>.\n"); exit(1); }
	
	string line;
	ifstream fGprof;
	fGprof.open(argv[1], ios::in|ios::binary);
	string name, time;
	int x1, x2;
	
	if (!fGprof.is_open()) { printf("File not found! -- %s\n", argv[1]); exit(1); }
	
	while (!fGprof.eof()) {
		getline(fGprof, line);
		if (line[0] == '[') {
			//sscanf(line.c_str(), "[%*s] %f %*s", &time);
			x2 = line.find_last_of('[')-1;
			x1 = line.substr(0,x2).find_last_of(' ')+1;
			name = line.substr(x1, x2-x1);
			
			x2 = line.find_first_of(']');
			x1 = line.find_first_not_of(' ', x2+1);
			x2 = line.find_first_of(' ', x1);
			time = line.substr(x1, x2-x1);
			
			
			printf("%s\t%s\n", time.c_str(), name.c_str());
		}
	}
	
	fGprof.close();
	
	return 0;
}
			
