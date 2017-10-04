from PyQt5 import QtWidgets
from PyQt5 import QtCore
from PyQt5 import QtGui
from PyQt5.QtCore import Qt

from . import linenums

class colNotMappedToCycle(Exception):
    pass

# Parent: SourceCodeTab
# Children: GanttLineNumbers, GanttScrollArea
class Gantt(QtWidgets.QWidget):
    def __init__(self, sourceFile):
        super().__init__()
        self.sourceFile = sourceFile
        
        self.grid = QtWidgets.QGridLayout()
        self.grid.setContentsMargins(0, 0, 0, 0)
        self.grid.setSpacing(0)
        
        self.setLayout(self.grid)
        
        self.lineNumberArea = linenums.LineNumberArea(self, self.sourceFile, False)        
        self.ganttScrollArea = GanttScrollArea(self)
        
        self.grid.addWidget(self.lineNumberArea, 0, 0)
        self.grid.addWidget(self.ganttScrollArea, 0, 1)
    
        # Link the source code scroll bar the the Gantt line #s and gantt chart
        self.sourceFile.sourceCodeViewer.textBox.updateRequest.connect(self.ganttScrollArea.update_area)
        self.sourceFile.sourceCodeViewer.textBox.updateRequest.connect(self.lineNumberArea.update_area)

    
# Parent: Gantt
# Children: GanttChart
class GanttScrollArea(QtWidgets.QScrollArea):
    def __init__(self, gantt):
        super().__init__()
        
        self.gantt = gantt        
        
        self.ganttChart = GanttChart(gantt)
            
        self.setVerticalScrollBarPolicy(QtCore.Qt.ScrollBarAlwaysOff)

        self.setWidgetResizable(True)
        self.setWidget(self.ganttChart)
        
    def wheelEvent(self, event):
        pass
    
    def gui(self):
        return self.codeTab.gui()
    
    def update_area(self, rect, dy):
        if dy:
            self.ganttChart.scroll(0, dy)

    def refresh(self):
        self.ganttChart.update()
    
    
class GanttChartAxisSection():
    ROUND_TO = 4
    
    def __init__(self, insn):
        self.startingCycle = insn.startState.number
        self.endingCycle = insn.getGanttEndStateNum()
        self.startingCol = None
        self.endingCol = None
    
    def addInsn(self, insn):
        self.endingCycle = max(self.endingCycle, insn.getGanttEndStateNum())

    def roundStartEnd(self):
        # Round down to nearest multiple of ROUND_TO
        self.startingCycle = self.startingCycle - (self.startingCycle % self.ROUND_TO)
        
        # Round up to nearest multiple of ROUND_TO
        if self.endingCycle % self.ROUND_TO:
            self.endingCycle += self.ROUND_TO - (self.endingCycle % self.ROUND_TO)

#     def setColStart(self, col):
#         self.startingCol = col
#         self.endingCol = col + (self.endingCycle - self.startingCycle)

class GanttChartSection():
    MAX_GAP = 1
    AXIS_SECTION_SPACING = 6
    
    def __init__(self, file, fcn, color):
        self.file = file
        self.fcn = fcn
        self.color = color
        self.axisSections = None
        self.startingLine = None
        self.endingLine = None
        self.emptyLines = None
        self.insns = []
        self.axisLineNum = None
        
    def printer(self):
        print("File: " + self.file + " Fcn: " + self.fcn.name +  " Lines: " + str(self.startingLine) + "-" + str(self.endingLine))
#     def addLine(self, lineNum):
#         self.lineNums.append(lineNum)

    def cycleToCol(self, cycle):
        sectionStartCol = 0
        for a in self.axisSections:
            if cycle < a.startingCycle:
                return None
            if cycle <= a.endingCycle:
                return sectionStartCol + cycle - a.startingCycle
            sectionStartCol += a.endingCycle - a.startingCycle + self.AXIS_SECTION_SPACING
    
    def colToCycle(self, col):
        sectionStartCol = 0
        for a in self.axisSections:
            if col < sectionStartCol:
                raise colNotMappedToCycle
            if col < sectionStartCol + a.endingCycle - a.startingCycle:
                return a.startingCycle + col - sectionStartCol
            sectionStartCol += a.endingCycle - a.startingCycle + self.AXIS_SECTION_SPACING
        
    
    def addInsn(self, insn):
        self.insns.append(insn)
        self.startingLine = min([insn.line for insn in self.insns])
        self.endingLine = max([insn.line for insn in self.insns])
    
    def buildAxisSections(self):
        self.insns.sort(key = lambda x: x.startState.number)
        
        axisSections = []
        for insn in self.insns:
            if len(axisSections) == 0 or (insn.startState.number - axisSections[-1].endingCycle) > self.MAX_GAP:
                axisSections.append(GanttChartAxisSection(insn))
            else:
                axisSections[-1].addInsn(insn)
                
        self.axisSections = axisSections
        
        for ax in self.axisSections:
            ax.roundStartEnd()
            
        # Combine axis sections that overalp
        i = 0
        while i < len(self.axisSections) - 1:
            if self.axisSections[i].endingCycle + GanttChartAxisSection.ROUND_TO >= self.axisSections[i+1].startingCycle:
                self.axisSections[i].endingCycle = self.axisSections[i+1].endingCycle
                del self.axisSections[i + 1]
            else:
                i += 1
    
    def buildEmptyLines(self):
        self.emptyLines = []
        for i in range(self.startingLine + 1, self.endingLine + 1):
            insnsThisLine = [insn for insn in self.insns if insn.line == i]
            if len(insnsThisLine) == 0:
                self.emptyLines.append(i)
    
    def drawAxisLabel(self, p, x, y, s):
        p.drawLine(x, y, x, y - 3)
        p.drawText(x - p.fontMetrics().boundingRect(s).width()/2, y - 3, s)
    
    def drawAxis(self, p, y):
        oldFont = p.font()
        newFont = QtGui.QFont("courier new", 8)
        p.setFont(newFont) 
        
        lastAS = None
        for axisSection in self.axisSections:
            xStart = GanttChart.colToX(self.cycleToCol(axisSection.startingCycle))
            xEnd = GanttChart.colToX(self.cycleToCol(axisSection.endingCycle))
            p.drawLine(xStart, y, xEnd, y)
#             self.drawAxisLabel(p, xStart, y, str(axisSection.startingCycle))
            for i in range(axisSection.startingCycle, axisSection.endingCycle + 1):
                if i % 4 == 0:
                    self.drawAxisLabel(p, GanttChart.colToX(self.cycleToCol(i)), y, str(i))
#             self.drawAxisLabel(p, xEnd, y, str(axisSection.endingCycle))

            if lastAS:
                xStart = GanttChart.colToX(self.cycleToCol(lastAS.endingCycle))
                xEnd = GanttChart.colToX(self.cycleToCol(axisSection.startingCycle))
                p.setPen(Qt.DotLine)
                p.drawLine(xStart, y, xEnd, y)
                p.setPen(Qt.SolidLine) 
            lastAS = axisSection
        p.setFont(oldFont)
        
class GanttChart(QtWidgets.QWidget):
    BOX_WIDTH = 7
    BOX_START_PADDING = 5
    BOX_VERTICAL_PADDING = 4
    
    @classmethod
    def colToX(cls, column):
        return column * cls.BOX_WIDTH + cls.BOX_START_PADDING
    
    @classmethod
    def  xToCol(cls, x):
        return (x - cls.BOX_START_PADDING) / cls.BOX_WIDTH
    
#     def h(self, cycle):
    
    def __init__(self, gantt):
        super().__init__()
        
        self.gantt = gantt
        self.centerOnRedLine = True        
        
        grid = QtWidgets.QGridLayout()
        self.setLayout(grid)

        self.selected_line_cycle = (0, 0)
        
        self.calc_width_for_insn_mode()
        
        self.buildChartSections()
        
    # Return the section that lineNum belongs to.
    # Returns 'None' if the line does not belong to a section
    def sectionsByLineNum(self, lineNum):
        ret = []
        for s in self.sections:
            if lineNum >= s.startingLine and lineNum <= s.endingLine:
                ret.append(s)
            
        return ret
    
    def buildChartSections(self):
        sourceFile = self.gantt.sourceFile
        gui = sourceFile.gui
        proj = gui.manager.design
        file = sourceFile.file
        code = sourceFile.sourceCodeViewer.textBox
        numLines = code.blockCount()
        
        # Insns for this source file
        insns = proj.insnsBySourceFileLine[file]
        
        sections = []
        sectionsBeingBuilt = {}
        
        # Loop through # of lines (lines start at 1)
        for lineNum in range(1, numLines + 1):
            
            if lineNum in insns:
                insnsThisLine = insns[lineNum]
            else:
                # No instructions this line
                continue
            
            # Key a list of functions for this line
            fcns = set([insn.function for insn in insnsThisLine])
            
            if fcns != set(sectionsBeingBuilt.keys()):
                sectionsBeingBuilt = {}
                
                # Create new sections
                for fcn in fcns:
                    newSection = GanttChartSection(file, fcn, Qt.gray)
                    sections.append(newSection)
                    sectionsBeingBuilt[fcn] = newSection
                    
            # Add instructions to sections:
            for insn in insnsThisLine:              
                sectionsBeingBuilt[insn.function].addInsn(insn)
        
        self.sections = sections
                
        # Add Title line if there is room
        # This is done in two passes otherwise overlapping sections fight for the same free line
        sectionsToAddLine = []
        for s in self.sections:
            if s.startingLine != 0 and len(self.sectionsByLineNum(s.startingLine - 1)) == 0:
                sectionsToAddLine.append(s)
        for s in sectionsToAddLine:
            s.startingLine -= 1
                
        # If possible, add an extra line as backup for the axis
        sectionsToAddLine = []
        for s in self.sections:
            if s.endingLine < numLines and len(self.sectionsByLineNum(s.endingLine + 1)) == 0:
                sectionsToAddLine.append(s)
        for s in sectionsToAddLine:
            s.endingLine += 1
            
        
        for s in self.sections:
            s.buildEmptyLines()
            s.buildAxisSections()
            
    def executionModeChanged(self):
        self.calc_width_for_insn_mode()
        self.update()
    
    def minimumSizeHint(self, *args, **kwargs):
        return QtCore.QSize(self.minWidth, 0)        
    
    def calc_width_for_insn_mode(self):
        manager = self.gantt.sourceFile.gui.manager
        
        design = manager.design
        assert design
        
        file = self.gantt.sourceFile.file
        
        max_cycle = max([insn.getGanttEndStateNum() for insns in design.insnsBySourceFileLine[file].values() for insn in insns])
        
        self.minWidth = self.BOX_START_PADDING + self.BOX_WIDTH * (max_cycle + 1)            

    def getSectionsForDrawing(self, lineNum):
        manager = self.gantt.sourceFile.gui.manager
        instanceNum = manager.currentState[0]
        
        if instanceNum:
            activeFcn = manager.design.instanceNumToInstance[instanceNum].function
        else:
            activeFcn = None
        
        sections = self.sectionsByLineNum(lineNum)
        
        # If this line of code is inlined in multiple functions, 
        # we only show the gantt chart when the function is being executed,
        # since it's too confusing to show two gantt charts simultaneously.                       
        sectionToDraw = None
        if len(sections) == 1:
            sectionToDraw = sections[0] 
        else:
            matches = [s for s in sections if s.fcn == activeFcn]
            assert len(matches) <= 1
            if len(matches) == 1:                    
                sectionToDraw = matches[0]
                
        return (sections, sectionToDraw)

    def paintEvent(self, event):   
#         print("Paint")     
        manager = self.gantt.sourceFile.gui.manager
        
        design = manager.design
        assert design
        
        code = self.gantt.sourceFile.sourceCodeViewer.textBox
        file = self.gantt.sourceFile.file
        
        instanceNum, stateNum = manager.currentState
        
        if instanceNum:
            activeFcn = manager.design.instanceNumToInstance[instanceNum].function
            cycle = stateNum
        else:
            activeFcn = None
            cycle = None
        
        p = QtGui.QPainter(self)
        p.fillRect(self.rect(), Qt.lightGray)
        
        proj = self.gantt.sourceFile.gui.manager.design
        if not proj:
            return
                    
        insns_per_line = proj.insnsBySourceFileLine[file] 
#         if manager.instruction_mode == project.INSTRUCTION_MODE_HW or manager.instruction_mode == project.INSTRUCTION_MODE_ASSEMBLY:
#             insns_per_line = proj.insnsBySourceFileLine[file] 
#         elif manager.instruction_mode == project.INSTRUCTION_MODE_C:
#             insns_per_line = proj.C_insns_by_source[file]
        
        b = code.firstVisibleBlock()
        top = code.blockBoundingRect(b).translated(code.contentOffset()).top()
        bottom = top + code.blockBoundingRect(b).height()
        lineNum = b.blockNumber() + 1           
        min_line = None
        max_line = None
        
        selected_insns = []
        getActiveInsns = []
        sectionsDrawn = []
        
        lastVisibleLine = lineNum
        p.setPen(QtCore.Qt.black)
        
        for s in self.sections:
            s.axisDrawn = False
        
        self.redLineX = None
        
        while b.isValid():
            bounding = code.blockBoundingRect(b)

            (sections, sectionToDraw) = self.getSectionsForDrawing(lineNum)
            
            # Make sure if there are IR instructions on this line that we are in a section
            if lineNum in insns_per_line:
                assert len(sections)
            
            if sectionToDraw:
                sectionsDrawn.append(sectionToDraw)

            if top < self.height():
                lastVisibleLine = lineNum
            
#             if lineNum == 202:
#                 print("here")
            
            if bottom >= event.rect().top() and top <= event.rect().bottom():
                # Draw section background and Gantt Title
        
                if len(sections) and sectionToDraw is None:
                    # Multiple overlapping sections, so display the conflict
                    p.fillRect(0, top, self.width(), bounding.height(), sections[0].color)
                    p.setPen(Qt.black)
                    sectionName = "*** Inlined in multiple functions (" + \
                                ", ".join([s.fcn.name for s in sections]) + ")***"
                    p.drawText(5, top + p.fontMetrics().height()-2, sectionName)
                
                else:        
                    # Draw background
                    if sectionToDraw:
                        p.fillRect(0, top, self.width(), bounding.height(), sectionToDraw.color)
                    else:
                        p.fillRect(0, top, self.width(), bounding.height(), Qt.lightGray)
                    
                    if sectionToDraw:
                        if lineNum == sectionToDraw.startingLine:
                            # Add a title to the Gantt chart section.                         
                            p.setPen(Qt.black)                        
                            p.drawText(5, top + p.fontMetrics().height()-2, sectionToDraw.fcn.name)
                        else:  
                            # Draw the vertical dividing lines for the section  
                            for s in sectionToDraw.axisSections:
                                for c in range(s.startingCycle, s.endingCycle+1, 4):
                                    x = self.colToX(sectionToDraw.cycleToCol(c))
                                    p.setPen(Qt.DotLine)
                                    p.drawLine(x, top, x, bottom)
                                    p.setPen(Qt.SolidLine)
                    
                    insns = insns_per_line.get(lineNum, [])
                
                    # If we have instructions, make sure we are in a section
                    assert len(insns) == 0 or sectionToDraw
                    
                    insns = [i for i in insns if i.function == sectionToDraw.fcn]
                
                    for insn in insns:
                        insn_cycle_start = insn.startState.number
                        insn_cycles = insn.getGanttEndStateNum() - insn_cycle_start
                        self.lineEdit_function = QtWidgets.QLineEdit()
                        
                        #Selected instructions are blue                   
                        if  self.selected_line_cycle[0] == lineNum and insn_cycle_start <= self.selected_line_cycle[1] < (insn_cycle_start + insn_cycles): 
                            p.setBrush(QtCore.Qt.blue)
                            selected_insns.append(insn)                           
                
                        # Active function          
                        elif instanceNum in [i.number for i in insn.function.instances]:                             
                            if insn in manager.getActiveInsns(): 
                                p.setBrush(QtCore.Qt.green)
                                getActiveInsns.append(insn)
                            else:
                                p.setBrush(QtCore.Qt.yellow)

                            if not min_line:
                                min_line = top - 5
                                max_line = bottom + 5
                            else:
                                min_line = min(top - 5, min_line)
                                max_line = max(bottom + 5, max_line)
                                
                        # Inactive functions
                        else:
                            p.setBrush(QtCore.Qt.yellow)

                        # Draw the box
                        col = sectionToDraw.cycleToCol(insn_cycle_start)
                        if col is None:
                            print(sectionToDraw.fcn.name)
                            print(insn_cycle_start)
                        assert col is not None
                        x = self.colToX(col)
                        w = insn_cycles * self.BOX_WIDTH
                        y = top + self.BOX_VERTICAL_PADDING / 2
                        h = bounding.height() - self.BOX_VERTICAL_PADDING
                            
                        p.drawRect(x, y, w, h)
                        
                if selected_insns:
                    self.gantt.sourceFile.setSelectedInsns(selected_insns)
                elif getActiveInsns:
                    self.gantt.sourceFile.setSelectedInsns(getActiveInsns)
                else:
                    self.gantt.sourceFile.setSelectedInsns([])
                
                if sectionToDraw and lineNum == sectionToDraw.axisLineNum:
                    sectionToDraw.drawAxis(p, bottom - 1)
            
                if cycle is not None and sectionToDraw and activeFcn == sectionToDraw.fcn and sectionToDraw:
                    p.setPen(Qt.red)
#                     print(activeFcn.name)
#                     print("Active cycle: " + str(cycle))
                    col = sectionToDraw.cycleToCol(cycle)
                    if col is not None:
                        x = self.colToX(col)            
                        p.drawRect(x, top, 1, bottom-top)
                        if self.redLineX is not None:
                            assert(self.redLineX == x)
                        self.redLineX = x
                
            top = bottom
            bottom = top + bounding.height()            
            b = b.next()
            lineNum += 1


        # Move axis and redraw, if necessary
        for section in sectionsDrawn:
            oldLineNum = section.axisLineNum
            
            # Axis location should be at the end of the section, but not 
            # beyond the last visible line. (Subtract 1 to ensure its on the
            # last 'entirely' visible line (not partial).  
            section.axisLineNum = min(section.endingLine, lastVisibleLine - 1)
            if section.axisLineNum != oldLineNum:
                self.update()
                
                
        redLineX = self.redLineX
        
        if redLineX is not None:            
            vp = self.gantt.ganttScrollArea.viewport()
            newScrollValue = redLineX - vp.width() / 2
            sb =  self.gantt.ganttScrollArea.horizontalScrollBar()
            if sb.value() != newScrollValue and self.centerOnRedLine:
                sb.setValue(newScrollValue)
                
                # This ensures we only center the gantt chartt once per stepping
                # Prevents it being moved while user is trying to manually scroll it.
                self.centerOnRedLine = False
            
    def mousePressEvent(self, event):
        lineNum = self.gantt.sourceFile.sourceCodeViewer.textBox.lineFromY(event.y())
        x = event.x()
        
        sectionToDraw = self.getSectionsForDrawing(lineNum)[1]
        
        if sectionToDraw:
            try:
                cycle = sectionToDraw.colToCycle(self.xToCol(x)) 
            except colNotMappedToCycle:
                self.selected_line_cycle = (None, None)
            else:
                self.selected_line_cycle = (lineNum, cycle)
        else:
            self.selected_line_cycle = (None, None)
        self.update()    
