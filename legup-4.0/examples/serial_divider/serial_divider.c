// Perform a serial division
// Refer to: Section 10.2.4 of Stephen Brown and Zvonko Vranesic, "Fundamentals
// of Digital Logic with Verilog Design", McGaw-Hill, 2003.

#include <stdio.h>

#define N 4
volatile unsigned TEST_NUMERATORS[N] = {140, 300, 424, 525};
volatile unsigned TEST_DENOMINATORS[N] = {9, 33, 12, 29};

// return value:
// upper 32 bits are the quotient, lower 32 bits are the remainder
unsigned long long serial_divider (unsigned numerator, unsigned denominator) {
    int i;

    unsigned quotient = 0;
    unsigned remainder = 0;

    loop: for (i = 0; i < 32 ; i++) {
        remainder = remainder << 1;
        remainder += numerator >> 31;
        numerator = numerator << 1;

        quotient = quotient << 1;
        if (remainder >= denominator) {
            quotient += 1;
            remainder = remainder - denominator;
        }
    }
    unsigned long long result = quotient;
    result = result << 32;
    result = result + remainder;
    return result;
}


int main() {

    int test, i;
    unsigned numerator, denominator, remainder, quotient;
    int passed = 0;

    for (test = 0; test < N; test++) {
        printf("Test: %d\n", test);
        numerator = TEST_NUMERATORS[test];
        denominator = TEST_DENOMINATORS[test];

        printf("Start Divide\n");
        unsigned long long result = serial_divider(numerator, denominator);
        printf("Done Divide: %d\n", result);

        quotient = result >> 32;
        remainder = result;

        printf("Numerator: %d\n", numerator);
        printf("Denominator: %d\n", denominator);
        printf("Quotient: %d\n", quotient);
        printf("Remainder: %d\n", remainder);

        unsigned expected_quotient = numerator/denominator;
        unsigned expected_remainder = numerator % denominator;
        if (quotient == expected_quotient && 
            remainder == expected_remainder) {
            printf("PASS\n");
            passed++;
        } else {
            printf("Expected quotient: %d\n", expected_quotient);
            printf("Expected remainder: %d\n", expected_remainder);
            printf("FAIL\n");
        }
    }

    return passed;
}
