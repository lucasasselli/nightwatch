#pragma once

#include <cglm/cglm.h>

#include "game_state.h"
#include "minigl/object.h"

extern minigl_objbuf_t obj_buf;

void renderer_draw(game_state_t* gs, mat4 trans, camera_t camera, float delta_t);
