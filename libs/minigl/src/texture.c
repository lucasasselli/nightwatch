#include "texture.h"

// TODO: Add hooks to be agnostic to the Playdate library
int minigl_tex_read_file(char *path, minigl_tex_t *out) {
    // Get the file handle
    SDFile *f = pd->file->open(path, kFileRead);
    if (f == NULL) {
        // FIXME
        const char *err = pd->file->geterr();
        pd->system->logToConsole("Can't open %s %s", path, err);
        return 1;
    }

    pd->file->seek(f, 0, SEEK_END);
    size_t file_size = pd->file->tell(f);
    pd->file->seek(f, 0, SEEK_SET);

    unsigned char *png_data = (unsigned char *)malloc(file_size);
    pd->file->read(f, png_data, file_size);

    // Create an spng context
    spng_ctx *ctx = spng_ctx_new(0);
    if (!ctx) {
        return 1;
    }

    // Set the input buffer
    spng_set_png_buffer(ctx, png_data, file_size);

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
    unsigned char *temp = (unsigned char *)malloc(out_size);
    spng_decode_image(ctx, temp, out_size, SPNG_FMT_RGBA8, 0);

    out->ptr = (uint8_t **)malloc(out->size_y * sizeof(uint8_t *));
    for (int j = 0; j < out->size_y; j++) {
        out->ptr[j] = (uint8_t *)malloc(out->size_x * sizeof(uint8_t));
    }

    for (size_t i = 0; i < out_size; i += 4) {
        uint8_t r = temp[i];
        uint8_t g = temp[i + 1];
        uint8_t b = temp[i + 2];

        // Calculate row and column indices
        size_t y = i / (4 * out->size_y);
        size_t x = (i / 4) % out->size_y;

        // Calculate pixel luminosity (https://stackoverflow.com/questions/596216/formula-to-determine-brightness-of-rgb-color)
        float lum = (r * 0.299) + (g * 0.587) + (b * 0.114);
        out->ptr[y][x] = (uint8_t)lum;
    }

    // Free resources
    spng_ctx_free(ctx);
    free(png_data);
    free(temp);
    pd->file->close(f);

    return 0;
}
