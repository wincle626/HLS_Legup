.. _NO_INLINE:

NO_INLINE
-------------

This is a Makefile parameter that can disable the LLVM compiler from inlining
functions. Note that all compiler optimizations will be turned off when
``NO_INLINE`` is enabled.
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

    ``NO_INLINE=1``

--------------------------------------------------------------------------------

