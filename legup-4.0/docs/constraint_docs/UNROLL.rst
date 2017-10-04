.. _UNROLL:

UNROLL
-------------

This is a Makefile parameter that allows user to specify additional flags
related to the unroll transformation in LLVM compiler.
This parameter can be set in ``examples/Makefile.config`` or in a local Makefile.
Please see example settings in ``examples/Makefile.config``.

Category
+++++++++

LLVM

Value Type
+++++++++++

string

Applicable Flows
+++++++++++++++++

All devices and flows

Test Status
++++++++++++

Actively in-use

Examples
+++++++++

    ``UNROLL = -unroll-allow-partial -unroll-threshold=1000``

--------------------------------------------------------------------------------

