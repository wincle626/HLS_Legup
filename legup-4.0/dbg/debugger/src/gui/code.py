from PyQt5 import QtWidgets
from PyQt5 import QtGui
from PyQt5 import QtCore

from . import gantt
from . import linenums


# This class is for a single source file (one tab)
# Parent:
# Children: SourceCodeViewerText, Gantt
class SourceFile(QtWidgets.QWidget):
    def __init__(self, file, gui):
        super().__init__()
        
        self.file = file
        self.gui = gui
        
        self.splitter_top = QtWidgets.QSplitter(self)
        
        grid = QtWidgets.QGridLayout()
        self.setLayout(grid)

        # Source Code
        self.sourceCodeViewer = SourceCodeViewer(self, file)          
        
        # Gantt Chart
        self.gantt = gantt.Gantt(self)
        
        # Splitter
        self.splitter_bottom = QtWidgets.QSplitter()
        self.splitter_bottom.setOrientation(QtCore.Qt.Vertical)
        
        # Assembly 
        self.assemblyBox = AssemblyBox(self)
        
        self.splitter_bottom.addWidget(self.splitter_top)
        self.splitter_bottom.addWidget(self.assemblyBox)
        
        self.splitter_top.addWidget(self.sourceCodeViewer)
        self.splitter_top.addWidget(self.gantt)
        
        self.splitter_bottom.setSizes([1,1])        
        self.splitter_bottom.setStretchFactor(0, 1)
        self.splitter_bottom.setStretchFactor(1, 0)
        
        self.splitter_top.setSizes([300,300])
        
        grid.addWidget(self.splitter_bottom)
        
        grid.setSpacing(0)
        grid.setContentsMargins(0, 0, 0, 0)
    
    def removeInstructionSelections(self):
        self.gantt.ganttScrollArea.ganttChart.selected_line_cycle = (0, 0)
        
    def setSelectedInsns(self, insns):
        text = ""
        
        assembly_insns = []

        for insn in insns:
            assembly_insns.append(insn)
#         
        for insn in assembly_insns:
            text += insn.desc + "\n"
        
        self.assemblyBox.setPlainText(text)

    def instructionModeChanged(self):
        self.gantt.ganttScrollArea.ganttChart.executionModeChanged()
        
    def refresh(self):
        self.gantt.ganttScrollArea.refresh()
        
    def removeHighlighting(self):
        self.sourceCodeViewer.textBox.removeHighlighting()        
    
    def highlightLineNumbers(self, lineNums):
        for lineNum in lineNums:
            self.sourceCodeViewer.textBox.highlightLineNumber(lineNum)
        
        #Navigate to highlighted line
        (first, last) = self.sourceCodeViewer.textBox.getVisibleLines()
        if len(lineNums) and not any(first <= lineNum <= last for lineNum in lineNums):
            lineNum = lineNums[0]
            self.sourceCodeViewer.textBox.goToLine(lineNum)
        
        self.gantt.ganttScrollArea.ganttChart.centerOnRedLine = True


# Parent: SourceCodeTab
class SourceCodeViewer(QtWidgets.QWidget):
    def __init__(self, sourceCodeTab, file):
        super().__init__()
        self.sourceCodeTab = sourceCodeTab
        
        self.grid = QtWidgets.QGridLayout()
        self.grid.setContentsMargins(0, 0, 0, 0)
        self.grid.setSpacing(0)
        
        self.setLayout(self.grid)
        
        self.textBox = SourceCodeViewerText(sourceCodeTab, file)
        self.lineNumberArea = linenums.LineNumberArea(self, self.sourceCodeTab, True)        
        
        self.textBox.updateRequest.connect(self.lineNumberArea.update_area)
        
        self.grid.addWidget(self.lineNumberArea, 0, 0)
        self.grid.addWidget(self.textBox, 0, 1)

# Parent: SourceCodeTab
# Children: LineNumberArea, 
class SourceCodeViewerText(QtWidgets.QPlainTextEdit):    
    def __init__(self, sourceCodeTab, file):
        super().__init__()    
        self.sourceCodeTab = sourceCodeTab
        
        text = open(file).read()              
        self.setPlainText(text)        
        
        self.setWordWrapMode(QtGui.QTextOption.NoWrap)
        self.setReadOnly(True)
        self.setTabStopWidth(20)
        
        self.updateRequest.connect(self.qt_bug_fix)
    
    
    # A QT bug scrolls the QPlainTextEdit to the cursor on lost focus
    # To prevent this problem, when the user scrolls we automatically move the cursor
    # (This is fine since the file is read-only and the cursor is hidden)
    # Fix for Qt bug https://bugreports.qt-design.org/browse/QTBUG-40755?page=com.atlassian.jira.plugin.system.issuetabpanels:all-tabpanel
    def qt_bug_fix(self, rect, dy):
        cursor = self.textCursor()
        old_pos = cursor.position()
        
        pos = self.firstVisibleBlock().position()
        if pos != old_pos:
            cursor.setPosition(self.firstVisibleBlock().position())
            self.setTextCursor(cursor)
    
    def file(self):
        return self.sourceCodeTab.file
                    
    def removeHighlighting(self):        
        self.setExtraSelections([])
    
    # Returns the pair (first, last) of visible line numbers
    def getVisibleLines(self):
        block = self.firstVisibleBlock()
        first = block.blockNumber() + 1     
        
        top = self.blockBoundingGeometry(block).translated(self.contentOffset()).top()
         
        while block.isValid() and top < self.rect().bottom():
            top += self.blockBoundingRect(block).height()
            block = block.next()

        block = block.previous()    
        last = block.blockNumber() + 1
        
        return (first, last)     
    
    def goToLine(self, lineNum):
        block = self.document().findBlockByNumber(lineNum - 1)
        cursor = QtGui.QTextCursor(block)
        self.setTextCursor(cursor)        
    
    def highlightLineNumber(self, line_num):
        selections_list = self.extraSelections()
        
        new_selection = QtWidgets.QTextEdit.ExtraSelection()
        new_selection.format.setBackground(QtCore.Qt.green)
        new_selection.format.setProperty(QtGui.QTextFormat.FullWidthSelection, True)
        new_selection.cursor = QtGui.QTextCursor(self.document().findBlockByLineNumber(line_num-1))
        
        selections_list.append(new_selection)
        
        self.setExtraSelections(selections_list)
        
    def lineFromY(self, y):
        block = self.firstVisibleBlock()
        top = self.blockBoundingGeometry(block).translated(self.contentOffset()).top()
        bottom = top + self.blockBoundingRect(block).height()
        
        lineNum = block.blockNumber()+1
        while block.isValid():
            if y >= top and y < bottom:
                return lineNum
            
            block = block.next()
            top = bottom
            bottom = top + self.blockBoundingRect(block).height()
            lineNum += 1
            
        return None
    
    def lineNumberInfo(self, event):
        block = self.firstVisibleBlock()
        lineNum = block.blockNumber() + 1
        top = self.blockBoundingGeometry(block).translated(self.contentOffset()).top()        
        bottom = top + self.blockBoundingRect(block).height()
        
        start = None
        end = None
        
        while(block.isValid() and top <= event.rect().bottom()):
            if (block.isVisible() and bottom >= event.rect().top()):
                if start is None:
                    start = lineNum
                    ret_top = top
                end = lineNum + 1
                    
            block = block.next()
            top = bottom
            bottom = top + self.blockBoundingRect(block).height()
            lineNum += 1
        
        return (ret_top, self.fontMetrics().height(), start, end)
    
class AssemblyBox(QtWidgets.QPlainTextEdit):
    def __init__(self, gui):
        super().__init__()
        self.gui = gui
        
        self.setReadOnly(True)
        
    def set_text(self, text):
        self.setPlainText(text)
    