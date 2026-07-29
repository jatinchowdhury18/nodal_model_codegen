// Auto-generated with netlist_codegen version a65dc78.
// Command: netlist_codegen rc_lowpass.net rc_lowpass.h

#pragma once

struct Params {
    float R1 = 1.0e+03f;
    float C1 = 1.0e-06f;
};

struct State {
    float zC1 {};
};

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto gR1 = 1.0f / params.R1;
    
    const auto gC1 = 2.0f * sample_rate * params.C1;
    
    const auto _t0 = (1.0 / (gR1 + gC1));
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zC1 = state[ch].zC1;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            const auto vo = (((gR1 * vi) + zC1) * _t0);
            const auto tC1 = (gC1 * (vo - 0));
            
            zC1 = 2 * tC1 - zC1;

            output[ch][n] = vo;
        }
        state[ch].zC1 = zC1;
    }
}
