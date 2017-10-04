.. _SYSTEM_DATA_CACHE_TYPE:

SYSTEM_DATA_CACHE_TYPE
----------------------

This parameter specifies the type of cache used in the QSys system.

Category
+++++++++

Board and Device Specification

Value Type
+++++++++++

String

Valid Values
+++++++++++++

Any QSys component that functions as a cache.

Default Value
++++++++++++++

N/A

Location Where Default is Specified
+++++++++++++++++++++++++++++++++++

``boards/<device_family>/<board>/<system>/legup.tcl``

e.g. ``boards/CycloneV/DE1-SoC/Tiger_SDRAM/legup.tcl``

Dependencies
+++++++++++++

None

Applicable Flows
+++++++++++++++++

Software and hybrid flows

Test Status
++++++++++++

Actively in-use

Examples
+++++++++

``set_parameter SYSTEM_DATA_CACHE_TYPE legup_dm_wt_cache``

--------------------------------------------------------------------------------
