from backends import backend
from tracing import decoder, trace
import manager
import design

class Replay(backend.Backend):
    IMPLEMENTS_BACKSTEP = True
    VARIABLES_AUTO_REFRESH = True
    IMPLEMENTS_TRACE = True
    IMPLEMENTS_RUN = True
    IMPLEMENTS_BREAKPOINTS = True
    
    def __init__(self, manager):
        super().__init__()
        
        self.manager = manager
        self.cycle = None
        self.breakpoints = []
    
    def activate(self):
        if not self.manager.commConnected():
            raise backend.CannotActivate("Must have a valid hardware connection before entering replay mode.")
        
            
    def clientDisconnected(self):
        pass
#       
    def newDesign(self):
        self.trace = trace.Trace(self.manager)
        traceBufferDecoder = decoder.TraceBufferDecoder(self.manager)   

        self.manager.comm.getPlaybackInfo(traceBufferDecoder)       
        traceBufferDecoder.toTrace(self.trace)
        
        self.cycle = 1
        self.updateState()

    def closeDesign(self):
        self.trace = None

    def step(self):
        self.cycle = min(self.cycle + 1, self.trace.numCycles())
        self.updateState()
        
    def run(self):
        # Get breakpoint instanceNum, stateNum
        breakpointLocations = [(b.insn.function, b.insn.startState.number) for b in self.breakpoints]
        
        while self.cycle < self.trace.numCycles():
            self.cycle = min(self.cycle + 1, self.trace.numCycles())
            
            # Have we hit a breakpoint?
            (instanceNum, stateNum) = self.trace.getState(self.cycle)
            fcn = self.manager.design.instanceNumToInstance[instanceNum].function
            location = (fcn, stateNum)
            
            if location in breakpointLocations:
                break
            
        self.updateState()
            
    def breakpointAdd(self, breakpoint):
        self.breakpoints.append(breakpoint)
        
    def breakpointRemove(self, breakpoint):
        self.breakpoints.remove(breakpoint)
        
    def stepBack(self):
        self.cycle = max(1, self.cycle - 1)
        self.updateState()
    
    def setTraceCycle(self, cycle):
        self.cycle = cycle
        self.updateState()
        
    def getTraceCycle(self):
        return self.cycle
    
    def updateState(self):
        state = self.trace.getState(self.cycle)
        self.manager.currentState = state
        
    def numTraceCycles(self):
        return self.trace.numCycles()
        
    def varRead(self, var, offset, size):
#         print ("Reading var: " + str(var.name) + " at cycle " + str(self.cycle))
        
        
        assert type(var) == design.Variable
        
        if not var.isTraced:
            raise manager.VariableNotTraced
        
        noValueYet = False
        
        try:
            val = self.trace.getVarValue(self.cycle, var, offset, size)
        except trace.VariableHasNoSource:
            raise manager.VariableOptimizedAway
        except trace.VariableNoSourceYet:
            raise manager.VariableNotAccessible     
        except trace.VariableValueIsUndefined:
            raise manager.VariableValueUndefined
        except trace.VariableNoValueYet:
            noValueYet = True
            
        if noValueYet:
            source = self.trace.getVarSource(self.cycle, var)
            if source.isRAM() or source.isPointer():
                # Variable value is in memory, but we don't have an update in the
                # trace buffer.  If there are no later updates we can read the 
                # value directly out of the FPGA memory
                if source.isPointer():
                    offset = source.offset

                try:
                    # Try getting the ram value at the last recorded cycle
                    raise manager.VariableValueUnknown
                    self.trace.getRamValue(len(self.trace.cycles), source.ram, offset)
                except trace.VariableNoValueYet:
                    # If there's still no value, we can safely read directly from memory
                    val = self.manager.comm.ramRead(source.ram, offset)
                else:
                    raise manager.VariableValueUnknown
            else:
                raise manager.VariableValueUnknown
        
        return val
                        
