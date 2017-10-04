.. _DEBUG_DB_HOST:

DEBUG_DB_HOST
-------------

Hostname for MySQL debug database. Used by both Inspect and Python Debugger.

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

localhost

Location Where Default is Specified
+++++++++++++++++++++++++++++++++++

``examples/legup.tcl``

Dependencies
+++++++++++++

:ref:`INSPECT_DEBUG` for Inspect debugger. None for Python debugger.

Related parameters:
:ref:`DEBUG_DB_USER`, :ref:`DEBUG_DB_PASSWORD`
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

    ``set_parameter DEBUG_DB_HOST localhost``

--------------------------------------------------------------------------------

