.. _INSPECT_DEBUG_DB_NAME:

INSPECT_DEBUG_DB_NAME
---------------------

MySQL debug database name. Used by Inspect debugger.

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

inspect_db

Location Where Default is Specified
+++++++++++++++++++++++++++++++++++

``examples/legup.tcl``

Dependencies
+++++++++++++

:ref:`INSPECT_DEBUG`

Related parameters:
:ref:`INSPECT_ONCHIP_BUG_DETECT_DEBUG`, :ref:`DEBUG_DB_HOST`, :ref:`DEBUG_DB_USER`, :ref:`DEBUG_DB_PASSWORD`, :ref:`INSPECT_DEBUG_DB_SCRIPT_FILE`

Applicable Flows
+++++++++++++++++

Inspect debugger for pure hardware

Test Status
++++++++++++

Prototype functionality

Examples
+++++++++

    ``set_parameter INSPECT_DEBUG_DB_NAME inspect_db``

--------------------------------------------------------------------------------

