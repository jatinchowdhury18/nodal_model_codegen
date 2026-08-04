// Auto-generated with netlist_codegen version ea0eac5.
// Command: netlist_codegen opamp_finite_gain.net opamp_finite_gain.h

#pragma once

struct Params {
    float R1 = 1.0e+03f;
    float RF = 1.0e+04f;
    float Eop_Aol = 2.0e+02f;
};

struct State {
};

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto Eop_Aol = params.Eop_Aol;
    
    const auto gR1 = 1.0f / params.R1;
    
    const auto gRF = 1.0f / params.RF;
    
    const auto _t0 = (1.0 / ((gR1 + gRF) + (gRF * Eop_Aol)));
    for (int ch = 0; ch < num_channels; ++ch)
    {
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            const auto vo = (-(((gR1 * vi) * Eop_Aol) * _t0));
            

            output[ch][n] = vo;
        }
    }
}
