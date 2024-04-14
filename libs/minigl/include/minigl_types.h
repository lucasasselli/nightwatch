#pragma once

typedef union {
    struct {
        int x;
        int y;
        int z;
    } coord;
    int array[3];
} int3_t;

typedef union {
    struct {
        float x;
        float y;
        float z;
    } coord;
    float array[3];
} vec3_t;

typedef union {
    struct {
        float x;
        float y;
        float z;
        float w;
    } coord;
    float array[4];
} vec4_t;
