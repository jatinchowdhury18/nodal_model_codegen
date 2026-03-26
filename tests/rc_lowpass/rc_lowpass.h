#pragma once

struct Params {
    float R1;
    float C1;
};

struct State {
    float zC1;
};

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    const auto gR1 = 1.0f / params.R1;
    const auto gC1 = 2.0f * sample_rate * params.C1;
    
    const auto temp3 = gC1 + gR1;
    const auto temp4 = 1/temp3;

    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zC1 = state[ch].zC1;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            const auto temp5 = gR1*vi;
            const auto temp6 = temp5 + zC1;
            
            const auto vo = temp4*temp6;
            const auto tC1 = gC1*temp4*temp6;
            
            zC1 = 2 * tC1 - zC1;

            output[ch][n] = vo;
        }
        state[ch].zC1 = zC1;
    }
}
