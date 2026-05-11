// Auto-generated with netlist_codegen version d2c1c08.
// Command: netlist_codegen eq_filter.net eq_filter.h -namespace nodal::eq_filter

#pragma once

namespace nodal::eq_filter
{

struct Params {
    float R33 = 4.75e+03f;
    float R37 = 4.75e+03f;
    float R35 = 4.75e+03f;
    float R40 = 3.5e+03f;
    float R39 = 2.35e+04f;
    float R66 = 1.2e+04f;
    float R45 = 1.0e+03f;
    float C20 = 1.0e-04f;
    float R44 = 1.0e+03f;
    float R47 = 7.5e+02f;
    float R48 = 3.3e+03f;
    float R42 = 2.43e+04f;
    float C17 = 4.7e-08f;
    float C16 = 3.3e-08f;
    float R41 = 1.8e+03f;
    float R38 = 4.7e+03f;
    float R43 = 2.51e+04f;
    float C18 = 3.3e-08f;
    float C19 = 4.7e-08f;
};

struct State {
    float zR45C20 {};
    float zR42C17C16 {};
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
    
    const auto R45 = params.R45;
    const auto C20 = params.C20;
    const auto gR45C20 = (2.0f * sample_rate * C20) / (1.0f + 2.0f * sample_rate * R45 * C20);
    const auto gnR45C20 = gR45C20 * (2.0f * sample_rate * R45 * C20 - 1.0f) / (2.0f * sample_rate * C20);
    
    const auto gR44 = 1.0f / params.R44;
    
    const auto gR47 = 1.0f / params.R47;
    
    const auto gR48 = 1.0f / params.R48;
    
    const auto R42 = params.R42;
    const auto C17C16 = sum(params.C17, params.C16);
    const auto gR42C17C16 = (2.0f * sample_rate * C17C16) / (1.0f + 2.0f * sample_rate * R42 * C17C16);
    const auto gnR42C17C16 = gR42C17C16 * (2.0f * sample_rate * R42 * C17C16 - 1.0f) / (2.0f * sample_rate * C17C16);
    
    const auto R41R38 = recip_sum(params.R41, params.R38);
    const auto gR41R38 = 1.0f / R41R38;
    
    const auto R43 = params.R43;
    const auto C18C19 = sum(params.C18, params.C19);
    const auto gR43C18C19 = 2.0f * sample_rate * C18C19 + (1.0f / R43);
    const auto gzR43C18C19 = 4.0f * sample_rate * C18C19;
    
    const auto _t0 = ((((gR40 + gR39) + gR66) + gR42C17C16) + gR41R38);
    const auto _t3 = (gR45C20 + gR44);
    const auto _t8 = (gR42C17C16 + gR43C18C19);
    const auto _t11 = (gR47 * gR42C17C16);
    const auto _t16 = (gR48 * gR43C18C19);
    const auto _t17 = (gR48 * _t8);
    const auto _t19 = (gR48 * gR42C17C16);
    const auto _t25 = (_t3 * _t19);
    const auto _t7 = (gR47 * _t8);
    const auto _t10 = (_t3 * _t11);
    const auto _t15 = ((_t3 * _t16) - (gR45C20 * _t17));
    const auto _t18 = (gR45C20 * _t19);
    const auto _t14 = (((_t0 * _t15) - (gR66 * _t10)) + (gR42C17C16 * _t18));
    const auto _t20 = (1 / ((gR37 * (gR40 * _t10)) + (gR35 * _t14)));
    const auto _t27 = (gR40 * _t18);
    const auto _t28 = (gR40 * _t25);
    const auto _t29 = (gR40 * _t15);
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zR45C20 = state[ch].zR45C20;
        auto zR42C17C16 = state[ch].zR42C17C16;
        auto zR43C18C19 = state[ch].zR43C18C19;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            const auto _t5 = (zR42C17C16 + zR43C18C19);
            const auto _t9 = (zR42C17C16 - (gR39 * vi));
            const auto _t12 = (zR45C20 * _t11);
            const auto _t13 = (gR33 * vi);
            const auto _t22 = (zR45C20 * _t16);
            const auto _t24 = (gR48 * _t5);
            const auto _t4 = (gR47 * _t5);
            const auto _t23 = (gR45C20 * _t24);
            const auto _t2 = (_t3 * _t4);
            const auto _t6 = (zR45C20 * _t7);
            const auto _t21 = (_t22 - _t23);
            const auto _t1 = (_t2 - _t6);
            const auto vo = (-(((gR37 * (((_t0 * _t1) - (_t9 * _t10)) + (gR42C17C16 * _t12))) + (_t13 * _t14)) * _t20));
            const auto vn9 = ((((gR37 * (gR40 * _t12)) + (gR35 * (((_t0 * _t21) - (gR66 * _t12)) + (_t9 * _t18)))) + (_t13 * _t27)) * _t20);
            const auto v36 = (((gR35 * (((_t0 * ((zR45C20 * _t17) - (_t3 * _t24))) - (gR42C17C16 * (zR45C20 * _t19))) + (_t9 * _t25))) + (_t13 * _t28)) * _t20);
            const auto v33 = (-((((gR37 * (gR40 * _t1)) - (gR35 * (((gR66 * _t1) - (_t9 * _t15)) + (gR42C17C16 * _t21)))) + (_t13 * _t29)) * _t20));
            const auto vR42C17C16 = (vn9 - v33);
            const auto _t26 = (vn9 - v36);
            const auto vR45C20 = _t26;
            const auto vR43C18C19 = _t26;
            
            zR45C20 = gR45C20 * (1 - gnR45C20) * vR45C20 + gnR45C20 * zR45C20; // RC series
            zR42C17C16 = gR42C17C16 * (1 - gnR42C17C16) * vR42C17C16 + gnR42C17C16 * zR42C17C16; // RC series
            zR43C18C19 = gzR43C18C19 * vR43C18C19 - zR43C18C19; // RC parallel

            output[ch][n] = vo;
        }
        state[ch].zR45C20 = zR45C20;
        state[ch].zR42C17C16 = zR42C17C16;
        state[ch].zR43C18C19 = zR43C18C19;
    }
}

} // namespace nodal::eq_filter
