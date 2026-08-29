// Auto-generated with netlist_codegen version 58e1e0e.
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
    
    const auto _t0 = (gRfb1 * 2048.0f);
    const auto _t1 = (((gR1 + gRfb2) + gRg) * 8192.0f);
    const auto _t4 = (gCf2 * 128.0f);
    const auto _t5 = ((gRin + gRfb1) * 2048.0f);
    const auto _t6 = (gRfb2 * 8192.0f);
    const auto _t8 = (gCf1 * 128.0f);
    const auto _t9 = (gRf2 * 128.0f);
    const auto _t11 = (_t8 * _t4);
    const auto _t13 = (gR1 * 8192.0f);
    const auto _t15 = (gRf1 * 128.0f);
    const auto _t16 = (_t1 * (_t15 * _t4));
    const auto _t14 = (_t15 * _t9);
    const auto _t12 = (1.0f / ((_t5 * ((_t13 * _t11) + (_t6 * _t14))) + (_t0 * _t16)));
    const auto _t20 = (_t1 * _t11);
    const auto _t21 = (_t1 * _t14);
    float c0_tCf2;
    float c_tCf2[3];
    float c0_tCf1;
    float c_tCf1[3];
    float c0_vh;
    float c_vh[3];
    float c0_vl;
    float c_vl[3];
    float c0_vb;
    float c_vb[3];
    
    for (int _k = 0; _k <= 3; ++_k)
    {
        const auto vi = (_k == 1) ? 1.0f : 0.0f;
        const auto zCf1 = (_k == 2) ? 1.0f : 0.0f;
        const auto zCf2 = (_k == 3) ? 1.0f : 0.0f;
        const auto _t3 = (zCf1 * 128.0f);
        const auto _t10 = ((gRin * vi) * 2048.0f);
        const auto _t18 = (zCf2 * 128.0f);
        const auto _t19 = (_t3 * _t9);
        const auto _t2 = (_t3 * _t4);
        const auto _t7 = (_t8 * _t18);
        const auto _t17 = (_t15 * _t18);
        const auto vh = (-(((_t0 * (_t1 * _t2)) - ((_t5 * (_t6 * (_t7 - _t19))) + (_t10 * _t20))) * _t12));
        const auto vb = (-(((_t5 * ((_t13 * _t2) + (_t6 * _t17))) + (_t10 * _t16)) * _t12));
        const auto vl = (-((((_t0 * (_t1 * _t17)) - (_t5 * (_t13 * (_t19 - _t7)))) - (_t10 * _t21)) * _t12));
        const auto tCf1 = (gCf1 * (0 - vb));
        const auto tCf2 = (gCf2 * (0 - vl));
        if (_k == 0) {
            c0_tCf2 = tCf2;
            c0_tCf1 = tCf1;
            c0_vh = vh;
            c0_vl = vl;
            c0_vb = vb;
        } else {
            c_tCf2[_k - 1] = tCf2 - c0_tCf2;
            c_tCf1[_k - 1] = tCf1 - c0_tCf1;
            c_vh[_k - 1] = vh - c0_vh;
            c_vl[_k - 1] = vl - c0_vl;
            c_vb[_k - 1] = vb - c0_vb;
        }
    }
    
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zCf1 = state[ch].zCf1;
        auto zCf2 = state[ch].zCf2;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            const auto tCf2 = c0_tCf2 + c_tCf2[0] * vi + c_tCf2[1] * zCf1 + c_tCf2[2] * zCf2;
            const auto tCf1 = c0_tCf1 + c_tCf1[0] * vi + c_tCf1[1] * zCf1 + c_tCf1[2] * zCf2;
            const auto vh = c0_vh + c_vh[0] * vi + c_vh[1] * zCf1 + c_vh[2] * zCf2;
            const auto vl = c0_vl + c_vl[0] * vi + c_vl[1] * zCf1 + c_vl[2] * zCf2;
            const auto vb = c0_vb + c_vb[0] * vi + c_vb[1] * zCf1 + c_vb[2] * zCf2;
            
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

static void reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc = 0.0f)
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
    
    const auto vi = vi_dc;

    const auto zCf1 = 0.0f;
    const auto zCf2 = (-((gCf2 * ((gRin * vi) * (((gR1 + gRfb2) + gRg) + (1.0f / 1000000000.0f)))) / ((gRin + gRfb1) * gRfb2)));

    const auto vh_dc_out = 0.0f;
    const auto vb_dc_out = 0.0f;
    const auto vl_dc_out = (((gRin * vi) * (((gR1 + gRfb2) + gRg) + (1.0f / 1000000000.0f))) / ((gRin + gRfb1) * gRfb2));

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].zCf1 = zCf1;
        state[ch].zCf2 = zCf2;
    }
}

