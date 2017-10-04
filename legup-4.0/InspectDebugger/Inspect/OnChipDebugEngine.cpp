/* 
 * File:   DebugEngine.cpp
 * Author: nazanin
 * 
 * Created on October 20, 2013, 8:39 PM
 * DebugEngine class is used to control the program's execution when on-chip debug is enabled
 * it somehow mimics the behavior of Modelsim process
 */

#include "OnChipDebugEngine.h"
#include "STPCreator.h"
#include "formMain.h"
#include "Globals.h"

std::string examinedSignalFileAddress = "examinedSignals.dat";
std::ofstream out;

OnChipDebugEngine::OnChipDebugEngine() {
    current_cycle = -1;
    isInitialized = false;
    lastId = 0;
}

OnChipDebugEngine::~OnChipDebugEngine() {
}

void OnChipDebugEngine::runForOneCycle() {
    current_cycle++;
}

void OnChipDebugEngine::initialize(std::map<std::string, std::map<int, std::string> >& onChipValues) {
    std::map<std::string, std::map<int, std::string> >::iterator it;
    for (it = onChipValues.begin(); it != onChipValues.end(); ++it) {
        int signalId = 0;
        if (signalNamesToIds.find((*it).first) != signalNamesToIds.end())
            signalId = signalNamesToIds[(*it).first];
        else {
            signalNamesToIds[(*it).first] = lastId;
            signalId = lastId;
            lastId++;
        }
        
        std::map<int, std::string>::iterator it2;
        for (it2 = (*it).second.begin(); it2 != (*it).second.end(); ++it2) {
            int cycle = (*it2).first;
            std::string val = (*it2).second;
            
            signalIdToValues[signalId][cycle] = val;
        }                
    }
}

void OnChipDebugEngine::initialize() {
    std::ifstream in((workDir + onChipDebugInfoFileAddress).c_str());
    std::string line;
    if (in.is_open()) {
        int currentSignalId = -1;
        bool newSignalSeen = false;
        while (getline(in, line)) {
            line = trim(line);
            if (line.compare("*") == 0) {
                currentSignalId++;
                newSignalSeen = true;
            } else if (newSignalSeen) {
                signalNamesToIds[line] = currentSignalId;
                newSignalSeen = false;
            } else {
                std::vector<std::string> splits = split(line, ',');
                int cycle = atoi(splits[0].c_str());
                std::string val = splits[1];
                signalIdToValues[currentSignalId][cycle] = val;
            }
        }
        in.close();
    }
    isInitialized = true;
    remove ((workDir+ "log.txt").c_str());
    
    
}

std::string OnChipDebugEngine::getSignalNameByPartialName(std::string partialSignalName) {
    std::map<std::string, int>::iterator it;
    for (it = signalNamesToIds.begin(); it != signalNamesToIds.end(); ++it) {
        if ((*it).first.find(partialSignalName) != std::string::npos) {
            return (*it).first;
        }
    }
    return "";
}

std::string OnChipDebugEngine::batchExamine(std::vector<std::string>& partialSignalNames) {
    std::string result;
    for (int i = 0 ; i < partialSignalNames.size(); i++) {
        result += examine(partialSignalNames[i]) + " ";
    }
    return result;
}

std::string OnChipDebugEngine::examine(std::string partialSignalName) {
    std::ofstream log;    
    /*if (partialSignalName.find("main_10_11") != std::string::npos) {
        int a;
        a = 10;
        int b;
        b = a + 2;
    }*/
    /*if (partialSignalName.find("memory_controller") == std::string::npos &&
            partialSignalName.find("cur_state") == std::string::npos &&
            partialSignalName.find("finish") == std::string::npos) {*/
        log.open((workDir + "log.txt").c_str(), std::ios::app);
        log << "TIME:" << current_cycle << ",SIGNAL:" << partialSignalName << std::endl;
        log.flush();
        log.close();
    //}
    std::string signalName;
    if (signalNamesToOnChipNames.find(partialSignalName) != signalNamesToOnChipNames.end()) {
        signalName = signalNamesToOnChipNames[partialSignalName];
    } else {
        //signalName = DA->OnChipGetSignalNameByPartialName(partialSignalName);
        signalName = getSignalNameByPartialName(partialSignalName);
        if (signalName == "")
            return "";
        signalNamesToOnChipNames[partialSignalName] = signalName;
    }    
        
    int signalId;
    if (signalNamesToIds.find(signalName) != signalNamesToIds.end())    
        signalId = signalNamesToIds[signalName];
    else { 
        //shouldn't come to this condition...
        signalId = DA->getOnChipSignalID(signalName);
        signalNamesToIds[signalName] = signalId;                
    }
    std::string value = "X";
    bool idFound = false;
    if (signalIdToValues.find(signalId) != signalIdToValues.end()) {    
        idFound = true;
        if (signalIdToValues[signalId].find(current_cycle) != signalIdToValues[signalId].end())
        {
            value = signalIdToValues[signalId][current_cycle];            
            return IntToString(BinaryToDecimal(value));
        }
        else
        {
            
        }
    }
    
    value = DA->onChipGetValue(signalId, current_cycle);
    signalIdToValues[signalId][current_cycle] = value;
    
    return IntToString(BinaryToDecimal(value));
    
    //return value;       
}
