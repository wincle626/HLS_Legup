.. _set_accelerator_function:

set_accelerator_function
-------------

This sets the C function to be accelerated to HW in the hybrid flow. 
It can be used on one or more functions. 

Category
+++++++++

HLS Constraints

Value Type
+++++++++++

String

Valid Values
+++++++++++++

Name of the function

Default Value
++++++++++++++

NULL

Location Where Default is Specified
+++++++++++++++++++++++++++++++++++

N/A

Dependencies
+++++++++++++

None

Applicable Flows
+++++++++++++++++

Hybrid flow

Test Status
++++++++++++

Actively in-use

Examples
+++++++++

      ``set_accelerator_function "add"``

      ``set_accelerator_function "div"``
    

--------------------------------------------------------------------------------

