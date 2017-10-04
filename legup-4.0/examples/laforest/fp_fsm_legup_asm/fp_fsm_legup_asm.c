
// Direct implementation of ASM as gotos to labels.

#include<stdio.h>

// c99 -o fp_fsm_legup fp_fsm_legup.c

//
// Implements: [+-]?(\.[0-9]+|[0-9]+\.[0-9]*)
// Matches simple Python FP numbers
// See: http://www.regexper.com/#[%2B-]%3F%28\.[0-9]%2B|[0-9]%2B\.[0-9]*%29
//

// "ideal" MIPS-like code

// NOTE on names:
// "plus" is '+' byte
// "minus" is '-'
// "dot" is '.'
// "space" is ' '
// "zero_char" is the ASCII decimal value of '0': 48
// (it's part of a couple of instructions to test if a char is in the '0-9' range)

// init:
//         ADD  array_top,    0, array_top_init

// state0:
//         LW   temp, array_top
//         BLTZ init, temp
//         ADD  array_top, 1, array_top
//         XOR  temp2,  temp,  space
//         BEQZ state0, temp2
//         XOR  temp2,  temp,  plus
//         BEQZ state1, temp2
//         XOR  temp2,  temp,  minus
//         BEQZ state1, temp2
//         XOR  temp2,  temp,  dot
//         BEQZ state2, temp2
//         SUB  temp2,  temp,  zero_char
//         BLTZ state7, temp2
//         SUB  temp2,  10,    temp2
//         BGEZ state4, temp2
//         JMP  state7

// state1:
//         LW   temp, array_top
//         BLTZ init, temp
//         ADD  array_top, 1, array_top
//         XOR  temp2,  temp,  dot
//         BEQZ state2, temp2
//         SUB  temp2,  temp,  zero_char
//         BLTZ state7, temp2
//         SUB  temp2,  10,    temp2
//         BGEZ state4, temp2
//         JMP  state7

// state2:
//         LW   temp, array_top
//         BLTZ init, temp
//         ADD  array_top, 1, array_top
//         SUB  temp2,  temp,  zero_char
//         BLTZ state7, temp2
//         SUB  temp2,  10,    temp2
//         BGEZ state3, temp2
//         JMP  state7

// state3:
//         LW   temp, array_top
//         BLTZ init, temp
//         ADD  array_top, 1, array_top
//         XOR  temp2,  temp,  space
//         BEQZ state6, temp2
//         SUB  temp2,  temp,  zero_char
//         BLTZ state7, temp2
//         SUB  temp2,  10,    temp2
//         BGEZ state3, temp2
//         JMP  state7

// state4:
//         LW   temp, array_top
//         BLTZ init, temp
//         ADD  array_top, 1, array_top
//         XOR  temp2,  temp,  dot
//         BEQZ state5, temp2
//         SUB  temp2,  temp,  zero_char
//         BLTZ state7, temp2
//         SUB  temp2,  10,    temp2
//         BGEZ state4, temp2
//         JMP  state7

// state5:
//         LW   temp, array_top
//         BLTZ init, temp
//         ADD  array_top, 1, array_top
//         XOR  temp2,  temp,  space
//         BEQZ state6, temp2
//         SUB  temp2,  temp,  zero_char
//         BLTZ state7, temp2
//         SUB  temp2,  10,    temp2
//         BGEZ state3, temp2
//         JMP  state7

// state6:
//         SW   1, OUTPUT_PORT_ACCEPT
//         JMP  state0

// state7:
//         SW   1, OUTPUT_PORT_REJECT
//         JMP  state0

// 25 ACCEPTS, 1 REJECT (makes it easy to see the outermost loop)
// Arranged to traverse all (non-rejecting) paths in FSM
// Comment denotes ACCEPT/REJECT and traversed states

volatile char input[] = {
    ' ', '-', '.', '9', ' ', // Accept 1 2 3
    '+', '8', '.', '6', ' ', // Accept 1 4 5 3
    '-', '5', '.', ' ',      // Accept 1 4 5
    '.', '7', ' ',           // Accept 2 3
    '4', '.', ' ',           // Accept 4 5
    '5', '.', '2', ' ',      // Accept 4 5 3
    '-', '.', '9', ' ',      // Accept 1 2 3
    '+', '8', '.', '6', ' ', // Accept 1 4 5 3
    '-', '5', '.', ' ',      // Accept 1 4 5
    '.', '7', ' ',           // Accept 2 3
    '4', '.', ' ',           // Accept 4 5
    '5', '.', '2', ' ',      // Accept 4 5 3
    '-', '.', '9', ' ',      // Accept 1 2 3
    '+', '8', '.', '6', ' ', // Accept 1 4 5 3
    '-', '5', '.', ' ',      // Accept 1 4 5
    '.', '7', ' ',           // Accept 2 3
    '4', '.', ' ',           // Accept 4 5
    '5', '.', '2', ' ',      // Accept 4 5 3
    '-', '.', '9', ' ',      // Accept 1 2 3
    '+', '8', '.', '6', ' ', // Accept 1 4 5 3
    '-', '5', '.', ' ',      // Accept 1 4 5
    '.', '7', ' ',           // Accept 2 3
    '4', '.', ' ',           // Accept 4 5
    '5', '.', '2', ' ',      // Accept 4 5 3
    '-', '.', '9', ' ',      // Accept 1 2 3
    '-', '.', '9', 'A',      // Reject 1 2 3
    ' ', -1                  // Ends array
};

int main(void)
{
    int correct = 0;
    int reject = 0;

    volatile char *ptr = input;
    char curr;

    state0:
        curr = *ptr;
        if (curr < 0)                       {goto exit;}
        ptr++;
        if (curr == ' ')                    {goto state0;}
        if ((curr == '+') || (curr == '-')) {goto state1;}
        if (curr == '.')                    {goto state2;}
        if ((curr >= '0') && (curr <= '9')) {goto state4;}
        goto state7;

    state1:
        curr = *ptr;
        if (curr < 0)                       {goto exit;}
        ptr++;
        if (curr == '.')                    {goto state2;}
        if ((curr >= '0') && (curr <= '9')) {goto state4;}
        goto state7;

    state2:
        curr = *ptr;
        if (curr < 0)                       {goto exit;}
        ptr++;
        if ((curr >= '0') && (curr <= '9')) {goto state3;}
        goto state7;


    state3:
        curr = *ptr;
        if (curr < 0)                       {goto exit;}
        ptr++;
        if (curr == ' ')                    {goto state6;}
        if ((curr >= '0') && (curr <= '9')) {goto state3;}
        goto state7;


    state4:
        curr = *ptr;
        if (curr < 0)                       {goto exit;}
        ptr++;
        if (curr == '.')                    {goto state5;}
        if ((curr >= '0') && (curr <= '9')) {goto state4;}
        goto state7;

    state5:
        curr = *ptr;
        if (curr < 0)                       {goto exit;}
        ptr++;
        if (curr == ' ')                    {goto state6;}
        if ((curr >= '0') && (curr <= '9')) {goto state3;}
        goto state7;

    state6:
        correct++;
        goto state0;

    state7:
        reject++;
        goto state0;

    exit:
        printf("%d %d\n", correct, reject);

    return correct;
}

