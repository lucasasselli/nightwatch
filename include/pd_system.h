#pragma once

#include "pd_api.h"
#include "stdio.h"

extern PlaydateAPI* pd;

void* minigl_fopen(char* path, char* flags) {
    if (strcmp(flags, "r") == 0) {
        return pd->file->open(path, kFileRead);
    }

    return NULL;
}

int minigl_fseek(void* file, int pos, int whence) {
    return pd->file->seek(file, pos, whence);
}

long minigl_ftell(void* f) {
    return pd->file->tell(f);
}

int minigl_fread(void* ptr, size_t size, void* f) {
    return pd->file->read(f, (void*)ptr, size);
}

int minigl_fwrite(void* file, const void* buf, unsigned int len) {
    return pd->file->write(file, buf, len);
}

int minigl_fclose(void* file) {
    return pd->file->close(file);
}
