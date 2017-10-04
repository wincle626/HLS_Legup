import manager
from backends import backend
import sys

def testDesign(designPath):
    m = manager.Manager(gui = None)
    
    m.errorOccurred.connect(errorHandler)
    m.openDesign(designPath)
    m.executionMode = backend.BACKEND_MODELSIM
    
    (instanceNum, stateNum) = m.currentState
    f = m.design.getFunctionFromInstanceNum(instanceNum)
    assert f.name == "main"
    
    # We start in state 0 for an instrumented design and state 1 for non-instrumented
    # (should probably look into the reason for this difference.  Likely due to how the circuits are started).
    assert stateNum == 0 or stateNum == 1
        
    m.run()
    
    # Make sure we can still get the current state
    (instanceNum, stateNum) = m.currentState

    m.closeDesign()
    
def errorHandler(msg):
    print(msg)
    sys.exit(1)
    
    
testDesign(sys.argv[1])