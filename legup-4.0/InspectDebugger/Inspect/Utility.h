/* 
 * File:   Utility.h
 * Author: nazanin
 *
 * Created on September 14, 2013, 11:59 AM
 */

#ifndef UTILITY_H
#define	UTILITY_H

#include <string>
#include <vector>
#include <sstream>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include <map>
#include <fstream>
#include <bitset>
#include <unistd.h>
#include <math.h>
#include <iostream>
#include <sys/wait.h>
#include <sys/time.h>

/*
extern std::map<std::string, std::vector<int> > statesToCycles;
extern std::string statesToCyclesFileAddress;
extern std::string workDir;
extern std::string dbgMakeFilePath;
extern std::string increamentalDebugMakeFilePath;
extern std::string fileName;
extern std::string legUpDir;
extern std::string vsimDir;
extern std::string designFilename;
extern std::string rawSourceFilename;
extern std::string codeFilename;
extern std::string SWBinaryFilename;
extern std::string stpFilename;
extern std::string csvFileName;
extern std::string alteraMFLibPath;
extern std::string statesToCyclesFileAddress;
extern std::string dbHost;
extern std::string dbUser;
extern std::string dbPass;
extern std::string dbName;
extern std::vector<std::string> alteraFPPaths;
*/


std::vector<std::string> split(const std::string &s, char delim);
std::string IntToString(unsigned short i);
std::string IntToString(unsigned int i);
std::string IntToString(int i);
// converts a float to a string with 2 decimal places
std::string ftostr (float i);
std::string doubleToStr(double i);
std::string longlongToStr(long long i);
std::string unsignedLongLongToStr(unsigned long long i);
int BinaryToDecimal(std::string binary);

std::string trimMessage(std::string msg);

// trim from start
std::string &ltrim(std::string &s);
// trim from end
std::string &rtrim(std::string &s);
// trim from both ends
std::string &trim(std::string &s);
std::string trim(std::string input, std::vector<char>& charList);
std::string trimVarName(std::string input);
std::string trimToHex(std::string input);

int loadStatesToCyclesFile();

// Convert the 32-bit binary into the decimal  
float getFloat32(std::string binary);
// Convert the 32-bit binary encoding into hexadecimal  
int binary2Hex(std::string binary);

int hexCharToInt(char hexChar);
long long hexToLongLong(std::string hexString);
int hexToInt(std::string hexString);
float hexToFloat(std::string hexString);
double hexToDouble(std::string hexString);
unsigned int hexToUnsignedInt(std::string hexString);

std::string processLine(std::string line);

union int_to_float_converter_union {
    int u_val;
    float f_val;
};

union long_long_to_double_converter_union {
    long long u_val;
    double d_val;
};

void createDebugAndIncrementalScripts();
void loadConfigs();
void setFileNames();

void InitializeAlteraFPPaths();
int system_alternative(const char* path, char *const argv[]);
double getDiffTime(timeval start, timeval end);

enum OnChipDebugMode {
    USER_MODE_SIGNAL_SELECTION = 0,
    AUTO_MODE_SIGNAL_SELECTION = 1
};

#endif	/* UTILITY_H */

