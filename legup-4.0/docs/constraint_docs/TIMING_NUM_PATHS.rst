.. _TIMING_NUM_PATHS:

TIMING_NUM_PATHS
----------------

This parameter defines the number of paths to be printed in the timing reports
``timingReport.legup.rpt`` and ``timingReport.overall.legup.rpt`` when LegUp HLS
is run.  The timing reports will contain the TIMING_NUM_PATHS longest paths.

Category
+++++++++

Miscellaneous

Value Type
+++++++++++

Integer

Valid Values
+++++++++++++

Positive integers

Default Value
++++++++++++++

10

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

``set_parameter TIMING_NUM_PATHS 20``

--------------------------------------------------------------------------------

