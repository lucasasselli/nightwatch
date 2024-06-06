#pragma once

#include <cglm/cglm.h>

#include "pd_api.h"
#include "types.h"

#ifdef DEBUG
#define debug pd->system->logToConsole
#else
#define debug fake_printf
#endif

#ifdef __arm__
#include <arm_fp16.h>
typedef __fp16 fp16_t;
#else
typedef float fp16_t;
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

void vec2_to_ivec2(vec2 in, ivec2 out);

void ivec2_to_vec2(ivec2 in, vec2 out);

void ivec2_to_vec2_center(ivec2, vec2 out);

float vec2_angle(vec2 a, vec2 b);

void tile_dir(ivec2 tile, vec2 pos, vec2 out);

bool tile_in_fov(ivec2 tile, camera_t camera, float fov, float r);

void mat4_billboard(camera_t camer, mat4 trans);
