#include "DebugVariable.h"
#include "Ram.h"
#include "FiniteStateMachine.h"
#include "Debug.h"
#include "DebugDatabase.h"

#include "llvm/IR/DebugInfo.h"

#include <vector>

using namespace llvm;
using namespace legup;

class LegUpDebugInfo;

void DebugVariableLocal::addDbgDeclare(Allocation *allocation,
                                       DbgDeclareInst *dbgDeclare) {
    if (isa<UndefValue>(dbgDeclare->getAddress()))
        return;

    // Find for this variable, and make sure a RAM exists
    assert(allocation->getRAM(dbgDeclare->getAddress()));

    dbgDeclareInsns.push_back(dbgDeclare);
}

void DebugVariableLocal::addDbgValue(DbgValueInst *dbgValueInsn) {
    values.push_back(new DebugValue(this, dbgValueInsn));
}

string DebugVariableLocal::getName() {
    DIVariable var(metaNode);
    return var.getName().str();
}

void DebugVariableGlobal::addToDebugDatabase(DebugDatabase *database,
                                             Allocation *alloc,
                                             DITypeIdentifierMap &typeMap) {
    DIGlobalVariable var(metaNode);

    MDNode *type = var.getType().resolve(typeMap);
    database->addVariable(this, var.getName().str(), true, NULL, NULL, type,
                          var.getDirectory().str() + "/" +
                              var.getFilename().str(),
                          var.getLineNumber(), "");

    if (address) {
        database->addVariableSourceRAM(this, NULL, address);
    }
}

void DebugValue::addToDebugDatabase(DebugDatabase *database,
                                    TraceScheduler *traceScheduler) {
    database->addVariableSourceDbgValue(this, traceScheduler);
}

string DebugValue::getSignalName() {
    DebugVariableLocal *localVar = dyn_cast<DebugVariableLocal>(parentVar);
    assert(localVar);

    GenerateRTL *genRtl = localVar->getGenRtl();

    if (isRegister()) {
        return genRtl->verilogName(dbgValueInsn->getValue()) + "_reg";
    } else if (isArgument()) {
        return genRtl->verilogName(dbgValueInsn->getValue());
    } else if (isFunctionCall()) {
        CallInst *ci = dyn_cast<CallInst>(dbgValueInsn->getValue());
        assert(ci);
        return genRtl->verilogName(getCalledFunction(ci)) + "_return_val_reg";
    } else {
        assert(false);
    }
    return "";
}

State *DebugValue::getRecordState() {
    GenerateRTL *genRtl = parentVar->getGenRtl();
    State *state = NULL;
    Value *val = dbgValueInsn->getValue();

    if (isFunctionCall()) {
        CallInst *ci = dyn_cast<CallInst>(val);
        assert(ci);

        state = genRtl->getFSM()->getStartState(ci);
        assert(state->getNumTransitions() == 2 &&
               state->getDefaultTransition() == state);

        // Record in the return state
        state = state->getTransitionState(0);
    } else if (isRegister()) {
        Instruction *i = dyn_cast<Instruction>(val);
        assert(i);

        // Record in the ending state of the instruction
        state = genRtl->getFSM()->getEndState(i);
    } else if (isArgument()) {
        state = genRtl->getFSM()->begin();
    }
    return state;
}

int DebugValue::getStateNum() {
    // Get state #
    FiniteStateMachine *fsm = parentVar->getGenRtl()->getFSM();
    return fsm->getStateNum(fsm->getStartState(dbgValueInsn));
}

void DebugVariableLocal::addToDebugDatabase(DebugDatabase *database,
                                            Allocation *alloc,
                                            DITypeIdentifierMap &typeMap,
                                            TraceScheduler *traceScheduler) {
    DIVariable var(metaNode);

    string sourcePath = "";
    MDNode *inlinedLocMD = var.getInlinedAt();

    // This variable has been inlined - find the inlining path
    if (inlinedLocMD) {
        DILocation inlinedLoc(inlinedLocMD);
        while (inlinedLoc.Verify()) {
            if (sourcePath != "")
                sourcePath = "|" + sourcePath;

            // Add line number
            sourcePath =
                std::to_string(inlinedLoc.getLineNumber()) + sourcePath;

            // Find subprogram
            DIScope scope = inlinedLoc.getScope();
            while (!scope.isSubprogram())
                scope = scope.getContext().resolve(typeMap);

            // Add function name
            sourcePath = scope.getName().str() + ":" + sourcePath;

            inlinedLoc = inlinedLoc.getOrigLocation();
        }
    }

    DIScope scope = var.getContext();
    while (!scope.isSubprogram())
        scope = scope.getContext().resolve(typeMap);
    assert(scope.isSubprogram());

    MDNode *origFunction = (MDNode *)scope;

    DISubprogram s(origFunction);
    MDNode *type = var.getType().resolve(typeMap);
    database->addVariable(this, var.getName().str(), false, origFunction,
                          genRtl, type,
                          var.getFile().getDirectory().str() + "/" +
                              var.getFile().getFilename().str(),
                          var.getLineNumber(), sourcePath);

    for (auto dd_it = dbgDeclareInsns.begin(), dd_end = dbgDeclareInsns.end();
         dd_it != dd_end; ++dd_it) {
        database->addVariableSourceRAM(this, *dd_it, (*dd_it)->getAddress());
    }

    for (auto val_it = values.begin(), e = values.end(); val_it != e;
         ++val_it) {
        (*val_it)->addToDebugDatabase(database, traceScheduler);
    }
}

DebugVariableGlobal::DebugVariableGlobal(MDNode *metaNode, Allocation *alloc)
    : DebugVariable(DVK_Global, metaNode), address(NULL) {

    DIGlobalVariable var(metaNode);

    if (var.getGlobal()) {
        // Find associated RAM
        address = var.getGlobal();
        assert(alloc->getRAM(address));
    }
}
