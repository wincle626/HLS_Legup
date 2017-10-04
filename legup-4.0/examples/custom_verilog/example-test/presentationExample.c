// Presentation Example
// Author: Mathew Hall
// Date: May 27, 2014

#include "stdiov.h"
//#include <stdio.h>

#define BUFFER_SIZE 10000

void __attribute__ ((noinline)) __attribute__ ((used)) updateCharactersEntered(int charactersEntered) {

    printf("Called updateCharactersEntered(int)\n");

}

void __attribute__ ((noinline)) __attribute__ ((used)) echoInput(void) {

    while (1) {
	putchar(getchar());
    }
}

int main(void) {

    echoInput();
    return 0;

}
