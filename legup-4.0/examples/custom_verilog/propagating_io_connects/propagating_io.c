// Propagating IO
// Author: Mathew Hall
// Date: May 8, 2014

#include <stdio.h>
#include <unistd.h>

/* ABOUT
 *
 * assignSwitchesToLEDs and assignSwitchesToLEDsInverted
 * are custom verilog functions that we have implemented
 * in verilog in the propagating_io.v file.  They assign
 * LEDs to switches asynchronously to demonstrate and
 * test the functionality of the custom verilog and
 * propagating signal functionality in LegUp.
 *
 * This example also uses a custom testbench.  You can
 * find the testbench for this example in testbench.v
 *
 */

void __attribute__((noinline)) assignSwitchesToLEDs(void) {

    // Printf in this function is required to prevent
    // clang from inlining custom verilog functions
    //
    // As an alternative, you could implement the C
    // equivalent of your custom verilog modules
    // to ensure that your program functions in the
    // same way when it is being run on a CPU as it
    // does when it is synthesized to hardware.
    //
    printf("Hello World");
}

void __attribute__((noinline)) assignSwitchesToLEDsInverted(void) {

    // Printf in this function is required to prevent
    // clang from inlining custom verilog functions
    //
    printf("dlroW olleH");
}

int main(void) {

    // The entire functionality for this
    // program is described in the
    // propagating_io.v file.  The following
    // function call tells LegUp where and
    // how to instantiate our custom module.
    //
    // You can also pass values as arguments to
    // custom verilog modules if you follow the
    // LegUp function call protocol.  For more
    // information about this, please read the
    // custom verilog tutorial.
    //
    assignSwitchesToLEDs();
    assignSwitchesToLEDsInverted();
    return 0;
}
