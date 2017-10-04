// Light Display Tutorial
// Author: Mathew Hall
// Date: June 20, 2014

#include <stdio.h>
#include <unistd.h>

// We will use 32 bit integers to set values to the LEDs on the board.
// This tutorial was designed for an Altera DE2 with 18 red LEDs.
// The least significant 18 bits in our integers will represent the
// state of each of the LEDs.  For example, if our integer is:
//
//     Integer:    32'b00000000000000000000000000100000
//     Led Number:                        ...9876543210
//
// Then we want LEDR[5] to be on and all other LEDs to be off.
//
// The following constants represent the highest and lowest integers
// we can use to represent a particular LED pattern.
//
const int LED_MAX = 0x00020000; //  LEDR[17]
const int LED_MIN = 0x00000001; //  LEDR[0]

// We will cycle our pattern in two directions
const enum Direction {
    left,
    right
};

int shiftPattern(int, char *);
void printToLEDs(int);

int main() {

    int pattern = 1;
    char direction = left;

    while (1) {
	pattern = shiftPattern(pattern, &direction);
    }
    return 0;
}

int shiftPattern(int previous, char *direction) {
    int new;
    
    if (*direction == left) {
	if (!(previous & LED_MAX) ) {
	shift_left:
	    new = previous << 1;
	}
	else {
	    *direction = right;
	    goto shift_right;
	}
    }
    else {
	if (!(previous & LED_MIN) ) {
	shift_right:
	    new = previous >> 1;
	}
	else {
	    *direction = left;
	    goto shift_left;
	}
    }
    
    return new;
}
