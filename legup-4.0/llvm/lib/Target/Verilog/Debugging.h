/* 
* File:   Debugging.h
* Author: legup
*
* Created on May 29, 2013, 3:38 PM
*/

#ifndef DEBUGGING_H
#define	DEBUGGING_H

#include "Allocation.h"
#include "LegupPass.h"
#include "VerilogWriter.h"
#include "utils.h"
#include "Binding.h"
#include "RTL.h"
#include "GenerateRTL.h"
#include "LegupConfig.h"
#include "Scheduler.h"
#include "ResourceEstimator.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/AssemblyAnnotationWriter.h"
#include <fstream>
#include <sys/time.h>
#include <mysql/mysql.h>

using namespace llvm;

namespace legup
{   
    
class Debugging {
public:
    Debugging();
    Debugging(const Debugging& orig);
    virtual ~Debugging();
    void createFunctionLineNumberTable(Function* F);     
    
    void mapIRsToStates(GenerateRTL* HW);
    
    void initializeDatabase();
    void initialize();
    void closeConnection();
    void runQuery(std::string query);       
    
    int getFunctionStartLineNumber(Function* F);
    void fillDebugDB(Function* F);
    void fillHardwareInfo(Allocation *allocation);
    void fillSignals(Allocation *allocation);
    void fillCurStateAndFinishSignals(Allocation *allocation);
    void fillVariables(Allocation *allocation);
    bool checkIfVarIsPassedToAnotherFunction(Instruction* instr, std::string &varName, int &functionId);
        
    //query function...
    void insertFunction(std::string functionName, int startLineNumber);
    int getFunctionDBId(std::string functionName);
    int getHLStatementDBId(int line_number, int column_number, std::string file_name);
    void insertIRInstruction(Instruction* instruction, Function* f, int instructionCount, int functionId, int HLStatementId);
    void insertHLStatement(int line_number, int column_number, std::string file_name);
    int insertState(int number, int belongingFunctionId, int functionCallId, std::string stateName);
    void insertVarInitialData(int type, int order_num, std::string value, int varId);
    void insertVariableDebugType(DebugType* debugType);
    void handleDebugType(DebugType* debugType);
    void insertTypeElementRelation(int parentTypeId, int elementTypeId);
    void insertVariableData(int varId, std::vector<std::string> initial);
    void insertVariable(int functionId, std::string name, std::string IRLabel, bool isGlobal, std::string initialValue, bool isArrayType);
    void insertVariable(std::string name, int functionId, std::string tag, int tagNum, std::string tagAddressName, int addressWidth,
        std::string mifFileName, int dataWidth, int numElements, bool isStruct, int IRId, int debugTypeId);
    void insertStateStoreInfo(int stateId, int signalId, std::string port, int offset, int IRId);
    int getVariableIdByIRNameAndFunctionId(int functionId, std::string name);
    int getVariableIdByTag(std::string tag);
    int getVariableIdByTagAddress(std::string tagAddress);
    int getStateIdByNameAndBelongingFunctionId(std::string stateName, int belongingFunctionId);
    /*int getStateIdByName(std::string stateName);*/
    int getIRInstructionDBId(std::string instructionString, int functionId);
    int getSignalIdByName(std::string name, int functionId);
    //void insertSignal(std::string name, int width, int functionId);
    int findSignalIdBySignalValue(RTLSignal* sig);
    int insertSignal(std::string name, int width, int functionId, bool isConst, int variable_id = -1);
    bool signalIsInserted(std::string name, int width, int functionId, bool isConst, int variable_id);
    void insertInstructionSignalMapping(const RTLSignal *signal, const Instruction *I);
    
    void insertIRStateMapping(int instructionId, int startStateId, int endStateId);
    void StateStoreInfoMapping(Allocation* allocation);
    
    void insertHardwareInformation(int IRId, std::string info);
        
private:
    DenseMap<MDNode*, unsigned> _mdnMap; //Map for MDNodes.
    unsigned _mdnNext;
    typedef DenseMap<MDNode*, unsigned>::iterator mdn_iterator;
    typedef std::map<std::pair<int, int> , std::vector<int> > hll_to_ir_map;
    typedef std::map<int , std::pair<int, int> > ir_to_hll_map;
    void handleSignal(const RTLSignal *signal, GenerateRTL* HW, Allocation* alloc);
    void handleCurStateFinishSignal(RTLSignal* signal, GenerateRTL* HW, Allocation *alloc);
    
    bool ignoreInstruction(const Instruction *I);
    
    std::map<std::string, hll_to_ir_map> HLL_to_IR_mapping;
    std::map<std::string, ir_to_hll_map> IR_to_HLL_mapping;
    std::map<const Function*, int> functionsToIds;
    std::map<State*, int> statesToIds;
    std::map<const Instruction*, int> IRInstructionsToIds;
    std::map<const RTLSignal*, int> signalsToIds;
    std::map<std::pair<int, std::string>, int> variablesToIds;//pair of functionId and variable name is mapped to variable id...
    std::map<std::pair<int, int>, int> stateVariableInfoToIds;//paiar of stateId and VariableId is mapped to stateVariableIDs...
    std::map<DebugType*, int> debugTypesToIds;
    
    //MySQL_variables
    MYSQL_RES *result;
    MYSQL_ROW row;
    MYSQL *connection, mysql;
    int state;

};

}

#endif	/* DEBUGGING_H */
