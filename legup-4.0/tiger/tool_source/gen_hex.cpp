#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <math.h>
using namespace std;

//#define INSERT_NOP

int h2i(char h);
char i2h(int i);
string i2hs(int i);
string parse_line(string line);
string gen_checksum(string line);
string format_line_num(int line_num);

int main(int argc, char** argv) {
	vector<string> code;
	string line = "";
	ifstream list;
	ofstream hex0, hex1, hex2, hex3;

	if (argc != 2) { printf("Incorrect number of arguments!\n"); exit(1); }

	// parse list.txt
	list.open(argv[1], ios::in);
	if (list.is_open()) {
			while(line != "00000000 <.text>:") getline(list,line);		// skip past beginning

			getline(list,line);
			while (!list.eof()) {
				code.push_back(parse_line(line));
				#ifdef INSERT_NOP
					code.push_back("00000000");
				#endif
				getline (list, line);
			}
	} else {
		printf("File not found! (%s) Quitting.\n", argv[1]);
		exit(1);
	}
	list.close();

	// create HEX files (":_10_00" + line# + "0_00" + 16 entries + chksum)
	// EX: ":10_0010_00_3c373c343c343c37ac0014240c000834_fe"

	// open files
	hex0.open("code0.hex", ios::out);
	hex1.open("code1.hex", ios::out);
	hex2.open("code2.hex", ios::out);
	hex3.open("code3.hex", ios::out);

	// output lines
	int buf = 0;
	string buf0, buf1, buf2, buf3;
	int line_num = 0;
	for (int k=0; k<code.size(); k++) {
		// reset buffers
		if (buf == 0) buf0 = buf1 = buf2 = buf3 = "";

		// add to buffers
		buf0 += code[k].substr(0,2);
		buf1 += code[k].substr(2,2);
		buf2 += code[k].substr(4,2);
		buf3 += code[k].substr(6,2);
		buf++;

 		// output!
		if (buf == 16) {	// use 16 not 15 b/c buf++ just happened
			buf0 = "10" + format_line_num(line_num) + "00" + buf0;
			buf1 = "10" + format_line_num(line_num) + "00" + buf1;
			buf2 = "10" + format_line_num(line_num) + "00" + buf2;
			buf3 = "10" + format_line_num(line_num) + "00" + buf3;

			hex0 << ":" << buf0 << gen_checksum(buf0) << "\n";
			hex1 << ":" << buf1 << gen_checksum(buf1) << "\n";
			hex2 << ":" << buf2 << gen_checksum(buf2) << "\n";
			hex3 << ":" << buf3 << gen_checksum(buf3) << "\n";

			line_num++;
			buf = 0;
		}
	}

	// output anything that was filling the last line
	if (buf > 0) {
		// fill the line
		string buf_zeros = "";
		buf_zeros.insert(0,(16-buf)*2, '0');
		buf0 = "10" + format_line_num(line_num) + "00" + buf0 + buf_zeros;
		buf1 = "10" + format_line_num(line_num) + "00" + buf1 + buf_zeros;
		buf2 = "10" + format_line_num(line_num) + "00" + buf2 + buf_zeros;
		buf3 = "10" + format_line_num(line_num) + "00" + buf3 + buf_zeros;

		//output
		hex0 << ":" << buf0 << gen_checksum(buf0) << "\n";
		hex1 << ":" << buf1 << gen_checksum(buf1) << "\n";
		hex2 << ":" << buf2 << gen_checksum(buf2) << "\n";
		hex3 << ":" << buf3 << gen_checksum(buf3) << "\n";
	}

	// output file_close line
	hex0 << ":00000001ff\n";
	hex1 << ":00000001ff\n";
	hex2 << ":00000001ff\n";
	hex3 << ":00000001ff\n";

	// close files
	hex0.close();
	hex1.close();
	hex2.close();
	hex3.close();

	return 0;
}

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

		default:
			cout << "h2i value invalid....wtf? " << h << "\n";
			exit(1);
	}
}

char i2h(int i) {
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

string i2hs(int i) {
	char h[2] = { i2h(i), '\0' };
	return string(&h[0]);
}

string parse_line(string line) {
	int xPC = line.find_first_of("\t", 0);		// find end of pc
	int xOP = line.find_first_of("\t", xPC+1);	// find end of opcode

	return line.substr(xPC+1, xOP-xPC-1);
}

string gen_checksum(string line) {
	int chk;
	int sum = 0;

	for (int j=0; j<line.size(); j+=2) sum += 16*h2i(line[j]) + h2i(line[j+1]);
	chk = (0x100 - (0x00000000FF & sum));
	if (chk == 0x100) chk = 0;

	return (i2hs(floor(chk/16)) + i2hs(chk%16));
}

string format_line_num(int n) {
	return (i2hs((n&0x00000F00)>>8) + i2hs((n&0x000000F0)>>4) + i2hs(n&0x0000000F) + "0");
}
