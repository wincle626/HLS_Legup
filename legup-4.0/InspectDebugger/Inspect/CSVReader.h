/* 
 * File:   CSVReader.h
 * Author: nazanin
 *
 * Created on September 20, 2013, 4:12 PM
 */

#ifndef CSVREADER_H
#define	CSVREADER_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <map>
#include "Utility.h"
#include "HWSignal.h"

/*
extern std::string workDir;
extern std::string nodeNamesFilename;
extern std::string csvFileName;
*/

class CSVReader {
public:
    CSVReader();
    virtual ~CSVReader();
    
    bool readCSV();
    
    bool isFinished;
    std::map<std::string, std::vector<std::string> > signalsToValues;
    
private:            

};

#endif	/* CSVREADER_H */

