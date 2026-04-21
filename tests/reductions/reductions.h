// Auto-generated with netlist_codegen version cc5c911.
// Command: netlist_codegen reductions.net reductions.h

#pragma once

struct Params {
    float R0 = 1.0e+03f;
    float C0 = 1.0e-06f;
    float Rl = 1.0e+04f;
    float R2 = 1.0e+03f;
    float R3 = 1.5e+03f;
    float R1p = 5.1e+03f;
    float R1 = 5.0e+03f;
    float C1 = 5.0e-06f;
    float C1p = 5.1e-06f;
    float C2 = 1.0e-06f;
    float C3 = 1.5e-06f;
};

struct State {
    float zR0C0 {};
    float zR2R3R1pR1C1C1pC2C3 {};
};

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto R0 = params.R0;
    const auto C0 = params.C0;
    const auto gR0C0 = 2.0f * sample_rate * C0 + (1.0f / R0);
    const auto gzR0C0 = 4.0f * sample_rate * C0;
    
    const auto gRl = 1.0f / params.Rl;
    
    const auto R2R3R1pR1 = sum(sum(params.R2, params.R3), recip_sum(params.R1p, params.R1));
    const auto C1C1pC2C3 = recip_sum(sum(params.C1, params.C1p), recip_sum(params.C2, params.C3));
    const auto gR2R3R1pR1C1C1pC2C3 = (2.0f * sample_rate * C1C1pC2C3) / (1.0f + 2.0f * sample_rate * R2R3R1pR1 * C1C1pC2C3);
    const auto gnR2R3R1pR1C1C1pC2C3 = gR2R3R1pR1C1C1pC2C3 * (2.0f * sample_rate * R2R3R1pR1 * C1C1pC2C3 - 1.0f) / (2.0f * sample_rate * C1C1pC2C3);
    
    const auto _t0 = (gR0C0 + gR2R3R1pR1C1C1pC2C3);
    const auto _t3 = (gRl + gR2R3R1pR1C1C1pC2C3);
    const auto _t2 = (1 / ((_t0 * _t3) - (gR2R3R1pR1C1C1pC2C3 * gR2R3R1pR1C1C1pC2C3)));
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zR0C0 = state[ch].zR0C0;
        auto zR2R3R1pR1C1C1pC2C3 = state[ch].zR2R3R1pR1C1C1pC2C3;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            const auto _t1 = (zR2R3R1pR1C1C1pC2C3 - ((gR0C0 * vi) - zR0C0));
            const auto vo = (((_t0 * zR2R3R1pR1C1C1pC2C3) - (_t1 * gR2R3R1pR1C1C1pC2C3)) * _t2);
            const auto v1 = (((gR2R3R1pR1C1C1pC2C3 * zR2R3R1pR1C1C1pC2C3) - (_t1 * _t3)) * _t2);
            const auto vR0C0 = (vi - v1);
            const auto vR2R3R1pR1C1C1pC2C3 = (vo - v1);
            
            zR0C0 = gzR0C0 * vR0C0 - zR0C0; // RC parallel
            zR2R3R1pR1C1C1pC2C3 = gR2R3R1pR1C1C1pC2C3 * (1 - gnR2R3R1pR1C1C1pC2C3) * vR2R3R1pR1C1C1pC2C3 + gnR2R3R1pR1C1C1pC2C3 * zR2R3R1pR1C1C1pC2C3; // RC series

            output[ch][n] = vo;
        }
        state[ch].zR0C0 = zR0C0;
        state[ch].zR2R3R1pR1C1C1pC2C3 = zR2R3R1pR1C1C1pC2C3;
    }
}
