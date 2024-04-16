#include "minigl.h"

char c_buff[SCREEN_SIZE_Y][SCREEN_SIZE_X];
float z_buff[SCREEN_SIZE_Y][SCREEN_SIZE_X];

minigl_cfg_t cfg;

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

void minigl_set_texture(minigl_texture_t t) {
    cfg.texture_mode = MINIGL_TEXTURE_2D;
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

static inline float edge_funct(vec4_t a, vec4_t b, vec4_t c) {
    return (c.coord.x - a.coord.x) * (b.coord.y - a.coord.y) - (c.coord.y - a.coord.y) * (b.coord.x - a.coord.x);
}

static inline float interpolate(vec3_t w, float a, float b, float c) {
    return (w.array[0] * a + w.array[1] * b + w.array[2] * c);
}

void minigl_draw(minigl_obj_t obj) {
    vec4_t v0, v1, v2;
    vec2_t t0, t1, t2;
    vec3_t w;

    if (cfg.texture_mode == MINIGL_TEXTURE_2D) {
        if (obj.tcoord_size == 0) {
            pd->system->error("Object has no texture data!");
            return;
        }
    }

    // FIXME: Need to improve performance!!!
    for (int f = 0; f < obj.face_size; f++) {
        // Get vertex coordinates
        v0 = obj.vcoord_ptr[obj.vface_ptr[f].array[0]];
        v1 = obj.vcoord_ptr[obj.vface_ptr[f].array[1]];
        v2 = obj.vcoord_ptr[obj.vface_ptr[f].array[2]];

        // Get texture coordinates
        if (cfg.texture_mode == MINIGL_TEXTURE_2D) {
            t0 = obj.tcoord_ptr[obj.tface_ptr[f].array[0]];
            t1 = obj.tcoord_ptr[obj.tface_ptr[f].array[1]];
            t2 = obj.tcoord_ptr[obj.tface_ptr[f].array[2]];
        }

        // Culling
        vec4_t p = {{(v0.coord.x + v1.coord.x + v2.coord.x) / 3.0f, (v0.coord.y + v1.coord.y + v2.coord.y) / 3.0f, 0, 0}};

        w.array[0] = edge_funct(v1, v2, p);
        w.array[1] = edge_funct(v2, v0, p);
        w.array[2] = edge_funct(v0, v1, p);

        // TODO: Make vertex order programmable
        if (w.array[0] < 0 || w.array[1] < 0 || w.array[2] < 0) continue;

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

        for (int j = mbr_min_y; j < mbr_max_y; j++) {
            for (int i = mbr_min_x; i < mbr_max_x; i++) {
                p = (vec4_t){{i, j, 0, 1}};

                w.array[0] = edge_funct(v1, v2, p);
                w.array[1] = edge_funct(v2, v0, p);
                w.array[2] = edge_funct(v0, v1, p);

                // TODO: Make vertex order programmable
                // Draw the vertices if they are clockwise
                if (w.array[0] >= 0 && w.array[1] >= 0 && w.array[2] >= 0) {
                    // Calculate barycentric coord.
                    w.array[0] /= a;
                    w.array[1] /= a;
                    w.array[2] /= a;

                    // Interpolate the Z
                    // TODO: This is wrong
                    float z = 1.0f / interpolate(w, 1.0f / v0.coord.z, 1.0f / v1.coord.z, 1.0f / v2.coord.z);

                    // Depth test
                    // TODO: Make depth programmable
                    if (z > z_buff[j][i]) {
                        z_buff[j][i] = z;

                        if (cfg.texture_mode == MINIGL_TEXTURE_2D) {
                            // Calculate barycentric coord. with Perpective correction
                            w.array[0] /= v0.coord.z;
                            w.array[1] /= v1.coord.z;
                            w.array[2] /= v2.coord.z;

                            float k = w.array[0] + w.array[1] + w.array[2];

                            w.array[0] /= k;
                            w.array[1] /= k;
                            w.array[2] /= k;

                            int tx = (int)(interpolate(w, t0.coord.x, t1.coord.x, t2.coord.x) * (float)(cfg.texture.size_x - 1));
                            // TODO: Change at objload
                            int ty = (int)((1.0f - interpolate(w, t0.coord.y, t1.coord.y, t2.coord.y)) * (float)(cfg.texture.size_y - 1));

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
