-----------------------------------------------------------------------------------------------------
-------------------------------LegUp Compilation for Inspect Debugger:-------------------------------
-----------------------------------------------------------------------------------------------------

Packages need to be installed:
mysql-server
libmysqlclient-dev

Makefile parameters:
	NO_OPT = 1
	NO_INLINE = 1
	DEBUG_G_FLAG = 1

"Make inspec" should be called in order to work with the inspect debugger.

LegUp parameters (legup.tcl):
INSPECT_DEBUG = 1
INSPECT_ONCHIP_BUG_DETECT_MODE = 0
DEBUG_DB_HOST = your database server address (localhost if you’re hosting the database on your own machine)
DEBUG_DB_USER = the database user "root" by default)
DEBUG_DB_PASSWORD = DB password
DEBUG_DB_SCRIPT_FILE = /address_to_legup_examples_folder/inspect_db.sql
The code checks for the existence of inspect database at every run. If the DB is not found, it will be installed automatically.

-----------------------------------------------------------------------------------------------------
-------------------------------------Running Inspect Debugger:---------------------------------------
-----------------------------------------------------------------------------------------------------
Packages need to be installed:
libmigdb (Download from: http://sourceforge.net/projects/libmigdb/) , see README for installation guide.

Before using the inspect debugger, the provided example should be compiled with "make inspect" directive and the debug database should be created.

"Inspect.config" file contains the debugger configurations:
ModelsimPath = ModelSim directory (/path_to_modelsim_main_dir/modelsim_ase/linuxaloem)
LegUpPath = path to the root folder of the LegUp
ExamplePath = the "folder" that contains the example that needs to be debugged
ExampleFile= the "file name" in the ExamplePath that contains the main C code.
DBHost = Database Host address (localhost)
DBUser = "root" by default
DBPass= database server password
DBName= legupDebug

Run "make" to compile the project. Make sure that "-lmysqlclient -lz -lmigdb" are added to "LIBS" and also the including directory for libmigdb is added to "INCPATH"

"./inspectDebugger" will run the inspect debugger tool.

-----------------------------------------------------------------------------------------------------
---------------------------------Compiling Discrepancy Binary:---------------------------------------
-----------------------------------------------------------------------------------------------------
Add "-DDISCREP" to DEFINES flag
Change TARGET to "Discrepancy.o"
Run make to get Discrepancy.o binary

