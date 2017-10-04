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
#include <algorithm>
using namespace std;

typedef struct {
	string label;
	string lo;
	string hi;
} RANGE;


typedef struct {
	string src1;
	string src2;
	string dest;
} REG;

typedef struct {
	int label;		// just 0 or 1 -- instruction string can hold the actual label
	string pc;		// sscanf(pc.c_str(), "%x", &ret.pc);	// convert pc to integer
	string op;		//		- not sure if theres a reason to need pc's value or not?
	string instr;
	REG reg;

	int ret() {	return (instr=="jr" && reg.dest=="$ra"); } //|| instr=="j" || instr=="b"); }

	string bra() {
		if (instr=="j" || instr=="b") 			 return reg.dest.substr(2);	// skip the "0x"
		else if (instr=="bnez" || instr=="beqz") return reg.src1.substr(2);	// skip the "0x"
		else									 return "-1";
	}
} LINE;

LINE parse_line(string line);
void break_returns(vector<LINE> &program);
void analyze_loops(vector<LINE> &program);
void set_labels(vector<LINE> &program, vector<string> labels);
void remove_useless_labels(vector<LINE> &program);
void display_program(vector<LINE> &program, vector<RANGE> &ranges, int out);
string trim(string s);
vector<RANGE> extract_ranges(vector<LINE> &program);
string ucase(string str);

int main(int argc, char** argv) {
	vector<LINE> program;
	vector<string> labels;
	vector<RANGE> ranges;
	string line = "";
	ifstream list, s_file;

	if (argc != 3) { printf("Incorrect number of arguments!\n"); exit(1); }

	// parse list.txt
	list.open(argv[1], ios::in);
	if (list.is_open()) {
			while(line != "00000000 <.text>:") getline(list,line);		// skip past beginning

			getline(list,line);
			while (!list.eof()) {
				program.push_back(parse_line(line));
				getline (list, line);
			}
	} else {
		printf("File not found! (%s) Quitting.\n", argv[1]);
		exit(1);
	}
	list.close();


	// parse *.s
	s_file.open(argv[2], ios::in);
	if (s_file.is_open()) {
			getline(s_file,line);
			while (line.find(".text") == string::npos) {
				getline(s_file,line);
			}

			labels.push_back("");
			labels.push_back("");
			while (!s_file.eof()) {
				if (line[0] != '\t' && line[line.size()-2] == ':' && line[line.size()-3]!='.') {
					labels.push_back(line.substr(0, line.size()-2));
				}
				getline (s_file, line);
			}
	} else {
		printf("File not found! (%s) Quitting.\n", argv[2]);
		exit(1);
	}
	s_file.close();

	// analyze program
	break_returns(program);
	set_labels(program, labels);
	ranges = extract_ranges(program);

	display_program(program, ranges, 2);	// 0 is ranges-only, 1 is program-only, 2 is both

	return 0;
}

LINE parse_line(string line) {
	string pc, op, instr, arg;
	LINE ret;
	ret.label = 0;

	int xPC = line.find_first_of("\t", 0);		// find end of pc
	int xOP = line.find_first_of("\t", xPC+1);	// find end of opcode
	int xIN = line.find_first_of("\t", xOP+1);	// find end of instruction string

	ret.pc    = trim(line.substr(0, xPC-1));	// copy pc, skip colon
	ret.op    = line.substr(xPC+1, xOP-xPC-1);	// copy opcode
	ret.instr = line.substr(xOP+1, xIN-xOP-1);	// copy instruction string

	arg = line.substr(xIN+1);					// copy all arguments
	int xDST = arg.find_first_of(",", 0);		// find 1st comma (if it exists)
	int xSRC = arg.find_first_of(",", xDST+1);	// find 2nd comma (if it exists)

	ret.reg.dest = "";							// initialize the destination register
	ret.reg.src1 = "";							// initialize the source registers
	ret.reg.src2 = "";							// initialize the source registers

	if (xIN  != string::npos) ret.reg.dest = arg.substr(0,xDST);			// extract the destination register
	if (xDST != string::npos) ret.reg.src1 = arg.substr(xDST+1, xSRC-xDST-1);	// if src1 exists, extract it
	if (xSRC != string::npos) ret.reg.src2 = arg.substr(xSRC+1);			// if src2 exists, extract it

	return ret;
}


// separate functions at return statements
void break_returns(vector<LINE> &program) {
	LINE label;
	label.label = 1;

	for (int k=0; k<program.size(); k++) {
		if (program[k].ret()) {
			k+=2;										// skip branch slot
			program.insert(program.begin()+k, label);	// insert null line
		}
	}

	if (program[program.size()-1].label == 1) program.pop_back();
}

// go through all label instructions in program, set label name from *.s file
void set_labels(vector<LINE> &program, vector<string> labels) {
	int l = 0;

	for (int k=0; k<program.size(); k++) {
		if (program[k].label == 1) {
			while (labels[l][0] == '.') l++;
			program[k].instr = labels[l++];
		}
	}
}


void display_program(vector<LINE> &program, vector<RANGE> &ranges, int out) {
	if (out==0 || out==2) {
		int max_len = 0;
		int max_bits = 0;
		string buf,lo,hi;

		// find longest function name
		for (int k=0; k<ranges.size(); k++) {
			if (ranges[k].label.size() > max_len) max_len = ranges[k].label.size();
			/*if (ranges[k].lo.size() > max_bits) max_bits = ranges[k].lo.size();
			if (ranges[k].hi.size() > max_bits) max_bits = ranges[k].hi.size();*/
		}

		// print out for verilog use
		for (int k=0; k<ranges.size(); k++) {
			buf = "";
			lo = ranges[k].lo;
			hi = ranges[k].hi;
			for (int i=0; i<(max_len - ranges[k].label.size() + 3); i++) buf += " ";		// generate whitespace
			//for (int i=0; i<(max_bits - ranges[k].lo.size()); i++)		 lo = "0" + lo;		// 0-extend lo-range
			//for (int i=0; i<(max_bits - ranges[k].hi.size()); i++)		 hi = "0" + hi;		// 0-extend hi-range

			printf("`define %s_LO %s 'h%s\n", ucase(ranges[k].label).c_str(), buf.c_str(), ucase(lo).c_str());
			printf("`define %s_HI %s 'h%s\n", ucase(ranges[k].label).c_str(), buf.c_str(), ucase(hi).c_str());
		}
	}

	if (out == 2) printf("\n\n");

	if (out==1 || out==2) {
		for (int k=0; k<program.size(); k++) {
			if (program[k].label == 0) {
				printf("%3s:\t%s\t%s\t", program[k].pc.c_str(), program[k].op.c_str(), program[k].instr.c_str());
				if (program[k].reg.dest != "") printf("%s", program[k].reg.dest.c_str());
				if (program[k].reg.src1 != "") printf(",%s", program[k].reg.src1.c_str());
				if (program[k].reg.src2 != "") printf(",%s", program[k].reg.src2.c_str());
				printf("\n");
			} else if (program[k].instr != "") {
				printf("\n%s:\n", program[k].instr.c_str());
			}
		}
	}
}


// remove leading and trailing whitespace
string trim(string s) {
	int pos;
	for (pos=0 ; s[pos]==' ' || s[pos]=='\t'; ++pos);
	s.erase(0, pos);

	for (pos=s.size() ; pos && s[pos-1]==' ' || s[pos]=='\t'; --pos);
	s.erase(pos, s.size()-pos);

	return s;
}


vector<RANGE> extract_ranges(vector<LINE> &program) {
	vector<RANGE> ret;
	RANGE tRange;

	for (int k=0; k<program.size(); k++) {
		if (program[k].label == 1) {
			if (tRange.label != "") {	// if we're not at first function found
				tRange.hi = program[k-1].pc;
				ret.push_back(tRange);
			}
			tRange.label = program[k++].instr;
			tRange.lo = program[k].pc;
		}
	}
	if (tRange.label != "") {	// if we're not at first function found
		tRange.hi = program[program.size()-1].pc;
		ret.push_back(tRange);
	}

	return ret;
}


string ucase(string str) {
	for (int i=0; i<str.size(); i++) str[i] = toupper(str[i]);
	return str;
}
