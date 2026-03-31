#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "reductions.h"

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
    printf ("Reductions test\n");

    size_t count;
    auto* input = get_data (&count);
    auto* output = (float*) malloc (count * sizeof (float));

    const auto fs = 48000.0f;

    Params params = {
        .C0 = 1.0e-06,
        .C1 = 5.0e-06,
        .C1p = 5.1e-06,
        .C2 = 1.0e-06,
        .C3 = 1.5e-06,
        .R0 = 1.0e+03,
        .R1 = 5.0e+03,
        .R1p = 5.1e+03,
        .R2 = 1.0e+03,
        .R3 = 1.5e+03,
        .Rl = 10.0e+03,
    };
    State state {};
    compute (&input, &output, 1, (int) count, params, &state, fs);

    write_data (output, count);

    free (input);
    free (output);

    return 0;
};
