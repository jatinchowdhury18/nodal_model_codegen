#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "summing_amp.h"

static float* get_data(const char* path, size_t* count)
{
    FILE* file = fopen (path, "rb");
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
    printf ("Summing Amplifier test\n");

    size_t count;
    auto* input_v1 = get_data ("input_v1.bin", &count);
    auto* input_v2 = get_data ("input_v2.bin", &count);
    auto* input_v3 = get_data ("input_v3.bin", &count);
    auto* output = (float*) malloc (count * sizeof (float));

    const auto fs = 48000.0f;

    Params params = {
        .R1 = 1.25e+05f,
        .R2 = 5.0e+04f,
        .R3 = 2.5e+04f,
        .RL = 1.0e+04f,
        .RF = 1.0e+04f,
        .Cf = 100.0e-9f,
    };
    State state {};
    compute (&input_v1, &input_v2, &input_v3, &output, 1, (int) count, params, &state, fs);

    write_data (output, count);

    free (input_v1);
    free (input_v2);
    free (input_v3);
    free (output);

    return 0;
};
