#pragma once

#include <cglm/cglm.h>

#include "pd_api.h"

#ifdef DEBUG
#define debug pd->system->logToConsole
#else
#define debug fake_printf
#endif

extern PlaydateAPI* pd;

// Pixel handling macros
#define samplepixel(data, x, y, rowbytes) (((data[(y) * rowbytes + (x) / 8] & (1 << (uint8_t)(7 - ((x) % 8)))) != 0) ? kColorWhite : kColorBlack)
#define setpixel(data, x, y, rowbytes) (data[(y) * rowbytes + (x) / 8] &= ~(1 << (uint8_t)(7 - ((x) % 8))))
#define clearpixel(data, x, y, rowbytes) (data[(y) * rowbytes + (x) / 8] |= (1 << (uint8_t)(7 - ((x) % 8))))
#define drawpixel(data, x, y, rowbytes, color) (((color) == kColorBlack) ? setpixel((data), (x), (y), (rowbytes)) : clearpixel((data), (x), (y), (rowbytes)))

#define IVEC2_INIT(x) ((ivec2){x[0], x[1]})
#define VEC2_INIT(x) (ivec2){x[0], x[1]})

int fake_printf(const char* format, ...);

int mini(int a, int b);

int maxi(int a, int b);

int clampi(int x, int min, int max);

void vec2_to_ivec2(vec2 in, ivec2 out);

void ivec2_to_vec2(ivec2 in, vec2 out);

void ivec2_to_vec2_center(ivec2, vec2 out);

float vec2_angle(vec2 a, vec2 b);

void tile_dir(ivec2 tile, vec2 pos, vec2 out);
