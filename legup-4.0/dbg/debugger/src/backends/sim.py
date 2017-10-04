import os

from tracing import trace
import paths
from backends import backend
import manager
import modelsim

class Simulate(backend.Backend):
    IMPLEMENTS_BACKSTEP = True
    VARIABLES_AUTO_REFRESH = True
    IMPLEMENTS_TRACE = True
    IMPLEMENTS_RUN = True
    IMPLEMENTS_BREAKPOINTS = True
    IMPLEMENTS_PAUSE = True
    
    MEM_READ_DELAY = 2

    class FutureRamUpdate():
        def __init__(self, delay, port, ram, offset, size):
            self.delay = delay
            self.port = port
            self.ram = ram
            self.offset = offset
            self.size = size

    def __init__(self, manager):
        super().__init__()
        
        self.modelsim = None
        self.manager = manager      
        self.trace = None
        self.breakpoints = []
        self.paused = True
        
    def activate(self):
        if self.manager.design and self.manager.design.isXilinx:
            raise backend.CannotActivate("Simulation mode not supported for Xilinx devices.")
                
    def closeDesign(self):
        if self.modelsim:
            self.modelsim.stop()
        
    def clientDisconnected(self):
        if self.modelsim:
            self.modelsim.stop()
        
    def newDesign(self):
        assert self.manager.design
        
        if self.manager.design.isXilinx:
            raise backend.CannotActivate("Simulation mode not supported for Xilinx devices.")
        
        self.initializeAlteraFPPaths()
        
        self.modelsim = modelsim.Modelsim()
        try:
            self.modelsim.run()
        except modelsim.CannotFindModelsim:
            errMsg = "Simulate executable (vsim) is not on the PATH"
            raise backend.CannotActivate(errMsg)

        self.trace = trace.Trace(self.manager)      
        self.cycle = 0
        
        if not os.path.exists(self.projectModelsimPath()):
            os.mkdir(self.projectModelsimPath())
        
        self.manager.enteringLongWork.emit()
        
#       self.compileDesign()
        verilogFilePath = self.manager.design.verilogFilePath()
        testbenchCompilePath = os.path.join(self.projectModelsimPath(), "work", "main_tb")
        
        verilogTime = os.path.getmtime(verilogFilePath)
        try:
            tbTime = os.path.getmtime(testbenchCompilePath)
        except FileNotFoundError:
            tbTime = 0
        
        if verilogTime > tbTime:
            self.manager.leavingLongWork.emit()
            self.manager.majorMessage.emit("The Verilog file has changed since last Simulate compilation.  The Verilog will now be recompiled.")
            self.manager.enteringLongWork.emit()
            self.compileDesign()

        self.simDesign()
        

        self.manager.leavingLongWork.emit()
        
        self.futureRamUpdates = []
        
        self.runOneClockCycle()
        self.runOneClockCycle()
        self.step()
        self.updateState()
        
    def breakpointAdd(self, breakpoint):
        self.breakpoints.append(breakpoint)
        
    def breakpointRemove(self, breakpoint):
        self.breakpoints.remove(breakpoint)
        
    def run(self):
        self.paused = False
        
        # Get breakpoint instanceNum, stateNum
        breakpointLocations = [(b.insn.function, b.insn.startState.number) for b in self.breakpoints]
        
        self.manager.enteringLongWork.emit()
        while True:
            
            if self.cycle == self.trace.numCycles() and self.modelsim.socketClosed:
                self.manager.minorMessage.emit("Simulation has ended")
                break

            self.step(sendUpdateStateSignal = False)
            
            # Have we hit a breakpoint?
            (instanceNum, stateNum) = self.trace.getState(self.cycle)
            fcn = self.manager.design.instanceNumToInstance[instanceNum].function
            location = (fcn, stateNum)
            
            if location in breakpointLocations:
                break
            
            try:
                self.manager.processGuiEvents()
            except manager.UserDisconnected:
                break
            
            if self.paused:
                break
        
        self.manager.leavingLongWork.emit()
        self.updateState()
        
    def pause(self):
        if self.paused:
            self.manager.minorMessage.emit("Design is already paused")
        else:
            self.paused = True
        
    def getInstanceState(self, instance, hierarchyPath):
        csSignalName = hierarchyPath + "/cur_state"
        cs = int(self.getSignalValue(csSignalName))
        
        functionCalled = instance.function.getFunctionCalledInStateNum(cs)
        if not functionCalled:
            return (instance.number, cs)
        else:
            finishedSignalName = hierarchyPath + "/" + functionCalled.name + "_finish_final"
            finished = int(self.getSignalValue(finishedSignalName))
            
            if finished:
                return (instance.number, cs)
            else:           
                childInstance = instance.findChildInstanceByFunction(functionCalled)
                return self.getInstanceState(childInstance, hierarchyPath + "/" + functionCalled.name) 
        
    def step(self, sendUpdateStateSignal = True):
        design = self.manager.design
        
        if self.cycle == self.trace.numCycles():
            if self.modelsim.socketClosed:
                self.manager.minorMessage.emit("Simulation has ended.")
                return
                
            self.runOneClockCycle()
            
            # The only way we know the simulation has ended is that the 
            # socket is closed.  So we need to send a command and test the 
            # socket after running the design for any amount of time.
            self.modelsim.sendCmd("set dummy_test_socket 0") 
            if not self.modelsim.socketClosed:
                mainFunction = design.getFunctionByName("main")
                assert len(mainFunction.instances) == 1
                startInstance = mainFunction.instances[0]
                startHierarchyPath = "/main_tb/top_inst/main_inst"
                (instanceNum, stateNum) = self.getInstanceState(startInstance, startHierarchyPath)
                
                self.trace.newCycle(instanceNum, stateNum)
                
                # Get writ eenable signals
                if (design.optionDebugRtlEnabled):
                    memCtrlSigNamePrefix = "/main_tb/top_inst/main_memory_controller"
                else:
                    memCtrlSigNamePrefix = "/main_tb/top_inst/memory_controller"
                    
                memPorts = ["a", "b"]
                for port in memPorts:
                    en = int(self.getSignalValue(memCtrlSigNamePrefix + "_enable_" + port))
                    wrEn = int(self.getSignalValue(memCtrlSigNamePrefix + "_write_enable_" + port))
                    if (wrEn):
                        wrAddr = int(self.getSignalValue(memCtrlSigNamePrefix + "_address_" + port))
                        wrData = int(self.getSignalValue(memCtrlSigNamePrefix + "_in_" + port))
                        wrSize = int(self.getSignalValue(memCtrlSigNamePrefix + "_size_" + port))
                        assert wrSize in [0, 1, 2, 3]
                        tag = wrAddr >> 23
                        ram = self.manager.design.ramFromTag(tag)
                        offset = wrAddr & ((1 << 23) - 1)
                        size = 2 ** wrSize
                        self.trace.newRamUpdate(ram, offset, size, wrData)                        
                    elif (en):
                        # This is a read, which will give us additional information about what is in the RAMs
                        # But we have to wait self.MEM_READ_DELAY before getting the value.
                        rdAddr = int(self.getSignalValue(memCtrlSigNamePrefix + "_address_" + port))
                        rdSize = int(self.getSignalValue(memCtrlSigNamePrefix + "_size_" + port))
                        assert rdSize in [0, 1, 2, 3]
                        tag = rdAddr >> 23
                        ram = self.manager.design.ramFromTag(tag)
                        offset = rdAddr & ((1 << 23) - 1)
                        size = 2 ** rdSize
                        futureRamUpdate = self.FutureRamUpdate(self.MEM_READ_DELAY, port, ram, offset, size)
                        self.futureRamUpdates.append(futureRamUpdate)
                        
                # Get reads that were requested self.MEM_READ_DELAY cycles ago
                ramUpdates = [ru for ru in self.futureRamUpdates if ru.delay == 0]
                
                assert len(ramUpdates) <= 2
                for ru in ramUpdates:
                    rdData = int(self.getSignalValue(memCtrlSigNamePrefix + "_out_" + ru.port))
                    self.trace.newRamUpdate(ru.ram, ru.offset, ru.size, rdData, self.MEM_READ_DELAY)
                    
                ramUpdates = [ru for ru in self.futureRamUpdates if ru.delay != 0]
                
                # Update future delays
                for f in self.futureRamUpdates:
                    f.delay -= 1
    
                self.manager.traceMaxChanged.emit(self.trace.numCycles())
            
            if not self.modelsim.socketClosed:
                self.cycle += 1
        else:
            self.cycle += 1
            
        if sendUpdateStateSignal:
            self.updateState()
        
    def stepBack(self):
        if self.cycle:
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
        try:
            return self.trace.getVarValue(self.cycle, var, offset, size)
        except trace.VariableHasNoSource:
            raise manager.VariableOptimizedAway
        except trace.VariableNoSourceYet:
            raise manager.VariableNotAccessible     
#         except trace.VariableValueIsUndefined:
#             raise manager.VariableValueUndefined
        except trace.VariableNoValueYet:
            # Since modelsim should have a trace from the start of execution
            # if the variable hasn't been written to yet, report it as undefined
            raise manager.VariableValueUndefined
        
    def runOneClockCycle(self):
        cmd = "run 20ns"
        self.modelsim.sendCmd(cmd)

    def getSignalValue(self, signal):
        cmd = "examine -unsigned -value " + signal;
        return self.modelsim.sendCmd(cmd)

    def compileDesign(self):
        assert self.manager.design
        
        workDir = self.projectModelsimPath()
                
        scriptPath = os.path.join(workDir, "compile.tcl")
        
        fp = open(scriptPath, 'w')
        fp.write("cd " + workDir + "\n")
        fp.write("vlib work\n")
        fp.write("vlog -work work " + self.manager.design.verilogFilePath() + "\n")
        for verilogPath in self.verilogPaths:
            fp.write("vlog -work work " + verilogPath + "\n")    
        fp.close();
        
        doCommand = "do \"" + scriptPath + "\"";
        self.modelsim.sendCmd(doCommand)    
        
    def simDesign(self):
        assert self.manager.design
        
        workDir = self.projectModelsimPath()
        
        simLogFile = os.path.join(workDir, "vsim.wlf")
        if os.path.isfile(simLogFile):
            os.remove(simLogFile)
        
        scriptPath = os.path.join(workDir, "sim.tcl")
        
        alteraMFLibPath = os.path.abspath(os.path.join(modelsim.Modelsim.getVsimPath(), os.pardir, os.pardir, "altera", "verilog", "altera_mf"))
        alteraCycloneIIPath = os.path.abspath(os.path.join(modelsim.Modelsim.getVsimPath(), os.pardir, os.pardir, "altera", "verilog", "cycloneii"))
        
        fp = open(scriptPath, 'w')
        fp.write("cd " + workDir + "\n")
        fp.write("vsim -novopt -Lf " + alteraMFLibPath + " -Lf " + alteraCycloneIIPath + " work.main_tb\n")
        fp.write("add wave \"/main_tb/pc_module\"\n")
        fp.write("add wave \"/main_tb/pc_state\"\n")
        fp.write("add wave \"/main_tb/top_inst/main_memory_controller_write_enable_a\"\n")
        fp.write("add wave \"/main_tb/top_inst/main_memory_controller_address_a\"\n")
        fp.write("add wave \"/main_tb/top_inst/main_memory_controller_in_a\"\n")
        fp.write("add wave \"/main_tb/top_inst/main_memory_controller_write_enable_b\"\n")
        fp.write("add wave \"/main_tb/top_inst/main_memory_controller_address_b\"\n")
        fp.write("add wave \"/main_tb/top_inst/main_memory_controller_in_b\"\n")
        fp.close();
        
        doCommand = "do \"" + scriptPath + "\"";
        self.modelsim.sendCmd(doCommand)
        
    def initializeAlteraFPPaths(self):
        hdlDir = paths.getRtlDir()
        alteraIpDir = paths.getAlteraIpDir()
        
        self.verilogPaths = []
        self.verilogPaths.append(os.path.join(alteraIpDir, "altera_mf.v"))
        self.verilogPaths.append(os.path.join(alteraIpDir, "220model.v"))
        self.verilogPaths.append(os.path.join(alteraIpDir, "altfp_adder_14.v"))
        self.verilogPaths.append(os.path.join(alteraIpDir, "altfp_adder64_14.v"))
        self.verilogPaths.append(os.path.join(alteraIpDir, "altfp_subtractor_14.v"))
        self.verilogPaths.append(os.path.join(alteraIpDir, "altfp_subtractor64_14.v"))
        self.verilogPaths.append(os.path.join(alteraIpDir, "altfp_multiplier_11.v"))
        self.verilogPaths.append(os.path.join(alteraIpDir, "altfp_multiplier64_11.v"))
        self.verilogPaths.append(os.path.join(alteraIpDir, "altfp_divider_33.v"))
        self.verilogPaths.append(os.path.join(alteraIpDir, "altfp_divider64_61.v"))
        self.verilogPaths.append(os.path.join(alteraIpDir, "altfp_compare32_1.v"))
        self.verilogPaths.append(os.path.join(alteraIpDir, "altfp_compare64_1.v"))
        self.verilogPaths.append(os.path.join(alteraIpDir, "altfp_truncate_3.v"))
        self.verilogPaths.append(os.path.join(alteraIpDir, "altfp_extend_2.v"))
        self.verilogPaths.append(os.path.join(alteraIpDir, "altfp_sitofp32_6.v"))
        self.verilogPaths.append(os.path.join(alteraIpDir, "altfp_sitofp64_6.v"))
        self.verilogPaths.append(os.path.join(alteraIpDir, "altfp_fptosi32_6.v"))
        self.verilogPaths.append(os.path.join(alteraIpDir, "altfp_fptosi64_6.v"))
        self.verilogPaths.append(os.path.join(alteraIpDir, "altfp_adder_13.v"))
        self.verilogPaths.append(os.path.join(alteraIpDir, "altfp_adder64_13.v"))
                
        self.verilogPaths.append(os.path.join(hdlDir, "hlsd.v"))
        self.verilogPaths.append(os.path.join(hdlDir, "comm.v"))
        self.verilogPaths.append(os.path.join(hdlDir, "trace.v"))
        self.verilogPaths.append(os.path.join(hdlDir, "trigger.v"))
        self.verilogPaths.append(os.path.join(hdlDir, "rams_altera.v"))
        self.verilogPaths.append(os.path.join(hdlDir, "uart_control.v"))
        self.verilogPaths.append(os.path.join(hdlDir, "uart_altera.v"))
        
        board = self.manager.design.board
        if board == "DE2" or board == "CycloneIIAuto":
            self.verilogPaths.append(os.path.join(hdlDir, "clockbuf_cycloneii.v"))
        elif board == "DE4":
            self.verilogPaths.append(os.path.join(hdlDir, "clockbuf_stratixiv.v"))
        else:
            assert False

        self.verilogPaths.append(os.path.join(hdlDir, "rs232.v"))
        
    def projectModelsimPath(self):
        assert self.manager.design
    
        # I used to run modelsim in a separate directory, but the .mif files were missing.  
        # Although I could copy the mif files, I just run in the design directory instead.
        return self.manager.design.path
#       return os.path.join(self.manager.design.path, "modelsim")
    

        

