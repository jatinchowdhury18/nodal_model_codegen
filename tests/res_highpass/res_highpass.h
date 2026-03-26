#pragma once

struct Params {
    float C21;
    float C22;
    float L1;
    float R9;
};

struct State {
    float zC21;
    float zC22;
    float zL1;
};

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    const auto gC21 = 2.0f * sample_rate * params.C21;
    const auto gC22 = 2.0f * sample_rate * params.C22;
    const auto gL1 = 1.0f / (2.0f * sample_rate * params.L1);
    const auto gR9 = 1.0f / params.R9;
    
    const auto temp3 = gC21 + gL1;
    const auto temp4 = gC22*temp3;
    const auto temp5 = gC21 + gC22 + gL1;
    const auto temp6 = temp5*gR9;
    const auto temp7 = temp4 + temp6;
    const auto temp8 = 1/temp7;
    const auto temp17 = gC22 + gR9;

    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zC21 = state[ch].zC21;
        auto zC22 = state[ch].zC22;
        auto zL1 = state[ch].zL1;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            const auto temp13 = zC21 + zL1;
            const auto temp9 = gL1*zC22;
            const auto temp10 = -(gC22*vi);
            const auto temp11 = temp10 + zC22;
            const auto temp12 = gC21*temp11;
            const auto temp14 = gC22*temp13;
            const auto temp15 = temp9 + temp12 + temp14;
            const auto temp18 = gC21*temp17*vi;
            const auto temp19 = -(gC22*temp13);
            const auto temp20 = -zC22;
            const auto temp21 = zC21 + temp20 + zL1;
            const auto temp22 = -(gR9*temp21);
            const auto temp23 = temp18 + temp19 + temp22;
            
            const auto vo = -(temp8*temp15);
            const auto tC21 = gC21*(vi - temp8*temp23);
            const auto tC22 = gC22*(temp8*temp15 + temp8*temp23);
            const auto tL1 = gL1*temp8*temp23;
            
            zC21 = 2 * tC21 - zC21;
            zC22 = 2 * tC22 - zC22;
            zL1 = 2 * tL1 + zL1;

            output[ch][n] = vo;
        }
        state[ch].zC21 = zC21;
        state[ch].zC22 = zC22;
        state[ch].zL1 = zL1;
    }
}
