#include "minigl/texture.h"

#include <assert.h>
#include <stdlib.h>

#include "minigl/system.h"

#ifdef MINIGL_PNG
int minigl_tex_read_png(const char *path, minigl_tex_t *out, minigl_tex_read_opts_t opts) {
    const int STRIDE = 4;

    // Get the file handle
    FILE *f = minigl_fopen(path, "r");
    if (f == NULL) {
        return 1;
    }

    minigl_fseek(f, 0, SEEK_END);
    size_t file_size = minigl_ftell(f);
    minigl_fseek(f, 0, SEEK_SET);

    uint8_t *png_data = (uint8_t *)malloc(sizeof(uint8_t) * file_size);
    minigl_fread(png_data, file_size, f);
    minigl_fclose(f);

    // Create an spng context
    spng_ctx *ctx = spng_ctx_new(0);
    if (!ctx) {
        return 1;
    }

    // Set the input buffer
    if (spng_set_png_buffer(ctx, png_data, file_size)) {
        return 1;
    }

    // Decode to 8-bit RGBA
    size_t out_size;
    spng_decoded_image_size(ctx, SPNG_FMT_RGBA8, &out_size);

    // Extract width and height
    struct spng_ihdr ihdr;
    if (spng_get_ihdr(ctx, &ihdr)) {
        return 1;
    }

    out->size_x = ihdr.width;
    out->size_y = ihdr.height;

    // Allocate the temp buffer
    uint8_t *temp = (uint8_t *)malloc(sizeof(uint8_t) * out_size);
    spng_decode_image(ctx, temp, out_size, SPNG_FMT_RGBA8, 0);

    // Check if alpha channel is used
    bool alpha_used = false;
    for (size_t i = 0; i < out_size; i += STRIDE) {
        uint8_t a = temp[i + 3];

        if (a != 255) {
            alpha_used = true;
            break;
        }
    }

    // Allocate final texture memory location
    if (opts.force_g8 || !alpha_used) {
        // Grey 8
        out->format = MINIGL_COLOR_FMT_G8;
        out->data_g8 = (minigl_pixel_g8_t **)malloc(out->size_y * sizeof(minigl_pixel_g8_t *));
        for (uint32_t j = 0; j < out->size_y; j++) {
            out->data_g8[j] = (minigl_pixel_g8_t *)malloc(out->size_x * sizeof(minigl_pixel_g8_t));
        }
    } else {
        // Grey 8 w/ Alpha
        out->format = MINIGL_COLOR_FMT_GA8;
        out->data_ga8 = (minigl_pixel_ga8_t **)malloc(out->size_y * sizeof(minigl_pixel_ga8_t *));
        for (uint32_t j = 0; j < out->size_y; j++) {
            out->data_ga8[j] = (minigl_pixel_ga8_t *)malloc(out->size_x * sizeof(minigl_pixel_ga8_t));
        }
    }

    for (size_t i = 0; i < out_size; i += STRIDE) {
        uint8_t r = temp[i];
        uint8_t g = temp[i + 1];
        uint8_t b = temp[i + 2];
        uint8_t a = temp[i + 3];

        // Calculate row and column indices
        size_t y = i / (STRIDE * out->size_x);
        size_t x = (i / STRIDE) % out->size_x;

        // Calculate pixel luminosity (https://stackoverflow.com/questions/596216/formula-to-determine-brightness-of-rgb-color)
        float lum = (r * 0.299) + (g * 0.587) + (b * 0.114);

        switch (out->format) {
            case MINIGL_COLOR_FMT_G8:
                if (a > 0) {
                    out->data_g8[y][x].color = (uint8_t)lum;
                } else {
                    out->data_g8[y][x].color = opts.alpha_color;
                }
                break;
            case MINIGL_COLOR_FMT_GA8:
                out->data_ga8[y][x].color = (uint8_t)lum;
                out->data_ga8[y][x].alpha = a > 0 ? 255 : 0;
                break;
        }
    }

    // Free resources
    spng_ctx_free(ctx);
    free(png_data);
    free(temp);

    return 0;
}
#endif

int minigl_tex_read_tex(const char *path, minigl_tex_t *out) {
    // Get the file handle
    FILE *f = minigl_fopen(path, "r");
    if (f == NULL) {
        return 1;
    }

    minigl_fread(&out->format, sizeof(minigl_color_fmt_t), f);
    minigl_fread(&out->size_x, sizeof(uint32_t), f);
    minigl_fread(&out->size_y, sizeof(uint32_t), f);

    assert(out->size_x > 0 && out->size_x < 1000);
    assert(out->size_x > 0 && out->size_x < 1000);

    switch (out->format) {
        case MINIGL_COLOR_FMT_G8:
            out->data_g8 = (minigl_pixel_g8_t **)malloc(out->size_y * sizeof(minigl_pixel_g8_t *));
            for (uint32_t j = 0; j < out->size_y; j++) {
                out->data_g8[j] = (minigl_pixel_g8_t *)malloc(out->size_x * sizeof(minigl_pixel_g8_t));
                minigl_fread(out->data_g8[j], sizeof(minigl_pixel_g8_t) * out->size_x, f);
            }
            break;
        case MINIGL_COLOR_FMT_GA8:
            out->data_ga8 = (minigl_pixel_ga8_t **)malloc(out->size_y * sizeof(minigl_pixel_ga8_t *));
            for (uint32_t j = 0; j < out->size_y; j++) {
                out->data_ga8[j] = (minigl_pixel_ga8_t *)malloc(out->size_x * sizeof(minigl_pixel_ga8_t));
                minigl_fread(out->data_ga8[j], sizeof(minigl_pixel_ga8_t) * out->size_x, f);
            }
            break;
    }

    minigl_fclose(f);

    return 0;
}
