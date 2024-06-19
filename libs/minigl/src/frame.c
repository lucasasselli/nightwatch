#include "minigl/frame.h"

#include <spng.h>

#include "minigl/system.h"

minigl_frame_t* minigl_frame_new(int width, int height) {
    minigl_frame_t* out = malloc(sizeof(minigl_frame_t));

    out->size_x = width;
    out->size_y = height;
    out->data = (minigl_pixel_t**)malloc(out->size_y * sizeof(minigl_pixel_t*));
    for (int j = 0; j < out->size_y; j++) {
        out->data[j] = (minigl_pixel_t*)malloc(out->size_x * sizeof(minigl_pixel_t));
    }

    return out;
}

int minigl_frame_to_file(minigl_frame_t* frame, char* path) {
    int ret = 0;

    size_t buf_size = SCREEN_SIZE_X * SCREEN_SIZE_Y * 2;
    uint8_t* buf = malloc(buf_size);

    // Create a context
    spng_ctx* ctx = spng_ctx_new(SPNG_CTX_ENCODER);
    if (!ctx) {
        ret = 1;
        goto png_free;
    }

    FILE* file = minigl_fopen(path, "wb");
    spng_set_png_file(ctx, file);

    // Set IHDR parameters
    struct spng_ihdr ihdr = {0};
    ihdr.width = SCREEN_SIZE_X;
    ihdr.height = SCREEN_SIZE_Y;
    ihdr.bit_depth = 8;
    ihdr.color_type = SPNG_COLOR_TYPE_GRAYSCALE_ALPHA;

    spng_set_ihdr(ctx, &ihdr);

    int i = 0;
    for (int y = 0; y < SCREEN_SIZE_Y; y++) {
        for (int x = 0; x < SCREEN_SIZE_X; x++) {
            buf[i + 0] = frame->data[y][x].color;
            buf[i + 1] = frame->data[y][x].depth > 0 ? 255 : 0;
            i += 2;
        }
    }

    ret = spng_encode_image(ctx, buf, buf_size, SPNG_FMT_PNG, SPNG_ENCODE_FINALIZE);
    if (ret) {
        goto png_free;
    }

    minigl_fclose(file);

png_free:
    // Free context memory
    spng_ctx_free(ctx);
    free(buf);

    return ret;
}
