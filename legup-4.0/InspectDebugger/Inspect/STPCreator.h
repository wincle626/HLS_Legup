/* 
 * File:   STPCreator.h
 * Author: nazanin
 *
 * Created on September 11, 2013, 11:46 AM
 */

#ifndef STPCREATOR_H
#define	STPCREATOR_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <map>
#include "rapidxml_print.hpp"
#include "Utility.h"
#include "HWSignal.h"
#include "DataAccess.h"
#include "formMain.h"

/*
extern std::string workDir;
extern std::string nodeNamesFilename;
extern std::string stpFilename;
extern std::string deviceInfoFileName;
extern int onChipDebugWindowSize;

extern DataAccess *DA;
*/

using namespace rapidxml;

class FPGANode {
public:
    std::string fullName;
    std::string type;
    std::string creator;
    int width;
    
    std::string getTypeName() {
        if (type == "output")
            return "output pin";
        else if (type == "input")
            return "intput pin";
        else if (type == "comb")
            return "combinatorial";
        else if (type == "reg")
            return "register";
        
        return "UNKNOWN";
    }                
};

class STPCreator {
public:
    STPCreator(std::vector<HWSignal*>& onChipWatchSignals, std::map<std::string, int>& selectedExtraSignals, std::map<std::string, int>& byDefaultAddedSignals, OnChipDebugMode signalSelectionMode);
    virtual ~STPCreator();
private:
    
    std::vector<HWSignal*> onChipWatchSignals;
    std::map<std::string, int> selectedExtraSignals;
    std::map<std::string, int> byDefaultAddedSignals;
    int triggerValue;
    OnChipDebugMode signalSelectionMode;
    xml_node<>* createXMLNode(xml_document<> *doc, node_type type, std::string nodeName, std::string nodeValue);
    xml_attribute<>* createXMLAttribute(xml_document<> *doc, std::string key, std::string value);    
        
    std::vector<std::string> nodeFileLines;
    
    void addSignalsListToSTP(std::map<std::string, int>& signalsList, std::vector<std::string>& allSignalsList);
    
public:
    void generateSTP(int triggerValue);
    void generateSTPForTimingSim(int triggerValue);        
    
    std::map<std::string, std::vector<FPGANode*> > signalsToNodesList;
};

#endif	/* STPCREATOR_H */
