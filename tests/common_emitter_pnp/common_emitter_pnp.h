// Auto-generated with netlist_codegen version 720cc46.
// Command: netlist_codegen common_emitter_pnp.net common_emitter_pnp.h

#pragma once

#include <cmath>
#include <cstdint>

static int32_t math_bits_from_float(float x) { union { float f; int32_t i; } u; u.f = x; return u.i; }
static float math_float_from_bits(int32_t i) { union { int32_t i; float f; } u; u.i = i; return u.f; }

static float math_exp_approx(float x) {
    x *= 1.4426950408889634f;
    if (x < -126.0f) x = -126.0f;
    const int32_t xi = (int32_t) x;
    const int32_t l = x < (float) 0 ? xi - 1 : xi;
    const float f = x - (float) l;
    const float f_sq = f * f;
    const int32_t vi = (l + 127) << 23;
    return math_float_from_bits(vi) * ((((1.0f + 0.69314718056000002f * f) + f_sq * ((0.24022825068600001f + 0.0554875633068f * f))) + (f_sq * f_sq) * (((0.00967475272129f + 0.00124453797252f * f) + f_sq * (0.000217714753229f)))));
}

static float math_log_approx(float x) {
    const int32_t vi = math_bits_from_float(x);
    const int32_t ex = vi & 0x7f800000;
    const int32_t e = (ex >> 23) - 127;
    const int32_t vfi = (vi - ex) | 0x3f800000;
    const float vf = math_float_from_bits(vfi);
    const float vf_sq = vf * vf;
    return 0.6931471805599453f * ((float) e + ((((-3.06081857306000015f + 6.19242937535999972f * vf) + vf_sq * ((-5.46521465639999971f + 3.38542517474999994f * vf))) + (vf_sq * vf_sq) * (((-1.3100709077499999f + 0.28479443750200001f * vf) + vf_sq * (-0.0265448504094f))))));
}

static float math_pow_approx(float x, float y) {
    return math_exp_approx(y * math_log_approx(x));
}


[[maybe_unused]] static auto limit_junction_voltage = [](auto v_new, auto v_old, auto vt, auto vcrit)
{
    if (v_new > vcrit && std::abs(v_new - v_old) > 2 * vt)
    {
        if (v_old > 0)
        {
            const auto arg = 1 + (v_new - v_old) / vt;
            v_new = arg > 0 ? v_old + vt * math_log_approx(arg) : vcrit;
        }
        else
        {
            v_new = vt * math_log_approx(v_new / vt);
        }
    }
    else if (v_new < -vcrit && std::abs(v_new - v_old) > 2 * vt)
    {
        if (v_old < 0)
        {
            const auto arg = 1 + (v_old - v_new) / vt;
            v_new = arg > 0 ? v_old - vt * math_log_approx(arg) : -vcrit;
        }
        else
        {
            v_new = -vt * math_log_approx(-v_new / vt);
        }
    }
    return v_new;
};

static constexpr auto newton_tol_sq = 0.00001;
static constexpr int newton_max_iter = 20;


struct Params {
    float VEE = -9.0e+00f;
    float C1 = 1.0e-07f;
    float R1 = 4.7e+05f;
    float R2 = 1.0e+05f;
    float RE = 2.7e+03f;
    float RC = 1.2e+04f;
    float Q2N5087_Is = 5.0e-14f;
    float Q2N5087_vt = 2.585e-02f;
    float Q2N5087_BetaF = 6.0e+02f;
    float Q2N5087_BetaR = 5.0e+01f;
    float C2 = 1.0e-07f;
    float RL = 1.0e+05f;
};

struct State {
    float zC1 {};
    float zC2 {};
    float vCBQ1 {};
    float vEBQ1 {};
};

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto Q2N5087_Is = params.Q2N5087_Is;
    const auto Q2N5087_vt = params.Q2N5087_vt;
    const auto Q2N5087_BetaF = params.Q2N5087_BetaF;
    const auto Q2N5087_BetaR = params.Q2N5087_BetaR;
    
    const auto VEE = params.VEE;
    
    const auto vn = VEE;
    
    const auto gC1 = 2.0f * sample_rate * params.C1;
    
    const auto gR1 = 1.0f / params.R1;
    
    const auto gR2 = 1.0f / params.R2;
    
    const auto gRE = 1.0f / params.RE;
    
    const auto gRC = 1.0f / params.RC;
    
    const auto gC2 = 2.0f * sample_rate * params.C2;
    
    const auto gRL = 1.0f / params.RL;
    
    const auto vcrit_Q2N5087_vt = Q2N5087_vt * std::log(Q2N5087_vt / (std::sqrt(2.0) * Q2N5087_Is));
    
    const auto _t0 = (gRE * 2048.0f);
    const auto _t6 = (gC2 * 128.0f);
    const auto _t7 = ((gRC + gC2) * 128.0f);
    const auto _t9 = (gRC * 128.0f);
    const auto _t11 = (262144.0f * gRE);
    const auto _t13 = ((gC2 + gRL) * 128.0f);
    const auto _t12 = ((_t7 * _t13) - (_t6 * _t6));
    const auto _t10 = (1.0f / (_t11 * _t12));
    const auto _t14 = (VEE * (_t0 * (_t9 * _t6)));
    const auto _t15 = (gR1 * VEE);
    const auto _t16 = (1.0f / ((gC1 + gR1) + gR2));
    const auto _t17 = (VEE * (_t0 * (_t9 * _t13)));
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zC1 = state[ch].zC1;
        auto zC2 = state[ch].zC2;
        auto vCBQ1 = state[ch].vCBQ1;
        auto vEBQ1 = state[ch].vEBQ1;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            // --- Newton-Raphson solve: Q1
            const auto _Q1_t1 = (gRE * 2048.0f);
            const auto _Q1_t12 = (gC2 + gRL);
            const auto _Q1_t13 = (gC2 * 128.0f);
            const auto _Q1_t19 = (gC1 * vi);
            const auto _Q1_t24 = (gR1 * VEE);
            const auto _Q1_t26 = (gC1 + gR1);
            const auto _Q1_t33 = (1.0f / (Q2N5087_vt * Q2N5087_BetaF));
            const auto _Q1_t35 = (16384.0f * Q2N5087_Is);
            const auto _Q1_t46 = (1.0f / (Q2N5087_vt * Q2N5087_BetaR));
            const auto _Q1_t11 = (_Q1_t12 * 128.0f);
            const auto _Q1_t14 = ((((gRC + gC2) * 128.0f) * _Q1_t11) - (_Q1_t13 * _Q1_t13));
            const auto _Q1_t18 = (_Q1_t19 - zC1);
            const auto _Q1_t25 = (1.0f / (_Q1_t26 + gR2));
            const auto _Q1_t48 = (_Q1_t13 * (zC2 * 128.0f));
            const auto _Q1_t49 = (VEE * (_Q1_t1 * ((gRC * 128.0f) * _Q1_t11)));
            const auto _Q1_t50 = (1.0f / ((262144.0f * gRE) * _Q1_t14));
            const auto _Q1_t51 = (1.0f / _Q1_t14);
            for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
            {
                const auto _Q1_t6 = (vEBQ1 / Q2N5087_vt);
                const auto _Q1_t8 = (vCBQ1 / Q2N5087_vt);
                const auto _Q1_t5 = math_exp_approx(_Q1_t6);
                const auto _Q1_t7 = math_exp_approx(_Q1_t8);
                const auto _Q1_t10 = (_Q1_t7 - 1.0f);
                const auto _Q1_t23 = (_Q1_t5 - 1.0f);
                const auto _Q1_t30 = (Q2N5087_Is * _Q1_t5);
                const auto _Q1_t32 = (_Q1_t30 * _Q1_t33);
                const auto _Q1_t41 = (_Q1_t7 / Q2N5087_vt);
                const auto _Q1_t42 = (_Q1_t41 / Q2N5087_BetaR);
                const auto _Q1_t45 = (Q2N5087_Is * _Q1_t7);
                const auto _Q1_t4 = (_Q1_t5 - _Q1_t7);
                const auto _Q1_t9 = (_Q1_t10 / Q2N5087_BetaR);
                const auto _Q1_t22 = (_Q1_t23 / Q2N5087_BetaF);
                const auto _Q1_t29 = (_Q1_t30 / Q2N5087_vt);
                const auto _Q1_t31 = (_Q1_t32 * _Q1_t25);
                const auto _Q1_t34 = (((((_Q1_t35 * _Q1_t5) * _Q1_t12) / Q2N5087_vt) * _Q1_t51) - _Q1_t31);
                const auto _Q1_t40 = (_Q1_t41 + _Q1_t42);
                const auto _Q1_t44 = (_Q1_t45 * _Q1_t46);
                const auto _Q1_t3 = (_Q1_t4 - _Q1_t9);
                const auto _Q1_t21 = (_Q1_t22 + _Q1_t9);
                const auto _Q1_t28 = (((((_Q1_t29 / Q2N5087_BetaF) + _Q1_t29) / gRE) + _Q1_t31) + 1.0f);
                const auto _Q1_t43 = (_Q1_t44 * _Q1_t25);
                const auto _Q1_t47 = (((((_Q1_t45 / Q2N5087_vt) / Q2N5087_BetaR) - (Q2N5087_Is * _Q1_t40)) / gRE) + _Q1_t43);
                const auto _Q1_t2 = (Q2N5087_Is * _Q1_t3);
                const auto _Q1_t20 = (Q2N5087_Is * _Q1_t21);
                const auto _Q1_t39 = (((((_Q1_t35 * _Q1_t40) * _Q1_t12) * _Q1_t51) + _Q1_t43) + 1.0f);
                const auto _Q1_t17 = (_Q1_t18 + _Q1_t20);
                const auto _Q1_t38 = ((_Q1_t39 * _Q1_t28) + (_Q1_t34 * _Q1_t47));
                const auto _Q1_t16 = (_Q1_t17 + _Q1_t24);
                const auto _Q1_t15 = (_Q1_t16 * _Q1_t25);
                const auto _Q1_t37 = (((_Q1_t20 + _Q1_t2) / gRE) + _Q1_t15);
                const auto _Q1_t0 = (((128.0f * ((_Q1_t1 * ((((zC2 + _Q1_t2) * 128.0f) * _Q1_t11) - _Q1_t48)) + _Q1_t49)) * _Q1_t50) - _Q1_t15);
                const auto _Q1_t27 = (vCBQ1 - _Q1_t0);
                const auto _Q1_t36 = (_Q1_t37 + vEBQ1);
                const auto res_vCBQ1 = (_Q1_t0 - vCBQ1);
                const auto delta_vCBQ1 = (-(((_Q1_t27 * _Q1_t28) + (_Q1_t34 * _Q1_t36)) / _Q1_t38));
                const auto res_vEBQ1 = (-_Q1_t36);
                const auto delta_vEBQ1 = (((_Q1_t27 * _Q1_t47) - (_Q1_t39 * _Q1_t36)) / _Q1_t38);
            
                auto residual_norm_sq = 0.0;
                residual_norm_sq += res_vCBQ1 * res_vCBQ1;
                residual_norm_sq += res_vEBQ1 * res_vEBQ1;
                auto step_norm_sq = 0.0;
                step_norm_sq += delta_vCBQ1 * delta_vCBQ1;
                step_norm_sq += delta_vEBQ1 * delta_vEBQ1;
            
                vCBQ1 = limit_junction_voltage(vCBQ1 + (delta_vCBQ1), vCBQ1, Q2N5087_vt, vcrit_Q2N5087_vt);
                vEBQ1 = limit_junction_voltage(vEBQ1 + (delta_vEBQ1), vEBQ1, Q2N5087_vt, vcrit_Q2N5087_vt);
            
                if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
                    break;
                
            }

            const auto _t2 = math_exp_approx((vEBQ1 / Q2N5087_vt));
            const auto _t3 = math_exp_approx((vCBQ1 / Q2N5087_vt));
            const auto _t5 = (_t3 - 1.0f);
            const auto _t8 = (zC2 * 128.0f);
            const auto _t4 = (_t5 / Q2N5087_BetaR);
            const auto _t1 = ((zC2 + (Q2N5087_Is * ((_t2 - _t3) - _t4))) * 128.0f);
            const auto vo = ((128.0f * ((_t0 * ((_t1 * _t6) - (_t7 * _t8))) + _t14)) * _t10);
            const auto vb = (((((gC1 * vi) - zC1) + (Q2N5087_Is * (((_t2 - 1.0f) / Q2N5087_BetaF) + _t4))) + _t15) * _t16);
            const auto tC1 = (gC1 * (vi - vb));
            const auto vc = ((128.0f * ((_t0 * ((_t1 * _t13) - (_t6 * _t8))) + _t17)) * _t10);
            const auto tC2 = (gC2 * (vc - vo));
            
            zC1 = 2 * tC1 - zC1;
            zC2 = 2 * tC2 - zC2;

            output[ch][n] = vo;
        }
        state[ch].zC1 = zC1;
        state[ch].zC2 = zC2;
        state[ch].vCBQ1 = vCBQ1;
        state[ch].vEBQ1 = vEBQ1;
    }
}

static float reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc = 0.0f)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto Q2N5087_Is = params.Q2N5087_Is;
    const auto Q2N5087_vt = params.Q2N5087_vt;
    const auto Q2N5087_BetaF = params.Q2N5087_BetaF;
    const auto Q2N5087_BetaR = params.Q2N5087_BetaR;
    
    const auto VEE = params.VEE;
    
    const auto vn = VEE;
    
    const auto gC1 = 2.0f * sample_rate * params.C1;
    
    const auto gR1 = 1.0f / params.R1;
    
    const auto gR2 = 1.0f / params.R2;
    
    const auto gRE = 1.0f / params.RE;
    
    const auto gRC = 1.0f / params.RC;
    
    const auto gC2 = 2.0f * sample_rate * params.C2;
    
    const auto gRL = 1.0f / params.RL;
    
    const auto vcrit_Q2N5087_vt = Q2N5087_vt * std::log(Q2N5087_vt / (std::sqrt(2.0) * Q2N5087_Is));
    
    const auto vi = vi_dc;

    float vCBQ1 = 0;
    float vEBQ1 = 0;

    // --- Newton-Raphson solve: Q1
    const auto _Q1_t11 = (1.0f / 1000000000.0f);
    const auto _Q1_t18 = (gR1 * VEE);
    const auto _Q1_t20 = (gR1 + gR2);
    const auto _Q1_t25 = (1.0f / (gRE + _Q1_t11));
    const auto _Q1_t28 = (1.0f / (Q2N5087_vt * Q2N5087_BetaF));
    const auto _Q1_t41 = (1.0f / (Q2N5087_vt * Q2N5087_BetaR));
    const auto _Q1_t10 = (1.0f / (gRC + _Q1_t11));
    const auto _Q1_t19 = (1.0f / (_Q1_t20 + _Q1_t11));
    const auto _Q1_t43 = (VEE * gRC);
    for (int newton_iter = 0; newton_iter < 10000; ++newton_iter)
    {
        const auto _Q1_t5 = (vEBQ1 / Q2N5087_vt);
        const auto _Q1_t7 = (vCBQ1 / Q2N5087_vt);
        const auto _Q1_t4 = math_exp_approx(_Q1_t5);
        const auto _Q1_t6 = math_exp_approx(_Q1_t7);
        const auto _Q1_t9 = (_Q1_t6 - 1.0f);
        const auto _Q1_t17 = (_Q1_t4 - 1.0f);
        const auto _Q1_t24 = (Q2N5087_Is * _Q1_t4);
        const auto _Q1_t27 = (_Q1_t24 * _Q1_t28);
        const auto _Q1_t36 = (_Q1_t6 / Q2N5087_vt);
        const auto _Q1_t37 = (_Q1_t36 / Q2N5087_BetaR);
        const auto _Q1_t40 = (Q2N5087_Is * _Q1_t6);
        const auto _Q1_t3 = (_Q1_t4 - _Q1_t6);
        const auto _Q1_t8 = (_Q1_t9 / Q2N5087_BetaR);
        const auto _Q1_t16 = (_Q1_t17 / Q2N5087_BetaF);
        const auto _Q1_t23 = (_Q1_t24 / Q2N5087_vt);
        const auto _Q1_t26 = (_Q1_t27 * _Q1_t19);
        const auto _Q1_t29 = ((_Q1_t23 * _Q1_t10) - _Q1_t26);
        const auto _Q1_t35 = (_Q1_t36 + _Q1_t37);
        const auto _Q1_t39 = (_Q1_t40 * _Q1_t41);
        const auto _Q1_t2 = (_Q1_t3 - _Q1_t8);
        const auto _Q1_t15 = (_Q1_t16 + _Q1_t8);
        const auto _Q1_t22 = (((((_Q1_t23 / Q2N5087_BetaF) + _Q1_t23) * _Q1_t25) + _Q1_t26) + 1.0f);
        const auto _Q1_t34 = (Q2N5087_Is * _Q1_t35);
        const auto _Q1_t38 = (_Q1_t39 * _Q1_t19);
        const auto _Q1_t42 = (((((_Q1_t40 / Q2N5087_vt) / Q2N5087_BetaR) - _Q1_t34) * _Q1_t25) + _Q1_t38);
        const auto _Q1_t1 = (Q2N5087_Is * _Q1_t2);
        const auto _Q1_t14 = (Q2N5087_Is * _Q1_t15);
        const auto _Q1_t33 = (((_Q1_t34 * _Q1_t10) + _Q1_t38) + 1.0f);
        const auto _Q1_t13 = (_Q1_t14 + _Q1_t18);
        const auto _Q1_t32 = ((_Q1_t33 * _Q1_t22) + (_Q1_t29 * _Q1_t42));
        const auto _Q1_t12 = (_Q1_t13 * _Q1_t19);
        const auto _Q1_t31 = (((_Q1_t14 + _Q1_t1) * _Q1_t25) + _Q1_t12);
        const auto _Q1_t0 = (((_Q1_t1 + _Q1_t43) * _Q1_t10) - _Q1_t12);
        const auto _Q1_t21 = (vCBQ1 - _Q1_t0);
        const auto _Q1_t30 = (_Q1_t31 + vEBQ1);
        const auto res_vCBQ1 = (_Q1_t0 - vCBQ1);
        const auto delta_vCBQ1 = (-(((_Q1_t21 * _Q1_t22) + (_Q1_t29 * _Q1_t30)) / _Q1_t32));
        const auto res_vEBQ1 = (-_Q1_t30);
        const auto delta_vEBQ1 = (((_Q1_t21 * _Q1_t42) - (_Q1_t33 * _Q1_t30)) / _Q1_t32);
    
        auto residual_norm_sq = 0.0;
        residual_norm_sq += res_vCBQ1 * res_vCBQ1;
        residual_norm_sq += res_vEBQ1 * res_vEBQ1;
        auto step_norm_sq = 0.0;
        step_norm_sq += delta_vCBQ1 * delta_vCBQ1;
        step_norm_sq += delta_vEBQ1 * delta_vEBQ1;
    
        vCBQ1 = limit_junction_voltage(vCBQ1 + (delta_vCBQ1), vCBQ1, Q2N5087_vt, vcrit_Q2N5087_vt);
        vEBQ1 = limit_junction_voltage(vEBQ1 + (delta_vEBQ1), vEBQ1, Q2N5087_vt, vcrit_Q2N5087_vt);
    
        if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
            break;
        
    }
    const auto zC1 = (gC1 * (vi - (((Q2N5087_Is * (((math_exp_approx((vEBQ1 / Q2N5087_vt)) - 1.0f) / Q2N5087_BetaF) + ((math_exp_approx((vCBQ1 / Q2N5087_vt)) - 1.0f) / Q2N5087_BetaR))) + (gR1 * VEE)) / ((gR1 + gR2) + (1.0f / 1000000000.0f)))));
    const auto zC2 = ((gC2 * ((Q2N5087_Is * ((math_exp_approx((vEBQ1 / Q2N5087_vt)) - math_exp_approx((vCBQ1 / Q2N5087_vt))) - ((math_exp_approx((vCBQ1 / Q2N5087_vt)) - 1.0f) / Q2N5087_BetaR))) + (VEE * gRC))) / (gRC + (1.0f / 1000000000.0f)));

    const auto vo_dc_out = 0.0f;

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].vCBQ1 = vCBQ1;
        state[ch].vEBQ1 = vEBQ1;
        state[ch].zC1 = zC1;
        state[ch].zC2 = zC2;
    }
    return vo_dc_out;
}

