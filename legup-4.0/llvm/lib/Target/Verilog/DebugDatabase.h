#ifndef DEBUGDATABASE_H
#define DEBUGDATABASE_H

#include <string>
#include <mysql/mysql.h>

// using namespace llvm;

namespace legup {

class DebugVariable;
class DebugValue;
class GenerateRTL;
class State;
class TraceScheduler;

class DebugDatabase {
  public:
    DebugDatabase(LegUpDebugInfo *dbgInfo) : dbgInfo(dbgInfo) {}

    void initialize();

    void runQuery(std::string query);

    void dropDesign(std::string path);
    void addDesign(std::string path, std::string name);
    void setDesignProperties(bool isDebugRtlEnabled, bool isXilinx,
                             std::string board, int memAddrWidth,
                             int memDataWidth);
    void setRtlInstrumentationProperties(int numInstanceIdBits,
                                         int numStateBits,
                                         unsigned int systemId);
    void setTraceBufferProperties(int controlBufWidth,
                                  int controlBufSequenceBits,
                                  int controlBufDepth, int memBufWidth,
                                  int memBufDepth, int regsBufEnabled,
                                  int regsBufWidth, int regsBufDepth);

    void addFunction(MDNode *subprogram, GenerateRTL *hw);
    void addInstance(GenerateRTL *hw, int instanceNum);
    void addInstanceChild(int instanceNum, int childInstanceNum);
    void addVariable(DebugVariable *var, std::string name, bool isGlobal,
                     MDNode *swFunction, GenerateRTL *hw, MDNode *type,
                     std::string filePath, int lineNum,
                     std::string inlinedPath);
    void addVariableSourceRAM(DebugVariable *var, Instruction *I,
                              Value *address);
    void addVariableSourceDbgValue(DebugValue *dbgVal,
                                   TraceScheduler *traceScheduler);
    void addStates(GenerateRTL *hw);
    void addIRInstructions(GenerateRTL *hw);
    void addRtlSignals(GenerateRTL *hw, TraceScheduler *ts);
    void addRAM(RAM *ram);

    void setTypeMap(DITypeIdentifierMap &typeMap) { this->typeMap = typeMap; }

  private:
    std::string version;
    LegUpDebugInfo *dbgInfo;
    MYSQL mysql;
    MYSQL *connection;

    MYSQL_RES *result;

    DITypeIdentifierMap typeMap;

    int designId;
    std::map<MDNode *, int> subprogramsToFunctionIds;
    std::map<const GenerateRTL *, int> hwToFunctionIds;
    std::map<const State *, int> statesToIds;
    std::map<const MDNode *, int> typeToIds;
    std::map<const DebugVariable *, int> varToIds;
    std::map<const Instruction *, int> insnToIds;
    std::map<const RTLSignal *, int> rtlSignalToIds;
    std::map<const int, int> instanceNumToIds;
    std::map<RAM *, int> ramToIds;

    //    std::string boolToBitStr(bool b);
    std::string addQuotesToStr(std::string s);

    void addIRInstruction(GenerateRTL *hw, Instruction *I, int numInFunction,
                          bool isDummyDbgCall, std::string filePath,
                          int lineNumber, int columnNumber);
    void addType(MDNode *type);
    int getTypeId(MDNode *type);
    int addVariableSource(DebugVariable *var, Instruction *I,
                          bool valSrcSupported);
    void getGEPBaseAndOffset(ConstantExpr *GEP, Value **getBase,
                             int *getOffset);
};
}

#endif
