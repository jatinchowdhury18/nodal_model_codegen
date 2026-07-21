#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "pedal_model_c.h"

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
    printf ("Pedal Model C test\n");

    size_t count;
    float* input = get_data (&count);
    float* output = (float*) malloc (count * sizeof (float));

    const float fs = 48000.0f;

    Params params = {
        .Vpp = 9.0e+00,
        .Vp = 4.5e+00,
        .R5 = 1.0e+06,
        .RL = 1.0e+05,
        .R4 = 1.0e+06,
        ._2N5485_Beta = 2.0e-03,
        ._2N5485_vp = -1.0e+00,
        .C4 = 1.0e-09,
        .Eamp_Aol = 3.0e+05,
        .Eamp_Vos = 2.0e-03,
        .Eamp_Ios = 1.0e-09,
        .Eamp_Ibias = 7.0e-09,
        .Eamp_Rin = 4.0e+07,
        .Eamp_Rcomp = 1.0e+00,
        .Eamp_Ccomp = 3.0e-11,
        .Eamp_Rout = 2.0e+02,
        .R11 = 1.0e+06,
        .C12 = 2.2e-08,
        .D1N914_Is = 2.52e-09,
        .D1N914_vt = 4.52892e-02,
        .R7 = 5.6e+02,
        .C6 = 4.7e-06,
        .C13 = 1.0e-05,
        .R6 = 1.0e+03,
        .C3 = 2.2e-08,
        .C11 = 3.3e-09,
        .R8 = 4.7e+01,
        .C7 = 2.2e-06,
        .R13 = 1.0e+04,
        .Rd = 1.0e+05,
        .C5 = 1.0e-10,
        .R9 = 1.0e+03,
        .C9 = 4.7e-06,
        .Rf = 1.0e+05,
        .R10 = 1.6e+03,
    };
    State state = {0};

    reset (params, &state, 1, fs, 0.0f);
    compute ((const float* const*) &input, &output, 1, (int) count, params, &state, fs);

    write_data (output, count);

    free (input);
    free (output);

    return 0;
}
