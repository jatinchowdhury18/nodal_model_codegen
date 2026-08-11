#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "common_emitter_c.h"

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
    printf ("Common Emitter C test\n");

    size_t count;
    float* input = get_data (&count);
    float* output = (float*) malloc (count * sizeof (float));

    const float fs = 48000.0f;

    Params params = {
        .VCC = 9.0e+00,
        .C1 = 1.0e-07,
        .R1 = 4.7e+05,
        .R2 = 1.0e+05,
        .RE = 2.7e+03,
        .RC = 1.2e+04,
        .Q2N5089_Is = 5.0e-14,
        .Q2N5089_vt = 2.585e-02,
        .Q2N5089_BetaF = 6.0e+02,
        .Q2N5089_BetaR = 5.0e+01,
        .C2 = 1.0e-07,
        .RL = 1.0e+05,
    };
    State state = {0};
    // vi's PULSE() source starts at -0.1V (not 0V/silence) -- match LTspice's
    // .op analysis, which solves the DC operating point using that value.
    reset (params, &state, 1, fs, -0.1f);
    compute ((const float* const*) &input, &output, 1, (int) count, params, &state, fs);

    write_data (output, count);

    free (input);
    free (output);

    return 0;
}
