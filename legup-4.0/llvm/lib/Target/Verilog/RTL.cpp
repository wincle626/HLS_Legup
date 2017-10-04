//===-- RTL.cpp -----------------------------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// Implements RTL data structure
//
//===----------------------------------------------------------------------===//

#include "RTL.h"
#include "Allocation.h"
#include "VerilogWriter.h"
#include "LegupConfig.h"
#include "llvm/IR/InstIterator.h"
#include "MinimizeBitwidth.h"
#include <queue>

using namespace llvm;
using namespace legup;


namespace legup {

std::string RTLWidth::str() const {
    if (hi.empty()) {
        return "";
    }
    return "[" + hi + ":" + lo + "]";
}

// evaluates `MEMORY_CONTROLLER_ADDR_SIZE-1 into an integer
unsigned replaceParametersAndEval(std::string num, const RTLModule *rtl, const
        Allocation *alloc) {
    for (RTLModule::const_signal_iterator i = rtl->param_begin(), e =
            rtl->param_end(); i != e; ++i) {
        RTLSignal *param = *i;
        replaceAll(num, param->getName(), param->getValue());
    }
    for (Allocation::const_define_iterator i = alloc->define_begin(), e =
            alloc->define_end(); i != e; ++i) {
        std::string name = i->first;
        std::string value = i->second;
        replaceAll(num, "`"+name, value);
    }

    // handle simple subtraction. ie. num = "64-1" -> final = 63
    std::stringstream ss(num);
    std::string s;
    unsigned final = 0;
    while (getline(ss, s, '-')) {
        if (final == 0) {
            final = atoi(s.c_str());
        } else {
            final = final - atoi(s.c_str());
        }
    }

    return final;
}

RTLWidth::RTLWidth(const Value *I, MinimizeBitwidth *MBW) {
    bool USE_MB = LEGUP_CONFIG->getParameterInt("MB_MINIMIZE_HW") && 
                  MBW->bitwidthIsKnown(I);
    Type* T = I->getType();
    nativeHi = getMSBIndex(T);
    lo = nativeLo = "0";
    if(USE_MB) {
        unsigned minWidth = MBW->getMinBitwidth(I); 
    //    errs()<<"minWidth: "<<utostr(minWidth)<<"\n";
        if(minWidth>1) hi = utostr(minWidth-1);
        else hi = "";
        isSigned = MBW->isSigned(I);
    }
    else {
        hi = nativeHi;
        isSigned = false;
    }
    newStyleRTLWidth=true;
}


unsigned RTLWidth::numBits(const RTLModule *rtl, const Allocation *alloc) const {

    static std::map<std::string, unsigned> cache;
    std::string key = hi + lo;
    if (cache.find(key) != cache.end()) {
        return cache[key];
    }
    unsigned numBits = replaceParametersAndEval(hi, rtl, alloc) + 1 -
        replaceParametersAndEval(lo, rtl, alloc);

    cache[key] = numBits;

    return numBits;
}
unsigned RTLWidth::numNativeBits(const RTLModule *rtl, const Allocation *alloc) const {
    static std::map<std::string, unsigned> cache;
    std::string key = hi + lo;
    if (cache.find(key) != cache.end()) {
        return cache[key];
    }
    unsigned numBits = replaceParametersAndEval(nativeHi, rtl, alloc) + 1 -
        replaceParametersAndEval(nativeLo, rtl, alloc);

    cache[key] = numBits;

    return numBits;
}

bool RTLWidth::operator==(const RTLWidth &other) const {
    return (str() == other.str());
}

std::vector<RTLWidth> RTLWidth::widthsFromExclusionWithWidth(const RTLWidth &width) const {

    std::vector<RTLWidth> widths;
    
    unsigned hi = strToInt(this->hi);
    unsigned lo = strToInt(this->lo);
    unsigned inHi = strToInt(width.getHi());
    unsigned inLo = strToInt(width.getLo());

    unsigned temp = max(hi, lo);
    lo = min(hi, lo);
    hi = temp;

    temp = max(inHi, inLo);
    inLo = min(inLo, inHi);
    inHi = temp;

    if (lo < inLo) {
	RTLWidth loWidth(inLo, lo);
	widths.push_back(loWidth);
    }

    if (hi > inHi) {
	RTLWidth hiWidth(hi, inHi);
	widths.push_back(hiWidth);
    }
    
    return widths;
}

RTLSignal::RTLSignal() : bitwidth(RTLWidth()), driver(0), defaultDriver(0),
    checkXs(true) {}
RTLSignal::RTLSignal(std::string name, std::string value, RTLWidth bitwidth)
    : name(name), value(value), bitwidth(bitwidth), driver(0), defaultDriver(0),
    checkXs(true) {}


bool RTLSignal::isReg() const {
    return (getType() == "reg" || getType() == "output reg");
}

RTLOp::RTLOp(Instruction *instr) : castWidth(false) {
    if (isa<BinaryOperator>(instr)) {
        switch (instr->getOpcode()) {
            case Instruction::Add:  op = Add; break;
            case Instruction::Sub:  op = Sub; break;
            case Instruction::Mul:  op = Mul; break;
            case Instruction::SRem:
            case Instruction::URem: op = Rem; break;
            case Instruction::SDiv:
            case Instruction::UDiv: op = Div; break;
            case Instruction::And:  op = And; break;
            case Instruction::Or:   op = Or; break;
            case Instruction::Xor:  op = Xor; break;
            case Instruction::Shl:  op = Shl; break;
            // arithmetic shift right - MSB replaced with sign bit
            case Instruction::AShr:
            case Instruction::LShr: op = Shr; break;
            case Instruction::FAdd: op = FAdd; break;
            case Instruction::FMul: op = FMul; break;
            case Instruction::FSub: op = FSub; break;
            case Instruction::FDiv: op = FDiv; break;
            case Instruction::FRem:
                assert(0 && "Floating point operations not supported\n");
            default: llvm_unreachable("Invalid operator type!");
        }

    } else if (const ICmpInst *cmp = dyn_cast<ICmpInst>(instr)) {
        switch (cmp->getPredicate()) {
            case ICmpInst::ICMP_EQ:  op = EQ; break;
            case ICmpInst::ICMP_NE:  op = NE; break;
            case ICmpInst::ICMP_SLT:
            case ICmpInst::ICMP_ULT: op = LT; break;
            case ICmpInst::ICMP_SLE:
            case ICmpInst::ICMP_ULE: op = LE; break;
            case ICmpInst::ICMP_SGT:
            case ICmpInst::ICMP_UGT: op = GT; break;
            case ICmpInst::ICMP_SGE:
            case ICmpInst::ICMP_UGE: op = GE; break;
            default: llvm_unreachable("Illegal ICmp predicate");
        }
    } else if (const FCmpInst *fcmp = dyn_cast<FCmpInst>(instr)) {
        switch (fcmp->getPredicate()) {
            case FCmpInst::FCMP_OEQ:
            case FCmpInst::FCMP_UEQ: op = EQ; break;
            case FCmpInst::FCMP_ONE:
            case FCmpInst::FCMP_UNE: op = NE; break;
            case FCmpInst::FCMP_OLT:
            case FCmpInst::FCMP_ULT: op = LT; break;
            case FCmpInst::FCMP_OLE:
            case FCmpInst::FCMP_ULE: op = LE; break;
            case FCmpInst::FCMP_OGT:
            case FCmpInst::FCMP_UGT: op = GT; break;
            case FCmpInst::FCMP_OGE:
            case FCmpInst::FCMP_UGE: op = GE; break;
            default: llvm_unreachable("Illegal FCmp predicate");
        } 
    }
    else if(instr->getOpcode()==Instruction::Select) op=Sel;
    else {
		switch (instr->getOpcode()) {
			case Instruction::FPTrunc:	op = FPTrunc; break;
			case Instruction::FPExt:	op = FPExt; break;
			case Instruction::FPToSI:	op = FPToSI; break;
			case Instruction::SIToFP:	op = SIToFP; break;
			default:
		        errs() << "Unrecognized instruction: " << *instr << "\n";
        		llvm_unreachable(0);
		}
    }

}


void RTLSignal::connect(RTLSignal *s, Instruction *I) {
    //assert(*bitwidth == *s->getWidth() && "Bitwidths don't match!");
    //assert(drivers.empty());
    drivers.clear();
    conditions.clear();
    instrs.clear();
    instrs.push_back(I);
    drivers.push_back(s);
}

const std::string RTLSignal::getDriverBits() const {
  return driverBits;
}

void RTLSignal::setDriverBits(const std::string bits) {
  driverBits = bits;
}

void RTLSignal::setDriver(unsigned i, RTLSignal *s, Instruction *I /* = 0 */ ) {
    assert(i < drivers.size());
    drivers[i] = s;
    if (I) {
        instrs[i] = I;
    }
}

void RTLSignal::addCondition(RTLSignal *cond, RTLSignal *driver,
	     Instruction *instr, bool setToDriverBits) {
    conditions.push_back(cond);
    drivers.push_back(driver);
    instrs.push_back(instr);
    if (setToDriverBits)
      driver->setDriverBits(driver->getWidth().str());
}

RTLWidth RTLSignal::getWidth() const {
    return bitwidth;
}

void RTLSignal::setWidth(RTLWidth w) {
    bitwidth = w;
}


RTLModule::~RTLModule() {
    for (std::set<Cell*>::iterator i = cells.begin(),
            e = cells.end(); i != e; ++i) {
        Cell *c = *i;
        assert(c);
        for (std::vector<Pin*>::iterator i = c->inPins.begin(), e =
                c->inPins.end(); i != e; ++i) {
            assert(*i);
            delete *i;
        }
        for (std::vector<Pin*>::iterator i = c->outPins.begin(), e =
                c->outPins.end(); i != e; ++i) {
            assert(*i);
            delete *i;
        }
        delete(c);
    }

    for (std::set<Net*>::iterator i = nets.begin(),
            e = nets.end(); i != e; ++i) {
        assert(*i);
        delete *i;
    }

    for (std::set<RTLOp*>::iterator i = operations.begin(),
            e = operations.end(); i != e; ++i) {
        assert(*i);
        delete *i;
    }

    for (std::set<RTLConst*>::iterator i = constants.begin(),
            e = constants.end(); i != e; ++i) {
        assert(*i);
        delete *i;
    }

    //std::set<RTLWidth*> deleted;
    for (signal_iterator i = signals.begin(), e = signals.end(); i != e; ++i) {
        RTLSignal *s = *i;
        assert(s);
        //RTLWidth *w = s->getWidth();
        //assert(w);
        //if (deleted.count(w)) continue;
        //delete w;
        //deleted.insert(w);
        delete s;
    }
    for (signal_iterator i = port_begin(), e = port_end(); i != e; ++i) {
        assert(*i);
        delete *i;
    }
    for (signal_iterator i = param_begin(), e = param_end(); i != e;
            ++i) {
        assert(*i);
        delete *i;
    }
    for (module_iterator m = instances_begin(), me = instances_end(); m
            != me; ++m) {
        assert(*m);
        delete *m;
    }
}

RTLSignal *RTLModule::find(std::string signal) {
    RTLSignal *r = findExists(signal);
    if (!r) {
        errs() << "signal: " << signal << "\n";
        assert(r && "Couldn't find signal");
    }
    return r;
}

RTLSignal *RTLModule::findExists(std::string signal) {
    for (signal_iterator i = port_begin(), e = port_end(); i != e; ++i) {
        if ((*i)->getName() == signal) {
            return *i;
        }
    }
    for (signal_iterator i = signals.begin(), e = signals.end(); i != e; ++i) {
        if ((*i)->getName() == signal) {
            return *i;
        }
    }
    for (signal_iterator i = param_begin(), e = param_end(); i != e; ++i) {
        if ((*i)->getName() == signal) {
            return *i;
        }
    }
    return 0;
}



RTLSignal *RTLModule::addParam(std::string name, std::string value) {
    RTLSignal *param = new RTLSignal(name, value);
    param->setType("parameter");
    params.push_back(param);
    return param;
}

RTLSignal *RTLModule::addIn(std::string name, RTLWidth width) {
    RTLSignal *in = new RTLSignal(name, "", width);
    in->setType("input");
    ports.push_back(in);
    return in;
}

void remove_from_vector(std::string name, std::vector<RTLSignal*> &v) {
    std::vector<RTLSignal*>::iterator i = v.begin();
    while (i != v.end()) {
        if ((*i)->getName() == name) {
            i = v.erase(i);
        } else {
            ++i;
        }
    }
}

void RTLModule::remove(std::string name) {
    remove_from_vector(name, params);
    remove_from_vector(name, ports);
    remove_from_vector(name, signals);
}

RTLOp *RTLModule::addOp(RTLOp::Opcode opcode) {
    RTLOp *op = new RTLOp(opcode);
    operations.insert(op);
    return op;
}

RTLOp *RTLModule::addOp(Instruction *i) {
    RTLOp *op = new RTLOp(i);
    operations.insert(op);
    return op;
}

RTLSignal *RTLModule::addReg(std::string name, RTLWidth width) {
    if (exists(name)) {
        RTLSignal *s = find(name);
        //if (width) {
            s->setWidth(width);
        //}
        return s;
    }
    RTLSignal *s = new RTLSignal(name, "", width);
    signals.push_back(s);
    s->setType("reg");
    return s;
}

RTLSignal *RTLModule::addWire(std::string name, RTLWidth width) {
    if (exists(name)) {
        RTLSignal *s = find(name);
        //if (width) {
            s->setWidth(width);
        //}
        return s;
    }

    RTLSignal *s = new RTLSignal(name, "", width);
    signals.push_back(s);
    s->setType("wire");
    return s;
}

RTLConst *RTLModule::addConst(std::string value, RTLWidth width) {
    RTLConst *c = new RTLConst(value, width);
    constants.insert(c);
    return c;
}


RTLSignal *RTLModule::addOut(std::string name, RTLWidth width) {
    RTLSignal *out = new RTLSignal(name, "", width);
    out->setType("output");
    ports.push_back(out);
    return out;
}

RTLSignal *RTLModule::addOutReg(std::string name, RTLWidth width) {
    RTLSignal *out = new RTLSignal(name, "", width);
    out->setType("output reg");
    ports.push_back(out);
    return out;
}

RTLModule *RTLModule::addModule(std::string name, std::string instName) {
    RTLModule *mod = new RTLModule(name, instName);
    instances.push_back(mod);
    return mod;
}


RTLModule::Cell *RTLModule::newCell(std::string name) {
    Cell *c = new Cell(name);
    cells.insert(c);
    return c;
}

RTLModule::Cell *RTLModule::addCell(const RTLSignal *signal) {
    Cell *s = NULL;
    if (mapSignalCell.find(signal) == mapSignalCell.end()) {
        s = newCell(signal->getName());
        s->outPins.push_back(new Pin(s));
        mapSignalCell[signal] = s;
        s->signal = signal;
    } else {
        s = mapSignalCell[signal];
    }
    return s;
}

void RTLModule::connect(const RTLSignal *signal, const RTLSignal *driver, Pin
        *p) {
    Cell *s = addCell(signal);
    Cell *d = addCell(driver);

    //errs() << "Connecting. Signal: " << signal->getName() << "(" << signal <<
    //") Driver: " << driver->getName() << "(" << driver << ")" << "\n";

    Pin *in = new Pin(s);
    s->inPins.push_back(in);


    assert(d->outPins.size() > 0);
    Pin *out = d->outPins.at(0);
    if (p) {
        out = p;
    }

    // output pin is already driving a net
    if (!out->net) {
        out->net = new Net();
        nets.insert(out->net);
        out->net->driver = out;
    }

    out->net->fanout.insert(in);

    assert(!in->net);
    in->net = out->net;
}

void RTLModule::recurseBackwards(const RTLSignal *signal) {
    static std::set<const RTLSignal*> visited;
    if (visited.find(signal) != visited.end()) return;
    visited.insert(signal);

    //errs() << "recurseBackwards: " << signal->getName() << "(" << signal <<
    //")\n";

    addCell(signal);
    if (signal->isOp()) {
        const RTLOp *op = (const RTLOp*)signal;
        for (unsigned k = 0; k < op->getNumOperands(); k++) {
            const RTLSignal *operand = op->getOperand(k);
            connect(signal, operand);
            recurseBackwards(operand);
        }
    } else {

        const RTLSignal *defaultDriver = signal->getDefaultDriver();
        if (defaultDriver) {
            connect(signal, defaultDriver);
            recurseBackwards(defaultDriver);
        } 

        for (unsigned j = 0; j < signal->getNumDrivers(); ++j) {
            const RTLSignal *driver = signal->getDriver(j);
            assert(driver);
            connect(signal, driver);
            recurseBackwards(driver);
        }

        for (unsigned j = 0; j < signal->getNumConditions(); ++j) {
            const RTLSignal *condition = signal->getCondition(j);
            assert(condition);

            connect(signal, condition);
            recurseBackwards(condition);
        }
    }
}

void RTLModule::dbgAddInstanceMapping(int parentInst, int thisInst) {
	// Make sure there isn't already a mapping for this parent instance
	std::map<int, int>::iterator it = dbgInstanceMapping.find(parentInst);
	assert(it == dbgInstanceMapping.end());

	dbgInstanceMapping[parentInst] = thisInst;
}

bool isInput(const RTLSignal *s) {
    return (s->getType() == "input");
}

bool isOutput(const RTLSignal *s) {
    return (s->getType() == "output" || s->getType() == "output reg");
}
void printNodeLabelExtras(raw_ostream &out, RTLModule::Cell *csignal) {

    const RTLSignal *signal = csignal->signal;
    if (!signal) return;

    if (signal->isReg()) {
        out << ",shape=box";
    }

    for (unsigned i = 0; i <= signal->getNumConditions(); ++i) {
        const Instruction *I = signal->getInstPtr(i);
        if (!I) continue;

        int pipelined = getMetadataInt(I->getParent()->getTerminator(),
                "legup.pipelined");
        if (pipelined) {
            out << ",color=red";
        } else {
            out << ",color=blue";
        }
    }

}

void printNodeLabel(raw_ostream &out, RTLModule::Cell *csignal) {
    static map<RTLOp::Opcode, string> opcodeStr;
    if (opcodeStr.empty()) {
        opcodeStr[RTLOp::Add] = "ADD";
        opcodeStr[RTLOp::Sub] = "SUB";
        opcodeStr[RTLOp::Mul] = "MUL";
        opcodeStr[RTLOp::Rem] = "REM";
        opcodeStr[RTLOp::Div] = "DIV";
        opcodeStr[RTLOp::And] = "AND";
        opcodeStr[RTLOp::LAnd] = "LAND";
        opcodeStr[RTLOp::Or] = "OR";
        opcodeStr[RTLOp::Xor] = "XOR";
        opcodeStr[RTLOp::Shl] = "SHL";
        opcodeStr[RTLOp::Shr] = "SHR";
        opcodeStr[RTLOp::EQ] = "EQ";
        opcodeStr[RTLOp::NE] = "NE";
        opcodeStr[RTLOp::LT] = "LT";
        opcodeStr[RTLOp::LE] = "LE";
        opcodeStr[RTLOp::GT] = "GT";
        opcodeStr[RTLOp::GE] = "GE";
        opcodeStr[RTLOp::SExt] = "SEXT";
        opcodeStr[RTLOp::ZExt] = "ZEXT";
        opcodeStr[RTLOp::Trunc] = "TRUNC";
        opcodeStr[RTLOp::Concat] = "CONCAT";
        opcodeStr[RTLOp::Sel] = "SEL";
        opcodeStr[RTLOp::Write] = "WRITE";
        opcodeStr[RTLOp::Display] = "DISPLAY";
        opcodeStr[RTLOp::Finish] = "FINISH";
    }

    const RTLSignal *signal = csignal->signal;
    if (!signal) {
        out << "?";
        return;
    }

    if (signal->isOp()) {
        const RTLOp* op = (const RTLOp*)signal;
        RTLOp::Opcode opcode = op->getOpcode();

        out << opcodeStr[opcode];
    } else if (signal->isConst()) {
        string v = signal->getValue();
        replaceAll(v, "\"", "\\\"");
        out << v;
    } else {
        out << signal->getName();
    }

}

void RTLModule::printPipelineDot(formatted_raw_ostream &out) {

    dotGraph<Cell> graph(out, printNodeLabel, printNodeLabelExtras);

    std::set<Cell*> marked;
    std::queue<Cell*> queue;
    for (std::vector<Cell*>::iterator i = outputs.begin(), e = outputs.end(); i
            != e; ++i) {
        queue.push(*i);
    }

    // add any printf statements. Signals driving these statements must be kept
    const RTLSignal* unsynth = getUnsynthesizableSignal();
    assert(unsynth);
    Cell *unsynthCell = mapSignalCell[unsynth];
    assert(unsynthCell);
    queue.push(unsynthCell);

    while (!queue.empty()) {
        Cell *c = queue.front();
        queue.pop();
        if (marked.count(c)) continue;
        marked.insert(c);
        //errs() << "Visiting: " << c->name << "\n";
        for (std::vector<Pin*>::iterator p = c->inPins.begin(), pe =
                c->inPins.end(); p != pe; ++p) {
            Pin *P = *p;
            Net *n = P->net;
            assert(n);
            Pin *driver = n->driver;
            assert(driver);
            Cell *fanin = driver->cell;
            assert(fanin);
            graph.connectDot(out, fanin, c, "");
            queue.push(fanin);
        }
    }

}



void RTLModule::buildCircuitStructure() {
    // create a cell for each input/output port
    for (const_signal_iterator i = port_begin(), e = port_end(); i != e;
            ++i) {
        const RTLSignal *s = *i;
        Cell *c = newCell(s->getName());
        mapSignalCell[s] = c;
        if (isInput(s)) {
            inputs.push_back(c);
            c->outPins.push_back(new Pin(c, "input"));
        } else {
            assert(isOutput(s));
            outputs.push_back(c);
            c->outPins.push_back(new Pin(c, "output"));
        }
    }

    // create a cell for each instantiated module
    std::vector<const RTLSignal *> inst_out_ports;
    for (const_module_iterator m = instances_begin(), me =
            instances_end(); m != me; ++m) {
        RTLModule *mod = *m;
        Cell *c = newCell(mod->getName());
        //errs() << "Adding module: " << mod->getName() << "\n";
        for (RTLModule::const_signal_iterator i = mod->port_begin(), e =
                mod->port_end(); i != e; ++i) {
            const RTLSignal *port = *i;
            mapSignalCell[port] = c;
            if (isInput(port)) {
                assert(port->getNumDrivers() == 1);
                const RTLSignal *driver = port->getDriver(0);
                connect(port, driver);
            } else {
                assert(isOutput(port));
                inst_out_ports.push_back(port);
                Pin *pin = new Pin(c, "output");
                c->outPins.push_back(pin);
                assert(port->getNumDrivers() == 1);
                const RTLSignal *output = port->getDriver(0);
                connect(output, port, pin);
            }
        }
    }

    for (std::vector<const RTLSignal *>::iterator i = inst_out_ports.begin(), e
            = inst_out_ports.end(); i != e; ++i) {
        const RTLSignal *signal = *i;
        recurseBackwards(signal);
    }

    for (const_signal_iterator i = port_begin(), e = port_end(); i != e; ++i) {
        const RTLSignal *signal = *i;
        recurseBackwards(signal);
    }

    for (const_signal_iterator i = signals.begin(), e = signals.end(); i != e;
            ++i) {
        const RTLSignal *signal = *i;
        recurseBackwards(signal);
    }

    // add any printf statements
    const RTLSignal* unsynth = getUnsynthesizableSignal();
    assert(unsynth);
    recurseBackwards(unsynth);
}

// start from the outputs and mark every node
// then remove nodes that aren't marked
void RTLModule::removeSignalsWithoutFanout() {
    buildCircuitStructure();

    std::set<Cell*> marked;
    std::queue<Cell*> queue;
    for (std::vector<Cell*>::iterator i = outputs.begin(), e = outputs.end(); i
            != e; ++i) {
        queue.push(*i);
    }

    // add any printf statements. Signals driving these statements must be kept
    const RTLSignal* unsynth = getUnsynthesizableSignal();
    assert(unsynth);
    Cell *unsynthCell = mapSignalCell[unsynth];
    assert(unsynthCell);
    queue.push(unsynthCell);

    while (!queue.empty()) {
        Cell *c = queue.front();
        queue.pop();
        if (marked.count(c)) continue;
        marked.insert(c);
        //errs() << "Visiting: " << c->name << "\n";
        for (std::vector<Pin*>::iterator p = c->inPins.begin(), pe =
                c->inPins.end(); p != pe; ++p) {
            Pin *P = *p;
            Net *n = P->net;
            assert(n);
            Pin *driver = n->driver;
            assert(driver);
            Cell *fanout = driver->cell;
            assert(fanout);
            queue.push(fanout);
        }
    }

    // remove signals
    signal_iterator i = signals.begin();
    while (i != signals.end()) {
        Cell *c = mapSignalCell[*i];
        assert(c);
        if (!marked.count(c)) {
            //outs() << "Removing signal: " << (*i)->getName() << "\n";
            delete *i;
            i = signals.erase(i);
        } else {
            ++i;
        }
    }
}


void RTLModule::verifyConnections(const Allocation *alloc) const {

    for (const_signal_iterator i = port_begin(), e = port_end(); i != e;
            ++i) {
        verifyConnection(*i, alloc);
    }

    for (const_signal_iterator i = param_begin(), e = param_end(); i != e;
            ++i) {
        verifyConnection(*i, alloc);
    }

    for (const_signal_iterator s = signals.begin(), se = signals.end(); s != se;
            ++s) {
        verifyConnection(*s, alloc);
    }
}

void RTLModule::verifyConnection(const RTLSignal *signal, const Allocation *alloc) const {
    if(LEGUP_CONFIG->getParameterInt("MB_MINIMIZE_HW")) return;
    for (unsigned i = 0; i < signal->getNumDrivers(); ++i) {
        const RTLSignal *driver = signal->getDriver(i);

	// The second condition prevents the warning from being printed
	// in the event that the signal is being driven by multiple signals
	// with different hi and low bit.
	//
	// TODO: print this error if the driver hi and low bits don't match
	// the bits returned in getDriverBits()
	//
        if ((signal->getWidth().numNativeBits(this, alloc) !=
	     driver->getWidth().numNativeBits(this, alloc)) && 
	    (signal->getDriverBits() != "")) {
            errs() << "Incompatible width in module " << this->getName() <<
            ". Signal width: " <<
            signal->getWidth().numNativeBits(this, alloc) << " Driver width: " <<
            driver->getWidth().numNativeBits(this, alloc) << "\n";
            errs() << "Signal name: " << signal->getName() << " value: " <<
                signal->getValue() << " width: " << signal->getWidth().str() <<
                "\n";
            errs() << "Driver name: " << driver->getName() << " value: " <<
                driver->getValue() << " width: " << driver->getWidth().str() <<
                "\n";
        }
        verifyBitwidth(driver, alloc);
    }

    verifyBitwidth(signal, alloc);
}

void RTLModule::verifyBitwidth(const RTLSignal *signal, const Allocation *alloc) const {
    if (isNumeric(signal->getValue())) {
        unsigned bits = signal->getWidth().numBits(this, alloc);
        unsigned reqBits = requiredBits(atoi(signal->getValue().c_str()));
        if (reqBits > bits) {
            errs() << "Constant '" << signal->getValue() << "' only has "
                << bits << " bits when it requires at least " <<
                reqBits << "!\n";
        }
    }
}

RTLOp *RTLModule::recursivelyAddOp(RTLOp::Opcode opcode, const std::vector<RTLSignal*> signal, int count) {
	
	RTLOp *Op, *Op2;
	count-=1;

	if (count > 1) {
		Op = recursivelyAddOp(opcode, signal, count);
		Op2 = this->addOp(opcode)->setOperands(Op, signal[count]);
		return Op2;
	} else if (count == 1) {
		Op = this->addOp(opcode)->setOperands(signal[0], signal[1]);
		return Op;
	} else {
		assert(0 && "There is only one signal, so it cannot be recursively added!\n");
	}
}

RTLOp *RTLOp::setOperands(RTLSignal *s0) {
    setOperand(0, s0);
    return this;
}

RTLOp *RTLOp::setOperands(RTLSignal *s0, RTLSignal *s1) {
    setOperand(0, s0);
    setOperand(1, s1);
    return this;
}

RTLOp *RTLOp::setOperands(RTLSignal *s0, RTLSignal *s1, RTLSignal *s2) {
    setOperand(0, s0);
    setOperand(1, s1);
    setOperand(2, s2);
    return this;
}

void RTLOp::setOperand(int i, RTLSignal *s) {
    if (getNumOperands() == 0) {
        switch (op) {
            case EQ:
            case NE:
            case LT:
            case LE:
            case GT:
            case GE:
                // size 1:
                setWidth(RTLWidth());
                break;
            case Concat:
				if (!castWidth) {
					setWidth(s->getWidth());
					castWidth = true;
				} else {
					//set the width of the signal as the sum of widths from both signals
					setWidth(RTLWidth(atoi(this->getWidth().getHi().c_str()) + 1 + atoi(s->getWidth().getHi().c_str()) + 1));
				}	
				break;
            case Trunc:
            case SExt:
            case ZExt:
                // do nothing in these cases
                // width should be set manually
                if (!castWidth) {
                    errs() << "Forgot to set castWidth for sext/zext/trunc! "
                        "Call castWidth() before adding an operand\n";
                    errs() << "Operand name: " << s->getName() << "value: " <<
                        s->getValue() << "width: " << s->getWidth().str() <<
                        "\n";
                    assert(0);
                }
                break;
            case Sel:
                // wait for second operand
                // first operand should have size 1
                if (!s->getWidth().str().empty()) {
                    errs() << "First select operand should has size 1!\n";
                    errs() << "Signal name: " << s->getName() << "value: " <<
                        s->getValue() << "width: " << s->getWidth().str() <<
                        "\n";
                    assert(0);
                }
                break;
            default:
                // size of operand:
                setWidth(s->getWidth());
                break;
        }
    }

    if (i == 1 && op == Sel) {
        setWidth(s->getWidth());
    }

    if (i == 2 && op == Sel) {
/*        if (getWidth().str() != s->getWidth().str()) {
            errs() << "Select width '" << getWidth().str() <<
                "' doesn't match '" << s->getWidth().str() << "'\n";
        }
        */
    }

    operands[i] = s;
}

} // End legup namespace
