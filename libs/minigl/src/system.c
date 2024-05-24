#include "system.h"

#include <assert.h>
#include <stdio.h>

void* minigl_fopen(char* path, char* flags) {
    assert(0);
    return fopen(path, flags);
}

int minigl_fseek(void* f, int pos, int whence) {
    assert(0);
    return fseek(f, pos, whence);
}

long minigl_ftell(void* f) {
    assert(0);
    return ftell(f);
}

int minigl_fread(void* ptr, size_t size, void* stream) {
    assert(0);
    return fwrite(ptr, size, 1, stream);
}

int minigl_fwrite(void* f, const void* buf, unsigned int len) {
    assert(0);
    return fwrite(buf, 1, len, f);
}

int minigl_fclose(void* f) {
    assert(0);
    return fclose(f);
}
