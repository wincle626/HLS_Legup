/* 
 * File:   CSVReader.cpp
 * Author: nazanin
 * 
 * Created on September 20, 2013, 4:13 PM
 */

#include <algorithm>

#include "CSVReader.h"
#include "Globals.h"

CSVReader::CSVReader() { 
    isFinished = false;
}

CSVReader::~CSVReader() {
    
}

bool CSVReader::readCSV() {
    
    std::ifstream in;
    in.open((workDir + csvFileName).c_str());
    std::vector<std::string> lines;
    while(in.good())
    {
        std::string line;
        getline(in, line);
        lines.push_back(line);
    }
    in.close();
    
    bool dataLineReached = false;
    int i = 0;
    while(!dataLineReached) {
        std::string line = lines[i];
        if (line.find("Data:") != std::string::npos)
            dataLineReached = true;
        i++;
    }
    
    //reading headers    
    std::string headerLine = lines[i];
    std::vector<std::string> headers = split(headerLine, ',');    
    
    
    i++;
    //reading data
    
    int rowCount = lines.size() - i + 1;
    std::string **rawData = new std::string*[rowCount];
    for (int r = 0 ; r < rowCount; r++)
    {
        //data[h] = new std::string[lines.size() - i];    
        rawData[r] = new std::string[headers.size()];
        
    }
    for ( int h = 0; h < headers.size(); h++)
    {        
        rawData[0][h] = headers[h];
    }
    int didx = 1;
    for (; i < lines.size(); i++) {
        std::string line = lines[i];
        std::vector<std::string> toks = split(line, ',');
        std::string currentHeader = "XXXXXXXXXXXXXX";//some header name that we're sure it doesn't match for the first step.
        std::string val = "";
        for (int t = 0 ; t < toks.size(); t++)
        {
            rawData[didx][t] = toks[t];
            std::string hdr = split(headers[t], '[')[0];
            if (hdr.find(currentHeader) != std::string::npos) {
                toks[t].erase(std::remove_if(toks[t].begin(), toks[t].end(), ::isspace), toks[t].end());
                val = toks[t] + val;
            } else {
                if (currentHeader != "XXXXXXXXXXXXXX") {
                    currentHeader.erase(std::remove_if(currentHeader.begin(), currentHeader.end(), ::isspace), currentHeader.end());
                    signalsToValues[currentHeader].push_back(val);
                }
                toks[t].erase(std::remove_if(toks[t].begin(), toks[t].end(), ::isspace), toks[t].end());
                val = toks[t];
                currentHeader = hdr;
                if(currentHeader.find("top_inst|finish") != std::string::npos)
                {
                    if (atoi(val.c_str()))
                    {
                        //isFinished = true;
                        //return isFinished;
                        //return false;                        
                    }
                   
                }
            }
            
        }
        didx++;
    }
    
    std::map<std::string, std::vector<std::string> >::iterator it;
    for (it = signalsToValues.begin(); it != signalsToValues.end(); ++it) {
        std::string signalFullName = (*it).first;
        std::string sigName = split(signalFullName, '|').back();
        //std::list<std::string> 
        std::cout << (*it).first << " : ";
        for (int i = 0 ; i < (*it).second.size(); i++) {
            std::cout << (*it).second[i] << " , ";
        }
        std::cout << std::endl;
    }
    return false;
}

