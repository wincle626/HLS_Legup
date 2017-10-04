.. _MULTIPUMPING:

MULTIPUMPING
-------------

This parameter controls whether LegUp multi-pumps multipliers to save DSP blocks
and achieve better performance. For details see:
Andrew Canis, Stephen D. Brown, and Jason H. Anderson, "Multi-Pumping for
Resource Reduction in FPGA High-Level Synthesis," Design, Automation, and Test
in Europe (DATE). Grenoble, France, March, 2013. (PDF) 

Category
+++++++++

HLS Constraints

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

None

Applicable Flows
+++++++++++++++++

All devices and flows

Test Status
++++++++++++

Actively in-use (examples/multipump)

Examples
+++++++++

    ``set_parameter MULTIPUMPING 1``

--------------------------------------------------------------------------------

