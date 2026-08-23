// Auto-generated with netlist_codegen version 5608cd2.
// Command: netlist_codegen reductions.net reductions.h -opt_port_matrix

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
    
    const auto _t0 = ((gR0C0 + gR2R3R1pR1C1C1pC2C3) * 8.0f);
    const auto _t3 = (gR2R3R1pR1C1C1pC2C3 * 8192.0f);
    const auto _t5 = ((gRl + gR2R3R1pR1C1C1pC2C3) * 8192.0f);
    const auto _t6 = (gR2R3R1pR1C1C1pC2C3 * 8.0f);
    const auto _t4 = (1.0f / ((_t0 * _t5) - (_t6 * _t3)));
    float c0_vR2R3R1pR1C1C1pC2C3;
    float c_vR2R3R1pR1C1C1pC2C3[3];
    float c0_vR0C0;
    float c_vR0C0[3];
    
    for (int _k = 0; _k <= 3; ++_k)
    {
        const auto vi = (_k == 1) ? 1.0f : 0.0f;
        const auto zR0C0 = (_k == 2) ? 1.0f : 0.0f;
        const auto zR2R3R1pR1C1C1pC2C3 = (_k == 3) ? 1.0f : 0.0f;
        const auto _t1 = (zR2R3R1pR1C1C1pC2C3 * 8192.0f);
        const auto _t2 = ((zR2R3R1pR1C1C1pC2C3 - ((gR0C0 * vi) - zR0C0)) * 8.0f);
        const auto vo = (((_t0 * _t1) - (_t2 * _t3)) * _t4);
        const auto v1 = (((_t6 * _t1) - (_t2 * _t5)) * _t4);
        const auto vR0C0 = (vi - v1);
        const auto vR2R3R1pR1C1C1pC2C3 = (vo - v1);
        if (_k == 0) {
            c0_vR2R3R1pR1C1C1pC2C3 = vR2R3R1pR1C1C1pC2C3;
            c0_vR0C0 = vR0C0;
        } else {
            c_vR2R3R1pR1C1C1pC2C3[_k - 1] = vR2R3R1pR1C1C1pC2C3 - c0_vR2R3R1pR1C1C1pC2C3;
            c_vR0C0[_k - 1] = vR0C0 - c0_vR0C0;
        }
    }
    
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zR0C0 = state[ch].zR0C0;
        auto zR2R3R1pR1C1C1pC2C3 = state[ch].zR2R3R1pR1C1C1pC2C3;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            const auto vR2R3R1pR1C1C1pC2C3 = c0_vR2R3R1pR1C1C1pC2C3 + c_vR2R3R1pR1C1C1pC2C3[0] * vi + c_vR2R3R1pR1C1C1pC2C3[1] * zR0C0 + c_vR2R3R1pR1C1C1pC2C3[2] * zR2R3R1pR1C1C1pC2C3;
            const auto vR0C0 = c0_vR0C0 + c_vR0C0[0] * vi + c_vR0C0[1] * zR0C0 + c_vR0C0[2] * zR2R3R1pR1C1C1pC2C3;
            const auto _t1 = (zR2R3R1pR1C1C1pC2C3 * 8192.0f);
            const auto _t2 = ((zR2R3R1pR1C1C1pC2C3 - ((gR0C0 * vi) - zR0C0)) * 8.0f);
            const auto vo = (((_t0 * _t1) - (_t2 * _t3)) * _t4);
            const auto v1 = (((_t6 * _t1) - (_t2 * _t5)) * _t4);
            
            zR0C0 = gzR0C0 * vR0C0 - zR0C0; // RC parallel
            zR2R3R1pR1C1C1pC2C3 = gR2R3R1pR1C1C1pC2C3 * (1 - gnR2R3R1pR1C1C1pC2C3) * vR2R3R1pR1C1C1pC2C3 + gnR2R3R1pR1C1C1pC2C3 * zR2R3R1pR1C1C1pC2C3; // RC series

            output[ch][n] = vo;
        }
        state[ch].zR0C0 = zR0C0;
        state[ch].zR2R3R1pR1C1C1pC2C3 = zR2R3R1pR1C1C1pC2C3;
    }
}

static float reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc = 0.0f)
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
    
    const auto vi = vi_dc;

    const auto zR0C0 = ((gzR0C0 * (vi + ((vi / R0) / ((-1.0f / R0) - (1.0f / 1000000000.0f))))) / 2.0f);
    const auto zR2R3R1pR1C1C1pC2C3 = (((gR2R3R1pR1C1C1pC2C3 * vi) / R0) / ((-1.0f / R0) - (1.0f / 1000000000.0f)));

    const auto vo_dc_out = 0.0f;

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].zR0C0 = zR0C0;
        state[ch].zR2R3R1pR1C1C1pC2C3 = zR2R3R1pR1C1C1pC2C3;
    }
    return vo_dc_out;
}

