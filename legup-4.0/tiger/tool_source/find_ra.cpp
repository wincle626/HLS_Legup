// C program to find the caller address of main's return (for use with GXemul to add breakpoint and see result sent to mprintf)

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
using namespace std;


int main(int argc, char** argv) {
    string line = "";
    ifstream src;
    string break_addr;

    if (argc != 2) {
        printf("Incorrect number of arguments!\n");
        exit(1);
    }

    // parse list.txt
    src.open(argv[1], ios::in);
    if (src.is_open()) {
        while (!src.eof() && line.rfind(" <main>:") == string::npos) getline(src,line);		// find main function
        if (src.eof()) {
            printf("Main function not found!! ERROR!\n");
            exit(1);
        }

        getline(src,line);
        while (!src.eof()) {
            // find and print the result
            if (line.find("jr") != string::npos && line.find("ra") != string::npos) {
                break_addr = line.substr(0, line.find(":"));
                while (break_addr.find(' ') != string::npos) break_addr.replace(break_addr.find(' '), 1, 1, '0');
				while (break_addr.size() < 8) break_addr = "0" + break_addr;
                printf("0xffffffff%s\n", break_addr.c_str());
                break;
            }
            getline (src, line);
        }
        if (src.eof()) {
            printf("'jr ra' call not found!! ERROR!\n");
            exit(1);
        }
    } else {
        printf("File not found! (%s) Quitting.\n", argv[1]);
        exit(1);
    }
    src.close();


    return 0;
}
