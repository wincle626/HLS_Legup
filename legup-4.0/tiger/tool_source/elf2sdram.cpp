#include <fstream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Debug_Stub/regdef.h" // contains definitions for STACK_ADDR & PROF_ADDR

using namespace std;

#define STRING_PER_LINE 4
#define BOOTLOADER_SIZE 8
#define CHAR_PER_INSTRUCTION 8

// bootloader code which gets run at the beginning of processor execution
// it sets the stack pointer, and sets return address, jumps to start of program
// when program is done, it jumps back to return address, so then it jumps stays
// in the address
const string bootcode_DE2[BOOTLOADER_SIZE] = {
    "3c1d0100", // set stack pointer, LUI r29, 0x0100_0000
    "0c200008", // jump and link to starting PC (80_0020), JAL 80_0020
    "00000000", // branch slot
    "08200003", // jump to itself (when finished execution), j pc = pc (80_000c)
    "00000000", // branch slot
    "00000000", // branch slot
    "00000000", // branch slot
    "00000000"  // branch slot
};
const string bootcode_DE4[BOOTLOADER_SIZE] = {
    "3c1d8000", // set stack pointer, LUI r29, 0x1000_0000
    "0c000004", // jump and link to starting PC (4000_0020), JAL 0c00_0004
    "00000000", // branch slot
    "08000003", // jump to itself (when finished execution), j pc = pc
    "00000000", // branch slot
    "00000000", // branch slot
    "00000000", // branch slot
    "00000000"  // branch slot
};

int MEM_START;
int CHAR_PER_LINE;

int NUM_FUNCTIONS;
void output_prof_data(FILE *sdram, char *hash_file, int data_size,
                      int &line_num);
void output_SnoopP_prof_data(FILE *sdram, char *range_file, int data_size,
                             int &line_num);

int main(int argc, char **argv) {
    if (argc != 5 && argc != 8 && argc != 9) {
        printf("Incorrect usage.  Expecting <elf_file> <MEM_START> "
               "<CHAR_PER_LINE> <sdram_file> [-p <prof_file> <NUM_FUNCTIONS> "
               "[-SnoopP] ].\n");
        printf("For the DE2 board MEM_START = 0x00800020, and CHAR_PER_LINE = "
               "4\n");
        printf("For the DE4 board MEM_START = 0x40000020, and CHAR_PER_LINE = "
               "32\n");
        exit(1);
    }

    ifstream fHDump;
    string line, name, data = "";
    unsigned long start_addr = 0;
    unsigned long text_end_addr = 0;
    bool isText = true;
    unsigned end_addr = 0;

    // get arguments
    MEM_START = (int)strtol(argv[2], NULL, 0);
    CHAR_PER_LINE = atoi(argv[3]);

    // disassemble elf headers
    system(("objdump -h " + string(argv[1]) + " > " + string(argv[1]) +
            ".header.dump").c_str());
    fHDump.open((string(argv[1]) + ".header.dump").c_str(),
                ios::in | ios::binary);

    if (fHDump.is_open()) {
        unsigned vma;
        unsigned size;
        // find first section
        while (!fHDump.eof() && line.substr(0, 8) != "Idx Name") {
            getline(fHDump, line);
        }
        while (!fHDump.eof()) {
            getline(fHDump, line);
            sscanf(line.c_str(), "%*d %*s %x %x", &size, &vma);
            if (vma != 0) {
                end_addr = size + vma;
            }
            // ignore attributes
            getline(fHDump, line);
        }
    }
    fHDump.close();

    // disassemble elf
    ifstream fDump;
    system(("objdump -s " + string(argv[1]) + " > " + string(argv[1]) + ".dump")
               .c_str());
    fDump.open((string(argv[1]) + ".dump").c_str(), ios::in | ios::binary);

    if (fDump.is_open() && !fDump.fail()) {
        // find first section
        while (!fDump.eof() && line.substr(0, 19) != "Contents of section")
            getline(fDump, line);

        // NOTE -- THIS SKIPS BSS!!
        // store section data
        while (!fDump.eof()) {
            if (line.substr(0, 19) == "Contents of section") {
                name = line.substr(21, line.size() - 22);

                // get next line so i know the address
                getline(fDump, line);

                sscanf(line.c_str(), "%lx ", &start_addr);

                if (isText) {
                    isText = false;
                } else if (start_addr > 0) {
                    // add spacing if necessary
                    unsigned space = start_addr - data.size() / 2 - MEM_START;
                    printf("  space = %u\n", space);
                    while (space-- > 0)
                        data += "00";
                }

                // if the address starts at 0, ignore
                if (start_addr > 0) {
                    sscanf(line.c_str(), "%lx", &text_end_addr);

                    printf("Section Name: %s, Section Start: %lx\n",
                           name.c_str(), start_addr);
                    // have to do line processing so the data isn't lost
                    int x = line.find_first_of(" ", 3);
                    for (int k = x; k < x + 36; k++) {
                        if (line[k] != ' ')
                            data += line[k];
                    }
                }
            } else if (start_addr > 0) {
                sscanf(line.c_str(), "%lx", &text_end_addr);

                int x = line.find_first_of(" ", 3);
                for (int k = x; k < x + 36; k++) {
                    if (line[k] != ' ')
                        data += line[k];
                }
            }

            if (!fDump.eof())
                getline(fDump, line);
        }
    } else {
        printf("File %s failed to load! Quitting.\n", argv[1]);
        exit(1);
    }

    fDump.close();

    // pad empty sections
    unsigned space = end_addr - data.size() / 2 - MEM_START;
    while (space-- > 0)
        data += "00";

    // pad with 0's so that size is a multiple of 4
    if (data.size() % 4 == 2)
        data += "00";

    FILE *sdram = fopen(argv[4], "w");
    int line_num = 0;
    int chars_on_line = 0;

    // TODO Can this be cleaned up?
    // Print the bootloader
    if (CHAR_PER_LINE == 4) {
        // print bootloader for DE2
        for (int j = 0; j < BOOTLOADER_SIZE; j += 1) {
            for (int k = 0; k < 4; k++)
                fprintf(sdram, "%c", bootcode_DE2[j][4 + k]);
            fprintf(sdram, "\n");
            line_num++;

            for (int k = 0; k < 4; k++)
                fprintf(sdram, "%c", bootcode_DE2[j][k]);
            fprintf(sdram, "\n");
            line_num++;
        }
    } else {
        // print bootloader for DE4, any other board with > 4 char per line
        // TODO: Not sure if this gives correct byte order for other values
        // of CHAR_PER_LINE.  Not sure what it is supposed to look like for
        // values other than 32.
        for (int j = 0; j < BOOTLOADER_SIZE;
             j += CHAR_PER_LINE / CHAR_PER_INSTRUCTION) {
            for (int k = CHAR_PER_LINE / CHAR_PER_INSTRUCTION; k > 0;) {
                k--;
                fprintf(sdram, "%s", bootcode_DE4[j + k].c_str());
            }
            fprintf(sdram, "\n");
            line_num++;
        }
    }

    // printing the values
    for (int j = 0; j < data.size(); j += CHAR_PER_LINE) {
        // print a line
        for (int k = CHAR_PER_LINE / 4; k > 0;) {
            k--;
            fprintf(sdram, "%c%c%c%c", data[j + k * 4 + 2], data[j + k * 4 + 3],
                    data[j + k * 4 + 0], data[j + k * 4 + 1]);
        }
        fprintf(sdram, "\n");

        line_num++;
        for (int i = 0; i < CHAR_PER_LINE; i++) {
            if (data[i] == '.') {
                printf("failed to produce %s\n", argv[4]);
                exit(1);
            }
        }
    }

    if (CHAR_PER_LINE == 4) {
        // if we want profiling information
        if ((argc == 8 || argc == 9) && string(argv[5]) == "-p") {
            sscanf(argv[7], "%d", &NUM_FUNCTIONS);

            if (argc == 9 && !strcmp(argv[8], "-SnoopP")) {
                output_SnoopP_prof_data(sdram, argv[6], data.size(), line_num);
            } else {
                output_prof_data(sdram, argv[6], data.size(), line_num);
            }
        }

        // pad rest of lines with 0's (so that sdram.dat fills the total space
        // on SDRAM (8MB) to initialize where there is no data with 0's)
        // TODO Is this necessary?
        for (int i = line_num; i < 4194304; i++) {
            fprintf(sdram, "0000\n");
        }
    }

    fclose(sdram);
    return 0;
}

// TODO
// NOTE: These functions work with the DE2 settings (CHAR_PER_LINE = 4)
// but may not work with other values.  This did not exist for elf2sdram_DE4.

void output_prof_data(FILE *sdram, char *hash_file, int data_size,
                      int &line_num) {
    long int V1;
    int A1, A2, B1, B2;
    int tab[NUM_FUNCTIONS + 1];
    char prof_data[(8 + NUM_FUNCTIONS) * 2 + 5];
    FILE *fHash = fopen(hash_file, "r");

    if (fHash == NULL) {
        printf("Failed to open file %s\n", hash_file);
        exit(1);
    }

    // Read/parse data from .prof file as 8+N bytes of information
    // (TAB,V1,A1,A2,B1,B2)
    fscanf(fHash, "tab[] = {");
    for (int i = 0; i < NUM_FUNCTIONS; i++)
        fscanf(fHash, "%d,", &tab[i]);
    tab[NUM_FUNCTIONS] = '\0'; // so we can use it as a string to output

    fscanf(fHash, "}\nV1 = 0x%lx\n", &V1);
    fscanf(fHash, "A1 = %d\n", &A1);
    fscanf(fHash, "A2 = %d\n", &A2);
    fscanf(fHash, "B1 = %d\n", &B1);
    fscanf(fHash, "B2 = 0x%x\n", &B2);

    // Test Output
    if (1) {
        printf("tab = { ");
        for (int i = 0; i < NUM_FUNCTIONS; i++)
            printf("%d, ", (int)tab[i]);
        printf("}\n");
        printf("V1 = 0x%lx, A1 = %d, A2 = %d, B1 = %d, B2 = %d\n", V1, A1, A2,
               B1, B2);
    }

    fclose(fHash);

    // put data into one char array for sending to profiler
    for (int i = 0; i < NUM_FUNCTIONS; i++)
        sprintf(
            &prof_data[i * 2], "%02x",
            tab[i]); // i*2 b/c 2 chars/byte (0xFF is one byte, two characters)
    sprintf(&prof_data[NUM_FUNCTIONS * 2], "%08x%02x%02x%02x%02x%c",
            (unsigned int)V1, A1, A2, B1, B2, '\0'); // NUM_FUNCTIONS

    printf("prof_data = %s\n", prof_data);

    // output spacing to sdram.dat
    for (int j = (data_size / 2); j < (PROF_ADDR - MEM_START);
         j += CHAR_PER_LINE) {
        fprintf(sdram, "0000\n0000\n");
        line_num += 2;
    }

    // output hash data to sdram.dat
    for (int i = 0; i < (8 + NUM_FUNCTIONS) * 2; i += 8) {
        fprintf(sdram, "%c%c%c%c\n", prof_data[i + 4], prof_data[i + 5],
                prof_data[i + 6], prof_data[i + 7]);
        fprintf(sdram, "%c%c%c%c\n", prof_data[i + 0], prof_data[i + 1],
                prof_data[i + 2], prof_data[i + 3]);
        line_num += 2;
    }

    // output end of sdram.dat
    /*for (int i=PROF_ADDR+(8+NUM_FUNCTIONS); i<0xFFFFFF; i+=4) {
        fprintf(sdram, "0000\n0000\n");
    }*/
}

void output_SnoopP_prof_data(FILE *sdram, char *range_file, int data_size,
                             int &line_num) {
    // output spacing to sdram.dat
    for (int j = (data_size / 2); j < (PROF_ADDR - MEM_START);
         j += CHAR_PER_LINE) {
        fprintf(sdram, "0000\n0000\n");
        line_num += 2;
    }

    int ranges[NUM_FUNCTIONS * 2];
    char prof_data[(NUM_FUNCTIONS * 2) * 8 + 1]; // 2 ranges per function, 8
                                                 // chars per range (32 bits is
                                                 // 8 hex)

    // Read/parse data from .flist_range file as 2*NUM_FUNCTIONS ranges (lo -->
    // hi)
    FILE *fRanges = fopen(range_file, "r");
    for (int i = 0; i < NUM_FUNCTIONS * 2; i += 2)
        fscanf(fRanges, "%x --> %x", &ranges[i], &ranges[i + 1]);
    fclose(fRanges);

    // put data into one char array for sending to profiler
    for (int i = 0; i < NUM_FUNCTIONS * 2; i++)
        sprintf(&prof_data[i * 8], "%08x", ranges[i]); // i*2 b/c 2 chars/byte
                                                       // (0xFF is one byte, two
                                                       // characters)

    // output to sdram.dat
    for (int i = 0; i < (NUM_FUNCTIONS * 2) * 8; i += 8) {
        fprintf(sdram, "%c%c%c%c\n", prof_data[i + 4], prof_data[i + 5],
                prof_data[i + 6], prof_data[i + 7]);
        fprintf(sdram, "%c%c%c%c\n", prof_data[i + 0], prof_data[i + 1],
                prof_data[i + 2], prof_data[i + 3]);
        line_num += 2;
    }

    // output end of sdram.dat
    /*for (int i=PROF_ADDR+(NUM_FUNCTIONS*2)*8; i<0xFFFFFF; i+=4) {
        fprintf(sdram, "0000\n0000\n");
    }*/
}
