#define N 10
#include <stdio.h>

/* This program is structed as follows:
 * memD  memC  memB  memA
 *   \   / \   / \   /
 *   funcB funcA  main
 *
 * where the connections show which memories are used
 * by which functions
 *
 * Existing implementation
 * Case1:
 * By default, using central memory controller,
 * in the nested hierarchy
 * (main instantiates funcA which instantiates funcB)
 * there will be 2 2-to-1 mux (coming up to memory controller)
 * then 1 1-to-4 demux (to select between memories)
 *
 * Existing implementation
 * Case2:
 * If we use LOCAL_RAMS,
 * in the nested hierarchy
 * memD, memA will be localized, then
 * there will be 2 2-to-1 mux (coming up to memory controller)
 * then 1 1-to-2 demux (to select between memories memB, memC)
 * The muxes inside funcB and main which assign
 * memory_controller_* signals depending the state
 * will also be smaller since there are now a separate set of
 * signals for the localized memD and memA
 *
 * New implementation
 * Case3:
 * If we flatten the modules, while keeping the
 * central memory controller,
 * there will be 1 3-to-1 mux (all modules now reside at
 * same level, and there are three modules) going into
 * memory controller,
 * and 1 1-to-4 demux (to select between memories)
 *
 * New implementation
 * Case4:
 * If we flatten the modules, and distribute memories
 * (make separate ports from module to each independent memory,
 * similar to LOCAL_RAMS but also works for memories shared
 * between modules),
 * there will be 2 2-to-1 mux (in front of memC, memB)
 * and direct connections for memD and memA
 * The muxes inside funcB and main which assign
 * memory_controller_* signals depending the state
 * will also be smaller since there are now a separate set of
 * ports for memD and memA
 *
 */

volatile int memA[N] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
volatile int memB[N]; 
volatile int memC[N] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
volatile int memD[N] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

__attribute__((noinline)) int funcB(int sum) {
    int i, temp = 0;
    for (i = 0; i < N; i++) {
        temp += memC[i] + memD[i];
    }

    return sum + temp;
}

__attribute__ ((noinline))
int funcA() {
    int i, sum=0;
    for (i=0; i<N; i++) {
        sum += memB[i] + memC[i];
    }

    return funcB(sum);
}

int main() {
    int i;    
    for (i=0; i<N; i++) {
        memB[i] = memA[i];
    }

    int result = funcA();

    printf("Result: %d\n", result);
    if (result == 180) {
        printf("RESULT: PASS\n");
    } else {
        printf("RESULT: FAIL\n");
    }
    return result;
}
