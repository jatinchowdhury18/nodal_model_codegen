#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "eq_filter.h"

static float* get_data(size_t* count)
{
    FILE* file = fopen ("input.bin", "rb");
    fseek (file, 0, SEEK_END);
    long file_size = ftell (file);
    rewind (file);

    auto* buffer = (char*) malloc (file_size);
    size_t bytes_read = fread (buffer, 1, file_size, file);
    assert (bytes_read == file_size);

    *count = bytes_read / sizeof (float);
    return (float*) buffer;
}

static void write_data (const float* data, size_t count)
{
    FILE* file = fopen("output.bin", "wb");
    assert(file != NULL);

    size_t bytes_to_write = count * sizeof(float);
    size_t bytes_written = fwrite(data, 1, bytes_to_write, file);

    assert(bytes_written == bytes_to_write);

    fclose(file);
}

int main()
{
    printf ("EQ Filter test\n");

    size_t count;
    auto* input = get_data (&count);
    auto* output = (float*) malloc (count * sizeof (float));

    const auto fs = 48000.0f;

    Params params = {
        .R33 = 4.75e3,
        .R37 = 4.75e3,
        .R35 = 4.75e3,
        .R40 = 3.5e3,
        .R39 = 23.5e3,
        .R66 = 12.0e3,
        .R41 = 1.8e3,
        .R38 = 4.7e3,
        .R44 = 1.0e3,
        .R47 = 750,
        .R48 = 3.3e3,
        .R43 = 25.1e3,
        .C18 = 33e-9,
        .C19 = 47e-9,
        .R45 = 1.0e3,
        .C20 = 100e-6,
        .R42 = 24.3e3,
        .C16 = 33e-9,
        .C17 = 47e-9,
    };
    State state {};
    compute (&input, &output, 1, (int) count, params, &state, fs);

    write_data (output, count);

    free (input);
    free (output);

    return 0;
};
