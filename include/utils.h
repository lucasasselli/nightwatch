#pragma once

#include <assert.h>
#include <stdlib.h>

#ifdef DEBUG
#define debug printf
#else
#define debug fake_printf
#endif

int fake_printf(const char *format, ...);

int rand_range(int min, int max);

int mini(int a, int b);

int maxi(int a, int b);
