#include "random.h"

#include <assert.h>
#include <stdlib.h>

int randi_range(int min, int max) {
    assert(min < max);
    return min + rand() % (max - min);
}

int randi(int max) {
    return randi_range(0, max);
}

float randf_range(float min, float max, float step) {
    assert(min < max);
    assert(step < max);
    float d = (max - min) / step;
    return min + randi_range(0, d) * step;
}

void randi_array(int* array, size_t size, int min, int max) {
    for (size_t i = 0; i < size; i++) {
        array[i] = randi_range(min, max);
    }
}

int randwi(const randw_constr_int_t* c) {
    assert(c->size > 0);

    int max = 0;
    for (size_t i = 0; i < c->size; i++) {
        max += c->values[i].weight;
    }

    int w = randi_range(0, max);

    int t = 0;
    for (size_t i = 0; i < c->size; i++) {
        if (w < (t + c->values[i].weight)) {
            if (c->values[i].type == RAND_POINT) {
                return c->values[i].a;
            } else {
                return randi_range(c->values[i].a, c->values[i].b);
            }
        }
        t += c->values[i].weight;
    }

    // Should never reach this
    assert(0);
    return 0;
}

float randwf(const randw_constr_float_t* c, float step) {
    assert(c->size > 0);

    int max = 0;
    for (size_t i = 0; i < c->size; i++) {
        max += c->values[i].weight;
    }

    int w = randi_range(0, max);

    int t = 0;
    for (size_t i = 0; i < c->size; i++) {
        if (w < (t + c->values[i].weight)) {
            if (c->values[i].type == RAND_POINT) {
                return c->values[i].a;
            } else {
                return randf_range(c->values[i].a, c->values[i].b, step);
            }
        }
        t += c->values[i].weight;
    }

    // Should never reach this
    assert(0);
    return 0;
}
