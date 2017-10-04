.. _SYSTEM_PROJECT_NAME:

SYSTEM_PROJECT_NAME
-------------

The name of the directory containing the Quartus II project and Qsys system to
be used by the software and hybrid makefile targets. The directory must also 
include a legup.tcl file that describes the Qsys system. This constraint is set 
in the set_project function and should not be set independently.

Category
+++++++++

Board and Device Specification

Value Type
+++++++++++

String

Valid Values
+++++++++++++

Any valid directory name

Default Value
++++++++++++++

Tiger_SDRAM

Location Where Default is Specified
+++++++++++++++++++++++++++++++++++

``examples/legup.tcl``

Dependencies
+++++++++++++

None

Applicable Flows
+++++++++++++++++

Software and hybrid flows

Test Status
++++++++++++

Actively in-use

Examples
+++++++++

    ``set_parameter SYSTEM_PROJECT_NAME Tiger_SDRAM``

--------------------------------------------------------------------------------

