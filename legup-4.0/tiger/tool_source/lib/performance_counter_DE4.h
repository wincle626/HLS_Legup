#define PERF_UNIT_BASE 0xF1000000

#define PERF_UNIT_OFFSET (PERF_UNIT_BASE + 0x00000020)

#define PERF_START		(volatile int * ) PERF_UNIT_BASE
#define PERF_STOP		(volatile int * ) PERF_UNIT_OFFSET

//void START_COUNTER();

void PERF_START_FUNC();

int PERF_STOP_FUNC();
//int STOP_COUNTER();

