//===-- Allocation.h - Allocation Information -------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// The Allocation class stores the Rams, RTL, and GlobalNames for a circuit
//
//===----------------------------------------------------------------------===//

#ifndef LEGUP_ALLOCATION_H
#define LEGUP_ALLOCATION_H

//#include "llvm/Analysis/ProfileInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Instructions.h"
#include "GlobalNames.h"
#include "GenerateRTL.h"
#include "PADriver.h"
#include <list>

using namespace llvm;

namespace legup {

class RAM;
class PhysicalRAM;
class LegupConfig;
class RTLModule;
class LiveVariableAnalysis;
class ModuleInstantiationGraph;
class PropagatingSignals;
class LegUpDebugInfo;

/// Allocation - Stores the global information needed for the circuit
/// RTLModules, TargetData, GlobalNames, RAMs
/// @brief Allocation Class
class Allocation {
public:
    Allocation(Module *M);
    ~Allocation();

    GenerateRTL *createGenerateRTL(Function *F);

    /// verilogName - return the verilog variable name of a LLVM value
    std::string verilogName(const Value *val);

    /// scope specific to a function
    std::string verilogNameFunction(const Value *val, Function *F);

    const DataLayout* getDataLayout() const { return TD; }

    /// RTLModule iterator methods
    ///
    typedef std::list<RTLModule*> rtlListType;
    typedef rtlListType::iterator       rtl_iterator;
    typedef rtlListType::const_iterator const_rtl_iterator;

    inline rtl_iterator       rtl_begin()       { return rtlList.begin(); }
    inline const_rtl_iterator rtl_begin() const { return rtlList.begin(); }
    inline rtl_iterator       rtl_end  ()       { return rtlList.end();   }
    inline const_rtl_iterator rtl_end  () const { return rtlList.end();   }


    /// GenerateRTL iterator methods
    ///
    typedef std::list<GenerateRTL*> HwListType;
    typedef HwListType::iterator       hw_iterator;
    typedef HwListType::const_iterator const_hw_iterator;

    inline hw_iterator       hw_begin()       { return hwList.begin(); }
    inline const_hw_iterator hw_begin() const { return hwList.begin(); }
    inline hw_iterator       hw_end  ()       { return hwList.end();   }
    inline const_hw_iterator hw_end  () const { return hwList.end();   }

    /// RAM iterator methods
    ///
    typedef std::list<RAM*> RamListType;
    typedef RamListType::iterator       ram_iterator;
    typedef RamListType::const_iterator const_ram_iterator;

    inline ram_iterator       ram_begin()       { return ramList.begin(); }
    inline const_ram_iterator ram_begin() const { return ramList.begin(); }
    inline ram_iterator       ram_end  ()       { return ramList.end();   }
    inline const_ram_iterator ram_end  () const { return ramList.end();   }

    typedef std::list<PhysicalRAM*> PhyRamListType;
    typedef PhyRamListType::iterator       phy_ram_iterator;
    typedef PhyRamListType::const_iterator const_phy_ram_iterator;

    inline phy_ram_iterator       phy_ram_begin()       { return phyRamList.begin(); }
    inline const_phy_ram_iterator phy_ram_begin() const { return phyRamList.begin(); }
    inline phy_ram_iterator       phy_ram_end  ()       { return phyRamList.end();   }
    inline const_phy_ram_iterator phy_ram_end  () const { return phyRamList.end();   }

    /// global `define statements
    ///
    typedef std::map<std::string, std::string> DefineListType;
    typedef DefineListType::iterator       define_iterator;
    typedef DefineListType::const_iterator const_define_iterator;

    inline define_iterator       define_begin()       { return defineList.begin(); }
    inline const_define_iterator define_begin() const { return defineList.begin(); }
    inline define_iterator       define_end  ()       { return defineList.end();   }
    inline const_define_iterator define_end  () const { return defineList.end();   }

    /// setDefine - create a global `define
    void setDefine(std::string name, std::string value, std::string comment="") {
        assert(defineList.find(name) == defineList.end());
        defineList[name] = value;
        defineCommentList[name] = comment;
    }

    std::string getDefineComment(std::string name) const {
        const_define_iterator it = defineCommentList.find(name);
        if (it == defineCommentList.end()) return "";
        return it->second;
    }

    /// allocateRAM - create a RAM object from LLVM value
    RAM* allocateRAM(const Value *I);

    // local RAMs are instantiated inside a specific function
    void addLocalRam(Function *F, RAM *r);
    RAM* getLocalRamFromValue(Value *op);
    RAM *getLocalRamFromInst(Instruction *I);

    bool isRAMLocaltoFct(Function *F, RAM *r);

    // returns true if all memory accesses
    // are localized for this function
    bool fctOnlyUsesLocalRAMs(Function *F);

    // returns true if all memory accesses
    // are localized for all functions
    bool noSharedMemoryController();

    // global RAMs are shared in the central memory controller
    void addGlobalRam(RAM *r);

    bool isRAMGlobal(RAM *r) {
        return (isGlobalRams.find(r) != isGlobalRams.end());
    }

    /// getRAM - return the RAM object of a LLVM value
    RAM* getRAM(const Value *I);

    //NC changes
    int getRamTagNum(RAM* ram);
    
    int getRamTagNum(const Value *op);

    /// getNumRAMs - return the number of RAM objects
    unsigned getNumRAMs() const { return mapValueRam.size(); }

    unsigned getNumHWs() const { return hwList.size(); }

    // Tags:
    // 9 bits for a total of 512 (2^9) local memories
    unsigned getTagSize() const { return 9; }

    /// setNoDSPMult - changes multiplier data to non-DSP implementation
    std::string setNoDSPMult(std::string OpName);

    bool usesGenericRAMs() const { return genericRAMs; }

    bool stripRAM(const Value *R);

    GenerateRTL *getGenerateRTL(Function *F);

    Module *getModule() const { return module; }

    std::list<GenerateRTL::PATH*> *getOverallLongestPaths(){return &pathList;}

    void addRTL(RTLModule *rtl) { rtlList.push_back(rtl); }

    /// getRegCount - returns total bits used by variables declared
    int getRegCount(GenerateRTL *hw);

    /// getVarCount - returns total variables declared
    int getVarCount(GenerateRTL *hw);

    /// addGlobalDefines - create global `define statements
    void addGlobalDefines();
    unsigned getDataSize() const { return dataSize; }

    // this calculates the actual number of FUs required post-scheduling
    void calculateRequiredFunctionalUnits();

    std::map <std::string, int> &getNumFuncUnits(Function *F)
        { return mapFunctionNumFUs[F]; }

    void addLVA(Function *F, LiveVariableAnalysis *LVA) { LVAmap[F] = LVA; }
    LiveVariableAnalysis* getLVA(Function *F) { return LVAmap[F]; }

    void addAA(AliasAnalysis *A) {AA = A;}
    AliasAnalysis* getAA() { return AA; }

    // LLVM profiling
    //void addPI(ProfileInfo *P);
    //ProfileInfo* getPI() { return PI; }
    int getTotalBasicBlockExecutions() { return profile_total_bb_executions; }
    float get_percentage_execution_for_BB(BasicBlock *BB);
    bool is_BB_executed_infrequently(BasicBlock *BB);
    void set_num_states_in_BB(BasicBlock *BB, int s) { BB_to_num_states[BB]=s; }
    int get_num_states_in_BB(BasicBlock *BB);
    void add_registered_instruction(Instruction *i, int s)
        { registered_instruction_to_state[i]=s; }
    int get_registered_instruction_state(Instruction *i);
    void set_register_type(Instruction *i, std::string s) 
        { registered_instruction_to_type[i]=s; }
    std::string get_register_type(Instruction *i);
    bool is_registered_instruction(Instruction *i);
    void add_multicycled_divider(Instruction *I) { multicycled_dividers.insert(I); }
    std::set<Instruction*> get_multicycled_dividers() { return multicycled_dividers; }

    // Loop Info
    void addLI(Function *F, LoopInfo *li) { LImap[F] = li;}
    LoopInfo* getLI(Function *F) { return LImap[F]; }

    // log files
    raw_fd_ostream &getMemoryFile() { return memoryFile; }
    raw_fd_ostream &getBindingFile() { return bindingFile; }
    raw_fd_ostream &getPatternFile() { return patternFile; }
    raw_fd_ostream &getSchedulingFile() { return schedulingFile; }
    raw_fd_ostream &getMultipumpingFile() { return multipumpingFile; }
    raw_fd_ostream &getPipelineDotFile() { return pipelineDotFile; }
    raw_fd_ostream &getPipeliningRTLFile() { return pipeliningRTLFile; }
    raw_fd_ostream &getGenerateRTLFile() { return generateRTLFile; }
    raw_fd_ostream &getTimingReportFile() { return TimingReportFile; }

    bool useExplicitDSP(Instruction *I);
    //void getSynchronizationUsage (std::map<std::string, std::set<std::string> > &syncMap) const;
    void getSynchronizationUsage (std::map<std::string, int> &syncMap) const;


    std::map<RAM*, Function*> isLocalFunctionRam;
    std::set<RAM*> isGlobalRams;

    bool structsExistInCode();

    // Setter and Getter for a datastructure that allows a function and a module to be paired
    //
    void setModuleForFunction(RTLModule *m, const Function *f) { modulesForFunctions[f] = m; }
    RTLModule *getModuleForFunction(const Function *f);

    /// ------------------------------------------------------------------------
    // Debugger related functions
    /// ------------------------------------------------------------------------
    LegUpDebugInfo *getDbgInfo() { return dbgInfo; }

    unsigned getNumInstancesforFunction(unsigned numInstance, const Function *F,
                                        std::set<const Function *> &visited);

    // Propagating Signals Functions
    PropagatingSignals *getPropagatingSignals() { return propagatingSignals; }

private:
  void allocateGlobalVarRAMs();
  void allocateStackRAMs();
  void allocateLocalRAMs();
  void runPointsToAnalysis();
  void visitLocalMemoryInst(Instruction *I);
  void allocatePhysicalRAMs();
  void calculateMinMaxFUs(FiniteStateMachine *fsm,
                          std::map<std::string, int> &minFUs,
                          std::map<std::string, int> &maxFUs);
    void restrictToMaxDSPs(
        std::map <Function *, std::map <std::string, int> > mapFunctionMinFUs,
        std::map <Function *, std::map <std::string, int> > mapFunctionMaxFUs);

    std::map<Function*, GenerateRTL*> mapFctModule;
    std::map<const Value*, RAM*> mapValueRam;
    std::map<RAM*, int> mapRamTag;
    HwListType hwList;
    rtlListType rtlList;
    RamListType ramList;
    PhyRamListType phyRamList;
    DefineListType defineList;
    DefineListType defineCommentList;
    bool genericRAMs;
    int ramTagNum;

    // map which associates a function to all of the RAMs
    // it needs to connect to
    std::map<Function *, std::set<RAM *>> functionToRams;

    std::set<Function *> functionsWithNoPointsToSet;

    Module *module;
    std::map<Function *, GlobalNames> functionScope;
    GlobalNames globalScope;
    const DataLayout *TD;
    unsigned dataSize;
    std::string FileError;
    std::map<Function *, std::map<std::string, int>> mapFunctionNumFUs;
    std::map<Function *, LiveVariableAnalysis *> LVAmap;
    std::map<Function *, LoopInfo *> LImap;
    AliasAnalysis *AA;
    PADriver *pointsToAnalysis;
    /// ------------------------------------------------------------------------
    // LLVM Profiling-related members
    /// ------------------------------------------------------------------------

    // ProfileInfo pass
    //ProfileInfo *PI;
    
    // Cached statistic used to calculate execution frequencies
    int profile_total_bb_executions;
    
    // Number of states in each BB
    std::map< BasicBlock*, int > BB_to_num_states; 
    
    // These 3 are for flows where registers are moved from data paths.
    
    // Some instructions must have registers (e.g. loads, return instructions,
    // and for now instructions used across BB). Store these instructions which
    // must have registers, as well as their state #, and their type. Type can
    // be e.g. ret, load, cross_bb
    std::map< Instruction*, int > registered_instruction_to_state;
    std::map< Instruction*, std::string > registered_instruction_to_type;
    
    // All the multi-cycle dividers
    std::set< Instruction* > multicycled_dividers;
    /// ------------------------------------------------------------------------
    
    
    raw_fd_ostream memoryFile, bindingFile, patternFile, schedulingFile,
                   multipumpingFile, pipelineDotFile, pipeliningRTLFile,
                   TimingReportFile, generateRTLFile;
    std::map<Function*, std::set<Instruction*> > same_inputs;
    void detect_multipliers_with_identical_inputs();
    std::list<GenerateRTL::PATH*> pathList;
    std::map<Instruction*, RAM*> instToRam;
    std::map<Value*, RAM*> valueToRam;
    std::map<unsigned, PhysicalRAM *> physicalRAMs, physicalROMs;
    std::map<const Function *, RTLModule *> modulesForFunctions;

    LegUpDebugInfo *dbgInfo;

    // Custom Verilog
    PropagatingSignals *propagatingSignals;

    // number of instances for a function
    std::map<const Value*, unsigned> functionNumInstances;
};

class PropagatingSignal {
public:

    // All public constructors take an RTLSignal.
    // 
    // The main constructor optionally takes a function
    // at which the signal should stop propagating and
    // exist as a wire.
    //
    PropagatingSignal(RTLSignal *sig, Function *finalFunction = 0)
    { init(false, false, finalFunction, sig); }

    // Constructor for signals that stop propagating at
    // the top level module.  This special case must be
    // handled because the top level module has no
    // function representation.
    //
    // Optionally takes a flag that will identify a
    // propagating signal as a memory signal.
    // Unfortunately, there is some special handling
    // for memory signals.
    //
    PropagatingSignal(RTLSignal *sig, bool stopsAtTopLevel, bool isMemory = false)
    { init(isMemory, stopsAtTopLevel, 0, sig); }

    PropagatingSignal(const PropagatingSignal &toCopy) {

	_isMemory = toCopy._isMemory;
	_stopsAtTopLevelModule = toCopy._stopsAtTopLevelModule;
	_finalFunction = toCopy._finalFunction;
	_merged = toCopy._merged;
	_pthreadsSignal = toCopy._pthreadsSignal;

	assert(toCopy._signal);
	_signal = toCopy._signal;

	// For debugging
	getName();

        _widthOverriden = toCopy._widthOverriden;
	if (_widthOverriden)
	    _overridenWidth = RTLWidth(toCopy._overridenWidth.getHi(), toCopy._overridenWidth.getLo());
	else
	    _overridenWidth = RTLWidth();

    }

    // These methods are hacks that allow the memory signal
    // code to be replaced with propagating signal code
    //
    bool isMemory() { return _isMemory; }
    void setMemory(bool flag) { _isMemory = flag; }

    // Set this for any signals that will be wires at the top
    // level module (as opposed to external I/O)
    //
    bool stopsAtTopLevelModule() { return _stopsAtTopLevelModule; }
    void setStopsAtTopLevelModule(bool flag) { _stopsAtTopLevelModule = flag; }

    // The function at which the propagating signal will stop
    // propagating and become a wire
    //
    void setFinalFunction(Function *f) { _finalFunction = f; }
    bool stopsPropagatingAtFunction(Function *f) { assert(f); return _finalFunction == f; }

    // Sets the underlying RTLSignal
    //
    void setSignal(RTLSignal *sig) { _signal = sig; }
    RTLSignal *getSignal() { assert(_signal); return _signal; }

    // Propagating signals can "merge" with other signals
    // The merging should not change the actual (RTL) signal,
    // instead, we will allow the width of the signal to
    // appear as the merged width once the signals have
    // merged.
    void overrideWidth(RTLWidth width) { assert(_signal);  _overridenWidth = width; _widthOverriden = true; }
    RTLWidth getWidth() { assert(_signal); return _widthOverriden?_overridenWidth:_signal->getWidth(); }

    // Convenience method for getting the name of the
    // underlying (RTL) signal
    //
    std::string getName();

    // Hack to patch propagating memory signal issue with
    // parallel flow
    // TODO
    void setShouldConnectToPthreadsMemName(bool pthreadsMemSignal) { _pthreadsSignal = pthreadsMemSignal; }
    bool shouldConnectToPthreadsMemName() { return _pthreadsSignal; }
    std::string getPthreadsMemSignalName();

    // Convenience method for getting the type of the
    // underlying (RTL) signal
    //
    std::string getType() { assert(_signal); return _signal->getType(); }

    // Signal could be a merge between two signals of the
    // same name.
    //
    void setMerged(bool merged = true) { _merged = merged; }
    bool isMerged() { return _merged; }

    bool operator==(const PropagatingSignal &rhs) {
	return (_signal->getName() == rhs._signal->getName());
    }

private:

    PropagatingSignal(){}

    bool _isMemory;
    bool _stopsAtTopLevelModule;
    Function *_finalFunction;
    RTLSignal *_signal;

    bool _merged;

    bool _widthOverriden;
    RTLWidth _overridenWidth;

    bool _pthreadsSignal;

    void init(bool mem, bool stop, Function *func, RTLSignal *sig) {
	_isMemory = mem;
	_stopsAtTopLevelModule = stop;
	_finalFunction = func;
	_signal = sig;
	_widthOverriden = false;
	_overridenWidth = RTLWidth();
	_merged = false;
	_pthreadsSignal = false;
    }

};

class FunctionWithSignals {
public:
    friend class PropagatingSignals;
    bool operator<(const FunctionWithSignals& rhs) const {
	return name < rhs.name;
    }
    bool operator<(const std::string& rhs) const {
	return name < rhs;
    }
    bool operator==(const FunctionWithSignals& rhs) const {
	return name == rhs.name;
    }
    
private:
    std::string name;
    std::vector<PropagatingSignal> signals;
};

class PropagatingSignals {
public:
    friend class Allocation;
    void addPropagatingSignalToFunctionNamed(std::string name, PropagatingSignal &signal);
    void addPropagatingSignalsToFunctionNamed(std::string name, std::vector<PropagatingSignal> &signals);
    std::vector<PropagatingSignal *> getPropagatingSignalsForFunctionNamed(std::string name);
    std::vector<PropagatingSignal *> getPropagatingSignalsForFunctionsWithNames(std::vector<std::string> names);
    bool functionUsesMemory(std::string name);

private:
    PropagatingSignals(){}
    PropagatingSignals(PropagatingSignals const&){}
    PropagatingSignals &operator=(PropagatingSignals const&){ return *this; }

    std::vector<FunctionWithSignals *>::iterator functionWithSignalsInVector(std::string name);
    std::vector<PropagatingSignal *> referenceVectorForSignalVector(std::vector<PropagatingSignal> &sigVec);
    void mergePropagatingSignalsWithExistingSignalsInVector(std::vector<PropagatingSignal> &signals,
							    std::vector<PropagatingSignal *> &vector);
    
    std::vector<FunctionWithSignals *> functionsAndSignals;    
    
};
} // End legup namespace

#endif
