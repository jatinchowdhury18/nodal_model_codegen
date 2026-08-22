// Auto-generated with netlist_codegen version ac50416.
// Command: netlist_codegen rc_lowpass.net rc_lowpass.h -opt_port_matrix

#pragma once

struct Params {
    float R1 = 1.0e+03f;
    float C1 = 1.0e-06f;
};

struct State {
    float zC1 {};
};

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto gR1 = 1.0f / params.R1;
    
    const auto gC1 = 2.0f * sample_rate * params.C1;
    
    const auto _t0 = (1.0f / (gR1 + gC1));
    float c0_tC1;
    float c_tC1[2];
    
    for (int _k = 0; _k <= 2; ++_k)
    {
        const auto vi = (_k == 1) ? 1.0f : 0.0f;
        const auto zC1 = (_k == 2) ? 1.0f : 0.0f;
        const auto vo = (((gR1 * vi) + zC1) * _t0);
        const auto tC1 = (gC1 * (vo - 0));
        if (_k == 0) {
            c0_tC1 = tC1;
        } else {
            c_tC1[_k - 1] = tC1 - c0_tC1;
        }
    }
    
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zC1 = state[ch].zC1;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            const auto tC1 = c0_tC1 + c_tC1[0] * vi + c_tC1[1] * zC1;
            const auto vo = (((gR1 * vi) + zC1) * _t0);
            
            zC1 = 2 * tC1 - zC1;

            output[ch][n] = vo;
        }
        state[ch].zC1 = zC1;
    }
}

static float reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc = 0.0f)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto gR1 = 1.0f / params.R1;
    
    const auto gC1 = 2.0f * sample_rate * params.C1;
    
    const auto vi = vi_dc;

    const auto zC1 = ((gC1 * (gR1 * vi)) / (gR1 + (1.0f / 1000000000.0f)));

    const auto vo_dc_out = ((gR1 * vi) / (gR1 + (1.0f / 1000000000.0f)));

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].zC1 = zC1;
    }
    return vo_dc_out;
}

