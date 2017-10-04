.. _NO_OPT:

NO_OPT
-------------

This is a Makefile parameter that disables LLVM optimizations, which is
equivalent to the ``-O0`` flag.
This parameter can be set in ``examples/Makefile.config`` or in a local Makefile.

Category
+++++++++

LLVM

Value Type
+++++++++++

Integer

Valid Values
+++++++++++++

0, 1

Default Value
++++++++++++++

unset (0)

Applicable Flows
+++++++++++++++++

All devices and flows

Test Status
++++++++++++

Actively in-use

Examples
+++++++++

    ``NO_OPT=1``

--------------------------------------------------------------------------------

