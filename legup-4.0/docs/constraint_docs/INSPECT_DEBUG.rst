.. _INSPECT_DEBUG:

INSPECT_DEBUG
-------------

Enables Inspect Debugger functionality and populates database. Creates full debugging information for hardware during code generation for the Inspect Debugger.

Category
+++++++++

Debugging

Value Type
+++++++++++

Integer

Valid Values
+++++++++++++

0, 1

Default Value
++++++++++++++

0

Location Where Default is Specified
+++++++++++++++++++++++++++++++++++

``examples/legup.tcl``

Dependencies
+++++++++++++

None

Related parameters:
:ref:`DEBUG_DB_HOST`, :ref:`DEBUG_DB_USER`, :ref:`DEBUG_DB_PASSWORD`, :ref:`INSPECT_DEBUG_DB_NAME`, :ref:`INSPECT_DEBUG_DB_SCRIPT_FILE`

Applicable Flows
+++++++++++++++++

Inspect debugger for pure hardware

Test Status
++++++++++++

Prototype functionality

Examples
+++++++++

    ``set_parameter INSPECT_DEBUG 1``

--------------------------------------------------------------------------------

