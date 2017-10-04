// Standard Board I/O
// Author: Mathew Hall
// Date: May 23, 2014

#include <stdio.h>
#include "clang_stdarg.h"

extern char *itoa(int, char *, int);

volatile int __attribute__ ((noinline)) __attribute__ ((used)) boardPutChar(int character) {
    volatile int i = character;
    return putchar(character);
}

volatile int __attribute__ ((noinline)) __attribute__ ((used)) boardGetChar(void) {
    volatile int i = 5;
    return getchar();

}
#define putchar(character) boardPutChar(character)
#define getchar() boardGetChar()

int boardPutString(const char * str) {
    
    int i = 0;
    while (str[i++] != 0) {

	putchar(str[i]);

    }

    return i;

}

int boardPuts (const char * str) {

    int i = boardPutString(str);
    putchar('\n');

    return i;

}

#define puts(character) boardPuts(character)

void __attribute__ ((noinline)) __attribute__ ((used)) boardPrintf(const char *format, ...) {

    va_list argList;
    int integer;
    char *string;
    char formatBuffer[256];
    const char *toPrint;
    
    va_start(argList, format);
    
    for(toPrint = format; *toPrint != '\0'; toPrint++)
	{
	    if(*toPrint != '%')
		{
		    putchar(*toPrint);
		    continue;
		}
	    
	    switch(*++toPrint)
		{
		case 'c':
		    integer = va_arg(argList, int);
		    putchar(integer);
		    break;
		    
		case 'd':
		    integer = va_arg(argList, int);
		    string = itoa(integer, formatBuffer, 10);
		    boardPutString(string);
		    break;
		    
		case 's':
		    string = va_arg(argList, char *);
		    boardPutString(string);
		    break;
		    
		case 'x':
		    integer = va_arg(argList, int);
		    string = itoa(integer, formatBuffer, 16);
		    boardPutString(string);
		    break;
		    
		case '%':
		    putchar('%');
		    break;
		}
	}
    
    va_end(argList);
    
}

#define printf(...) boardPrintf(__VA_ARGS__)
