.. _LLVM_PROFILE:

LLVM_PROFILE
-------------

Enabling this parameter will allow the scheduler to modify scheduling based on
the llvm profiling information. To enable this option, ``LLVM_PROFILE=1`` should
be set in ``Makefile.config`` and ``set_parameter LLVM_PROFILE 1`` should be
set in ``config.tcl`` or ``legup.tcl``.

With ``LLVM_PROFILE`` enabled, the schedules will be modified for the
infrequently used basic blocks, which the basic blocks executed at or below the
frequency threshold ``LLVM_PROFILE_MAX_BB_FREQ_TO_ALTER``.
This value can be set via 
    ``set_parameter LLVM_PROFILE_MAX_BB_FREQ_TO_ALTER $(USER_DEFINED_FREQUENCY)``.

Two options are provided to further specify how the scheduling should be
modified. The first option is to extend paths in infrequently executed BBs by a
fixed number of states, which can be set with 
    ``set_parameter LLVM_PROFILE_EXTRA_CYCLES $(USER_DEFINED_CYCLES)``.
Note that this change can only be applied to multi-cycle paths thus the
parameter ``MULTI_CYCLE_REMOVE_REG`` must be set.

The second option is to adjust the target period for the infrequently executed
basic blocks.
As an alternative to ``LLVM_PROFILE_EXTRA_CYCLES`` which simply extends paths
in infrequent basic blocks by a fixed amount, ``LLVM_PROFILE_PERIOD_<DEVICE>``
can be used to change the target period for paths in infrequent blocks
(e.g. a 13ns constraint on Cyclone II might be the best, but infrequent blocks
can use a 6ns constraint instead). This target frequency parameter is set via
a FPGA device-dependent parameter, either ``LLVM_PROFILE_PERIOD_CII`` (for
Cyclone II) or ``LLVM_PROFILE_PERIOD_SIV`` (for Stratix IV).

Category
+++++++++

HLS Constraints.

Value Type
+++++++++++

Integer

Valid Values
+++++++++++++

| ``LLVM_PROFILE``: 0, 1
| ``LLVM_PROFILE_MAX_BB_FREQ_TO_ALTER``: positive integers
| ``LLVM_PROFILE_EXTRA_CYCLES``: positive integers
| ``LLVM_PROFILE_PERIOD_CII``: positive integers
| ``LLVM_PROFILE_PERIOD_SIV``: positive integers

Default Value
++++++++++++++

0 for LLVM_PROFILE.

Location Where Default is Specified
+++++++++++++++++++++++++++++++++++

``examples/legup.tcl``
``examples/Makefile.config``

Dependencies
+++++++++++++

``MULTI_CYCLE_REMOVE_REG``

Applicable Flows
+++++++++++++++++

All devices and flows

Test Status
++++++++++++

Prototype functionality

Examples
+++++++++

  Enable scheduling modification based on profiling information,
    ``set_parameter PRINT_BB_STATS LLVM_PROFILE`` in ``config.tcl``
    ``LLVM_PROFILE=1`` in ``Makefile.config``

  Specify the maximum execution frequency for infrequent basic blocks,
    ``set LLVM_PROFILE_MAX_BB_FREQ_TO_ALTER 100``

  With ``set_parameter MULTI_CYCLE_REMOVE_REG 1`` enabled, paths in infrequent
  executed basic block may be,
  
  1) extended for 2 cycles,
    ``set LVM_PROFILE_EXTRA_CYCLES 2``

  2) or be extended to match a more constrainted clock period,
  say 13ns for Cyclone II or 6ns for Stratix IV.
   | ``set LLVM_PROFILE_PERIOD_CII 13``
   | ``set LLVM_PROFILE_PERIOD_SIV 6``

--------------------------------------------------------------------------------

