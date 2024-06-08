#include "minigl/utils.h"

minigl_perf_data_t perf_data;

void minigl_perf_event(minigl_perf_event_t e) {
    perf_data.array[e]++;
}

void minigl_perf_clear(void) {
    for (int i = 0; i < 4; i++) {
        perf_data.array[i] = 0;
    }
}

minigl_perf_data_t minigl_perf_get(void) {
    return perf_data;
}
