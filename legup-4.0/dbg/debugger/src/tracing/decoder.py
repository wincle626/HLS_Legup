import math
import bisect

class TraceBufferLine():    
    def __init__(self, decoder):
        self.decoder = decoder
        self.idx = None
        
    def set_idx_is_second_pass(self):
        self.idx = self.idx | (2**self.decoder.manager.design.configBufferCtrlAddrBits)

class TraceCycle():
    def __init__(self, instanceNum, stateNum):
        self.instanceNum = instanceNum
        self.stateNum = stateNum
        
        self.memoryLines = []
        self.signalValues = []
        
    def print(self):
        print("Cycle: (" + str(self.instanceNum) + "," + str(self.stateNum) + ")")
        for m in self.memoryLines:
            print("\tMem Addr: " + str(m.addr) + " Val: " + str(m.val))
        for v in self.signalValues:
            print("\tSignal: " + v[0].name + " Val: " + str(v[1]))

# This decodes a line of data from the control trace buffer
class ControlBufferLine(TraceBufferLine):
    def __init__(self, decoder, data):
        super().__init__(decoder)
        
        proj = decoder.manager.design
        
        data = int.from_bytes(data, byteorder='little')
        
        self.sequence = data & (2**proj.configBufferCtrlSequenceBits-1)
        data = data >> proj.configBufferCtrlSequenceBits
        
        self.pc_state = data & (2**proj.configStateNumBitWidth-1)
        data = data >> proj.configStateNumBitWidth
        
        self.pc_module = data & (2**proj.configInstanceNumBitWidth-1) 
        data = data >> proj.configInstanceNumBitWidth
        
# This decodes a line of data from the Data trace buffer 
class DataBufferLine(TraceBufferLine):
    def __init__(self, decoder, data):
        super().__init__(decoder)       
        
        proj = decoder.manager.design
        
        self.data = int.from_bytes(data, byteorder='little')
        
        self.val = self.data & (2**proj.configMemDataBits - 1)
        self.data = self.data >> proj.configMemDataBits
        
        self.addr = self.data & (2**proj.configMemAddrBits - 1)
        self.data = self.data >> proj.configMemAddrBits
        
        assert self.data in [0, 1, 2, 3]
        
        # size is the number of bytes written: 1, 2, 4, or 8
        self.size = 2 ** self.data
        
        # Verify alignment - Although the code handles unaligned 
        # writes, I don't think they occur in LegUp, so I've added
        # this as an early check that the data is decoded properly.
        assert self.addr % self.size == 0
         
                
#       print("Tag: " + str(self.addr>>23) + " Offset: " + str(self.addr & (2**23-1)) + " Val: " + str(self.val))
        
# This decodes a line of data from the Data trace buffer 
class RegsBufferLine(TraceBufferLine):
    def __init__(self, decoder, data):
        self.data = int.from_bytes(data, byteorder='little')
        
        # Get regs data
        self.val = self.data
        
#       print("Reg: " + str(self.val))
        
# This class decodes data from the trace buffer memories
class TraceBufferDecoder():
    def __init__(self, manager):
        self.manager = manager
        design = self.manager.design

        # Control Buffer
        assert design.configBufferCtrlWidth == design.configInstanceNumBitWidth + design.configStateNumBitWidth + design.configBufferCtrlSequenceBits
#         print("Control Flow Trace Buffer: " + str(design.configBufferCtrlWidth) + " bits wide, " + str(design.configBufferCtrlDepth) + " entries deep.")
        
        # Local Memory Buffer
#         print("Memory Updates Trace Buffer: " + str(design.configBufferMemWidth) + " bits wide, " + str(design.configBufferMemDepth) + " entries deep.")
        
        # Regs Trace Buffer
#         if (design.configBufferRegsEnabled):
#             print("Datapath Registers Trace Buffer: " + str(design.configBufferRegsWidth) + " bits wide, " + str(design.configBufferRegsDepth) + " entries deep.")
#         else:
#             print("Datapath Registers Trace Buffer: disabled")
#             
        # Readback bytes
        self.traceControlReadbackBytes = math.ceil(design.configBufferCtrlWidth / 8)
        self.traceDataReadbackBytes = math.ceil(design.configBufferMemWidth / 8)
        if design.configBufferRegsEnabled:
            self.traceRegsReadbackBytes = math.ceil(design.configBufferRegsWidth / 8)
        else:
            self.traceRegsReadbackBytes = 0
        
        self.controlLines = []
        self.memoryLines = []
        self.regsLines = []
    
        
    def addControlLine(self, data):    
        new_line = ControlBufferLine(self, data)
        self.controlLines.append(new_line)
        
    def addMemoryLine(self, data):
        new_line = DataBufferLine(self, data)
        self.memoryLines.append(new_line)
        
    def addRegsLine(self, data):
        new_line = RegsBufferLine(self, data)
        self.regsLines.append(new_line)
        
    # Use the data from the trace buffers to fill the Trace class
    def toTrace(self, trace):
        design = self.manager.design
        
        traceCycles = []
        traceCyclesWithData = []
        
        # Decompress the control lines
        for ctrlLine in self.controlLines:
            for seq in range(ctrlLine.sequence + 1):
                assert ctrlLine.pc_module
                traceCycles.append(TraceCycle(ctrlLine.pc_module, ctrlLine.pc_state + seq))
                
        
        cycleIter = reversed(traceCycles)
        memIter = reversed(self.memoryLines)
        regIter = reversed(self.regsLines)
        
        # This gets switches to true once any of the buffers run out and we don't 
        # have enough data to add another complete cycle of history.
        exhausted = False
        
        # Since registers can be traced in a delayed fashion, as we iterate over the register buffer
        # from most recent to oldest, we will encounter register values that belong to an earlier
        # cycle.  We keep them in this list of lists [[delay, (signal, val)]], decrementing delay each
        # iteration until it is 0, which means the signal value belongs to the current cycle. 
        delayedSigTraces = []
        
        while not exhausted:
            # Get the next control line
            try:
                traceCycle = next(cycleIter)
            except StopIteration:
                exhausted = True 
            
            if not exhausted:
                # Get the instance/state for this cycle
                f = design.getFunctionFromInstanceNum(traceCycle.instanceNum)
                if not f:
                    print(traceCycle.instanceNum)
                assert f
                state = f.getStateByNum(traceCycle.stateNum)
                assert state
                
                # Get the memory writes
                if state.storeB:
                    try:
                        memLine = next(memIter)
                    except StopIteration:
                        exhausted = True
                    else:
                        traceCycle.memoryLines.append(memLine)
                        
                if state.storeA:
                    try:
                        memLine = next(memIter)
                    except StopIteration:
                        exhausted = True
                    else:
                        traceCycle.memoryLines.append(memLine)
            
            if not exhausted:
                # Get the register values, and merge them if necessary
                regValue = 0
                
                if state.traceRegsB:
                    try:
                        regsB = next(regIter)
                    except StopIteration:
                        exhausted = True
                    else:
                        regValue |= (regsB.val << design.configBufferRegsWidth)
                    
                if state.traceRegsA:
                    try: 
                        regsA = next(regIter)
                    except StopIteration:
                        exhausted = True
                    else:
                        regValue |= regsA.val
                        
                if state.traceRegsA:
                    traceCycle.regValue = regValue
                    
                    # Find signals that are scheduled to this trace line
                    signalTraces = [(sig, sigTrace) for sig in f.signals for sigTrace in sig.traces if sigTrace.state == state]
                    
                    for (sig, sigTrace) in signalTraces:
                        val = regValue >> sigTrace.lo
                        val &= (1 << (sigTrace.hi - sigTrace.lo + 1)) - 1
                        signalValPair = (sig, val)
                        delayedSigTraces.append([sigTrace.delay, signalValPair])
            
            # Add delayedSigTraces with delay=0 to this cycle
            for s in delayedSigTraces:
                if s[0] == 0:
                    traceCycle.signalValues.append(s[1])
                
            # Remove everything with delay=0
            delayedSigTraces = [s for s in delayedSigTraces if s[0] > 0]
                                
            # Decrement all delayedSigTraces
            for s in delayedSigTraces:
                s[0] = s[0] - 1
                    
            # Only consider this cycle if we have all of the necessary data
            # If exhausted=True, then a buffer was missing required data 
            if not exhausted:
#                 traceCycle.print()
                traceCyclesWithData.insert(0, traceCycle)
        
        
        # Now that we have decoded all of the information from the trace buffers into
        # cycle by cycle information, we need to add it to the Trace object
        for c in traceCyclesWithData:           
            trace.newCycle(c.instanceNum, c.stateNum)
                
            for d in c.memoryLines:
                tag = d.addr >> 23
#               if tag == 0:
#                   print("Ignoring tag 0")
#                   continue
                offset = d.addr & (2**23-1) 
                ram = self.manager.design.ramFromTag(tag)
                assert(ram)             
                trace.newRamUpdate(ram, offset, d.size, d.val)
                    
            for s in c.signalValues:
                trace.newSignalUpdate(s[0], s[1])
                
        
                    