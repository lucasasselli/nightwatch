#include "utils.h"

#include <time.h>

struct timespec meas_time_start_data[5];
struct timespec meas_time_stop_data[5];

int fake_printf(const char* format, ...) {
    return 0;
}

int rand_range(int min, int max) {
    assert(max > min);
    return min + (rand() % (max - min));
}

int mini(int a, int b) {
    return (a < b) ? a : b;
}

int maxi(int a, int b) {
    return (a > b) ? a : b;
}

int64_t difftimespec_ns(const struct timespec after, const struct timespec before) {
    return ((int64_t)after.tv_sec - (int64_t)before.tv_sec) * (int64_t)1000000000 + ((int64_t)after.tv_nsec - (int64_t)before.tv_nsec);
}

void meas_time_start(int id) {
#ifdef DEBUG_PERF
    clock_gettime(CLOCK_REALTIME, &meas_time_start_data[id]);
#endif
}

void meas_time_stop(int id) {
#ifdef DEBUG_PERF
    clock_gettime(CLOCK_REALTIME, &meas_time_stop_data[id]);
#endif
}

void meas_time_print(int id, const char* msg) {
#ifdef DEBUG_PERF
    pd->system->logToConsole("%s: %ld", msg, difftimespec_ns(meas_time_stop_data[id], meas_time_start_data[id]));
#endif
}
