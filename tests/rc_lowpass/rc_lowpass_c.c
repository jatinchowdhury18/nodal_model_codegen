#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "rc_lowpass_c.h"

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
    printf ("RC Lowpass C test\n");

    size_t count;
    float* input = get_data (&count);
    float* output = (float*) malloc (count * sizeof (float));

    const float fs = 48000.0f;

    Params params = {
        .R1 = 1.0e+03,
        .C1 = 1.0e-06,
    };
    State state = {0};
    compute ((const float* const*) &input, &output, 1, (int) count, params, &state, fs);

    write_data (output, count);

    free (input);
    free (output);

    return 0;
}