#pragma once

#include <stdio.h>

#include "pd_api.h"

#define GFX_RASTER_WIREFRAME

#define SCREEN_SIZE_X 400
#define SCREEN_SIZE_Y 240

#define BUFFER_SIZE 1000

typedef union
{
    struct
    {
        float x;
        float y;
        float z;
    } coord;
    float array[3];
} vec3_t;

typedef union
{
    struct
    {
        float x;
        float y;
        float z;
        float w;
    } coord;
    float array[4];
} vec4_t;

typedef unsigned int face_t[3];

// TODO: Create struct
extern vec4_t o_buff[BUFFER_SIZE]; // Original
extern vec4_t v_buff[BUFFER_SIZE];
extern int v_buff_cnt;

extern face_t f_buff[BUFFER_SIZE];
extern int f_buff_cnt;

// TODO: Create object
extern char c_buff[SCREEN_SIZE_Y][SCREEN_SIZE_X];
extern float z_buff[SCREEN_SIZE_Y][SCREEN_SIZE_X];

void load_obj_file(PlaydateAPI* pd, char* path);

void convert_to_ndc(float camera_fov, float camera_ratio, float clip_near, float clip_far);

void convert_to_raster(void);

void vertex_scale(vec4_t* v, float k);

void vertex_move(vec4_t* v, float x, float y, float z, float r);

void raster(void);

void gfx_draw(PlaydateAPI* pd);
