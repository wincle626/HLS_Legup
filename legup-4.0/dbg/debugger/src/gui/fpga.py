import os

from PyQt5 import QtWidgets
from PyQt5 import QtGui

import paths
import subprocess
import re
import serial
from serial.tools import list_ports

from backends import backend

class FpgaTab(QtWidgets.QWidget): 
    def __init__(self, gui):
        
        super().__init__()
        
        self.gui = gui
        self.manager = gui.manager
        
        self.manager.designOpened.connect(self.refreshConnection)
        
        grid = QtWidgets.QGridLayout()
        self.setLayout(grid)
        
        font = QtGui.QFont()
        font.setBold(True)
        font.setUnderline(True)
        
        self.labelFPGA = QtWidgets.QLabel("FPGA")
        self.labelFPGA.setFont(font)
        
        self.buttonProgram = QtWidgets.QPushButton("Program Bitstream")
        self.buttonProgram.clicked.connect(self.programFpga)
        
        self.labelSerial = QtWidgets.QLabel("RS232 Connection")
        self.labelSerial.setFont(font)
        
        self.labelConnection = QtWidgets.QLabel("Status:")
        self.labelConnectionStatus = QtWidgets.QLabel("")
        
        self.labelPort = QtWidgets.QLabel("Port:")
        
        self.comboPort = QtWidgets.QComboBox()
        
        self.buttonRefreshPorts = QtWidgets.QPushButton()
        self.buttonRefreshPorts.setIcon(QtGui.QIcon(os.path.join(paths.getResDir(), "refresh.png")))
        self.buttonRefreshPorts.clicked.connect(self.refreshAvailPorts)

        self.buttonConnect = QtWidgets.QPushButton("Disconnect")
        self.buttonConnect.clicked.connect(self.connect_disconnect)
        
        
        r = 0
        
        grid.addWidget(self.labelFPGA, r, 0, 1, 3)
        r += 1
        
        grid.addWidget(self.buttonProgram, r, 0, 1, 3)
        r += 1
        
        grid.setRowStretch(r, 1)
        r += 1
        
        grid.addWidget(self.labelSerial, r, 0, 1, 3)
        r += 1
        
        grid.addWidget(self.labelConnection, r, 0)
        grid.addWidget(self.labelConnectionStatus, r, 1)
        r += 1
        
        grid.addWidget(self.labelPort, r, 0)
        grid.addWidget(self.comboPort, r, 1)
        grid.addWidget(self.buttonRefreshPorts, r, 2)
        r += 1
        
        grid.addWidget(self.buttonConnect, r, 1)
        r += 1
        
        grid.setRowStretch(r, 10)
                
        self.refreshAvailPorts()
        self.refreshConnection()
        
        self.connect_if_single()
        
    def programFpga(self):
        if not self.manager.design:
            self.manager.errorMessage("There is no design open")
        
        designFolder = self.manager.design.path
        sofPath = os.path.join(designFolder, "top.sof")
        
        if not os.path.isfile(sofPath):
            self.manager.errorMessage(sofPath + " not found.")
            return
        
        self.manager.minorMessage.emit("Programming " + sofPath)
        
        p = subprocess.Popen(["quartus_pgm", "-l"], stdout = subprocess.PIPE, stderr = subprocess.PIPE)
        output = p.communicate()[0].decode()
        m = re.search("^\d+\) (USB-Blaster\s+\S+)$", output, re.M)
        if m:
            cable = m.group(1)
        else:
            self.manager.errorMessage("No USB-Blaster cable detected")
            return
        
        self.gui.setBusyCursor()
        p = subprocess.Popen(["quartus_pgm", "-c", cable, "-m", "jtag", "-o", "p;" + sofPath], stdout = subprocess.PIPE, stderr = subprocess.PIPE)
        output = p.communicate()[0].decode()
        self.gui.clrBusyCursor()
        
        if not re.search("successful", output):
            self.manager.errorMessage(output)
        else:
            self.manager.minorMessage.emit("FPGA programming successful.")
        
#       print(output)
        
        
    def connect_if_single(self):
        if self.comboPort.count() == 1:
            self.connect_disconnect()
        
    def connect_disconnect(self):
        if self.manager.executionMode != backend.BACKEND_FPGA_LIVE:
            self.manager.errorMessage("You must be in 'FPGA Live' mode to connect to a FPGA board.")
            return
        
        if self.buttonConnect.text() == "Connect":
            text = self.comboPort.currentText()
            if not text:
                return 
            
            if self.manager.commConnected() and not self.manager.system_id_verified:
                self.manager.verifySystemID()
            elif not self.manager.commConnected():
                self.manager.commConnect(text)
            else:
                assert False
        else:
            self.manager.commDisconnect()
    
        self.refreshConnection()
        
    def refreshConnection(self):
        if self.manager.commConnected():
            self.labelConnectionStatus.setText("Connected")
            self.labelConnectionStatus.setStyleSheet("QLabel {color:green}")
            self.buttonConnect.setText("Disconnect")
            self.manager.activeBackend.updateState()
        else:
            self.labelConnectionStatus.setText("Disconnected")
            self.labelConnectionStatus.setStyleSheet("QLabel {color:red}")
            self.buttonConnect.setText("Connect")
            
    
    def refreshAvailPorts(self):
        while self.comboPort.count():
            self.comboPort.removeItem(0)
            
        ports = list(self.serial_ports())
        self.comboPort.addItems(ports)
        
    def serial_ports(self):
        """
        Returns a generator for all available serial ports
        """
        if os.name == 'nt':
            # windows
            for i in range(256):
                try:
                    s = serial.Serial(i)
                    s.close()
                    yield ("COM" + str(i + 1))
                except serial.SerialException:
                    pass
        else:
            # unix
            for port in list_ports.comports():
                yield port[0]
