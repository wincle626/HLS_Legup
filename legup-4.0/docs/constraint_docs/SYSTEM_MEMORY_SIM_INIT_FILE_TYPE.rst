.. _SYSTEM_MEMORY_SIM_INIT_FILE_TYPE:

SYSTEM_MEMORY_SIM_INIT_FILE_TYPE
--------------------------------

This parameter specifies the file format of the simulation initialization file
specified by :ref:`SYSTEM_MEMORY_SIM_INIT_FILE_NAME`.

Category
+++++++++

Board and Device Specification

Value Type
+++++++++++

String

Valid Values
+++++++++++++

DAT

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

Related parameters: :ref:`SYSTEM_MEMORY_SIM_INIT_FILE_NAME`

Applicable Flows
+++++++++++++++++

Software and hybrid flows

Test Status
++++++++++++

Actively in-use.

Examples
+++++++++

``set_parameter SYSTEM_MEMORY_SIM_INIT_FILE_TYPE DAT``

--------------------------------------------------------------------------------
