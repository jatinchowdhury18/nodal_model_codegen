// Auto-generated with netlist_codegen version d2c1c08.
// Command: netlist_codegen sk_lpf.net sk_lpf.h

#pragma once

struct Params {
    float R1 = 1.0e+04f;
    float R2 = 1.0e+04f;
    float C1 = 9.999999e-08f;
    float C2 = 9.999999e-08f;
    float Rf = 8.0e+03f;
    float Rg = 6.0e+03f;
    float Rload = 1.0e+05f;
};

struct State {
    float zC1 {};
    float zC2 {};
};

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto gR1 = 1.0f / params.R1;
    
    const auto gR2 = 1.0f / params.R2;
    
    const auto gC1 = 2.0f * sample_rate * params.C1;
    
    const auto gC2 = 2.0f * sample_rate * params.C2;
    
    const auto gRf = 1.0f / params.Rf;
    
    const auto gRg = 1.0f / params.Rg;
    
    const auto gRload = 1.0f / params.Rload;
    
    const auto _t0 = ((gR1 + gR2) + gC2);
    const auto _t2 = (gRf + gRg);
    const auto _t4 = (gR2 * _t2);
    const auto _t6 = ((gR2 + gC1) * gRf);
    const auto _t7 = (gR2 * gRf);
    const auto _t5 = (1 / (((_t0 * _t6) - (gC2 * _t4)) - (gR2 * _t7)));
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zC1 = state[ch].zC1;
        auto zC2 = state[ch].zC2;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            const auto _t3 = (zC2 - (gR1 * vi));
            const auto _t8 = (zC1 * gRf);
            const auto _t1 = (zC1 * _t2);
            const auto vo = (((_t0 * _t1) - (_t3 * _t4)) * _t5);
            const auto n3 = (((_t0 * _t8) - (_t3 * _t7)) * _t5);
            const auto tC1 = (gC1 * (n3 - 0));
            const auto n1 = ((((gC2 * _t1) - (_t3 * _t6)) + (gR2 * _t8)) * _t5);
            const auto tC2 = (gC2 * (vo - n1));
            
            zC1 = 2 * tC1 - zC1;
            zC2 = 2 * tC2 - zC2;

            output[ch][n] = vo;
        }
        state[ch].zC1 = zC1;
        state[ch].zC2 = zC2;
    }
}
