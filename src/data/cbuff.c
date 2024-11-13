#include "data/cbuff.h"

#include <stdlib.h>
#include <string.h>

#include "random.h"

cbuff_t* cbuff_new(unsigned int data_size, unsigned int size) {
    cbuff_t* buff = malloc(sizeof(cbuff_t));
    buff->head = 0;
    buff->tail = 0;
    buff->size = size;
    buff->data_size = data_size;
    buff->data_ptr = malloc(size * data_size);

    return buff;
}

bool cbuff_push(cbuff_t* buff, void* ptr) {
    if (cbuff_size(buff) == buff->size) {
        return false;
    }
    int head = buff->head % buff->size;
    memcpy(buff->data_ptr + head * buff->data_size, ptr, buff->data_size);
    buff->head++;

    return true;
}

unsigned int cbuff_size(cbuff_t* buff) {
    return (buff->head - buff->tail);
}

static void memswap(void* a, void* b, size_t size) {
    char *a_swap = (char*)a, *b_swap = (char*)b;
    char* a_end = a + size;

    while (a_swap < a_end) {
        char temp = *a_swap;
        *a_swap = *b_swap;
        *b_swap = temp;

        a_swap++, b_swap++;
    }
}

void cbuff_shuffle(cbuff_t* buff, int n) {
    if (buff->tail < buff->head) {
        for (int i = 0; i < n; i++) {
            int a = randi_range(buff->tail, buff->head) % buff->size;
            int b = randi_range(buff->tail, buff->head) % buff->size;
            if (a != b) {
                memswap(buff->data_ptr + a * buff->data_size, buff->data_ptr + b * buff->data_size, buff->data_size);
            }
        }
    }
}

bool cbuff_pop(cbuff_t* buff, void* ptr) {
    if (buff->head <= buff->tail) {
        return false;
    }

    int tail = buff->tail % buff->size;
    memcpy(ptr, buff->data_ptr + tail * buff->data_size, buff->data_size);
    buff->tail++;

    return true;
}

void cbuff_free(cbuff_t* buff) {
    free(buff->data_ptr);
}
