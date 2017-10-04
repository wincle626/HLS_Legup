.. _SYSTEM_MEMORY_INTERFACE:

SYSTEM_MEMORY_INTERFACE
-----------------------

This parameter specifies the 'Avalon Memory Mapped Slave' interface for the
system memory.

Category
+++++++++

Board and Device Specification

Value Type
+++++++++++

String

Valid Values
+++++++++++++

The 'Avalon Memory Mapped Slave' interface of the QSys module that supplies the
system memory.

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

Related parameters: :ref:`SYSTEM_MEMORY_MODULE`

Applicable Flows
+++++++++++++++++

Software and hybrid flows

Test Status
++++++++++++

Actively in-use

Examples
+++++++++

``set_parameter SYSTEM_MEMORY_INTERFACE cache_slave``

.. image:: images/SYSTEM_images/DCache_cache_slave.png

--------------------------------------------------------------------------------
