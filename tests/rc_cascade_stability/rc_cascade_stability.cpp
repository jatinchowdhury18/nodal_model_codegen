#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "rc_cascade_stability.h"

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
    printf ("RC cascade stability test\n");

    size_t count;
    auto* input = get_data (&count);
    auto* output = (float*) malloc (count * sizeof (float));

    const auto fs = 48000.0f;

    Params params = {
        .R1 = 1.0e+06,
        .C1 = 1.0e-11,
        .R2 = 1.0e+06,
        .C2 = 1.0e-11,
        .R3 = 1.0e+06,
        .C3 = 1.0e-11,
        .R4 = 1.0e+06,
        .C4 = 1.0e-11,
        .R5 = 1.0e+06,
        .C5 = 1.0e-11,
        .R6 = 1.0e+06,
        .C6 = 1.0e-11,
        .R7 = 1.0e+06,
        .C7 = 1.0e-11,
        .R8 = 1.0e+06,
        .C8 = 1.0e-11,
        .R9 = 1.0e+06,
        .C9 = 1.0e-11,
        .R10 = 1.0e+06,
        .C10 = 1.0e-11,
    };
    State state {};
    compute (&input, &output, 1, (int) count, params, &state, fs);

    write_data (output, count);

    free (input);
    free (output);

    return 0;
};
