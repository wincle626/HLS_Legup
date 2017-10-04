/* 
 * File:   DataAccess.h
 * Author: nazanin
 *
 * Created on June 19, 2013, 9:06 AM
 */

#ifndef DATAACCESS_H
#define	DATAACCESS_H

#include <mysql/mysql.h>
#include <string.h>
#include <vector>
#include <map>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <algorithm>

class IRInstruction;
class HLStatement;

#include "Utility.h"
#include "VariableUpdateInfo.h"
#include "HLStatement.h"
#include "Variable.h"
#include "State.h"
#include "OnChipSignal.h"

/*
extern std::vector<IRInstruction*> IRInstructions;
extern std::map<int, IRInstruction*> IRIdsToInstructions;
extern std::vector<HLStatement*> HLStatements;
extern std::map<int, HLStatement*> HLIdsToStatements;
extern std::map<int, std::vector<HLStatement*> > lineNumToStatement;
extern std::vector<HWSignal*> Signals;
extern std::map<int, HWSignal*> IdsToSignals;
extern std::vector<State*> States;
extern std::map<int, State*> IdsToStates;
extern std::vector<Variable*> Variables;
extern std::map<int, Variable*> IdsToVariables;
extern std::vector<VariableType*> VariableTypes;
extern std::map<int, VariableType*> IdsToVariableTypes;
extern std::vector<Function*> functions;
extern std::map<int, Function*> IdsToFunctions;
extern std::vector<OnChipSignal*> OnChipSignals;
extern std::map<int, OnChipSignal*> IdsToOnChipSignals;
extern std::string dbHost;
extern std::string dbUser;
extern std::string dbPass;
extern std::string dbName;
*/

class DataAccess {
public:
    DataAccess();    
    virtual ~DataAccess();
    
    void initializeDatabase();
    void closeConnection();
    void runQuery(std::string query);        
    
    //queries   
    std::vector<IRInstruction*> getIRInstructions(int highLevelLineNumber);
    std::vector<IRInstruction*> getIRInstructions(int highLevelLineNumber, int stateNumber);
    void getAllIRInstructions();
    void getAllHLStatements();
    void getAllHWSignals();
    void getAllOnChipSignals();
    std::vector<HWSignal*> getOnChipWatchSignals();
    std::vector<HWSignal*> getAllCurStateSignals();
    std::vector<HWSignal*> getAllFinishSignals();
    void getAllStates();
    void getAllFunctions();
    void getAllVariables();
    void getAllVariableTypes();
    void fillVariableTypeElements(VariableType* type);
    VariableType* getVariableType(int typeId);
    Type getTypeByDBId(int id);
    std::vector<std::string> getVariableData(int varId);
    int handleOnChipSignal(std::string name);
    int getOnChipSignalID(std::string name);
    int insertOnChipSignal(std::string name);
    void insertOnChipValue(int sigId, int time, std::string value);
    void insertOnChipValues(int sigId, std::map<int, std::string>& sigValMapping);
    void handleOnChipValue(int sigId, int time, std::string value);
    bool onChipValueExists(int sigId, int time);
    void DeleteOnChipSignals();
    std::string onChipGetValue(int sigId, int time);
    std::string OnChipGetSignalNameByPartialName(std::string partialName);
    int getSignalId(std::string name);
    std::vector<State*> getSignalStates(int signalId);
    std::vector<VariableUpdateInfo*> getStateUpdatingVariables(int stateId);  
    HWSignal* getStateStoreValueSignal(int stateId);
    int getStateStoreOffset(int stateId);
    std::string getStateStorePort(int stateId);    
    std::vector<std::string> getIRHardwareInfo(int IRid);
    std::vector<HLStatement*> getEffectiveStatementsForState(int stateId);
    std::vector<IRInstruction*> getStateInstructions(int stateId);
    std::string getStateNameByNumberAndBelongingFunctionId(int stateNum, int belongingFunctionId, int &stateId);
    int findEndState(IRInstruction *instr);
    bool isLineAFunctionCall(int lineNumber);
    int getCallingFunctionIdByLineNumber(int lineNumber);
    int getBelongingFunctionIdByLineNumber(int lineNumber);
    HLStatement* getCallStateHLStatement(int stateId);
    int getCallStateLineNumber(int stateId);
    bool StateRelatedToHLStatement(int stateId, int HLStatementId);
    
    std::vector<HWSignal*> getIRSignals(int instructionId);
    
private:
    MYSQL_RES *result;
    MYSQL_ROW row;
    MYSQL *connection, mysql;    
};

#endif	/* DATAACCESS_H */
