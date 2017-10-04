.. _loop_pipeline:

loop_pipeline
-------------

This parameter enables pipelining for a given loop in the code.
Loop pipelining allows a new iteration of the loop to begin before the current one has completed, achieving higher throughput.
In a loop nest, only the innermost loop can be pipelined.
Optional arguments:

+--------------------+-----------------------------------------------------------------------+
| Parameter          | Description                                                           |
+====================+=======================================================================+
| -ii                | Force a specific pipeline initiation interval                         |
+--------------------+-----------------------------------------------------------------------+
| -ignore-mem-deps   | Ignore loop carried dependencies for all memory accesses in the loop  |
+--------------------+-----------------------------------------------------------------------+

Category
+++++++++

HLS Constraints

Value Type
+++++++++++

+--------------------+------------+
| Parameter          | Value Type |
+====================+============+
| loop_pipeline      | String     |
+--------------------+------------+
| -ii                | Integer    |
+--------------------+------------+
| -ignore-mem-deps   | None       |
+--------------------+------------+

Valid Values
+++++++++++++

See Examples

Default Value
++++++++++++++

N/A

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

``loop_pipeline "loop1"``

``loop_pipeline "loop2" -ii 1``

``loop_pipeline "loop3" -ignore-mem-deps``

--------------------------------------------------------------------------------

