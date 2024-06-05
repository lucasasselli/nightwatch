#pragma once

#include <stdio.h>

typedef struct {
    int value;
    int weight;
} randw_int_t;

typedef struct {
    size_t size;
    randw_int_t values[];
} randw_constr_int_t;

int randi(int min, int max);

void randi_array(int* array, size_t size, int min, int max);

int randwi(const randw_constr_int_t* arg);
