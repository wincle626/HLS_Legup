import os

def getSrcDir():
    return os.path.dirname(os.path.realpath(__file__))

def getDebuggerDir():
    return os.path.abspath(os.path.join(getSrcDir(), os.pardir))

def getScriptsDir():
    return os.path.join(getDebuggerDir(), "scripts")

def getLegUpDir():
    return os.path.abspath(os.path.join(getDebuggerDir(), os.pardir, os.pardir))

def getRtlDir():
    return os.path.join(getLegUpDir(), "dbg", "rtl")

def getResDir():
    return os.path.join(getDebuggerDir(), "res")

def getExamplesDir():
    return os.path.join(getLegUpDir(), "examples")

def getDebugExamplesDir():
    return os.path.join(getExamplesDir(), "debug")

def getBinDir():
    return os.path.join(getDebuggerDir(), "bin")

def getAlteraIpDir():
    return os.path.join(getLegUpDir(), "ip", "libs", "altera")