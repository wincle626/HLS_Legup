#include <iostream>
#include <fstream>
#include <iomanip>
#include <assert.h>
#include <string>
#include <map>
#include <unistd.h>

#include "Debug.h"
#include "Ram.h"
#include "FiniteStateMachine.h"
#include "Scheduler.h"
#include "utils.h"
#include "DebugDatabase.h"

using namespace llvm;
using namespace legup;

int RTLModuleInstance::nextId = 1;

const bool dualPortTraceing = true;

void RTLDebugPort::populateSourceInstance(LegUpDebugInfo *dbgInfo) {
    instance = dbgInfo->getInstance(getHierarchyPath());
}

string RTLDebugPort::getHierarchyPath() {
    string sigName = signal->getName();
    string sourceSigName = sourceSignal->getName();

    // Make sure that the signal name ends with the source signal name
    assert(sigName.length() >= sourceSigName.length() &&
           (sigName.compare(sigName.length() - sourceSigName.length(),
                            sourceSigName.length(), sourceSigName) == 0));

    string path = sigName;
    path.erase(sigName.length() - sourceSigName.length(),
               sourceSigName.length());
    return path;
}

RTLModuleInstance::RTLModuleInstance(LegUpDebugInfo *dbgInfo,
                                     GenerateRTL *genRtl, int parentInstId)
    : dbgInfo(dbgInfo), id(nextId++), hierarchyPath(""), genRtl(genRtl) {

    RTLModuleInstance *parent = dbgInfo->getInstance(parentInstId);

    string parentPath;
    if (parent) {
        parentPath = parent->getHierarchyPath();
        parent->addChild(this);
    } else {
        parentPath = "";
    }

    hierarchyPath = parentPath + genRtl->getFunction()->getName().str() +
                    DEBUG_HIERARCHY_PATH_SEP;
}

string RTLModuleInstance::getChildHierarchyPath() {

    return hierarchyPath + genRtl->getFunction()->getName().str() +
           DEBUG_HIERARCHY_PATH_SEP;
}

void RTLModuleInstance::addChild(RTLModuleInstance *child) {
    children.push_back(child);
}

DbgDeclareInst *LegUpDebugInfo::findDebugDeclare(Allocation *alloc,
                                                 MDNode *md_node) {
    /* Find the associated llvm.dbg.declare instruction */
    for (Allocation::hw_iterator hw_it = alloc->hw_begin();
         hw_it != alloc->hw_end(); hw_it++) {
        Function *F = (*hw_it)->getFunction();
        for (Function::iterator FI = F->begin(), FE = F->end(); FI != FE;
             ++FI) {
            for (BasicBlock::iterator BI = (*FI).begin(), BE = (*FI).end();
                 BI != BE; ++BI) {
                if (DbgDeclareInst *DDI = dyn_cast<DbgDeclareInst>(BI))
                    if (DDI->getVariable() == md_node)
                        return DDI;
            }
        }
    }
    return NULL;
}

DbgDeclareInst *LegUpDebugInfo::findDebugDeclareLocal(Function *Fp,
                                                      MDNode *metaNode) {
    /* Find the associated llvm.dbg.declare instruction */
    for (Function::iterator FI = Fp->begin(), FE = Fp->end(); FI != FE; ++FI) {
        for (BasicBlock::iterator BI = (*FI).begin(), BE = (*FI).end();
             BI != BE; ++BI) {
            if (DbgDeclareInst *DDI = dyn_cast<DbgDeclareInst>(BI))
                if (DDI->getVariable() == metaNode)
                    return DDI;
        }
    }
    return NULL;
}

vector<DbgValueInst *> LegUpDebugInfo::findDebugValues(Function *Fp,
                                                       MDNode *md_node) {
    vector<DbgValueInst *> DVIs;

    /* Find the associated llvm.dbg.value instruction */
    for (Function::iterator FI = Fp->begin(), FE = Fp->end(); FI != FE; ++FI) {
        for (BasicBlock::iterator BI = (*FI).begin(), BE = (*FI).end();
             BI != BE; ++BI) {
            if (DbgValueInst *DVI = dyn_cast<DbgValueInst>(BI)) {
                if (DVI->getVariable() == md_node) {
                    DVIs.push_back(DVI);
                }
            }
        }
    }
    return DVIs;
}

vector<DbgValueInst *> LegUpDebugInfo::findDebugValues(Allocation *alloc,
                                                       MDNode *md_node) {
    vector<DbgValueInst *> DVIs;

    /* Find the associated llvm.dbg.value instruction */
    for (Allocation::hw_iterator hw_it = alloc->hw_begin();
         hw_it != alloc->hw_end(); hw_it++) {
        Function *F = (*hw_it)->getFunction();
        for (Function::iterator FI = F->begin(), FE = F->end(); FI != FE;
             ++FI) {
            for (BasicBlock::iterator BI = (*FI).begin(), BE = (*FI).end();
                 BI != BE; ++BI) {
                if (DbgValueInst *DVI = dyn_cast<DbgValueInst>(BI)) {
                    if (DVI->getVariable() == md_node) {
                        DVIs.push_back(DVI);
                    }
                }
            }
        }
    }
    return DVIs;
}

int LegUpDebugInfo::getStateBits() {
    assert(stateBits > 0);
    return stateBits;
}

int LegUpDebugInfo::getStateBitsOneHot() {
    assert(stateBitsOneHot > 0);
    return stateBitsOneHot;
}

int LegUpDebugInfo::getInstanceIdBits() {
    assert(instanceBits > 0);
    return instanceBits;
}

LegUpDebugInfo::LegUpDebugInfo(Allocation *allocation)
    : alloc(allocation), debugDatabase(NULL), instanceBits(-1), stateBits(-1),
      regsTraceBits(-1), traceScheduler(NULL), traceSchedulerRtl(NULL),
      stateMuxer(NULL) {
    Module *module = alloc->getModule();

    if (LEGUP_CONFIG->getParameterInt("DEBUG_FILL_DATABASE")) {
        optionFillDatabase = true;
    } else {
        optionFillDatabase = false;
    }

    if (LEGUP_CONFIG->getParameterInt("DEBUG_INSERT_DEBUG_RTL")) {
        assert(optionFillDatabase);
        optionInsertRtl = true;
    } else {
        optionInsertRtl = false;
    }

    /* Load options */
    if (LEGUP_CONFIG->getParameterInt("DEBUG_CORE_TRACE_REGS")) {
        optionTraceRegs = true;
    } else {
        optionTraceRegs = false;
    }

    optionPreserveOneHot = true;

    if (LEGUP_CONFIG->getParameterInt("DEBUG_CORE_TRACE_REGS_DELAY_WORST")) {
        optionTraceRegsDelayWorst = true;
    } else {
        optionTraceRegsDelayWorst = false;
    }

    if (LEGUP_CONFIG->getParameterInt("DEBUG_CORE_TRACE_REGS_DELAY_ALL")) {
        optionTraceRegsDelayAll = true;
    } else {
        optionTraceRegsDelayAll = false;
    }

    if (LEGUP_CONFIG->getParameterInt("DEBUG_CORE_TRACE_REGS_DUAL_PORT")) {
        optionRegBufferDualPorted = true;
    } else {
        optionRegBufferDualPorted = false;
    }

    if (LEGUP_CONFIG->getParameterInt("DEBUG_CORE_SIZE_BUFS_STATIC_ANALYSIS")) {
        optionSizeBufsStaticAnalysis = true;
    } else {
        optionSizeBufsStaticAnalysis = false;
    }

    if (LEGUP_CONFIG->getParameterInt("DEBUG_CORE_SIZE_BUFS_SIMULATION")) {
        optionSizeBufsSimulation = true;
    } else {
        optionSizeBufsSimulation = false;
    }

    if (LEGUP_CONFIG->getParameterInt("DEBUG_CORE_TRACE_REGS_DELAY_DEBUG")) {
        optionPrintDelayedTracingDebug = true;
    } else {
        optionPrintDelayedTracingDebug = false;
    }

    optionSupportReadFromMem = true;

    moduleName = module->getModuleIdentifier();
    string extension(".bc");
    if (stringEndsWith(moduleName, extension)) {
        moduleName.erase(moduleName.end() - extension.length(),
                         moduleName.end());
    }

    /* Check if debug information is present */
    if (optionFillDatabase) {
        NamedMDNode *debug_metadata = module->getNamedMetadata("llvm.dbg.cu");
        if (!debug_metadata)
            report_fatal_error(
                "Debug metadata is missing.  Add 'CFLAG = -g' to Makefile.");
        if (debug_metadata->getNumOperands() != 1) {
            report_fatal_error(
                "Debug metadata must contain 1 entry in llvm.dbg.cu");
        }
        compileUnit = DICompileUnit(debug_metadata->getOperand(0));
    }

    /* Output System ID */
    if (optionInsertRtl) {
        generateSystemID();
    }
}

void LegUpDebugInfo::analyzeProgram() {
    // Number of memory writes
    numWrites = 0;
    for (Allocation::hw_iterator hw_it = alloc->hw_begin(),
                                 hw_end = alloc->hw_end();
         hw_it != hw_end; ++hw_it) {
        for (FiniteStateMachine::iterator
                 state_it = (*hw_it)->getFSM()->begin(),
                 state_end = (*hw_it)->getFSM()->end();
             state_it != state_end; ++state_it) {
            for (State::iterator insn_it = state_it->begin(),
                                 insn_end = state_it->end();
                 insn_it != insn_end; ++insn_it) {
                if (dyn_cast<StoreInst>(*insn_it))
                    numWrites++;
            }
        }
    }

    // Find total number of states
    numStates = 0;
    for (Allocation::hw_iterator hw_it = alloc->hw_begin(),
                                 hw_end = alloc->hw_end();
         hw_it != hw_end; ++hw_it) {
        numStates += (*hw_it)->getFSM()->getNumStates();
    }
}

void LegUpDebugInfo::generateVariableInfo() {
    //    DITypeIdentifierMap typeMap =
    //    generateDITypeIdentifierMap(debug_metadata);

    /* Global variables */
    DIArray globals = compileUnit.getGlobalVariables();

    for (unsigned int i = 0; i < globals.getNumElements(); i++) {
        //        DIGlobalVariable global = (DIGlobalVariable)
        //        globals.getElement(i);
        DebugVariableGlobal *var =
            new DebugVariableGlobal(globals.getElement(i), alloc);
        globalVars.push_back(var);

        //        DIType t = global.getType().resolve(typeMap);
        //
        //        debugDatabase->addVariable(global.getName().str(), true, NULL,
        //        t,
        //                global.getDirectory().str() + "/" +
        //                global.getFilename().str(),
        //                global.getLineNumber());
        //
        //        RAM * ram = NULL;
        //        for (auto it = alloc->ram_begin(), e = alloc->ram_end(); it !=
        //        e; ++it) {
        //            if ((*it)->getValue() == global.getGlobal())
        //                ram = *it;
        //        }
    }

    //    NamedMDNode *metaGlobals =
    //    alloc->getModule()->getNamedMetadata("llvm.dbg.gv");
    //    if (metaGlobals) {
    //        for (unsigned i = 0, e = metaGlobals->getNumOperands(); i != e;
    //        ++i) {
    //            MDNode *metaGlobal = metaGlobals->getOperand(i);
    //
    //            DebugVariableGlobal *var = new DebugVariableGlobal(metaGlobal,
    //            alloc);
    //
    //
    //        }
    //    }

    /* Local vars */
    for (Allocation::hw_iterator hw_it = alloc->hw_begin();
         hw_it != alloc->hw_end(); hw_it++) {
        GenerateRTL *HW = (*hw_it);
        FiniteStateMachine *fsm = HW->getFSM();
        for (FiniteStateMachine::iterator state = fsm->begin();
             state != fsm->end(); state++) {

            // If there are multiple llvm.dbg.value statements for a
            // given variable in this state, we only care about the last one
            map<MDNode *, DbgValueInst *> lastDbgVal;

            for (State::iterator instr_it = state->begin();
                 instr_it != state->end(); instr_it++) {
                Instruction *insn = *instr_it;

                if (DbgDeclareInst *dbgDeclare =
                        dyn_cast<DbgDeclareInst>(insn)) {
                    MDNode *metaNode = dbgDeclare->getVariable();
                    DebugVariableLocal *var = HW->dbgGetVariable(metaNode);
                    var->addDbgDeclare(alloc, dbgDeclare);
                } else if (DbgValueInst *dbgValue =
                               dyn_cast<DbgValueInst>(insn)) {

                    // For now, we only handle offset == 0
                    assert(dbgValue->getOffset() == 0);

                    lastDbgVal[dbgValue->getVariable()] = dbgValue;
                }
            }

            for (map<MDNode *, DbgValueInst *>::iterator m = lastDbgVal.begin(),
                                                         end = lastDbgVal.end();
                 m != end; ++m) {

                DebugVariableLocal *var = HW->dbgGetVariable(m->first);
                var->addDbgValue(m->second);
            }
        }
    }
}

void LegUpDebugInfo::outputDebugDatabase() {
    std::string designPath;

    char buf[512];
    char *ret = getcwd(buf, sizeof(buf));
    assert(ret == buf);

    designPath = std::string(buf);

    debugDatabase = new DebugDatabase(this);
    debugDatabase->initialize();

    debugDatabase->dropDesign(designPath);
    debugDatabase->addDesign(designPath, moduleName);

    debugDatabase->setDesignProperties(
        optionInsertRtl, LEGUP_CONFIG->isXilinxBoard(),
        LEGUP_CONFIG->getFPGABoard(),
        alloc->getDataLayout()->getPointerSizeInBits(), alloc->getDataSize());

    if (optionInsertRtl) {
        debugDatabase->setRtlInstrumentationProperties(
            getInstanceIdBits(), getStateBits(), getSystemID());
    }

    debugDatabase->setTraceBufferProperties(
        traceCtrlWidth, optionCtrlSequenceBits, traceCtrlDepth, traceMemWidth,
        traceMemDepth, optionTraceRegs, traceRegsWidth, traceRegsDepth);

    // RAMs
    for (auto it = alloc->ram_begin(), e = alloc->ram_end(); it != e; ++it) {
        debugDatabase->addRAM(*it);
    }

    // Global Variables
    NamedMDNode *debug_metadata =
        alloc->getModule()->getNamedMetadata("llvm.dbg.cu");

    DITypeIdentifierMap typeMap = generateDITypeIdentifierMap(debug_metadata);

    debugDatabase->setTypeMap(typeMap);

    for (auto it = globalVars.begin(), e = globalVars.end(); it != e; ++it) {
        (*it)->addToDebugDatabase(debugDatabase, alloc, typeMap);
    }

    // Functions
    DebugInfoFinder finder;
    finder.processModule(*(alloc->getModule()));
    for (auto i = finder.subprograms().begin(), e = finder.subprograms().end();
         i != e; ++i) {
        DISubprogram s(*i);
        Function *F = s.getFunction();
        GenerateRTL *hw = NULL;
        //        dbgs() << "About to add " << s.getName().str() << "\n";
        if (F) {
            hw = alloc->getGenerateRTL(F);
            //            dbgs() << "has hw " <<
            //            hw->getFunction()->getName().str() << "\n";
        }

        debugDatabase->addFunction((MDNode *)s, hw);
    }

    // Functions without debug metadata (legup_memset_, etc)
    // If the function has already been added with metadata it will be ignored.
    for (auto hw_it = alloc->hw_begin(); hw_it != alloc->hw_end(); hw_it++) {
        debugDatabase->addFunction(NULL, *hw_it);
    }

    // States
    // IR Instructions
    for (auto hw_it = alloc->hw_begin(); hw_it != alloc->hw_end(); hw_it++) {
        debugDatabase->addStates(*hw_it);
        debugDatabase->addIRInstructions(*hw_it);
        debugDatabase->addRtlSignals(*hw_it, traceScheduler);
    }

    // Local variables
    for (auto hw_it = alloc->hw_begin(); hw_it != alloc->hw_end(); hw_it++) {
        GenerateRTL *HW = *hw_it;
        for (auto var_it = HW->getDbgVars()->begin(),
                  var_end = HW->getDbgVars()->end();
             var_it != var_end; ++var_it) {
            DebugVariableLocal *var = *var_it;
            var->addToDebugDatabase(debugDatabase, alloc, typeMap,
                                    traceScheduler);
        }
    }

    // Instances
    for (auto it = instances.begin(), e = instances.end(); it != e; ++it) {
        RTLModuleInstance *instance = *it;
        debugDatabase->addInstance(instance->getGenRtl(), instance->getId());
    }
    for (auto it = instances.begin(), e = instances.end(); it != e; ++it) {
        RTLModuleInstance *instance = *it;
        for (auto it_child = instance->getChildren()->begin(),
                  end_children = instance->getChildren()->end();
             it_child != end_children; ++it_child) {
            RTLModuleInstance *childInstance = *it_child;
            debugDatabase->addInstanceChild(instance->getId(),
                                            childInstance->getId());
        }
    }
}

void LegUpDebugInfo::outputVariableStats() {
    const char *SEP = "|";
    std::ofstream fp;

    Module *M = alloc->getModule();

    int num_vars = 0;
    float num_in_mem = 0;
    float num_constant = 0;
    float num_argument = 0;
    float num_addr = 0;
    float num_reg = 0;

    string module_name = M->getModuleIdentifier();
    string extension(".bc");
    if (stringEndsWith(module_name, extension)) {
        module_name.erase(module_name.end() - extension.length(),
                          module_name.end());
    }
    string var_filename(module_name + ".vars");
    fp.open(var_filename.c_str(), ios::out);

    NamedMDNode *md_sp = M->getNamedMetadata("llvm.dbg.sp");
    for (unsigned int i_sp = 0; i_sp < md_sp->getNumOperands(); i_sp++) {
        MDNode *md_function = md_sp->getOperand(i_sp);

        DISubprogram di_func(md_function);

        string func_name = di_func.getName().str();
        string md_lv_name = "llvm.dbg.lv." + func_name;
        NamedMDNode *md_lv = M->getNamedMetadata(md_lv_name);

        unsigned int num_local_vars = 0;
        if (md_lv) {
            num_local_vars = md_lv->getNumOperands();
        }

        fp << "Function" << SEP << func_name << SEP << num_local_vars << SEP
           << endl;

        for (unsigned int i = 0; i < num_local_vars; i++) {
            MDNode *md_node = md_lv->getOperand(i);

            DIVariable di_var(md_node);
            fp << "\t" << di_var.getName().str() << SEP;

            num_vars++;

            const DbgDeclareInst *DDI =
                LegUpDebugInfo::findDebugDeclare(alloc, md_node);
            vector<DbgValueInst *> DVIs =
                LegUpDebugInfo::findDebugValues(alloc, md_node);

            // Can't have variable in memory, plus DebugValue insns
            //			assert(!(DDI && DVIs.size()));

            float total = 0;
            if (DDI)
                total++;
            total += DVIs.size();

            if (DDI) {
                fp << "Mem" << SEP;
                num_in_mem += 1 / total;
            }

            if (DVIs.size()) {

                fp << "DVI" << SEP;

                for (vector<DbgValueInst *>::iterator it = DVIs.begin(),
                                                      ie = DVIs.end();
                     it != ie; it++) {

                    DbgValueInst *dvi = *it;

                    Value *val = dvi->getValue();

                    if (!val) {
                        fp << "UnknownNULL,";
                    } else if (isa<ConstantInt>(val)) {
                        /* Value is an integer */
                        num_constant += 1 / total;
                        fp << "ConstInt,";
                    } else if (isa<ConstantPointerNull>(val)) {
                        num_constant += 1 / total;
                        fp << "ConstNULL,";
                    } else if (isa<Argument>(val)) {
                        num_argument += 1 / total;
                        fp << "Arg,";
                    } else if (isa<ConstantExpr>(val)) {
                        num_addr += 1 / total;
                        fp << "ConstExpr,";
                    } else if (isa<Instruction>(val)) {
                        num_reg += 1 / total;
                        fp << "Reg,";
                    } else {
                        //						cout << "Invalid llvm.dbg.value
                        //type:
                        //"
                        //								<< val->getValueID() << SEP
                        //<<
                        // val->getNameStr()
                        //								<< endl;
                        assert(false);
                    }
                }
            }

            if (!DDI & !DVIs.size()) {
                fp << "Gone" << SEP;
            }
            fp << endl;
        }
    }

    fp << "Variables" << endl;
    fp << "Total Vars: " << num_vars << endl;
    fp << "Mem: " << num_in_mem << endl;
    fp << "Constant: " << num_constant << endl;
    fp << "Constant Arg: " << num_argument << endl;
    fp << "Addr: " << num_addr << endl;
    fp << "Reg: " << num_reg << endl;
    fp << (num_vars - num_in_mem - num_constant - num_argument - num_addr -
           num_reg) << endl;

    fp.close();
}

RTLModuleInstance *LegUpDebugInfo::newInstance(GenerateRTL *genRtl,
                                               int parentInst) {

    RTLModuleInstance *newInst =
        new RTLModuleInstance(this, genRtl, parentInst);
    instances.push_back(newInst);
    return newInst;
}

RTLModuleInstance *LegUpDebugInfo::getInstance(int instanceId) {
    RTLModuleInstance *ret = NULL;
    for (vector<RTLModuleInstance *>::iterator i = instances.begin(),
                                               i_end = instances.end();
         i != i_end; ++i) {
        if ((*i)->getId() == instanceId) {
            ret = *i;
            break;
        }
    }
    return ret;
}

RTLModuleInstance *LegUpDebugInfo::getInstance(string prefix) {
    RTLModuleInstance *ret = NULL;
    for (vector<RTLModuleInstance *>::iterator i = instances.begin(),
                                               i_end = instances.end();
         i != i_end; ++i) {
        if ((*i)->getHierarchyPath() == prefix) {
            ret = *i;
            break;
        }
    }
    return ret;
}

void LegUpDebugInfo::generateSystemID() {
    int x;

    srand(time(NULL));
    x = rand() & 0xff;
    x |= (rand() & 0xff) << 8;
    x |= (rand() & 0xff) << 16;
    x |= (rand() & 0xff) << 24;

    systemID = x;
}

void LegUpDebugInfo::addDebugRtl() {
    instanceBits = requiredBits(RTLModuleInstance::numInstances());

    /* Get # of state bits */
    int max_state = 0;
    for (Allocation::hw_iterator hw_it = alloc->hw_begin();
         hw_it != alloc->hw_end(); hw_it++) {
        int states = (*hw_it)->getFSM()->getNumStates();
        max_state = std::max(max_state, states);
    }
    stateBitsOneHot = max_state + 1;
    stateBits = requiredBits(max_state);

    for (Allocation::hw_iterator hw_it = alloc->hw_begin(),
                                 hw_end = alloc->hw_end();
         hw_it != hw_end; ++hw_it) {
        GenerateRTL *hw = *hw_it;
        hw->addDebugRtl();
    }

    if (optionPreserveOneHot)
        setupStateMuxer();

    if (optionTraceRegs)
        setupTraceScheduler();
    else
        traceRegsWidth = 0;

    // Calculate sizes of trace buffers
    sizeTraceBuffers();

    // We only use dual ported if 1/2 * s * w_r - (1-s) * w_idx > 0 (SEE PAPER)
    //    if (optionTraceRegs) {
    //        float s = 1 -
    //                  traceScheduler->getNumStatesWithTracingOverHalf() /
    //                      (float)traceScheduler->getNumStatesWithTracing();
    //        dbgs() << "s: " << s << "\n";
    ////        if (optionRegBufferDualPorted &&
    ////            !(0.5 * s * regsTraceBits - (1 - s) * 0 > 0)) {
    //        if (optionRegBufferDualPorted && !(s > 0)) {
    //            optionRegBufferDualPorted = false;
    //            sizeTraceBuffers();
    //        }
    //    }
}

void LegUpDebugInfo::sizeTraceBuffers() {

    if (optionSizeBufsSimulation) {
        // Buffer allocation method: Dynamic analsysi
        assert(fileExists("dbg_fill_rates.txt"));

        fstream fs;
        fs.open("dbg_fill_rates.txt", ios::in);
        string s;
        getline(fs, s);
        ctrlFillRate = atof(s.c_str());
        getline(fs, s);
        memFillRate = atof(s.c_str());
        getline(fs, s);
        regsFillRate = atof(s.c_str());
        fs.close();
    } else if (optionSizeBufsStaticAnalysis) {
        // Buffer allocation method: Static analysis
        ctrlFillRate = 0.21;
        memFillRate = numWrites / (float)numStates;
        regsFillRate =
            traceScheduler->getNumStatesWithTracing() / (float)numStates;
    } else {
        // Buffer allocation method: Constant factor
        ctrlFillRate = 0.21;
        memFillRate = 0.11;

        if (optionRegBufferDualPorted && optionTraceRegsDelayAll &&
            optionTraceRegsDelayAll) {
            regsFillRate = 0.34;
        } else if ((!optionRegBufferDualPorted) && optionTraceRegsDelayAll &&
                   optionTraceRegsDelayWorst) {
            regsFillRate = 0.32;
        } else if ((!optionRegBufferDualPorted) && (!optionTraceRegsDelayAll) &&
                   optionTraceRegsDelayWorst) {
            regsFillRate = 0.34;
        } else if ((!optionRegBufferDualPorted) && (!optionTraceRegsDelayAll) &&
                   (!optionTraceRegsDelayWorst)) {
            regsFillRate = 0.20;
        } else {
            assert(false);
        }
    }

    traceCtrlWidth = instanceBits + stateBits + optionCtrlSequenceBits;

    // 2 = number of bits for memory size signal
    traceMemWidth = 2 + alloc->getDataLayout()->getPointerSizeInBits() +
                    alloc->getDataSize();

    if (optionTraceRegs) {
        // We want to solve this equation:
        // ControlWidth * ControlDepth + MemWidth * MemDepth + RegsWidth *
        // RegsDepth = Total Mem Bits
        // Where: MemDepth / CtrlDepth = memFillRate / ctrlFillRate
        // and: RegsDepth / CtrlDepth = regsFillRate / ctrlFillRate

        traceCtrlDepth =
            (ctrlFillRate * optionTraceMemBits) /
            (ctrlFillRate * traceCtrlWidth + memFillRate * traceMemWidth +
             regsFillRate * traceRegsWidth);
        traceMemDepth = (traceCtrlDepth * memFillRate / ctrlFillRate);
        traceRegsDepth = (traceCtrlDepth * regsFillRate / ctrlFillRate);

        if (!traceRegsDepth)
            traceRegsDepth = 1;
    } else {
        // We want to solve this equation:
        // ControlWidth * ControlDepth + MemWidth * MemDepth  = Total Mem
        // Bits
        // Where: MemDepth / CtrlDepth = memFillRate / ctrlFillRate

        traceCtrlDepth =
            (ctrlFillRate * optionTraceMemBits) /
            (ctrlFillRate * traceCtrlWidth + memFillRate * traceMemWidth);
        traceMemDepth = (traceCtrlDepth * memFillRate / ctrlFillRate);
        traceRegsDepth = 0;
    }

    assert(traceCtrlDepth);
}

void LegUpDebugInfo::assignInstances() {
    Function *FpMain = alloc->getModule()->getFunction("main");
    GenerateRTL *main = alloc->getGenerateRTL(FpMain);

    main->generateInstances(0);
}

void LegUpDebugInfo::setupStateMuxer() {
    assert(!stateMuxer);
    stateMuxer = new RTLModule("dbgStateMuxer");

    Function *FpMain = alloc->getModule()->getFunction("main");
    GenerateRTL *main = alloc->getGenerateRTL(FpMain);

    RTLSignal *sigCurState = stateMuxer->addOut(DEBUG_SIGNAL_NAME_CURRENT_STATE,
                                                RTLWidth(stateBits));
    RTLSignal *sigActiveInst = stateMuxer->addIn(DEBUG_SIGNAL_NAME_ACTIVE_INST,
                                                 RTLWidth(instanceBits));

    vector<RTLDebugPort *> *statePorts = main->getDbgStatePorts();

    RTLConst *ZERO = stateMuxer->addConst("0");
    RTLConst *ONE = stateMuxer->addConst("1");

    for (vector<RTLDebugPort *>::iterator port_it = statePorts->begin(),
                                          port_end = statePorts->end();
         port_it != port_end; ++port_it) {
        RTLDebugPort *port = *port_it;
        RTLSignal *signal = port->getSignal();

        RTLSignal *in =
            stateMuxer->addIn(signal->getName(), signal->getWidth());

        int inWidth = in->getWidth().numBits(stateMuxer, alloc);
        int inEncodedWidth = requiredBits(inWidth - 1);

        // Encode the state signal
        RTLSignal *inEncoded = stateMuxer->addWire(in->getName() + "_encoded",
                                                   RTLWidth(inEncodedWidth));
        inEncoded->setDefaultDriver(ZERO);

        for (int i = 0; i < inWidth; ++i) {
            RTLOp *bit = stateMuxer->addOp(RTLOp::Trunc);
            bit->setCastWidth(RTLWidth(IntToString(i), IntToString(i)));
            bit->setOperand(0, in);

            RTLOp *eq = stateMuxer->addOp(RTLOp::EQ);
            eq->setOperand(0, bit);
            eq->setOperand(1, ONE);

            RTLConst *driver =
                stateMuxer->addConst(IntToString(i), RTLWidth(inEncodedWidth));
            inEncoded->addCondition(eq, driver);
        }

        port->populateSourceInstance(this);

        RTLOp *op = stateMuxer->addOp(RTLOp::EQ);
        op->setOperand(0, sigActiveInst);

        assert(port->getInstance());
        int instanceId = port->getInstance()->getId();
        assert(instanceId >= 0);
        op->setOperand(
            1, new RTLConst(IntToString(instanceId), requiredBits(instanceId)));

        sigCurState->addCondition(op, inEncoded);
    }
}

void LegUpDebugInfo::setupTraceScheduler() {
    assert(!traceSchedulerRtl);
    traceSchedulerRtl = new RTLModule("traceScheduler");

    Function *FpMain = alloc->getModule()->getFunction("main");
    GenerateRTL *main = alloc->getGenerateRTL(FpMain);

    vector<RTLDebugPort *> *mainTracePorts = main->getDbgTracePorts();

    /* Add register inputs */
    schedulerInputs.clear();
    for (vector<RTLDebugPort *>::iterator port_it = mainTracePorts->begin(),
                                          port_end = mainTracePorts->end();
         port_it != port_end; ++port_it) {
        RTLDebugPort *port = *port_it;
        RTLSignal *signal = port->getSignal();
        RTLSignal *in =
            traceSchedulerRtl->addIn(signal->getName(), signal->getWidth());

        RTLDebugPort *newPort = new RTLDebugPort(port, in);
        newPort->populateSourceInstance(this);
        schedulerInputs.push_back(newPort);
    }

    /* Add state inputs */
    RTLSignal *activeInst = traceSchedulerRtl->addIn(
        DEBUG_SIGNAL_NAME_ACTIVE_INST, RTLWidth(instanceBits));

    RTLSignal *sigCurState = NULL;
    map<RTLModuleInstance *, RTLSignal *> stateForInstance;
    if (optionPreserveOneHot) {
        vector<RTLDebugPort *> *statePorts = main->getDbgStatePorts();
        for (vector<RTLDebugPort *>::iterator port_it = statePorts->begin(),
                                              port_end = statePorts->end();
             port_it != port_end; ++port_it) {
            RTLDebugPort *port = *port_it;
            RTLSignal *signal = port->getSignal();
            RTLSignal *in =
                traceSchedulerRtl->addIn(signal->getName(), signal->getWidth());

            assert(port->getInstance());
            stateForInstance[port->getInstance()] = in;
        }
    } else {
        sigCurState = traceSchedulerRtl->addIn(DEBUG_SIGNAL_NAME_CURRENT_STATE,
                                               RTLWidth(stateBits));
    }

    RTLSignal *out = traceSchedulerRtl->addOut(DEBUG_SIGNAL_NAME_TRACE_REGS);

    string outEnSigName = DEBUG_SIGNAL_NAME_TRACE_REGS_EN;
    RTLSignal *outEnA = traceSchedulerRtl->addOut(outEnSigName + "_a");
    RTLSignal *outEnB = traceSchedulerRtl->addOut(outEnSigName + "_b");

    RTLConst *zero = traceSchedulerRtl->addConst("0");
    RTLConst *one = traceSchedulerRtl->addConst("1");

    outEnA->setDefaultDriver(zero);
    outEnB->setDefaultDriver(zero);

    traceScheduler = new TraceScheduler(this);

    // Schedule tracing
    if (optionTraceRegsDelayWorst) {
        traceScheduler->scheduleDelayFixWorst();
    }
    if (optionTraceRegsDelayAll) {
        traceScheduler->scheduleDelayFixAverage();
    }

    regsTraceBits = traceScheduler->getWorstCaseWidth();
    if (optionRegBufferDualPorted)
        traceRegsWidth = (regsTraceBits + 1) / 2;
    else
        traceRegsWidth = regsTraceBits;

    TraceScheduler::function_mapping_t *schedule =
        traceScheduler->getSchedule();

    // Loop through modules
    for (auto hw_it = schedule->begin(), hw_end = schedule->end();
         hw_it != hw_end; ++hw_it) {
        GenerateRTL *genRtl = hw_it->first;

        // Loop through instances
        auto instances = genRtl->getInstances();
        for (auto i_it = instances->begin(), i_end = instances->end();
             i_it != i_end; ++i_it) {
            RTLModuleInstance *inst = *i_it;

            RTLOp *op1 = new RTLOp(RTLOp::EQ);
            op1->setOperand(0, activeInst);
            op1->setOperand(
                1, traceSchedulerRtl->addConst(IntToString(inst->getId()),
                                               RTLWidth(instanceBits)));

            // Loop through states
            for (auto s_it = hw_it->second.begin(), s_end = hw_it->second.end();
                 s_it != s_end; ++s_it) {

                // If state has nothing assigned to it, then skip it
                if (s_it->second->getWidth() == 0)
                    continue;

                State *state = s_it->first;
                int stateNum = genRtl->getFSM()->getStateNum(state);

                RTLOp *op2 = new RTLOp(RTLOp::EQ);
                if (optionPreserveOneHot) {
                    RTLOp *bit = new RTLOp(RTLOp::Trunc);
                    bit->setCastWidth(
                        RTLWidth(IntToString(stateNum), IntToString(stateNum)));
                    bit->setOperand(0, stateForInstance[inst]);

                    op2->setOperand(0, bit);
                    op2->setOperand(1, new RTLConst("1"));
                } else {
                    op2->setOperand(0, sigCurState);
                    op2->setOperand(
                        1, traceSchedulerRtl->addConst(IntToString(stateNum),
                                                       RTLWidth(stateBits)));
                }

                RTLOp *op3 = new RTLOp(RTLOp::And);
                op3->setOperand(0, op1);
                op3->setOperand(1, op2);

                // Loop through signals assigned this state
                auto signals = s_it->second->getSignals();
                int width = 0;
                RTLOp *opConcat = NULL;
                RTLSignal *lastConcat = NULL;
                RTLSignal *lastSignal = NULL;

                assert(signals->size());
                for (auto sig_it = signals->begin(), sig_e = signals->end();
                     sig_it != sig_e; ++sig_it) {
                    DebugScheduledSignal *portScheduled = *sig_it;

                    RTLSignal *signal;
//                    if (portScheduled->getDelay())
//                        signal = portScheduled->getTracedSignal()
//                                     ->getPortReg(inst)
//                                     ->getSignal();
//                    else
                    signal = portScheduled->getTracedSignal()->getPort(inst)->getSignal();

                    int signalWidth =
                        signal->getWidth().numBits(traceSchedulerRtl, alloc);

                    portScheduled->setLo(width);
                    portScheduled->setHi(width + signalWidth - 1);

                    width += signalWidth;

                    if (lastSignal) {
                        opConcat = new RTLOp(RTLOp::Concat);
                        opConcat->setOperand(0, signal);
                        opConcat->setOperand(1, lastConcat);
                        lastConcat = opConcat;
                    } else {
                        lastConcat = signal;
                    }
                    lastSignal = signal;
                }

                RTLSignal *driver = NULL;
                if (opConcat) {
                    driver = opConcat;
                } else {
                    driver = lastSignal;
                }
                assert(driver);
                out->addCondition(op3, driver);

                outEnA->addCondition(op3, one);
                if (s_it->second->getWidth() > traceRegsWidth) {
                    assert(optionRegBufferDualPorted);
                    outEnB->addCondition(op3, one);
                }
            }
        }
    }

    // Can't have 0 width
    int outWidth = max(regsTraceBits, 1);
    out->setWidth(RTLWidth(outWidth));
}

DebugTracedSignal *TraceScheduler::findTracedSignal(RTLSignal *sig) {
    for (auto i = tracedSignals.begin(), e = tracedSignals.end(); i != e; ++i) {
        if (((*i)->getSignal() == sig))
            return *i;
    }
    return NULL;
}

std::vector<std::pair<State *, DebugScheduledSignal *>> *
TraceScheduler::findDbgValueScheduling(DebugValue *dbgVal) {
    GenerateRTL *hw = dbgVal->getVariable()->getGenRtl();
    string sigName = dbgVal->getSignalName();

    std::vector<std::pair<State *, DebugScheduledSignal *>> *ret =
        new std::vector<std::pair<State *, DebugScheduledSignal *>>;

    // Find the right genRtl
    for (auto i_it = mapping.begin(), i_end = mapping.end(); i_it != i_end;
         ++i_it) {
        if (i_it->first != hw)
            continue;

        // Loop through states
        for (auto s_it = i_it->second.begin(), s_end = i_it->second.end();
             s_it != s_end; ++s_it) {

            // Loop through signals
            auto signals = s_it->second->getSignals();
            for (auto sig_it = signals->begin(), sig_end = signals->end();
                 sig_it != sig_end; ++sig_it) {
                DebugScheduledSignal *sig = *sig_it;

                if (sig->getTracedSignal()->getSignal()->getName() == sigName) {
                    ret->push_back(std::make_pair(s_it->first, sig));
                }
            }
        }
    }
    assert(ret->size());
    return ret;
}

RTLDebugPort *LegUpDebugInfo::findTracePort(RTLModuleInstance *instance,
                                            std::string sigName) {
    for (auto it = schedulerInputs.begin(), e = schedulerInputs.end(); it != e;
         ++it) {
        RTLDebugPort *port = *it;
        assert(port->getInstance());

        if (port->getInstance() != instance)
            continue;

        if (port->getSourceSignal()->getName() == sigName) {
            return port;
            break;
        }
    }
    return NULL;
}

int TraceScheduler::getWorstCaseWidth() {
    int width = 0;
    for (auto hw_it = mapping.begin(), hw_end = mapping.end(); hw_it != hw_end;
         ++hw_it) {
        for (auto s_it = hw_it->second.begin(), s_end = hw_it->second.end();
             s_it != s_end; ++s_it) {
            width = std::max(width, s_it->second->getWidth());
        }
    }
    return width;
}

TraceScheduler::TraceScheduler(LegUpDebugInfo *dbgInfo)
    : dbgInfo(dbgInfo), bitsToTrace(0) {

    // Need to find all the signals to be traced,
    // as well as the associated state to trace them in.

    // Loop through modules
    for (auto hw_it = dbgInfo->getAlloc()->hw_begin(),
              hw_end = dbgInfo->getAlloc()->hw_end();
         hw_it != hw_end; ++hw_it) {
        GenerateRTL *genRtl = *hw_it;

        // Loop through local variables
        auto vars = genRtl->getDbgVars();
        for (auto var_it = vars->begin(), var_end = vars->end();
             var_it != var_end; ++var_it) {
            DebugVariableLocal *var = *var_it;

            // Loop through DebugValues for each variable
            auto varValues = var->getDbgValues();
            for (auto val_it = varValues->begin(), val_end = varValues->end();
                 val_it != val_end; ++val_it) {
                DebugValue *val = *val_it;

                if (val->requiresSignalTracing()) {
                    string sigName = val->getSignalName();
                    State *state = val->getRecordState();
                    assert(state);

                    auto instances = genRtl->getInstances();
                    assert(instances->size() >= 1);

                    // Look through the ports, and find the signal
                    // Each signal may have multiple ports (1 per instance)
                    // so we assert that all instances point to the same signal.
                    RTLSignal *sig = NULL;
                    for (auto i_it = instances->begin(),
                              i_end = instances->end();
                         i_it != i_end; ++i_it) {
                        RTLDebugPort *port =
                            dbgInfo->findTracePort(*i_it, sigName);
                        assert(port);

                        if (sig)
                            assert(sig == port->getSourceSignal());
                        else
                            sig = port->getSourceSignal();
                    }

                    DebugTracedSignal *tracedSignal = findTracedSignal(sig);
                    if (!tracedSignal) {
                        // We need to create the DebugTracedSignal object
                        tracedSignal = new DebugTracedSignal(
                            sig, sig->getWidth().numBits(
                                     dbgInfo->getTraceSchedulerRtl(),
                                     dbgInfo->getAlloc()));
                        for (auto i_it = instances->begin(),
                                  i_end = instances->end();
                             i_it != i_end; ++i_it) {
                            RTLDebugPort *port =
                                dbgInfo->findTracePort(*i_it, sigName);
                            assert(port);
                            tracedSignal->setPort(*i_it, port);

//                            RTLDebugPort *portReg =
//                                dbgInfo->findTracePort(*i_it, sigName + "_reg");
//                            assert(portReg || val->isArgument());
//
//                            if (portReg) {
//                                tracedSignal->setPortReg(*i_it, portReg);
//                            } else {
//                                tracedSignal->setPortReg(*i_it, port);
//                            }
                        }
                        tracedSignals.push_back(tracedSignal);
                    }

                    // Check if we are already tracing this signal in this state
                    TraceState *ts = getTraceState(genRtl, state);
                    auto signals = ts->getSignals();
                    bool found = false;
                    for (auto sig_it = signals->begin(),
                              sig_end = signals->end();
                         sig_it != sig_end; ++sig_it) {
                        if ((*sig_it)->getTracedSignal() == tracedSignal) {
                            found = true;
                            break;
                        }
                    }

                    if (!found) {
                        DebugScheduledSignal *scheduledSignal =
                            new DebugScheduledSignal(this, tracedSignal, 0);
                        ts->addSignal(scheduledSignal);
                    }
                }
            }
        }
    }

    // Count how many bits we are tracing
    for (instance_iterator i_it = mapping.begin(), i_end = mapping.end();
         i_it != i_end; ++i_it) {
        for (state_iterator s_it = i_it->second.begin(),
                            s_end = i_it->second.end();
             s_it != s_end; ++s_it) {
            TraceState *ts = s_it->second;
            this->bitsToTrace += ts->getWidth();
        }
    }
}

// void TraceScheduler::getSchedule(map<
//    RTLModuleInstance *, map<State *, vector<DebugPort *>>> *scheduledSignals)
//    {
//    for (instance_iterator i = mapping.begin(), e = mapping.end(); i != e;
//         ++i) {
//        for (state_iterator s = i->second.begin(), s_end = i->second.end();
//             s != s_end; ++s) {
//            assert(s->second);
//            vector<DebugPort *> assignments;
//            s->second->getAssignments(&assignments);
//            if (assignments.size())
//                (*scheduledSignals)[i->first][s->first] = assignments;
//        }
//    }
//}

bool TraceScheduler::widestTraceState(GenerateRTL **genRtl, State **state) {
    GenerateRTL *widestGenRtl = NULL;
    State *widestState = NULL;
    bool secondWidestExists = false;
    int maxWidth = 0;

    for (auto i = mapping.begin(), e = mapping.end(); i != e; ++i) {
        for (auto s = i->second.begin(), s_end = i->second.end(); s != s_end;
             ++s) {

            // Make sure the TraceState has been allocated
            assert(s->second);

            if (s->second->getWidth() > maxWidth) {
                if (maxWidth > 0)
                    secondWidestExists = true;
                maxWidth = s->second->getWidth();
                widestGenRtl = i->first;
                widestState = s->first;
            } else if (s->second->getWidth() > 0 &&
                       s->second->getWidth() < maxWidth) {
                secondWidestExists = true;
            }
        }
    }
    *genRtl = widestGenRtl;
    *state = widestState;
    return secondWidestExists;
}

State *TraceScheduler::getNextState(State *state, unsigned i) {
    State *nextState;

    assert(i < state->getNumTransitions());

    if (i == 0)
        nextState = state->getDefaultTransition();
    else
        nextState = state->getTransitionState(i - 1);

    assert(nextState);

    if (nextState->getName() == "LEGUP_0")
        nextState = NULL;

    return nextState;
}

bool TraceScheduler::delayTraceState(TraceState *ts, int incomingWidth,
                                     bool commit,
                                     std::set<State *> checkedStates) {
    GenerateRTL *genRtl = ts->getGenRtl();
    State *thisState = ts->getState();
    checkedStates.insert(thisState);

    bool canMove = true;

    // Get an item to move
    DebugScheduledSignal *signalToMove = ts->getSignalToDelay();
    if (!signalToMove) {
        // There is nothing to move
        canMove = false;
    }

    // Make sure we are not moving past the max delay
    if (TraceScheduler::max_delay >= 0 &&
        ((signalToMove->getDelay() + 1) > TraceScheduler::max_delay)) {
        canMove = false;
    }

    // Check if we can move it
    if (canMove) {
        int width = signalToMove->getWidth();
        if (dbgInfo->getOptionPrintDelayedTracingDbg()) {
            cout << "\tChecking move from state " << thisState->getName()
                 << "\n";
            cout << "\t\tSignal: "
                 << signalToMove->getTracedSignal()->getSignal()->getName()
                 << " Delay: " << signalToMove->getDelay()
                 << " Width: " << width << ".\n";
        }

        int thisStateWidth = getTraceState(genRtl, thisState)->getWidth();

        // Check if this next state has enough room for what we want to move
        bool nextStateExists = false;
        for (unsigned int i = 0; i < thisState->getNumTransitions(); ++i) {
            State *nextState = getNextState(thisState, i);

            if (!nextState)
                continue;

            nextStateExists = true;

            if (nextState == thisState) {
                cout << "State " << thisState->getName()
                     << " transitions to itself.\n";
                assert(false);
            }

            if (nextState->getPredecessors()->size() > 1) {
                if (dbgInfo->getOptionPrintDelayedTracingDbg()) {
                    cout << "\t" << nextState->getName()
                         << " has multiple predecessors\n";
                }
                canMove = false;
                continue;
            }

            TraceState *nextTS = getTraceState(genRtl, nextState);
            assert(nextTS);

            if (nextTS->getWidth() + width >= thisStateWidth + incomingWidth) {
                // The next state is already too full, so we will check if that
                // state
                // can be made smaller though delaying

                // First we check if have have already visited that state (we
                // don't want
                // to get stuck in a recursive loop)
                //                if (checkedStates.find(nextState) !=
                //                checkedStates.end()) {
                //                    cout << "\tLeaving recursive loop\n";
                //                    canMove = false;
                //                    break;
                //                }
                if (!delayTraceState(nextTS, width, false, checkedStates)) {
                    if (dbgInfo->getOptionPrintDelayedTracingDbg()) {
                        cout << "\t" << nextState->getName()
                             << " already has width " << nextTS->getWidth()
                             << " and cannot be delayed\n";
                    }
                    canMove = false;
                    continue;
                }
            }
        }

        if (!nextStateExists)
            canMove = false;
    }

    /* Perform the move */
    if (canMove && commit) {
        if (dbgInfo->getOptionPrintDelayedTracingDbg()) {
            cout << "\tMoving "
                 << signalToMove->getTracedSignal()->getSignal()->getName()
                 << "\n";
        }

        for (unsigned int i = 0; i < thisState->getNumTransitions(); ++i) {
            State *moveTo;
            if (i == 0)
                moveTo = thisState->getDefaultTransition();
            else
                moveTo = thisState->getTransitionState(i - 1);

            if (moveTo == thisState)
                continue;

            DebugScheduledSignal *newPortDelay = new DebugScheduledSignal(
                signalToMove, signalToMove->getDelay() + 1);
            getTraceState(genRtl, moveTo)->addSignal(newPortDelay);
            if (dbgInfo->getOptionPrintDelayedTracingDbg()) {
                cout << "\t\tMoving to " << moveTo->getName() << "\n";
            }
        }
        getTraceState(genRtl, thisState)->removeSignal(signalToMove);
        delete signalToMove;
    }
    return canMove;
}

void TraceScheduler::scheduleDelayFixWorst() {

    if (dbgInfo->getOptionPrintDelayedTracingDbg()) {
        cout << "Delayed Tracing Scheduling Fix Worst-Case State\n";
    }

    bool delayed = true;
    while (delayed) {
        // Find max state
        GenerateRTL *genRtl;
        State *fromState;

        if (!widestTraceState(&genRtl, &fromState)) {
            if (dbgInfo->getOptionPrintDelayedTracingDbg()) {
                cout << "No second widest.  Ending schedule.\n";
            }
            break;
        } else if (fromState->getCalledFunction()) {
            if (dbgInfo->getOptionPrintDelayedTracingDbg()) {
                cout << "Widest is function call. Cannot stall.\n";
            }
            break;
        }

        TraceState *ts = getTraceState(genRtl, fromState);

        unsigned int fromStateWidth = ts->getWidth();
        if (dbgInfo->getOptionPrintDelayedTracingDbg()) {
            cout << "Widest state: " << fromState->getName() << " has width "
                 << fromStateWidth << "\n";
        }

        for (auto it = ts->getSignals()->begin(), e = ts->getSignals()->end();
             it != e; ++it) {
            if (dbgInfo->getOptionPrintDelayedTracingDbg()) {
                dbgs() << "Signal "
                       << (*it)->getTracedSignal()->getSignal()->getName()
                       << " " << (*it)->getTracedSignal()->getWidth() << "\n";
            }
        }

        set<State *> checkedStates;
        delayed = delayTraceState(ts, 0, true, checkedStates);
    }
}

int TraceScheduler::worstCaseWidth() {
    int w = 0;
    for (instance_iterator i_it = mapping.begin(), i_end = mapping.end();
         i_it != i_end; ++i_it) {
        for (state_iterator s_it = i_it->second.begin(),
                            s_end = i_it->second.end();
             s_it != s_end; ++s_it) {
            w = max(w, getTraceState(i_it->first, s_it->first)->getWidth());
        }
    }
    return w;
}

TraceState *TraceScheduler::getTraceState(GenerateRTL *genRtl, State *state) {
    if (!mapping[genRtl][state])
        mapping[genRtl][state] = new TraceState(this, genRtl, state);

    return mapping[genRtl][state];
}

unsigned TraceScheduler::getNumStatesWithTracing() {
    unsigned cnt = 0;

    for (instance_iterator i_it = mapping.begin(), i_end = mapping.end();
         i_it != i_end; ++i_it) {
        for (state_iterator s_it = i_it->second.begin(),
                            s_end = i_it->second.end();
             s_it != s_end; ++s_it) {
            TraceState *ts = s_it->second;
            if (ts->getWidth()) {
                cnt++;
                if (ts->getWidth() > dbgInfo->getRegsBufferWidth()) {
                    cnt++;
                }
            }
        }
    }
    return cnt;
}

unsigned TraceScheduler::getNumStatesWithTracingOverHalf() {
    unsigned cnt = 0;

    for (instance_iterator i_it = mapping.begin(), i_end = mapping.end();
         i_it != i_end; ++i_it) {
        for (state_iterator s_it = i_it->second.begin(),
                            s_end = i_it->second.end();
             s_it != s_end; ++s_it) {
            TraceState *ts = s_it->second;
            if (ts->getWidth() > (dbgInfo->getRegsTraceBits() + 1) / 2)
                cnt++;
        }
    }
    return cnt;
}

void TraceScheduler::scheduleDelayFixAverage() {
    // Find the worst case
    int worstWidth = worstCaseWidth();

    if (dbgInfo->getOptionPrintDelayedTracingDbg()) {
        cout << "Delayed Tracing: Fixing average case (worst case is "
             << worstWidth << ")\n";
    }

    for (auto hw_it = mapping.begin(), hw_end = mapping.end(); hw_it != hw_end;
         ++hw_it) {
        GenerateRTL *getRtl = hw_it->first;
        bool somethingMoved = true;
        while (somethingMoved) {

            somethingMoved = false;

            // Loop through states until we find something to move
            for (auto s_it = hw_it->second.begin(), s_end = hw_it->second.end();
                 s_it != s_end; ++s_it) {
                State *state = s_it->first;
                TraceState *ts = s_it->second;

                // Get width
                int width = ts->getWidth();

                if (width == 0 || width == worstWidth)
                    continue;

                if (dbgInfo->getOptionPrintDelayedTracingDbg()) {
                    cout << "Considering moving everything from state "
                         << state->getName() << "\n";
                }

                // Need to find states we can move to
                map<State *, int> moveToStates;
                bool validMove = true;
                bool setChanged = true;

                // Add the initial successor states to the delay map
                for (unsigned i = 0; i < state->getNumTransitions(); i++) {
                    State *nextState = getNextState(state, i);
                    if (!nextState)
                        continue;
                    //                    if (nextState->getName() == "LEGUP_0")
                    //                    {
                    //                        validMove = false;
                    //                        break;
                    //                    }
                    if (nextState == state) {
                        cout << "Transition to self (" << state->getName()
                             << ")\n";
                        //                        assert(false);
                        validMove = false;
                    }

                    // Add it
                    moveToStates[nextState] = 1;
                }

                if (validMove) {
                    if (moveToStates.size() == 0) {
                        if (dbgInfo->getOptionPrintDelayedTracingDbg()) {
                            cout << "\tCan't, no valid successor states\n";
                        }
                        validMove = false;
                    } else {
                        if (dbgInfo->getOptionPrintDelayedTracingDbg()) {
                            cout << "\tChecking " << moveToStates.size()
                                 << " successor states\n";
                        }
                    }
                }

                // Continue to check successor states, and if they are
                // unassigned (width == 0),
                // then consider their successor states.  This ends once we find
                // a successor that is invalid,
                // or we find candidate successors.
                while (validMove && setChanged) {
                    setChanged = false;

                    // Visit each state
                    for (map<State *, int>::iterator
                             delay_map_it = moveToStates.begin(),
                             delay_map_end = moveToStates.end();
                         delay_map_it != delay_map_end; ++delay_map_it) {

                        // We mark the delay as -1 if the state has been
                        // replaced by its successors, no need
                        // to visit it again
                        if (delay_map_it->second == -1)
                            continue;

                        State *nextState = delay_map_it->first;

                        if (dbgInfo->getOptionPrintDelayedTracingDbg()) {
                            cout << "\tSuccessor " << nextState->getName()
                                 << " ";
                        }

                        // We can't move to states with multiple predecessors
                        if (nextState->getPredecessors()->size() > 1) {
                            if (dbgInfo->getOptionPrintDelayedTracingDbg()) {
                                cout << "has multiple preds.\n";
                            }
                            validMove = false;
                            break;
                        }

                        int nextWidth =
                            getTraceState(getRtl, nextState)->getWidth();

                        if (nextWidth == 0) {
                            // We can't move here, but perhaps we can move to
                            // the successor state(s)
                            bool nextNextStateExists = false;
                            if (dbgInfo->getOptionPrintDelayedTracingDbg()) {
                                cout << "is empty, checking successors.\n";
                            }
                            for (unsigned i = 0;
                                 i < nextState->getNumTransitions(); i++) {
                                State *nextNextState =
                                    getNextState(nextState, i);
                                if (!nextNextState)
                                    continue;
                                if (nextState->getName() == "LEGUP_0") {
                                    validMove = false;
                                    break;
                                }
                                nextNextStateExists = true;
                                if (nextNextState == nextState) {
                                    cout << "Transition to self\n";
                                    assert(false);
                                    validMove = false;
                                    break;
                                }

                                // Make sure this state isn't already in the
                                // set, otherwise we've made a loop
                                if (moveToStates.find(nextNextState) !=
                                    moveToStates.end()) {
                                    validMove = false;
                                } else {
                                    // Add it
                                    moveToStates[nextNextState] =
                                        moveToStates[nextState] + 1;
                                    if (dbgInfo
                                            ->getOptionPrintDelayedTracingDbg()) {
                                        cout << "\tAdding successor "
                                             << nextNextState->getName()
                                             << "\n";
                                    }
                                }
                            }
                            if (!nextNextStateExists) {
                                if (dbgInfo
                                        ->getOptionPrintDelayedTracingDbg()) {
                                    cout << "\tNo successors.\n";
                                }
                                validMove = false;
                                break;
                            }

                            moveToStates[nextState] = -1;
                            setChanged = true;
                            break;

                        } else if (nextWidth + width > worstWidth) {
                            if (dbgInfo->getOptionPrintDelayedTracingDbg()) {
                                cout << "is too wide.\n";
                            }
                            // We can't move here, it would make a new
                            // worst-case width
                            validMove = false;
                            break;
                        } else {
                            if (dbgInfo->getOptionPrintDelayedTracingDbg()) {
                                cout << "is OK.\n";
                            }
                        }
                    }
                }

                if (validMove) {
                    somethingMoved = true;

                    if (dbgInfo->getOptionPrintDelayedTracingDbg()) {
                        cout << "Moving everything from state "
                             << ts->getState()->getName()
                             << " (width: " << ts->getWidth() << ")\n";
                    }

                    for (map<State *, int>::iterator
                             delay_map_it = moveToStates.begin(),
                             delay_map_end = moveToStates.end();
                         delay_map_it != delay_map_end; ++delay_map_it) {

                        if (delay_map_it->second == -1)
                            continue;
                        assert(delay_map_it->second >= 1);

                        TraceState *newTS =
                            getTraceState(getRtl, delay_map_it->first);

                        // Make sure we don't violate the worst-case width
                        assert(newTS->getWidth() + ts->getWidth() <=
                               worstWidth);

                        // Add everything from the trace state to the new trace
                        // state
                        auto signals = ts->getSignals();
                        for (auto sig_it = signals->begin(),
                                  sig_end = signals->end();
                             sig_it != sig_end; ++sig_it) {
                            DebugScheduledSignal *sig = *sig_it;
                            DebugScheduledSignal *newSig =
                                new DebugScheduledSignal(
                                    sig,
                                    sig->getDelay() + delay_map_it->second);
                            newTS->addSignal(newSig);
                        }

                        if (dbgInfo->getOptionPrintDelayedTracingDbg()) {
                            cout << "\tTo state "
                                 << delay_map_it->first->getName()
                                 << " (new width: " << newTS->getWidth()
                                 << ", delay: " << delay_map_it->second
                                 << ") \n";
                        }
                    }

                    // Remove everything from the trace state
                    ts->deleteAssignments();
                }
            } // state loop
        }     // while(somethingMoved)
    }         // instance loop
}

int TraceState::getWidth() {
    int w = 0;
    for (auto i = signals.begin(), e = signals.end(); i != e; ++i) {
        w += (*i)->getWidth();
    }
    return w;
}

// void TraceState::getAssignments(vector<DebugPort *> *ports) {
//    ports->clear();
//
//    for (vector<DebugPortScheduled *>::iterator i = assigned.begin(),
//                                            e = assigned.end();
//         i != e; ++i) {
//        ports->push_back((*i)->port);
//    }
//}

void TraceState::removeSignal(DebugScheduledSignal *signal) {
    assert(signal);

    auto i = std::find(signals.begin(), signals.end(), signal);

    assert(i != signals.end());

    signals.erase(i);
}

void TraceState::deleteAssignments() {
    auto sig_it = signals.begin();

    while (sig_it != signals.end()) {
        delete *sig_it;
        sig_it = signals.erase(sig_it);
    }
}

DebugScheduledSignal *TraceState::getSignalToDelay() {
    if (signals.size() == 0)
        return NULL;
    else
        return signals[0];
}

void TraceState::addSignal(DebugScheduledSignal *signal) {
    // Make sure we aren't already tracing this signal
    for (auto i = signals.begin(), e = signals.end(); i != e; ++i)
        assert((*i)->getTracedSignal() != signal->getTracedSignal());

    signals.push_back(signal);
    std::sort(signals.begin(), signals.end());
}

unsigned DebugScheduledSignal::getWidth() const {
    return tracedSignal->getWidth();
}
