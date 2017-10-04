# This module contains all information relating to a design, loaded from the debug database.

import os
import math
import abc

import mysql.connector

# INSTRUCTION_MODE_HW, INSTRUCTION_MODE_ASSEMBLY, INSTRUCTION_MODE_C = range(3)
# NEW_CYCLE_AT_INSN_START = False

class DesignNoDatabase (Exception):
    pass

class DesignDatabaseCorrupt(Exception):
    def __init__(self, msg):
        self.msg = msg
        
DW_TAG_ARRAY = 1
DW_TAG_STRUCT = 19
DW_TAG_TYPEDEF = 22
DW_TAG_BASE_TYPE = 36
DW_TAG_CONST = 38

# This class contains all information relative to the 'design'.
# The design is essentially a debug session for one design.
class Design(): 
    def __init__(self, manager, path):
        
        self.manager = manager
        self.path = path
        
        self.mysqlConnection = mysql.connector.connect(user='root', 
                                                        password='letmein', 
                                                        host='localhost', 
                                                        database='legupDebug')
        
        # SystemID is a unique identifier, generated on each creation
        # of the debug database and the Verilog file.  This ensures
        # syncronization between database and hardware.
        self.configSystemID = 0
                
        self.board = None
        
        self.optionDebugRtlEnabled = None
        self.optionTraceRegs = None
        
        self.configInstanceNumBitWidth = None
        self.configStateNumBitWidth = None
        self.configRegsBitWidth = None

        self.configMemAddrBits = None
        self.configMemDataBits = None
        
        self.configBufferCtrlWidth = None
        self.configBufferCtrlDepth = None
        self.configBufferCtrlSequenceBits = None

        self.configBufferMemWidth = None
        self.configBufferMemDepth = None

        self.configBufferRegsEnabled = False
        self.configBufferRegsWidth = None
        self.configBufferRegsDepth = None
        
        # Options
        self.optionTraceRegs = None
        
        # List of functions
        self.functions = []
        
        self.instanceNumToInstance = {}
        
        self.rams = []
        self.globalVariables = []
        
        self.types = []
        
        # 2-D Dictionary of Insns by source file/line #. 
        # Dictionary key = [source file path (string)][line number (int)]
        self.insnsBySourceFileLine = None
        
        
    def __del__(self):
        self.mysqlConnection.close()
    
    # Read-* in the debug database and populate it into a 'Design' class
    def populateFromDatabase(self):
        mysqlCursor = self.mysqlConnection.cursor(buffered=True)
        
        query = "SELECT id, name FROM Designs WHERE path = %s"          
        mysqlCursor.execute(query, [self.path])
        
        if mysqlCursor.rowcount == 0:
            raise DesignNoDatabase()
        
        assert mysqlCursor.rowcount == 1
        
        (designId, self.name) = mysqlCursor.fetchone()
#       self.designId = designId
        
        # Get design options
        query = "SELECT isDebugRtlEnabled, isXilinx, board, memoryAddrWidth, memoryDataWidth " \
                "FROM DesignProperties " \
                "WHERE designId = %s"       
        mysqlCursor.execute(query, [designId])
        assert mysqlCursor.rowcount == 1
        
        (self.optionDebugRtlEnabled, self.isXilinx, self.board, self.configMemAddrBits, 
            self.configMemDataBits) = mysqlCursor.fetchone() 

        if (self.optionDebugRtlEnabled):
            query = "SELECT numInstanceBits, numStateBits, systemId " \
                    "FROM InstrumentationProperties " \
                    "WHERE designId = %s"       
            mysqlCursor.execute(query, [designId])
            assert mysqlCursor.rowcount == 1
            
            (self.configInstanceNumBitWidth, self.configStateNumBitWidth, self.configSystemID) = mysqlCursor.fetchone() 

        # Get trace buffer options
        query = "SELECT controlBufWidth, controlBufSequenceBits, controlBufDepth, " \
                "memoryBufWidth, memoryBufDepth, regsBufEnabled, regsBufWidth, regsBufDepth " \
                "FROM TraceBufferProperties " \
                "WHERE designId = %s"
        mysqlCursor.execute(query, [designId])
        assert mysqlCursor.rowcount == 1
        
        (self.configBufferCtrlWidth, self.configBufferCtrlSequenceBits, self.configBufferCtrlDepth,
            self.configBufferMemWidth, self.configBufferMemDepth,
            self.configBufferRegsEnabled, self.configBufferRegsWidth,
            self.configBufferRegsDepth) = mysqlCursor.fetchone()
        
        # Functions
        functionById = {}
        query = "SELECT id, name FROM Function WHERE designId = %s"
        mysqlCursor.execute(query, [designId])
        
        for (funcId, funcName) in mysqlCursor:
            function = Function(self, funcName) 
            self.functions.append(function)
            functionById[funcId] = function
            
        # Instances
        query = "SELECT id, instanceNum, functionId FROM Instance " \
                "WHERE designId = %s "
        mysqlCursor.execute(query, [designId])
        
        instanceById = {}
        for (instanceId, instanceNum, funcId) in mysqlCursor:
            function = functionById[funcId]
            instance = Instance(function, instanceNum)
            instanceById[instanceId] = instance
            self.instanceNumToInstance[instanceNum] = instance
            function.instances.append(instance)
            
        # Instance Children
        query = "SELECT instanceId, childInstanceId FROM InstanceChildren ic " \
                "INNER JOIN Instance i ON i.id = ic.instanceId " \
                "WHERE i.designId = %s " 
        mysqlCursor.execute(query, [designId])
        
        for (instanceId, childInstanceId) in mysqlCursor:
            instance = instanceById[instanceId]
            childInstance = instanceById[childInstanceId]
            instance.children.append(childInstance)
        
        # States
        query = "SELECT s.id, belongingFunctionId, calledFunctionId, number, s.name, s.storeA, s.storeB, " \
                "traceRegsPortA, traceRegsPortB " \
                "FROM State s INNER JOIN Function f " \
                "ON s.belongingFunctionId = f.id " \
                "WHERE f.designId = %s "
        mysqlCursor.execute(query, [designId])
        
        statesById = {}
        for (stateId, funcId, calledFuncId, stateNum, stateName, storeA, storeB, traceRegsA, traceRegsB) in mysqlCursor:
            function = functionById[funcId]
            
            state = State(function, stateNum, stateName, functionById.get(calledFuncId), storeA, storeB, traceRegsA, traceRegsB)
            statesById[stateId] = state
            function.states.append(state)
        
        # IRInstructions
        query = "SELECT ir.id, functionId, isDummyDebugCall, dump, startStateId, endStateId, filePath, lineNumber, columnNumber " \
                    "FROM IRInstr ir " \
                    "INNER JOIN Function f ON f.id = ir.functionId " \
                    "WHERE f.designId = %s"
        mysqlCursor.execute(query, [designId])
            
        insnsById = {}
        for (insnId, funcId, isDummyDbgCall, desc, startStateId, endStateId, path, line, col) in mysqlCursor:
            function = functionById[funcId]
            startState = statesById[startStateId]
            endState = statesById[endStateId]
            insn = IRInstruction(function, desc, startState, endState, path, line, col, isDummyDbgCall)
            insnsById[insnId] = insn
            function.insns.append(insn)
        
        # Signals
        query = "SELECT r.id, functionId, signalName, width " \
                "FROM RtlSignal r " \
                "INNER JOIN Function f ON f.id = r.functionId " \
                "WHERE f.designId = %s"
        mysqlCursor.execute(query, [designId])
        
        signalsById = {}
        for (signalId, functionId, name, width) in mysqlCursor:
            function = functionById[functionId]
            signal = RtlSignal(function, name, width)
            function.signals.append(signal)
            signalsById[signalId] = signal
        
        # Types
        query = "SELECT id, dw_tag, name, size, alignment, offset, derivedTypeId " \
                "FROM VariableType " \
                "WHERE designId = %s " \
                "ORDER BY id"
        mysqlCursor.execute(query, [designId])
        
        typesById = {}
        for (typeId, dw_tag, name, size, alignment, offset, derivedTypeId) in mysqlCursor:
            if derivedTypeId:
                assert derivedTypeId in typesById
                derivedType = typesById[derivedTypeId]
            else:
                derivedType = None
            varType = VariableType(dw_tag, name, size, alignment, offset, derivedType)
            self.types.append(varType)
            typesById[typeId] = varType
            
        # Type Members
        query = "SELECT ownerVariableTypeId, variableTypeId, subrangeCount " \
                "FROM VariableTypeMember vtm " \
                "INNER JOIN VariableType vt ON vtm.ownerVariableTypeId = vt.id " \
                "WHERE designId = %s " \
                "ORDER BY ownerVariableTypeId, idx "
        mysqlCursor.execute(query, [designId])
        
        for (ownerTypeId, variableTypeId, subrangeCount) in mysqlCursor:
            ownerType = typesById[ownerTypeId]
            if variableTypeId:
                varType = typesById[variableTypeId]
            else:
                varType = None
            typeMember = VariableTypeMember(varType, subrangeCount)
            ownerType.members.append(typeMember)
        
        # Variables
        query = "SELECT id, name, functionId, filePath, lineNumber, typeId " \
                "FROM Variable " \
                "WHERE designId = %s"
        mysqlCursor.execute(query, [designId])
        
        variableById = {}
        for (varId, varName, funcId, filePath, lineNum, typeId) in mysqlCursor:
            varType = typesById[typeId]
            if funcId:
                function = functionById[funcId]
            else:
                function = None
                
            var = Variable(function, varId, varName, filePath, lineNum, varType)
            variableById[varId] = var
            
            if function:
                function.variables.append(var)
            else:
                self.globalVariables.append(var)
            
        # RAMs
        ramsById = {}
        query = "SELECT id, tagNum, dataWidth, numElements " \
                "FROM RAM " \
                "WHERE designId = %s "
        mysqlCursor.execute(query, [designId])
        for (ramId, tagNum, dataWidth, numElements) in mysqlCursor:
            ram = RAM(tagNum, dataWidth, numElements)
            ramsById[ramId] = ram
            self.rams.append(ram)
        
        # Variable Source - RAM
        query = "SELECT IRInstrId, v.id, ramid " \
                "FROM VariableSourceRAM vsr " \
                "INNER JOIN VariableSource vs ON vsr.VariableSourceId = vs.id " \
                "INNER JOIN Variable v ON v.id = vs.variableId " \
                "WHERE v.designId = %s "
        mysqlCursor.execute(query, [designId])
        
        for (insnId, varId, ramId) in mysqlCursor:
            insn = insnsById.get(insnId)
            ram = ramsById[ramId]
            source = VariableSourceRAM(insn, ram)
            var = variableById[varId]
            var.sources.append(source)
            
        # Variable Source - Constant Int
        query = "SELECT IRInstrId, v.id, constantInt " \
                "FROM VariableSourceConstantInt vsci " \
                "INNER JOIN VariableSource vs ON vsci.VariableSourceId = vs.id " \
                "INNER JOIN Variable v ON v.id = vs.variableId " \
                "WHERE v.designId = %s "
        mysqlCursor.execute(query, [designId])
        
        for (insnId, varId, val) in mysqlCursor:
            if val is None:  # Convert NULL to 0
                val = 0
            insn = insnsById.get(insnId)
            var = variableById[varId]
            source = VariableSourceConstantInt(insn, val)
            var.sources.append(source)
            
        # Variable Source - Undefined
        query = "SELECT IRInstrId, v.id " \
                "FROM VariableSourceUndefined vsu " \
                "INNER JOIN VariableSource vs ON vsu.VariableSourceId = vs.id " \
                "INNER JOIN Variable v ON v.id = vs.variableId " \
                "WHERE v.designId = %s "
        mysqlCursor.execute(query, [designId])
        
        for (insnId, varId) in mysqlCursor:
            insn = insnsById.get(insnId)
            var = variableById[varId]
            source = VariableSourceUndefined(insn)
            var.sources.append(source)
            
        # Variable Source - Pointer
        query = "SELECT IRInstrId, v.id, ramId, offset " \
                "FROM VariableSourcePointer vsp " \
                "INNER JOIN VariableSource vs ON vsp.VariableSourceId = vs.id " \
                "INNER JOIN Variable v ON v.id = vs.variableId " \
                "WHERE v.designId = %s "
        mysqlCursor.execute(query, [designId])
        
        for (insnId, varId, ramId, offset) in mysqlCursor:
            insn = insnsById.get(insnId)
            var = variableById[varId]
            ram = ramsById[ramId]
            source = VariableSourcePointer(insn,ram, offset)
            var.sources.append(source)
            
        # Variable Source - Signal
        query = "SELECT vss.id, IRInstrId, v.id, rtlSignalId " \
                "FROM VariableSourceSignal vss " \
                "INNER JOIN VariableSource vs ON vss.VariableSourceId = vs.id " \
                "INNER JOIN Variable v ON v.id = vs.variableId " \
                "WHERE v.designId = %s "
        mysqlCursor.execute(query, [designId])
        
        valSrcSignalById = {}
        for (valSrcId, insnId, varId, signalId) in mysqlCursor:
            insn = insnsById.get(insnId)
            var = variableById[varId]
            signal = signalsById[signalId]
            source = VariableSourceSignal(insn, signal)
            var.sources.append(source)
            valSrcSignalById[valSrcId] = source
            
        # Signal Trace Schedule
        query = "SELECT rtlSignalId, delayedCycles, recordInStateId, hiBit, loBit " \
                "FROM RtlSignalTraceSchedule rsts " \
                "INNER JOIN RtlSignal rs ON rsts.rtlSignalId = rs.id " \
                "INNER JOIN Function f ON f.id = rs.functionId " \
                "WHERE f.designId = %s "
        mysqlCursor.execute(query, [designId])
        
        for (signalId, delay, stateId, hi, lo) in mysqlCursor:
            signal = signalsById[signalId]
            state = statesById[stateId]
            assert signal.width == (hi - lo + 1)
            signalTrace = RtlSignalTrace(delay, state, hi, lo)
            signal.traces.append(signalTrace)


        self.buildMappings()

    def getFilenames(self):
        files = []
        for fcn in self.functions:
            for insn in fcn.insns:
                files.append(insn.path)
                
        files = [f for f in set(files) if f != ""]
        return files               
    
    def getFunctionByName(self, functionName):
        matches = [f for f in self.functions if f.name == functionName]
        
        assert len(matches) <= 1
        if len(matches) == 0:
            return None
        else:
            return matches[0]

    def printer(self):
        for fcn in self.functions.sources():
            fcn.printer()    
        
    def buildInsnsBySourceFileLine(self):
        # Assembly instructions
        self.insnsBySourceFileLine = {}
        
        for fcn in self.functions:
            for insn in fcn.insns:
                if insn.path == "":
                    continue
                if insn.isDummyDbgCall:
                    continue

                if insn.path not in self.insnsBySourceFileLine:
                    self.insnsBySourceFileLine[insn.path] = {}
                    
                if insn.line not in self.insnsBySourceFileLine[insn.path]:
                    self.insnsBySourceFileLine[insn.path][insn.line] = [insn]
                else:
                    self.insnsBySourceFileLine[insn.path][insn.line].append(insn)

    
    def buildMappings(self):
        self.buildInsnsBySourceFileLine()
        
    def verilogFilePath(self):
        return os.path.join(self.path, self.name + ".v")

    def instanceNumBytes(self):
        return int(math.ceil(self.configInstanceNumBitWidth/8))

    def stateNumBytes(self):
        return int(math.ceil(self.configStateNumBitWidth/8))

    def ramFromTag(self, tag):
        matches = [r for r in self.rams if r.tag == tag]
        assert len(matches) <= 1
        
        if len(matches) == 1:
            return matches[0]
        else:
            return None
        

#   def varIdFromTag(self, tag):
#       vars = [v for f in self.functions for v in f.variables] + self.globalVariables
#       ids = [v.varId for v in vars if v.ownsTag(tag)]
#       if len(ids) != 1:
#           print (str(len(ids)) + " variables with tag " + str(tag))
#           return None
# #             assert False
#       return ids[0]
    
    def getFunctionFromInstanceNum(self, instanceNum):
        matches = [f for f in self.functions for i in f.instances if i.number == instanceNum]
        
        assert len(matches) <= 1
        if len(matches):
            return matches[0]
        else:
            return None
        
    def getAllVariables(self):
        return self.globalVariables + [v for f in self.functions for v in f.variables]

# This is an instruction of assembly code 
# This is for the gantt chart, when operating in assembly-mode, and
# corresponds to one block in the chart
class IRInstruction():
    
    def __init__(self, function, desc, startState, endState, path, line, col, isDummyDbgCall):
        assert startState
        assert endState
        
        self.function = function
        self.desc = desc
        self.path = path 
        self.line = line
        self.col = col
        self.isDummyDbgCall = isDummyDbgCall
        
        # The start/end cycle in the hw domain
        self.startState = startState
        self.endState = endState
        
        # The start/end cycle in the assembly domain
        self.assembly_cycle_start = None
        self.assembly_cycle_end = None
        
    def getGanttEndStateNum(self):
        if self.startState == self.endState:
            return self.startState.number + 1
        else:
            return self.endState.number 

    def printer(self):
        print(str(self.hw_cycle_start) + "-" + str(self.hw_cycle_start + self.duration_C0 -1) + "\t" + self.desc + "\t" + str(self.line))


# This is a hardware instance of a function
class Instance():
    def __init__(self, function, instanceNum):
        self.function = function
        self.number = instanceNum
        
        # List of child Instance objects
        self.children = []
    
    def findChildInstanceByFunction(self, function):
        matches = [i for i in self.children if i.function == function]
        
        assert len(matches) <= 1
        if len(matches) == 0:
            return None
        else:
            return matches[0]

class State():
    def __init__(self, function, number, name, calledFunction, storeA, storeB, traceRegsA, traceRegsB):
        self.function = function
        self.number = number
        self.name = name
        self.calledFunction = calledFunction
        self.storeA = storeA
        self.storeB = storeB
        self.traceRegsA = traceRegsA
        self.traceRegsB = traceRegsB

class RtlSignal():
    def __init__(self, function, name, width):
        self.function = function
        self.name = name
        self.width = width
        self.traces = []
        
class RtlSignalTrace():
    def __init__(self, delay, state, hi, lo):
        self.delay = delay
        self.state = state
        self.hi = hi
        self.lo = lo

# This class for a C function
class Function():
    def __init__(self, design, name):
        self.design = design
        self.name = name
        
        # List of Instance objects
        self.instances = []
        
        # List of IRInstruction objects
        self.insns = []
        
        # List of State objects
        self.states = []

        # List of Variable objects
        self.variables = []
        
        # List of RtlSignal objects
        self.signals = []
        
        # Indicates whether the function is recorded in trace buffers
        self.isTraced = True
    
    def getInstanceNums(self):
        return [i.number for i in self.instances]
    
    def getStateByNum(self, stateNum):
        matches = [s for s in self.states if s.number == stateNum]
        
        assert len(matches) <= 1
        if len(matches) == 0:
            return None
        else:
            return matches[0]
        
    def getFunctionCalledInStateNum(self, stateNum):
        state = self.getStateByNum(stateNum)
        return state.calledFunction
        
    # Returns all instructions that are active in the current hardware cycle
    def getActiveInsns(self, stateNum):
        if stateNum is None:
            return None
        
        return [insn for insn in self.insns 
            if stateNum >= insn.startState.number and stateNum <= insn.endState.number
            and not insn.isDummyDbgCall]
                
    def printer(self):
        print("Function: " + self.name)
        for cycle in self.insns_by_cycle:
            print("\tCycle " + str(cycle) + ":")
            insn_list = self.insns_by_cycle[cycle]
            for insn in insn_list:
                print("\t\t" + insn.desc)
    
class VariableType:
    def __init__(self, dw_tag, name, size, alignment, offset, derivedType):
        self.dw_tag = dw_tag
        self.name = name
        self.size = size
        self.alignment = alignment
        self.offset = offset
        self.derivedType = derivedType
        self.members = []
        
        
class VariableTypeMember:
    def __init__(self, varType, subrangeCount):
        self.varType = varType
        self.subrangeCount = subrangeCount
        
    def isSubrangeCount(self):
        return self.subrangeCount is not None
    
# This class is used for all variables within the design
class Variable:
    def __init__(self, function, varId, name, path, line, varType):
        self.function = function
        self.varId = varId
        self.name = name
        self.path = path
        self.line = line
        self.varType = varType
        
        # Whether or not this variable should be saved in the trace buffers
        self.isTraced = True
        
        # Dictionary of 
        self.sources = []
        
    def hasSingleSourceThatIsRam(self):
        return len(self.sources) == 1 and self.sources[0].isRAM()
        
#   def ownsTag(self, tag):
#       matches = [s for s in self.sources if s.isRAM() and s.ram.tag == tag]
#       if len(matches):
#           return True
#       else:
#           return False
#       
#   def getTag(self):
#       assert self.isInMemory()
#       return self.sources[0].ram.tag
    
#   def getVarWidthInBytes(self):
#       assert self.isInMemory()
#       
#       widthInBits = self.sources[0].ram.dataWidth
#       assert widthInBits % 8 == 0
#       return int(widthInBits / 8)
#       
#   def valuesAtState(self, state):
#       if state in self.sources:
#           return self.sources[state]
#       else:
#           return []


class VariableSource(metaclass=abc.ABCMeta):
    def __init__(self, insn):
        self.insn = insn
        
    def isSignal(self):
        return type(self) is VariableSourceSignal
    def isRAM(self):
        return type(self) is VariableSourceRAM
    def isConstant(self):
        return type(self) is VariableSourceConstantInt
    def isUndefind(self):
        return type(self) is VariableSourceUndefined
    def isPointer(self):
        return type(self) is VariableSourcePointer

class VariableSourceSignal(VariableSource):
    def __init__(self, insn, signal):
        super().__init__(insn)
        self.signal = signal

class VariableSourceRAM(VariableSource):
    def __init__(self, insn, ram):
        super().__init__(insn)
        self.ram = ram
        
class VariableSourceConstantInt(VariableSource):
    def __init__(self, insn, val):
        super().__init__(insn)
        self.val = val
        
class VariableSourcePointer(VariableSource):
    def __init__(self, insn, ram, offset):
        super().__init__(insn)
        self.ram = ram
        self.offset = offset
        
class VariableSourceUndefined(VariableSource):
    pass


        
class RAM():
    def __init__(self, tagNum, dataWidth, numElements):
        self.tag = tagNum
        self.dataWidth = dataWidth
        self.numElements = numElements