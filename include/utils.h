#pragma once

#include <assert.h>
#include <stdlib.h>

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
