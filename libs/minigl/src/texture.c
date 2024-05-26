#include "texture.h"

#include "system.h"

// TODO: Add hooks to be agnostic to the Playdate library
int minigl_tex_read_file(char *path, minigl_tex_t *out) {
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

    // Allocate final texture memory location
    out->color = (uint8_t **)malloc(out->size_y * sizeof(uint8_t *));
    out->opacity = (uint8_t **)malloc(out->size_y * sizeof(uint8_t *));
    for (int j = 0; j < out->size_y; j++) {
        out->color[j] = (uint8_t *)malloc(out->size_x * sizeof(uint8_t));
        out->opacity[j] = (uint8_t *)malloc(out->size_x * sizeof(uint8_t));
    }

    const int STRIDE = 4;

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

        out->color[y][x] = (uint8_t)lum;
        out->opacity[y][x] = a > 0 ? 255 : 0;
    }

    // Free resources
    spng_ctx_free(ctx);
    free(png_data);
    free(temp);

    return 0;
}
