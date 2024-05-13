#include "minigl.h"

#include "cglm/types.h"

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
    for (int x = 0; x < SCREEN_SIZE_X; x++) {
        for (int y = 0; y < SCREEN_SIZE_Y; y++) {
            c_buff[y][x] = (uint8_t)color;
            z_buff[y][x] = depth;
        }
    }
}

static inline float edge(vec4 a, vec4 b, vec4 c) {
    return (c[0] - a[0]) * (b[1] - a[1]) - (c[1] - a[1]) * (b[0] - a[0]);
}

static inline float interpolate(vec3 w, float a, float b, float c) {
    return (w[0] * a + w[1] * b + w[2] * c);
}

void minigl_draw(minigl_obj_t obj) {
    vec4 v[3];
    vec2 t[3];

    vec3 b;

    if (cfg.texture_mode == MINIGL_TEX_2D) {
        if (obj.tcoord_size == 0) {
            pd->system->error("Object has no texture data!");
            return;
        }
    }

    // FIXME: Need to improve performance!!!
    for (int f = 0; f < obj.face_size; f++) {
        //---------------------------------------------------------------------------
        // Indexing
        //---------------------------------------------------------------------------

        // Get vertex coordinates
        bool drop = false;

        for (int i = 0; i < 3; i++) {
            glm_vec4_copy(obj.vcoord_ptr[obj.vface_ptr[f][i]], v[i]);

            if (v[i][3] <= 0) {
                drop = true;
                continue;
            }

            v[i][3] = 1.0f / v[i][3];
            v[i][0] *= v[i][3];
            v[i][1] *= v[i][3];
            v[i][2] *= v[i][3];
        }

        if (drop) continue;

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
            (v[0][1] < -1.0f && v[1][1] < -1.0f && v[2][1] < -1.0f) || (v[0][1] > 1.0f && v[1][1] > 1.0f && v[2][1] > 1.0f))
            continue;

        vec4 p = GLM_VEC3_ZERO_INIT;
        p[0] = (v[0][0] + v[1][0] + v[2][0]) / 3.0f;
        p[1] = (v[0][1] + v[1][1] + v[2][1]) / 3.0f;

        b[0] = edge(v[1], v[2], p);
        b[1] = edge(v[2], v[0], p);
        b[2] = edge(v[0], v[1], p);

        int cw_wind_order = (b[0] > 0.0f || b[1] > 0.0f || b[2] > 0.0f);

        // if (cw_wind_order) continue;

        if (cw_wind_order) {
            vec4 temp;
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

        // Clip X and Y
        mbr_min_x = mbr_min_x > 0.0f ? mbr_min_x : 0.0f;
        mbr_max_x = mbr_max_x < SCREEN_SIZE_X ? mbr_max_x : SCREEN_SIZE_X;
        mbr_min_y = mbr_min_y > 0.0f ? mbr_min_y : 0.0f;
        mbr_max_y = mbr_max_y < SCREEN_SIZE_Y ? mbr_max_y : SCREEN_SIZE_Y;

        float area = edge(v[0], v[1], v[2]);

        for (int y = mbr_min_y; y < mbr_max_y; y++) {
            for (int x = mbr_min_x; x < mbr_max_x; x++) {
                // Calculate the fragment coordinates
                // FIXME: add 0.5 offset?
                glm_vec4_copy((vec4){x, y, 0, 1}, p);

                // auto interpolate = [](const auto a[3], auto p, const vec3 &coord) {
                // return coord.x*a[0].*p + coord.y*a[1].*p + coord.z*a[2].*p; };

                b[0] = edge(v[1], v[2], p);
                b[1] = edge(v[2], v[0], p);
                b[2] = edge(v[0], v[1], p);

                // TODO: Make vertex order programmable
                // Check if the point belongs to the triangle
                if ((b[0] > 0.0f || b[1] > 0.0f || b[2] > 0.0f)) continue;

                // Calculate barycentric coord.
                glm_vec3_divs(b, area, b);

                // Interpolate Z
                float z = interpolate(b, v[0][2], v[1][2], v[2][2]);

                // Depth test
                // TODO: Make depth test programmable
                if (z < -1.0f || z > 1.0f || z > z_buff[y][x]) continue;
                z_buff[y][x] = z;

                // FIXME: Conditional statements at this level are poison!!!
                if (cfg.texture_mode == MINIGL_TEX_2D) {
                    // Interpolate texture coordinates
                    int tex_u, tex_v;
#ifdef PERSP_CORRECT
                    // NOTE: https://stackoverflow.com/questions/24441631/how-exactly-does-opengl-do-perspectively-correct-linear-interpolation
                    // Interpolate W
                    for (int i = 0; i < 3; i++) {
                        b[i] *= v[i][3];
                    }

                    glm_vec3_divs(b, b[0] + b[1] + b[2], b);
#endif

                    tex_u = (int)(interpolate(b, t[0][0], t[1][0], t[2][0]) * (float)(cfg.texture.size_x - 1));
                    tex_v = (int)(interpolate(b, t[0][1], t[1][1], t[2][1]) * (float)(cfg.texture.size_y - 1));

                    tex_u = tex_u < 0 ? 0 : tex_u >= cfg.texture.size_x ? cfg.texture.size_x - 1 : tex_u;
                    tex_v = tex_v < 0 ? 0 : tex_v >= cfg.texture.size_y ? cfg.texture.size_y - 1 : tex_v;

                    c_buff[y][x] = cfg.texture.ptr[tex_v][tex_u];

                    // FIXME: Add no dither
                    c_buff[y][x] = (c_buff[y][x] >= cfg.dither.ptr[y % cfg.dither.size_y][x % cfg.dither.size_x]);
                } else {
                    c_buff[y][x] = 1;
                }
            }
        }
    }
}

void minigl_swap_frame(void) {
    pd->graphics->clear(kColorBlack);
    for (int y = 0; y < SCREEN_SIZE_Y; y++) {
        for (int x = 0; x < SCREEN_SIZE_X; x++) {
            if (c_buff[y][x]) {
                // TODO: Access the screen memory directly
                // TODO: Extern to make platform agnostic
                pd->graphics->drawLine(x, SCREEN_SIZE_Y - y - 1, x, SCREEN_SIZE_Y - y - 1, 1, kColorWhite);
            }
        }
    }
}
