.. _DEBUG_DB_PASSWORD:

DEBUG_DB_PASSWORD
-----------------

Password for MySQL debug database. Used by both Inspect and Python Debugger.

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

letmein

Location Where Default is Specified
+++++++++++++++++++++++++++++++++++

``examples/legup.tcl``

Dependencies
+++++++++++++

:ref:`INSPECT_DEBUG` for Inspect debugger. None for Python debugger.

Related parameters:
:ref:`DEBUG_DB_HOST`, :ref:`DEBUG_DB_USER`
For Python debugger:
:ref:`DEBUG_DB_NAME`, :ref:`DEBUG_DB_SCRIPT_FILE`
For Inspect debugger:
:ref:`INSPECT_DEBUG_DB_NAME`, :ref:`INSPECT_DEBUG_DB_SCRIPT_FILE`

Applicable Flows
+++++++++++++++++

Pure hardware, Inspect debugger for pure hardware

.. Test Status
.. ++++++++++++


Examples
+++++++++

    ``set_parameter DEBUG_DB_PASSWORD letmein``

--------------------------------------------------------------------------------

