from PyQt5 import QtWidgets
from PyQt5.QtCore import Qt

class Stats(QtWidgets.QWidget):
    def __init__(self, gui):
        super().__init__()
        
        self.gui = gui
        
        self.gui.manager.designOpened.connect(self.designOpened)
        self.gui.manager.designClosed.connect(self.designClosed)
        
        grid = QtWidgets.QGridLayout()
        
        self.setLayout(grid)
        r = 0
        align1 = Qt.AlignLeft
        align2 = Qt.AlignRight
        align3 = Qt.AlignLeft
        
        w = QtWidgets.QLabel("<b>Variables</b>")
        grid.addWidget(w, r, 0)
        r += 1
        
        
        w = QtWidgets.QLabel("Size in RAM:")
        w.setAlignment(align1)
        grid.addWidget(w, r, 0)
                
        self.varInRam = QtWidgets.QLabel("")
        self.varInRam.setAlignment(align2)
        grid.addWidget(self.varInRam, r, 1)
        
        w = QtWidgets.QLabel("bits")
        w.setAlignment(align3)
        grid.addWidget(w, r, 2)
        r += 1
        
        w = QtWidgets.QLabel("Size in Regs:")
        grid.addWidget(w, r, 0)
                
        self.varInRegs = QtWidgets.QLabel("0")
        self.varInRegs.setAlignment(Qt.AlignRight)
        grid.addWidget(self.varInRegs, r, 1)
        
        w = QtWidgets.QLabel("bits")
        grid.addWidget(w, r, 2)
        r += 1
        
        
        r += 1
        w = QtWidgets.QLabel("<b>Trace Buffers</b>")
        grid.addWidget(w, r, 0)
        r += 1
        
        
        w = QtWidgets.QLabel("Total Size:")
        w.setAlignment(align1)
        grid.addWidget(w, r, 0)
                
        self.tbSize = QtWidgets.QLabel("0")
        self.tbSize.setAlignment(align2)
        grid.addWidget(self.tbSize, r, 1)
        
        w = QtWidgets.QLabel("bits")
        w.setAlignment(align3)
        grid.addWidget(w, r, 2)
        r += 2
        
        
        w = QtWidgets.QLabel("Ctrl Buf Width:")
        w.setAlignment(align1)
        grid.addWidget(w, r, 0)
                
        self.ctrlBufWidth = QtWidgets.QLabel("0")
        self.ctrlBufWidth.setAlignment(align2)
        grid.addWidget(self.ctrlBufWidth, r, 1)
        
        w = QtWidgets.QLabel("bits")
        w.setAlignment(align3)
        grid.addWidget(w, r, 2)
        r += 1
                
        w = QtWidgets.QLabel("Mem Buf Width:")
        w.setAlignment(align1)
        grid.addWidget(w, r, 0)
                
        self.memBufWidth = QtWidgets.QLabel("0")
        self.memBufWidth.setAlignment(align2)
        grid.addWidget(self.memBufWidth, r, 1)
        
        w = QtWidgets.QLabel("bits")
        w.setAlignment(align3)
        grid.addWidget(w, r, 2)
        r += 1
        
        
        w = QtWidgets.QLabel("Regs Buf Width:")
        w.setAlignment(align1)
        grid.addWidget(w, r, 0)
                
        self.regsBufWidth = QtWidgets.QLabel("0")
        self.regsBufWidth.setAlignment(align2)
        grid.addWidget(self.regsBufWidth, r, 1)
        
        w = QtWidgets.QLabel("bits")
        w.setAlignment(align3)
        grid.addWidget(w, r, 2)
        r += 2
        
        w = QtWidgets.QLabel("Ctrl Buf Depth:")
        w.setAlignment(align1)
        grid.addWidget(w, r, 0)
                
        self.ctrlBufDepth = QtWidgets.QLabel("0")
        self.ctrlBufDepth.setAlignment(align2)
        grid.addWidget(self.ctrlBufDepth, r, 1)
        
        w = QtWidgets.QLabel("entries")
        w.setAlignment(align3)
        grid.addWidget(w, r, 2)
        r += 1
        
        
        w = QtWidgets.QLabel("Mem Buf Depth:")
        w.setAlignment(align1)
        grid.addWidget(w, r, 0)
                
        self.memBufDepth = QtWidgets.QLabel("0")
        self.memBufDepth.setAlignment(align2)
        grid.addWidget(self.memBufDepth, r, 1)
        
        w = QtWidgets.QLabel("entries")
        w.setAlignment(align3)
        grid.addWidget(w, r, 2)
        r += 1
        
        
        w = QtWidgets.QLabel("Regs Buf Depth:")
        w.setAlignment(align1)
        grid.addWidget(w, r, 0)
                
        self.regsBufDepth = QtWidgets.QLabel("0")
        self.regsBufDepth.setAlignment(align2)
        grid.addWidget(self.regsBufDepth, r, 1)
        
        w = QtWidgets.QLabel("entries")
        w.setAlignment(align3)
        grid.addWidget(w, r, 2)
        r += 1
        
        grid.setColumnStretch(0, 1.0)
        grid.setColumnStretch(1, 1.0)
        grid.setColumnStretch(2, 1.0)
        
        for i in range(r):
            grid.setRowStretch(i, 1)
        grid.setRowStretch(r, 50)
        
    def designClosed(self):
        self.varInRam.setText("")
        self.varInRegs.setText("")
        
        self.tbSize.setText("")
        self.ctrlBufWidth.setText("")
        self.memBufWidth.setText("")
        self.regsBufWidth.setText("")
        
        self.ctrlBufDepth.setText("")
        self.memBufDepth.setText("")
        self.regsBufDepth.setText("")
        
    def designOpened(self):
        if not self.gui.manager.design:
            return
        
        design = self.gui.manager.design
        
        ramBits = sum([(r.dataWidth * r.numElements) for r in design.rams])
        self.varInRam.setText('{:,}'.format(ramBits))
        
        regBits = sum([s.width for f in design.functions for s in f.signals if len(s.traces)])
        self.varInRegs.setText('{:,}'.format(regBits))
        
        self.tbSize.setText('{:,}'.format(design.configBufferCtrlWidth * design.configBufferCtrlDepth + design.configBufferMemWidth * design.configBufferMemDepth + design.configBufferRegsWidth * design.configBufferRegsDepth))
        self.ctrlBufWidth.setText('{:,}'.format(design.configBufferCtrlWidth))
        self.memBufWidth.setText('{:,}'.format(design.configBufferMemWidth))
        self.regsBufWidth.setText('{:,}'.format(design.configBufferRegsWidth))
        
        self.ctrlBufDepth.setText('{:,}'.format(design.configBufferCtrlDepth))
        self.memBufDepth.setText('{:,}'.format(design.configBufferMemDepth))
        self.regsBufDepth.setText('{:,}'.format(design.configBufferRegsDepth))
