// Auto-generated with netlist_codegen version de49196.
// Command: netlist_codegen pedal_tone.net pedal_tone.h -opt_port_matrix

#pragma once

struct Params {
    float R8 = 3.9e+04f;
    float C8 = 1.0e-08f;
    float C9 = 4.0e-09f;
    float R5 = 2.2e+04f;
    float Rtp = 7.0e+04f;
    float Rtm = 3.0e+04f;
};

struct State {
    float zC8 {};
    float zC9 {};
};

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto gR8 = 1.0f / params.R8;
    
    const auto gC8 = 2.0f * sample_rate * params.C8;
    
    const auto gC9 = 2.0f * sample_rate * params.C9;
    
    const auto gR5 = 1.0f / params.R5;
    
    const auto gRtp = 1.0f / params.Rtp;
    
    const auto gRtm = 1.0f / params.Rtm;
    
    const auto _t0 = (((gR8 + gC8) + gRtm) * 1024.0f);
    const auto _t2 = (gRtp * 32768.0f);
    const auto _t5 = (((gC9 + gR5) + gRtp) * 2048.0f);
    const auto _t6 = (gRtm * 32768.0f);
    const auto _t8 = (gRtm * 1024.0f);
    const auto _t10 = ((gRtp + gRtm) * 32768.0f);
    const auto _t11 = (gRtp * 2048.0f);
    const auto _t4 = (_t5 * _t6);
    const auto _t9 = ((_t5 * _t10) - (_t11 * _t2));
    const auto _t7 = (1.0f / ((_t8 * _t4) - (_t0 * _t9)));
    const auto _t13 = (_t11 * _t6);
    float c0_tC9;
    float c_tC9[3];
    float c0_n2;
    float c_n2[3];
    float c0_tC8;
    float c_tC8[3];
    float c0_vo;
    float c_vo[3];
    
    for (int _k = 0; _k <= 3; ++_k)
    {
        const auto vi = (_k == 1) ? 1.0f : 0.0f;
        const auto zC8 = (_k == 2) ? 1.0f : 0.0f;
        const auto zC9 = (_k == 3) ? 1.0f : 0.0f;
        const auto _t1 = ((zC9 - (gC9 * vi)) * 2048.0f);
        const auto _t3 = (((gR8 * vi) + zC8) * 1024.0f);
        const auto _t12 = (_t1 * _t2);
        const auto vo = (((_t0 * _t12) - (_t3 * _t4)) * _t7);
        const auto n1 = (((_t8 * _t12) - (_t3 * _t9)) * _t7);
        const auto tC8 = (gC8 * (n1 - 0));
        const auto n2 = ((((_t0 * (_t1 * _t10)) - (_t3 * _t13)) - (_t8 * (_t1 * _t6))) * _t7);
        const auto tC9 = (gC9 * (vi - n2));
        if (_k == 0) {
            c0_tC9 = tC9;
            c0_n2 = n2;
            c0_tC8 = tC8;
            c0_vo = vo;
        } else {
            c_tC9[_k - 1] = tC9 - c0_tC9;
            c_n2[_k - 1] = n2 - c0_n2;
            c_tC8[_k - 1] = tC8 - c0_tC8;
            c_vo[_k - 1] = vo - c0_vo;
        }
    }
    
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zC8 = state[ch].zC8;
        auto zC9 = state[ch].zC9;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            const auto tC9 = c0_tC9 + c_tC9[0] * vi + c_tC9[1] * zC8 + c_tC9[2] * zC9;
            const auto n2 = c0_n2 + c_n2[0] * vi + c_n2[1] * zC8 + c_n2[2] * zC9;
            const auto tC8 = c0_tC8 + c_tC8[0] * vi + c_tC8[1] * zC8 + c_tC8[2] * zC9;
            const auto vo = c0_vo + c_vo[0] * vi + c_vo[1] * zC8 + c_vo[2] * zC9;
            const auto _t1 = ((zC9 - (gC9 * vi)) * 2048.0f);
            const auto _t3 = (((gR8 * vi) + zC8) * 1024.0f);
            const auto _t12 = (_t1 * _t2);
            const auto n1 = (((_t8 * _t12) - (_t3 * _t9)) * _t7);
            
            zC8 = 2 * tC8 - zC8;
            zC9 = 2 * tC9 - zC9;

            output[ch][n] = vo;
        }
        state[ch].zC8 = zC8;
        state[ch].zC9 = zC9;
    }
}

static float reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc = 0.0f)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto gR8 = 1.0f / params.R8;
    
    const auto gC8 = 2.0f * sample_rate * params.C8;
    
    const auto gC9 = 2.0f * sample_rate * params.C9;
    
    const auto gR5 = 1.0f / params.R5;
    
    const auto gRtp = 1.0f / params.Rtp;
    
    const auto gRtm = 1.0f / params.Rtm;
    
    const auto vi = vi_dc;

    const auto zC8 = (-((gC8 * ((gR8 * vi) * ((((gR5 + gRtp) + (1.0f / 1000000000.0f)) * ((gRtp + gRtm) + (1.0f / 1000000000.0f))) - (gRtp * gRtp)))) / ((gRtm * (((gR5 + gRtp) + (1.0f / 1000000000.0f)) * gRtm)) - (((gR8 + gRtm) + (1.0f / 1000000000.0f)) * ((((gR5 + gRtp) + (1.0f / 1000000000.0f)) * ((gRtp + gRtm) + (1.0f / 1000000000.0f))) - (gRtp * gRtp))))));
    const auto zC9 = (gC9 * (vi + (((gR8 * vi) * (gRtp * gRtm)) / ((gRtm * (((gR5 + gRtp) + (1.0f / 1000000000.0f)) * gRtm)) - (((gR8 + gRtm) + (1.0f / 1000000000.0f)) * ((((gR5 + gRtp) + (1.0f / 1000000000.0f)) * ((gRtp + gRtm) + (1.0f / 1000000000.0f))) - (gRtp * gRtp)))))));

    const auto vo_dc_out = (-(((gR8 * vi) * (((gR5 + gRtp) + (1.0f / 1000000000.0f)) * gRtm)) / ((gRtm * (((gR5 + gRtp) + (1.0f / 1000000000.0f)) * gRtm)) - (((gR8 + gRtm) + (1.0f / 1000000000.0f)) * ((((gR5 + gRtp) + (1.0f / 1000000000.0f)) * ((gRtp + gRtm) + (1.0f / 1000000000.0f))) - (gRtp * gRtp))))));

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].zC8 = zC8;
        state[ch].zC9 = zC9;
    }
    return vo_dc_out;
}

