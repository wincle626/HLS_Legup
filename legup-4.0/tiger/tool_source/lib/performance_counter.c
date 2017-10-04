#include "performance_counter.h"

void legup_start_counter(int index) {
	*(PERF_START + 2*index) = 1;
}

int legup_stop_counter(int index) {
	*(PERF_STOP + 2*index) = 1;
	return *(PERF_START + 2*index);
}

