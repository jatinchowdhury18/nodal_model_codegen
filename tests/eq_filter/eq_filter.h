#pragma once

struct Params {
    float R33 = 4.750e+03f;
    float R37 = 4.750e+03f;
    float R35 = 4.750e+03f;
    float R40 = 3.500e+03f;
    float R39 = 2.350e+04f;
    float R66 = 1.200e+04f;
    float R44 = 1.000e+03f;
    float R47 = 7.500e+02f;
    float R48 = 3.300e+03f;
    float R45 = 1.000e+03f;
    float C20 = 1.000e-04f;
    float R38 = 4.700e+03f;
    float R41 = 1.800e+03f;
    float R42 = 2.430e+04f;
    float C16 = 3.300e-08f;
    float C17 = 4.700e-08f;
    float R43 = 2.510e+04f;
    float C18 = 3.300e-08f;
    float C19 = 4.700e-08f;
};

struct State {
    float zR45C20 {};
    float zR42C16C17 {};
    float zR43C18C19 {};
};

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto gR33 = 1.0f / params.R33;
    
    const auto gR37 = 1.0f / params.R37;
    
    const auto gR35 = 1.0f / params.R35;
    
    const auto gR40 = 1.0f / params.R40;
    
    const auto gR39 = 1.0f / params.R39;
    
    const auto gR66 = 1.0f / params.R66;
    
    const auto gR44 = 1.0f / params.R44;
    
    const auto gR47 = 1.0f / params.R47;
    
    const auto gR48 = 1.0f / params.R48;
    
    const auto R45 = params.R45;
    const auto C20 = params.C20;
    const auto gR45C20 = (2.0f * sample_rate * C20) / (1.0f + 2.0f * sample_rate * R45 * C20);
    const auto gnR45C20 = gR45C20 * (2.0f * sample_rate * R45 * C20 - 1.0f) / (2.0f * sample_rate * C20);
    
    const auto R41R38 = recip_sum(params.R41, params.R38);
    const auto gR41R38 = 1.0f / R41R38;
    
    const auto R42 = params.R42;
    const auto C16C17 = sum(params.C16, params.C17);
    const auto gR42C16C17 = (2.0f * sample_rate * C16C17) / (1.0f + 2.0f * sample_rate * R42 * C16C17);
    const auto gnR42C16C17 = gR42C16C17 * (2.0f * sample_rate * R42 * C16C17 - 1.0f) / (2.0f * sample_rate * C16C17);
    
    const auto R43 = params.R43;
    const auto C18C19 = sum(params.C18, params.C19);
    const auto gR43C18C19 = 2.0f * sample_rate * C18C19 + (1.0f / R43);
    const auto gzR43C18C19 = 4.0f * sample_rate * C18C19;
    
    const auto temp3 = gR44 + gR45C20;
    const auto temp4 = gR37*gR40*gR42C16C17*temp3*gR47;
    const auto temp5 = gR39 + gR40 + gR41R38 + gR42C16C17;
    const auto temp6 = temp5*gR43C18C19*gR44;
    const auto temp7 = gR39 + gR40 + gR41R38;
    const auto temp8 = -(temp7*gR42C16C17*gR45C20);
    const auto temp9 = temp6 + temp8;
    const auto temp10 = gR35*temp9*gR48;
    const auto temp11 = gR42C16C17*temp3*gR47;
    const auto temp12 = -(gR43C18C19*gR44*gR48);
    const auto temp13 = gR42C16C17*gR45C20*gR48;
    const auto temp14 = temp11 + temp12 + temp13;
    const auto temp15 = -(gR35*temp14*gR66);
    const auto temp16 = temp4 + temp10 + temp15;
    const auto temp17 = 1/temp16;
    const auto temp21 = -(gR43C18C19*gR44);
    const auto temp22 = gR42C16C17*gR45C20;
    const auto temp23 = temp21 + temp22;
    const auto temp42 = gR42C16C17 + gR43C18C19;
    const auto temp33 = gR39 + gR40 + gR41R38 + gR66;
    const auto temp35 = gR39 + gR40 + gR41R38 + gR42C16C17 + gR66;
    const auto temp39 = gR41R38*gR42C16C17;
    const auto temp40 = gR41R38*gR43C18C19;
    const auto temp41 = gR42C16C17*gR43C18C19;
    const auto temp43 = gR39*temp42;
    const auto temp44 = gR40*temp42;
    const auto temp45 = temp42*gR66;
    const auto temp46 = temp39 + temp40 + temp41 + temp43 + temp44 + temp45;
    const auto temp58 = temp5*gR43C18C19*gR48;
    const auto temp59 = -(gR42C16C17*gR47);
    const auto temp60 = gR43C18C19*gR48;
    const auto temp61 = temp59 + temp60;
    const auto temp62 = temp61*gR66;
    const auto temp63 = temp58 + temp62;

    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zR45C20 = state[ch].zR45C20;
        auto zR42C16C17 = state[ch].zR42C16C17;
        auto zR43C18C19 = state[ch].zR43C18C19;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            const auto temp32 = -(gR39*gR42C16C17*vi);
            const auto temp34 = -(temp33*zR42C16C17);
            const auto temp36 = temp35*zR43C18C19;
            const auto temp37 = temp32 + temp34 + temp36;
            const auto temp57 = gR35*gR45C20*gR48*temp37;
            const auto temp64 = gR35*temp63*zR45C20;
            const auto temp65 = gR33*gR45C20*gR48*vi;
            const auto temp66 = gR37*gR47*zR45C20;
            const auto temp67 = temp65 + temp66;
            const auto temp68 = gR40*gR42C16C17*temp67;
            const auto temp69 = temp57 + temp64 + temp68;
            const auto temp70 = temp17*temp69;
            const auto temp52 = gR33*gR40*gR42C16C17*temp3*vi;
            const auto temp53 = gR35*temp3*temp37;
            const auto temp54 = gR35*temp46*zR45C20;
            const auto temp55 = temp52 + temp53 + temp54;
            
            const auto vo = temp17*(gR33*(-(gR41R38*gR43C18C19*gR44*gR48) - gR42C16C17*gR43C18C19*gR44*gR48 + gR41R38*gR42C16C17*gR45C20*gR48 + gR39*temp23*gR48 + gR40*temp23*gR48 + gR42C16C17*gR44*gR47*gR66 + gR42C16C17*gR45C20*gR47*gR66 - gR43C18C19*gR44*gR48*gR66 + gR42C16C17*gR45C20*gR48*gR66)*vi + gR37*gR47*(temp3*temp37 + temp46*zR45C20));
            const auto vR45C20 = -(gR48*temp17*temp55) + temp70;
            const auto vR42C16C17 = temp70 - temp17*(gR35*(gR39*(gR43C18C19*gR44 - gR42C16C17*gR45C20)*gR48*vi - gR43C18C19*gR44*gR48*zR42C16C17 + gR44*gR47*gR66*zR42C16C17 + gR45C20*gR47*gR66*zR42C16C17 + gR42C16C17*gR45C20*gR48*zR43C18C19 - gR44*gR47*gR66*zR43C18C19 - gR45C20*gR47*gR66*zR43C18C19 + gR42C16C17*gR43C18C19*gR48*zR45C20 - gR42C16C17*gR47*gR66*zR45C20 - gR43C18C19*gR47*gR66*zR45C20) + gR40*(gR33*temp23*gR48*vi + gR37*gR47*(-(temp3*(zR42C16C17 - zR43C18C19)) + temp42*zR45C20)));
            const auto vR43C18C19 = gR48*temp17*temp55 - temp17*temp69;
            
            zR45C20 = gR45C20 * (1 - gnR45C20) * vR45C20 + gnR45C20 * zR45C20; // RC series
            zR42C16C17 = gR42C16C17 * (1 - gnR42C16C17) * vR42C16C17 + gnR42C16C17 * zR42C16C17; // RC series
            zR43C18C19 = gzR43C18C19 * vR43C18C19 - zR43C18C19; // RC parallel

            output[ch][n] = vo;
        }
        state[ch].zR45C20 = zR45C20;
        state[ch].zR42C16C17 = zR42C16C17;
        state[ch].zR43C18C19 = zR43C18C19;
    }
}
