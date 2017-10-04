#include <string>

#include "Allocation.h"
#include "LegupConfig.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/DebugInfo.h"
#include "Debug.h"
#include "Ram.h"
#include "DebugDatabase.h"
#include "DebugVariable.h"
#include "GenerateRTL.h"
#include "FiniteStateMachine.h"

// using namespace llvm;
using namespace legup;

void DebugDatabase::initialize() {
    // These compile time constants will ensure that if this file changes
    // the debug database will be re-installed.
    version = __DATE__;
    version += " ";
    version += __TIME__;

    mysql_init(&mysql);
    std::string host = LEGUP_CONFIG->getParameter("DEBUG_DB_HOST");
    std::string user = LEGUP_CONFIG->getParameter("DEBUG_DB_USER");
    std::string password = LEGUP_CONFIG->getParameter("DEBUG_DB_PASSWORD");
    std::string db = LEGUP_CONFIG->getParameter("DEBUG_DB_NAME");
    std::string dbScriptFile =
        LEGUP_CONFIG->getParameter("DEBUG_DB_SCRIPT_FILE");

    connection = mysql_real_connect(&mysql, host.c_str(), user.c_str(),
                                    password.c_str(), db.c_str(), 3306, 0, 0);

    bool installDatabase = false;

    if (connection == NULL) {
        outs() << "Debug database is not found. A fresh database will be "
                  "installed...\n";
        installDatabase = true;
    } else {
        string query = "SELECT date_time FROM Version";
        runQuery(query);

        MYSQL_ROW row;
        row = mysql_fetch_row(result);
        if (row[0] != version) {
            outs() << "Debug database is out of date.  A fresh database will "
                      "be installed...\n";

            string query =
                "DROP SCHEMA " + LEGUP_CONFIG->getParameter("DEBUG_DB_NAME");
            runQuery(query);
            installDatabase = true;
        }
    }

    if (installDatabase) {
        mysql_init(&mysql);
        std::string cmd = "mysql --user=" + user + " --password=" + password +
                          " <" + dbScriptFile;
        int ret = system(cmd.c_str());
        if (ret) {
            report_fatal_error("Debug database script creation failed.  Make "
                               "sure you have set the 'DEBUG_DB_SCRIPT_FILE' "
                               "parameter to "
                               "<legup>/examples/createDebugDB.sql");
        }

        connection =
            mysql_real_connect(&mysql, host.c_str(), user.c_str(),
                               password.c_str(), db.c_str(), 3306, 0, 0);
        if (connection == NULL) {
            errs() << "Debug database creation failed...\n";
            exit(1);
        }

        string query = "INSERT INTO Version (date_time) VALUES (" +
                       addQuotesToStr(version) + ")";
        runQuery(query);

        outs() << "Debug database created successfully.\n";
    }
}

void DebugDatabase::runQuery(std::string query) {
    int err_code = mysql_query(connection, query.c_str());
    if (err_code) {
        errs() << "something went wrong with the query! error code: "
               << err_code << "\n";
        errs() << "query: " << query << "\n";
        errs() << mysql_error(connection) << "\n";
    } else
        result = mysql_store_result(connection);
}

void DebugDatabase::dropDesign(std::string path) {
    std::string query;
    query = "DELETE FROM Designs WHERE Designs.path = '" + path + "';";
    runQuery(query);
}

void DebugDatabase::addDesign(std::string path, std::string name) {
    std::string query;
    query = "INSERT INTO Designs (path, name) VALUES (";
    query += addQuotesToStr(path);
    query += "," + addQuotesToStr(name);
    query += ");";
    runQuery(query);

    designId = mysql_insert_id(connection);
}

// std::string DebugDatabase::boolToBitStr(bool b) {
//    return b ? "1" : "0";
//}

std::string DebugDatabase::addQuotesToStr(std::string s) {
    std::string ret;
    ret += "'" + s + "'";
    return ret;
}

void DebugDatabase::setDesignProperties(bool isDebugRtlEnabled, bool isXilinx,
                                        std::string board, int memAddrWidth,
                                        int memDataWidth) {
    std::string query;
    query = "INSERT INTO DesignProperties ";
    query += "(designId, isDebugRtlEnabled, isXilinx, board, memoryAddrWidth, "
             "memoryDataWidth) ";
    query += "VALUES ";
    query += "(" + std::to_string(designId);
    query += "," + std::to_string(isDebugRtlEnabled);
    query += "," + std::to_string(isXilinx);
    query += "," + addQuotesToStr(board);
    query += "," + std::to_string(memAddrWidth);
    query += "," + std::to_string(memDataWidth);
    query += ");";

    runQuery(query);
}

void DebugDatabase::setRtlInstrumentationProperties(int numInstanceIdBits,
                                                    int numStateBits,
                                                    unsigned int systemId) {
    std::string query;
    query = "INSERT INTO InstrumentationProperties ";
    query += "(designId, numInstanceBits, numStateBits, systemId) ";
    query += "VALUES ";
    query += "(" + std::to_string(designId);
    query += "," + std::to_string(numInstanceIdBits);
    query += "," + std::to_string(numStateBits);
    query += "," + std::to_string(systemId);
    query += ");";

    runQuery(query);
}

void DebugDatabase::setTraceBufferProperties(
    int controlBufWidth, int controlBufSequenceBits, int controlBufDepth,
    int memBufWidth, int memBufDepth, int regsBufEnabled, int regsBufWidth,
    int regsBufDepth) {
    std::string query;
    query = "INSERT INTO TraceBufferProperties ";
    query +=
        "(designId, controlBufWidth, controlBufSequenceBits, controlBufDepth, "
        "memoryBufWidth, memoryBufDepth, regsBufEnabled, regsBufWidth, "
        "regsBufDepth) ";
    query += "VALUES ";
    query += "(" + std::to_string(designId);
    query += "," + std::to_string(controlBufWidth);
    query += "," + std::to_string(controlBufSequenceBits);
    query += "," + std::to_string(controlBufDepth);
    query += "," + std::to_string(memBufWidth);
    query += "," + std::to_string(memBufDepth);
    query += "," + std::to_string(regsBufEnabled);
    query += "," + std::to_string(regsBufWidth);
    query += "," + std::to_string(regsBufDepth);
    query += ");";

    runQuery(query);
}

void DebugDatabase::addFunction(MDNode *subprogram, GenerateRTL *hw) {
    string name;
    DISubprogram s;

    assert(subprogram || hw);

    if (subprogram) {
        s = DISubprogram(subprogram);
        name = s.getName().str();
    }

    if (hw) {
        //        dbgs() << "Adding function " <<
        //        hw->getFunction()->getName().str() << "\n";
        if (hwToFunctionIds.find(hw) != hwToFunctionIds.end()) {
            // This function has already been added
            // This can happen since we add functions with metadata first, then
            // add all functions with hardware next.  Those with metadata will
            // have
            // already been added
            //            dbgs() << "exiting\n";
            return;
        } else {
            //            dbgs() << "not exiting\n";
        }

        if (subprogram) {
            assert(name == hw->getFunction()->getName().str());
        } else {
            name = hw->getFunction()->getName().str();
        }
    }

    std::string query = "INSERT INTO Function (designId, name, inlined, "
                        "hasMetadata, startLineNumber) ";
    query += "VALUES (" + std::to_string(designId);
    query += "," + addQuotesToStr(name);
    query += "," + std::to_string(hw ? false : true);
    query += "," + std::to_string(subprogram ? true : false);
    query += "," + (subprogram ? std::to_string(s.getLineNumber()) : "NULL");
    query += ");";
    runQuery(query);

    int functionId = mysql_insert_id(connection);

    if (subprogram) {
        subprogramsToFunctionIds[subprogram] = functionId;
    }

    if (hw) {
        hwToFunctionIds[hw] = functionId;
    }
}

void DebugDatabase::addInstance(GenerateRTL *hw, int instanceNum) {
    int functionId = hwToFunctionIds[hw];

    std::string query =
        "INSERT INTO Instance (designId, functionId, instanceNum) ";
    query += "VALUES (" + std::to_string(designId);
    query += "," + std::to_string(functionId);
    query += "," + std::to_string(instanceNum);
    query += ");";

    runQuery(query);

    int instanceId = mysql_insert_id(connection);
    instanceNumToIds[instanceNum] = instanceId;
}

void DebugDatabase::addInstanceChild(int instanceNum, int childInstanceNum) {
    std::string query =
        "INSERT INTO InstanceChildren (instanceId, childInstanceId) ";
    query += "VALUES (" + std::to_string(instanceNumToIds[instanceNum]);
    query += "," + std::to_string(instanceNumToIds[childInstanceNum]);
    query += ");";

    runQuery(query);
}

void DebugDatabase::addStates(GenerateRTL *hw) {
    FiniteStateMachine *fsm = hw->getFSM();
    int state_num = 0;
    for (auto state = fsm->begin(); state != fsm->end(); state++, state_num++) {
        std::string query = "INSERT INTO State ";
        query += "(belongingFunctionId, calledFunctionId, number, name, "
                 "storeA, storeB, ";
        query += "traceRegsPortA, traceRegsPortB) ";
        query += "VALUES (" + std::to_string(hwToFunctionIds[hw]);

        Function *calledF = state->getCalledFunction();
        if (calledF) {
            GenerateRTL *calledHw =
                dbgInfo->getAlloc()->getGenerateRTL(calledF);
            query += "," + std::to_string(hwToFunctionIds[calledHw]);
        } else {
            query += ",NULL";
        }

        query += "," + std::to_string(state_num);
        query += "," + addQuotesToStr(state->getName());

        bool storeA = false;
        bool storeB = false;

        for (auto i_it = state->begin(), i_end = state->end(); i_it != i_end;
             ++i_it) {
            if (isa<StoreInst>(*i_it)) {
                if (hw->connectedToPortB(*i_it)) {
                    assert(!storeB);
                    storeB = true;
                } else {
                    assert(!storeA);
                    storeA = true;
                }
            }
        }

        query += "," + std::to_string(storeA);
        query += "," + std::to_string(storeB);

        bool traceRegsPortA = false;
        bool traceRegsPortB = false;

        if (dbgInfo->getOptionTraceRegs()) {
            TraceScheduler *scheduler = dbgInfo->getTraceScheduler();
            TraceState *ts = scheduler->getTraceState(hw, state);
            if (ts && ts->getWidth() > 0) {
                traceRegsPortA = true;
                if (ts->getWidth() > dbgInfo->getRegsBufferWidth()) {
                    assert(dbgInfo->getOptionRegBufferDualPorted());
                    traceRegsPortB = true;
                }
            }
        }

        query += "," + std::to_string(traceRegsPortA);
        query += "," + std::to_string(traceRegsPortB);
        query += ");";

        runQuery(query);

        int stateId = mysql_insert_id(connection);
        statesToIds[state] = stateId;
    }
}

void DebugDatabase::addIRInstructions(GenerateRTL *hw) {
    Function *F = hw->getFunction();
    int instr_count = 0;
    for (Function::iterator b = F->begin(), be = F->end(); b != be; b++) {
        for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; i++) {

            instr_count++;

            bool isDummyDbgCall = false;
            if (isa<DbgDeclareInst>(i) || isa<DbgValueInst>(i))
                isDummyDbgCall = true;

            if (i->hasMetadata()) {
                MDNode *n = i->getMetadata("dbg");
                DILocation loc(n);
                int lineNumber = loc.getLineNumber();
                int columnNumber = loc.getColumnNumber();
                std::string filePath =
                    loc.getDirectory().str() + "/" + loc.getFilename().str();

                addIRInstruction(hw, i, instr_count, isDummyDbgCall, filePath,
                                 lineNumber, columnNumber);
            } else {
                addIRInstruction(hw, i, instr_count, isDummyDbgCall, "", 0, 0);
            }
        }
    }
}

void DebugDatabase::addRtlSignals(GenerateRTL *hw, TraceScheduler *ts) {
    for (auto i = hw->getRTL()->signals_begin(),
              e = hw->getRTL()->signals_end();
         i != e; ++i) {
        RTLSignal *rtlSignal = *i;
        int width = rtlSignal->getWidth().numBits(hw->getRTL(), dbgInfo->getAlloc());

        std::string query = "INSERT into RtlSignal(functionId, signalName, width) ";
        query += "VALUES (" + std::to_string(hwToFunctionIds[hw]);
        query += "," + addQuotesToStr(rtlSignal->getName());
        query += "," + std::to_string(width);
        query += ");";

        runQuery(query);

        int rtlSignalId = mysql_insert_id(connection);
        assert(rtlSignalId);
        rtlSignalToIds[rtlSignal] = rtlSignalId;
    }

    for (auto i = hw->getRTL()->port_begin(), e = hw->getRTL()->port_end();
         i != e; ++i) {
        RTLSignal *rtlSignal = *i;
        int width = rtlSignal->getWidth().numBits(hw->getRTL(), dbgInfo->getAlloc());

        std::string query = "INSERT into RtlSignal(functionId, signalName, width) ";
        query += "VALUES (" + std::to_string(hwToFunctionIds[hw]);
        query += "," + addQuotesToStr(rtlSignal->getName());
        query += "," + std::to_string(width);
        query += ");";

        runQuery(query);

        int rtlSignalId = mysql_insert_id(connection);
        assert(rtlSignalId);
        rtlSignalToIds[rtlSignal] = rtlSignalId;
    }

    if (dbgInfo->getOptionTraceRegs()) {
        auto states = (*ts->getSchedule())[hw];
        for (auto s_it = states.begin(), s_end = states.end(); s_it != s_end;
             ++s_it) {
            State *state = s_it->first;
            TraceState *ts = s_it->second;

            auto signals = ts->getSignals();
            for (auto sig_it = signals->begin(), sig_end = signals->end();
                 sig_it != sig_end; ++sig_it) {
                DebugScheduledSignal *sig = *sig_it;
                string query;
                query = "INSERT INTO RtlSignalTraceSchedule ";
                query += "(rtlSignalId, delayedCycles, recordInStateId, hiBit, "
                         "loBit) ";

                RTLSignal *signal = sig->getTracedSignal()->getSignal();
                assert(sig);

                int signalId = rtlSignalToIds[signal];
                assert(signalId);

                query += "VALUES (" + std::to_string(signalId);
                query += "," + std::to_string(sig->getDelay());
                query += "," + std::to_string(statesToIds[state]);
                query += "," + std::to_string(sig->getHi());
                query += "," + std::to_string(sig->getLo());
                query += ");";
                runQuery(query);
            }
        }
    }
}

void DebugDatabase::addIRInstruction(GenerateRTL *hw, Instruction *I,
                                     int numInFunction, bool isDummyDbgCall,
                                     std::string filePath, int lineNumber,
                                     int columnNumber) {
    std::string query = "INSERT into IRInstr(functionId, numInFunction, "
                        "isDummyDebugCall, filePath, lineNumber, columnNumber, "
                        "dump, startStateId, endStateId) ";
    query += "VALUES (" + std::to_string(hwToFunctionIds[hw]);
    query += "," + std::to_string(numInFunction);
    query += "," + std::to_string(isDummyDbgCall);
    query += "," + addQuotesToStr(filePath);
    query += "," + std::to_string(lineNumber);
    query += "," + std::to_string(columnNumber);
    query += "," + addQuotesToStr(getValueStr(I));
    query += "," + std::to_string(statesToIds[hw->getFSM()->getStartState(I)]);
    query += "," + std::to_string(statesToIds[hw->getFSM()->getEndState(I)]);
    query += ");";

    runQuery(query);

    int insnId = mysql_insert_id(connection);
    insnToIds[I] = insnId;
}

int DebugDatabase::getTypeId(MDNode *type) {
    if (typeToIds.find(type) == typeToIds.end())
        addType(type);

    return typeToIds[type];
}

void DebugDatabase::addType(MDNode *type) {
    DIType diType(type);

    string query;

    query = "INSERT INTO VariableType ";
    query +=
        "(designId, dw_tag, name, size, alignment, offset, derivedTypeId) ";
    query += "VALUES (" + std::to_string(designId);
    query += "," + std::to_string(diType.getTag());
    query += "," + addQuotesToStr(diType.getName().str());
    query += "," + std::to_string(diType.getSizeInBits());
    query += "," + std::to_string(diType.getAlignInBits());
    query += "," + std::to_string(diType.getOffsetInBits());

    if (diType.isBasicType()) {
        query += ",NULL";
    } else if (diType.isDerivedType()) {
        DIDerivedType typeDerived(type);
        MDNode *derivedMD = typeDerived.getTypeDerivedFrom().resolve(typeMap);
        if (derivedMD)
            query += "," + std::to_string(getTypeId(derivedMD));
        else
            query += ",NULL";
    } else {
        assert(false);
    }
    query += ");";

    runQuery(query);

    int typeId = mysql_insert_id(connection);
    typeToIds[&(*type)] = typeId;

    if (diType.isCompositeType()) {
        DICompositeType typeComposite(type);

        DIArray members = typeComposite.getTypeArray();
        for (unsigned int i = 0; i < members.getNumElements(); ++i) {
            DIDescriptor s = members.getElement(i);
            query = "INSERT INTO VariableTypeMember ";
            query +=
                "(ownerVariableTypeId, idx, variableTypeId, subrangeCount) ";
            query += "VALUES (" + std::to_string(typeId);
            query += "," + std::to_string(i);
            if (s.isSubrange()) {
                DISubrange subRange = (DISubrange)s;
                assert(subRange.getLo() == 0);
                query += ",NULL";
                query += "," + std::to_string(subRange.getCount());
            } else if (s.isType()) {
                MDNode *mdNode = &*s;
                query += "," + std::to_string(getTypeId(mdNode));
                query += ",NULL";
            }
            query += ");";
            runQuery(query);
        }
    }
}

void DebugDatabase::addVariable(DebugVariable *var, std::string name,
                                bool isGlobal, MDNode *swFunction,
                                GenerateRTL *hw, MDNode *type,
                                std::string filePath, int lineNum,
                                std::string inlinedPath) {
    std::string query = "INSERT INTO Variable ";
    query += "(designId, name, isGlobal, origFunctionId, functionId, typeId, "
             "filePath, lineNumber, inlinedPath) ";
    query += "VALUES (" + std::to_string(designId);
    query += "," + addQuotesToStr(name);
    query += "," + std::to_string(isGlobal);

    if (isGlobal) {
        query += ",NULL,NULL";
    } else {
        assert(swFunction);
        query += "," + std::to_string(subprogramsToFunctionIds[swFunction]);
        query += "," + std::to_string(hwToFunctionIds[hw]);
    }

    query += "," + std::to_string(getTypeId(type));
    query += "," + addQuotesToStr(filePath);
    query += "," + std::to_string(lineNum);
    query += "," + ((inlinedPath == "") ? "NULL" : addQuotesToStr(inlinedPath));
    query += ");";

    runQuery(query);

    int varId = mysql_insert_id(connection);
    varToIds[var] = varId;
}

int DebugDatabase::addVariableSource(DebugVariable *var, Instruction *I,
                                     bool valSrcSupported) {
    std::string query = "INSERT INTO VariableSource ";
    query += "(variableId, IRInstrId, valSrcSupported) ";
    query += "VALUES (" + std::to_string(varToIds[var]);

    if (I)
        query += "," + std::to_string(insnToIds[I]);
    else
        query += ",NULL";

    query += "," + std::to_string(valSrcSupported);
    query += ");";

    runQuery(query);

    return mysql_insert_id(connection);
}

void DebugDatabase::addRAM(RAM *ram) {
    // Make sure we aren't adding this twice
    assert(ramToIds.find(ram) == ramToIds.end());

    std::string query = "INSERT INTO RAM ";
    query += "(designId, tag, tagNum, tagAddressName, addressWidth, "
             "mifFileName, dataWidth, numElements) ";
    query += "VALUES (" + std::to_string(designId);
    query += "," + addQuotesToStr(ram->getTag());
    query += "," + std::to_string(dbgInfo->getAlloc()->getRamTagNum(ram));
    query += "," + addQuotesToStr(ram->getTagAddrName());
    query += "," + std::to_string(ram->getAddrWidth());
    query += "," + addQuotesToStr(ram->getMifFileName());
    query += "," + std::to_string(ram->getDataWidth());
    query += "," + std::to_string(ram->getElements());
    query += ");";

    runQuery(query);

    int ramId = mysql_insert_id(connection);
    ramToIds[ram] = ramId;
}

void DebugDatabase::addVariableSourceRAM(DebugVariable *var, Instruction *I,
                                         Value *address) {
    int id = addVariableSource(var, I, true);
    int ramId = ramToIds[dbgInfo->getAlloc()->getRAM(address)];

    std::string query = "INSERT INTO VariableSourceRAM ";
    query += "(VariableSourceId, ramId) ";
    query += "VALUES (" + std::to_string(id);
    query += "," + std::to_string(ramId);
    query += ");";

    runQuery(query);
}

void DebugDatabase::getGEPBaseAndOffset(ConstantExpr *GEP, Value **getBase,
                                        int *getOffset) {
    int offset = 0;
    Value *base = NULL;

    User *baseAddress = GEP->getOperand(0);
    ConstantExpr *CE = dyn_cast<ConstantExpr>(baseAddress);

    if (CE && CE->getOpcode() == Instruction::GetElementPtr) {
        // Nested GEP
        getGEPBaseAndOffset(CE, &base, &offset);
    } else if (isa<GlobalVariable>(baseAddress) ||
               isa<AllocaInst>(baseAddress)) {
        base = baseAddress;
    } else {
        assert(false);
    }

    assert(base);

    gep_type_iterator GTI = gep_type_begin(GEP);
    for (User::op_iterator i = GEP->op_begin() + 1, e = GEP->op_end(); i != e;
         ++i, ++GTI) {
        Value *Op = *i;

        // Build a mask for high order bits.
        const DataLayout *TD = dbgInfo->getAlloc()->getDataLayout();
        unsigned IntPtrWidth = TD->getPointerSizeInBits();
        uint64_t PtrSizeMask = ~0ULL >> (64 - IntPtrWidth);

        // apply mask
        uint64_t Size =
            TD->getTypeAllocSize(GTI.getIndexedType()) & PtrSizeMask;

        assert(isa<ConstantInt>(Op));
        if (ConstantInt *OpC = dyn_cast<ConstantInt>(Op)) {
            // Handle a struct index, which adds its field offset.
            if (StructType *STy = dyn_cast<StructType>(*GTI)) {
                offset += TD->getStructLayout(STy)
                              ->getElementOffset(OpC->getZExtValue());
            } else {
                offset += Size * OpC->getValue().getSExtValue();
            }
        }
    }
    *getBase = base;
    *getOffset = offset;
}

void DebugDatabase::addVariableSourceDbgValue(DebugValue *dbgVal,
                                              TraceScheduler *traceScheduler) {
    DbgValueInst *dbgValInst = dbgVal->getDbgValInsn();
    assert(dbgValInst);

    int id = addVariableSource(dbgVal->getVariable(), dbgValInst, true);

    std::string query;
    if (dbgVal->isConstantInt()) {
        // Get the integer value
        ConstantInt *ci = cast<ConstantInt>(dbgValInst->getValue());

        query = "INSERT INTO VariableSourceConstantInt ";
        query += "(VariableSourceId, constantInt) ";
        query += "VALUES (" + std::to_string(id);
        query += "," + std::to_string(ci->getSExtValue());
        query += ");";
        runQuery(query);

    } else if (dbgVal->isConstantNull()) {
        query = "INSERT INTO VariableSourceConstantInt ";
        query += "(VariableSourceId, constantInt) ";
        query += "VALUES (" + std::to_string(id);
        query += ",NULL";
        query += ");";
        runQuery(query);

    } else if (dbgVal->isConstantPointer()) {
        ConstantExpr *CE = dyn_cast<ConstantExpr>(dbgValInst->getValue());
        assert(CE);

        Value *base;
        int offset;
        getGEPBaseAndOffset(CE, &base, &offset);

        int ramId = ramToIds[dbgInfo->getAlloc()->getRAM(base)];
        assert(ramId);

        query = "INSERT INTO VariableSourcePointer ";
        query += "(VariableSourceId, ramId, offset) ";
        query += "VALUES (" + std::to_string(id);
        query += "," + std::to_string(ramId);
        query += "," + std::to_string(offset);
        query += ");";
        runQuery(query);

    } else if (dbgVal->isAlloca()) {
        RAM *ram =
            dbgInfo->getAlloc()->getRAM(dbgVal->getDbgValInsn()->getValue());
        assert(ram);

        int offset = dbgVal->getDbgValInsn()->getOffset();

        int ramId = ramToIds[ram];
        assert(ramId);

        query = "INSERT INTO VariableSourcePointer ";
        query += "(VariableSourceId, ramId, offset) ";
        query += "VALUES (" + std::to_string(id);
        query += "," + std::to_string(ramId);
        query += "," + std::to_string(offset);
        query += ");";
        runQuery(query);

        //        dbgs() << *(dbgVal->getDbgValInsn()) << " is an alloca\n";
    } else if (dbgVal->isArgument() || dbgVal->isFunctionCall() ||
               dbgVal->isRegister()) {
        GenerateRTL *hw = dbgVal->getVariable()->getGenRtl();

        string signalName = dbgVal->getSignalName();
        RTLSignal *signal;

        signal = hw->getRTL()->findExists(signalName);

        if (!signal) {
            dbgs() << *(dbgVal->getDbgValInsn()) << "\n";
            dbgs() << "Can't find signal " << dbgVal->getSignalName() << "\n";
        }

        //        RTLSignal *signal =
        //        hw->getRTL()->find(dbgVal->getSignalName());

        query = "INSERT INTO VariableSourceSignal ";
        query += "(VariableSourceId, rtlSignalId) ";
        query += "VALUES (" + std::to_string(id);
        query += "," + std::to_string(rtlSignalToIds[signal]);
        query += ");";
        runQuery(query);

    } else if (dbgVal->isUndefined()) {
        query = "INSERT INTO VariableSourceUndefined ";
        query += "(VariableSourceId) ";
        query += "VALUES (" + std::to_string(id);
        query += ");";
        runQuery(query);
    } else {
        dbgs() << *(dbgValInst) << "\n";
        dbgs() << *(dbgValInst->getValue()) << "\n";
        assert(false);
    }
}
