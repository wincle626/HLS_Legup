.. _SYSTEM_MEMORY_BASE_ADDRESS:

SYSTEM_MEMORY_BASE_ADDRESS
--------------------------

This parameter specifies the system memory base address.  This is the address
at which the program will be linked.  It should be the same as the memory base
address in QSys.

Category
+++++++++

Board and Device Specification

Value Type
+++++++++++

Integer

Valid Values
+++++++++++++

The base address of the memory in QSys.

Default Value
++++++++++++++

N/A

Location Where Default is Specified
+++++++++++++++++++++++++++++++++++

``boards/<device_family>/<board>/<system>/legup.tcl``

e.g. ``boards/CycloneV/DE1-SoC/Tiger_SDRAM/legup.tcl``

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

``set_parameter SYSTEM_MEMORY_BASE_ADDRESS 0x40000000``

--------------------------------------------------------------------------------
