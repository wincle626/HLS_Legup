/* 
* File:   Debugging.cpp
* Author: legup
* 
* Created on May 29, 2013, 3:38 PM
*/

#include "Ram.h"
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
#include <bits/stl_map.h>
#include "llvm/IR/DerivedTypes.h"

#include "Debugging.h"
#include "llvm/ADT/StringMap.h"
#include "DebugType.h"

using namespace llvm;
using namespace legup;

namespace legup
{

Debugging::Debugging() {
}

Debugging::Debugging(const Debugging& orig) {
}

Debugging::~Debugging() {
}

const char* BoolToString(bool b)
{
  return b ? "true" : "false";
}

void Debugging::insertVarInitialData(int type, int order_num, std::string value, int varId) {
    std::string query = "INSERT into VariableData(type, order_num, value, Variable_id) values(";
    query += IntToString(type) + ", ";
    query += IntToString(order_num) + ", '";
    query += value + "', ";
    query += IntToString(varId) + ");";
    runQuery(query);
}

void Debugging::insertVariable(std::string name, int functionId, std::string tag, int tagNum, std::string tagAddressName, int addressWidth,
        std::string mifFileName, int dataWidth, int numElements, bool isStruct, int IRId, int debugTypeId) {
    std::string query = "INSERT into Variable(name, Function_id, tag, tagNum, tagAddressName, addressWidth, mifFileName, dataWidth, numElements, isStruct, IRInstr_id, debugTypeId) values(";
    query += "'" + name + "', ";
    query += IntToString(functionId) + ", ";
    query += "'" + tag + "', ";
    query += IntToString(tagNum) + ", ";
    query += "'" + tagAddressName + "', ";
    query += IntToString(addressWidth) + ", ";
    query += "'" + mifFileName + "', ";
    query += IntToString(dataWidth) + ", ";
    query += IntToString(numElements) + ", ";   
    if (isStruct)
        query += "true";
    else
        query += "false";
    query += ", ";    
    if (IRId == -1)
        query += "NULL";
    else        
        query += IntToString(IRId);
    query += ", ";
    query += IntToString(debugTypeId);
    query += ");";
    
    runQuery(query);
}

void Debugging::insertVariable(int functionId, std::string name, std::string IRLabel, bool isGlobal, std::string initialValue, bool isArrayType)
{
   std::string query = "INSERT into Variable(name, Function_id, IRLabel, isGlobal, initialValue, isArrayType) values(";
   query += "'" + name + "', ";
   query += IntToString(functionId)+ ", ";
   query += "'" + IRLabel + "', ";
   query += BoolToString(isGlobal);
   query += ", ";
   query += "'" + initialValue + "'";   
   if (isArrayType)
       query += ", true";
   else
       query += ", false";   
   query += ");";
       
   runQuery(query);
}

void Debugging::createFunctionLineNumberTable(Function* F)
{                
    hll_to_ir_map hll_to_ir;    
    ir_to_hll_map ir_to_hll;    
        
    int instr_count = 0;
    for (Function::iterator b = F->begin(), be = F->end(); b != be; b++) {
            for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {

                instr_count++;
                //i->dump();                
                Instruction *I = i;
                //if (isa<BranchInst>(I) && !cast<BranchInst>(I)->isConditional())
                    //continue;                                   
                            
                //filtering llvm.dbg.declare and llvm.dbg.value call instructions...
                if (const CallInst *CI = dyn_cast<CallInst>(I)) {
                   Function *calledFunc = CI->getCalledFunction();
                   /*if (!calledFunc) {
                       std::cout << "oooooops function was null!!!" << std::endl;
                       I->dump();
                        continue;
                   }*/
                   std::string funcName = "";
                   if (calledFunc)
                       funcName = calledFunc->getName();
                   if (funcName == "llvm.dbg.declare" || funcName == "llvm.dbg.value")
                       continue;
                }
                    
                if (i->hasMetadata())
                {                
                    //i->dump();
                    MDNode* n = i->getMetadata("dbg");
                    DILocation loc(n);
                    int line_number = loc.getLineNumber();
                    int column_number = loc.getColumnNumber();
                    //std::cout << "line number: " << line_number << std::endl;
                    //std::cout << "column number: " << column_number << std::endl;
                    std::string file_name = loc.getFilename().operator  std::string();
                    std::pair<int, int> LCPair = std::make_pair(line_number, column_number);
                    if (hll_to_ir.find(LCPair) == hll_to_ir.end())
                    {
                        insertHLStatement(line_number, column_number, file_name);
                    }
                    
                    int HLStatementId = getHLStatementDBId(line_number, column_number, file_name);
                    insertIRInstruction(i, F, instr_count, functionsToIds[F], HLStatementId);
                    int instrId = getIRInstructionDBId(getValueStr(i), functionsToIds[F]);
                    IRInstructionsToIds[i] = instrId;
                            
                    
                    hll_to_ir[LCPair].push_back(instr_count);
                    ir_to_hll[instr_count] = LCPair;
                }
                else
                {
                    insertIRInstruction(i, F, instr_count, functionsToIds[F], -1);
                    int instrId = getIRInstructionDBId(getValueStr(i), functionsToIds[F]);                    
                    IRInstructionsToIds[i] = instrId;
                }
            }
    }
    HLL_to_IR_mapping[F->getName().str()] = hll_to_ir;
    IR_to_HLL_mapping[F->getName().str()] = ir_to_hll;
}

void Debugging::insertHardwareInformation(int IRId, std::string info)
{
    if (info.size() == 0)
        return;
    std::string newInfo = "";
    for (unsigned i = 0 ; i < info.size(); i++)
    {        
        if (info[i] != '\'')
            newInfo += info[i];
        else
        {
            newInfo += "\\";
            newInfo += "'";
        }                    
    }
    std::string query = "INSERT into HardwareInfo(IRid, info) values(";
    query += IntToString(IRId);
    query += ", '";
    query += newInfo;
    query += "');";
    runQuery(query);
    
}

void Debugging::insertFunction(std::string functionName, int startLineNumber)
{
    std::string query = "INSERT into Function(name, startLineNumber) ";
    query += "values('";
    query += functionName;
    query += "', ";
    query += IntToString(startLineNumber);
    query += ");";
    runQuery(query);
}

int Debugging::getFunctionDBId(std::string functionName)
{
    std::string query = "SELECT id from Function where name = '";
    query += functionName;
    query += "' limit 1;";
    runQuery(query);
    MYSQL_ROW row;    
    if ((row = mysql_fetch_row(result)) != NULL)
    {
        return atoi(row[0]);
    }
    return -1;
}

int Debugging::getHLStatementDBId(int line_number, int column_number, std::string file_name)
{
    std::string query = "SELECT id from HLStatement where line_number = ";
    query += IntToString(line_number);
    query += " AND column_number = ";
    query += IntToString(column_number);    
    query += " AND file_name = '";
    query += file_name;
    query += "' limit 1;";
    runQuery(query);
    MYSQL_ROW row;    
    if ((row = mysql_fetch_row(result)) != NULL)
    {
        return atoi(row[0]);
    }
    return -1;
}

int Debugging::getIRInstructionDBId(std::string instructionString, int functionId)
{
    std::string query = "SELECT id from IRInstr where dump = '";
    query += instructionString;
    query += "' AND function_id = ";
    query += IntToString(functionId);
    query += ";";
    runQuery(query);
    MYSQL_ROW row;    
    if ((row = mysql_fetch_row(result)) != NULL)
    {
        return atoi(row[0]);
    }
    std::cout << "id returned -1: ir string: " << instructionString << std::endl;
    return -1;
}

void Debugging::insertIRInstruction(Instruction* instruction, Function* f, int instructionCount, int functionId, int HLStatementId)
{
    std::string query = "INSERT into IRInstr(number_in_function, function_id, HLStatement_id, dump)";
    query += "values(";
    query += IntToString(instructionCount);
    query += ", ";
    query += IntToString(functionId);
    query += ", ";    
    if (HLStatementId == -1)
        query += "NULL";
    else
        query += IntToString(HLStatementId);
    query += ", '";
    query += getValueStr(instruction);
    query += "');";
        
    runQuery(query);
}

void Debugging::insertHLStatement(int line_number, int column_number, std::string file_name)
{
    std::string query = "INSERT into HLStatement(line_number, column_number, file_name) values(";
    query += IntToString(line_number);        
    query += ", ";
    query += IntToString(column_number);
    query += ", '";    
    query += file_name;
    query += "');";    
    runQuery(query);
}

int Debugging::insertState(int number, int belongingFunctionId, int functionCallId, std::string stateName)
{
    std::string query = "INSERT into State(belongingFunctionId, calledFunctionId, number, design_name) values(";
    query += IntToString(belongingFunctionId);
    query += ", ";
    if (functionCallId == -1)
        query += "NULL, ";
    else {
        query += IntToString(functionCallId);
        query += ", ";
    }
    query += IntToString(number);
    query += ", '";
    query += stateName;
    query += "');";
    runQuery(query);
    return mysql.insert_id;
}

int Debugging::getStateIdByNameAndBelongingFunctionId(std::string stateName, int belongingFunctionId) {
    std::string query = "SELECT id from State where design_name = '";
    query += stateName;
    query += "' ";
    query += "AND belongingFunctionId = ";
    query += IntToString(belongingFunctionId);
    query += ";";
    runQuery(query);
    MYSQL_ROW row;    
    if ((row = mysql_fetch_row(result)) != NULL)
    {
        return atoi(row[0]);
    }
    return -1;
}

/*int Debugging::getStateIdByName(std::string stateName)
{
    std::string query = "SELECT id from State where design_name = '";
    query += stateName;
    query += "';";
    runQuery(query);
    MYSQL_ROW row;    
    if ((row = mysql_fetch_row(result)) != NULL)
    {
        return atoi(row[0]);
    }
    return -1;
}*/

void Debugging::insertIRStateMapping(int instructionId, int startStateId, int endStateId)
{
    //this check should be handled when the function being called actually, not here...
    if (startStateId == 0 || endStateId == 0)
        return;
    std::string query = "INSERT INTO IRState(IRInstr_Id, StartStateId, EndStateId) values(";
    query += IntToString(instructionId);
    query += ", ";
    query += IntToString(startStateId);
    query += ", ";
    query += IntToString(endStateId);
    query += ");";
    runQuery(query);
}

void Debugging::mapIRsToStates(GenerateRTL* HW)
{    
    FiniteStateMachine *fsm = HW->getFSM();
    std::map<State*, int> statesToNumbers;
    std::vector<State*> callStates;
    
    int number = 0;
    for (FiniteStateMachine::iterator state = fsm->begin(), se = fsm->end();
            state != se; ++state) {        
        std::string stateName = state->getName();
        Function* calledFunc = NULL;
        if (stateName.find("LEGUP_function_call") != string::npos) {            
            Instruction* I = (*state->begin());
            if (CallInst *CI = dyn_cast<CallInst>(I)) {                
                //Function *f = CI->getCalledFunction();
                Function *f = getCalledFunction(CI);
                   /*if (!f) {
                       continue;
                   }*/
                      
                   /*if (f)
                        std::cout << "called function: " << std::string(f->getName()) << std::endl;
                   else {                       
                       std::cout << "called function: NULL" << std::endl;
                   }*/
                   calledFunc = f;
            }
        }
        int functionCallId = -1;
        if (calledFunc == NULL ||
                calledFunc->getName().str() == "llvm.dbg.declare" || 
                calledFunc->getName().str() == "llvm.dbg.value")
            functionCallId = -1;
        else
        {
            std::string fname(calledFunc->getName());
            
            //I don't know why I was filtering the legup functions calls here.. I will comment these lines and allow the code to redirect to functions such as legup_memset and legup_memcpy
            //and then if I don't want to go inside them I have to handle this in the Debugger Engine not here.
            functionCallId = functionsToIds[calledFunc];
            /*if (fname.find("legup_") != string::npos)
                functionCallId = -1;
            else
                functionCallId = functionsToIds[calledFunc];*/
        }        
            
        /*if (!isCallState) {
            insertState(number, functionsToIds[HW->getFunction()], functionCallId, stateName);
            number++;
        }
        else {
            insertState(callStateNumber, functionsToIds[HW->getFunction()], functionCallId, stateName);
            callStateNumber++;
        }*/
        int stateId = insertState(number, functionsToIds[HW->getFunction()], functionCallId, stateName);
        number++;
        //int stateId = getStateIdByNameAndBelongingFunctionId(stateName, functionsToIds[HW->getFunction()]);
        statesToIds[state] = stateId;
        
    }
    
    Function* F = HW->getFunction();
    for (Function::iterator b = F->begin(), be = F->end(); b != be; b++) {
            for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {
                Instruction *I = i;
                
                if (b && I == b->getTerminator())
                {
                    for (FiniteStateMachine::iterator state = fsm->begin(), se = fsm->end();state != se; ++state)
                    {
                        if (state->isTerminating()){
                            BasicBlock* bb = state->getBasicBlock();
                            if (bb == b)
                            {
                                if (IRInstructionsToIds.find(I) != IRInstructionsToIds.end())
                                //if (IRInstructionsToIds[I] != 0)
                                {                                    
                                    insertIRStateMapping(IRInstructionsToIds[I], statesToIds[state] , statesToIds[state]);
                                    break;
                                }
                            }
                        }
                    }
                    continue;
                }
                
                //if (I == b->getTerminator())
                    //continue;
                //I->dump();
                int startStateId = statesToIds[fsm->getStartState(I)];
                int endStateId;
                if (fsm->EndStateExists(I))
                    endStateId = statesToIds[fsm->getEndState(I)];
                else
                    endStateId = startStateId;
                //std::cout << "IRID: " << IRInstructionsToIds[I] << std::endl;
                //std::cout << "StartStateID: " << startStateId << std::endl;
                //std::cout << "EndStateID: " << endStateId << std::endl;                
                //if (IRInstructionsToIds.find(I) == IRInstructionsToIds.end())
                //    continue;
                //if (IRInstructionsToIds[I] == 0)
                    //continue;
                if (IRInstructionsToIds.find(I) != IRInstructionsToIds.end()) {                    
                    insertIRStateMapping(IRInstructionsToIds[I], startStateId, endStateId);
                }
            }
    }
    
    
    /*for (FiniteStateMachine::iterator state = fsm->begin(), se = fsm->end();
            state != se; ++state) {        
        for (State::iterator instr = state->begin(), ie = state->end(); instr
                != ie; ++instr) {
            Instruction* I = *instr;      
            insertIRStateMapping(IRInstructionsToIds[I], statesToIds[fsm->getStartState(I)], statesToIds[fsm->getEndState(I)]);
        }
    }*/
    
    /*
    formatted_raw_ostream out(Scheduler::alloc->getDebuggingFile());
    std::map<BasicBlock*, unsigned> bbCount;
    out << "Start Function: " << HW->getFunction()->getName() << "\n";
    for (FiniteStateMachine::iterator state = fsm->begin(), se = fsm->end();
            state != se; ++state) {
        out << "state: " << state->getName() << "\n";
        BasicBlock *bb = state->getBasicBlock();
        bbCount[bb]++;
        for (State::iterator instr = state->begin(), ie = state->end(); instr
                != ie; ++instr) {
            Instruction *I = *instr;            
            if (bb && I == bb->getTerminator()) continue;
            assert(fsm->getStartState(I) == state);
            out << " " << getValueStr(I) << " (endState: " <<
                fsm->getEndState(I)->getName() << ")\n";
        }
        
        if (bb && state->isTerminating()) {
            out << " " << getValueStr(bb->getTerminator()) << "\n";
        }fil
        out << "   ";
        state->printTransition(out);
        out << "\n";
    }
    out << "\n";
    for(std::map<BasicBlock*, unsigned>::iterator i = bbCount.begin(), e =
            bbCount.end(); i != e; ++i) {
        BasicBlock *bb = i->first;
        if (!bb) continue;
        unsigned count = i->second;
        out << "Basic Block: " << getLabel(bb) << " Num States: " << count << "\n";
    }
    out << "End Function: " << HW->getFunction()->getName() << "\n";
    out << "--------------------------------------------------------------------------------\n\n";*/
}

int Debugging::getFunctionStartLineNumber(Function* F) {
    DebugInfoFinder Finder;
    Finder.processModule(*F->getParent());
    //for (DebugInfoFinder::const_iterator i = Finder.subprogram_begin(), e = Finder.subprogram_end(); i != e; ++i) {
    for (auto i = Finder.subprograms().begin(), e = Finder.subprograms().end(); i != e; ++i) {
        DISubprogram S(*i);
        if (S.getFunction() == F)
            return S.getLineNumber();        
    }
    return -1;
}

void Debugging::fillDebugDB(Function* F)
{
    int startLineNumber = getFunctionStartLineNumber(F);
    insertFunction(F->getName().str(), startLineNumber);
    
    int function_id = getFunctionDBId(F->getName().str());
    
    functionsToIds[F] = function_id;
    
    createFunctionLineNumberTable(F);
}

bool Debugging::ignoreInstruction(const Instruction *I) {
   // ignore stores (don't need a wire/reg)
   if (I->getType()->getTypeID() == Type::VoidTyID) return true;
   // ignore allocations
   if (isa<AllocaInst>(I)) return true;
   // ignore printf calls
   if (isaDummyCall(I)) return true;
   return false;
}

void Debugging::insertVariableData(int varId, std::vector<std::string> initial) {
    if (initial.size() == 0) {
        insertVarInitialData(0, 0, "N/A", varId);
        return;
    }        
    for (unsigned i = 0; i < initial.size(); i++) {
        std::string val = initial[i];        
        insertVarInitialData(0, i, val, varId);
    }
}

void Debugging::insertVariableDebugType(DebugType* debugType) {
    std::string query = "INSERT into VariableType(typeId, numElements, byteSize) values(";
    query += IntToString(debugType->type) + ", ";
    query += IntToString(debugType->numElements) + ", ";
    query += IntToString(debugType->byteSize) + ");";    
    runQuery(query);
    //std::cout << "debug type inserted id is: " << mysql.insert_id << std::endl;
    debugTypesToIds[debugType] = mysql.insert_id;
}

void Debugging::insertTypeElementRelation(int parentTypeId, int elementTypeId) {
    std::string query = "INSERT into TypeElement(parentTypeId, ElementTypeId) values(";
    query += IntToString(parentTypeId) + ", ";
    query += IntToString(elementTypeId) + ");";
    runQuery(query);
}

void Debugging::handleDebugType(DebugType* debugType) {
    insertVariableDebugType(debugType);
    for (unsigned i = 0; i < debugType->elementTypes.size(); i++) {
        //insertVariableDebugType(debugType->elementTypes[i]);
        handleDebugType(debugType->elementTypes[i]);
        insertTypeElementRelation(debugTypesToIds[debugType], debugTypesToIds[debugType->elementTypes[i]]);
    }
}

bool Debugging::checkIfVarIsPassedToAnotherFunction(Instruction* instr, std::string &varName, int &functionId) {
    Function* F = instr->getParent()->getParent();
    Function* destFunc = NULL;
    int argNumber = -1;
        bool found = false;
        for (Function::iterator b = F->begin(), be = F->end(); b != be; b++) {
                for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {
                        Instruction *I = i;
                        if (const CallInst *CI = dyn_cast<CallInst>(I)) {                            
                            for (unsigned op = 0; op < CI->getNumArgOperands(); op++) {
                                if (CI->getArgOperand(op) == instr) {
                                    //functionId = functionsToIds[CI->getCalledFunction()];
                                    destFunc = CI->getCalledFunction();
                                    argNumber = op;
                                    found = true;
                                    break;
                                }                                                 
                            }
                        }
                        if (found)
                            break;
                }
                if (found)
                    break;
        }
        if (found) {
            functionId = functionsToIds[destFunc];
            llvm::Function::arg_iterator it = destFunc->arg_begin();
            for (int i = 0; i < argNumber; i++)
                it++;
            varName = (*it).getName().str();
            //std::cout << "FOUND!!!! name: " << varName << " func: " << destFunc->getName().str() << std::endl;
            return true;
        }
        return false;
}

void Debugging::fillVariables(Allocation *allocation) {
    for (Allocation::ram_iterator i = allocation->ram_begin(),
                                  e = allocation->ram_end();
         i != e; ++i) {
        RAM *r = *i;
        const Value *val = r->getValue();
        // const Instruction *I = dyn_cast<Instruction>(val);
        // assert(I);
        // const Function* F = I->getParent()->getParent();
        // assert(F);

        int functionId = 1;

        // std::cout << "value is : "<< I->getName().str()<<std::endl;
        // I->dump();
        std::string varName;
        if (r->getValue()->hasName())
            varName = r->getValue()->getName().str();
        else {

            // bool found = checkIfVarIsPassedToAnotherFunction(I, varName,
            // functionId);
            bool found = false;
            if (!found) {
                int slotNum = r->getValue()->getFunctionLocalSlotNumber();
                varName = "%" + IntToString(slotNum);
            }

            if (false) {

                int slotNum = r->getValue()->getFunctionLocalSlotNumber();

                Function *F = ((Instruction *)(val))->getParent()->getParent();
                std::cout << "F: " << F->getName().str() << std::endl;
                llvm::Function::arg_iterator it = F->arg_begin();
                bool argFound = true;
                for (int s = 1; s < slotNum; s++) {
                    if (it == F->arg_end()) {
                        argFound = false;
                        break;
                    }
                            it++;
                        }   
                        if (F->arg_begin() == F->arg_end())
                            argFound = false;
                        if (argFound)
                            varName = (*it).getName().str();
                        else
                            varName = "%" + IntToString(slotNum);
                        }
                    }
                    std::string tag = r->getTag();
                    int tagNum = allocation->getRamTagNum(r);
                    std::string tagAddressName = r->getTagAddrName();
                    int addressWidth = r->getAddrWidth();
                    std::string mifFileName = r->getMifFileName();
                    int dataWidth = r->getDataWidth();
                    int numElements = r->getElements();
                    bool isStruct = r->isStruct();
                    int IRId = -1;
                    if (IRInstructionsToIds.find(
                            (Instruction *)r->getValue()) !=
                        IRInstructionsToIds.end())
                        IRId =
                            IRInstructionsToIds[(Instruction *)r->getValue()];

                    Function *F = ((Instruction *)r->getValue())
                                      ->getParent()
                                      ->getParent();
                    if (functionId == 1) {
                        if (functionsToIds.find(F) != functionsToIds.end())
                            functionId = functionsToIds[F];
                    }
                    
                    handleDebugType(r->debugType);
                    
                    int debugTypeId = debugTypesToIds[r->debugType];
                    
                    insertVariable(varName, functionId, tag, tagNum, tagAddressName, addressWidth, mifFileName, dataWidth, numElements, isStruct, IRId, debugTypeId);
                    int varId = getVariableIdByTag(tag);
                    variablesToIds[std::make_pair(functionId, varName)] = varId;
                    
                    //std::cout << "ram int initial size: " << r->getInitial().size() << std::endl;
                    //std::cout << "ram float initial size: " << r->getFPInitial().size() << std::endl;
                    insertVariableData(varId, r->getDebugInitialValues());
    }
    return;
    //std::cout << "***** FILL VARIALBES *******" << std::endl;
    for (Allocation::hw_iterator h = allocation->hw_begin(), he =
           allocation->hw_end(); h != he; ++h) {
        GenerateRTL *HW = *h;
        llvm::Function *F = HW->getFunction();
        int functionId = functionsToIds[F];
        for (Function::iterator b = F->begin(), be = F->end(); b != be; b++) {
           for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {
               Instruction *I = i;
               if (const CallInst *CI = dyn_cast<CallInst>(I)) {
                   Function *calledFunc = CI->getCalledFunction();
                   if (!calledFunc)
                        continue;
                   std::string funcName = calledFunc->getName();
                   if (funcName != "llvm.dbg.declare")
                       continue;
                                      
                   bool isArrayType = false;
                  std::string IRValueName = "";
                  MDNode *N = dyn_cast_or_null<MDNode>(I->getOperand(0));
                   if(N != NULL){          
                       Value* DVTemp = N->getOperand(0);                       
                       
                       if (!DVTemp->hasName())
                       {
                           int slotNum = DVTemp->getFunctionLocalSlotNumber();
                           //IR value of nth slotNum is the name of function's Nth argument
                           llvm::Function::arg_iterator it = F->arg_begin();
                           for (int i = 1 ; i < slotNum; i++)
                               it++;
                           IRValueName = (*it).getName().str();
                       }
                       else
                           IRValueName = DVTemp->getName().str();                       
                       if (((PointerType*)DVTemp->getType())->getElementType()->getTypeID() == Type::ArrayTyID)//need to handle structs as well...
                           isArrayType = true;
                   }                   
                   
                   Value *v = CI->getOperand(1);//should have C variable name....
                   MDNode *mdnode = (MDNode*)v;
                   DIVariable DV(mdnode);
                   /*DIDescriptor descriptor(mdnode);
                   if (descriptor.isLexicalBlock())
                   {
                       ((DILexicalBlock)descriptor).
                   }*/                   
                   std::string varname = DV.getName().data();
                   
                   insertVariable(functionId, varname, IRValueName, false, "", isArrayType);
                   int varId = getVariableIdByIRNameAndFunctionId(functionId, IRValueName);
                   variablesToIds[std::make_pair(functionId, IRValueName)] = varId;
               }
           }
        }                
    }    
}

void Debugging::StateStoreInfoMapping(Allocation* allocation) {
    for (Allocation::hw_iterator h = allocation->hw_begin(), he =
           allocation->hw_end(); h != he; ++h) {
        GenerateRTL *HW = *h;
        Function *F = HW->getFunction();        
        for (unsigned i = 0 ; i < HW->statesStoreMapping.size(); i++) {
            //int functionId = functionsToIds[F];
            int stateId = statesToIds[HW->statesStoreMapping[i]->state];
            std::string memPort = HW->statesStoreMapping[i]->port;
            
            RTLSignal* sig = HW->statesStoreMapping[i]->addressSignal;
            
            int signalId;
            int sigWidth = sig->getWidth().numBits(HW->getRTL(), allocation);
            std::string sigName;
            bool isConst = sig->isConst();
            bool isOp = sig->isOp();
            int variableId;
            
            if (isOp) {                
                RTLOp* op = (RTLOp*) sig;
                sigName = op->getOperand(0)->getValue();
                variableId = getVariableIdByTagAddress(op->getOperand(0)->getValue().substr(1));
                if (!signalIsInserted(sigName, sigWidth, functionsToIds[F], isConst, variableId)) {
                    signalId = insertSignal(sigName, sigWidth, functionsToIds[F], isConst, variableId);
                } else {
                    signalId = getSignalIdByName(sigName, functionsToIds[F]);
                }
            } else if (isConst) {
                //std::cout << "const signal: value: " <<  sig->getValue() << " name: " << sig->getName() << std::endl;
                                
                sigName = sig->getValue();                                
                variableId = getVariableIdByTagAddress(sigName.substr(1));
                if (!signalIsInserted(sigName, sigWidth, functionsToIds[F], isConst, variableId)) {
                    signalId = insertSignal(sigName, sigWidth, functionsToIds[F], isConst, variableId);
                } else {
                    signalId = getSignalIdByName(sigName, functionsToIds[F]);
                }
            } else {
                //std::cout << "SIG IS NOT CONST********" << std::endl;
                if (signalsToIds.find(sig) == signalsToIds.end()) {                    
                    sigName = sig->getName();                    
                    int sid = insertSignal(sigName, sigWidth, functionsToIds[F], isConst);
                    signalsToIds[sig] = sid;
                }
                else {                    
                    //std::cout << "sig name: " << sig->getName() << std::endl;
                }
                signalId = signalsToIds[sig];
            }
                        
            int IRId = IRInstructionsToIds[HW->statesStoreMapping[i]->IR];
            insertStateStoreInfo(stateId, signalId, memPort, HW->statesStoreMapping[i]->adrOffsetValue, IRId);
         
        }
    }
}

void Debugging::insertStateStoreInfo(int stateId, int signalId, std::string port, int offset, int IRId) {
    std::string query = "INSERT into StateStoreInfo(state_id, HWSignal_id, port, offset, IRInstr_id) values(";
    query += IntToString(stateId);
    query += ", ";
    query += IntToString(signalId);
    query += ", '" + port + "', ";
    if (offset != -1)
        query += IntToString(offset);
    else
        query += "NULL";
    query += ", " + IntToString(IRId);
    query += ");";
    runQuery(query);
}

int Debugging::getVariableIdByTagAddress(std::string tagAddress) {
    std::string query = "SELECT id from Variable where tagAddressName = '" + tagAddress + "' ";   
   runQuery(query);
   MYSQL_ROW row;
   if ((row = mysql_fetch_row(result)) != NULL)
   {
       return atoi(row[0]);
   }
   return -1;
}

int Debugging::getVariableIdByTag(std::string tag) {
   std::string query = "SELECT id from Variable where tag = '" + tag + "' ";   
   runQuery(query);
   MYSQL_ROW row;
   if ((row = mysql_fetch_row(result)) != NULL)
   {
       return atoi(row[0]);
   }
   return -1;
}

int Debugging::getVariableIdByIRNameAndFunctionId(int functionId, std::string name)
{
   std::string query = "SELECT id from Variable where IRLabel = '" + name + "' ";
   query += "AND Function_id = ";
   query += IntToString(functionId);
   query += ";";
   runQuery(query);
   MYSQL_ROW row;
   if ((row = mysql_fetch_row(result)) != NULL)
   {
       return atoi(row[0]);
   }
   return -1;
}

void Debugging::insertInstructionSignalMapping(const RTLSignal *signal, const Instruction *I)
{
    std::string query = "INSERT into InstructionSignal(HWSignal_id, IRInstr_id) values(";
    query += IntToString(signalsToIds[signal]);
    query += ", ";
    query += IntToString(IRInstructionsToIds[I]);
    query += ");";
    runQuery(query);
}

void Debugging::handleCurStateFinishSignal(RTLSignal* signal, GenerateRTL* HW, Allocation *alloc)
{
    Function *F = HW->getFunction();
    if (signalsToIds.find(signal) == signalsToIds.end())
    {               
        int sigWidth = signal->getWidth().numBits(HW->getRTL(), alloc);
        bool isConst = signal->isConst();
        int sigId = insertSignal(signal->getName(), sigWidth, functionsToIds[F], isConst);        
        signalsToIds[signal] = sigId;                
    }
}

void Debugging::handleSignal(const RTLSignal* signal, GenerateRTL* HW, Allocation *alloc)
{
    Function *F = HW->getFunction();
    if (signalsToIds.find(signal) == signalsToIds.end())
    {               
        int sigWidth = signal->getWidth().numBits(HW->getRTL(), alloc);
        bool isConst = signal->isConst();
        int sigId = insertSignal(signal->getName(), sigWidth, functionsToIds[F], isConst);        
        signalsToIds[signal] = sigId;
        
        int conditions = signal->getNumConditions();                
        
        if (conditions == 0)//only a single driver...
        {
            const Instruction* instr = signal->getInstPtr(0);
            if (instr != NULL)
                insertInstructionSignalMapping(signal, instr);
        }
        else
        {
            for (int i = 0 ; i < conditions; i++)
            {
                const Instruction *instr = signal->getInst(i);
                if (IRInstructionsToIds.find(instr) != IRInstructionsToIds.end())
                    insertInstructionSignalMapping(signal, instr);
            }
        }
    }
}

void Debugging::fillCurStateAndFinishSignals(Allocation *allocation) {
    for (Allocation::hw_iterator i = allocation->hw_begin(), ie =
            allocation->hw_end(); i != ie; ++i) {
        GenerateRTL *HW = *i;
                
        for (RTLModule::const_signal_iterator sig = HW->getRTL()->signals_begin(), e =
                HW->getRTL()->signals_end(); sig != e; ++sig) {   
            if ((*sig)->getName() == "cur_state" ||
                    (*sig)->getName() == "finish")
                        handleCurStateFinishSignal((*sig), HW, allocation);
        }
        
        for (RTLModule::const_signal_iterator sig = HW->getRTL()->param_begin(), e =
                HW->getRTL()->param_end(); sig != e; ++sig) {
            if ((*sig)->getName() == "cur_state" ||
                    (*sig)->getName() == "finish")
                        handleCurStateFinishSignal((*sig), HW, allocation);
        }
                    
        for (RTLModule::const_signal_iterator sig = HW->getRTL()->port_begin(), e =
                HW->getRTL()->port_end(); sig != e; ++sig) {
            if ((*sig)->getName() == "cur_state" ||
                    (*sig)->getName() == "finish")
                        handleCurStateFinishSignal((*sig), HW, allocation);
        }
   }
}

void Debugging::fillSignals(Allocation *allocation)
{
    for (Allocation::hw_iterator i = allocation->hw_begin(), ie =
            allocation->hw_end(); i != ie; ++i) {
        GenerateRTL *HW = *i;
                
        for (RTLModule::const_signal_iterator sig = HW->getRTL()->signals_begin(), e =
                HW->getRTL()->signals_end(); sig != e; ++sig) {                        
            handleSignal((*sig), HW, allocation);
        }
        
        for (RTLModule::const_signal_iterator sig = HW->getRTL()->param_begin(), e =
                HW->getRTL()->param_end(); sig != e; ++sig) {
            handleSignal((*sig), HW, allocation);
        }
                    
        for (RTLModule::const_signal_iterator sig = HW->getRTL()->port_begin(), e =
                HW->getRTL()->port_end(); sig != e; ++sig) {
            handleSignal((*sig), HW, allocation);
        }
                
        RTLSignal* unsynSig = HW->getRTL()->getUnsynthesizableSignal();
        for (unsigned cond = 0 ; cond < unsynSig->getNumConditions(); cond++)
        {            
            RTLSignal* driver = unsynSig->getDriver(cond);
            const RTLOp* driverOp = (const RTLOp*) driver;                        
            int numOperands = ((const RTLOp*) driver)->getNumOperands();
            for (int o = 0 ; o < numOperands; o++)
            {                
                const RTLSignal* operand = driverOp->getOperand(o);
                if (signalsToIds.find(operand) == signalsToIds.end())
                    continue;                
                
                handleSignal(operand, HW, allocation);
                insertInstructionSignalMapping(operand, unsynSig->getInst(0));
            }
        }
    }
}

int Debugging::getSignalIdByName(std::string name, int functionId)
{
    std::string query = "SELECT id from HWSignal where name = '" + name + "' ";
    query += "AND function_id = ";
    query += IntToString(functionId);
    query += ";";
    runQuery(query);
    MYSQL_ROW row;
    if ((row = mysql_fetch_row(result)) != NULL)
    {
        return atoi(row[0]);
    }
    return -1;
}

bool Debugging::signalIsInserted(std::string name, int width, int functionId, bool isConst, int variable_id) {
    std::string query = "SELECT id from HWSignal where name = ";
    query += "'" + name + "' ";
    query += "AND width = "  + IntToString(width) + " ";
    query += "AND function_id = " + IntToString(functionId) + " ";
    query += "AND isConst = ";
    if (isConst)
        query += "true";
    else
        query += "false";
    query += " AND Variable_id = " + IntToString(variable_id) + ";";
    runQuery(query);
    MYSQL_ROW row;
    if ((row = mysql_fetch_row(result)) != NULL)
    {
        //return atoi(row[0]);
        return true;
    }
    return false;
}

int Debugging::insertSignal(std::string name, int width, int functionId, bool isConst, int variable_id)
{
    std::string query = "INSERT into HWSignal(name, width, function_id, isConst, Variable_id) values('" + name + "'";
    query += ", " + IntToString(width);
    query += ", " + IntToString(functionId);
    query += ", ";
    if (isConst)
        query += "true";
    else
        query += "false";
    query += ", ";
    if (variable_id == -1)
        query += "null";
    else
        query += IntToString(variable_id);
    query += ");";
    runQuery(query);
    return mysql.insert_id;
}

void Debugging::fillHardwareInfo(Allocation *allocation)
{    
    raw_fd_ostream S(1, false, true);
    VerilogWriter writer(S, allocation);
    //writer.setWritingToStringStream();
    
    for (Allocation::hw_iterator i = allocation->hw_begin(), ie =
            allocation->hw_end(); i != ie; ++i) {
        
        GenerateRTL *HW = *i;
        Function *F = HW->getFunction();
        
        for (Function::iterator b = F->begin(), be = F->end(); b != be; b++) {
                for (BasicBlock::iterator instr = b->begin(), ie = b->end(); instr != ie; instr++) {
                    
                    Instruction *I = instr;                    
                    
                    int IRid = IRInstructionsToIds[I];
                    
                    
                    writer.setRTL(HW->getRTL());
                    writer.clearStringStreamBuffer();
                    writer.printSignalConditionForInstruction(HW->getRTL()->getUnsynthesizableSignal(), I);
                    std::string str = writer.getStringStreamOut();
                    writer.clearStringStreamBuffer();
                    //I->dump();
                    insertHardwareInformation(IRid, str);                                    
                    
                    for (RTLModule::const_signal_iterator sig = HW->getRTL()->signals_begin(), e =
                HW->getRTL()->signals_end(); sig != e; ++sig) {
                        
                        writer.setRTL(HW->getRTL());
                        writer.clearStringStreamBuffer();                        
                        writer.printSignalConditionForInstruction((*sig), I);
                        std::string str = writer.getStringStreamOut();
                        writer.clearStringStreamBuffer();
                        //I->dump();
                        insertHardwareInformation(IRid, str);                                    
                    }
                    
                    for (RTLModule::const_signal_iterator sig = HW->getRTL()->param_begin(), e =
                HW->getRTL()->param_end(); sig != e; ++sig) {
                        writer.setRTL(HW->getRTL());
                        writer.clearStringStreamBuffer();                        
                        writer.printSignalConditionForInstruction((*sig), I);
                        std::string str = writer.getStringStreamOut();
                        writer.clearStringStreamBuffer();
                        //I->dump();
                        insertHardwareInformation(IRid, str);
                    }
                    
                    for (RTLModule::const_signal_iterator sig = HW->getRTL()->port_begin(), e =
                HW->getRTL()->port_end(); sig != e; ++sig) {
                        writer.setRTL(HW->getRTL());
                        writer.clearStringStreamBuffer();                        
                        writer.printSignalConditionForInstruction((*sig), I);
                        std::string str = writer.getStringStreamOut();
                        writer.clearStringStreamBuffer();
                        //I->dump();
                        insertHardwareInformation(IRid, str);
                    }
                }
        }
    }
}

void Debugging::initialize() { insertFunction("GLOBAL", -1); }

void Debugging::initializeDatabase() {
    mysql_init(&mysql);
    std::string host = LEGUP_CONFIG->getParameter("DEBUG_DB_HOST");
    std::string user = LEGUP_CONFIG->getParameter("DEBUG_DB_USER");
    std::string password = LEGUP_CONFIG->getParameter("DEBUG_DB_PASSWORD");
    std::string db = LEGUP_CONFIG->getParameter("INSPECT_DEBUG_DB_NAME");
    std::string inspectScriptFile =
        LEGUP_CONFIG->getParameter("INSPECT_DEBUG_DB_SCRIPT_FILE");

    connection = mysql_real_connect(&mysql, host.c_str(), user.c_str(),
                                    password.c_str(), db.c_str(), 3306, 0, 0);
    if (connection == NULL) {
        std::cout << "Inspect database is not found. A fresh database will be "
                     "installed..." << std::endl;
        std::string cmd = "mysql --user=" + user + " --password=" + password +
                          " <" + inspectScriptFile;
       int ret = system(cmd.c_str());
       if (ret) {
           report_fatal_error("Debug database script creation failed.");
       }
       connection =
           mysql_real_connect(&mysql, host.c_str(), user.c_str(),
                              password.c_str(), db.c_str(), 3306, 0, 0);
       if (connection == NULL) {
           std::cerr << "Inspect database creation is failed..." << std::endl;
           exit(1);
       }
       std::cout << "Inspect database is created..." << std::endl;
       //runQuery("read content from sql file and put it here....");
       //std::cerr << "SQL connection error! Make sure that Legup Inspect database is installed and the parameters are set correctly in legup.tcl file" << std::endl;
       //exit(1);
   }
   //mysql -Nse 'show tables' DATABASE_NAME | while read table; do mysql -e "truncate table $table" DATABASE_NAME; done
   //mysql_list_tables()
   
   std::string query = "SET FOREIGN_KEY_CHECKS = 0;";
   runQuery(query);
   query = "SHOW TABLES IN " + db + ";"; 
   runQuery(query);
   MYSQL_ROW row;
   MYSQL_RES *result2;
   
   result2 = result;
   
   while((row = mysql_fetch_row(result2)) != NULL)
   {
       query= "TRUNCATE TABLE ";
       query += row[0];
       query +=";";
       //std::cout<<"here1"<<endl;
       runQuery(query);
       //std::cout<<"here2"<<endl;

       
   }
   query = "SET FOREIGN_KEY_CHECKS = 1;";
   runQuery(query);      
}

void Debugging::closeConnection()
{
    mysql_close(&mysql);
}

void Debugging::runQuery(std::string query)
{        
    //std::cout << "query: " << query << std::endl;
    int err_code = mysql_query(connection, query.c_str());    
    if (err_code)//non-zero return means error!
    {
        std::cout << "something went wrong with the query! error code: " << err_code << std::endl;
        std::cout << "query: " << query << std::endl;
        std::cout << mysql_error(connection) << std::endl;
    }
    else
        result = mysql_store_result(connection);    
}

}
