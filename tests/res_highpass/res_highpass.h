// Auto-generated with netlist_codegen version f57da4b.
// Command: netlist_codegen res_highpass.net res_highpass.h

#pragma once

struct Params {
    float C21 = 2.2e-07f;
    float C22 = 2.2e-07f;
    float L1 = 3.0e+00f;
    float R9 = 5.1e+03f;
};

struct State {
    float zC21 {};
    float zC22 {};
    float zL1 {};
};

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto gC21 = 2.0f * sample_rate * params.C21;
    
    const auto gC22 = 2.0f * sample_rate * params.C22;
    
    const auto gL1 = 1.0f / (2.0f * sample_rate * params.L1);
    
    const auto gR9 = 1.0f / params.R9;
    
    const auto _t0 = ((gC21 + gC22) + gL1);
    const auto _t3 = (gC22 + gR9);
    const auto _t2 = (1 / ((_t0 * _t3) - (gC22 * gC22)));
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zC21 = state[ch].zC21;
        auto zC22 = state[ch].zC22;
        auto zL1 = state[ch].zL1;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            const auto _t1 = (zL1 - (((gC21 * vi) - zC21) + zC22));
            const auto vo = (-(((_t0 * zC22) + (_t1 * gC22)) * _t2));
            const auto vl = (-(((_t1 * _t3) + (gC22 * zC22)) * _t2));
            const auto tC21 = (gC21 * (vi - vl));
            const auto tC22 = (gC22 * (vl - vo));
            const auto tL1 = (gL1 * (vl - 0));
            
            zC21 = 2 * tC21 - zC21;
            zC22 = 2 * tC22 - zC22;
            zL1 = 2 * tL1 + zL1;

            output[ch][n] = vo;
        }
        state[ch].zC21 = zC21;
        state[ch].zC22 = zC22;
        state[ch].zL1 = zL1;
    }
}
