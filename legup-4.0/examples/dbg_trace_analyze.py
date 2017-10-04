#!/usr/bin/python3

import sys
import re
import os
import math

import mysql.connector

def printer(x):
    sys.stdout.write(str(x) + "\t")
    
class Instance():
    def __init__(self, instanceNum, function):
        self.instanceNum = instanceNum
        self.function = function

class Function():
    def __init__(self):
        self.states = []
        
    def getState(self, stateNum):
        matches = [s for s in self.states if s.stateNum == stateNum]
        assert len(matches) == 1
        return matches[0]

class State():
    def __init__(self, function, stateNum, storeA, storeB, regsA, regsB):
        self.function = function
        self.stateNum = stateNum
        self.storeA = storeA
        self.storeB = storeB
        self.regsA = regsA
        self.regsB = regsB
        self.tracedSignals = []
        
    def getTracedSigWidth(self):
        return sum([s.width for s in self.tracedSignals])

class RtlSignal():
    def __init__(self, width):
        self.width = width

class Design():
    def __init__(self, path):
        self.path = path
        self.functions = []
        self.instances = []
        
    def loadDatabase(self):
        mysqlConnection = mysql.connector.connect(user='root', 
                                                        password='letmein', 
                                                        host='localhost', 
                                                        database='legupDebug')
        mysqlCursor = mysqlConnection.cursor(buffered=True)
    
        query = "SELECT id, name " \
                "FROM Designs " \
                "WHERE path = %s"
        mysqlCursor.execute(query, [self.path])
        assert mysqlCursor.rowcount == 1
    
        (designId, self.name) = mysqlCursor.fetchone()
        
        query = "SELECT memoryAddrWidth, memoryDataWidth " \
                "FROM DesignProperties " \
                "WHERE designId = %s"
        mysqlCursor.execute(query, [designId])
        (self.memAddrWidth, self.memDataWidth) = mysqlCursor.fetchone()
        
        query = "SELECT numInstanceBits, numStateBits " \
                "FROM InstrumentationProperties " \
                "WHERE designId = %s"
        mysqlCursor.execute(query, [designId])
        (self.pcInstanceWidth, self.pcStateWidth) = mysqlCursor.fetchone()
        
        query = "SELECT controlBufWidth, controlBufSequenceBits, controlBufDepth, " \
                "memoryBufWidth, memoryBufDepth, regsBufEnabled, regsBufWidth, regsBufDepth " \
                "FROM TraceBufferProperties " \
                "WHERE designId = %s"
        mysqlCursor.execute(query, [designId])
        (self.ctrlBufWidth, self.sequenceBits, self.ctrlBufDepth, 
         self.memBufWidth, self.memBufDepth, self.regsBufEnabled, self.regsBufWidth,
         self.regsBufDepth) = mysqlCursor.fetchone()
        
        query = "SELECT sum(rs.width) FROM RtlSignal rs " \
                "INNER JOIN Function f WHERE rs.functionId = f.id " \
                "AND rs.id IN (SELECT rtlSignalId FROM RtlSignalTraceSchedule) " \
                "AND f.designId = %s"
        mysqlCursor.execute(query, [designId])
        (self.regsBitsToTrace,) = mysqlCursor.fetchone()
        if self.regsBitsToTrace is None:
            self.regsBitsToTrace = 0
        
        query = "SELECT id FROM Function WHERE designId = %s"
        mysqlCursor.execute(query, [designId])
        functionsById = {}
        for (funcId,) in mysqlCursor:
            function = Function() 
            functionsById[funcId] = function
            self.functions.append(function)
            
        query = "SELECT s.id, belongingFunctionId, number, storeA, storeB, traceRegsPortA, traceRegsPortB " \
                "FROM State s INNER JOIN Function f ON s.belongingFunctionId = f.id " \
                "WHERE f.designId = %s"
        mysqlCursor.execute(query, [designId])
        statesById = {}
        for (stateId, funcId, stateNum, storeA, storeB, regsA, regsB) in mysqlCursor:
            function = functionsById[funcId]
            state = State(function, stateNum, storeA, storeB, regsA, regsB)
            statesById[stateId] = state
            function.states.append(state)
            
        query = "SELECT instanceNum, functionId " \
                "FROM Instance WHERE designId = %s"
        mysqlCursor.execute(query, [designId])
        for (instanceNum, funcId) in mysqlCursor:
            function = functionsById[funcId]
            instance = Instance(instanceNum, function)
            self.instances.append(instance)
            
        query = "SELECT rs.id, f.id, width " \
                "FROM RtlSignal rs " \
                "INNER JOIN Function f ON rs.functionId = f.id " \
                "WHERE f.designId = %s"
        mysqlCursor.execute(query, [designId])
        signalsById = {}
        for (sigId, funcId, width) in mysqlCursor:
            signal = RtlSignal(width)
            signalsById[sigId] = signal
            
        query = "SELECT rtlSignalId, recordInStateId, hiBit, loBit " \
                "FROM RtlSignalTraceSchedule rsts " \
                "INNER JOIN RtlSignal rs ON rsts.rtlSignalId = rs.id " \
                "INNER JOIN Function f ON rs.functionId = f.id " \
                "WHERE f.designId = %s"
        mysqlCursor.execute(query, [designId])
        for (sigId, stateId, hi, lo) in mysqlCursor:
            sig = signalsById[sigId]
            state = statesById[stateId]
            assert sig.width == (hi - lo + 1)
            state.tracedSignals.append(sig)

    def getInstance(self, instanceNum):
        matches = [i for i in self.instances if i.instanceNum == instanceNum]
        assert len(matches) == 1
        return matches[0]
        
 

        
class Cycle():
    def __init__(self, line):
        d = line.split()
        self.cycle = int(d[1])/20000
        self.instanceNum = int(d[2])
        self.stateNum = int(d[3])
        
class Trace:
    def __init__(self):
        self.cycles = []
    
    def loadTraceFile(self, path):
        lines = [line.rstrip('\n') for line in open(path)]
        for line in lines:
            if re.match("State\s+", line):
                s = Cycle(line)
                self.cycles.append(s)
 
class Simulation():
    def __init__(self, design):
        self.design = design
        
        
        self.regsUtilization = 0.0
    
    def simulateTrace(self, trace, ElaDepth):
        design = self.design
        
        lastInstanceNum = None
        lastStateNum = None
        memUseful = 0
        sequenceCnt = 0
        
        ctrlTrace = []
        ctrlLinesUsed = 0
        
        memTrace = []
        memLinesUsed = 0
        
        regsTrace = []
        regsLinesUsed = 0
        
        replayLengthHistory = []
        replayLengthHistoryEla = []
        
        cycleNum = 0
        bufferSaturated = False
        for c in trace.cycles:
            function = design.getInstance(c.instanceNum).function
            state = function.getState(c.stateNum)
            
            cyclesAvailable = [0, 0]
            if design.regsBufEnabled:
                cyclesAvailable.append(0)
            
            # Control
            cycleNum += 1
            
            if c.instanceNum == lastInstanceNum and c.stateNum == lastStateNum + 1 and sequenceCnt + 1 < 2**design.sequenceBits:
                sequenceCnt += 1
            else:
                ctrlTrace.append(cycleNum)
                ctrlLinesUsed += 1
                sequenceCnt = 0
                
            if len(ctrlTrace) > design.ctrlBufDepth:
                cyclesAvailable[0] = cycleNum - ctrlTrace[-(design.ctrlBufDepth+1)]
                bufferSaturated = True
            else:     
                cyclesAvailable[0] = cycleNum
            
            # Memory
            if state.storeA:
                memLinesUsed += 1
                memTrace.append(cycleNum)
            if state.storeB:
                memLinesUsed += 1
                memTrace.append(cycleNum)
                
            if len(memTrace) > design.memBufDepth and design.memBufDepth > 0:
                cyclesAvailable[1] = cycleNum - memTrace[-(design.memBufDepth+1)]
                bufferSaturated = True    
            else:     
                cyclesAvailable[1] = cycleNum
            
            # Register Tracing
            if design.regsBufEnabled:
                # At the end of functions, the state goes back to 0.  We don't want 
                # to re-record all the entry data again, so we only record in 
                # state 0 if we just came from another state.
                if not (state.stateNum == 0 and lastInstanceNum == c.instanceNum):            
                    memUseful += state.getTracedSigWidth()
                    if state.regsA:
                        regsLinesUsed += 1
                        regsTrace.append(cycleNum)
                    if state.regsB:
                        regsLinesUsed += 1
                        regsTrace.append(cycleNum)
                
                if len(regsTrace) > design.regsBufDepth:
                    cyclesAvailable[2] = cycleNum - regsTrace[-(design.regsBufDepth+1)]
                    bufferSaturated = True
                else:     
                    cyclesAvailable[2] = cycleNum
            
            if bufferSaturated:
                replayLength, idx = min((val, idx) for (idx, val) in enumerate(cyclesAvailable)) 
                replayLengthHistory.append(replayLength)
#                 offender[idx] += 1
        
                if (cycleNum < ElaDepth):
                    replayLengthHistoryEla.append(cycleNum)
                else:
                    replayLengthHistoryEla.append(ElaDepth)
            
            lastInstanceNum = c.instanceNum
            lastStateNum = c.stateNum
        
        if len(replayLengthHistory):
            self.averageReplayLength = sum(replayLengthHistory) / len(replayLengthHistory)
        else:
            # Benchmark did not run long enough to saturate a buffer
            self.averageReplayLength = None
            
        if len(replayLengthHistoryEla):
            self.averageReplayLengthEla = sum(replayLengthHistoryEla) / len(replayLengthHistoryEla)
        else:
            # Benchmark did not run long enough to saturate a buffer
            self.averageReplayLengthEla = None
            
        self.numCycles = len(trace.cycles)
        self.ctrlLinesUsed = ctrlLinesUsed
        self.memLinesUsed = memLinesUsed
        self.regsLinesUsed = regsLinesUsed
        
        if self.regsLinesUsed:
            self.regsUtilization = memUseful / (self.regsLinesUsed * design.regsBufWidth)
        else:
            self.regsUtilization = 0
           
        
def main():
    trace = Trace()
    trace.loadTraceFile(sys.argv[1])
    
    design = Design(os.getcwd())
    design.loadDatabase()
    
    ctrlBitsToTrace = design.pcInstanceWidth + design.pcStateWidth
    memBitsToTrace = 2 * (1 + design.memAddrWidth + design.memDataWidth)
    regsBitsToTrace = design.regsBitsToTrace
    
    totalBitsToTrace = ctrlBitsToTrace + memBitsToTrace + regsBitsToTrace
    
    
    studyMemSize = 100000
    elaDepth = math.floor(studyMemSize / totalBitsToTrace)
    
    sim = Simulation(design)
    sim.simulateTrace(trace, elaDepth)
    
    ctrlBufferMemUsed = sim.ctrlLinesUsed * design.ctrlBufWidth
    ctrlImprovement = float(ctrlBitsToTrace * len(trace.cycles)) / float(ctrlBufferMemUsed)

    memBufferMemUsed = sim.memLinesUsed * design.memBufWidth
    try:
        memImprovement = float(memBitsToTrace * len(trace.cycles)) / float(memBufferMemUsed)
    except ZeroDivisionError:
        memImprovement = float('inf')

    regBufferMemUsed = sim.regsLinesUsed * design.regsBufWidth
    try:
        regsImprovement = float(regsBitsToTrace * len(trace.cycles)) / float(regBufferMemUsed)
    except ZeroDivisionError:
        if regsBitsToTrace == 0:
            regsImprovement = None
        else:
            regsImprovement = float('inf')
    
    if sim.averageReplayLength and sim.averageReplayLengthEla:
        totalImprovement = sim.averageReplayLength / sim.averageReplayLengthEla
    else:
        totalImprovement = None
    
    ctrlFillRate = sim.ctrlLinesUsed / sim.numCycles
    memFillRate = sim.memLinesUsed / sim.numCycles
    regsFillRate = sim.regsLinesUsed / sim.numCycles
    
    memRelativeFillRate = memFillRate / ctrlFillRate
    regsRelativeFillRate = regsFillRate / ctrlFillRate
    
    memRelativeBufferSize = design.memBufDepth / design.ctrlBufDepth
    regsRelativeBufferSize = design.regsBufDepth / design.ctrlBufDepth
    
    printer(design.name)
    printer(design.regsBitsToTrace)
    printer(totalBitsToTrace)
    printer(ctrlImprovement)
    printer(memImprovement)
    printer(regsImprovement)
    printer(sim.regsUtilization)
    printer(totalImprovement)
    printer(memRelativeFillRate)
    printer(regsRelativeFillRate)
    printer(memRelativeBufferSize)
    printer(regsRelativeBufferSize)
    print("")
    
    fp = open("dbg_fill_rates.txt", 'w')
    fp.write(str(ctrlFillRate) + "\n")
    fp.write(str(memFillRate) + "\n")
    fp.write(str(regsFillRate) + "\n")
    fp.close()

main()