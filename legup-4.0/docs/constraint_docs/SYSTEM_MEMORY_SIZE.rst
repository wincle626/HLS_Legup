.. _SYSTEM_MEMORY_SIZE:

SYSTEM_MEMORY_SIZE
------------------

This parameter specifies the size of the memory in the QSys system.

Category
+++++++++

Board and Device Specification

Value Type
+++++++++++

Integer

Valid Values
+++++++++++++

The size of the system memory, in bytes.

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

Related parameters: :ref:`SYSTEM_MEMORY_BASE_ADDRESS` :ref:`SYSTEM_MEMORY_WIDTH`

Applicable Flows
+++++++++++++++++

Software and hybrid flows

Test Status
++++++++++++

Actively in-use

Examples
+++++++++

``set_parameter SYSTEM_MEMORY_SIZE 0x04000000``

--------------------------------------------------------------------------------
