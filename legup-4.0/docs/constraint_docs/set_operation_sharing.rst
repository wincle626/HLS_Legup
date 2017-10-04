
.. _set_operation_sharing:

set_operation_sharing
----------------

This parameter allows operation sharing to be turned on or off for a given
operation.
Operation sharing is on by default for all operations.

Operation sharing adds multiplexing before and after an operation (e.g. divider)
so that it can be used for multiple operations, instead of creating duplicate
hardware.

Note: A constraint on "signed_add" will apply to:
  - **signed_add**\ _8
  - **signed_add**\ _16
  - **signed_add**\ _32
  - **signed_add**\ _64
  - un\ **signed_add**\ _8
  - un\ **signed_add**\ _16
  - un\ **signed_add**\ _32
  - un\ **signed_add**\ _64
  - ...

Category
+++++++++

Constraints

Value Type
+++++++++++

String

Valid Values
+++++++++++++

-on <operation>, -off <operation>

Default Value
++++++++++++++

unset ('-on' by default for all operations)

Location Where Default is Specified
+++++++++++++++++++++++++++++++++++

By default, operation sharing is enbled for all operations in the LegUp source
code.

A few operations are cheaper than multiplexing, so operation sharing is turned
off for them in the file:

``examples/legup.tcl``

Dependencies
+++++++++++++

None

Related parameters: :ref:`RESTRICT_TO_MAXDSP`

Applicable Flows
+++++++++++++++++

All devices and flows

Test Status
++++++++++++

Actively in-use

Examples
+++++++++

``set_operation_sharing -off signed_add``

``set_operation_sharing -off signed_subtract``


--------------------------------------------------------------------------------

