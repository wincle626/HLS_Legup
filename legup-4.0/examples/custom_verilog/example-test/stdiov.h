// Standard Board I/O
// Author: Mathew Hall
// Date: May 23, 2014

#include <stdio.h>

int __attribute__ ((noinline)) __attribute__ ((used)) boardPutChar(int character) {
    printf("x");
    return 0;//putchar(character);
}

int __attribute__ ((noinline)) __attribute__ ((used)) boardGetChar(void) {
    printf("y");
    return 0;
}

#define putchar(character) boardPutChar(character)
#define getchar() boardGetChar()
