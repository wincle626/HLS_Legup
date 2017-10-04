import abc

######################## EXCEPTIONS #######################
BACKEND_FPGA_LIVE, BACKEND_FPGA_REPLAY, BACKEND_MODELSIM = range(3)

class CannotActivate(Exception):
    pass

class CannotLoadProject(Exception):
    pass
   
class InvalidSystemID(Exception):
    def __init__(self, system_id):
        self.configSystemID = system_id
        
class CannotSetBreakpoint(Exception):
    def __init__(self, msg):
        self.msg = msg
        
class InvalidBreakpointLocation(Exception):
    def __init__(self, msg):
        self.msg = msg
        
class TooManyBreakpoints(Exception):
    pass
  
# This is an abstract class
class Backend(metaclass=abc.ABCMeta):
    
    IMPLEMENTS_BACKSTEP = False
    IMPLEMENTS_RESET = False
    IMPLEMENTS_RUN = False
    IMPLEMENTS_TRACE = False
    IMPLEMENTS_BREAKPOINTS = False
    IMPLEMENTS_PAUSE = False
    
    VARIABLES_AUTO_REFRESH = False
    
    def __init__(self):
        if self.IMPLEMENTS_BACKSTEP:
            assert "stepBack" in dir(self)
        if self.IMPLEMENTS_RESET:
            assert "reset" in dir(self)
        if self.IMPLEMENTS_RUN:
            assert "run" in dir(self)
        if self.IMPLEMENTS_PAUSE:
            assert "pause" in dir(self)
        if self.IMPLEMENTS_TRACE:
            assert "setTraceCycle" in dir(self)
            assert "getTraceCycle" in dir(self)
        if self.IMPLEMENTS_BREAKPOINTS:
            assert "breakpointAdd" in dir(self)
            assert "breakpointRemove" in dir(self)
        
    @abc.abstractmethod
    def step(self):
        pass
    
    @abc.abstractmethod
    def updateState(self):
        pass
    
    @abc.abstractmethod
    def activate(self):
        pass
    
    @abc.abstractmethod
    def newDesign(self):
        pass
    
    @abc.abstractmethod
    def closeDesign(self):
        pass
    
    @abc.abstractmethod
    def clientDisconnected(self):
        pass
    
class Breakpoint():
    def __init__(self, design, filePath, lineNum):
        self.filePath = filePath
        self.lineNum = lineNum
        
        # Find first insn at (filePath, lineNum)
        try:
            insns = design.insnsBySourceFileLine[filePath][lineNum]
        except KeyError:
            raise InvalidBreakpointLocation("No IR instructions at this line")
        else:
            self.insn = min(insns, key = lambda x: x.startState.number)
