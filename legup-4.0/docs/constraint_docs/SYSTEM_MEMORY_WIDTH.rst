.. _SYSTEM_MEMORY_WIDTH:

SYSTEM_MEMORY_WIDTH
-------------------

This parameter specifies the width of the memory component specified by
:ref:`SYSTEM_MEMORY_MODULE`'

Category
+++++++++

Board and Device Specification

Value Type
+++++++++++

Integer

Valid Values
+++++++++++++

1, 2, 4, 8, etc.

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

Related parameters: :ref:`SYSTEM_MEMORY_SIZE` :ref:`SYSTEM_MEMORY_BASE_ADDRESS`

Applicable Flows
+++++++++++++++++

Software and hybrid flows

Test Status
++++++++++++

Actively in-use

Examples
+++++++++

``set_parameter SYSTEM_MEMORY_WIDTH 2``

--------------------------------------------------------------------------------
