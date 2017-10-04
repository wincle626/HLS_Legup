.. _SYSTEM_PROCESSOR_ARCHITECTURE:

SYSTEM_PROCESSOR_ARCHITECTURE
-----------------------------

This parameter specifies the architecture of the processor in the QSys system.

Category
+++++++++

Board and Device Specification

Value Type
+++++++++++

String

Valid Values
+++++++++++++

MIPSI, ARMA9

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

``set_parameter SYSTEM_PROCESSOR_ARCHITECTURE MIPSI``

--------------------------------------------------------------------------------
