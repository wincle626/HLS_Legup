# This module handles everything to do with trace buffer
# record and replay operations.

import design

class VariableHasNoSource(Exception):
    pass

class VariableNoSourceYet(Exception):
    pass

class VariableNoValueYet(Exception):
    pass

class VariableValueIsUndefined(Exception):
    pass

# This contains a recording of a circuit execution
# including an API to interact with the recording (step forward, back, etc).
class Trace():    
    def __init__(self, manager):
        self.manager = manager
        self.ramUpdates = []
        self.signalUpdates = []
        self.cycles = []
        self.varSourceUpdates = []

    def getState(self, cycle):
        if cycle == 0:
            return (None, None)
        
        assert 1 <= cycle <= len(self.cycles)        
        traceCycle = self.cycles[cycle-1]
        return (traceCycle.instanceId, traceCycle.state)
        
    def newCycle(self, instanceNum, stateNum):
        newCycle = TraceCycle(self, len(self.cycles) + 1, instanceNum, stateNum)
        self.cycles.append(newCycle)
        
        function = self.manager.design.getFunctionFromInstanceNum(instanceNum)
        
        # Find all variables that have a change in source at this state
        for v in self.manager.design.getAllVariables():
            matches = [s for s in v.sources if s.insn and s.insn.endState.number == stateNum and s.insn.function == function]
            assert len(matches) <= 1
            if len(matches) == 1:
                vs = VariableSourceUpdate(newCycle, v, matches[0])
                self.varSourceUpdates.append(vs)
                
#         print("New cycle " + str(newCycle.cycleNum) + " (" + str(instanceNum) + "," + str(stateNum) + ")")
        
        
    def newRamUpdate(self, ram, offset, size, value, cyclesAgo = 0):
        # New write on latest cycle
        assert type(ram) == design.RAM   
        assert size in [1, 2, 4, 8]
        
        # Record each byte separately
        for i in range(size):
            mask = 0xFF << (i * 8)
            byteVal = (value & mask) >> (i * 8)             
            update = TraceRamUpdate(self.cycles[-(1 + cyclesAgo)], ram, offset + i, byteVal)
            self.ramUpdates.append(update)
        
    def newSignalUpdate(self, signal, value):
        assert type(signal) == design.RtlSignal
        update = TraceSignalUpdate(self.cycles[-1], signal, value)
        self.signalUpdates.append(update)
        
    def numCycles(self):
        return len(self.cycles)
    
    def getVarSource(self, cycleNum, var):
        assert type(var) == design.Variable
        
        pastSourceUpdates = [su for su in self.varSourceUpdates if su.traceCycle.cycleNum <= cycleNum and su.var == var]
        pastSourceUpdates.sort(key = lambda su: su.traceCycle.cycleNum)
        
        # Latest update
        if len(pastSourceUpdates):
            return pastSourceUpdates[-1].source
        else:
            # There have been no source updates yet.  If the variable has only one source,
            # and it is a RAM, we will assume the source is a RAM, even if we haven't
            # encountered an llvm.dbg.declare statement yet (which would update the source)
            # This is mandatory for globals, since they do not have an llvm.dbg.declare statement.
            if var.hasSingleSourceThatIsRam():
                return var.sources[0]
            
            if len(var.sources) == 0:
                raise VariableHasNoSource
            else:
                raise VariableNoSourceYet
    
    def getRamValue(self, cycleNum, ram, offset, size):        
        assert type(ram) == design.RAM
        
        value = 0
        
        for i in range(size):
            pastRamUpdates = [ru for ru in self.ramUpdates if ru.traceCycle.cycleNum < cycleNum and ru.ram == ram and ru.offset == (offset + i)]
            pastRamUpdates.sort(key = lambda ru: ru.traceCycle.cycleNum)
        
            if len(pastRamUpdates) == 0:
                raise VariableNoValueYet
            
            value |= pastRamUpdates[-1].val << (i * 8)
        
        return value
    
    def getSignalValue(self, cycleNum, signal):
        assert type(signal) == design.RtlSignal
        
        pastSignalUpdates = [su for su in self.signalUpdates if su.traceCycle.cycleNum <= cycleNum and su.signal == signal]
        pastSignalUpdates.sort(key = lambda su: su.traceCycle.cycleNum)
        
        if len(pastSignalUpdates) == 0:
            raise VariableNoValueYet
        
        return pastSignalUpdates[-1].val
        
    # This can throw exceptions: VariableHasNoSource, VariableNoSourceYet, VariableNoValueYet, 
    #                               VariableValueIsUndefined
    def getVarValue(self, cycleNum, var, offset, size):
        # This can throw exceptions, but we will let them continue up
        source = self.getVarSource(cycleNum, var)
        
        assert offset == 0 or source.isRAM()
        
        if source.isSignal():
            return self.getSignalValue(cycleNum, source.signal)
        elif source.isRAM():
            return self.getRamValue(cycleNum, source.ram, offset, size)
        elif source.isPointer():
            return self.getRamValue(cycleNum, source.ram, source.offset)
        elif source.isConstant():
            return source.val
        elif source.isUndefined():
            raise VariableValueIsUndefined
        else:
            assert False

# This class contains information about an update to a variable source
# ie. it indicates a new source for a variable (RAM, signal, constant, etc)
class VariableSourceUpdate():
    def __init__(self, traceCycle, var, source):
        self.traceCycle = traceCycle
        self.var = var
        self.source = source
        
# This class contains an update to a signal value
class TraceSignalUpdate():
    def __init__(self, traceCycle, signal, val):
        self.traceCycle = traceCycle
        self.signal = signal
        self.val = val

# This class contains an update to a RAM entry
class TraceRamUpdate():
    def __init__(self, traceCycle, ram, offset, val):
        self.traceCycle = traceCycle        
        self.ram = ram
        self.offset = offset
        self.val = val
    
# This class contains recorded control flow information for a single hardware cycle
class TraceCycle():
    def __init__(self, trace, cycleNum, instanceId, state):
        self.trace = trace
        self.cycleNum = cycleNum
        self.instanceId = instanceId
        self.state = state
