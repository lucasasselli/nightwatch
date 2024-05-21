#include "minigl.h"

#include "cglm/types.h"
#include "utils.h"

uint8_t c_buff[SCREEN_SIZE_Y * SCREEN_SIZE_X];
float z_buff[SCREEN_SIZE_Y * SCREEN_SIZE_X];

minigl_cfg_t cfg;

typedef enum {
    PERF_CLIP,
    PERF_CULL,
    PERF_POLY,
    PERF_FRAG
} perf_event_t;

int perf_cnt[4];

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
    for (int i = 0; i < SCREEN_SIZE_X * SCREEN_SIZE_Y; i++) {
        c_buff[i] = (uint8_t)color;
        z_buff[i] = depth;
    }
}

void minigl_perf_event(perf_event_t e) {
#ifdef DEBUG_PERF
    perf_cnt[e]++;
#endif
}

void minigl_perf_clear(void) {
    for (int i = 0; i < 4; i++) {
        perf_cnt[i] = 0;
    }
}

void minigl_perf_print(void) {
#ifdef DEBUG_PERF
    debug("Clip count: %d", perf_cnt[PERF_CLIP]);
    debug("Cull count: %d", perf_cnt[PERF_CULL]);
    debug("Poly count: %d", perf_cnt[PERF_POLY]);
    debug("Frag count: %d", perf_cnt[PERF_FRAG]);
#endif
}

MINIGL_INLINE float edge(vec4 a, vec4 b, vec4 c) {
    return (c[0] - a[0]) * (b[1] - a[1]) - (c[1] - a[1]) * (b[0] - a[0]);
}

MINIGL_INLINE float interpolate(vec3 w, float a, float b, float c) {
    return (w[0] * a + w[1] * b + w[2] * c);
}

MINIGL_INLINE float minf3_clamp(float a, float b, float c, float min, float max) {
    float x;
    x = a < b ? a : b;
    x = x < c ? x : c;

    x = x > min ? x : min;
    x = x < max ? x : max;

    return x;
}

MINIGL_INLINE float maxf3_clamp(float a, float b, float c, float min, float max) {
    float x;
    x = a > b ? a : b;
    x = x > c ? x : c;

    x = x > min ? x : min;
    x = x < max ? x : max;
    return x;
}

void minigl_draw(minigl_obj_t obj) {
    vec4 v[3];
    vec2 t[3];

    vec3 b;

#ifdef DEBUG
    if (cfg.texture_mode == MINIGL_TEX_2D) {
        if (obj.tcoord_size == 0) {
            pd->system->error("Object has no texture data!");
            return;
        }
    }
#endif

    // FIXME: Need to improve performance!!!
    for (int f = 0; f < obj.face_size; f++) {
        //---------------------------------------------------------------------------
        // Indexing
        //---------------------------------------------------------------------------

        // Get vertex coordinates
        bool drop = false;

        // FIXME: move out
        for (int i = 0; i < 3; i++) {
            glm_vec4_copy(obj.vcoord_ptr[obj.vface_ptr[f][i]], v[i]);

            if (v[i][3] <= 0) {
                drop = true;
                break;
            }

            v[i][3] = 1.0f / v[i][3];
            v[i][0] *= v[i][3];
            v[i][1] *= v[i][3];
            v[i][2] *= v[i][3];
        }

        if (drop) {
            minigl_perf_event(PERF_CLIP);
            continue;
        }

        //---------------------------------------------------------------------------
        // Culling/Clipping
        //---------------------------------------------------------------------------

        // Trivial reject
        if ((v[0][0] < -1.0f && v[1][0] < -1.0f && v[2][0] < -1.0f) || (v[0][0] > 1.0f && v[1][0] > 1.0f && v[2][0] > 1.0f) ||
            (v[0][1] < -1.0f && v[1][1] < -1.0f && v[2][1] < -1.0f) || (v[0][1] > 1.0f && v[1][1] > 1.0f && v[2][1] > 1.0f)) {
            minigl_perf_event(PERF_CLIP);
            continue;
        }

        vec4 p;
        p[0] = (v[0][0] + v[1][0] + v[2][0]) / 3.0f;
        p[1] = (v[0][1] + v[1][1] + v[2][1]) / 3.0f;

        b[0] = edge(v[1], v[2], p);
        b[1] = edge(v[2], v[0], p);
        b[2] = edge(v[0], v[1], p);

        // NOTE: Obj file have counter-clock wise wind order
        int cw_wind_order = (b[0] > 0.0f || b[1] > 0.0f || b[2] > 0.0f);

        // FIXME: Allow programmable backface Culling
        // if (cw_wind_order) continue;

        // Get texture coordinates
        if (cfg.texture_mode == MINIGL_TEX_2D) {
            glm_vec2_copy(obj.tcoord_ptr[obj.tface_ptr[f][0]], t[0]);
            glm_vec2_copy(obj.tcoord_ptr[obj.tface_ptr[f][1]], t[1]);
            glm_vec2_copy(obj.tcoord_ptr[obj.tface_ptr[f][2]], t[2]);
        }

        if (cw_wind_order) {
            vec4 temp;
            glm_vec4_copy(v[0], temp);
            glm_vec4_copy(v[2], v[0]);
            glm_vec4_copy(temp, v[2]);

            if (cfg.texture_mode == MINIGL_TEX_2D) {
                glm_vec2_copy(t[0], temp);
                glm_vec2_copy(t[2], t[0]);
                glm_vec2_copy(temp, t[2]);
            }
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

        minigl_perf_event(PERF_POLY);

        // Calculate min bounding rectangle
        float mbr_min_x = minf3_clamp(v[0][0], v[1][0], v[2][0], 0.0f, SCREEN_SIZE_X);
        float mbr_max_x = maxf3_clamp(v[0][0], v[1][0], v[2][0], 0.0f, SCREEN_SIZE_X);
        float mbr_min_y = minf3_clamp(v[0][1], v[1][1], v[2][1], 0.0f, SCREEN_SIZE_Y);
        float mbr_max_y = maxf3_clamp(v[0][1], v[1][1], v[2][1], 0.0f, SCREEN_SIZE_Y);

        float area = edge(v[0], v[1], v[2]);

        for (int y = mbr_min_y; y < mbr_max_y; y++) {
            for (int x = mbr_min_x; x < mbr_max_x; x++) {
                minigl_perf_event(PERF_FRAG);

                int buff_i = y * SCREEN_SIZE_X + x;
                // Calculate the fragment coordinates
                // FIXME: add 0.5 offset?
                vec4 p;
                p[0] = x;
                p[1] = y;

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
                if (z < -1.0f || z > 1.0f || z > z_buff[buff_i]) continue;
                z_buff[buff_i] = z;

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

                    c_buff[buff_i] = cfg.texture.ptr[tex_v][tex_u];

                    // FIXME: Add option to disable dither
                    c_buff[buff_i] = (c_buff[buff_i] >= cfg.dither.ptr[y % cfg.dither.size_y][x % cfg.dither.size_x]);
                } else {
                    c_buff[buff_i] = 1;
                }
            }
        }
    }
}
