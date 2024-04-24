#include "minigl.h"

char c_buff[SCREEN_SIZE_Y][SCREEN_SIZE_X];
float z_buff[SCREEN_SIZE_Y][SCREEN_SIZE_X];

minigl_cfg_t cfg;

void minigl_set_tex(minigl_tex_t t) {
    cfg.texture_mode = MINIGL_TEX_2D;
    cfg.texture = t;
}

void minigl_clear(int color, int depth) {
    // Flush buffer
    for (int i = 0; i < SCREEN_SIZE_X; i++) {
        for (int j = 0; j < SCREEN_SIZE_Y; j++) {
            c_buff[j][i] = (char)color;
            z_buff[j][i] = depth;
        }
    }
}

static inline float edge_funct(vec4 a, vec4 b, vec4 c) {
    return (c[0] - a[0]) * (b[1] - a[1]) - (c[1] - a[1]) * (b[0] - a[0]);
}

static inline float interpolate(vec3 w, float a, float b, float c) {
    return (w[0] * a + w[1] * b + w[2] * c);
}

void minigl_draw(minigl_obj_t obj) {
    vec4 v[3];
    vec2 t0, t1, t2;
    vec3 w;

    if (cfg.texture_mode == MINIGL_TEX_2D) {
        if (obj.tcoord_size == 0) {
            pd->system->error("Object has no texture data!");
            return;
        }
    }

    // FIXME: Need to improve performance!!!
    for (int f = 0; f < obj.face_size; f++) {
        // Get vertex coordinates
        glm_vec4_copy(obj.vcoord_ptr[obj.vface_ptr[f][0]], v[0]);
        glm_vec4_copy(obj.vcoord_ptr[obj.vface_ptr[f][1]], v[1]);
        glm_vec4_copy(obj.vcoord_ptr[obj.vface_ptr[f][2]], v[2]);

        // Get texture coordinates
        if (cfg.texture_mode == MINIGL_TEX_2D) {
            glm_vec2_copy(obj.tcoord_ptr[obj.tface_ptr[f][0]], t0);
            glm_vec2_copy(obj.tcoord_ptr[obj.tface_ptr[f][1]], t1);
            glm_vec2_copy(obj.tcoord_ptr[obj.tface_ptr[f][2]], t2);
        }

        // Culling
        vec4 p = GLM_VEC3_ZERO_INIT;
        p[0] = (v[0][0] + v[1][0] + v[2][0]) / 3.0f;
        p[1] = (v[0][1] + v[1][1] + v[2][1]) / 3.0f;

        w[0] = edge_funct(v[1], v[2], p);
        w[1] = edge_funct(v[2], v[0], p);
        w[2] = edge_funct(v[0], v[1], p);

        // Cull bases on frustrum TODO

        // TODO: Make vertex order programmable
        if (w[0] > 0 || w[1] > 0 || w[2] > 0) continue;

        // Cover to screen size
        for (int i = 0; i < 3; i++) {
            v[i][0] *= 0.5f * SCREEN_SIZE_X;
            v[i][0] += 0.5f * SCREEN_SIZE_X;
            v[i][1] *= 0.5f * SCREEN_SIZE_Y;
            v[i][1] += 0.5f * SCREEN_SIZE_Y;
        }

        // Calculate min bounding rectangle
        float mbr_min_x = fminf(v[0][0], fminf(v[1][0], v[2][0]));
        float mbr_max_x = fmaxf(v[0][0], fmaxf(v[1][0], v[2][0]));
        float mbr_min_y = fminf(v[0][1], fminf(v[1][1], v[2][1]));
        float mbr_max_y = fmaxf(v[0][1], fmaxf(v[1][1], v[2][1]));

        mbr_min_x = mbr_min_x > 0 ? mbr_min_x : 0;
        mbr_max_x = mbr_max_x < SCREEN_SIZE_X ? mbr_max_x : SCREEN_SIZE_X;
        mbr_min_y = mbr_min_y > 0 ? mbr_min_y : 0;
        mbr_max_y = mbr_max_y < SCREEN_SIZE_Y ? mbr_max_y : SCREEN_SIZE_Y;

        float a = edge_funct(v[0], v[1], v[2]);

        for (int j = mbr_min_y; j < mbr_max_y; j++) {
            for (int i = mbr_min_x; i < mbr_max_x; i++) {
                glm_vec4_copy((vec4){i, j, 0, 1}, p);

                w[0] = edge_funct(v[1], v[2], p);
                w[1] = edge_funct(v[2], v[0], p);
                w[2] = edge_funct(v[0], v[1], p);

                // TODO: Make vertex order programmable
                // Draw the vertices if they are clockwise
                if (w[0] <= 0 && w[1] <= 0 && w[2] <= 0) {
                    // Calculate barycentric coord.
                    glm_vec3_divs(w, a, w);

                    // Interpolate the Z
                    // TODO: This is wrong
                    float z = 1.0f / interpolate(w, 1.0f / v[0][2], 1.0f / v[1][2], 1.0f / v[2][2]);

                    // Depth test
                    // TODO: Make depth programmable
                    if (z > z_buff[j][i]) {
                        z_buff[j][i] = z;

                        if (cfg.texture_mode == MINIGL_TEX_2D) {
                            // Calculate barycentric coord. with Perpective correction
                            w[0] /= v[0][2];
                            w[1] /= v[1][2];
                            w[2] /= v[2][2];

                            float k = w[0] + w[1] + w[2];
                            glm_vec3_divs(w, k, w);

                            int tx = (int)(interpolate(w, t0[0], t1[0], t2[0]) * (float)(cfg.texture.size_x - 1));
                            int ty = (int)((interpolate(w, t0[1], t1[1], t2[1])) * (float)(cfg.texture.size_y - 1));

                            tx = tx < 0 ? 0 : tx >= cfg.texture.size_x ? cfg.texture.size_x - 1 : tx;
                            ty = ty < 0 ? 0 : ty >= cfg.texture.size_y ? cfg.texture.size_y - 1 : ty;

                            c_buff[j][i] = cfg.texture.ptr[ty][tx];
                        } else {
                            c_buff[j][i] = 1;
                        }
                    }
                }
            }
        }
    }
}

void minigl_swap_frame(void) {
    pd->graphics->clear(kColorWhite);
    for (int i = 0; i < SCREEN_SIZE_X; i++) {
        for (int j = 0; j < SCREEN_SIZE_Y; j++) {
            if (c_buff[j][i]) {
                // TODO: Access the screen memory directly
                // TODO: Extern to make platform agnostic
                pd->graphics->drawLine(i, j, i, j, 1, kColorBlack);
            }
        }
    }
}
