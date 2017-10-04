/*
 * File:   Utility.cpp
 * Author: nazanin
 *
 * Created on September 14, 2013, 12:10 PM
 */

#include "Utility.h"
#include "Globals.h"

std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }    
    return elems;
}

std::string IntToString(unsigned short i) {
  std::string s;
  std::stringstream out;
    out << i;
    s = out.str();
  return s;
}

std::string IntToString(unsigned int i) {
  std::string s;
  std::stringstream out;
    out << i;
    s = out.str();
  return s;
}

std::string IntToString(int i) {
  std::string s;
  std::stringstream out;
    out << i;
    s = out.str();
  return s;
}

// converts a float to a string with 2 decimal places
std::string ftostr (float i) {
    char f[100];
    snprintf(f, 100, "%.2f", i);
    return f;
}

std::string doubleToStr(double i) {
    char f[100];
    snprintf(f, 100, "%.2f", i);
    return f;
}

std::string longlongToStr(long long i) {
    char f[100];
    snprintf(f, 100, "%lli", i);
    return f;
}

std::string unsignedLongLongToStr(unsigned long long i) {
    char f[100];
    snprintf(f, 100, "%llu", i);
    return f;
}

int BinaryToDecimal(std::string binary) {
    if (binary.find("X") != std::string::npos)
        return 0;
    int decimal = 0;
    for (int bit = binary.size() - 1; bit >= 0; bit--)
                if (binary.c_str()[bit] == '1')
                    decimal += pow(2, binary.size() - bit - 1);
    return decimal;
}

// trim from start
std::string &ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
}

// trim from end
std::string &rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}

// trim from both ends
std::string &trim(std::string &s) {
        return ltrim(rtrim(s));
}

//inputs like 'a = 10' will be changed to ' 10'. everything before the '=' character will be removed.
std::string trimVarName(std::string input) {
    std::string result = "";
    for (int i = input.size() - 1; i >= 0; i--) {
        if (input[i] == '=')
            return result;
        result = input[i] + result;
    }
    return result;
}

std::string trim(std::string input, std::vector<char>& charList) {
    std::string result = "";
    for (int i = 0; i < input.size(); i++) {
        if (std::find(charList.begin(), charList.end(), input[i]) == charList.end())
            result += input[i];
    }
    return result;
}

std::string trimToHex(std::string input) {
    std::string result = "";
    for (int i = 0; i < input.size(); i++) {
        char ch = input[i];
        bool ok = false;
        if (ch == '-')
            ok = true;
        else if (ch >= '0' && ch<= '9')
            ok = true;
        else if (ch == 'a' || ch == 'b' || ch == 'c' || ch == 'd' || ch == 'e' || ch == 'f' ||
                 ch == 'A' || ch == 'B' || ch == 'C' || ch == 'D' || ch == 'E' || ch == 'F')
            ok = true;
        if (ok)
            result += ch;
    }
    return result;
}

std::string trimMessage(std::string msg)
{
    std::string trimmedStr = "";
    bool end = false;
    for (int i = 0 ; i < msg.size(); i++)
    {
        switch(msg[i])
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':            
                trimmedStr += msg[i];
                break;
            default:
                end = true;
                break;
        }
        if (end)
            break;
    }
    return trimmedStr;
}

int loadStatesToCyclesFile() {
    statesToCycles.clear();
    int maxCycle = -1;
    std::ifstream in;
    in.open((workDir + statesToCyclesFileAddress).c_str());
    std::string line;
    std::string currentState;
    while (in.good()) {
        getline(in, line);
        if (!line.compare(""))
            break;        
        if (line.find("state:") != std::string::npos)
            currentState = split(line, ':')[1];
        else { //the line must be the cycles line containing cycle numbers...
            std::vector<int> cycles;
            std::vector<std::string> splits = split(split(line, ':')[1], ',');
            for (int i = 0 ; i < splits.size(); i++) {
                std::string token = splits[i];
                if (token == "")
                    continue;
                int cycle = atoi(token.c_str());
                if (cycle > maxCycle)
                    maxCycle = cycle;
                cycles.push_back(cycle);
            }
            statesToCycles[currentState] = cycles;
        }
    }
    in.close();
    return maxCycle;
}

// Convert the 32-bit binary encoding into hexadecimal  
int binary2Hex(std::string binary)
{  
    std::bitset<32> set(binary);
    int hex = set.to_ulong();  
      
    return hex;  
} 

// Convert the 32-bit binary into the decimal  
float getFloat32(std::string binary)
{  
    int HexNumber = binary2Hex(binary);
  
    bool negative  = !!(HexNumber & 0x80000000);  
    int  exponent  =   (HexNumber & 0x7f800000) >> 23;      
    int sign = negative ? -1 : 1;  
  
    // Subtract 127 from the exponent  
    exponent -= 127;
  
    // Convert the mantissa into decimal using the  
    // last 23 bits  
    int power = -1;  
    float total = 0.0;  
    for ( int i = 0; i < 23; i++ )  
    {  
        int c = binary[ i + 9 ] - '0';  
        total += (float) c * (float) pow( 2.0, power );  
        power--;  
    }  
    total += 1.0;  
  
    float value = sign * (float) pow( 2.0, exponent ) * total;  
  
    return value;  
}

long long hexToLongLong(std::string hexString) {
    int pos;
    long long value = 0;
    bool isNeg = (hexString[0] == '-');
    if (isNeg)
        hexString = hexString.substr(1);
    for (int i = hexString.size() - 1; i>= 0; i--) {
        pos = hexString.size() - i - 1;
        char ch = hexString[i];
        value += hexCharToInt(ch) * (long long)(pow(16, pos));
    }
    if (isNeg)
        value *= -1;
    return value;
}

unsigned int hexToUnsignedInt(std::string hexString) {
    int pos;
    unsigned int value = 0;
    bool isNeg = (hexString[0] == '-');
    if (isNeg)
        hexString = hexString.substr(1);
    for (int i = hexString.size() - 1; i>= 0; i--) {
        pos = hexString.size() - i - 1;
        char ch = hexString[i];
        value += (unsigned int)hexCharToInt(ch) * (unsigned int)(pow(16, pos));
    }
    if (isNeg)
        value *= -1;
    return value;
}

int hexToInt(std::string hexString) {
    int pos;
    int value = 0;
    bool isNeg = (hexString[0] == '-');
    if (isNeg)
        hexString = hexString.substr(1);
    for (int i = hexString.size() - 1; i>= 0; i--) {
        pos = hexString.size() - i - 1;
        char ch = hexString[i];
        value += hexCharToInt(ch) * (int)(pow(16, pos));
    }
    if (isNeg)
        value *= -1;
    return value;
}

int hexCharToInt(char hexChar) {
    if (hexChar >= '0' && hexChar <= '9')
        return hexChar - '0';
    else if (hexChar == 'a' || hexChar == 'A')
        return 10;
    else if (hexChar == 'b' || hexChar == 'B')
        return 11;
    else if (hexChar == 'c' || hexChar == 'C')
        return 12;
    else if (hexChar == 'd' || hexChar == 'D')
        return 13;
    else if (hexChar == 'e' || hexChar == 'E')
        return 14;
    else if (hexChar == 'f' || hexChar == 'F')
        return 15;
    else
        return 0;       
}

float hexToFloat(std::string hexString) {
    union int_to_float_converter_union converter;
    int u_val = hexToInt(hexString);
    converter.u_val = u_val;
    return converter.f_val;
}

double hexToDouble(std::string hexString) {
    union long_long_to_double_converter_union converter;
    long long u_val = hexToLongLong(hexString);
    converter.u_val = u_val;
    return converter.d_val;
}

std::string processLine(std::string line){
    std::string result;
    for (int i = 0 ; i < line.size(); i++)
    {
        if (line[i] == '<')
            result += "&lt;";
        else if (line[i] == '>')
            result += "&gt;";
        /*else if (line[i] == ' ')
            result += "&nbsp;";
        else if (line[i] == '\t')
            result += "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";*/
        else
            result += line[i];
    }
    if (result == "")
        //result += "&nbsp;";
        result += "/**/";
    return result;
}

void setFileNames() {
    designFilename = fileName + ".v";
    rawSourceFilename = fileName + ".c";
    codeFilename = fileName + ".c";//_labeled
    SWBinaryFilename = fileName + ".out";
    stpFilename = fileName + ".stp";
    csvFileName = fileName + ".csv";
    
    alteraMFLibPath = vsimDir + "../altera/verilog/altera_mf/";
    statesToCyclesFileAddress = fileName + ".states_to_cycles.rpt";
}

void loadConfigs() {
    std::ifstream configFile("Inspect.config");
    if (configFile.is_open()) {
        std::string line;
        
        //1st config line: Modelsim Path
        std::getline(configFile, line);
        vsimDir = split(line, '=')[1];
        
        //2nd config line: Modelsim Path
        std::getline(configFile, line);
        legUpDir = split(line, '=')[1];           
        
        //3rd config line: Example Path
        std::getline(configFile, line);
        workDir = split(line, '=')[1];
		
        //4th config line: Example File
        std::getline(configFile, line);
        fileName = split(line, '=')[1];
        
        //5th config line: DB host
        std::getline(configFile, line);
        dbHost = split(line, '=')[1];
        
        //6th config line: DB user
        std::getline(configFile, line);
        dbUser = split(line, '=')[1];
        
        //7th config line: DB pass
        std::getline(configFile, line);
        dbPass = split(line, '=')[1];
        
        //8th config line: DB name
        std::getline(configFile, line);
        dbName = split(line, '=')[1];
        
        configFile.close();
        
    } else {
        std::cout << "config file not found. Program is exiting." << std::endl;
        exit(1);
    }
}

void createDebugAndIncrementalScripts() {
    std::ofstream dbgFile;
    dbgFile.open(dbgMakeFilePath.c_str());
    dbgFile << "#!/bin/sh" << std::endl;
    dbgFile << "make -C " << workDir << " debug" << std::endl;
    dbgFile.close();
    
    std::ofstream incFile;
    incFile.open(increamentalDebugMakeFilePath.c_str());
    incFile << "#!/bin/sh" << std::endl;
    incFile << "make -C " << workDir << " dbg" << std::endl;
    incFile.close();
}

void InitializeAlteraFPPaths() {
#ifdef PYTHON_WRAPPER
	loadConfigs();
#endif
   alteraFPPaths.push_back(legUpDir + "ip/libs/altera/altera_mf.v");
   alteraFPPaths.push_back(legUpDir + "ip/libs/altera/220model.v");
   alteraFPPaths.push_back(legUpDir + "ip/libs/altera/altfp_adder_14.v");
   alteraFPPaths.push_back(legUpDir + "ip/libs/altera/altfp_adder64_14.v");
   alteraFPPaths.push_back(legUpDir + "ip/libs/altera/altfp_subtractor_14.v");
   alteraFPPaths.push_back(legUpDir + "ip/libs/altera/altfp_subtractor64_14.v");
   alteraFPPaths.push_back(legUpDir + "ip/libs/altera/altfp_multiplier_11.v");
   alteraFPPaths.push_back(legUpDir + "ip/libs/altera/altfp_multiplier64_11.v");
   alteraFPPaths.push_back(legUpDir + "ip/libs/altera/altfp_divider_33.v");
   alteraFPPaths.push_back(legUpDir + "ip/libs/altera/altfp_divider64_61.v");
   alteraFPPaths.push_back(legUpDir + "ip/libs/altera/altfp_compare32_1.v");
   alteraFPPaths.push_back(legUpDir + "ip/libs/altera/altfp_compare64_1.v");
   alteraFPPaths.push_back(legUpDir + "ip/libs/altera/altfp_truncate_3.v");
   alteraFPPaths.push_back(legUpDir + "ip/libs/altera/altfp_extend_2.v");
   alteraFPPaths.push_back(legUpDir + "ip/libs/altera/altfp_sitofp32_6.v");
   alteraFPPaths.push_back(legUpDir + "ip/libs/altera/altfp_sitofp64_6.v");
   alteraFPPaths.push_back(legUpDir + "ip/libs/altera/altfp_fptosi32_6.v");
   alteraFPPaths.push_back(legUpDir + "ip/libs/altera/altfp_fptosi64_6.v");
   alteraFPPaths.push_back(legUpDir + "ip/libs/altera/altfp_adder_13.v");
   alteraFPPaths.push_back(legUpDir + "ip/libs/altera/altfp_adder64_13.v");        
}

//an alternative to system call that creates a fork and run it until end...
int system_alternative(const char* path, char *const argv[])
{
   pid_t pid = fork();
            
    if (pid > 0) {
        // We're the parent, so wait for child to finish
        int status;
        waitpid(pid, &status, 0);
        return status;
    }   
    else if (pid == 0) {
                
        // We're the child, so run the specified program.  Our exit status will
        // be that of the child program unless the execv() syscall fails.
        char *envp[] = {"LM_LICENSE_FILE=1802@ra.eecg.toronto.edu", NULL};
        return execve(path, argv, envp);
    }
    else{
         // Something horrible happened, like system out of memory
        return -1;
    }
}

double getDiffTime(timeval start, timeval end) {
    long seconds = end.tv_sec - start.tv_sec;
    long mseconds = end.tv_usec - start.tv_usec;
    
    long elapsed = ((seconds) * 1000 + mseconds / 1000.0) + 0.5;
    return (double)elapsed/1000.0;
}
