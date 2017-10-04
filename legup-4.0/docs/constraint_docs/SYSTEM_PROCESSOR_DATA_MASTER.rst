.. _SYSTEM_PROCESSOR_DATA_MASTER:

SYSTEM_PROCESSOR_DATA_MASTER
----------------------------

This parameter specifies the 'Avalon Memory Mapped Master' for the data
interface of the system processor.

Category
+++++++++

Board and Device Specification

Value Type
+++++++++++

String

Valid Values
+++++++++++++

The 'Avalon Memory Mapped Master' used for data by the QSys module that supplies
the system processor.

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

Related parameters: :ref:`SYSTEM_PROCESSOR_NAME`

Applicable Flows
+++++++++++++++++

Software and hybrid flows

Test Status
++++++++++++

Actively in-use

Examples
+++++++++

``set_parameter SYSTEM_PROCESSOR_DATA_MASTER data_master``

.. image:: images/SYSTEM_images/Tiger_MIPS_data_master.png

--------------------------------------------------------------------------------
