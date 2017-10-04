.. _SYSTEM_CLOCK_INTERFACE:

SYSTEM_CLOCK_INTERFACE
----------------------

This parameter specifies the 'Clock Output' interface of the QSys module
that provides the system clock.

Category
+++++++++

Board and Device Specification

Value Type
+++++++++++

String

Valid Values
+++++++++++++

The 'Clock Output' interface of the QSys module that supplies the system clock.

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

Related parameters: :ref:`SYSTEM_CLOCK_MODULE`

Applicable Flows
+++++++++++++++++

Software and hybrid flows

Test Status
++++++++++++

Actively in-use

Examples
+++++++++

``set_parameter SYSTEM_CLOCK_INTERFACE clk``

.. image:: images/SYSTEM_images/clk_clk.png

--------------------------------------------------------------------------------
