#pragma once

#include <stdio.h>

// TODO: implement random number generation for consistent result between PC and Playdate

typedef enum {
    RAND_POINT,
    RAND_RANGE
} rand_val_type_t;

typedef struct {
    rand_val_type_t type;
    int a;
    int b;
    int weight;
} randw_int_t;

typedef struct {
    rand_val_type_t type;
    float a;
    float b;
    int weight;
} randw_float_t;

typedef struct {
    size_t size;
    randw_int_t values[];
} randw_constr_int_t;

typedef struct {
    size_t size;
    randw_int_t values[];
} randw_constr_float_t;

// clang-format off
#define RANDW_CONSTR_BEGIN(t, n, s) \
    const randw_constr_##t##_t n = { \
        .size = s, \
        .values = {

#define RANDW_CONSTR_END }}

#define RANDW_POINT(x, y) \
    { .type = RAND_POINT, .a = x, .b = 0, .weight = y }

#define RANDW_RANGE(x, y, z) \
    { .type = RAND_RANGE, .a = x, .b = y, .weight = z }
// clang-format on

int randi(int min, int max);

float randf(float min, float max, float step);

void randi_array(int* array, size_t size, int min, int max);

int randwi(const randw_constr_int_t* arg);

float randwf(const randw_constr_float_t* arg, float step);
