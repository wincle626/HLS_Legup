import sys
import subprocess
import os
import collections
import threading



from PyQt5 import QtWidgets
from PyQt5 import QtCore
from PyQt5 import QtGui
from PyQt5.QtCore import Qt

import manager
from backends import backend
import paths 
import discrepancy

from . import variables
from . import code
from . import fpga
from . import stats

def run():
    app = QtWidgets.QApplication(sys.argv)
#     app.setStyle(QtWidgets.QStyleFactory.create("fusion"))
#     app.setStyle(QtWidgets.QStyleFactory.create("windows"))
    MainGui(app)
    sys.exit(app.exec_()) 
    


# Parent:
# Children: QTreeView
class TraceOptions(QtWidgets.QWidget):
    def __init__(self, gui):
        super().__init__()        
        
        self.gui = gui        
        
        self.gui.manager.designOpened.connect(self.populate)       
        
        grid = QtWidgets.QGridLayout()
        self.setLayout(grid)
        self.checkboxvalues=[]
        
        # Temporarily disable updates - this is done when we are dynamically
        # changing entries (such as enabling/disabling check boxes)
        # and don't want it to trigger a backendFPGA event.
        self.disable_updates = False
        
        model = QtGui.QStandardItemModel()
        self.treeview = QtWidgets.QTreeView()
        self.treeview.setModel(model)
        
        grid.addWidget(self.treeview, 0 ,0)
        
        model.setColumnCount(2)
        model.setHeaderData(0, QtCore.Qt.Horizontal, "Trace?", QtCore.Qt.DisplayRole)
        model.setHeaderData(1, QtCore.Qt.Horizontal, "", QtCore.Qt.DisplayRole)
        model.itemChanged.connect(self.itemChanged)
        
        self.item_globals = QtGui.QStandardItem("Global Variables")
        self.item_globals.setEditable(False)
        model.appendRow(self.item_globals)
        self.treeview.setFirstColumnSpanned(0, model.invisibleRootItem().index(), True)
        
        self.item_functions = QtGui.QStandardItem("Functions")
        self.item_functions.setEditable(False)
        model.appendRow(self.item_functions)
        self.treeview.setFirstColumnSpanned(1, model.invisibleRootItem().index(), True)
        

    def populate(self):
        if not self.gui.manager.design:
            return
        
        # Populate global variables
        for g in self.gui.manager.design.globalVariables:            
            item_var_name = QtGui.QStandardItem(g.name)
            item_var_chkbox = QtGui.QStandardItem()
            item_var_chkbox.setCheckable(True)
            item_var_chkbox.setCheckState(QtCore.Qt.Checked)
            item_var_chkbox.var = g
            self.item_globals.appendRow([item_var_chkbox, item_var_name])
        
        # Populate functions
        for fcn in self.gui.manager.design.functions:
            item_fcn_name = QtGui.QStandardItem(fcn.name)
            item_fcn_chkbox = QtGui.QStandardItem()
            item_fcn_chkbox.setCheckable(True)
            item_fcn_chkbox.setCheckState(QtCore.Qt.Checked)
            item_fcn_chkbox.fcn = fcn
            self.item_functions.appendRow([item_fcn_chkbox, item_fcn_name])
            
            # Populate local variables
            for v in fcn.variables:
                item_var_name = QtGui.QStandardItem(v.name)
                item_var_chkbox = QtGui.QStandardItem()
                item_var_chkbox.setCheckable(True)
                item_var_chkbox.setCheckState(QtCore.Qt.Checked)
                item_var_chkbox.var = v
                item_fcn_chkbox.appendRow([item_var_chkbox, item_var_name])

    def itemChanged(self, item):
        if self.disable_updates:
            return
        
        comm = self.gui.manager.comm                                            
        
        if item.parent() == self.item_globals:
            # Must be a global variable
            g_var = item.var
            if item.checkState()==QtCore.Qt.Checked:
                g_var.isTraced = True
            else:
                g_var.isTraced = False
            if comm.connected():
                comm.traceVarEnable(g_var.tag, g_var.isTraced)                
                
        elif item.parent() == self.item_functions:
            # Must be a function
            fcn = item.fcn
            if item.checkState() == QtCore.Qt.Checked:
                fcn.isTraced = True
            else:
                fcn.isTraced = False

            # Cascade to children, and enable/disable children                
            for i in range(item.rowCount()):
                child = item.child(i, 0)
                if fcn.isTraced:
                    child.setCheckState(QtCore.Qt.Checked)
                else:
                    child.setCheckState(QtCore.Qt.Unchecked)
                self.disable_updates = True
                child.setEnabled(fcn.isTraced)
                self.disable_updates = False
                    
            if comm.connected():
                comm.traceFunctionEnable(fcn.id, fcn.isTraced)
                
        elif item.parent() and item.parent().parent() == self.item_functions:
            # Must be a local variable
            var = item.var
            if item.checkState()==QtCore.Qt.Checked:
                var.isTraced = True
            else:
                var.isTraced = False
            if comm.connected():
                comm.traceVarEnable(var.tag, var.isTraced)                

        else:
            assert(False)
        

        
#########################################################################################################
# class OptionsTab(QtWidgets.QWidget):
#     modes = [(design.INSTRUCTION_MODE_HW, "HW Cycles"),
#              (design.INSTRUCTION_MODE_ASSEMBLY, "Assembly Instructions"),
#              (design.INSTRUCTION_MODE_C, "C Instructions")]
#     
#     default_mode = design.INSTRUCTION_MODE_HW
#     
#     def __init__(self, gui):
#         super().__init__()
#         
#         self.gui = gui
# #         self.setTitle("Options")
#         
#         grid = QtWidgets.QGridLayout()
#         self.setLayout(grid)
#         
#         self.labelMode = QtWidgets.QLabel("Mode:")
#         self.comboMode = QtWidgets.QComboBox()
#         self.comboMode.addItems([m[1] for m in self.modes])
#     
#         r = 0
#         grid.addWidget(self.labelMode, r, 0)
#         grid.addWidget(self.comboMode, r, 1)
#         r += 1
#         
#         grid.setRowStretch(r, 1)
#         
#         # Need to set to -1, to ensure that when index is changed
#         # two lines later, it detects an actual change and fires
#         # the currentIndexChanged signal
#         self.comboMode.setCurrentIndex(-1)
#         
#         self.comboMode.currentIndexChanged[str].connect(self.instructionModeChanged)
#         self.comboMode.setCurrentIndex(self.default_mode)
#         
#     def instructionModeChanged(self, text):
#         new_mode = next(mode[0] for mode in self.modes if mode[1] == text)
#         self.gui.instructionModeChanged(new_mode)
#         
#     def text_for_mode(self, mode_idx):
#         return next(mode[1] for mode in self.modes if mode[0] == mode_idx)
    
class Tools(QtWidgets.QWidget):
    def __init__(self, gui):
        super().__init__()
        self.gui = gui
        
        grid = QtWidgets.QGridLayout()
        self.setLayout(grid)
        
        
        self.labelDiscrepancy = QtWidgets.QLabel("C/RTL Discrepancy Check")
        font = QtGui.QFont()
        font.setBold(True)
        font.setUnderline(True)
        self.labelDiscrepancy.setFont(font)
        
        self.buttonRunDiscrepancy = QtWidgets.QPushButton("Run")
        self.buttonRunDiscrepancy.clicked.connect(self.runDiscrepancyCheck)
        
        self.buttonViewDiscrepancyRpt = QtWidgets.QPushButton("View Report")
        self.buttonViewDiscrepancyRpt.clicked.connect(self.viewDiscrepancyReport)

        
        r = 0
        
        grid.addWidget(self.labelDiscrepancy, r, 0, 1, 2)
        r += 1
        
        grid.addWidget(self.buttonRunDiscrepancy, r, 0)
        grid.addWidget(self.buttonViewDiscrepancyRpt, r, 1)
        r += 1
        
        grid.setRowStretch(r, 1)
        
        self.gui.manager.designOpened.connect(self.onDesignLoaded)
        
    def onDesignLoaded(self):
        self.updateIfDiscrepancyReportIsViewable()
        
    def updateIfDiscrepancyReportIsViewable(self):
        if self.gui.manager.design and os.path.isfile(discrepancy.rptPath(self.gui.manager.design)):
            self.buttonViewDiscrepancyRpt.setEnabled(True)
        else:
            self.buttonViewDiscrepancyRpt.setEnabled(False)
    
    def runDiscrepancyCheck(self):
        design = self.gui.manager.design
        
        if not design:
            self.gui.errorMessage("No design loaded")
            return
            
        rptPath = discrepancy.rptPath(design)
        if os.path.isfile(rptPath):
            os.remove(rptPath)
            
        self.updateIfDiscrepancyReportIsViewable()
        
        def runInThread():
            discrepancy.run(design)
            self.updateIfDiscrepancyReportIsViewable()
            
        thread = threading.Thread(target=runInThread)
        thread.start()     
    
    def viewDiscrepancyReport(self):
        design = self.gui.manager.design 
        
        rptPath = discrepancy.rptPath(design)
        assert os.path.isfile(rptPath)
        subprocess.Popen(["gedit", rptPath])   
        

class ItemList(QtCore.QAbstractListModel):
    def __init__(self):
        super().__init__()
        self.items = []
        
    def rowCount(self, parent):
        return 3
    
#     def insertRows(self, pos, rows, parent):
#         self.beginInsertRows(parent, pos, pos + rows-1)
#         new_items = []
#         for _ in range(rows):
#             new_items.append(None)
#         self.items = self.items[:pos] + new_items + self.items[pos:]
#         self.endInsertRows()
#         return True
#         
#     def setData(self, index, value, role):
#         print ("setData")
#         print (role)
#         self.items[index] = value
#         self.dataChanged.emit()
    
    def data(self, index, role):
        if not index.isValid():
            return QtCore.QVariant()
        
        row = index.row()
        if row == 0:
            if role == Qt.DisplayRole:                
                return "FPGA Live"
            elif role == Qt.TextColorRole:
#                 print("Color")
                return Qt.red
            
        elif row == 1:
            if role == Qt.DisplayRole:                
                return "FPGA Replay"
            elif role == Qt.TextColorRole:
                return Qt.green
            elif role == Qt.DecorationRole:
                return QtGui.QColor(Qt.green)
            elif role == Qt.BackgroundRole:
                print("fore")
            else:
                print ("role: " + str(role))
        elif row == 2:
            if role == Qt.DisplayRole:
                return "Modelsim"
            elif role == Qt.BackgroundRole:
                return Qt.yellow
    
#     def flags(self, index):
#         return Qt.ItemIsEditable or Qt.ItemIsEditable

class ExecutionControlButton(QtWidgets.QToolButton):
    def __init__(self, text, fcn, enable_fcn, icon_path, shortcut = ""):
        super().__init__()
        
        action = QtWidgets.QAction(QtGui.QIcon(icon_path), text, self)
        if action.shortcut():
            action.setShortcut(shortcut)
            
        self.setDefaultAction(action)
        
        self.setIconSize(QtCore.QSize(30, 30))
        self.triggered.connect(fcn)
        
        self.enable_fcn = enable_fcn
        
    def reenable(self):
        self.setEnabled(self.enable_fcn())


class ExecutionControlSlider(QtWidgets.QWidget):
    def __init__(self, enable_fcn):
        super().__init__()
        
        self.enable_fcn = enable_fcn
        
        self.grid = QtWidgets.QGridLayout()
        self.setLayout(self.grid)

        self.slider = QtWidgets.QSlider()
        self.grid.addWidget(self.slider, 0, 0, 1, 3)
        
        self.grid.setColumnStretch(0, 1)
        self.grid.setColumnStretch(1, 1)
        self.grid.setColumnStretch(2, 1)
        
        self.grid.setRowStretch(0, 1)
        self.grid.setRowStretch(1, 1)
        
        self.grid.setContentsMargins(10, 0, 10, 0)
        self.grid.setSpacing(0)
        
        self.label_min = QtWidgets.QLabel("-")
        self.grid.addWidget(self.label_min, 1, 0, 1, 1, QtCore.Qt.AlignLeft)

        self.label_curr = QtWidgets.QLabel("-")
        self.grid.addWidget(self.label_curr, 1, 1, 1, 1, QtCore.Qt.AlignHCenter)
        
        self.label_max = QtWidgets.QLabel("-")
        self.grid.addWidget(self.label_max, 1, 2, 1, 1, QtCore.Qt.AlignRight)
        
#         self.setMinimumHeight(50)
#     def sizeHint(self, *args, **kwargs):
#         return QtCore.QSize(0, 30)
#     def minimumSizeHint(self, *args, **kwargs):
#         return QtCore.QSize(0, 30)
        
    def setMinimum(self, min_val):
        self.slider.setMinimum(min_val)
        self.label_min.setText(str(min_val))
        
    def setMaximum(self, max_val):
        self.slider.setMaximum(max_val)
        self.label_max.setText(str(max_val))
        
    def setValue(self, val):
        self.slider.setValue(val)
        self.label_curr.setText(str(val))

    def reenable(self):
        self.setEnabled(self.enable_fcn())


class ExecutionControls(QtWidgets.QFrame):   

    def __init__(self, gui):
        super().__init__()
        
        self.gui = gui
        
        self.executionModes = collections.OrderedDict()
        self.executionModes[backend.BACKEND_FPGA_LIVE] = ("FPGA Live", Qt.red) 
        self.executionModes[backend.BACKEND_FPGA_REPLAY] = ("FPGA Replay", Qt.green)
        self.executionModes[backend.BACKEND_MODELSIM] = ("Modelsim", Qt.blue)

        self.ignoreExecutionModeChange = False
        self.userChangingSlider = False
        self.ignoreSliderMove = False

        self.setFrameStyle(QtWidgets.QFrame.Box | QtWidgets.QFrame.Raised)
        self.setLineWidth(2)
        self.setMidLineWidth(0)
        self.setContentsMargins(0, 0, 0, 0)
        
        self.grid = QtWidgets.QGridLayout()
        self.setLayout(self.grid)
        
#         self.grid.setContentsMargins(0, 0, 0, 0)

        self.label = QtWidgets.QLabel("Execution Mode")
        self.label.setStyleSheet("font: bold 16pt")

        # Setup combo box        
        self.combo = QtWidgets.QComboBox()
        self.combo.setStyleSheet("font: 16pt; padding: 2px 2px 2px 2px")
        
        for mode in self.executionModes:
            modeData = self.executionModes[mode]
            self.combo.addItem(modeData[0], mode)
            self.combo.setItemData(self.combo.count()-1, QtGui.QBrush(modeData[1]), Qt.ForegroundRole)        
        
        self.grid.addWidget(self.label, 0, 0)
        self.grid.addWidget(self.combo, 0, 1)
        
        f = QtWidgets.QFrame()
        f.setLineWidth(1)
        f.setMidLineWidth(1)
        
        f_grid = QtWidgets.QGridLayout()
        f.setLayout(f_grid)
        
        f_grid.setContentsMargins(0, 0, 0, 0)

        self.executionControlWidgets = []
        
        b = ExecutionControlButton(
            "Open Debug File", 
            self.gui.openFile,
            lambda: True, 
            os.path.join(paths.getResDir(), "open.png"), 
            "Ctrl+O")
        self.executionControlWidgets.append(b)
        
        b = ExecutionControlButton(
            "Refresh", 
            self.gui.manager.updateState,
            lambda: True, 
            os.path.join(paths.getResDir(), "refresh.png"), 
            "F5")
        self.executionControlWidgets.append(b)
        
        b = ExecutionControlButton(
            "Reset", 
            self.gui.manager.reset,
            lambda: self.gui.manager.activeBackend.IMPLEMENTS_RESET, 
            os.path.join(paths.getResDir(), "reset.png"))
        self.executionControlWidgets.append(b)
        
        b = ExecutionControlButton(
            "Run", 
            self.gui.manager.run,
            lambda: self.gui.manager.activeBackend.IMPLEMENTS_RUN, 
            os.path.join(paths.getResDir(), "run.png"))
        self.executionControlWidgets.append(b)
        
        b = ExecutionControlButton(
            "Pause",
            self.gui.manager.pause,
            lambda: self.gui.manager.activeBackend.IMPLEMENTS_PAUSE,
            os.path.join(paths.getResDir(), "pause.png"))
        self.executionControlWidgets.append(b)
                
        b = ExecutionControlButton(
            "Step Back", 
            self.gui.manager.stepBack,
            lambda: self.gui.manager.activeBackend.IMPLEMENTS_BACKSTEP, 
            os.path.join(paths.getResDir(), "step_into_back.png"),
            "F6")
        self.executionControlWidgets.append(b)
        
        b = ExecutionControlButton(
            "Step", 
            self.gui.manager.step,
            lambda: True, 
            os.path.join(paths.getResDir(), "step_into.png"),
            "F7")
        self.executionControlWidgets.append(b)
        
        # Place buttons
        c = 0
        for b in self.executionControlWidgets:
            f_grid.addWidget(b, 0, c)
            c += 1
        
        # Add slider
        self.slider = ExecutionControlSlider(lambda: self.gui.manager.activeBackend.IMPLEMENTS_TRACE)
        self.executionControlWidgets.append(self.slider)
        
        self.slider.setMinimum(0)
        self.slider.slider.setOrientation(QtCore.Qt.Horizontal)
        self.slider.slider.valueChanged.connect(self.sliderChanged)
        self.slider.slider.sliderPressed.connect(self.sliderPressed)
        self.slider.slider.sliderReleased.connect(self.sliderReleased)
        f_grid.addWidget(self.slider, 0, c)
        
        f_grid.setColumnStretch(c, 1)
        self.grid.addWidget(f, 1, 0, 1, 3)

        # Add current function/state fields
        self.labelFunction = QtWidgets.QLabel("Active function: ")
        self.labelState = QtWidgets.QLabel("State: ")

        self.grid.addWidget(self.labelFunction, 2, 0)
        self.grid.addWidget(self.labelState, 2, 1)
        self.grid.setColumnStretch(2, 1)
            
            
            
#         self.grid.setRowStretch(2, 1)
#         self.grid.setColumnStretch(0, 1)
            
        # Color won't show up the first time unless you do this hack
        self.combo.setEditable(True)
        self.combo.setEditable(False)
        
        self.combo.setCurrentIndex(self.gui.manager.executionMode)
        self.executionModeChanged()
        
        self.combo.currentIndexChanged.connect(self.changeExecutionMode)
        self.gui.manager.executionModeChanged.connect(self.executionModeChanged)
        self.gui.manager.stateChanged.connect(self.stateChanged)
        self.gui.manager.traceMaxChanged.connect(self.sliderMaxChanged)
            
    def sliderMaxChanged(self, val):
        self.ignoreSliderMove = True
        self.slider.setMaximum(val)
        self.ignoreSliderMove = False
        
    def stateChanged(self):
        if self.slider.isEnabled():
            cycle = self.gui.manager.getTraceCycle()
            
            # We are moving the slider in response to the stateNum change, so
            # we don't want it to trigger a new stateNum change (infinite loop)
            self.ignoreSliderMove = True
            self.slider.setValue(cycle)
            self.ignoreSliderMove = False
            
        (instanceNum, stateNum) = self.gui.manager.currentState
        if instanceNum is None:
            self.labelFunction.setText("Function: (None)")
        else:
            fcn = self.gui.manager.design.instanceNumToInstance[instanceNum].function
            self.labelFunction.setText("Function: " + fcn.name)
            
        if stateNum is None:
            self.labelState.setText("State: (None)")
        else:
            self.labelState.setText("State: " + str(stateNum))
    
    def sliderChanged(self, val):
        if not self.ignoreSliderMove:
            self.gui.manager.setTraceCycle(val)        
        
    def sliderPressed(self):
        # Don't refresh variables while sliding
        self.gui.variables.disableAutoUpdate = True
        self.userChangingSlider = True
    
    def sliderReleased(self):
        self.gui.variables.disableAutoUpdate = False
        self.userChangingSlider = False
        self.gui.variables.autoRefresh()
        
    def changeExecutionMode(self, idx):
        if self.ignoreExecutionModeChange:
            return
        mode = self.combo.itemData(idx)
        
        self.gui.manager.executionMode = mode
        
        # Check if failed
        if self.gui.manager.executionMode != mode:
            
            # Find the appropriate combox index
            for i in range(self.combo.count()):
                if self.combo.itemData(i) == self.gui.manager.executionMode:
                    self.ignoreExecutionModeChange = True
                    self.combo.setCurrentIndex(i)
                    self.ignoreExecutionModeChange = False
                    break
        
    def executionModeChanged(self):
        mode = self.gui.manager.executionMode
        
        pal = QtGui.QPalette(self.combo.palette())
        pal.setColor(QtGui.QPalette.Text, self.executionModes[mode][1])
        self.combo.setPalette(pal)
        
        for b in self.executionControlWidgets:
            b.reenable()
                
        if (self.slider.isEnabled()):
            self.ignoreSliderMove = True
            self.slider.setMinimum(1)
            self.slider.setMaximum(max(1,self.gui.manager.numTraceCycles()))
            self.ignoreSliderMove = False
        else:
            self.ignoreSliderMove = True
            self.slider.setValue(0)
            self.slider.setMaximum(0)
            self.ignoreSliderMove = False

class MainGui(QtWidgets.QMainWindow):
    def __init__(self, app):
        super().__init__()
        
        # QApplication
        self.app = app
        
        self.dbgmp = None
        self.manager = None 
        self.lastInstanceNum = None
        
        # Create manager
        self.manager = manager.Manager(self)
        
        # Register functions with manager
        self.manager.errorOccurred.connect(self.errorMessage)
        self.manager.stateChanged.connect(self.refresh)     
        self.manager.minorMessage.connect(self.minorMessage)
        self.manager.majorMessage.connect(self.majorMessage)
        self.manager.designClosed.connect(self.designClosed)
        self.manager.designOpened.connect(self.designOpened)
        self.manager.enteringLongWork.connect(self.setBusyCursor)
        self.manager.leavingLongWork.connect(self.clrBusyCursor)
        
        self.tabEditor = None
        
        # Actions
        exitAction = QtWidgets.QAction("Exit", self)
        exitAction.setShortcut("Ctrl+Q")
        exitAction.setStatusTip("Exit Application")
        exitAction.triggered.connect(QtWidgets.qApp.quit)
        
        # Status Bar
        self.statusBar() #.showMessage("Status Bar Message")
#         
        # Menubar 
        menubar = self.menuBar()
        fileMenu = menubar.addMenu("&File")
        fileMenu.addAction(exitAction)
        
        # Content
        self.executionMode = ExecutionControls(self)
        
        self.tabEditor = TabManager(self)
        self.tabEditor.setTabsClosable(True)
        self.tabEditor.tabCloseRequested.connect(self.tabEditor.removeTab)
                
        # Tabs
        self.serialTab = fpga.FpgaTab(self)
#       self.optionsTab = OptionsTab(self)
        self.variables = variables.Variables(self)        
        self.traceOptionsTab = TraceOptions(self)
        self.toolsTab = Tools(self)
        self.statsTab = stats.Stats(self)
        
        self.right_tabs = QtWidgets.QTabWidget()
        self.right_tabs.addTab(self.serialTab, "FPGA")
#       self.right_tabs.addTab(self.optionsTab, "Opts")
        self.right_tabs.addTab(self.variables, "Vars")
#       self.right_tabs.addTab(self.traceOptionsTab, "Trace")
        self.right_tabs.addTab(self.toolsTab, "Tools")
        self.right_tabs.addTab(self.statsTab, "Design Stats")
        self.right_tabs.setCurrentIndex(0)
        
        self.splitter1 = QtWidgets.QSplitter()
        self.splitter1.setOrientation(Qt.Vertical)
        
        self.splitter2 = QtWidgets.QSplitter()
        self.splitter2.setOrientation(Qt.Horizontal)
        
        self.splitter1.addWidget(self.splitter2)
#         self.splitter1.addWidget(self.bottom_tabs)
        
        self.splitter2.addWidget(self.tabEditor)
        self.splitter2.addWidget(self.right_tabs)

        self.splitter2.setStretchFactor(0, 1)
        self.splitter2.setStretchFactor(1, 0)

        
        grid = QtWidgets.QGridLayout()

        r = 0
        grid.addWidget(self.executionMode, r, 0)
        r += 1
        
        grid.addWidget(self.splitter1, r, 0)
        r += 1
        
        grid.setRowStretch(r-1, 1)
        
        window = QtWidgets.QWidget()
        window.setLayout(grid)
        self.setCentralWidget(window)
        
        self.setGeometry(100, 100, 1200, 900)
        self.setWindowTitle("HLS Debugger")
        
        self.show()
        self.refresh()
        
        try:
            fp = open(self.app_config_path(), "r")
        except (OSError, IOError):
            pass
        else:        
            design = fp.readline()
            fp.close()
            self.openDesign(design)
    
    def setBusyCursor(self):
        self.app.setOverrideCursor(QtGui.QCursor(Qt.WaitCursor))
        
    def clrBusyCursor(self):
        self.app.restoreOverrideCursor()
    
    def logModelsim(self, text):
        self.modelsimLog.appendPlainText(text)
    
    def app_data_dir(self):
        if os.name == "nt":
            return os.path.join(os.getenv('APPDATA'), "HLSDebugger")
        elif os.name == "posix":
            return os.path.expanduser(os.path.join("~", ".HLSDebugger"))
        else:
            assert False
        
    def app_config_path(self):
        return os.path.join(self.app_data_dir(), "config.txt")
        
    def closeEvent(self, e):
#         print("Close event")
        if self.manager.design:
            if not os.path.exists(self.app_data_dir()):
                os.makedirs(self.app_data_dir())
            fp = open(self.app_config_path(), "w")
            fp.write(self.manager.design.path)
            fp.close()
        
    def errorMessage(self, msg):
#         print("Error: " + msg)
        msgbox = QtWidgets.QMessageBox()
        msgbox.critical(self, "Error", msg)
        
    def disconnect(self):
        self.refreshConnection()
    
    def instructionModeChanged(self, mode):
        pass
#       self.manager.instruction_mode = mode
#       
#       if self.tabEditor:
#           self.tabEditor.instructionModeChanged()
#           
#       if self.manager.executionMode == manager.BACKEND_FPGA_REPLAY:     
#           self.update_slider_range()
#           self.slider.setValue(self.manager.trace_cycle())
        
    def refresh(self):
        design = self.manager.design
        
        if not design:
            return
            
        # Get the current state
        instanceNum = self.manager.currentState[0]
        
        # If the function has changed, reload the local variable names        
        if instanceNum != self.lastInstanceNum:
            self.lastInstanceNum = instanceNum
            
            if instanceNum is None:
                self.variables.locals.loadVariables([])
            else:
                function = design.instanceNumToInstance[instanceNum].function            
                self.variables.locals.loadVariables(function.variables)
        
        # Highlight lines
        if self.manager.design:
            activeInsns = self.manager.getActiveInsns()
            lines = []
            if activeInsns:
                for insn in activeInsns:
                    lines.append((insn.path, insn.line))
            
            self.tabEditor.updateActiveLines(lines)    
            self.tabEditor.refresh()
                
        self.variables.autoRefresh()
    
    def minorMessage(self, msg):
        self.statusBar().showMessage(msg, 5000)
        
    def majorMessage(self, msg):
        msgbox = QtWidgets.QMessageBox()
        msgbox.setWindowTitle(self.windowTitle())
        msgbox.setText(msg)
        msgbox.exec()
        
    def pause(self):
        pass
        
    def openFile(self):
        fname = QtWidgets.QFileDialog.getExistingDirectory(self, "Open Design Folder", paths.getDebugExamplesDir())
#         fname = QtWidgets.QFileDialog.getOpenFileName(self, "Open Debug File", paths.getDebugExamplesDir(), filter="Debug Mapping (*.dbgm)")[0]
#         print(fname)
        if not fname:
            return
        
        self.openDesign(fname)
        
    def openDesign(self, filename):
        self.manager.openDesign(filename)
            
    def designClosed(self):
        self.tabEditor.close_all_files()        
            
    def designOpened(self):
        for file in self.manager.design.getFilenames():
            if not os.path.isfile(file):
                self.errorMessage(file + " is missing from the file system.")
                self.manager.closeDesign()
                return
            self.tabEditor.add_file(file)
            
        self.setWindowTitle("HLS Debugger (" + self.manager.design.path + ")")
        
    def processEvents(self):
        self.app.processEvents()
            
class TabManager(QtWidgets.QTabWidget):
    def __init__(self, gui):
        super().__init__()
        
        self.widgetsByPath = {}
        self.gui = gui
    
    def close_all_files(self):
        for tab in self.widgetsByPath.values():            
            self.removeTab(self.indexOf(tab))
        self.widgetsByPath.clear()
        
    def instructionModeChanged(self):
        for i in range(self.count()):
            self.widget(i).instructionModeChanged()
            
    def removeInstructionSelections(self):
        for i in range(self.count()):
            self.widget(i).removeInstructionSelections()
        
    # This indicates a new set of lines to be active
    # lines should be a list of tuples: (filepath, lineNum)
    def updateActiveLines(self, lines):
        # uniquify
        lines = list(set(lines))
        
        for i in range(self.count()):            
            self.widget(i).removeHighlighting()
                
        lines.sort()
        
        # we want to build a single list of lineNums for each file
        fileLineNums = {}       
        
        for line in lines:
            # Remove insns that have no file source
            if line[0] == "":
                continue
            
            fileLineNums.setdefault(line[0], []).append(line[1])
            
        for file in fileLineNums:
            self.widgetsByPath[file].highlightLineNumbers(fileLineNums[file])
            
        files = list(fileLineNums.keys())
        
        # Select a new tab if necessary
        if len(files):
            activeFilepath = self.getFilepathFromWidget(self.currentWidget())       
            if not activeFilepath in files:
                self.setCurrentWidget(self.widgetsByPath[files[0]])

    def add_file(self, file):
        filename = os.path.split(file)[1]
        
        if file in self.widgetsByPath:
            raise
        
        sourceFile = code.SourceFile(file, self.gui)
        self.widgetsByPath[file] = sourceFile
        self.addTab(sourceFile, filename)
        
    def getFilepathFromWidget(self, sourceFile):
        for path, widget in self.widgetsByPath.items():
            if sourceFile == widget:
                return path
        return None   
    
    def refresh(self):
        for i in range(self.count()):
            self.widget(i).refresh()
        
        
