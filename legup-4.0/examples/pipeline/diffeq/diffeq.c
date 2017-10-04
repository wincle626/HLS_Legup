#include <stdio.h>

// benchmark from:
// P.G. Paulin, J.P. Knight, and E.F. Girczyc, "HAL: A Multi-Paradigm Approach
// to Automatic Data Path Synthesis," DAC, pp. 266, 1986. 
//
// Solve the second order differential equation:
//      y'' + 5xy' + 3y = 0
// given some initial conditions on the x, y, and y'
// we must incrementally find the y value at a given x value greater than the
// initial conditions
//
// This might be used to describe a subsytem of a controller or have a digital
// signal processing application
//
// first create a new variable for the first derivative:
//      u = y' 
//        = dy/dx
// then incrementally solve by increasing x by dx:
//      x = x + dx
// the new y will be:
//      y = y + dy/dx * dx 
//        = y + u * dx
// the new u will be:
//      u = u + y'' * dx
// we know from the differential equation that:
//      y'' = -5xy' - 3y
// therefore:
//      u = u + (-5xy' - 3y) * dx
//        = u - 5x*u*dx - 3y*dx
//
// keep incrementing x until we reach the desired value
//

int diffEqnSolver(int a, int dx, int x0, int u0, int y0) {
    int x1, u1, y1;
    int x, u, y;

    x = x0;
    u = u0;
    y = y0;

    loop: while (x < a) {
        x1 = x + dx;
        u1 = u - 5*x*u*dx - 3*y*dx;
        y1 = y + u*dx;
        x = x1;
        u = u1;
        y = y1;
        //printf("step x=%d\n", x);
        //printf("step x=%u,y=%u,u=%u\n", x,y,u);
    }

    printf("y = %d\n", y);
    return y;
}

int main() {

    // assume a step size of 1
    int dx = 1;

    // inputs
    volatile int a, x0, u0, y0;

    // the x value at which we want the y value of the function
    a = 50;
    // initial conditions
    x0 = 2;
    u0 = 1;
    y0 = 1;

    int y = diffEqnSolver(a, dx, x0, u0, y0);

    if (y == 1461495984) {
        printf("PASSED\n");
    } else {
        printf("FAILED\n");
    }

    return y;
}
