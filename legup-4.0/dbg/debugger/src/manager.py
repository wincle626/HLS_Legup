# This is the debug manager class, which is the heart of the debugger.
# The gui class is an overlay on top of the manager, and sends requests to this manager
#
# Ideally this manager could be used with other user interfaces (ie command line),
# although this has not been explored yet.
#

import design
from backends import backend, hardware, replay, sim
import comm

# from backendFPGA import HLSDComm
import signals
import threading

from PyQt5 import QtCore

class UserDisconnected(Exception):
    pass

class VariableNotTraced(Exception):
    pass

class VariableNotAccessible(Exception):
    pass

class VariableOptimizedAway(Exception):
    pass

class VariableValueUnknown(Exception):
    pass

class VariableValueUndefined(Exception):
    pass

class TaskThread(threading.Thread):
    """Thread that executes a task every N seconds"""
    
    def __init__(self, interval, function):
        threading.Thread.__init__(self)
        self._finished = threading.Event()
        self._interval = interval
        self._function = function
        self.daemon = True
    
    def setInterval(self, interval):
        """Set the number of seconds we sleep between executing our task"""
        self._interval = interval
    
    def shutdown(self):
        """Stop this thread"""
        self._finished.set()
    
    def run(self):
        while 1:
            if self._finished.isSet(): return
            self.task()
            
            # sleep for interval or until shutdown
            self._finished.wait(self._interval)
    
    def task(self):
        self._function()


class Manager():
    TIMER_INTERVAL = 0.1
#   DEFAULT_INSTRUCTION_MODE = design.INSTRUCTION_MODE_HW
    
    def __del__(self):
        if self.timer:
            self.timer.stop()
    
    def __init__(self, gui):
        # The current open design
        self.design = None
        
        self.gui = gui
        self.timer = None   
        self.comm = comm.Comm()
        
        # A list of folder mappings for the source code.
        # This is useful if you are debugging on a different file
        # system than where the compile was done.   
        self.folder_mapping = {}
        
        # Debugging state
        self._currentState = (None, None)
        self._executionMode = backend.BACKEND_FPGA_LIVE
        self._instruction_mode = None

        # Trigger functions
        self.executionModeChanged = signals.Signal()
        self.stateChanged = signals.Signal()
        self.comm_disconnected = signals.Signal()
        self.traceMaxChanged = signals.Signal()
        self.designClosed = signals.Signal()
        self.designOpened = signals.Signal()
        self.errorOccurred = signals.Signal()
        self.minorMessage = signals.Signal()
        self.majorMessage = signals.Signal()
        self.enteringLongWork = signals.Signal()
        self.leavingLongWork = signals.Signal()
        
        # Initialize back-ends
        self.backends = {}
        self.backends[backend.BACKEND_FPGA_LIVE] = hardware.Hardware(self)
        self.backends[backend.BACKEND_FPGA_REPLAY] = replay.Replay(self)
        self.backends[backend.BACKEND_MODELSIM] = sim.Simulate(self)
        
#       self.instruction_mode = self.DEFAULT_INSTRUCTION_MODE
        
#       self.backends[self.executionMode].updateState()
        
        if self.gui:
            self.timer = QtCore.QTimer()
            self.timer.setInterval(self.TIMER_INTERVAL * 1000)
            self.timer.timeout.connect(self.timer_timeout)
            self.timer.start()

    @property
    def activeBackend(self):
        return self.backends[self.executionMode]

    @property
    def currentState(self):
        return self._currentState
        
    @currentState.setter
    def currentState(self, newState):
#       if newState != self.currentState:
        self._currentState = newState
        self.stateChanged.emit() 

    @property
    def breakpoints(self):
        if self.activeBackend.IMPLEMENTS_BREAKPOINTS:
            return self.activeBackend.breakpoints
        else:
            return []

    @property
    def executionMode(self):
        return self._executionMode
    
    @executionMode.setter
    def executionMode(self, executionMode):
        
        if executionMode == self.executionMode:
            # No change
            return

        try:
            self.backends[executionMode].activate()
        except backend.CannotActivate as e:
            self.errorMessage(str(e))
            return

        self._executionMode = executionMode            
        
        if self.design:
            self.activeBackend.newDesign()
            
        self.executionModeChanged.emit()
        
    # Forward these to the backend
    def updateState(self):
        self.activeBackend.updateState()
        
    def stepBack(self):
        self.activeBackend.stepBack()
        
    def step(self):
        self.activeBackend.step()
        
    def run(self):
        self.activeBackend.run()

    def pause(self):
        self.activeBackend.pause()

    def reset(self):
        self.activeBackend.reset()
        
    def getBreakpointLocations(self):
        return [(b.filePath, b.lineNum) for b in self.breakpoints]
        
    def breakpointAdd(self, filePath, lineNum):
        if not self.activeBackend.IMPLEMENTS_BREAKPOINTS:
            self.minorMessage.emit("Execution mode does not support breakpoints")
            return

        assert (filePath, lineNum) not in self.getBreakpointLocations()

        try:
            breakpoint = backend.Breakpoint(self.design, filePath, lineNum)
        except backend.InvalidBreakpointLocation as e:
            self.minorMessage.emit("Invalid breakpoint location.  " + e.msg)
            return
        
        try:
            self.activeBackend.breakpointAdd(breakpoint)            
        except backend.InvalidBreakpointLocation as e:
            self.errorMessage("Invalid breakpoint location.  " + e.msg)
        except backend.CannotSetBreakpoint as e:
            self.errorMessage("Cannot set breakpoint. " + e.msg) 
        
    def breakpointRemove(self, filePath, lineNum):
        assert self.activeBackend.IMPLEMENTS_BREAKPOINTS
        
        matches = [b for b in self.activeBackend.breakpoints if b.filePath == filePath and b.lineNum == lineNum]
        assert len(matches) == 1
        
        breakpoint = matches[0]
        self.activeBackend.breakpointRemove(breakpoint)
        
    def varRead(self, var, offset, size):
        return self.activeBackend.varRead(var, offset, size)
    
    def setTraceCycle(self, cycle):
        self.activeBackend.setTraceCycle(cycle)

    def getTraceCycle(self):
        return self.activeBackend.getTraceCycle()

    def numTraceCycles(self):
        return self.activeBackend.numTraceCycles()
        
    def timer_timeout(self):                
        do_refresh = self.comm.check_for_refresh()        
        
        if do_refresh:
            self.activeBackend.updateState()

    def error_serial_connection(self):
        self.errorMessage("Serial connection is not connected.")
        
    def trace_cycle(self):
        return self.trace.cycle
        
    def errorMessage(self, message):
        self.errorOccurred.emit(message)
        
    def openDesign(self, path):
#       assert os.path.isdir(path)
        
        if self.executionMode == backend.BACKEND_FPGA_REPLAY:
            self.executionMode = backend.BACKEND_FPGA_LIVE
        
        self.closeDesign()              
        self.design = design.Design(self, path)
        
        try:
            self.design.populateFromDatabase()
                    
        except design.DesignNoDatabase as e:
            self.errorOccurred.emit("Error: Could not open design " + self.design.path + ". Missing database.")
            del self.design
            self.design = None
        except design.DesignDatabaseCorrupt as e:
            self.errorOccurred.emit("Error: Design " + path + " is corrupted or an old version (" + e.msg + ")")
            del self.design
            self.design = None
            
        if not self.design:
            return
        
        self.comm.systemIdVerified = False
                
        self.designOpened.emit()        
        self.activeBackend.newDesign()
    
    def closeDesign(self):
        self.activeBackend.closeDesign()
        del self.design
        self.design = None
        self.designClosed.emit()
        self.currentState = (None, None)

    def getActiveInsns(self):
        instanceNum, stateNum = self.currentState
        if not instanceNum:
            return None
    
        fcn = self.design.instanceNumToInstance[instanceNum].function   
        return fcn.getActiveInsns(stateNum)
    
    def commConnected(self):
        return self.comm.connected() and self.comm.systemIdVerified
    
    def commConnect(self, port_name):
        assert self.executionMode == backend.BACKEND_FPGA_LIVE
        
        # First we need to connect to the serial
        if not self.comm.connected():
            try:
                self.comm.connect(port_name)                            
            except comm.CannotConnect as e:
                self.errorMessage("Could not connect to " + port_name + ". " + e.args[0])
                return
        
        # Then verify the serial
        assert self.comm.connected()
        self.activeBackend.serialHasConnected()
    
    def commDisconnect(self):
        self.comm.disconnect()
    
    def processGuiEvents(self):
        if self.gui:
            self.gui.processEvents()
            if not self.gui.isVisible():
                raise UserDisconnected
