#include "object.h"

#include <stdlib.h>

minigl_obj_t obj_file_read(char *path) {
    // TODO: Add hooks to be agnostic to the Playdate library
    // Get the file handle
    SDFile *f = pd->file->open(path, kFileRead);
    if (f == NULL) {
        pd->system->error("Failed to open %s: %s", path, pd->file->geterr());
    }

    char line_buffer[OBJ_LINE_BUFFER_SIZE];
    int char_index = 0;

    // Read file line by line into the buffer
    minigl_obj_t obj;
    obj.vcoord_size = 0;
    obj.face_size = 0;
    obj.tcoord_size = 0;

    char c;

    while (pd->file->read(f, (void *)&c, 1)) {
        line_buffer[char_index] = c;
        if (c == '\n' || c == EOF) {
            // Found EOL
            line_buffer[char_index] = '\0';

            // Count fields for malloc
            switch (line_buffer[0]) {
                case 'v':
                    if (line_buffer[1] == 't') {
                        // Texture coordinate
                        obj.tcoord_size++;
                    } else {
                        // Vertex coordidate
                        obj.vcoord_size++;
                    }
                    break;

                case 'f':
                    // Face
                    obj.face_size++;
                    break;
            }

            // Reset line
            char_index = 0;
        } else {
            char_index++;
        }
    }

    // Allocate memory
    obj.vcoord_ptr = malloc(sizeof(vec4) * obj.vcoord_size);
    obj.vface_ptr = malloc(sizeof(ivec3) * obj.face_size);

    if (obj.face_size > 0) {
        obj.tcoord_ptr = malloc(sizeof(vec4) * obj.tcoord_size);
        obj.tface_ptr = malloc(sizeof(ivec3) * obj.face_size);
    }

    // Load data
    int has_tex = obj.tcoord_size > 0;
    obj.vcoord_size = 0;
    obj.face_size = 0;
    obj.tcoord_size = 0;

    pd->file->seek(f, 0, SEEK_SET);

    while (pd->file->read(f, (void *)&c, 1)) {
        line_buffer[char_index] = c;
        if (c == '\n' || c == EOF) {
            // Found EOL
            line_buffer[char_index] = '\0';
            pd->system->logToConsole("%s", line_buffer);

            switch (line_buffer[0]) {
                case 'v':
                    if (line_buffer[1] == 't') {
                        // Texture coordinate
                        float x, y;
                        sscanf(line_buffer, "vt %f %f", &x, &y);
                        obj.tcoord_ptr[obj.tcoord_size][0] = x;
                        obj.tcoord_ptr[obj.tcoord_size][1] = y;
                        obj.tcoord_size++;
                    } else {
                        // Vertex coordidate
                        float x, y, z;
                        sscanf(line_buffer, "v %f %f %f", &x, &y, &z);
                        obj.vcoord_ptr[obj.vcoord_size][0] = x;
                        obj.vcoord_ptr[obj.vcoord_size][1] = y;
                        obj.vcoord_ptr[obj.vcoord_size][2] = z;
                        obj.vcoord_ptr[obj.vcoord_size][3] = 1.0f;
                        obj.vcoord_size++;
                    }
                    break;

                case 'f':
                    // Face
                    if (has_tex) {
                        int x, y, z, tx, ty, tz;
                        sscanf(line_buffer, "f %d/%d %d/%d %d/%d", &x, &tx, &y, &ty, &z, &tz);
                        obj.vface_ptr[obj.face_size][0] = x - 1;
                        obj.vface_ptr[obj.face_size][1] = y - 1;
                        obj.vface_ptr[obj.face_size][2] = z - 1;
                        obj.tface_ptr[obj.face_size][0] = tx - 1;
                        obj.tface_ptr[obj.face_size][1] = ty - 1;
                        obj.tface_ptr[obj.face_size][2] = tz - 1;
                    } else {
                        int x, y, z;
                        sscanf(line_buffer, "f %d %d %d", &x, &y, &z);
                        obj.vface_ptr[obj.face_size][0] = x - 1;
                        obj.vface_ptr[obj.face_size][1] = y - 1;
                        obj.vface_ptr[obj.face_size][2] = z - 1;
                    }
                    obj.face_size++;
                    break;
            }

            // Reset line
            char_index = 0;
        } else {
            char_index++;
        }
    }

    pd->file->close(f);

    return obj;
}
