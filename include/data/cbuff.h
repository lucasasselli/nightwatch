#pragma once

#include <stdbool.h>

typedef struct {
    unsigned int head;
    unsigned int tail;
    unsigned int size;
    unsigned int data_size;
    void* data_ptr;
} cbuff_t;

cbuff_t* cbuff_new(unsigned int, unsigned int);

bool cbuff_push(cbuff_t*, void*);

unsigned int cbuff_size(cbuff_t*);

bool cbuff_pop(cbuff_t*, void*);

void cbuff_shuffle(cbuff_t*, int);

void cbuff_free(cbuff_t*);
