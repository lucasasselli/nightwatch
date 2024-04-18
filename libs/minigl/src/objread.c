#include "objread.h"

#include <stdlib.h>

// TODO: Write a script to convert to binary directly?
minigl_obj_t obj_file_read(char *path) {
    // Get the file handle
    SDFile *f = pd->file->open(path, kFileRead);
    if (f == NULL) {
        pd->system->error("Failed to open %s: %s", path, pd->file->geterr());
    }

    // NOTE: Read the entire buffer in memory in a string array
    char file_buffer[OBJ_FILE_BUFFER_SIZE][20];
    int char_index = 0;
    int file_lines = 0;

    // Read file line by line into the buffer
    minigl_obj_t obj;
    obj.vcoord_size = 0;
    obj.face_size = 0;
    obj.tcoord_size = 0;

    char c;

    while (pd->file->read(f, (void *)&c, 1)) {
        file_buffer[file_lines][char_index] = c;
        if (c == '\n' || c == EOF) {
            // Found EOL
            file_buffer[file_lines][char_index] = '\0';

            // Count fields for malloc
            switch (file_buffer[file_lines][0]) {
                case 'v':
                    if (file_buffer[file_lines][1] == 't') {
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
            file_lines++;
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

    for (int i = 0; i < file_lines; i++) {
        pd->system->logToConsole("%s", file_buffer[i]);
        switch (file_buffer[i][0]) {
            case 'v':
                if (file_buffer[i][1] == 't') {
                    // Texture coordinate
                    float x, y;
                    sscanf(file_buffer[i], "vt %f %f", &x, &y);
                    obj.tcoord_ptr[obj.tcoord_size][0] = x;
                    obj.tcoord_ptr[obj.tcoord_size][1] = y;
                    obj.tcoord_size++;
                } else {
                    // Vertex coordidate
                    float x, y, z;
                    sscanf(file_buffer[i], "v %f %f %f", &x, &y, &z);
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
                    sscanf(file_buffer[i], "f %d/%d %d/%d %d/%d", &x, &tx, &y, &ty, &z, &tz);
                    obj.vface_ptr[obj.face_size][0] = x - 1;
                    obj.vface_ptr[obj.face_size][1] = y - 1;
                    obj.vface_ptr[obj.face_size][2] = z - 1;
                    obj.tface_ptr[obj.face_size][0] = tx - 1;
                    obj.tface_ptr[obj.face_size][1] = ty - 1;
                    obj.tface_ptr[obj.face_size][2] = tz - 1;
                } else {
                    int x, y, z;
                    sscanf(file_buffer[i], "f %d %d %d", &x, &y, &z);
                    obj.vface_ptr[obj.face_size][0] = x - 1;
                    obj.vface_ptr[obj.face_size][1] = y - 1;
                    obj.vface_ptr[obj.face_size][2] = z - 1;
                }
                obj.face_size++;
                break;
        }
    }

    return obj;
}
