// Auto-generated with netlist_codegen version ce593e9.
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
    
    const auto _t0 = (((gR1 + gR2) + gC2) * 128.0f);
    const auto _t3 = ((gRf + gRg) * 8192.0f);
    const auto _t6 = (gR2 * 128.0f);
    const auto _t9 = (gRf * 8192.0f);
    const auto _t10 = (gC2 * 128.0f);
    const auto _t11 = (_t6 * _t9);
    const auto _t5 = (_t6 * _t3);
    const auto _t8 = (((gR2 + gC1) * 128.0f) * _t9);
    const auto _t7 = (1.0f / (((_t0 * _t8) - (_t10 * _t5)) - (_t6 * _t11)));
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zC1 = state[ch].zC1;
        auto zC2 = state[ch].zC2;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            const auto _t2 = (zC1 * 128.0f);
            const auto _t4 = ((zC2 - (gR1 * vi)) * 128.0f);
            const auto _t12 = (_t2 * _t9);
            const auto _t1 = (_t2 * _t3);
            const auto vo = (((_t0 * _t1) - (_t4 * _t5)) * _t7);
            const auto n3 = (((_t0 * _t12) - (_t4 * _t11)) * _t7);
            const auto tC1 = (gC1 * (n3 - 0));
            const auto n1 = ((((_t10 * _t1) - (_t4 * _t8)) + (_t6 * _t12)) * _t7);
            const auto tC2 = (gC2 * (vo - n1));
            
            zC1 = 2 * tC1 - zC1;
            zC2 = 2 * tC2 - zC2;

            output[ch][n] = vo;
        }
        state[ch].zC1 = zC1;
        state[ch].zC2 = zC2;
    }
}

static float reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc = 0.0f)
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
    
    const auto vi = vi_dc;

    const auto zC1 = ((gC1 * ((gR1 * vi) * (gR2 * gRf))) / ((((gR1 + gR2) + (1.0f / 1000000000.0f)) * (gR2 * gRf)) - (gR2 * (gR2 * gRf))));
    const auto zC2 = (gC2 * ((((gR1 * vi) * (gR2 * ((gRf + gRg) + (1.0f / 1000000000.0f)))) / ((((gR1 + gR2) + (1.0f / 1000000000.0f)) * (gR2 * gRf)) - (gR2 * (gR2 * gRf)))) - (((gR1 * vi) * (gR2 * gRf)) / ((((gR1 + gR2) + (1.0f / 1000000000.0f)) * (gR2 * gRf)) - (gR2 * (gR2 * gRf))))));

    const auto vo_dc_out = (((gR1 * vi) * (gR2 * ((gRf + gRg) + (1.0f / 1000000000.0f)))) / ((((gR1 + gR2) + (1.0f / 1000000000.0f)) * (gR2 * gRf)) - (gR2 * (gR2 * gRf))));

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].zC1 = zC1;
        state[ch].zC2 = zC2;
    }
    return vo_dc_out;
}

