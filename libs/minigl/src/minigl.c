#include "minigl.h"

#include <assert.h>
#include <math.h>

#include "cglm/types.h"

minigl_cfg_t cfg = {0};
minigl_frame_t frame;

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
    for (int y = 0; y < SCREEN_SIZE_Y; y++) {
        for (int x = 0; x < SCREEN_SIZE_X; x++) {
            frame.c_buff[y][x] = (uint8_t)color;  // Clear color buffer
            frame.z_buff[y][x] = depth;           // Clear z buffer
        }
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

MINIGL_INLINE void set_pixel(int x, int y, float z, uint8_t color) {
    if (z > frame.z_buff[y][x] || z > 1.0f || z < -1.0f) return;
    frame.z_buff[y][x] = z;

#ifndef MINIGL_NO_DITHERING
    frame.c_buff[y][x] = (color >= cfg.dither.color[y % cfg.dither.size_y][x % cfg.dither.size_x]);
#else
    frame.c_buff[y][x] = color;
#endif
}

MINIGL_INLINE void set_pixel_tex_2d(int x, int y, float z, float uf, float vf) {
    // Scale
    uf *= (float)(cfg.texture.size_x - 1);
    vf *= (float)(cfg.texture.size_y - 1);

    int u = clampi(uf, 0, cfg.texture.size_x - 1);
    int v = clampi(vf, 0, cfg.texture.size_y - 1);

    if (cfg.texture.opacity[v][u] == 0) return;

    set_pixel(x, y, z, cfg.texture.color[v][u]);
}

MINIGL_INLINE float scanline_slope(float a, float b, vec4 p0, vec4 p1) {
    return (a - b) / (p0[1] - p1[1]);  // Slope is computed on y
}

MINIGL_INLINE void scanline_slopes(float a, float b, float c, vec4 v[3], vec3 out) {
    out[0] = scanline_slope(b, a, v[1], v[0]);
    out[1] = scanline_slope(c, a, v[2], v[0]);
    out[2] = scanline_slope(c, b, v[2], v[1]);
}

MINIGL_INLINE void scanline_range1(float a, float b, vec4 v[3], vec3 slope, float y, vec2 out) {
    out[0] = a + slope[0] * (y - v[0][1]);
    out[1] = a + slope[1] * (y - v[0][1]);
}

MINIGL_INLINE void scanline_range2(float a, float b, vec4 v[3], vec3 slope, float y, vec2 out) {
    out[0] = b + slope[2] * (y - v[1][1]);
    out[1] = a + slope[1] * (y - v[0][1]);
}

MINIGL_INLINE void scanline_add_slope1(vec3 slope, vec2 out) {
    out[0] += slope[0];
    out[1] += slope[1];
}

MINIGL_INLINE void scanline_add_slope2(vec3 slope, vec2 out) {
    out[0] += slope[2];
    out[1] += slope[1];
}

MINIGL_INLINE void scanline_draw(const int y, vec2 x, vec2 z, vec2 u, vec2 v, minigl_tex_mode_t tex_mode) {
    vec2 x_l;
    glm_vec2_copy(x, x_l);
    glm_vec2_clamp(x_l, 0, SCREEN_SIZE_X);

    // Swap x_l[0] and x_l[1]
    int start_i = 0;
    int stop_i = 1;
    if (x_l[1] < x_l[0]) {
        start_i = 1;
        stop_i = 0;
    }

    for (int x_act = floorf(x_l[start_i]); x_act < ceilf(x_l[stop_i]); x_act++) {
        minigl_perf_event(PERF_FRAG);

        float t = ((float)(x_act)-x[start_i]) / (x[stop_i] - x[start_i]);
        float z_act = glm_lerp(z[start_i], z[stop_i], t);
        float u_act = glm_lerp(u[start_i], u[stop_i], t);
        float v_act = glm_lerp(v[start_i], v[stop_i], t);

        if (tex_mode == MINIGL_TEX_2D) {
            set_pixel_tex_2d(x_act, y, z_act, u_act, v_act);
        } else {
            set_pixel(x_act, y, z_act, cfg.draw_color);
        }
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
        if (v[0][1] > v[1][1]) {
            vec4_swap(v[0], v[1]);
            if (tex_mode == MINIGL_TEX_2D) vec2_swap(t[0], t[1]);
        }
        if (v[0][1] > v[2][1]) {
            vec4_swap(v[0], v[2]);
            if (tex_mode == MINIGL_TEX_2D) vec2_swap(t[0], t[2]);
        }
        if (v[1][1] > v[2][1]) {
            vec4_swap(v[1], v[2]);
            if (tex_mode == MINIGL_TEX_2D) vec2_swap(t[1], t[2]);
        }

        vec3 slope_x;
        vec3 slope_z;
        vec3 slope_tex_u;
        vec3 slope_tex_v;

        scanline_slopes(v[0][0], v[1][0], v[2][0], v, slope_x);
        scanline_slopes(v[0][2], v[1][2], v[2][2], v, slope_z);
        scanline_slopes(t[0][0], t[1][0], t[2][0], v, slope_tex_u);
        scanline_slopes(t[0][1], t[1][1], t[2][1], v, slope_tex_v);

        // Initialize scanline
        float y0 = glm_clamp(v[0][1], 0, SCREEN_SIZE_Y - 1);
        float y1 = glm_clamp(v[1][1], 0, SCREEN_SIZE_Y - 1);
        float y2 = glm_clamp(v[2][1], 0, SCREEN_SIZE_Y - 1);
        float y = y0;

        vec2 x;
        vec2 z;
        vec2 tex_u;
        vec2 tex_v;

        // Draw scanline
        if (!isinf(slope_x[0]) && !isinf(slope_x[1])) {
            scanline_range1(v[0][0], v[1][0], v, slope_x, y, x);
            scanline_range1(v[0][2], v[1][2], v, slope_z, y, z);
            scanline_range1(t[0][0], t[1][0], v, slope_tex_u, y, tex_u);
            scanline_range1(t[0][1], t[1][1], v, slope_tex_v, y, tex_v);

            for (; y <= y1; y += 1.0f) {
                scanline_draw(y, x, z, tex_u, tex_v, tex_mode);

                scanline_add_slope1(slope_x, x);
                scanline_add_slope1(slope_z, z);
                scanline_add_slope1(slope_tex_u, tex_u);
                scanline_add_slope1(slope_tex_v, tex_v);
            }
        }

        if (!isinf(slope_x[1]) && !isinf(slope_x[2])) {
            scanline_range2(v[0][0], v[1][0], v, slope_x, y, x);
            scanline_range2(v[0][2], v[1][2], v, slope_z, y, z);
            scanline_range2(t[0][0], t[1][0], v, slope_tex_u, y, tex_u);
            scanline_range2(t[0][1], t[1][1], v, slope_tex_v, y, tex_v);

            for (; y <= y2; y += 1.0f) {
                scanline_draw(y, x, z, tex_u, tex_v, tex_mode);

                scanline_add_slope2(slope_x, x);
                scanline_add_slope2(slope_z, z);
                scanline_add_slope2(slope_tex_u, tex_u);
                scanline_add_slope2(slope_tex_v, tex_v);
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

                if (tex_mode == MINIGL_TEX_2D) {

#ifdef MINIGL_PERSP_CORRECT
                    // NOTE: https://stackoverflow.com/questions/24441631/how-exactly-does-opengl-do-perspectively-correct-linear-interpolation
                    // Interpolate W
                    for (int i = 0; i < 3; i++) {
                        b[i] *= v[i][3];
                    }

                    glm_vec3_divs(b, b[0] + b[1] + b[2], b);
#endif

                    // Interpolate texture coordinates
                    float tex_u = (int)(interpolate(b, t[0][0], t[1][0], t[2][0]);
                    float tex_v = (int)(interpolate(b, t[0][1], t[1][1], t[2][1]);

                    set_pixel_tex_2d(x, y, z, tex_u, tex_v);
                } else {
                    set_pixel(x, y, z, cfg.draw_color);
                }
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

minigl_frame_t* minigl_get_frame(void) {
    return &frame;
}
