.. _DEBUG_DB_SCRIPT_FILE:

DEBUG_DB_SCRIPT_FILE
--------------------

Path to script file for creating MySQL debug database used by Python debugger.

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

examples/createDebugDB.sql

Location Where Default is Specified
+++++++++++++++++++++++++++++++++++

``examples/legup.tcl``

Dependencies
+++++++++++++

Related parameters:
:ref:`DEBUG_DB_HOST`, :ref:`DEBUG_DB_USER`, :ref:`DEBUG_DB_PASSWORD`, :ref:`DEBUG_DB_NAME`

Applicable Flows
+++++++++++++++++

Pure hardware

Test Status
++++++++++++

Prototype functionality

Examples
+++++++++

    ``set_parameter DEBUG_DB_SCRIPT_FILE $::CURRENT_PATH/createDebugDB.sql``

--------------------------------------------------------------------------------

