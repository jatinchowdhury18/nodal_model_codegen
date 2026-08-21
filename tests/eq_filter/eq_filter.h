// Auto-generated with netlist_codegen version d1e5ccb.
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
    
    const auto _t0 = (gR37 * 4096.0f);
    const auto _t1 = (((((gR40 + gR39) + gR66) + gR42C17C16) + gR41R38) * 4096.0f);
    const auto _t4 = ((gR45C20 + gR44) * 1024.0f);
    const auto _t5 = (gR47 * 1024.0f);
    const auto _t8 = ((gR42C17C16 + gR43C18C19) * 16384.0f);
    const auto _t12 = (gR42C17C16 * 16384.0f);
    const auto _t13 = (gR42C17C16 * 4096.0f);
    const auto _t19 = (gR48 * 1024.0f);
    const auto _t20 = (gR45C20 * 1024.0f);
    const auto _t21 = (_t19 * _t8);
    const auto _t22 = (gR66 * 4096.0f);
    const auto _t24 = (_t19 * _t12);
    const auto _t26 = (gR40 * 4096.0f);
    const auto _t27 = (gR35 * 4096.0f);
    const auto _t31 = (_t4 * _t24);
    const auto _t35 = (_t5 * _t8);
    const auto _t11 = (_t5 * _t12);
    const auto _t18 = (_t19 * (gR43C18C19 * 16384.0f));
    const auto _t23 = (_t20 * _t24);
    const auto _t10 = (_t4 * _t11);
    const auto _t17 = ((_t4 * _t18) - (_t20 * _t21));
    const auto _t16 = (((_t1 * _t17) - (_t22 * _t10)) + (_t13 * _t23));
    const auto _t25 = (1.0f / ((_t0 * (_t26 * _t10)) + (_t27 * _t16)));
    const auto _t37 = (_t26 * _t23);
    const auto _t38 = (_t26 * _t31);
    const auto _t39 = (_t26 * _t17);
    float c0_vR42C17C16;
    float c_vR42C17C16[4];
    float c0_v33;
    float c_v33[4];
    float c0_vn9;
    float c_vn9[4];
    float c0_vo;
    float c_vo[4];
    float c0_v36;
    float c_v36[4];
    
    for (int _k = 0; _k <= 4; ++_k)
    {
        const auto vi = (_k == 1) ? 1.0f : 0.0f;
        const auto zR45C20 = (_k == 2) ? 1.0f : 0.0f;
        const auto zR42C17C16 = (_k == 3) ? 1.0f : 0.0f;
        const auto zR43C18C19 = (_k == 4) ? 1.0f : 0.0f;
        const auto _t6 = ((zR42C17C16 + zR43C18C19) * 16384.0f);
        const auto _t7 = (zR45C20 * 1024.0f);
        const auto _t9 = ((zR42C17C16 - (gR39 * vi)) * 4096.0f);
        const auto _t15 = ((gR33 * vi) * 4096.0f);
        const auto _t30 = (_t19 * _t6);
        const auto _t33 = (_t5 * _t6);
        const auto _t36 = (_t20 * _t30);
        const auto _t3 = (_t4 * _t33);
        const auto _t14 = (_t7 * _t11);
        const auto _t29 = (_t7 * _t18);
        const auto _t34 = (_t7 * _t35);
        const auto _t2 = (_t3 - _t34);
        const auto _t28 = (_t29 - _t36);
        const auto vo = (-(((_t0 * (((_t1 * _t2) - (_t9 * _t10)) + (_t13 * _t14))) + (_t15 * _t16)) * _t25));
        const auto vn9 = ((((_t0 * (_t26 * _t14)) + (_t27 * (((_t1 * _t28) - (_t22 * _t14)) + (_t9 * _t23)))) + (_t15 * _t37)) * _t25);
        const auto v36 = (-(((_t27 * (((_t13 * (_t7 * _t24)) - (_t1 * ((_t7 * _t21) - (_t4 * _t30)))) - (_t9 * _t31))) - (_t15 * _t38)) * _t25));
        const auto v33 = (-((((_t0 * (_t26 * _t2)) - (_t27 * (((_t22 * _t2) - (_t9 * _t17)) + (_t13 * _t28)))) + (_t15 * _t39)) * _t25));
        const auto vR42C17C16 = (vn9 - v33);
        if (_k == 0) {
            c0_vR42C17C16 = vR42C17C16;
            c0_v33 = v33;
            c0_vn9 = vn9;
            c0_vo = vo;
            c0_v36 = v36;
        } else {
            c_vR42C17C16[_k - 1] = vR42C17C16 - c0_vR42C17C16;
            c_v33[_k - 1] = v33 - c0_v33;
            c_vn9[_k - 1] = vn9 - c0_vn9;
            c_vo[_k - 1] = vo - c0_vo;
            c_v36[_k - 1] = v36 - c0_v36;
        }
    }
    
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zR45C20 = state[ch].zR45C20;
        auto zR42C17C16 = state[ch].zR42C17C16;
        auto zR43C18C19 = state[ch].zR43C18C19;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            const auto vR42C17C16 = c0_vR42C17C16 + c_vR42C17C16[0] * vi + c_vR42C17C16[1] * zR45C20 + c_vR42C17C16[2] * zR42C17C16 + c_vR42C17C16[3] * zR43C18C19;
            const auto v33 = c0_v33 + c_v33[0] * vi + c_v33[1] * zR45C20 + c_v33[2] * zR42C17C16 + c_v33[3] * zR43C18C19;
            const auto vn9 = c0_vn9 + c_vn9[0] * vi + c_vn9[1] * zR45C20 + c_vn9[2] * zR42C17C16 + c_vn9[3] * zR43C18C19;
            const auto vo = c0_vo + c_vo[0] * vi + c_vo[1] * zR45C20 + c_vo[2] * zR42C17C16 + c_vo[3] * zR43C18C19;
            const auto v36 = c0_v36 + c_v36[0] * vi + c_v36[1] * zR45C20 + c_v36[2] * zR42C17C16 + c_v36[3] * zR43C18C19;
            const auto _t6 = ((zR42C17C16 + zR43C18C19) * 16384.0f);
            const auto _t7 = (zR45C20 * 1024.0f);
            const auto _t9 = ((zR42C17C16 - (gR39 * vi)) * 4096.0f);
            const auto _t15 = ((gR33 * vi) * 4096.0f);
            const auto _t30 = (_t19 * _t6);
            const auto _t32 = (vn9 - v36);
            const auto _t36 = (_t20 * _t30);
            const auto _t14 = (_t7 * _t11);
            const auto _t29 = (_t7 * _t18);
            const auto _t28 = (_t29 - _t36);
            const auto vR45C20 = _t32;
            const auto vR43C18C19 = _t32;
            
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

static float reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc = 0.0f)
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
    
    const auto vi = vi_dc;

    const auto zR45C20 = 0.0f;
    const auto zR42C17C16 = ((gR42C17C16 * (((((gR33 * vi) * gR40) / R43) - (((gR35 * gR39) * vi) / R43)) * R43)) / (gR35 * ((((gR40 + gR39) + gR66) + gR41R38) + (1.0f / 1000000000.0f))));
    const auto zR43C18C19 = 0.0f;

    const auto vo_dc_out = (-((gR33 * vi) / gR35));

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].zR45C20 = zR45C20;
        state[ch].zR42C17C16 = zR42C17C16;
        state[ch].zR43C18C19 = zR43C18C19;
    }
    return vo_dc_out;
}


} // namespace nodal::eq_filter
