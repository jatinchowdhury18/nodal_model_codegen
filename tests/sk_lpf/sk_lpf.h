#pragma once

struct Params {
    float R1;
    float R2;
    float C1;
    float C2;
    float Rf;
    float Rg;
    float Rload;
};

struct State {
    float zC1 {};
    float zC2 {};
};

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    const auto gR1 = 1.0f / params.R1;
    const auto gR2 = 1.0f / params.R2;
    const auto gC1 = 2.0f * sample_rate * params.C1;
    const auto gC2 = 2.0f * sample_rate * params.C2;
    const auto gRf = 1.0f / params.Rf;
    const auto gRg = 1.0f / params.Rg;
    const auto gRload = 1.0f / params.Rload;
    
    const auto temp4 = gR1*gR2*gRf;
    const auto temp5 = gC2 + gR1 + gR2;
    const auto temp6 = gC1*temp5*gRf;
    const auto temp7 = -(gC2*gR2*gRg);
    const auto temp8 = temp4 + temp6 + temp7;
    const auto temp9 = 1/temp8;
    const auto temp10 = gC2 + gR2;
    const auto temp3 = gRf + gRg;

    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zC1 = state[ch].zC1;
        auto zC2 = state[ch].zC2;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            const auto temp11 = temp10*zC1;
            const auto temp12 = gR2*vi;
            const auto temp13 = temp12 + zC1;
            const auto temp14 = gR1*temp13;
            const auto temp15 = -(gR2*zC2);
            const auto temp16 = temp11 + temp14 + temp15;
            const auto temp17 = temp3*temp9*temp16;
            
            const auto vo = temp17;
            const auto tC1 = gC1*gRf*temp9*temp16;
            const auto tC2 = gC2*(temp17 - temp9*(gC1*gR1*gRf*vi + gR1*gR2*gRf*vi + gC2*gRf*zC1 + gR2*gRf*zC1 + gC2*gRg*zC1 - (gC1 + gR2)*gRf*zC2));
            
            zC1 = 2 * tC1 - zC1;
            zC2 = 2 * tC2 - zC2;

            output[ch][n] = vo;
        }
        state[ch].zC1 = zC1;
        state[ch].zC2 = zC2;
    }
}
