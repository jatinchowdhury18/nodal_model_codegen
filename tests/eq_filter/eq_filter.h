#pragma once

struct Params {
    double R33 = 4.75e+03f;
    double R37 = 4.75e+03f;
    double R35 = 4.75e+03f;
    double R40 = 3.5e+03f;
    double R39 = 2.35e+04f;
    double R66 = 1.2e+04f;
    double R41 = 1.8e+03f;
    double R38 = 4.7e+03f;
    double R44 = 1.0e+03f;
    double R47 = 7.5e+02f;
    double R48 = 3.3e+03f;
    double R43 = 2.51e+04f;
    double C18 = 3.3e-08f;
    double C19 = 4.7e-08f;
    double R45 = 1.0e+03f;
    double C20 = 1.0e-04f;
    double R42 = 2.43e+04f;
    double C16 = 3.3e-08f;
    double C17 = 4.7e-08f;
};

struct State {
    double zC18 {};
    double zC19 {};
    double zC20 {};
    double zC16 {};
    double zC17 {};
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
    
    const auto gR41 = 1.0f / params.R41;
    
    const auto gR38 = 1.0f / params.R38;
    
    const auto gR44 = 1.0f / params.R44;
    
    const auto gR47 = 1.0f / params.R47;
    
    const auto gR48 = 1.0f / params.R48;
    
    const auto gR43 = 1.0f / params.R43;
    
    const auto gC18 = 2.0f * sample_rate * params.C18;
    
    const auto gC19 = 2.0f * sample_rate * params.C19;
    
    const auto gR45 = 1.0f / params.R45;
    
    const auto gC20 = 2.0f * sample_rate * params.C20;
    
    const auto gR42 = 1.0f / params.R42;
    
    const auto gC16 = 2.0f * sample_rate * params.C16;
    
    const auto gC17 = 2.0f * sample_rate * params.C17;
    
    const auto _t1 = (gR44 + gR45);
    const auto _t2 = (gR43 + gC18);
    const auto _t3 = (gR66 * gR37);
    const auto _t4 = (gR66 * gR48);
    const auto _t5 = (gC16 + gC17);
    const auto _t8 = (_t2 + gC19);
    const auto _t9 = ((gR37 + gR66) + gR48);
    const auto _t10 = (gC20 * _t1);
    const auto _t11 = (gR35 * _t9);
    const auto _t12 = (gR40 * _t9);
    const auto _t13 = ((((gR47 + gR43) + gC18) + gC19) + gC20);
    const auto _t15 = (_t8 * _t1);
    const auto _t17 = (_t1 * _t10);
    const auto _t18 = (_t13 * _t1);
    const auto _t19 = (_t1 * _t15);
    const auto _t20 = (_t10 + (gR45 * _t8));
    const auto _t21 = ((_t5 * _t9) * _t11);
    const auto _t22 = ((gR66 * _t11) - (gR37 * _t12));
    const auto _t23 = ((gR42 * _t1) * _t18);
    const auto _t24 = ((_t22 * _t5) * _t1);
    const auto _t25 = (_t23 * _t17);
    const auto _t26 = ((_t21 * _t4) * _t18);
    const auto _t29 = ((((((((((gR40 + gR39) + gR66) + gR41) + gR38) + gC16) + gC17) * _t9) - (gR66 * gR66)) * _t11) + (_t3 * _t12));
    const auto _t30 = (((gR45 * (_t8 + gR42)) * _t18) - (_t20 * _t15));
    const auto _t31 = ((gR47 * _t9) * _t29);
    const auto _t32 = (_t1 * _t31);
    const auto _t33 = (_t20 * _t31);
    const auto _t34 = ((gR37 * _t29) + (_t22 * _t3));
    const auto _t35 = ((gR48 * _t29) + (_t22 * _t4));
    const auto _t36 = (_t32 * _t22);
    const auto _t37 = ((_t20 * _t19) + (_t30 * _t1));
    const auto _t38 = (_t35 * _t18);
    const auto _t39 = (_t32 * _t24);
    const auto _t41 = ((_t30 * _t17) - ((((((gR45 + gC20) * _t1) - (gR45 * gR45)) * _t18) - (_t20 * _t10)) * _t19));
    const auto _t43 = (_t32 * _t34);
    const auto _t44 = (((_t33 * _t22) * _t19) + (_t30 * _t36));
    const auto _t45 = (((_t33 * _t34) * _t19) + (_t30 * _t43));
    const auto _t46 = ((_t30 * _t39) - ((((gR45 * (gR42 * _t29)) * _t38) - (_t33 * _t24)) * _t19));
    const auto _t47 = (((((((((((gR42 + gC16) + gC17) * _t29) - (_t21 * _t5)) * _t1) * _t38) + (_t26 * _t24)) * _t19) - (_t23 * _t39)) * _t41) + (_t25 * _t46));
    const auto _t49 = ((_t25 * _t45) - ((((((_t21 * _t3) * _t38) - (_t26 * _t34)) * _t19) + (_t23 * _t43)) * _t41));
    const auto _t50 = (1 / (((((((((_t11 * _t29) * _t35) * _t13) * _t1) * _t1) * _t8) * _t1) * _t41) * _t47));
    const auto _t51 = (_t25 * gR45);
    const auto _t52 = (1 / ((_t1 * _t41) * _t47));
    const auto _t53 = ((_t25 * _t37) - ((_t23 * _t1) * _t41));
    const auto _t54 = (1 / (((((_t18 * _t1) * _t8) * _t1) * _t41) * _t47));
    const auto _t55 = (1 / (_t41 * _t47));
    const auto _t56 = ((_t25 * _t44) - (((((_t21 * _t38) - (_t26 * _t22)) * _t19) + (_t23 * _t36)) * _t41));
    const auto _t57 = (1 / ((((((((_t29 * _t35) * _t13) * _t1) * _t1) * _t8) * _t1) * _t41) * _t47));
    const auto _t58 = (1 / _t47);
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zC18 = state[ch].zC18;
        auto zC19 = state[ch].zC19;
        auto zC20 = state[ch].zC20;
        auto zC16 = state[ch].zC16;
        auto zC17 = state[ch].zC17;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            const auto _t0 = (zC18 + zC19);
            const auto _t14 = ((_t0 + zC20) * _t1);
            const auto _t16 = ((gR33 * vi) * _t9);
            const auto _t27 = ((_t16 * _t12) - (((((gR39 * vi) + zC16) + zC17) * _t9) * _t11));
            const auto _t28 = (((_t0 * _t1) * _t18) - (_t14 * _t15));
            const auto _t40 = ((_t28 * _t17) - ((((zC20 * _t1) * _t18) - (_t14 * _t10)) * _t19));
            const auto _t42 = (((_t27 * _t4) * _t18) + (_t14 * _t31));
            const auto _t48 = ((((_t28 * _t39) - (((((((zC16 + zC17) * _t29) + (_t27 * _t5)) * _t1) * _t38) - (_t42 * _t24)) * _t19)) * _t41) - (_t40 * _t46));
            const auto vo = (-(((((((_t28 * _t43) - (((((_t27 * _t3) - (_t16 * _t29)) * _t38) - (_t42 * _t34)) * _t19)) * _t41) - (_t40 * _t45)) * _t47) - (_t48 * _t49)) * _t50));
            const auto vn9 = ((((_t40 * gR45) * _t47) + (_t48 * _t51)) * _t52);
            const auto v36 = (-(((((((_t14 * _t19) + (_t28 * _t1)) * _t41) - (_t40 * _t37)) * _t47) - (_t48 * _t53)) * _t54));
            const auto vr45 = (((_t40 * _t47) + (_t48 * _t25)) * _t55);
            const auto tC20 = (gC20 * (vr45 - v36));
            const auto v33 = (((((((_t28 * _t36) - (((_t27 * _t38) - (_t42 * _t22)) * _t19)) * _t41) - (_t40 * _t44)) * _t47) - (_t48 * _t56)) * _t57);
            const auto v37 = (_t48 * _t58);
            const auto _t6 = (vn9 - v36);
            const auto _t7 = (v33 - v37);
            const auto tC18 = (gC18 * _t6);
            const auto tC19 = (gC19 * _t6);
            const auto tC16 = (gC16 * _t7);
            const auto tC17 = (gC17 * _t7);
            
            zC18 = 2 * tC18 - zC18;
            zC19 = 2 * tC19 - zC19;
            zC20 = 2 * tC20 - zC20;
            zC16 = 2 * tC16 - zC16;
            zC17 = 2 * tC17 - zC17;

            output[ch][n] = vo;
        }
        state[ch].zC18 = zC18;
        state[ch].zC19 = zC19;
        state[ch].zC20 = zC20;
        state[ch].zC16 = zC16;
        state[ch].zC17 = zC17;
    }
}
