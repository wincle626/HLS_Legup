from PyQt5 import QtWidgets
from PyQt5 import QtCore
from PyQt5 import QtGui
from PyQt5.QtCore import Qt

import manager
import comm
import design

class VariablesWidget(QtWidgets.QWidget):
    def __init__(self, gui, name):
        super().__init__()
                
        self.gui = gui

        
        self.variables = None
        self.var_by_row = {}
                
        self.grid = QtWidgets.QGridLayout()
        self.setLayout(self.grid)
        
        self.table = QtWidgets.QTableWidget()
        self.table.cellChanged.connect(self.cell_changed)
        
        self.label_title = QtWidgets.QLabel(name + " Variables")
        self.table.horizontalHeader().setSectionResizeMode(QtWidgets.QHeaderView.Stretch)
        self.grid.addWidget(self.label_title, 0, 0)
        self.grid.addWidget(self.table, 1, 0)
        
        self.table.setColumnCount(2)
        self.table.setHorizontalHeaderLabels(["Name", "Value"])
        self.table.verticalHeader().setVisible(False)
        
    def cell_changed(self, row, col):
        return
    
        if self.disable_cell_changed:
            return
        
        assert row in self.var_by_row
        assert col == 1
        
        var = self.var_by_row[row]
        val = int(self.table.item(row, col).text(), 0)
        self.gui.manager.varWrite(var.tag, 0, val)        
        
#         print("changed " + str(row) + "," + str(col) + "tag: " + str(self.var_by_row[row].tag))
    
    def loadVariables(self, var_list):
        # Don't get notifications of cell updates when adding new rows to the table
        self.disable_cell_changed = True
        
        self.variables = var_list
        
        # Delete existing rows
        self.table.setRowCount(0)
        self.var_by_row = {}
        
        for var in self.variables:
            row = self.table.rowCount()
            self.table.insertRow(row)
            var_name_item = QtWidgets.QTableWidgetItem(var.name)
            var_name_item.setFlags(var_name_item.flags() ^ QtCore.Qt.ItemIsEditable)
            self.table.setItem(row, 0, var_name_item)
            self.var_by_row[row] = var

        # Continue to get notifications of cell updates
        self.disable_cell_changed = False
    
    def updateVariableValues(self):
        # Don't get notifications of cell updates when updating sources
        self.disable_cell_changed = True
        
        for row in range(len(self.variables)):
            var = self.variables[row]
            try:
                val = self.gui.manager.varRead(var, 0)
                val = str(val)
#                 val = hex(val)
            except manager.VariableNotAccessible:
                val = "<N/A>"
            except manager.VariableOptimizedAway:
                val = "<Optimized Away>"
            except manager.VariableValueUnknown:
                val = "<Unknown>"
            except manager.VariableNotTraced:
                val = "<Not Recorded>"
            except manager.VariableValueUndefined:
                val = "<Undefined>" 

            item = self.table.item(row, 1)
            if item is None:
                item = QtWidgets.QTableWidgetItem()
                item.setTextAlignment(QtCore.Qt.AlignRight)
                self.table.setItem(row, 1, item)   
            item.setText(val)
    
        # Continue to get notifications of cell updates
        self.disable_cell_changed = False
        
class VariableTreeItem(QtWidgets.QTreeWidgetItem):
    def __init__(self, var, *args, **kw):
        super().__init__(*args, **kw)
        self.var = var
        self.isPrimitive = False
        
    def setIsPrimitive(self, offset, width):
        self.isPrimitive = True
        self.offset = offset
        self.width = width
        
    def isVisible(self):
        item = self.parent()
        while item:
            if not item.isExpanded():
                return False
            item = item.parent()
        return True
        
class Variables(QtWidgets.QWidget):
    def __init__(self, gui):
        super().__init__()
        self.gui = gui
        self.locals_function = None
        self.chkbox_auto_refresh_old_value = None
        self.disableAutoUpdate = False
        
        self.disable_cell_changed = False
        
        self.items = {}
        
        self.gui.manager.executionModeChanged.connect(self.executionModeChanged)
        self.gui.manager.designClosed.connect(self.designClosed)
        self.gui.manager.designOpened.connect(self.designOpened)
        
        self.button_refresh = QtWidgets.QPushButton("Refresh Values")             
        self.chkboxAutoRefresh = QtWidgets.QCheckBox("Auto-Refresh")
        self.chkboxAutoRefresh.setTristate(False)
        
        self.button_refresh.clicked.connect(self.refreshValues)

        self.tree = QtWidgets.QTreeWidget()
        self.tree.setColumnCount(2)
        self.tree.setHeaderLabels(["Variable", "Value"])
        self.tree.setIndentation(12)

        # Global widget
        self.globals = VariablesWidget(gui, "Global")
        
        # Local widget
        self.locals = VariablesWidget(gui, "Local")
        
        # Splitter
        self.splitter = QtWidgets.QSplitter()
        self.splitter.setOrientation(QtCore.Qt.Vertical)
        
        self.splitter.addWidget(self.globals)
        self.splitter.addWidget(self.locals)
        
        # Grid Layout
        grid = QtWidgets.QGridLayout()
        self.setLayout(grid)
        
        
        r = 0
        grid.addWidget(self.chkboxAutoRefresh, r, 0)
        grid.addWidget(self.button_refresh, r, 1)
        r += 1
        
        grid.addWidget(self.tree, r, 0, 1, 4)
        r += 1
        
#         grid.addWidget(self.splitter, r, 0, 1, 4)
#         r += 1
        
        grid.setColumnStretch(0, 1)
        grid.setRowStretch(0, 1)
        grid.setRowStretch(1, 1)     

    def designClosed(self):
        self.tree.clear()
    
    def designOpened(self):
        if not self.gui.manager.design:
            return
        
        design = self.gui.manager.design
        
        self.tree.clear()

        self.headerGlobals = QtWidgets.QTreeWidgetItem(self.tree)
        self.headerGlobals.setText(0, "--Globals--")
        self.headerFont = QtGui.QFont(self.headerGlobals.font(0))
        self.headerFont.setBold(True)
        self.headerFont.setPointSize(11)
        self.headerGlobals.setFont(0, self.headerFont)
        self.headerGlobals.setExpanded(True)
        self.tree.addTopLevelItem(self.headerGlobals)
                
        for var in design.globalVariables:
            self.addItem(self.headerGlobals, var, 0, var.name, var.varType, 0)
            
        for f in design.functions:
            if len(f.variables) == 0:
                continue
            fItem = QtWidgets.QTreeWidgetItem(self.tree)
            fItem.setText(0, f.name)
            fItem.setFont(0, self.headerFont)
            fItem.setExpanded(True)
            
            for var in f.variables:
                self.addItem(fItem, var, 0, var.name, var.varType, 0)
              
        self.autoRefresh() 
        
#     def buildArrayLevel(self, var, baseOffset, parentItem, varType, memberIdx):
#         assert varType.members[memberIdx].isSubrangeCount()
#         offset = baseOffset
#         for i in range(varType.members[memberIdx].subrangeCount):
#              if memberIdx == 
#             item = VariableTreeItem(parentItem, offset, parentItem)
#             item.setText(0, "[" + str(i) + "]")
        
#     def buildCompositeItem(self, parentItem, baseOffset, varType, memberIdx):
#         for i in range:
        
    def addItem(self, header, var, offset, name, varType, memberIdx):
        
        # These types of variables we just ignore and recursively call addItem on the
        # type they are derived from.
        if varType.dw_tag in (design.DW_TAG_CONST, design.DW_TAG_TYPEDEF):
            return self.addItem(header, var, offset, name, varType.derivedType, memberIdx)
        
        item = VariableTreeItem(var, header)
        item.setText(0, name)
        size = 0
        
        if varType.dw_tag == design.DW_TAG_ARRAY:

            
            for i in range(varType.members[memberIdx].subrangeCount):
                childName = "[" + str(i) + "]"
                if memberIdx == len(varType.members) - 1:
                    # Leaf nodes of array
                    childSize = self.addItem(item, var, offset + size, childName, varType.derivedType, 0)
                else:
                    # There is another dimention of array
                    childSize = self.addItem(item, var, offset + size, childName, varType, memberIdx + 1)
                if not childSize:
                    print (var.name)
                assert childSize
                size += childSize
        elif varType.dw_tag == design.DW_TAG_STRUCT:
            for m in varType.members:
                childType = m.varType
                assert childType
                
                assert childType.offset % 8 == 0
                self.addItem(item, var, offset + childType.offset // 8, childType.name, childType.derivedType, 0)
            assert varType.size % 8 == 0
            size = varType.size // 8
                
        elif varType.dw_tag == design.DW_TAG_BASE_TYPE:
            assert varType.size % 8 == 0
            width = varType.size // 8
            item.setIsPrimitive(offset, width)
            size = width
        
        if item.childCount() < 10:
            item.setExpanded(True)
        else:
            item.setExpanded(False)
        return size
            
    def forceAutoRefresh(self):
        self.chkboxAutoRefresh.setChecked(Qt.Checked)
        self.chkboxAutoRefresh.setEnabled(False)
        self.refreshValues()
        
    def unforceAutoRefresh(self):
        self.chkboxAutoRefresh.setChecked(Qt.Unchecked)
        self.chkboxAutoRefresh.setEnabled(True)
        
    def refreshValues(self):
        if not self.gui.manager.design:
            return 
        
        # Don't get notifications of cell updates when updating sources
        self.disable_cell_changed = True
        
        treeIter = QtWidgets.QTreeWidgetItemIterator(self.tree)
        
        while treeIter.value():
            item = treeIter.value()
            if type(item) == VariableTreeItem and item.isPrimitive:
                
                if item.isVisible():
                    var = item.var
                    offset = item.offset
                    size = item.width
                    try:
                        val = self.gui.manager.varRead(var, offset, size)
                        val = str(val)
        #                 val = hex(val)
                    except comm.NotConnected:
                        self.gui.errorMessage("Not connected to FPGA")
                        return
                    except manager.VariableNotAccessible:
                        val = "<N/A>"
                    except manager.VariableOptimizedAway:
                        val = "<Optimized Away>"
                    except manager.VariableValueUnknown:
                        val = "<Unknown>"
                    except manager.VariableNotTraced:
                        val = "<Not Recorded>"
                    except manager.VariableValueUndefined:
                        val = "<Undefined>" 
        
                    item.setText(1, val)
                    item.setTextAlignment(1, QtCore.Qt.AlignRight)
                
            treeIter += 1
            
#             if item is None:
#                 item = QtWidgets.QTableWidgetItem()
#                 item.setTextAlignment(QtCore.Qt.AlignRight)
#                 self.table.setItem(row, 1, item)   
#             item.setText(val)
    
        # Continue to get notifications of cell updates
        self.disable_cell_changed = False
        
    def autoRefresh(self):
        if self.chkboxAutoRefresh.isChecked() and not self.disableAutoUpdate:
            self.refreshValues()

    def executionModeChanged(self):
        if self.gui.manager.activeBackend.VARIABLES_AUTO_REFRESH:
            self.forceAutoRefresh()
        else:
            self.unforceAutoRefresh()
