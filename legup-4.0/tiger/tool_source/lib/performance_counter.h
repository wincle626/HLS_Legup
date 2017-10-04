#define PERF_START      (volatile int*) 0xF1000000
#define PERF_STOP       (volatile int*) 0xF1000004

void legup_start_counter(int index);
int legup_stop_counter(int index);


