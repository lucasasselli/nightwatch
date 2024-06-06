#pragma once

#include <cglm/cglm.h>

#include "map.h"
#include "minigl/object.h"

extern minigl_objbuf_t obj_buf;

void map_init(void);

void map_draw(map_t map, mat4 trans, camera_t camera);

// FIXME
void map_tile_to_world(ivec2 tile, vec2 world);
