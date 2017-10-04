
.. _set_operation_attributes:

set_operation_attributes
----------------

This option is used to define certain device-specific parameters for a given 
operation.
For example, ``set_operation_attributes`` can be used to set the fmax, latency,
and resource usage for a pipelined divider on a Cyclone V device.
All parameters are required.
The parameters are described in the following table:

+---------------+--------------------------------------------+
| Parameter     | Description                                |
+===============+============================================+
| -Name         | Operation name                             |
+---------------+--------------------------------------------+
| -Fmax         | Maximum frequency for operation            |
+---------------+--------------------------------------------+
| -CritDelay    | Input to output delay for operation        |
+---------------+--------------------------------------------+
| -StaticPower  | Static power for operation                 |
+---------------+--------------------------------------------+
| -DynamicPower | Dynamic power for operation                |
+---------------+--------------------------------------------+
| -LUTs         | Number of LUTs used by operation           |
+---------------+--------------------------------------------+
| -Registers    | Number of registers used by operation      |
+---------------+--------------------------------------------+
| -LEs          | Number of logic elements used by operation |
+---------------+--------------------------------------------+
| -DSP          | Number of DSP blocks used by operation     |
+---------------+--------------------------------------------+
| -Latency      | Cycles of latency for operation            |
+---------------+--------------------------------------------+

Note: You should not have to use ``set_operation_attributes`` unless you are
adding support for a new board or operation.

Category
+++++++++

Board and Device Specification

Value Type
+++++++++++

+---------------+------------+
| Parameter     | Value Type |
+===============+============+
| -Name         | String     |
+---------------+------------+
| -Fmax         | Real       |
+---------------+------------+
| -CritDelay    | Real       |
+---------------+------------+
| -StaticPower  | Real       |
+---------------+------------+
| -DynamicPower | Real       |
+---------------+------------+
| -LUTs         | Integer    |
+---------------+------------+
| -Registers    | Integer    |
+---------------+------------+
| -LEs          | Integer    |
+---------------+------------+
| -DSP          | Integer    |
+---------------+------------+
| -Latency      | Integer    |
+---------------+------------+

Valid Values
+++++++++++++

See Examples

Default Value
++++++++++++++

N/A

Location Where Default is Specified
+++++++++++++++++++++++++++++++++++

``boards/<device_family>/<device_family>.tcl``

e.g.: ``boards/CycloneV/CycloneV.tcl``

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

``set_operation_attributes -Name signed_add_16 -Fmax 471.48 -CritDelay 2.121 -StaticPower 0.00 -DynamicPower 0.00 -LUTs 16 -Registers 48 -LEs 24 -DSP 0 -Latency 0``

``set_operation_attributes -Name signed_multiply_64 -Fmax 112.3 -CritDelay 8.905 -StaticPower 0.00 -DynamicPower 0.00 -LUTs 62 -Registers 192 -LEs 100 -DSP 6 -Latency 0``

``set_operation_attributes -Name unsigned_multiply_pipelined_4_64 -Fmax 114.5 -CritDelay 8.734 -StaticPower 0.00 -DynamicPower 0.00 -LUTs 57 -Registers 512 -LEs 260 -DSP 6 -Latency 4``


--------------------------------------------------------------------------------

