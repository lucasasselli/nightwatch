#include "minigl.h"

uint8_t c_buff[SCREEN_SIZE_Y][SCREEN_SIZE_X];
float z_buff[SCREEN_SIZE_Y][SCREEN_SIZE_X];

minigl_cfg_t cfg;

void minigl_set_tex(minigl_tex_t t) {
    cfg.texture_mode = MINIGL_TEX_2D;
    cfg.texture = t;
}

void minigl_set_dither(minigl_tex_t t) {
    cfg.dither_mode = MINIGL_DITHER_ON;
    cfg.dither = t;
}

void minigl_clear(uint8_t color, int depth) {
    // Flush buffer
    for (int i = 0; i < SCREEN_SIZE_X; i++) {
        for (int j = 0; j < SCREEN_SIZE_Y; j++) {
            c_buff[j][i] = (uint8_t)color;
            z_buff[j][i] = depth;
        }
    }
}

static inline float _edge_funct(vec4 a, vec4 b, vec4 c) {
    return (c[0] - a[0]) * (b[1] - a[1]) - (c[1] - a[1]) * (b[0] - a[0]);
}

static inline float _interpolate(vec3 w, float a, float b, float c) {
    return (w[0] * a + w[1] * b + w[2] * c);
}

void minigl_draw(minigl_obj_t obj) {
    vec4 v[3];
    vec2 t[3];
    vec3 w;
    vec4 temp;

    if (cfg.texture_mode == MINIGL_TEX_2D) {
        if (obj.tcoord_size == 0) {
            pd->system->error("Object has no texture data!");
            return;
        }
    }

    // NOTE:
    // https://redirect.cs.umbc.edu/~olano/papers/2dh-tri/

    // FIXME: Need to improve performance!!!
    for (int f = 0; f < obj.face_size; f++) {
        //---------------------------------------------------------------------------
        // Indexing
        //---------------------------------------------------------------------------

        // Get vertex coordinates
        glm_vec4_copy(obj.vcoord_ptr[obj.vface_ptr[f][0]], v[0]);
        glm_vec4_copy(obj.vcoord_ptr[obj.vface_ptr[f][1]], v[1]);
        glm_vec4_copy(obj.vcoord_ptr[obj.vface_ptr[f][2]], v[2]);

        // Get texture coordinates
        if (cfg.texture_mode == MINIGL_TEX_2D) {
            glm_vec2_copy(obj.tcoord_ptr[obj.tface_ptr[f][0]], t[0]);
            glm_vec2_copy(obj.tcoord_ptr[obj.tface_ptr[f][1]], t[1]);
            glm_vec2_copy(obj.tcoord_ptr[obj.tface_ptr[f][2]], t[2]);
        }

        //---------------------------------------------------------------------------
        // Culling/Clipping
        //---------------------------------------------------------------------------

        // Trivial reject
        if ((v[0][0] < -1.0f && v[1][0] < -1.0f && v[2][0] < -1.0f) || (v[0][0] > 1.0f && v[1][0] > 1.0f && v[2][0] > 1.0f) ||
            (v[0][1] < -1.0f && v[1][1] < -1.0f && v[2][1] < -1.0f) || (v[0][1] > 1.0f && v[1][1] > 1.0f && v[2][1] > 1.0f) ||
            (v[0][2] < -1.0f && v[1][2] < -1.0f && v[2][2] < -1.0f) || (v[0][2] > 1.0f && v[1][2] > 1.0f && v[2][2] > 1.0f) ||
            (v[0][3] <= 0.0f || v[1][3] <= 0.0f || v[2][3] <= 0.0f))
            continue;

        vec4 p = GLM_VEC3_ZERO_INIT;
        p[0] = (v[0][0] + v[1][0] + v[2][0]) / 3.0f;
        p[1] = (v[0][1] + v[1][1] + v[2][1]) / 3.0f;

        w[0] = _edge_funct(v[1], v[2], p);
        w[1] = _edge_funct(v[2], v[0], p);
        w[2] = _edge_funct(v[0], v[1], p);

        int cw_wind_order = (w[0] > 0.0f || w[1] > 0.0f || w[2] > 0.0f);

        // if (cw_wind_order) continue;

        if (cw_wind_order) {
            glm_vec4_copy(v[0], temp);
            glm_vec4_copy(v[2], v[0]);
            glm_vec4_copy(temp, v[2]);

            glm_vec2_copy(t[0], temp);
            glm_vec2_copy(t[2], t[0]);
            glm_vec2_copy(temp, t[2]);
        }

        //---------------------------------------------------------------------------
        // Convert to Viewport
        //---------------------------------------------------------------------------

        for (int i = 0; i < 3; i++) {
            v[i][0] *= 0.5f * ((float)SCREEN_SIZE_X);
            v[i][0] += 0.5f * ((float)SCREEN_SIZE_X);
            v[i][1] *= 0.5f * ((float)SCREEN_SIZE_Y);
            v[i][1] += 0.5f * ((float)SCREEN_SIZE_Y);
        }

        //---------------------------------------------------------------------------
        // Rasterization
        //---------------------------------------------------------------------------

        // Calculate min bounding rectangle
        float mbr_min_x = fminf(v[0][0], fminf(v[1][0], v[2][0]));
        float mbr_max_x = fmaxf(v[0][0], fmaxf(v[1][0], v[2][0]));
        float mbr_min_y = fminf(v[0][1], fminf(v[1][1], v[2][1]));
        float mbr_max_y = fmaxf(v[0][1], fmaxf(v[1][1], v[2][1]));

        mbr_min_x = mbr_min_x > 0.0f ? mbr_min_x : 0.0f;
        mbr_max_x = mbr_max_x < SCREEN_SIZE_X ? mbr_max_x : SCREEN_SIZE_X;
        mbr_min_y = mbr_min_y > 0.0f ? mbr_min_y : 0.0f;
        mbr_max_y = mbr_max_y < SCREEN_SIZE_Y ? mbr_max_y : SCREEN_SIZE_Y;

        float area = _edge_funct(v[0], v[1], v[2]);

#ifdef PERSP_CORRECT
        for (int i = 0; i < 3; i++) {
            t[i][0] /= v[i][2];
            t[i][1] /= v[i][2];
            v[i][2] = 1.0f / v[i][2];
        }
#endif

        for (int j = mbr_min_y; j < mbr_max_y; j++) {
            for (int i = mbr_min_x; i < mbr_max_x; i++) {
                glm_vec4_copy((vec4){i, j, 0, 1}, p);

                w[0] = _edge_funct(v[1], v[2], p);
                w[1] = _edge_funct(v[2], v[0], p);
                w[2] = _edge_funct(v[0], v[1], p);

                // TODO: Make vertex order programmable
                // Draw the vertices if they are clockwise
                if ((w[0] <= 0.0f && w[1] <= 0.0f && w[2] <= 0.0f)) {
                    // Calculate barycentric coord.
                    glm_vec3_divs(w, area, w);

                    // Interpolate the Z
                    float z = 1.0f / _interpolate(w, v[0][2], v[1][2], v[2][2]);

                    // Depth test
                    // TODO: Make depth test programmable
                    if (z < z_buff[j][i]) {
                        z_buff[j][i] = z;

                        // FIXME: Conditional statements at this level are poison!!!
                        if (cfg.texture_mode == MINIGL_TEX_2D) {
                            // Interpolate texture coordinates
                            int tx = (int)(_interpolate(w, t[0][0], t[1][0], t[2][0]) * (float)(cfg.texture.size_x - 1));
                            int ty = (int)(_interpolate(w, t[0][1], t[1][1], t[2][1]) * (float)(cfg.texture.size_y - 1));

#ifdef PERSP_CORRECT
                            tx *= z;
                            ty *= z;
#endif

                            tx = tx < 0 ? 0 : tx >= cfg.texture.size_x ? cfg.texture.size_x - 1 : tx;
                            ty = ty < 0 ? 0 : ty >= cfg.texture.size_y ? cfg.texture.size_y - 1 : ty;

                            c_buff[j][i] = cfg.texture.ptr[ty][tx];
                        } else {
                            c_buff[j][i] = 255;
                        }
                    }
                }
            }
        }
    }
}

void minigl_swap_frame(void) {
    pd->graphics->clear(kColorBlack);
    for (int y = 0; y < SCREEN_SIZE_Y; y++) {
        for (int x = 0; x < SCREEN_SIZE_X; x++) {
            // FIXME: Add no dither
            if (c_buff[y][x] >= cfg.dither.ptr[y % cfg.dither.size_y][x % cfg.dither.size_x]) {
                // TODO: Access the screen memory directly
                // TODO: Extern to make platform agnostic
                pd->graphics->drawLine(x, SCREEN_SIZE_Y - y - 1, x, SCREEN_SIZE_Y - y - 1, 1, kColorWhite);
            }
        }
    }
}
