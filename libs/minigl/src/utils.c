#include "minigl/utils.h"

#ifdef MINIGL_DEBUG_PERF
minigl_perf_data_t perf_data;
#endif

void minigl_perf_event(minigl_perf_event_t e) {
#ifdef MINIGL_DEBUG_PERF
    perf_data.array[e]++;
#endif
}

void minigl_perf_clear(void) {
#ifdef MINIGL_DEBUG_PERF
    for (int i = 0; i < 4; i++) {
        perf_data.array[i] = 0;
    }
#endif
}

#ifdef MINIGL_DEBUG_PERF
minigl_perf_data_t minigl_perf_get(void) {
    return perf_data;
}
#endif
