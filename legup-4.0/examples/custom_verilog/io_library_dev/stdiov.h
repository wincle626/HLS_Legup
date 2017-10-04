// Standard Board I/O
// Author: Mathew Hall
// Date: May 23, 2014

#include <stdio.h>

int __attribute__ ((noinline)) __attribute__ ((used)) boardPutChar(int character) {
    printf("x");
    return 0;//putchar(character);
}

#define putchar(character) boardPutChar(character)
