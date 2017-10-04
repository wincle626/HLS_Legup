/*
    FIR with 32 inputs, 32bit, 16 taps
    NOTE: To change INPUTSIZE and TAPS, uncomment the other FIRFilterStreaming function
    and comment out the current one. Currently, a loop has been unrolled for better performance.
*/

#include <stdio.h>


#define INPUTSIZE 32
#define TAPS 16
#define EXPECTED_TOTAL 44880

int FIRFilterStreaming ( int in, int coefficients[], int previous[]){
    int temp = 0, j;    

    // UNROLLING THIS IMPROVES PERFROMANCE
     previous[15] = previous[14];
     previous[14] = previous[13];
     previous[13] = previous[12];
     previous[12] = previous[11];
     previous[11] = previous[10];
     previous[10] = previous[9];
     previous[9] = previous[8];
     previous[8] = previous[7];
     previous[7] = previous[6];
     previous[6] = previous[5];
     previous[5] = previous[4];
     previous[4] = previous[3];
     previous[3] = previous[2];
     previous[2] = previous[1];
     previous[1] = previous[0];
    previous[0] = in;

    if (previous[TAPS-1] == 0)
        return 0;
    else {
        for (j = 0; j < TAPS; j++){
            temp += previous[TAPS - j - 1]*coefficients[j];
        }

        return temp;
    }

}

/*
int FIRFilterStreaming ( int in, int coefficients[], int previous[]){
    int j, temp;
     for(j=(TAPS-1); j>=1; j-=1 ){

         previous[j] = previous[j-1];

     }
    previous[0] = in;

    if (previous[TAPS-1] == 0)
        return 0;
    else {
        temp = 0;

        for (j = 0; j < TAPS; j++){
            temp += previous[TAPS - j - 1]*coefficients[j];
        }
        return temp;
    }

}
*/

int main(){

    int previous[TAPS] = {0};
    int coefficients[TAPS] = {[0 ... 15] = 10};
    int output[INPUTSIZE];
    int i;
    unsigned int total = 0;
    for (i = 1; i <= INPUTSIZE; i++){
        output[i-1] = FIRFilterStreaming (i, coefficients, previous);
        total += output[i-1];
    }

    printf ("Result: %d\n", total);
    if (total == EXPECTED_TOTAL) {
        printf("RESULT: PASS\n");
    } else {
        printf("RESULT: FAIL\n");
    }
    return total;
}
