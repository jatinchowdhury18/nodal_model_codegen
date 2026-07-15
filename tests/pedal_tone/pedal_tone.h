// Auto-generated with netlist_codegen version e70acf8.
// Command: netlist_codegen pedal_tone.net pedal_tone.h

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
    
    const auto _t0 = ((gR8 + gC8) + gRtm);
    const auto _t5 = ((gC9 + gR5) + gRtp);
    const auto _t8 = (gRtp + gRtm);
    const auto _t4 = (_t5 * gRtm);
    const auto _t7 = ((_t5 * _t8) - (gRtp * gRtp));
    const auto _t6 = (1 / ((gRtm * _t4) - (_t0 * _t7)));
    const auto _t9 = (gRtp * gRtm);
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zC8 = state[ch].zC8;
        auto zC9 = state[ch].zC9;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            const auto _t2 = (zC9 - (gC9 * vi));
            const auto _t3 = ((gR8 * vi) + zC8);
            const auto _t1 = (_t2 * gRtp);
            const auto vo = (((_t0 * _t1) - (_t3 * _t4)) * _t6);
            const auto n1 = (((gRtm * _t1) - (_t3 * _t7)) * _t6);
            const auto tC8 = (gC8 * (n1 - 0));
            const auto n2 = ((((_t0 * (_t2 * _t8)) - (_t3 * _t9)) - (gRtm * (_t2 * gRtm))) * _t6);
            const auto tC9 = (gC9 * (vi - n2));
            
            zC8 = 2 * tC8 - zC8;
            zC9 = 2 * tC9 - zC9;

            output[ch][n] = vo;
        }
        state[ch].zC8 = zC8;
        state[ch].zC9 = zC9;
    }
}
