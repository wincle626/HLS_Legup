.. _MULT_BY_CONST_INFER_DSP:

MULT_BY_CONST_INFER_DSP
------------------------

This parameter assumes that all 

LegUp detects whether multiply-by-constant operations will infer DSPs.
For instance, x * 10 = x * (2^3 + 2) = (x << 3) + (x << 1)
Therefore, the final circuit will not require a DSP block for this multiplier.

In general, Quartus will not infer if a multiply by constant
can be replaced by shifts (which are free) and at most one addition
to see if this is possible:
1) calculate the closest power of two
2) can get there by adding/subtracting x or (x << n)?

This impacts binding because we don't want to share a multiply operation that requires
DSP blocks with a multiply that can be done with shifts/adds.

This parameter turns off this detection and assumes at all multiply-by-constant operations
will infer DSP blocks.

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

None

Applicable Flows
+++++++++++++++++

All devices and flows

Test Status
++++++++++++

Untested

Examples
+++++++++

    ``set_parameter MULT_BY_CONST_INFER_DSP 1``

--------------------------------------------------------------------------------

