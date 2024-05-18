#pragma once

#include <assert.h>
#include <stdlib.h>

#include "pd_api.h"

#ifdef DEBUG
#define debug pd->system->logToConsole
#else
#define debug fake_printf
#endif

extern PlaydateAPI* pd;

int fake_printf(const char* format, ...);

int rand_range(int min, int max);

int mini(int a, int b);

int maxi(int a, int b);
