import math

from backends import backend
import manager
import comm

class Hardware(backend.Backend):
    IMPLEMENTS_REFRESH = True
    IMPLEMENTS_RUN = True
    IMPLEMENTS_RESET = True
    IMPLEMENTS_BREAKPOINTS = True
    VERIFIES_SERIAL_CONNECTION = True
    
    def __init__(self, manager):
        super().__init__()
        
        self.manager = manager
        self.designOpened = False
        self.breakpoints = []

    def activate(self):
        if self.manager.design:
            self.updateState()
    
    def clientDisconnected(self):
        pass
    
    def newDesign(self):
        pass
    
    def closeDesign(self):
        pass
        
    def serialHasConnected(self):
        if self.manager.design:
            self.verifySystemID()
        else:
            self.manager.errorMessage("You must open a design before connecting to the FPGA board.")
            return
        
        if self.manager.comm.systemIdVerified:
#           self.sendTraceOptionsToComm()
            self.updateState()
        
    def verifySystemID(self):
        try:
            self.manager.comm.verifySystemID(self.manager.design.configSystemID)
        except comm.NotConnected:
            pass
        except comm.NotResponding:
            self.errorMessage("System is not reponding via Serial.  Ensure design is programmed to FPGA board properly.")
        except comm.InvalidSystemId as e:
            self.manager.errorMessage("SystemID of design (" + hex(self.manager.design.configSystemID) + ") does not match SystemID of connected device (" + hex(e.args[0]) + ")")
        
    def reset(self):
        if not self.manager.comm.connected():
            self.manager.minorMessage.emit("Not connected to FPGA board")
            return
        
        self.manager.comm.reset()
        self.updateState()

    def step(self):
        if not self.manager.comm.connected():
            self.manager.minorMessage.emit("Not connected to FPGA board")
            return
        
        self.manager.comm.run_cycles(1)
        
    def run(self):
        if not self.manager.comm.connected():
            self.manager.minorMessage.emit("Not connected to FPGA board")
            return
        
        self.manager.comm.run()
        
    def updateState(self):
        if not self.manager.comm.connected():
            self.manager.minorMessage.emit("Not connected to FPGA board")
            newState = (None, None)
        else:
            newState = self.manager.comm.getState(self.manager.design.instanceNumBytes(), self.manager.design.stateNumBytes())
        self.manager.currentState = newState
            
    def getMaxNumBreakpoints(self):
        return 1
    
    def breakpointAdd(self, breakpoint):
        design = self.manager.design
        assert design
        
        if not self.manager.comm.connected():
            raise backend.CannotSetBreakpoint("Must be connected to set breakpoints")
            
        if len(self.breakpoints) == 1:
            self.breakpointRemove(self.breakpoints[0])
            
        assert len(self.breakpoints) == 0
        
        fcn = breakpoint.insn.function
        
        if len(fcn.instances) > 1:
            raise backend.InvalidBreakpointLocation("The selected function has multiple hardware instances.  Breakpoints are not supported for these types of functions.")
        else:
            instanceNum = fcn.instances[0].number
            
        stateNum = breakpoint.insn.startState.number
        
        self.manager.comm.setBreakpoint(math.ceil(design.configInstanceNumBitWidth / 8), instanceNum, 
                                    math.ceil(design.configStateNumBitWidth / 8), stateNum)
        self.breakpoints.append(breakpoint)
        
    def breakpointRemove(self, breakpoint):
        assert breakpoint in self.breakpoints 
        assert len(self.breakpoints) == 1
        self.manager.comm.clr_breakpoint()
        del self.breakpoints[0]
        
    def varRead(self, var, offset, size):
        if not self.manager.commConnected():
            raise comm.NotConnected
        
        if len(var.sources) == 0:
            raise manager.VariableOptimizedAway     
        
        if not var.hasSingleSourceThatIsRam():
            raise manager.VariableNotAccessible
        else:
            return self.manager.comm.ramRead(var.sources[0].ram, offset, size)

    # This function sends all of the trace options to the device
    def sendTraceOptionsToComm(self):
        comm = self.manager.comm 
        
        for g_var in self.manager.design.global_vars():
            comm.traceVarEnable(g_var.tag, g_var.isTraced)
            
        for fcn in self.manager.design.functions:
            for instanceID in fcn.instanceIDs:
                comm.traceFunctionEnable(instanceID, fcn.isTraced)
            for var in fcn.variables():
                if var.isInMemory():
                    comm.traceVarEnable(var.tag, var.isTraced)
                    
