#include "minigl.h"

char c_buff[SCREEN_SIZE_Y][SCREEN_SIZE_X];
float z_buff[SCREEN_SIZE_Y][SCREEN_SIZE_X];

void vertex_scale(vec4_t* v, float k) {
    v->coord.x *= k;
    v->coord.y *= k;
    v->coord.z *= k;
}

void vertex_move(vec4_t* v, float x, float y, float z, float r) {
    float r_x = v->coord.x * cosf(r) - v->coord.z * sinf(r);
    float r_z = v->coord.x * sinf(r) + v->coord.z * cosf(r);

    v->coord.x = r_x;
    v->coord.z = r_z;

    v->coord.x += x;
    v->coord.y += y;
    v->coord.z += z;
}

void minigl_perspective(vec4_t* v, float camera_fov, float camera_ratio, float clip_near, float clip_far) {
    // Convert to NDC
    v->coord.x /= -v->coord.z;
    v->coord.y /= -v->coord.z;

    v->coord.x *= 1.0f / tanf(camera_fov / 2.0f);
    v->coord.y *= 1.0f / (tanf(camera_fov / 2.0f) * camera_ratio);
    v->coord.z *= 1.0f / (clip_far - clip_near);
    v->coord.z += -(2.0f * clip_near) / (clip_far - clip_near);

    // Convert to raster
    v->coord.x *= 0.5f * ((float)SCREEN_SIZE_X);
    v->coord.x += 0.5f * ((float)SCREEN_SIZE_X);
    v->coord.y *= -0.5f * ((float)SCREEN_SIZE_Y);
    v->coord.y += 0.5f * ((float)SCREEN_SIZE_Y);
}

static inline float edge_funct(vec4_t a, vec4_t b, vec4_t c) {
    return (c.coord.x - a.coord.x) * (b.coord.y - a.coord.y) - (c.coord.y - a.coord.y) * (b.coord.x - a.coord.x);
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

void minigl_draw(vec4_t* vptr, int3_t* fptr, int fptr_size) {
    vec4_t v0, v1, v2;
    float w0, w1, w2;

    // FIXME: Need to improve performance!!!
    for (int f = 0; f < fptr_size; f++) {
        // Get vertices
        v0 = vptr[fptr[f].array[0]];
        v1 = vptr[fptr[f].array[1]];
        v2 = vptr[fptr[f].array[2]];

        // Culling
        // FIXME: Can be vec3
        vec4_t p = {(v0.coord.x + v1.coord.x + v2.coord.x) / 3.0f, (v0.coord.y + v1.coord.y + v2.coord.y) / 3.0f, 0, 0};

        w0 = edge_funct(v1, v2, p);
        w1 = edge_funct(v2, v0, p);
        w2 = edge_funct(v0, v1, p);

        // TODO: Make vertex order programmable
        if (w0 > 0 || w1 > 0 || w2 > 0) continue;

        // Calculate min bounding rectangle
        float mbr_min_x = fminf(v0.coord.x, fminf(v1.coord.x, v2.coord.x));
        float mbr_max_x = fmaxf(v0.coord.x, fmaxf(v1.coord.x, v2.coord.x));
        float mbr_min_y = fminf(v0.coord.y, fminf(v1.coord.y, v2.coord.y));
        float mbr_max_y = fmaxf(v0.coord.y, fmaxf(v1.coord.y, v2.coord.y));

        mbr_min_x = mbr_min_x > 0 ? mbr_min_x : 0;
        mbr_max_x = mbr_max_x < SCREEN_SIZE_X ? mbr_max_x : SCREEN_SIZE_X;
        mbr_min_y = mbr_min_y > 0 ? mbr_min_y : 0;
        mbr_max_y = mbr_max_y < SCREEN_SIZE_Y ? mbr_max_y : SCREEN_SIZE_X;

        float a = edge_funct(v0, v1, v2);

        for (int i = mbr_min_x; i < mbr_max_x; i++) {
            for (int j = mbr_min_y; j < mbr_max_y; j++) {
                p = (vec4_t){i, j, 0, 1};

                w0 = edge_funct(v1, v2, p);
                w1 = edge_funct(v2, v0, p);
                w2 = edge_funct(v0, v1, p);

                // TODO: Make vertex order programmable
                if (w0 <= 0 && w1 <= 0 && w2 <= 0) {
                    // Compute Z
                    w0 /= a;
                    w1 /= a;
                    w2 /= a;

                    // TODO: This is wrong
                    float z = 1.0f / (w0 * (1.0f / v0.coord.z) + w1 * (1.0f / v1.coord.z) + w2 * (1.0f / v2.coord.z));

                    // TODO: Make depth programmable
                    if (z > z_buff[j][i]) {
                        z_buff[j][i] = z;
                        c_buff[j][i] = 1;
                    }
                }
            }
        }
    }
}

void minigl_swap_frame(void) {
    for (int i = 0; i < SCREEN_SIZE_X; i++) {
        for (int j = 0; j < SCREEN_SIZE_Y; j++) {
            if (c_buff[j][i]) {
                pd->graphics->drawLine(i, j, i, j, 1, kColorBlack);
            }
        }
    }
}
