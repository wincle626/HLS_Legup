//===-- GlobalNames.cpp -----------------------------------------*- C++ -*-===//
//
// This file is distributed under the LegUp license. See LICENSE for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the GlobalNames object
//
//===----------------------------------------------------------------------===//

#include "GlobalNames.h"
#include "utils.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include <string>

using namespace llvm;
using namespace legup;


namespace legup {

const std::string verilogReservedKeywords[] = {
    "and",          "always",       "assign",   "attribute", "begin",
    "buf",          "bufif0",       "bufif1",   "bufif1",    "case",
    "cmos",         "deassign",     "default",  "defparam",  "disable",
    "else",         "endattribute", "end",      "endcase",   "endfunction",
    "endprimitive", "endmodule",    "endtable", "endtask",   "event",
    "for",          "force",        "forever",  "fork",      "function",
    "highhz0",      "highhz1",      "if",       "initial",   "inout",
    "input",        "integer",      "join",     "large",     "medium",
    "module",       "nand",         "negedge",  "nor",       "not",
    "notif0",       "notif1",       "nmos",     "or",        "output",
    "parameter",    "pmos",         "posedge",  "primitive", "pulldown",
    "pullup",       "pull0",        "pull1",    "rcmos",     "reg",
    "release",      "repeat",       "rnmos",    "rpmos",     "rtran",
    "rtranif0",     "rtranif1",     "scalared", "small",     "specify",
    "specparam",    "strong0",      "strong1",  "supply0",   "supply1",
    "table",        "task",         "tran",     "tranif0",   "tranif1",
    "time",         "tri",          "triand",   "trior",     "trireg",
    "tri0",         "tri1",         "vectored", "wait",      "wand",
    "weak0",        "weak1",        "while",    "wire",      "wor",
    // SystemVerilog-2005 keywords - these give warnings from Quartus
    "type",         "return"};

std::string GlobalNames::verilogName(const Value *val) {

    static std::set<std::string> verilogReserved(verilogReservedKeywords,
                                                 verilogReservedKeywords + 97);

    if (uniquename.find(val) == uniquename.end()) {
        std::string name = getLabel(val);

        // empty or Verilog reserved keywords
        if (name.empty()) {
            name = "var" + utostr(varcount++);
            // name exists
        }

        // if possible prefix with "<module>_<bb>_"
        if (const Instruction *I = dyn_cast<Instruction>(val)) {
            if (const BasicBlock *parent = I->getParent()) {
                std::string bb = getLabel(parent);
                std::string function = parent->getParent()->getName();
                name = function + "_" + bb + "_" + name;
            }
        } else if (isa<Argument>(val)) {
            name = "arg_" + name;
        }
        stripInvalidCharacters(name);

        if(uniquenameset.count(name) || verilogReserved.count(name)) {
            name = name + "_var" + utostr(varcount++);
        }

        // make sure the llvm name was unique
        assert(!uniquenameset.count(name));
        uniquenameset.insert(name);

        uniquename[val] = name;
    }
    return uniquename[val];
}

} // End legup namespace

