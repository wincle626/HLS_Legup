/* 
 * File:   DebugEngine.h
 * Author: nazanin
 *
 * Created on October 20, 2013, 8:39 PM
 */

#ifndef DEBUGENGINE_H
#define	DEBUGENGINE_H

#include <map>
#include <vector>
#include <algorithm>
#include <string>
#include "DataAccess.h"
#include "Utility.h"

/*
extern DataAccess *DA;
*/


class OnChipDebugEngine {
public:
    OnChipDebugEngine();    
    virtual ~OnChipDebugEngine();           
    
    void runForOneCycle();
    std::string examine(std::string signalName);
    std::string batchExamine(std::vector<std::string>& partialSignalNames);
    void initialize(std::map<std::string, std::map<int, std::string> >& onChipValues);
    void initialize();
    std::string getSignalNameByPartialName(std::string partialSignalName);
    
    bool isInitialized;
    
private:
    
    int current_cycle;
    std::map<std::string, int> signalNamesToIds;
    std::map<int, std::map<int, std::string> > signalIdToValues;
    std::map<std::string, std::string> signalNamesToOnChipNames;
    int lastId;

};

#endif	/* DEBUGENGINE_H */

