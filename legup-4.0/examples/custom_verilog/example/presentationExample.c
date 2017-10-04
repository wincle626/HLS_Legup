// Presentation Example
// Author: Mathew Hall
// Date: May 27, 2014

#include "stdiov.h"
//#include <stdio.h>

#define BUFFER_SIZE 1000

/*void __attribute__ ((noinline)) __attribute__ ((used)) updateCharactersEntered(int charactersEntered) {

    printf("Called updateCharactersEntered(int)\n");

    }*/

volatile int __attribute__ ((noinline)) __attribute__ ((used)) getLine(int *buffer) {

    printf("Called getLine(int *)\n");

    int charactersEntered = 0;

    do {

	//updateCharactersEntered(charactersEntered);

	int input = getchar();
	
	// 8 is ASCII backspace
	// If a user hits backspace with 0 characters entered
	// we will just return
	if (input == 8 && charactersEntered == 0) {
	    return 0;
	}

	// process valid input
	if (input == 8) {
	    charactersEntered --;
	}
	else {
	    buffer[charactersEntered] = input;
	}
	
	
    } while (buffer[charactersEntered++] != 13 &&
	     charactersEntered < BUFFER_SIZE);

    return charactersEntered;

}

void __attribute__ ((noinline)) __attribute__ ((used)) echoInput(void) {

    printf("Called echoInput()\n");

    int buffer[BUFFER_SIZE];
    int charactersEntered = 0;
    int previousCharacterCount = 0;
    
    while (charactersEntered < BUFFER_SIZE) {

	charactersEntered = getLine(buffer);

	int i;

	for (i = 0; i < previousCharacterCount; i++) {

	    int output = ' ';
	    putchar(output);

	}

	putchar(13);
	
	for (i = 0; i < charactersEntered; i++) {

	    int output = buffer[i];
	    putchar(output);

	}

	previousCharacterCount = charactersEntered;
    }
}

int main(void) {

    printf("hello %d\n", 5);

    echoInput();
    return 0;

}
