#include <fstream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

using namespace std;
 
int main(int argc, char** argv) {
	if (argc != 2) { printf("Incorrect usage.  Expecting <file.trace>.\n"); exit(1); }

	ifstream fTrace;
	ofstream pcTrace, instrTrace, asmTrace;
	string line;
	
	vector<string> pc, instr, assembly;

	// Read in each line
	fTrace.open(argv[1], ios::in|ios::binary);
	if (fTrace.is_open()) {
		while (!fTrace.eof()) {
			getline(fTrace, line);
			
			// if its an instruction trace line, store it
			if (line.size() > 29 && line.substr(0,8) == "ffffffff") {
				pc.push_back(line.substr(8,8));
				instr.push_back(line.substr(18,8));
				assembly.push_back(line.substr(27));
			}
		}
	}
	fTrace.close();
	
	// remove file extension
	string basename = string(argv[1]);
	basename = basename.substr(0,basename.find_last_of('.'));
	
	// output PCs & instrs
	pcTrace.open((basename+".pc_trace").c_str(), ios::out);
	instrTrace.open((basename+".instr_trace").c_str(), ios::out);
	asmTrace.open((basename+".asm_trace").c_str(), ios::out);
	for (int i=0; i<pc.size(); i++) {
		pcTrace << pc[i] << "\n";
		instrTrace << instr[i] << "\n";
		asmTrace << assembly[i] << "\n";
	}
	pcTrace.close();
	instrTrace.close();
	asmTrace.close();
	
	return 0;
}
	