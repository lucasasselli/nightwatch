#pragma once

typedef enum {
    DIR_NORTH,
    DIR_EAST,
    DIR_SOUTH,
    DIR_WEST
} dir_t;

#define DIR_NUM 4

dir_t dir_flip(dir_t a);

dir_t dir_delta(dir_t a, dir_t b);

dir_t dir_rotate(dir_t a, dir_t r);
