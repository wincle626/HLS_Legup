#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "legup_mem.h"

struct args {
    int * a;
    int b;
}; 

void Init(int *a) {
    *a = 10;
}

int ByVal(int a, int b) {
    return a + b;
}

// Note: a is both an input and an output
void ByRef(int *a, int b) {
    *a = *a + b;
}

void* ByRef_thread (void * args) {
    int * a = ((struct args *)args)->a;
    int b = ((struct args *)args)->b;
    ByRef(a, b);
    return NULL;
}


void ByRef_sw(int *a, int b) {
    *a = *a + b;
}

int main() {
    int num_threads = 4;
    int global[num_threads];
    int compare[num_threads];
    int increment[num_threads];
    pthread_t pool[num_threads];
    struct args arg_array[num_threads];

    int i, j, max = 10;
    //printf("Incrementing by %d for %d iterations:\n", increment, max);

    int *SHARED_MEM_global = malloc_shared(num_threads*sizeof(int), &global, LEGUP_RAM_LOCATION_ONCHIP);

    for (j = 0; j<num_threads;j++) {
        global[j] = 5;
        compare[j] = 5;
        increment[j] = 2*j;
    }

    for (i = 0; i<max; i++) {
        // pre-accelerator copy
        memcpy_to_shared(SHARED_MEM_global, &global, num_threads*sizeof(int));
        // accelerator call
        for (j = 0; j < num_threads; j++) {
            struct args tmp = {&SHARED_MEM_global[j], increment[j]};
            memcpy(arg_array+j, &tmp, sizeof(tmp));
            pthread_create(pool+j, NULL, ByRef_thread, (void *)(arg_array + j));
        }
        for (j = 0; j < num_threads; j++) {
            pthread_join(pool[j], NULL);
        }

        //ByRef(SHARED_MEM_global, increment);
	// post-accelerator copy
        memcpy_from_shared(&global, SHARED_MEM_global, num_threads*sizeof(int));

        for (j = 0; j < num_threads; j++) {
            printf("value from thread %d is %d\n", j, global[j]);
        }
    }

    free_shared(SHARED_MEM_global);

    return 0;
}
