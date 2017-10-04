.. _INSPECT_DEBUG_DB_SCRIPT_FILE:

INSPECT_DEBUG_DB_SCRIPT_FILE
----------------------------

Path to script file for creating MySQL debug database used by Inspect debugger.

Category
+++++++++

Debugging

Value Type
+++++++++++

String

.. Valid Values
.. +++++++++++++


Default Value
++++++++++++++

examples/inspect_db.sql

Location Where Default is Specified
+++++++++++++++++++++++++++++++++++

``examples/legup.tcl``

Dependencies
+++++++++++++

:ref:`INSPECT_DEBUG`

Related parameters:
:ref:`INSPECT_ONCHIP_BUG_DETECT_DEBUG`, :ref:`DEBUG_DB_HOST`, :ref:`DEBUG_DB_USER`, :ref:`DEBUG_DB_PASSWORD`, :ref:`INSPECT_DEBUG_DB_NAME`

Applicable Flows
+++++++++++++++++

Inspect debugger for pure hardware

Test Status
++++++++++++

Prototype functionality

Examples
+++++++++

    ``set_parameter INSPECT_DEBUG_DB_SCRIPT_FILE $::CURRENT_PATH/inspect_db.sql``

--------------------------------------------------------------------------------

