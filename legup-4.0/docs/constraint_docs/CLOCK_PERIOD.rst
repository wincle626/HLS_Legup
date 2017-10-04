.. _CLOCK_PERIOD

CLOCK_PERIOD
-------------

This is a widely used constraint that allows the user to set the
target clock period for a design.  The clock period is specified in
nanoseconds.

It has a significant impact on scheduling: the scheduler will schedule
operators into clock cycles using delay estimates for each operator,
such that the specified clock period is honored.  In other words,
operators will be chained together combinationally to the extent
allowed by the value of the CLOCK_PERIOD parameter.

LegUp has a default CLOCK_PERIOD value for each device family that is
supported.  That default value was chosen to minimize the wall-clock
time for a basket of benchmark programs (mainly the CHStone benchmark
circuits).

If the parameter SDC_NO_CHAINING is 1, then the CLOCK_PERIOD parameter
has no effect.

Category
+++++++++

HLS Constraints

Value Type
+++++++++++

Integer represent a value in nanoseconds

Valid Values
+++++++++++++

Integer

Default Value
++++++++++++++

Depends on the target device

Location Where Default is Specified
+++++++++++++++++++++++++++++++++++

``boards/CycloneII/CycloneII.tcl``

``boards/StratixIV/StratixIV.tcl``

and so on...

Dependencies
+++++++++++++

SDC_NO_CHAINING: If this parameter is set to 1, then CLOCK_PERIOD does
nothing.

Applicable Flows
+++++++++++++++++

All devices and flows

Test Status
++++++++++++

Actively in-use

Examples
+++++++++

    ``set_parameter CLOCK_PERIOD 15``

--------------------------------------------------------------------------------

