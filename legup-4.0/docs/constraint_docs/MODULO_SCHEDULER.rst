.. _MODULO_SCHEDULER:

MODULO_SCHEDULER
-----------------

This parameter specifies the type of modulo scheduler to use for loop pipelining. There are
three options:
  SDC_BACKTRACKING
      Default. Used SDC modulo scheduling with a backtracking mechanism to
      resolve conflicting resource and recurrence constraints
  SDC_GREEDY
      SDC modulo scheduling using a greedy approach to resolving resource
      constraints. This method may not achieve minimum II II for loops with
      resource constraints and cross-iteration dependencies.
  ITERATIVE
      Classic iterative modulo scheduler approach using a list scheduler (no
      operator chaining support)

Category
+++++++++

HLS Constraints

Value Type
+++++++++++

String

Valid Values
+++++++++++++

SDC_BACKTRACKING, SDC_GREEDY, ITERATIVE

Default Value
++++++++++++++

SDC_BACKTRACKING

Location Where Default is Specified
+++++++++++++++++++++++++++++++++++

``examples/legup.tcl``

Dependencies
+++++++++++++

None

Applicable Flows
+++++++++++++++++

All devices and flows

Test Status
++++++++++++

Actively in-use

Examples
+++++++++

    ``set_parameter MODULO_SCHEDULER SDC_BACKTRACKING``
    ``set_parameter MODULO_SCHEDULER SDC_GREEDY``
    ``set_parameter MODULO_SCHEDULER ITERATIVE``

--------------------------------------------------------------------------------

