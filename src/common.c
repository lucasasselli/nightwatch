#include "common.h"

#include <assert.h>

dir_t dir_flip(dir_t a) {
    return (a + DIR_NUM / 2) % DIR_NUM;
}

dir_t dir_delta(dir_t a, dir_t b) {
    return (DIR_NUM + a - b) % DIR_NUM;
}

dir_t dir_rotate(dir_t a, dir_t r) {
    assert(r >= 0 && r < DIR_NUM);
    return (a + r) % DIR_NUM;
}
