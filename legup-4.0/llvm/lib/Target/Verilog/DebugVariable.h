#ifndef LEGUP_DEBUG_VARIABLE_H
#define LEGUP_DEBUG_VARIABLE_H

//#include "Allocation.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/Support/Casting.h"

#include <fstream>
#include <iostream>

using namespace llvm;

namespace legup {

class RAM;
class State;
class Allocation;
class GenerateRTL;
class LegUpDebugInfo;
class RTLDebugPort;
class DebugDatabase;
class DebugVariable;
class DebugVariableGlobal;
class DebugVariableLocal;
class TraceScheduler;

class DebugValue {
  protected:
    DebugVariableLocal *parentVar;
    DbgValueInst *dbgValueInsn;

  public:
    DebugValue(DebugVariableLocal *parentVar, DbgValueInst *dbgValue)
        : parentVar(parentVar), dbgValueInsn(dbgValue) {

        // Ensure we are writing to offset = 0 in the variable
        ConstantInt *ci = cast<ConstantInt>(dbgValue->getOperand(1));
        assert(ci->getSExtValue() == 0);
    }

    DbgValueInst *getDbgValInsn() { return dbgValueInsn; }
    void addToDebugDatabase(DebugDatabase *database,
                            TraceScheduler *traceScheduler);
    DebugVariableLocal *getVariable() { return parentVar; }
    int getStateNum();
    std::string getSignalName();

    bool isConstantInt() { return isa<ConstantInt>(dbgValueInsn->getValue()); }
    bool isConstantNull() {
        return isa<ConstantPointerNull>(dbgValueInsn->getValue());
    }
    bool isConstantPointer() {
        ConstantExpr *ce = dyn_cast<ConstantExpr>(dbgValueInsn->getValue());
        return ce && ce->isGEPWithNoNotionalOverIndexing();
    }
    bool isArgument() { return isa<Argument>(dbgValueInsn->getValue()); }
    bool isFunctionCall() { return isa<CallInst>(dbgValueInsn->getValue()); }
    bool isAlloca() { return isa<AllocaInst>(dbgValueInsn->getValue()); }
    bool isRegister() {
        return isa<Instruction>(dbgValueInsn->getValue()) &&
               !isFunctionCall() && !isAlloca();
    }
    bool isUndefined() { return isa<UndefValue>(dbgValueInsn->getValue()); }

    bool requiresSignalTracing() {
        return isRegister() || isFunctionCall() || isArgument();
    }
    State *getRecordState();
};

class DebugVariable {
public:
  enum DebugVariableKind { DVK_Global, DVK_Local };

private:
  DebugVariableKind kind;

protected:
  llvm::MDNode *metaNode;
  int tag;

public:
  static int nextVarID();

  DebugVariable(DebugVariableKind kind, llvm::MDNode *metaNode)
      : kind(kind), metaNode(metaNode), tag(-1) {}
  virtual ~DebugVariable() {}

  DebugVariableKind getKind() const { return kind; }

  llvm::MDNode *getMetaNode() { return metaNode; }
};

class DebugVariableGlobal : public DebugVariable {
  private:
    Value *address;

  public:
    DebugVariableGlobal(MDNode *metaNode, Allocation *alloc);
    void addToDebugDatabase(DebugDatabase *database, Allocation *alloc,
                            DITypeIdentifierMap &typeMap);
    //    Value *getAddress() { return address; }
    static bool classof(const DebugVariable *dv) {
        return dv->getKind() == DVK_Global;
    }
};

class DebugVariableLocal : public DebugVariable {
  private:
    GenerateRTL *genRtl;
    std::vector<DebugValue *> values;
    std::vector<DbgDeclareInst *> dbgDeclareInsns;

  public:
    DebugVariableLocal(MDNode *metaNode, GenerateRTL *genRTL)
        : DebugVariable(DVK_Local, metaNode), genRtl(genRTL) {}

    static bool classof(const DebugVariable *dv) {
        return dv->getKind() == DVK_Local;
    }
    void addToDebugDatabase(DebugDatabase *database, Allocation *alloc,
                            DITypeIdentifierMap &typeMap,
                            TraceScheduler *traceScheduler);
    std::string getName();

    void addDbgDeclare(Allocation *allocation, DbgDeclareInst *dbgDeclare);
    std::vector<DebugValue *> *getDbgValues() { return &values; }
    GenerateRTL *getGenRtl() { return genRtl; }

    void addDbgValue(DbgValueInst *dbgValue);
};
}

#endif
