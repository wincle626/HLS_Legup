#ifndef LEGUP_DEBUG_H
#define LEGUP_DEBUG_H

#include "DebugVariable.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/IntrinsicInst.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <set>

using namespace llvm;

namespace legup {

class Allocation;
class RAM;
class RTLModule;
class RTLSignal;
class LegUpDebugInfo;
class State;
class RTLModuleInstance;
class TraceScheduler;
class DebugDatabase;
class DebugTracedSignal;

#define DEBUG_HIERARCHY_PATH_SEP "$"
#define DEBUG_PARAM_NAME_PARENT_INST "DEBUG_PARENT_INSTANCE"
#define DEBUG_PARAM_NAME_INST "DEBUG_INSTANCE"
#define DEBUG_SIGNAL_NAME_ACTIVE_INST "dbg_active_instance"
#define DEBUG_SIGNAL_NAME_CURRENT_STATE "dbg_current_state"
#define DEBUG_VERILOG_FUNC_NAME_INSTANCE_MAPPING "myInstance"
#define DEBUG_SIGNAL_NAME_TRACE_REGS "dbg_regs_trace_data"
#define DEBUG_SIGNAL_NAME_TRACE_REGS_EN "dbg_regs_trace_en"

class RTLDebugPort {
  private:
    RTLSignal *signal;
    GenerateRTL *genRtl;
    RTLSignal *sourceSignal;
    RTLModuleInstance *instance;

  public:
    RTLDebugPort(RTLSignal *signal, GenerateRTL *genRtl,
                 RTLSignal *sourceSignal)
        : signal(signal), genRtl(genRtl), sourceSignal(sourceSignal),
          instance(NULL) {}
    RTLDebugPort(RTLDebugPort *port, RTLSignal *signal)
        : signal(signal), genRtl(port->genRtl),
          sourceSignal(port->sourceSignal), instance(port->instance) {}

    RTLSignal *getSignal() { return signal; }
    RTLSignal *getSourceSignal() { return sourceSignal; }
    RTLModuleInstance *getInstance() { return instance; }

    void populateSourceInstance(LegUpDebugInfo *dbgInfo);

    std::string getHierarchyPath();
};

class DebugTracedSignal {
  private:
    std::map<RTLModuleInstance *, RTLDebugPort *> portMap;
    RTLSignal *signal;
    int width;

  public:
    DebugTracedSignal(RTLSignal *signal, int width)
        : signal(signal), width(width) {}

    RTLSignal *getSignal() { return signal; }

    RTLDebugPort *getPort(RTLModuleInstance *instance) {
        return portMap[instance];
    }
    void setPort(RTLModuleInstance *instance, RTLDebugPort *port) {
        portMap[instance] = port;
    }

    int getWidth() { return width; }
};

class DebugScheduledSignal {
  private:
    DebugTracedSignal *tracedSignal;
    unsigned int delay;
    unsigned int hi;
    unsigned int lo;

  public:
    DebugScheduledSignal(TraceScheduler *tss, DebugTracedSignal *tracedSignal,
                         unsigned delay)
        : tracedSignal(tracedSignal), delay(delay), hi(0), lo(0) {}
    DebugScheduledSignal(DebugScheduledSignal *source, unsigned int newDelay)
        : tracedSignal(source->tracedSignal), delay(newDelay), hi(source->hi),
          lo(source->lo) {}

    unsigned getWidth() const;

    bool operator<(const DebugScheduledSignal &port) const {
        return ((delay < port.delay) ||
                ((delay == port.delay) && (getWidth() < port.getWidth())));
    }

    unsigned int getHi() { return hi; }
    void setHi(unsigned int hi) { this->hi = hi; }

    unsigned int getLo() { return lo; }
    void setLo(unsigned int lo) { this->lo = lo; }

    DebugTracedSignal *getTracedSignal() { return tracedSignal; }
    //    void setPort(RTLDebugPort * port) { this->port = port; }

    unsigned int getDelay() { return delay; }
};

class RTLModuleInstance {
  private:
    static int nextId;
    LegUpDebugInfo *dbgInfo;
    int id;
    std::string hierarchyPath;
    GenerateRTL *genRtl;
    std::vector<RTLModuleInstance *> children;

  public:
    static int numInstances() { return nextId - 1; }

    RTLModuleInstance(LegUpDebugInfo *dbgInfo, GenerateRTL *genRtl,
                      int parentInstId);

    std::string getHierarchyPath() { return hierarchyPath; }
    GenerateRTL *getGenRtl() { return genRtl; }
    int getId() { return id; }

    std::string getChildHierarchyPath();
    void addChild(RTLModuleInstance *child);
    std::vector<RTLModuleInstance *> *getChildren() { return &children; }
};

class LegUpDebugInfo {

  public:
    typedef std::vector<RTLModuleInstance *> instance_listtype;
    typedef instance_listtype::iterator instance_iterator;

    inline instance_iterator instance_begin() { return instances.begin(); }
    inline instance_iterator instance_end() { return instances.end(); }

    typedef std::vector<RTLDebugPort *> port_listtype;
    typedef port_listtype::iterator port_iterator;

    inline port_iterator scheduler_ports_begin() {
        return schedulerInputs.begin();
    }
    inline port_iterator scheduler_ports_end() { return schedulerInputs.end(); }

    typedef std::vector<DebugVariableGlobal *> globalvars_listtype;
    typedef globalvars_listtype::iterator globalvars_iterator;

  private:
    static const int optionCtrlSequenceBits = 6;
    static const unsigned optionTraceMemBits = 100000;

    Allocation *alloc;

    DICompileUnit compileUnit;
    DebugDatabase *debugDatabase;

    /* Options */
    bool optionFillDatabase;
    bool optionInsertRtl;
    bool optionTraceRegs;
    bool optionRegBufferDualPorted;
    bool optionTraceRegsDelayWorst;
    bool optionTraceRegsDelayAll;
    bool optionPreserveOneHot;
    bool optionSizeBufsStaticAnalysis;
    bool optionSizeBufsSimulation;
    bool optionPrintDelayedTracingDebug;
    bool optionSupportReadFromMem;

    std::string moduleName;

    std::vector<DebugVariableGlobal *> globalVars;

    std::vector<RTLModuleInstance *> instances;

    unsigned int systemID;
    int instanceBits; // number of bits required to identify the active instance
    int stateBits;    // number of bits required to identify the current state,
                      // fully encoded
    int stateBitsOneHot; // number of bits required to identify the current
                         // state, one-hot encoding
    int regsTraceBits;

    //    int
    int traceCtrlDepth;
    int traceCtrlWidth;
    int traceCtrlAddrWidth;

    int traceMemDepth;
    int traceMemWidth;

    int traceRegsDepth;
    int traceRegsWidth;

    int numWrites;
    int numStates;

    float ctrlFillRate;
    float memFillRate;
    float regsFillRate;

    TraceScheduler *traceScheduler;
    RTLModule *traceSchedulerRtl;
    std::vector<RTLDebugPort *> schedulerInputs;

    RTLModule *stateMuxer;

    /////////// Functions //////////////
    static int getNextInstanceId();

    void setupTraceScheduler();
    void setupStateMuxer();
    void sizeTraceBuffers();

  public:
    static DbgDeclareInst *findDebugDeclare(Allocation *alloc, MDNode *md_node);
    static DbgDeclareInst *findDebugDeclareLocal(Function *Fp, MDNode *md_node);
    static std::vector<DbgValueInst *> findDebugValues(Allocation *alloc,
                                                       MDNode *md_node);
    static std::vector<DbgValueInst *> findDebugValues(Function *Fp,
                                                       MDNode *md_node);

    LegUpDebugInfo(Allocation *allocation);

    bool isDatabaseEnabled() { return optionFillDatabase; }
    bool isDebugRtlEnabled() { return optionInsertRtl; }
    bool isRegisterTracingEnabled() { return optionTraceRegs; }

    Allocation *getAlloc() { return alloc; }
    bool getOptionTraceRegs() { return optionTraceRegs; }
    bool getOptionPreserveOneHot() { return optionPreserveOneHot; }
    bool getOptionRegBufferDualPorted() { return optionRegBufferDualPorted; }
    bool getOptionSupportsReadFromMem() { return optionSupportReadFromMem; }
    bool getOptionPrintDelayedTracingDbg() {
        return optionPrintDelayedTracingDebug;
    }

    void generateVariableInfo();
    void analyzeProgram();

    void assignInstances(GenerateRTL *genRtl, std::string prefix);

    void outputDebugDatabase();
    void outputVariableStats();

    RTLModuleInstance *newInstance(GenerateRTL *genRtl, int parentInst);
    RTLModuleInstance *getInstance(int instanceId);
    RTLModuleInstance *getInstance(std::string prefix);

    int getInstanceIdBits();
    int getStateBits();
    int getStateBitsOneHot();
    int getTraceCtrlDepth() { return traceCtrlDepth; }
    int getTraceMemDepth() { return traceMemDepth; }
    int getTraceRegsDepth() { return traceRegsDepth; }
    int getRegsTraceBits() { return regsTraceBits; }
    int getRegsBufferWidth() { return traceRegsWidth; }

    unsigned int getSystemID() const { return systemID; }
    void generateSystemID();

    void assignInstances();
    void addDebugRtl();

    TraceScheduler *getTraceScheduler() { return traceScheduler; }
    RTLModule *getTraceSchedulerRtl() { return traceSchedulerRtl; }
    RTLModule *getStateMuxer() { return stateMuxer; }

    RTLDebugPort *findTracePort(RTLModuleInstance *instance,
                                std::string sigName);
};

class TraceState {
  private:
    TraceScheduler *tss;
    GenerateRTL *genRtl;
    State *state;
    std::vector<DebugScheduledSignal *> signals;

  public:
    TraceState(TraceScheduler *tss, GenerateRTL *genRtl, State *state)
        : tss(tss), genRtl(genRtl), state(state) {}

    State *getState() { return state; }
    int getWidth();

    void removeSignal(DebugScheduledSignal *signal);
    void addSignal(DebugScheduledSignal *signal);

    void deleteAssignments();

    DebugScheduledSignal *getSignalToDelay();

    GenerateRTL *getGenRtl() { return genRtl; }
    std::vector<DebugScheduledSignal *> *getSignals() { return &signals; }
};

class TraceScheduler {

  public:
    typedef std::map<State *, TraceState *> state_mapping_t;
    typedef state_mapping_t::iterator state_iterator;

    typedef std::map<GenerateRTL *, state_mapping_t> function_mapping_t;
    typedef function_mapping_t::iterator instance_iterator;

  private:
    static const int max_delay = -1;

    LegUpDebugInfo *dbgInfo;
    function_mapping_t mapping;
    unsigned bitsToTrace;

    bool delayTraceState(TraceState *ts, int incomingWidth, bool commit,
                         std::set<State *> checkedStates);
    int worstCaseWidth();
    State *getNextState(State *state, unsigned i);

    std::vector<DebugTracedSignal *> tracedSignals;

  public:
    TraceScheduler(LegUpDebugInfo *dbgInfo);

    bool widestTraceState(GenerateRTL **genRtl, State **state);
    int getWorstCaseWidth();

    void scheduleDelayFixWorst();
    void scheduleDelayFixAverage();

    function_mapping_t *getSchedule() { return &mapping; }

    unsigned getBitsToTrace() { return bitsToTrace; }
    unsigned getNumStatesWithTracing();
    unsigned getNumStatesWithTracingOverHalf();

    TraceState *getTraceState(GenerateRTL *genRtl, State *state);
    DebugTracedSignal *findTracedSignal(RTLSignal *sig);

    std::vector<std::pair<State *, DebugScheduledSignal *>> *
    findDbgValueScheduling(DebugValue *dbgVal);
};
}

#endif
