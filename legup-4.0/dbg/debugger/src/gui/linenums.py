from PyQt5 import QtWidgets
from PyQt5 import QtGui
from PyQt5 import QtCore

class LineNumberArea(QtWidgets.QWidget):
    def __init__(self, parent, sourceFile, supportBreakpoints):
        
        super().__init__(parent)
        self.sourceFile = sourceFile
        self.supportBreakpoints = supportBreakpoints        

    def update_area(self, rect, dy):
        if dy:
            self.scroll(0, dy)
        
    def minWidth(self):
        textBox = self.sourceFile.sourceCodeViewer.textBox

        digits = len(str(textBox.blockCount()))                
        space = textBox.fontMetrics().widthChar("9") * (digits + 3)
        
        return space
    
    def minimumSizeHint(self):
        return QtCore.QSize(self.minWidth(), 0)
        
    def paintEvent(self, e):
#         print("Line Number Area Paint Event")        
        (top, height, start, end) = self.sourceFile.sourceCodeViewer.textBox.lineNumberInfo(e)
        
#         print(str(top) + " " + str(height) + " " + str(start) + " " + str(end))

        painter = QtGui.QPainter(self)        
        painter.fillRect(e.rect(), QtCore.Qt.lightGray)

        painter.setPen(QtCore.Qt.black)
        breakpointLocations = self.sourceFile.gui.manager.getBreakpointLocations()

        for i in range(start, end):
            painter.drawText(0, top, self.width(), self.fontMetrics().height(), QtCore.Qt.AlignRight, str(i))

            if self.supportBreakpoints:
                if  (self.sourceFile.file, i) in breakpointLocations:
                    painter.setBrush(QtCore.Qt.red)
                    painter.drawEllipse(1, top, self.fontMetrics().height(),self.fontMetrics().height())
                
            top += height

    def mouseDoubleClickEvent(self, event):
        if not self.supportBreakpoints:
            return
        
        gui = self.sourceFile.gui
        
        lineNum = self.sourceFile.sourceCodeViewer.textBox.lineFromY(event.y())
        
        if lineNum:
            breakpointLocations = self.sourceFile.gui.manager.getBreakpointLocations()
            newBreakpointLoc = (self.sourceFile.sourceCodeViewer.textBox.file(), lineNum)
            
            if newBreakpointLoc in breakpointLocations:
                self.sourceFile.gui.manager.breakpointRemove(*newBreakpointLoc)
            else:
                gui.manager.breakpointAdd(*newBreakpointLoc)
            self.update() 
