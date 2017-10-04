.. _SYSTEM_PROCESSOR_NAME:

SYSTEM_PROCESSOR_NAME
---------------------

This parameter specifies the QSys module that supplies the system processor.

Category
+++++++++

Board and Device Specification

Value Type
+++++++++++

String

Valid Values
+++++++++++++

The name of the QSys processor module.

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

``set_parameter SYSTEM_PROCESSOR_NAME Tiger_MIPS``

.. image:: images/SYSTEM_images/Tiger_MIPS.png

--------------------------------------------------------------------------------
