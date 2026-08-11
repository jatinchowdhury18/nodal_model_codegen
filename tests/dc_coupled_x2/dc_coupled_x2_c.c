#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "dc_coupled_x2_c.h"

static float* get_data(size_t* count)
{
    FILE* file = fopen ("input.bin", "rb");
    fseek (file, 0, SEEK_END);
    long file_size = ftell (file);
    rewind (file);

    char* buffer = (char*) malloc (file_size);
    size_t bytes_read = fread (buffer, 1, file_size, file);
    assert (bytes_read == (size_t)file_size);

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
    printf ("DC-Coupled x2 C Test\n");

    size_t count;
    float* input = get_data (&count);
    float* output = (float*) malloc (count * sizeof (float));

    const float fs = 48000.0f;

    Params params = {
        .stage1 = {
            .Vbias = 5.0e+00,
            .R1 = 1.0e+03,
            .R2 = 1.0e+03,
            .D1N914_Is = 2.52e-09,
            .D1N914_vt = 4.52892e-02,
            .R3 = 1.0e+04,
            .C1 = 9.999999e-08,
        },
        .stage2 = {
            .R1x = 1.0e+03,
            .D1N914_Is = 2.52e-09,
            .D1N914_vt = 4.52892e-02,
            .R3x = 1.0e+04,
            .C1x = 9.999999e-08,
        },
    };
    State state = {0};
    reset (params, &state, 1, fs, input[0]);  // seed DC operating point from initial input level
    compute ((const float* const*) &input, &output, 1, (int) count, params, &state, fs);

    write_data (output, count);

    free (input);
    free (output);

    return 0;
}
