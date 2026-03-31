#pragma once

struct Params {
    float Rl;
    float R0;
    float C0;
    float R1p;
    float R1;
    float R2;
    float R3;
    float C1;
    float C3;
    float C2;
    float C1p;
};

struct State {
    float zR0C0 {};
    float zR3R2R1R1pC2C3C1C1p {};
};

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto gRl = 1.0f / params.Rl;
    
    const auto R0 = params.R0;
    const auto C0 = params.C0;
    const auto gR0C0 = 2.0f * sample_rate * C0 + (1.0f / R0);
    const auto gzR0C0 = 4.0f * sample_rate * C0;
    
    const auto R3R2R1R1p = sum(params.R3, sum(params.R2, recip_sum(params.R1, params.R1p)));
    const auto C2C3C1C1p = recip_sum(recip_sum(params.C2, params.C3), sum(params.C1, params.C1p));
    const auto gR3R2R1R1pC2C3C1C1p = (2.0f * sample_rate * C2C3C1C1p) / (1.0f + 2.0f * sample_rate * R3R2R1R1p * C2C3C1C1p);
    const auto gnR3R2R1R1pC2C3C1C1p = gR3R2R1R1pC2C3C1C1p * (2.0f * sample_rate * R3R2R1R1p * C2C3C1C1p - 1.0f) / (2.0f * sample_rate * C2C3C1C1p);
    
    const auto temp3 = gR3R2R1R1pC2C3C1C1p*gRl;
    const auto temp4 = gR3R2R1R1pC2C3C1C1p + gRl;
    const auto temp5 = gR0C0*temp4;
    const auto temp6 = temp3 + temp5;
    const auto temp7 = 1/temp6;

    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zR0C0 = state[ch].zR0C0;
        auto zR3R2R1R1pC2C3C1C1p = state[ch].zR3R2R1R1pC2C3C1C1p;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            const auto temp15 = gR0C0*vi;
            const auto temp16 = temp15 + zR0C0;
            const auto temp17 = temp4*temp16;
            const auto temp18 = -(gRl*zR3R2R1R1pC2C3C1C1p);
            const auto temp19 = temp17 + temp18;
            const auto temp8 = gR3R2R1R1pC2C3C1C1p*zR0C0;
            const auto temp9 = gR3R2R1R1pC2C3C1C1p*vi;
            const auto temp10 = temp9 + zR3R2R1R1pC2C3C1C1p;
            const auto temp11 = gR0C0*temp10;
            const auto temp12 = temp8 + temp11;
            const auto temp13 = temp7*temp12;
            
            const auto vo = temp13;
            const auto vR0C0 = -vi + temp7*temp19;
            const auto vR3R2R1R1pC2C3C1C1p = -(temp7*temp19) + temp13;
            
            zR0C0 = gzR0C0 * vR0C0 - zR0C0; // RC parallel
            zR3R2R1R1pC2C3C1C1p = gR3R2R1R1pC2C3C1C1p * (1 - gnR3R2R1R1pC2C3C1C1p) * vR3R2R1R1pC2C3C1C1p + gnR3R2R1R1pC2C3C1C1p * zR3R2R1R1pC2C3C1C1p; // RC series

            output[ch][n] = vo;
        }
        state[ch].zR0C0 = zR0C0;
        state[ch].zR3R2R1R1pC2C3C1C1p = zR3R2R1R1pC2C3C1C1p;
    }
}
