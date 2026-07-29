#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "common_cathode_c.h"

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
    printf ("Common Cathode C test\n");

    size_t count;
    float* input = get_data (&count);
    float* output = (float*) malloc (count * sizeof (float));

    const float fs = 48000.0f;

    Params params = {
        .VCC = 2.5e+02,
        .Cin = 2.0e-08,
        .Rg = 1.0e+06,
        .Cout = 2.0e-08,
        .RL = 1.0e+06,
        .Rp = 1.0e+05,
        ._12AX7_DEMPWOLF_Gk = 2.242e-03,
        ._12AX7_DEMPWOLF_Mu = 1.032e+02,
        ._12AX7_DEMPWOLF_Gamma = 1.26e+00,
        ._12AX7_DEMPWOLF_Ck = 3.4e+00,
        ._12AX7_DEMPWOLF_Gg = 6.177e-04,
        ._12AX7_DEMPWOLF_Xi = 1.314e+00,
        ._12AX7_DEMPWOLF_Cg = 9.901e+00,
        ._12AX7_DEMPWOLF_Ig0 = 8.025e-08,
        .Rk = 1.5e+03,
        .Ck = 2.5e-05,
    };
    State state = {0};
    // vi's SIN() source starts at 0V, matching LTspice's .op analysis.
    reset (params, &state, 1, fs, 0.0f);
    compute ((const float* const*) &input, &output, 1, (int) count, params, &state, fs);

    write_data (output, count);

    free (input);
    free (output);

    return 0;
}
