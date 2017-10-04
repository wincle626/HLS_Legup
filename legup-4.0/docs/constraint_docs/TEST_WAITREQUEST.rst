.. _TEST_WAITREQUEST:

TEST_WAITREQUEST
-------------

Test handling of the waitrequest signal by setting waitrequest high of 5 cycles
at the beginning of every transfer. This should have no impact on circuit 
functionality but just make the total cycle count about 5x higher.

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

unset (0)

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

    ``set_parameter TEST_WAITREQUEST 1``

--------------------------------------------------------------------------------

