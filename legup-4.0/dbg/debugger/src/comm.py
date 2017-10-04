# This module handles all communications between the 'Manager' python module
# and the hardware (comm.v)

import serial
import time
import threading
import sys

from backends import backend
import trace

# Full list of message codes.  A matching list is found in comm.v
# The two lists need to stay sync'd.
MSG_RUN = 1
MSG_RESET = 2
MSG_RUN_CYCLES = 3
MSG_PC_REQ = 4
MSG_PC_DATA = 5
MSG_SET_BREAKPOINT = 6
MSG_CLR_BREAKPOINT = 7
MSG_READ_MEM_REQ = 8
MSG_READ_MEM_DATA = 9
MSG_REFRESH_NOTICE = 10
MSG_READBACK_REQ = 11
MSG_READBACK_CONTROL = 12
MSG_READBACK_DATA = 13
MSG_READBACK_REGS = 14
MSG_READBACK_DONE = 15
MSG_SYSTEM_ID_REQ = 16
MSG_SYSTEM_ID_DATA = 17
MSG_WRITE_MEM_REQ = 18
MSG_TRACE_VAR_ENABLE = 19
MSG_TRACE_VAR_DISABLE = 20
MSG_TRACE_MODULE_ENABLE = 21
MSG_TRACE_MODULE_DISABLE = 22

# Exceptions

class CannotConnect(Exception):
    pass

class NotResponding(Exception):
    pass

class NotConnected(Exception):
    pass

class InvalidSystemId(Exception):
    pass

class NoMessageReceived(Exception):
    pass

class UnexpectedMessageTypeReceived(Exception):
    pass



class Comm():
    """
    HLSD Communication class.  Used to communicate with FPGA board via RS232
    
    :ivar PySerial ser: An instance of the PySerial class.
    :ivar bool refresh_logged: Indicates whether a request to refresh has been received from the circuit.  Once the refresh has been serviced, this will be set to False.
    """
    
    def __init__(self): 
        super().__init__()
                       
        self.ser = None
        self.refresh_logged = False
        
        # Communication lock, ensure that the timer doesn't interrupt 
        # long communications
        self.lock = threading.Lock()
        
        self.systemIdVerified = False
    
    def step(self):
        self.run_cycles(1)
    
    def check_for_refresh(self):
        """ 
        Checks if an unhandled refresh has occurred.  If a refresh message is pending, it will read it from the serial port.
        
        :returns: whether an unhandled refresh has occurred.
        """
        
        if not self.ser:
            return False
        
        if self.refresh_logged:
            self.refresh_logged = False
            return True
        
        elif self.ser.inWaiting():
            self.lock.acquire()
            data = self.ser.read(1)
            assert len(data) == 1
            self.lock.release()
            if data[0] != MSG_REFRESH_NOTICE:
                print("Unkown data: " + str(data[0]))
            assert data[0] == MSG_REFRESH_NOTICE
            return True        
        else:
            return False 
        
    # Attempt to connect to the serial port 
    def connect(self, comm_num):
        try:
            self.ser = serial.Serial(port = comm_num, baudrate=115200, timeout = 0.5)
        except serial.serialutil.SerialException as e:
            self.ser = None
            raise CannotConnect(str(e))
        
        # Read any leftover data from the serial
        leftover = 0
        while True:
            data = self.ser.read(1)
            if len(data) == 0:
                break
            leftover += 1
        
    # Return whether connected to the serial port
    def connected(self):
        if not self.ser:
            return False
        if not self.ser.readable():
            return False
        return True
        
    # Disconnect from the serial port
    def disconnect(self):
        self.ser.close()
        self.ser = None
            
    # Retrieve playback information from the trace buffer
    def getPlaybackInfo(self, traceDecoder):
        self.verifyIsConnected()
        
        self.lock.acquire()
        
        msg = bytearray([MSG_READBACK_REQ])
        self.ser.write(msg)
        
        done = False
        
        control_bytes = traceDecoder.traceControlReadbackBytes
        data_bytes = traceDecoder.traceDataReadbackBytes
        regs_bytes = traceDecoder.traceRegsReadbackBytes
        
        while not done:
            msg = self.get_msg_type()
            if msg == MSG_READBACK_CONTROL:
                data = self.ser.read(control_bytes)
                traceDecoder.addControlLine(data)
            elif msg == MSG_READBACK_DATA:
                data = self.ser.read(data_bytes)
                traceDecoder.addMemoryLine(data)
            elif msg == MSG_READBACK_REGS:
                data = self.ser.read(regs_bytes)
                traceDecoder.addRegsLine(data)
            elif msg == MSG_READBACK_DONE:
                done = True
            else:
                assert False
        self.lock.release()
        
    def get_msg_type(self):
        while True:
            msg = self.ser.read(1)
            if len(msg) == 0:
                raise NoMessageReceived
            elif msg[0] == MSG_REFRESH_NOTICE:
                self.refresh_logged = True
            else:
                return msg[0]
            
    # Retrieve the current state of the circuit
    def getState(self, moduleBytes, stateNumBytes):
        self.verifyIsConnected()
        
        self.lock.acquire()
        msg = bytearray([MSG_PC_REQ])
        self.ser.write(msg)
        
        try:
            msg = self.get_msg_type()
        except NoMessageReceived:
            self.disconnect()
            return (None, None)
            
        if msg != MSG_PC_DATA:
            print("Invalid message recevied: " + str(msg[0]))
            raise UnexpectedMessageTypeReceived
            
        data = self.ser.read(moduleBytes)
        module = int.from_bytes(data, byteorder='little')

        data = self.ser.read(stateNumBytes)     
        state = int.from_bytes(data, byteorder='little')
        
        self.lock.release()
        
        return (module, state)

    # Run the circuit for 'cycles' number of cycles
    def run_cycles(self, cycles):
        if cycles < 256:
            msg = bytearray([MSG_RUN_CYCLES, cycles])
            self.ser.write(msg)
        else:
            raise
        
    # Run the circuit indefinitely
    def run(self):
        msg_go = bytearray([MSG_RUN])
        self.ser.write(msg_go)
        
    # Reset the user circuit
    def reset(self):
        msg_reset = bytearray([MSG_RESET])
        self.ser.write(msg_reset)
        
    # Clear the hardware breakpoint
    def clr_breakpoint(self):
        msg = bytearray([MSG_CLR_BREAKPOINT])
        self.ser.write(msg)
    
    # Set the hardware breakpoint
    def setBreakpoint(self, instanceNumBytes, instanceNum, stateNumBytes, stateNum):
        self.verifyIsConnected()
        
        data = bytearray([MSG_SET_BREAKPOINT])
        self.ser.write(data)
        
        instanceMsg = instanceNum.to_bytes(instanceNumBytes, byteorder='little')
        stateMsg = stateNum.to_bytes(stateNumBytes, byteorder='little')
        
        self.ser.write(instanceMsg)
        self.ser.write(stateMsg)
        
        # Num conditions
#       data = bytearray([0])
#       self.ser.write(data)
        
    # Retrieve the system ID, and check that it matches
    def verifySystemID(self, system_id):
        self.systemIdVerified = False
        
        self.verifyIsConnected()
        
        self.lock.acquire()
        msg = bytearray([MSG_SYSTEM_ID_REQ])
        self.ser.write(msg)
        
        msg = self.get_msg_type()
        assert msg == MSG_SYSTEM_ID_DATA
        
        msg = self.ser.read(4)
        self.lock.release()
        
        if len(msg) == 0:
            raise NotResponding
        
        system_id_recvd = int.from_bytes(msg, byteorder='little') 
        
        if system_id == system_id_recvd:
            self.systemIdVerified = True
        else:
            raise InvalidSystemId(system_id_recvd)
        
    # Write to a variable
    def varWrite(self, tag, offset, val):
        raise NotImplementedError
        msg = bytearray([MSG_WRITE_MEM_REQ])
        self.ser.write(msg)
        
        # Send address
        addr = int ((tag << 23) | offset)
        msg = addr.to_bytes(4, byteorder="little")
        self.ser.write(msg)
        
        # Send data
        msg = val.to_bytes(8, byteorder="little") 
        self.ser.write(msg)
                
    # Read a variable
    def ramRead(self, ram, offset, size):
        msg = bytearray([MSG_READ_MEM_REQ])
        tag = ram.tag
        
        # Message Type
        assert size in (1, 2, 4, 8)
        msg.append(size)
        
        # Offset        
        assert offset < (1 << 23)
        
        # Address
        address = int((tag << 23) | offset)
        msg += address.to_bytes(4, byteorder="little")            
            
        for b in msg:
            self.ser.write(bytearray([b]))
            time.sleep(0.01)
        
        msg = self.get_msg_type()
        
        if msg != MSG_READ_MEM_DATA:
            print("Invalid message recevied: " + str(msg))
            raise UnexpectedMessageTypeReceived
        
        data = self.ser.read(size)
#         print("Read from (" + str(tag) + "," + str(offset) + ")")
        if len(data) != size:
            print("Read read error. Got " + str(len(data)) + " bytes, expected " + str(size))
            assert False
            
        assert len(data) == size
        
        result = int.from_bytes(data, byteorder='little')
        return result
    
    def verifyIsConnected(self):
        if not self.connected():
            raise NotConnected
    
    # Enable/Disable tracing of a variable
    def traceVarEnable(self, tag, enable):
        self.verifyIsConnected()
        
        if enable:
            msg = bytearray([MSG_TRACE_VAR_ENABLE])
        else:
            msg = bytearray([MSG_TRACE_VAR_DISABLE])

        msg+= tag.to_bytes(2, byteorder="little")        
        self.ser.write(msg)
        self.serialSleep()

    # Enable/Disable tracing of a module
    def traceFunctionEnable(self, instanceID, enable):
        self.verifyIsConnected()
        
        if enable:
            msg = bytearray([MSG_TRACE_MODULE_ENABLE])
        else:        
            msg = bytearray([MSG_TRACE_MODULE_DISABLE])
                    
        msg += instanceID.to_bytes (2, byteorder="little")
        self.ser.write(msg)
        self.serialSleep()
        
    def serialSleep(self):
        time.sleep(0.01)
