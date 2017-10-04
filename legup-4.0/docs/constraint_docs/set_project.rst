.. _set_project:

set_project
-----------

This parameter sets the default target project, or device, used. Changing the project also updates the associated family and board parameters.

Category
+++++++++

Board and Device Specification

Value Type
+++++++++++

String

Valid Values
+++++++++++++

See Examples

Default Value
++++++++++++++

``CycloneV DE1-SoC Tiger_SDRAM``

Location Where Default is Specified
+++++++++++++++++++++++++++++++++++

``examples/legup.tcl``

Dependencies
+++++++++++++

None

Applicable Flows
+++++++++++++++++

All devices and flows

Test Status
++++++++++++

Actively in-use

Examples
+++++++++

``set_project CycloneV DE1-SoC Tiger_SDRAM``

``set_project CycloneV DE1-SoC ARM_Simple_Hybrid_System``

``set_project CycloneV SoCKit ARM_Simple_Hybrid_System``

``set_project CycloneIV DE2-115 Tiger_SDRAM``

``set_project CycloneII DE2 Tiger_SDRAM``

``set_project CycloneII CycloneIIAuto Tiger_SDRAM``

``set_project StratixV DE5-Net Tiger_DDR3``

``set_project StratixIV DE4-230 Tiger_DDR2``

``set_project StratixIV DE4-530 Tiger_DDR2``

--------------------------------------------------------------------------------

