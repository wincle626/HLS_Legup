import subprocess
import time
import socket
import os
import paths
import select
import sys

class CannotFindModelsim(Exception):
    pass

# modelsimSock = None
# modelsimProcess = None

class Modelsim():
    def __init__(self):
        self.process = None
        self.sock = None
        self.socketClosed = None

    @staticmethod
    def getVsimPath():
        process = subprocess.Popen(["which", "vsim"], stdout = subprocess.PIPE)
        output = process.communicate()[0]
        process.wait()
        output = output.decode('utf-8')
        
        if output == "":        
            raise CannotFindModelsim        
        else:
            return output.strip()
        
    
    def run(self):
        
        vsimPath = self.getVsimPath()
        
        listenerScriptPath = os.path.join(paths.getScriptsDir(), "ModelsimListener.tcl")
        remoteFileLoadCommand = listenerScriptPath
        FNULL = open(os.devnull, 'w')
        self.process = subprocess.Popen([vsimPath, "-c", "-do", remoteFileLoadCommand], stdout = FNULL, stderr = FNULL)
        
        success = False
        self.sock = socket.socket()
        for _ in range(15):
            try:
                self.sock.connect(("127.0.0.1", 2000,))
            except ConnectionRefusedError:
                time.sleep(1)
            else:
                success = True
                break
            
        assert success
        print (self.getSocketData())
        self.socketClosed = False
    
    def sendCmd(self, cmd):
        # We want to make sure there isn't already an unhandled message sitting
        # in the socket.  
        rd_socks = select.select([self.sock], [], [], 0)[0]
        if self.sock in rd_socks:
            s = self.getSocketData()
            if len(s) == 0:
                self.socketClosed = True
            else:
                print ("Unexpected data from Modelsim: '" + s + "'")
                sys.exit(-1)
        
        cmdWithNewline = cmd + "\n"
        self.sock.send(cmdWithNewline.encode('utf-8'))
        
        data = self.getSocketData()
                
        if data[-7:] == "\r\nVSIM>":
            data = data[:-7]
            
        return data
    
    def getSocketData(self):
        data = self.sock.recv(2048)
        data = data.decode('utf-8')
        return data

    def stop(self):
        if not self.socketClosed:
            self.sendCmd(chr(27))
        
        # Wait a few seconds for it to exit gracefully, then kill it
        start = time.clock()
        while time.clock() - start < 3.0:
            if self.process.poll() is not None:
                return
            
        print ("Killing modelsim...")
        self.process.kill()