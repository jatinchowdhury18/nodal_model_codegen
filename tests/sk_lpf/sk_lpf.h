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
    
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zC1 = state[ch].zC1;
        auto zC2 = state[ch].zC2;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            const auto vo = (((((((-(gC1 * 0)) - zC1) * (((-gR1) - gR2) + (-gC2))) - (((-(gR1 * vi)) + zC2) * gR2)) * ((-(((-gRf) - gRg) * (gC2 * gR2))) - (((((-gR2) - gC1) * (((-gR1) - gR2) + (-gC2))) - (gR2 * gR2)) * gRf))) - ((((gRg * 0) * (gC2 * gR2)) - (((((-(gC1 * 0)) - zC1) * (((-gR1) - gR2) + (-gC2))) - (((-(gR1 * vi)) + zC2) * gR2)) * gRf)) * ((((-gR2) - gC1) * (((-gR1) - gR2) + (-gC2))) - (gR2 * gR2)))) / ((-((gC2 * gR2) * ((-(((-gRf) - gRg) * (gC2 * gR2))) - (((((-gR2) - gC1) * (((-gR1) - gR2) + (-gC2))) - (gR2 * gR2)) * gRf)))) - (((-(gRf * (gC2 * gR2))) - (-((gC2 * gR2) * gRf))) * ((((-gR2) - gC1) * (((-gR1) - gR2) + (-gC2))) - (gR2 * gR2)))));
            const auto n3 = ((((gRg * 0) * (gC2 * gR2)) - (((((-(gC1 * 0)) - zC1) * (((-gR1) - gR2) + (-gC2))) - (((-(gR1 * vi)) + zC2) * gR2)) * gRf)) / ((-(((-gRf) - gRg) * (gC2 * gR2))) - (((((-gR2) - gC1) * (((-gR1) - gR2) + (-gC2))) - (gR2 * gR2)) * gRf)));
            const auto tC1 = (gC1 * (n3 - 0));
            const auto n1 = (((((-(((-(gR1 * vi)) + zC2) * (gC2 * gR2))) - (((((-(gC1 * 0)) - zC1) * (((-gR1) - gR2) + (-gC2))) - (((-(gR1 * vi)) + zC2) * gR2)) * gC2)) * ((-(((-gRf) - gRg) * (gC2 * gR2))) - (((((-gR2) - gC1) * (((-gR1) - gR2) + (-gC2))) - (gR2 * gR2)) * gRf))) - ((((gRg * 0) * (gC2 * gR2)) - (((((-(gC1 * 0)) - zC1) * (((-gR1) - gR2) + (-gC2))) - (((-(gR1 * vi)) + zC2) * gR2)) * gRf)) * ((-(gR2 * (gC2 * gR2))) - (((((-gR2) - gC1) * (((-gR1) - gR2) + (-gC2))) - (gR2 * gR2)) * gC2)))) / ((((-((((-gR1) - gR2) + (-gC2)) * (gC2 * gR2))) - (((gR2 * (((-gR1) - gR2) + (-gC2))) - ((((-gR1) - gR2) + (-gC2)) * gR2)) * gC2)) * ((-(((-gRf) - gRg) * (gC2 * gR2))) - (((((-gR2) - gC1) * (((-gR1) - gR2) + (-gC2))) - (gR2 * gR2)) * gRf))) - (-((((gR2 * (((-gR1) - gR2) + (-gC2))) - ((((-gR1) - gR2) + (-gC2)) * gR2)) * gRf) * ((-(gR2 * (gC2 * gR2))) - (((((-gR2) - gC1) * (((-gR1) - gR2) + (-gC2))) - (gR2 * gR2)) * gC2))))));
            const auto tC2 = (gC2 * (vo - n1));
            
            zC1 = 2 * tC1 - zC1;
            zC2 = 2 * tC2 - zC2;

            output[ch][n] = vo;
        }
        state[ch].zC1 = zC1;
        state[ch].zC2 = zC2;
    }
}
