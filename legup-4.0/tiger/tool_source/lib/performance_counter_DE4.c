#include "performance_counter_DE4.h"

void PERF_START_FUNC() {
	*PERF_START = 1;
}

int PERF_STOP_FUNC() {
	*PERF_STOP = 1;
	return *PERF_STOP;
}

