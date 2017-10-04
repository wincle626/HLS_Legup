.. _SYSTEM_MEMORY_SIM_INIT_FILE_NAME:

SYSTEM_MEMORY_SIM_INIT_FILE_NAME
--------------------------------

This parameter specifies the .dat file to be used to initialize the memory for
hybrid system simulation.

Category
+++++++++

Board and Device Specification

Value Type
+++++++++++

String

Valid Values
+++++++++++++

Any valid .dat file.

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

Related parameters: :ref:`SYSTEM_MEMORY_SIM_INIT_FILE_TYPE`

Applicable Flows
+++++++++++++++++

Software and hybrid flows

Test Status
++++++++++++

Actively in-use

Examples
+++++++++

``set_parameter SYSTEM_MEMORY_SIM_INIT_FILE_NAME altera_sdram_partner_module.dat``

--------------------------------------------------------------------------------
