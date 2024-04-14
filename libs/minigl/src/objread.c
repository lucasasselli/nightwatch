#include "objread.h"

#include <stdlib.h>

// TODO: Write a script to convert to binary directly
obj_data_t obj_file_read(char *path) {
    SDFile *f = pd->file->open(path, kFileRead);
    if (f == NULL) {
        pd->system->error("Failed to open %s: %s", path, pd->file->geterr());
    }

    char a;
    float x, y, z;

    char file_buffer[OBJ_FILE_BUFFER_SIZE];
    int file_size = pd->file->read(f, file_buffer, OBJ_FILE_BUFFER_SIZE);

    char line_buffer[20];
    int line_size;

    obj_data_t obj;

    // First pass to calculate size
    obj.vptr_size = 0;
    obj.fptr_size = 0;
    line_size = 0;
    for (int i = 0; i < file_size; i++) {
        line_buffer[line_size] = file_buffer[i];
        if (file_buffer[i] == '\n' || file_buffer[i] == EOF) {
            line_buffer[line_size] = '\0';

            // Found EOL
            sscanf(line_buffer, "%c %f %f %f", &a, &x, &y, &z);
            switch (a) {
                case 'v':
                    obj.vptr_size++;
                    break;

                case 'f':
                    obj.fptr_size++;
                    break;
            }

            // Reset line
            line_size = 0;
        } else {
            line_size++;
        }
    }

    // Allocate memory
    obj.vptr = malloc(sizeof(vec4_t) * obj.vptr_size);
    obj.fptr = malloc(sizeof(vec3_t) * obj.fptr_size);

    // Load data
    obj.vptr_size = 0;
    obj.fptr_size = 0;
    line_size = 0;
    for (int i = 0; i < file_size; i++) {
        line_buffer[line_size] = file_buffer[i];
        if (file_buffer[i] == '\n' || file_buffer[i] == EOF) {
            line_buffer[line_size] = '\0';

            // Found EOL
            sscanf(line_buffer, "%c %f %f %f", &a, &x, &y, &z);
            switch (a) {
                case 'v':
                    // Vertex
                    obj.vptr[obj.vptr_size] = (vec4_t){{x, y, z, 1}};
                    obj.vptr_size++;
                    break;

                case 'f':
                    // Face
                    // NOTE: Indexing starts at 1
                    obj.fptr[obj.fptr_size].array[0] = x - 1;
                    obj.fptr[obj.fptr_size].array[1] = y - 1;
                    obj.fptr[obj.fptr_size].array[2] = z - 1;
                    obj.fptr_size++;
                    break;
            }

            // TODO: Add debug macro
            pd->system->logToConsole("%c %f %f %f", a, x, y, z);

            // Reset line
            line_size = 0;
        } else {
            line_size++;
        }
    }

    pd->file->close(f);

    return obj;
}
