// Auto-generated with netlist_codegen version a65dc78.
// Command: netlist_codegen summing_amp.net summing_amp.h

#pragma once

struct Params {
    float R1 = 1.25e+05f;
    float R2 = 5.0e+04f;
    float R3 = 2.5e+04f;
    float Vdc = -1.5e+00f;
    float R4 = 5.0e+04f;
    float Vbias = -2.5e-01f;
    float RL = 1.0e+04f;
    float RF = 1.0e+04f;
    float Cf = 9.999999e-08f;
};

struct State {
    float zRFCf {};
};

static void compute (const float* const* input_vi1, const float* const* input_vi2, const float* const* input_vi3, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto gR1 = 1.0f / params.R1;
    
    const auto gR2 = 1.0f / params.R2;
    
    const auto gR3 = 1.0f / params.R3;
    
    const auto Vdc = params.Vdc;
    
    const auto vdc = Vdc;
    
    const auto gR4 = 1.0f / params.R4;
    
    const auto Vbias = params.Vbias;
    
    const auto NNON = Vbias;
    
    const auto gRL = 1.0f / params.RL;
    
    const auto RF = params.RF;
    const auto Cf = params.Cf;
    const auto gRFCf = 2.0f * sample_rate * Cf + (1.0f / RF);
    const auto gzRFCf = 4.0f * sample_rate * Cf;
    
    const auto _t0 = ((((((gR1 + gR2) + gR3) + gR4) + gRFCf) * Vbias) - (gR4 * Vdc));
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zRFCf = state[ch].zRFCf;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi1 = input_vi1[ch][n];
            const auto vi2 = input_vi2[ch][n];
            const auto vi3 = input_vi3[ch][n];

            const auto vo = ((_t0 + (zRFCf - (((gR1 * vi1) + (gR2 * vi2)) + (gR3 * vi3)))) / gRFCf);
            const auto NINV = Vbias;
            const auto vRFCf = (vo - NINV);
            
            zRFCf = gzRFCf * vRFCf - zRFCf; // RC parallel

            output[ch][n] = vo;
        }
        state[ch].zRFCf = zRFCf;
    }
}
