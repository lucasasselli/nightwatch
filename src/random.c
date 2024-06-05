#include "random.h"

#include <assert.h>
#include <stdlib.h>

// TODO: implement random number generation for consistent result between PC and Playdate

int randi(int min, int max) {
    assert(min < max);
    return min + rand() % (max - min);
}

void randi_array(int* array, size_t size, int min, int max) {
    for (size_t i = 0; i < size; i++) {
        array[i] = randi(min, max);
    }
}

int randwi(const randw_constr_int_t* c) {
    assert(c->size > 0);

    int max = 0;
    for (size_t i = 0; i < c->size; i++) {
        max += c->values[i].weight;
    }

    int val = randi(0, max);

    int t = 0;
    for (size_t i = 0; i < c->size; i++) {
        if (val < (t + c->values[i].weight)) {
            return c->values[i].value;
        }
        t += c->values[i].weight;
    }

    assert(0);
    return 0;
}
