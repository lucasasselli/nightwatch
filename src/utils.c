#include "utils.h"

int fake_printf(const char *format, ...) {
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
