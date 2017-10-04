//===-- Ram.h -------------------------------------------------*- C++ -*-===//
//===-- Ram.h -------------------------------------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// RAM represents a ram holding an LLVM Value object
//
//===----------------------------------------------------------------------===//

#ifndef LEGUP_RAM_H
#define LEGUP_RAM_H

#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/raw_ostream.h"
#include "Allocation.h"
#include <stack>

using namespace llvm;


namespace legup {

class PhysicalRAM;

//NC changes...
class DebugType;

/// RAM represents a ram holding an LLVM Value object
/// @brief Legup RAM Representation
class RAM {
public:
    RAM(const Value *I, Allocation *alloc);

    /// getName - get the verilog name of the ram
    std::string getName() const;

    /// setName - set the verilog name of the ram
    void setName(std::string _name);

    /// getTag - get the name of the tag `define variable
    std::string getTag() const {
        std::string ret = "TAG_";
        // add prefix for global variables
        if (isa<GlobalVariable>(value)) {
            ret += "g_";
        }

        // FIX - need the actual RAM verilog name to get the right tag
        // not the SIMPLE_MEM assigned name
        //ret += getName();
        ret += alloc->verilogName(value); 

        return  ret;
    }

    std::string getTagAddrName() const {
        assert(getScope() == GLOBAL);
        return getTag() + "_a";
    }

    std::string getTagAddr() const;

    /// getValue - get the underlying LLVM Value stored in this ram
    const Value* getValue() const { return value; }

    /// getAddrWidth - get the ram address width in bits
    unsigned getAddrWidth() const { return addresswidth; }

    /// getDataWidth - get the ram data width in bits
    unsigned getDataWidth() const { return datawidth; }

    /// getElements - number of words in the ram
    unsigned getElements() const { return elements; }

    // return the address alignment requirement (only for
    // GROUP_RAMS_SIMPLE_OFFSET)
    unsigned getAddressAlignment() { return alignment; }
    void setAddressAlignment(unsigned a) { alignment = a; }

    /// getNumInstances - number of instances of the ram
    /// there may be multiple instances if it is used
    /// by multiple threads
    unsigned getNumInstances() const { return numInstances; } 

    //NC changes...
    std::vector<APInt> getInitial() const { return initial; }
    std::vector<APFloat> getFPInitial() const { return FPinitial; }    
    std::vector<std::string> getDebugInitialValues() const {return debugInitialValues; }
    //
    /// getMifFileName - name of the mif file initialization
    std::string getMifFileName() const { return getName() + ".mif"; }

    /// getInitializer - get the initializer if the ram stores a global
    /// variable
    const Constant* getInitializer() const { return initializer; }

    /// isStruct() - return if the RAM uses byte enables (ie for structs)
    bool isStruct() const { return IsStruct; }

    /// generateMIF() - generate MIF file for RAM
    static void generateMIFHeader(raw_fd_ostream &File, int depth, int width);
    //raw_fd_ostream &generateMIFHeader() const;
    //void generateMIF() const;
    void generateMIFContent(raw_fd_ostream &File, std::string name);
    static void generateMIFFooter(raw_fd_ostream &File);
    void initializeToZero();

    int getLatency(Allocation *alloc);

    // does the RAM hold constant data?
    bool isROM() {
        if (LEGUP_CONFIG->getParameterInt("NO_ROMS")) {
            return false;
        }
        return ROM;
    }
    
    void setROM(bool rom) { ROM = rom; }

    /// set the number of intances of the ram
    /// there may be multiple instances if it is used
    /// by multiple threads
    void setNumInstances(unsigned num) { numInstances = num; }

    enum SCOPE {
        // stored in a local block ram inside a function
        LOCAL,
        // stored in the global memory controller
        GLOBAL
    };

    SCOPE getScope() const { return scope; }
    void setScope(SCOPE _scope) { scope = _scope; }

    PhysicalRAM *getPhysicalRAM() { return phyRAM; }
    void setPhysicalRAM(PhysicalRAM *p) { phyRAM = p; }
    
    //NC changes...
    DebugType* debugType;
    
private:
    
    //NC changes...
    std::map<Type*, DebugType*> typesToDebugTypes;
    //
    void initializeStruct();
    void initialize();
    void buildInitializer();
	// TODO: LLVM 3.4 update: may be ConstantArray or ConstantDataArray
    //void initializeArray(const ConstantArray *C);
    void initializeArray(const Constant *C);
    void initializeConstantExpr(const ConstantExpr *expr);

    APInt getPointerFromGEP(const ConstantExpr *expr);
void visitConstantStruct(
        const ConstantStruct *cs,
        std::stack<const Constant*> &lifo,
        std::stack<unsigned> &elemLifo,
        std::stack<unsigned> &structOffset,
        unsigned &elemCount,
        unsigned &offset
        );

void visitConstantArray(
        const ConstantArray *ca,
        std::stack<const Constant*> &lifo,
        std::stack<unsigned> &elemLifo,
        std::stack<unsigned> &structOffset,
        unsigned &elemCount,
        unsigned &offset
        );

void visitConstantDataArray(
        const ConstantDataArray *ca,
        std::stack<const Constant*> &lifo,
        std::stack<unsigned> &elemLifo,
        std::stack<unsigned> &structOffset,
        unsigned &elemCount,
        unsigned &offset
        );

    //NC changes
    std::string convertSmallVectorToString(SmallVectorImpl<char>& v);
    void insertDebugInitialValue(const APInt& intValue);
    void insertDebugInitialValue(const APFloat& floatValue);
    void insertDebugInitialValue(std::string str);
    void getRAMStructure(); 
    void findStructure(Type* T);

    uint64_t getConstantVal(const Constant *c, unsigned offset, uint64_t val);
    void getRAMType();
    int getStructElements(StructType *STy);
    void getRAMTypeForArray(ArrayType *ATy);
    void getRAMTypeForStruct(StructType *STy);
void visitConstant(
        const Constant *c,
        uint64_t *val,
        std::stack<const Constant*> &lifo,
        std::stack<unsigned> &elemLifo,
        std::stack<unsigned> &structOffset,
        unsigned &elemCount,
        unsigned &offset
        );

const Value *value;
bool IsStruct;
unsigned elements, addresswidth, datawidth;
unsigned alignment;
std::vector<APInt> initial;
std::vector<APFloat> FPinitial;
Allocation *alloc;
    const Constant *initializer;
    std::string name;
    bool ROM;
    SCOPE scope;
    PhysicalRAM *phyRAM;
    // if RAM is used by multiple threads
    // multiple instances are created
    unsigned numInstances;   
    
    //NC changes   
    std::vector<std::string> debugInitialValues;
};

/// PhysicalRAM represents a ram holding multiple other rams with the same
//  bitwidth
//  RAMs are grouped by bitwidth: 8, 16, 32, 64
class PhysicalRAM {
  public:
    PhysicalRAM() : numwords(0) {}

    // assign a RAM into this physical RAM
    void addRAM(RAM *r, formatted_raw_ostream &out);

    // for a RAM contained in this physical ram.
    // Returns the offset in bytes for address lookup of the ram
    unsigned getOffset(const RAM *r) {
        assert(ramOffset.find(r) != ramOffset.end());
        return ramOffset[r];
    }

    // the total number of words in this physical ram (including all rams)
    unsigned getNumWords() { return numwords; }

    // get/set the ram data width in bits
    unsigned getDataWidth() const { return datawidth; }
    void setDataWidth(unsigned d) { datawidth = d; }

    // get/set the physical ram name
    std::string getName() const { return name; }
    void setName(std::string n) { name = n; }

    // get/set the physical ram tag
    int getTag() const { return tag; }
    void setTag(int t) { tag = t; }

    /// RAM iterator methods
    ///
    typedef std::vector<RAM *> RamListType;
    typedef RamListType::iterator ram_iterator;
    typedef RamListType::const_iterator const_ram_iterator;

    inline ram_iterator       ram_begin()       { return ramList.begin(); }
    inline const_ram_iterator ram_begin() const { return ramList.begin(); }
    inline ram_iterator       ram_end  ()       { return ramList.end();   }
    inline const_ram_iterator ram_end() const { return ramList.end(); }

    bool isStruct() const { return IsStruct; }

    void staticMemoryAllocation(formatted_raw_ostream &out);

  private:
    unsigned numwords;
    RamListType ramList;
    std::map<const RAM *, unsigned> ramOffset;
    int tag;
    bool IsStruct;
    std::string name;
    unsigned elements, addresswidth, datawidth;
    int width;
    bool ROM;
};
} // End legup namespace

#endif
