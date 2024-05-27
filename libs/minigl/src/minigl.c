#include "minigl.h"

#include <assert.h>
#include <math.h>

#include "cglm/types.h"

uint8_t c_buff[SCREEN_SIZE_Y * SCREEN_SIZE_X];
float z_buff[SCREEN_SIZE_Y * SCREEN_SIZE_X];

minigl_cfg_t cfg = {0};
minigl_perf_data_t perf_data;

void minigl_set_tex(minigl_tex_t t) {
    cfg.texture_mode = MINIGL_TEX_2D;
    cfg.texture = t;
}

void minigl_set_dither(minigl_tex_t t) {
    cfg.dither = t;
}

void minigl_set_color(uint8_t color) {
    cfg.texture_mode = MINIGL_TEX_0D;
    cfg.draw_color = color;
}

void minigl_clear(uint8_t color, int depth) {
    for (int i = 0; i < SCREEN_SIZE_X * SCREEN_SIZE_Y; i++) {
        c_buff[i] = (uint8_t)color;  // Clear color buffer
        z_buff[i] = depth;           // Clear z buffer
    }
}

void minigl_perf_event(minigl_perf_event_t e) {
#ifdef DEBUG_PERF
    perf_data.array[e]++;
#endif
}

void minigl_perf_clear(void) {
    for (int i = 0; i < 4; i++) {
        perf_data.array[i] = 0;
    }
}

minigl_perf_data_t minigl_perf_get(void) {
    return perf_data;
}

MINIGL_INLINE float edge(vec4 a, vec4 b, vec4 c) {
    return (c[0] - a[0]) * (b[1] - a[1]) - (c[1] - a[1]) * (b[0] - a[0]);
}

MINIGL_INLINE float interpolate(vec3 w, float a, float b, float c) {
    return (w[0] * a + w[1] * b + w[2] * c);
}

MINIGL_INLINE int clampi(int x, int min, int max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

MINIGL_INLINE float clampf(float x, float min, float max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

MINIGL_INLINE void vec2_swap(float* a, float* b) {
    glm_swapf(&a[0], &b[0]);
    glm_swapf(&a[1], &b[1]);
}

MINIGL_INLINE void vec4_swap(float* a, float* b) {
    glm_swapf(&a[0], &b[0]);
    glm_swapf(&a[1], &b[1]);
    glm_swapf(&a[2], &b[2]);
    glm_swapf(&a[3], &b[3]);
}

MINIGL_INLINE float min3_clampf(float a, float b, float c, float min, float max) {
    float x;
    x = a < b ? a : b;
    x = x < c ? x : c;

    x = x > min ? x : min;
    x = x < max ? x : max;

    return x;
}

MINIGL_INLINE float max3_clampf(float a, float b, float c, float min, float max) {
    float x;
    x = a > b ? a : b;
    x = x > c ? x : c;

    x = x > min ? x : min;
    x = x < max ? x : max;
    return x;
}

MINIGL_INLINE void draw_scanline(float x1, float x2, float z1, float z2, const int y) {
    x1 = clampf(x1, 0, SCREEN_SIZE_X - 1);
    x2 = clampf(x2, 0, SCREEN_SIZE_X - 1);

    // Swap x1 and x2
    if (x2 < x1) {
        glm_swapf(&x1, &x2);
    }

    int buff_off = y * SCREEN_SIZE_X;

    for (int x = floorf(x1); x < ceilf(x2); x++) {
        minigl_perf_event(PERF_FRAG);

        // FIXME: Z calculation is slow
        float t = (float)(x - x1) / (x2 - x1);
        float z = (1 - t) * z1 + t * z2;

        if (z > z_buff[buff_off + x]) continue;
        z_buff[buff_off + x] = z;

        c_buff[buff_off + x] = cfg.draw_color;
        c_buff[buff_off + x] = (c_buff[buff_off + x] >= cfg.dither.color[y % cfg.dither.size_y][x % cfg.dither.size_x]);
    }
}

MINIGL_INLINE void draw(const minigl_obj_buf_t buf, const minigl_tex_mode_t tex_mode) {
    vec4 v[3];
    vec2 t[3];

    vec3 b;

#ifdef DEBUG
    if (tex_mode == MINIGL_TEX_2D) {
        assert(buf.tcoord_size > 0);
    }
#endif

    // FIXME: Need to improve performance!!!
    for (int f = 0; f < buf.face_size; f++) {
        //---------------------------------------------------------------------------
        // Indexing
        //---------------------------------------------------------------------------

        // Get vertex coordinates
        bool drop = false;

        // FIXME: move out
        for (int i = 0; i < 3; i++) {
            glm_vec4_copy(buf.vcoord_ptr[buf.vface_ptr[f][i]], v[i]);

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
        // FIXME: This might not be necessary
        if ((v[0][0] < -1.0f && v[1][0] < -1.0f && v[2][0] < -1.0f) || (v[0][0] > 1.0f && v[1][0] > 1.0f && v[2][0] > 1.0f) ||
            (v[0][1] < -1.0f && v[1][1] < -1.0f && v[2][1] < -1.0f) || (v[0][1] > 1.0f && v[1][1] > 1.0f && v[2][1] > 1.0f) ||
            (v[0][2] < -0.0f && v[1][2] < -0.0f && v[2][2] < -0.0f) || (v[0][2] > 1.0f && v[1][2] > 1.0f && v[2][2] > 1.0f)) {
            minigl_perf_event(PERF_CLIP);
            continue;
        }

        vec4 p;
        p[0] = (v[0][0] + v[1][0] + v[2][0]) / 3.0f;
        p[1] = (v[0][1] + v[1][1] + v[2][1]) / 3.0f;

        b[0] = edge(v[1], v[2], p);
        b[1] = edge(v[2], v[0], p);
        b[2] = edge(v[0], v[1], p);

        // NOTE: buf file have counter-clock wise wind order
        int cw_wind_order = (b[0] > 0.0f || b[1] > 0.0f || b[2] > 0.0f);

        // FIXME: Allow programmable backface Culling
        if (cw_wind_order) {
            // minigl_perf_event(PERF_CULL);
            //  continue;
        }

        // Get texture coordinates
        if (tex_mode == MINIGL_TEX_2D) {
            glm_vec2_copy(buf.tcoord_ptr[buf.tface_ptr[f][0]], t[0]);
            glm_vec2_copy(buf.tcoord_ptr[buf.tface_ptr[f][1]], t[1]);
            glm_vec2_copy(buf.tcoord_ptr[buf.tface_ptr[f][2]], t[2]);
        }

        if (cw_wind_order) {
            vec4_swap(v[0], v[2]);

            if (tex_mode == MINIGL_TEX_2D) {
                vec2_swap(t[0], t[2]);
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

#ifdef MINIGL_SCANLINE
        // Sort vertices by y-coordinate
        if (v[0][1] > v[1][1]) vec4_swap(v[0], v[1]);
        if (v[0][1] > v[2][1]) vec4_swap(v[0], v[2]);
        if (v[1][1] > v[2][1]) vec4_swap(v[1], v[2]);

        float slope1 = (v[1][0] - v[0][0]) / (v[1][1] - v[0][1]);
        float slope2 = (v[2][0] - v[0][0]) / (v[2][1] - v[0][1]);
        float slope3 = (v[2][0] - v[1][0]) / (v[2][1] - v[1][1]);

        // Initialize scanline
        float y0 = clampf(v[0][1], 0, SCREEN_SIZE_Y - 1);
        float y1 = clampf(v[1][1], 0, SCREEN_SIZE_Y - 1);
        float y2 = clampf(v[2][1], 0, SCREEN_SIZE_Y - 1);
        float y = y0;

        // Draw scanline
        if (!isinf(slope1) && !isinf(slope2)) {
            float x1 = v[0][0] + slope1 * (y - v[0][1]);
            float x2 = v[0][0] + slope2 * (y - v[0][1]);
            for (; y <= y1; y += 1.0f) {
                draw_scanline(x1, x2, v[1][2], v[2][2], y);
                x1 += slope1;
                x2 += slope2;
            }
        }

        if (!isinf(slope2) && !isinf(slope3)) {
            float x1 = v[1][0] + slope3 * (y - v[1][1]);
            float x2 = v[0][0] + slope2 * (y - v[0][1]);
            for (; y <= y2; y += 1.0f) {
                draw_scanline(x1, x2, v[1][2], v[0][2], y);
                x1 += slope3;
                x2 += slope2;
            }
        }
#else
        // Calculate min bounding rectangle
        float mbr_min_x = min3_clampf(v[0][0], v[1][0], v[2][0], 0.0f, SCREEN_SIZE_X);
        float mbr_max_x = max3_clampf(v[0][0], v[1][0], v[2][0], 0.0f, SCREEN_SIZE_X);
        float mbr_min_y = min3_clampf(v[0][1], v[1][1], v[2][1], 0.0f, SCREEN_SIZE_Y);
        float mbr_max_y = max3_clampf(v[0][1], v[1][1], v[2][1], 0.0f, SCREEN_SIZE_Y);

        float area = edge(v[0], v[1], v[2]);

        for (int y = mbr_min_y; y < mbr_max_y; y++) {
            for (int x = mbr_min_x; x < mbr_max_x; x++) {
                minigl_perf_event(PERF_FRAG);

                int buff_i = y * SCREEN_SIZE_X + x;

                // Calculate the fragment coordinates
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

                uint8_t color;

                if (tex_mode == MINIGL_TEX_2D) {
                    // Interpolate texture coordinates
                    int tex_u, tex_v;

#ifdef MINIGL_PERSP_CORRECT
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

                    if (cfg.texture.opacity[tex_v][tex_u] == 0) continue;

                    color = cfg.texture.color[tex_v][tex_u];
                } else {
                    color = cfg.draw_color;
                }

                // Depth test
                // TODO: Make depth test programmable
                if (z > z_buff[buff_i] || z > 1.0f) continue;
                z_buff[buff_i] = z;

#ifndef MINIGL_NO_Z_FADE
                if (z > MINIGL_Z_FADE_THRESHOLD) {
                    color = ((float)color) * (1.0f - z) / (1.0f - MINIGL_Z_FADE_THRESHOLD);
                }
#endif

                // FIXME: Should we do this at frame swap?
#ifndef MINIGL_NO_DITHERING
                c_buff[buff_i] = (color >= cfg.dither.color[y % cfg.dither.size_y][x % cfg.dither.size_x]);
#else
                c_buff[buff_i] = color;
#endif
            }
        }
#endif
    }
}

void minigl_draw(minigl_obj_buf_t buf) {
    if (cfg.texture_mode == MINIGL_TEX_2D) {
        draw(buf, MINIGL_TEX_2D);
    } else if (cfg.texture_mode == MINIGL_TEX_0D) {
        draw(buf, MINIGL_TEX_0D);
    }
}
