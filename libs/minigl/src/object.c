#include "object.h"

#include <stdlib.h>
#include <string.h>

#include "system.h"

// TODO: Add hooks to be agnostic to the Playdate library
int minigl_obj_read_file(char *path, minigl_obj_t *out) {
    // Get the file handle
    void *f = minigl_fopen(path, "r");
    if (f == NULL) {
        return 1;
    }

    char line_buffer[OBJ_LINE_BUFFER_SIZE];
    int char_index = 0;

    // Read file line by line into the buffer
    out->vcoord_size = 0;
    out->face_size = 0;
    out->tcoord_size = 0;

    char c;

    while (minigl_fread((void *)&c, 1, f)) {
        line_buffer[char_index] = c;
        if (c == '\n' || c == EOF) {
            // Found EOL
            line_buffer[char_index] = '\0';

            // Count fields for malloc
            switch (line_buffer[0]) {
                case 'v':
                    if (line_buffer[1] == 't') {
                        // Texture coordinate
                        out->tcoord_size++;
                    } else {
                        // Vertex coordidate
                        out->vcoord_size++;
                    }
                    break;

                case 'f':
                    // Face
                    out->face_size++;
                    break;
            }

            // Reset line
            char_index = 0;
        } else {
            char_index++;
        }
    }

    // Allocate memory
    out->vcoord_ptr = malloc(sizeof(vec4) * out->vcoord_size);
    out->vface_ptr = malloc(sizeof(ivec3) * out->face_size);

    if (out->face_size > 0) {
        out->tcoord_ptr = malloc(sizeof(vec4) * out->tcoord_size);
        out->tface_ptr = malloc(sizeof(ivec3) * out->face_size);
    }

    // Load data
    int has_tex = out->tcoord_size > 0;
    out->vcoord_size = 0;
    out->face_size = 0;
    out->tcoord_size = 0;

    minigl_fseek(f, 0, SEEK_SET);

    while (minigl_fread((void *)&c, 1, f)) {
        line_buffer[char_index] = c;
        if (c == '\n' || c == EOF) {
            // Found EOL
            line_buffer[char_index] = '\0';

            switch (line_buffer[0]) {
                case 'v':
                    if (line_buffer[1] == 't') {
                        // Texture coordinate
                        float x, y;
                        sscanf(line_buffer, "vt %f %f", &x, &y);
                        out->tcoord_ptr[out->tcoord_size][0] = x;
                        out->tcoord_ptr[out->tcoord_size][1] = 1.0f - y;  // Flip texture
                        out->tcoord_size++;
                    } else {
                        // Vertex coordidate
                        float x, y, z;
                        sscanf(line_buffer, "v %f %f %f", &x, &y, &z);
                        out->vcoord_ptr[out->vcoord_size][0] = x;
                        out->vcoord_ptr[out->vcoord_size][1] = y;
                        out->vcoord_ptr[out->vcoord_size][2] = z;
                        out->vcoord_ptr[out->vcoord_size][3] = 1.0f;
                        out->vcoord_size++;
                    }
                    break;

                case 'f':
                    // Face
                    if (has_tex) {
                        int x, y, z, tx, ty, tz;
                        sscanf(line_buffer, "f %d/%d %d/%d %d/%d", &x, &tx, &y, &ty, &z, &tz);
                        out->vface_ptr[out->face_size][0] = x - 1;
                        out->vface_ptr[out->face_size][1] = y - 1;
                        out->vface_ptr[out->face_size][2] = z - 1;
                        out->tface_ptr[out->face_size][0] = tx - 1;
                        out->tface_ptr[out->face_size][1] = ty - 1;
                        out->tface_ptr[out->face_size][2] = tz - 1;
                    } else {
                        int x, y, z;
                        sscanf(line_buffer, "f %d %d %d", &x, &y, &z);
                        out->vface_ptr[out->face_size][0] = x - 1;
                        out->vface_ptr[out->face_size][1] = y - 1;
                        out->vface_ptr[out->face_size][2] = z - 1;
                    }
                    out->face_size++;
                    break;
            }

            // Reset line
            char_index = 0;
        } else {
            char_index++;
        }
    }

    minigl_fclose(f);

    return 0;
}

void minigl_obj_copy(minigl_obj_t in, minigl_obj_t *out) {
    *out = in;
    out->vcoord_ptr = (vec4 *)malloc(in.vcoord_size * sizeof(vec4));
    memcpy(out->vcoord_ptr, in.vcoord_ptr, in.vcoord_size * sizeof(vec4));
}

void minigl_obj_copy_trans(minigl_obj_t in, mat4 trans, minigl_obj_t *out) {
    // Copy all fields from in obj
    *out = in;

    // Allocate
    out->vcoord_ptr = (vec4 *)malloc(in.vcoord_size * sizeof(vec4));

    // Apply transformation
    for (int i = 0; i < in.vcoord_size; i++) {
        glm_mat4_mulv(trans, in.vcoord_ptr[i], out->vcoord_ptr[i]);
    }
}

void minigl_obj_trans(minigl_obj_t *in, mat4 trans) {
    // Apply transformation
    for (int i = 0; i < in->vcoord_size; i++) {
        glm_mat4_mulv(trans, in->vcoord_ptr[i], in->vcoord_ptr[i]);
    }
}

minigl_obj_buf_t minigl_obj_buf_init(size_t size) {
    minigl_obj_buf_t buf;
    buf.vcoord_ptr = (vec4 *)malloc(size * sizeof(vec4));
    return buf;
}

void minigl_obj_buf_free(minigl_obj_buf_t buf) {
    free(buf.vcoord_ptr);
}

void minigl_obj_to_obj_buf_trans(minigl_obj_t in, mat4 trans, minigl_obj_buf_t *out) {
    out->tcoord_ptr = in.tcoord_ptr;
    out->vface_ptr = in.vface_ptr;
    out->tface_ptr = in.tface_ptr;
    out->vcoord_size = in.vcoord_size;
    out->tcoord_size = in.tcoord_size;
    out->face_size = in.face_size;

    // Apply transformation
    for (int i = 0; i < in.vcoord_size; i++) {
        glm_mat4_mulv(trans, in.vcoord_ptr[i], out->vcoord_ptr[i]);
    }
}
