#pragma once

struct Params {
    double R1 = 1.0e+04f;
    double R2 = 1.0e+04f;
    double C1 = 9.999999e-08f;
    double C2 = 9.999999e-08f;
    double Rf = 8.0e+03f;
    double Rg = 6.0e+03f;
    double Rload = 1.0e+05f;
};

struct State {
    double zC1 {};
    double zC2 {};
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
    
    const auto _t0 = (gC2 * gR2);
    const auto _t1 = ((gR1 + gR2) + gC2);
    const auto _t3 = (((gR2 + gC1) * _t1) - (gR2 * gR2));
    const auto _t6 = (((gRf + gRg) * _t0) - (_t3 * gRf));
    const auto _t7 = (1 / (_t0 * _t6));
    const auto _t8 = ((gR2 * _t0) + (_t3 * gC2));
    const auto _t9 = (1 / (((_t1 * gC2) * gR2) * _t6));
    const auto _t10 = (1 / _t6);
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zC1 = state[ch].zC1;
        auto zC2 = state[ch].zC2;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            const auto _t2 = (zC2 - (gR1 * vi));
            const auto _t4 = ((zC1 * _t1) - (_t2 * gR2));
            const auto _t5 = (_t4 * gRf);
            const auto vo = (-(((_t4 * _t6) + (_t5 * _t3)) * _t7));
            const auto n3 = (-(_t5 * _t10));
            const auto tC1 = (gC1 * (n3 - 0));
            const auto n1 = (-(((((_t2 * _t0) + (_t4 * gC2)) * _t6) + (_t5 * _t8)) * _t9));
            const auto tC2 = (gC2 * (vo - n1));
            
            zC1 = 2 * tC1 - zC1;
            zC2 = 2 * tC2 - zC2;

            output[ch][n] = vo;
        }
        state[ch].zC1 = zC1;
        state[ch].zC2 = zC2;
    }
}
