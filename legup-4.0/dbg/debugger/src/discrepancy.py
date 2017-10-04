import modelsim
import subprocess
import paths
import os
import shutil

def binPath():
    return os.path.join(paths.getBinDir(), "Discrepancy.o")

def rptPath(design):
    return os.path.join(design.path, "SWRTLDiscrepancy.log")

def run(design):
    os.chdir(design.path)
    shutil.copyfile(os.path.join(paths.getScriptsDir(), "ModelsimListener.tcl"), "ModelsimListener.tcl")
    
    fp = open("Inspect.config", "w")
    fp.write("ModelsimPath=" + os.path.split(modelsim.Modelsim.getVsimPath())[0] + os.sep + "\n")
    fp.write("LegUpPath=" + paths.getLegUpDir() + os.sep + "\n")
    fp.write("ExamplePath=" + design.path + os.sep + "\n")
    fp.write("ExampleFile=" + design.name + "\n")
    fp.write("DBHost=localhost\n")
    fp.write("DBUser=root\n")
    fp.write("DBPass=letmein\n")
    fp.write("DBName=inspect_db\n")
    fp.close()
    
   
#         discrepancyBinPath = "/home/jgoeders/Dropbox/linux/legup-repo/InspectDebugger/Inspect/InspectDebugger"

#         self.process = subprocess.Popen([discrepancyBinPath], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    # By default when you call a new terminal it returns immediately (even if you don't use '&')
    # --disable-factory forces it to run blocking 
    p = subprocess.Popen(["gnome-terminal", "--disable-factory", "-x", binPath()])
    p.wait()
