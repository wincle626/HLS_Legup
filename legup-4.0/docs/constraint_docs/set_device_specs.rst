
.. _set_device_specs:

set_device_specs
----------------

This option is used to define certain device-specific parameters.
All parameters are required.
The parameters are described in the following table:

+-------------+---------------------------------------------------------------+
| Parameter   | Description                                                   |
+=============+===============================================================+
| -Family     | The device family                                             |
+-------------+---------------------------------------------------------------+
| -Device     | The device number                                             |
+-------------+---------------------------------------------------------------+
| -MaxALMs    | The total number of ALMs in the device                        |
+-------------+---------------------------------------------------------------+
| -MaxM4Ks    | The total number of M4K memory blocks in the device           |
+-------------+---------------------------------------------------------------+
| -MaxRAMBits | The total number of RAM bits that can be stored in the device |
+-------------+---------------------------------------------------------------+
| -MaxDSPs    | The total number of DSP blocks in the device                  |
+-------------+---------------------------------------------------------------+

You should not need to use ``set_device_specs`` unless you are adding support
for a new board or device.

Category
+++++++++

Board and Device Specification

Value Type
+++++++++++

+-------------+------------+
| Parameter   | Value type |
+=============+============+
| -Family     | String     |
+-------------+------------+
| -Device     | String     |
+-------------+------------+
| -MaxALMs    | Integer    |
+-------------+------------+
| -MaxM4Ks    | Integer    |
+-------------+------------+
| -MaxRAMBits | Integer    |
+-------------+------------+
| -MaxDSPs    | Integer    |
+-------------+------------+

Valid Values
+++++++++++++

See Examples

Default Value
++++++++++++++

N/A

Location Where Default is Specified
+++++++++++++++++++++++++++++++++++

``boards/<device_family>/<board>/<board>.tcl``

e.g.: ``boards/CycloneV/DE1-SoC/DE1-SoC.tcl``

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

``set_device_specs -Family CycloneV -Device 5CSEMA5F31C6 -MaxALMs 32075 -MaxM4Ks 397 -MaxRAMBits 4065280 -MaxDSPs 87``

``set_device_specs -Family StratixV -Device 5SGXEA7N2F45C2 -MaxALMs 234720 -MaxM4Ks 2560 -MaxRAMBits 520428800 -MaxDSPs 256``


--------------------------------------------------------------------------------

