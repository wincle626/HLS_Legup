.. _set_resource_constraint:

set_resource_constraint
-----------------------

This parameter constrains the number of times a given operation can occur in a cycle.
 
Note: A constraint on "signed_add" will apply to:
  - signed_add_8
  - signed_add_16
  - signed_add_32
  - signed_add_64
  - unsigned_add_8
  - unsigned_add_16
  - unsigned_add_32
  - unsigned_add_64

Category
+++++++++

HLS Constraints

Value Type
+++++++++++

<operation> integer

Valid Values
+++++++++++++

See Default and Examples
Note: operator name should match the device family operation database file:
boards/StratixIV/StratixIV.tcl or boards/CycloneII/CycloneII.tcl

Default Values
++++++++++++++

::

    mem_dual_port 2
    divide 1
    modulus 1
    multiply 2
    altfp_add 1
    altfp_subtract 1
    altfp_multiply 1
    altfp_divide 1
    altfp 1

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

``set_resource_constraint signed_divide_16 3``

``set_resource_constraint signed_divide 2``

``set_resource_constraint divide 1``

--------------------------------------------------------------------------------

