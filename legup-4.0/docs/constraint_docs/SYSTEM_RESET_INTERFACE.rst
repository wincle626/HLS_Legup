.. _SYSTEM_RESET_INTERFACE:

SYSTEM_RESET_INTERFACE
----------------------

This parameter specifies the 'Reset Output' interface of the QSys module
that supplies the system reset.

Category
+++++++++

Board and Device Specification

Value Type
+++++++++++

String

Valid Values
+++++++++++++

The 'Reset Output' interface of the QSys module that supplies the system reset.

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

Related parameters: :ref:`SYSTEM_RESET_MODULE`

Applicable Flows
+++++++++++++++++

Software and hybrid flows

Test Status
++++++++++++

Actively in-use

Examples
+++++++++

``set_parameter SYSTEM_RESET_INTERFACE clk_reset``

.. image:: images/SYSTEM_images/clk_clk_reset.png

--------------------------------------------------------------------------------
