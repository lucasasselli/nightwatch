#pragma once

#include <cglm/cglm.h>

#include "minigl.h"
#include "pd_api.h"

#ifdef DEBUG
#define debug pd->system->logToConsole
#else
#define debug fake_printf
#endif

// Pixel handling macros
#define samplepixel(data, x, y, rowbytes) (((data[(y) * rowbytes + (x) / 8] & (1 << (uint8_t)(7 - ((x) % 8)))) != 0) ? kColorWhite : kColorBlack)
#define setpixel(data, x, y, rowbytes) (data[(y) * rowbytes + (x) / 8] &= ~(1 << (uint8_t)(7 - ((x) % 8))))
#define clearpixel(data, x, y, rowbytes) (data[(y) * rowbytes + (x) / 8] |= (1 << (uint8_t)(7 - ((x) % 8))))
#define drawpixel(data, x, y, rowbytes, color) (((color) == kColorBlack) ? setpixel((data), (x), (y), (rowbytes)) : clearpixel((data), (x), (y), (rowbytes)))

extern PlaydateAPI* pd;

int fake_printf(const char* format, ...);

int rand_range(int min, int max);

int mini(int a, int b);

int maxi(int a, int b);

float vec2_angle(vec2 a, vec2 b);

bool ivec2_eq(ivec2 a, ivec2 b);

void tile_dir(ivec2 tile, vec3 pos, vec2 out);

bool tile_in_fov(ivec2 tile, minigl_camera_t camera, float fov, float r);

void mat4_billboard(minigl_camera_t camer, mat4 trans);

void meas_time_start(int id);

void meas_time_stop(int id);

void meas_time_print(int id, const char* msg);
