// Auto-generated with netlist_codegen version 25c9320.
// Command: netlist_codegen multi_svf.net multi_svf.h

#pragma once

struct Params {
    float Rin = 1.0e+04f;
    float R1 = 1.0e+04f;
    float Rf1 = 1.5e+04f;
    float Cf1 = 9.999999e-08f;
    float Rf2 = 1.5e+04f;
    float Cf2 = 9.999999e-08f;
    float Rfb1 = 2.5e+03f;
    float Rfb2 = 1.0e+04f;
    float Rg = 1.52e+04f;
};

struct State {
    float zCf1 {};
    float zCf2 {};
};

static void compute (const float* const* input, float** output_vh, float** output_vb, float** output_vl, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto gRin = 1.0f / params.Rin;
    
    const auto gR1 = 1.0f / params.R1;
    
    const auto gRf1 = 1.0f / params.Rf1;
    
    const auto gCf1 = 2.0f * sample_rate * params.Cf1;
    
    const auto gRf2 = 1.0f / params.Rf2;
    
    const auto gCf2 = 2.0f * sample_rate * params.Cf2;
    
    const auto gRfb1 = 1.0f / params.Rfb1;
    
    const auto gRfb2 = 1.0f / params.Rfb2;
    
    const auto gRg = 1.0f / params.Rg;
    
    const auto _t0 = ((gR1 + gRfb2) + gRg);
    const auto _t2 = (gRin + gRfb1);
    const auto _t6 = (gCf1 * gCf2);
    const auto _t8 = (gRf1 * gRf2);
    const auto _t9 = (_t0 * (gRf1 * gCf2));
    const auto _t7 = (1.0 / ((_t2 * ((gR1 * _t6) + (gRfb2 * _t8))) + (gRfb1 * _t9)));
    const auto _t11 = (_t0 * _t6);
    const auto _t12 = (_t0 * _t8);
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zCf1 = state[ch].zCf1;
        auto zCf2 = state[ch].zCf2;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            const auto _t1 = (zCf1 * gCf2);
            const auto _t3 = (gCf1 * zCf2);
            const auto _t4 = (zCf1 * gRf2);
            const auto _t5 = (gRin * vi);
            const auto _t10 = (gRf1 * zCf2);
            const auto vh = (-(((gRfb1 * (_t0 * _t1)) - ((_t2 * (gRfb2 * (_t3 - _t4))) + (_t5 * _t11))) * _t7));
            const auto vb = (-(((_t2 * ((gR1 * _t1) + (gRfb2 * _t10))) + (_t5 * _t9)) * _t7));
            const auto vl = (-((((gRfb1 * (_t0 * _t10)) - (_t2 * (gR1 * (_t4 - _t3)))) - (_t5 * _t12)) * _t7));
            const auto tCf1 = (gCf1 * (0 - vb));
            const auto tCf2 = (gCf2 * (0 - vl));
            
            zCf1 = 2 * tCf1 - zCf1;
            zCf2 = 2 * tCf2 - zCf2;

            output_vh[ch][n] = vh;
            output_vb[ch][n] = vb;
            output_vl[ch][n] = vl;
        }
        state[ch].zCf1 = zCf1;
        state[ch].zCf2 = zCf2;
    }
}
