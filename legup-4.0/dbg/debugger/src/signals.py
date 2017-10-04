# This class is used to register/emit signals (callbacks)
class Signal():
    def __init__(self):
        self.fcns = []
        
    def connect(self, fcn):
        if fcn in self.fcns:
            raise ValueError
        
        self.fcns.append(fcn)
        
    def disconnect(self, fcn = None):
        if fcn:
            if fcn in self.fcns:
                self.fcns.remove(fcn)
            else:
                raise ValueError
        else:
            self.fcns = []
        
    def slots_connected(self):
        return len(self.fcns) > 0
        
    def emit(self, *args):
        for fcn in self.fcns:
            fcn(*args)
        