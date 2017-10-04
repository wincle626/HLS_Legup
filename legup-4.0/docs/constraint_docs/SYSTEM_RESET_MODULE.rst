.. _SYSTEM_RESET_MODULE:

SYSTEM_RESET_MODULE
-------------------

This parameter specifies the QSys module that supplies the system reset.
Probably the same as :ref:`SYSTEM_CLOCK_INTERFACE`.

Category
+++++++++

Board and Device Specification

Value Type
+++++++++++

String

Valid Values
+++++++++++++

The name of the QSys module that supplies the system reset.

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

Related parameters: :ref:`SYSTEM_RESET_INTERFACE`

Applicable Flows
+++++++++++++++++

Software and hybrid flows

Test Status
++++++++++++

Actively in-use

Examples
+++++++++

``set_parameter SYSTEM_RESET_MODULE clk``

.. image:: images/SYSTEM_images/clk.png

--------------------------------------------------------------------------------
