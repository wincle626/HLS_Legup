.. _INSPECT_ONCHIP_BUG_DETECT_DEBUG:

INSPECT_ONCHIP_BUG_DETECT_DEBUG
-------------

Creates partial debugging information for Inspect Debugger. This parameter should only be set when Inspect ON_CHIP_BUG_DETECT mode is required. Requires INSPECT_DEBUG parameter to be enabled.

Category
+++++++++

Debugging

Value Type
+++++++++++

Integer

Valid Values
+++++++++++++

0, 1

Default Value
++++++++++++++

0

Location Where Default is Specified
+++++++++++++++++++++++++++++++++++

``examples/legup.tcl``

Dependencies
+++++++++++++

:ref:`INSPECT_DEBUG`

Applicable Flows
+++++++++++++++++

Inspect debugger for pure hardware

Test Status
++++++++++++

Prototype functionality

Examples
+++++++++

    ``set_parameter INSPECT_ONCHIP_BUG_DETECT_DEBUG 1``

--------------------------------------------------------------------------------

