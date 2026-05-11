#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "multi_svf.h"

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

static void write_data (const char* path, const float* data, size_t count)
{
    FILE* file = fopen(path, "wb");
    assert(file != NULL);

    size_t bytes_to_write = count * sizeof(float);
    size_t bytes_written = fwrite(data, 1, bytes_to_write, file);

    assert(bytes_written == bytes_to_write);

    fclose(file);
}

int main()
{
    printf ("Multi-Output SVF test\n");

    size_t count;
    auto* input = get_data (&count);
    auto* output_vh = (float*) malloc (count * sizeof (float));
    auto* output_vb = (float*) malloc (count * sizeof (float));
    auto* output_vl = (float*) malloc (count * sizeof (float));

    const auto fs = 48000.0f;

    Params params = {
        .Rin = 1.0e+04f,
        .R1 = 1.0e+04f,
        .Rf1 = 1.59e+04f,
        .Cf1 = 9.999999e-08f,
        .Rf2 = 1.59e+04f,
        .Cf2 = 9.999999e-08f,
        .Rfb1 = 2.5e+03f,
        .Rfb2 = 1.0e+04f,
        .Rg = 1.52e+04f,
    };
    State state {};
    compute (&input, &output_vh, &output_vb, &output_vl, 1, (int) count, params, &state, fs);

    write_data ("output_vh.bin", output_vh, count);
    write_data ("output_vb.bin", output_vb, count);
    write_data ("output_vl.bin", output_vl, count);

    free (input);
    free (output_vh);
    free (output_vb);
    free (output_vl);

    return 0;
};
