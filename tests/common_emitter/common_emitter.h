// Auto-generated with netlist_codegen version 50a08af.
// Command: netlist_codegen common_emitter.net common_emitter.h

#pragma once

#include <cmath>

struct Params {
    float VCC = 9.0e+00f;
    float C1 = 1.0e-07f;
    float R1 = 4.7e+05f;
    float R2 = 1.0e+05f;
    float RE = 2.7e+03f;
    float RC = 1.2e+04f;
    float Q2N5089_Is = 5.0e-14f;
    float Q2N5089_vt = 2.585e-02f;
    float Q2N5089_BetaF = 6.0e+02f;
    float Q2N5089_BetaR = 5.0e+01f;
    float C2 = 1.0e-07f;
    float RL = 1.0e+05f;
};

struct State {
    float zC1 {};
    float zC2 {};
    float vBCQ1 {};
    float vBEQ1 {};
};

[[maybe_unused]] static auto limit_junction_voltage = [](auto v_new, auto v_old, auto vt, auto vcrit)
{
    if (v_new > vcrit && std::abs(v_new - v_old) > 2 * vt)
    {
        if (v_old > 0)
        {
            const auto arg = 1 + (v_new - v_old) / vt;
            v_new = arg > 0 ? v_old + vt * std::log(arg) : vcrit;
        }
        else
        {
            v_new = vt * std::log(v_new / vt);
        }
    }
    else if (v_new < -vcrit && std::abs(v_new - v_old) > 2 * vt)
    {
        if (v_old < 0)
        {
            const auto arg = 1 + (v_old - v_new) / vt;
            v_new = arg > 0 ? v_old - vt * std::log(arg) : -vcrit;
        }
        else
        {
            v_new = -vt * std::log(-v_new / vt);
        }
    }
    return v_new;
};

static constexpr auto newton_tol_sq = 1.0e-05;
static constexpr int newton_max_iter = 20;

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto Q2N5089_Is = params.Q2N5089_Is;
    const auto Q2N5089_vt = params.Q2N5089_vt;
    const auto Q2N5089_BetaF = params.Q2N5089_BetaF;
    const auto Q2N5089_BetaR = params.Q2N5089_BetaR;
    
    const auto VCC = params.VCC;
    
    const auto vp = VCC;
    
    const auto gC1 = 2.0f * sample_rate * params.C1;
    
    const auto gR1 = 1.0f / params.R1;
    
    const auto gR2 = 1.0f / params.R2;
    
    const auto gRE = 1.0f / params.RE;
    
    const auto gRC = 1.0f / params.RC;
    
    const auto gC2 = 2.0f * sample_rate * params.C2;
    
    const auto gRL = 1.0f / params.RL;
    
    const auto vcrit_Q2N5089_vt = Q2N5089_vt * std::log(Q2N5089_vt / (std::sqrt(2.0) * Q2N5089_Is));
    
    const auto _t0 = (gRC + gC2);
    const auto _t6 = (VCC * gRC);
    const auto _t8 = (gC2 + gRL);
    const auto _t7 = (1.0 / ((_t0 * _t8) - (gC2 * gC2)));
    const auto _t9 = (_t6 * gC2);
    const auto _t10 = (gR1 * VCC);
    const auto _t11 = (1.0 / ((gC1 + gR1) + gR2));
    const auto _t12 = (_t6 * _t8);
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zC1 = state[ch].zC1;
        auto zC2 = state[ch].zC2;
        auto vBCQ1 = state[ch].vBCQ1;
        auto vBEQ1 = state[ch].vBEQ1;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            // --- Newton-Raphson solve: Q1
            const auto _Q1_t3 = (gR1 * VCC);
            const auto _Q1_t16 = (gC1 * vi);
            const auto _Q1_t18 = (gC1 + gR1);
            const auto _Q1_t22 = (gC2 + gRL);
            const auto _Q1_t23 = (1.0 / (((gRC + gC2) * _Q1_t22) - (gC2 * gC2)));
            const auto _Q1_t29 = (1.0 / (Q2N5089_vt * Q2N5089_BetaF));
            const auto _Q1_t39 = (1.0 / (Q2N5089_vt * Q2N5089_BetaR));
            const auto _Q1_t15 = (_Q1_t16 - zC1);
            const auto _Q1_t17 = (1.0 / (_Q1_t18 + gR2));
            const auto _Q1_t45 = (gC2 * zC2);
            const auto _Q1_t46 = ((VCC * gRC) * _Q1_t22);
            for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
            {
                const auto _Q1_t10 = (vBEQ1 / Q2N5089_vt);
                const auto _Q1_t14 = (vBCQ1 / Q2N5089_vt);
                const auto _Q1_t9 = exp(_Q1_t10);
                const auto _Q1_t13 = exp(_Q1_t14);
                const auto _Q1_t21 = (_Q1_t9 - _Q1_t13);
                const auto _Q1_t28 = (Q2N5089_Is * _Q1_t9);
                const auto _Q1_t30 = (_Q1_t28 / Q2N5089_vt);
                const auto _Q1_t38 = (Q2N5089_Is * _Q1_t13);
                const auto _Q1_t42 = (_Q1_t13 / Q2N5089_vt);
                const auto _Q1_t43 = (_Q1_t42 / Q2N5089_BetaR);
                const auto _Q1_t8 = (_Q1_t9 - 1.0);
                const auto _Q1_t12 = (_Q1_t13 - 1.0);
                const auto _Q1_t27 = (_Q1_t28 * _Q1_t29);
                const auto _Q1_t37 = (_Q1_t38 * _Q1_t39);
                const auto _Q1_t41 = (_Q1_t42 + _Q1_t43);
                const auto _Q1_t7 = (_Q1_t8 / Q2N5089_BetaF);
                const auto _Q1_t11 = (_Q1_t12 / Q2N5089_BetaR);
                const auto _Q1_t20 = (_Q1_t21 - _Q1_t11);
                const auto _Q1_t26 = (_Q1_t27 * _Q1_t17);
                const auto _Q1_t31 = ((((_Q1_t28 * _Q1_t22) / Q2N5089_vt) * _Q1_t23) - _Q1_t26);
                const auto _Q1_t36 = (_Q1_t37 * _Q1_t17);
                const auto _Q1_t40 = (Q2N5089_Is * _Q1_t41);
                const auto _Q1_t44 = (_Q1_t36 + ((((_Q1_t38 / Q2N5089_vt) / Q2N5089_BetaR) - _Q1_t40) / gRE));
                const auto _Q1_t6 = (_Q1_t7 + _Q1_t11);
                const auto _Q1_t19 = (Q2N5089_Is * _Q1_t20);
                const auto _Q1_t25 = ((_Q1_t26 + (((_Q1_t30 / Q2N5089_BetaF) + _Q1_t30) / gRE)) + 1.0);
                const auto _Q1_t35 = ((_Q1_t36 + ((_Q1_t40 * _Q1_t22) * _Q1_t23)) + 1.0);
                const auto _Q1_t5 = (Q2N5089_Is * _Q1_t6);
                const auto _Q1_t34 = ((_Q1_t35 * _Q1_t25) + (_Q1_t31 * _Q1_t44));
                const auto _Q1_t4 = (_Q1_t5 - _Q1_t15);
                const auto _Q1_t2 = (_Q1_t3 - _Q1_t4);
                const auto _Q1_t1 = (_Q1_t2 * _Q1_t17);
                const auto _Q1_t33 = (_Q1_t1 - ((_Q1_t5 + _Q1_t19) / gRE));
                const auto _Q1_t0 = (_Q1_t1 + (((((_Q1_t19 - zC2) * _Q1_t22) + _Q1_t45) - _Q1_t46) * _Q1_t23));
                const auto _Q1_t24 = (vBCQ1 - _Q1_t0);
                const auto _Q1_t32 = (vBEQ1 - _Q1_t33);
                const auto res_vBCQ1 = (_Q1_t0 - vBCQ1);
                const auto delta_vBCQ1 = (-(((_Q1_t24 * _Q1_t25) + (_Q1_t31 * _Q1_t32)) / _Q1_t34));
                const auto res_vBEQ1 = (_Q1_t33 - vBEQ1);
                const auto delta_vBEQ1 = (((_Q1_t24 * _Q1_t44) - (_Q1_t35 * _Q1_t32)) / _Q1_t34);
            
                auto residual_norm_sq = 0.0;
                residual_norm_sq += res_vBCQ1 * res_vBCQ1;
                residual_norm_sq += res_vBEQ1 * res_vBEQ1;
                auto step_norm_sq = 0.0;
                step_norm_sq += delta_vBCQ1 * delta_vBCQ1;
                step_norm_sq += delta_vBEQ1 * delta_vBEQ1;
            
                vBCQ1 = limit_junction_voltage(vBCQ1 + delta_vBCQ1, vBCQ1, Q2N5089_vt, vcrit_Q2N5089_vt);
                vBEQ1 = limit_junction_voltage(vBEQ1 + delta_vBEQ1, vBEQ1, Q2N5089_vt, vcrit_Q2N5089_vt);
            
                if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
                    break;
                
            }

            const auto _t2 = exp((vBEQ1 / Q2N5089_vt));
            const auto _t3 = exp((vBCQ1 / Q2N5089_vt));
            const auto _t5 = (_t3 - 1.0);
            const auto _t4 = (_t5 / Q2N5089_BetaR);
            const auto _t1 = ((Q2N5089_Is * ((_t2 - _t3) - _t4)) - zC2);
            const auto vo = (-((((_t0 * zC2) + (_t1 * gC2)) - _t9) * _t7));
            const auto vb = ((_t10 - ((Q2N5089_Is * (((_t2 - 1.0) / Q2N5089_BetaF) + _t4)) - ((gC1 * vi) - zC1))) * _t11);
            const auto tC1 = (gC1 * (vi - vb));
            const auto vc = (-((((_t1 * _t8) + (gC2 * zC2)) - _t12) * _t7));
            const auto tC2 = (gC2 * (vc - vo));
            
            zC1 = 2 * tC1 - zC1;
            zC2 = 2 * tC2 - zC2;

            output[ch][n] = vo;
        }
        state[ch].zC1 = zC1;
        state[ch].zC2 = zC2;
        state[ch].vBCQ1 = vBCQ1;
        state[ch].vBEQ1 = vBEQ1;
    }
}

static void reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc = 0.0f)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto Q2N5089_Is = params.Q2N5089_Is;
    const auto Q2N5089_vt = params.Q2N5089_vt;
    const auto Q2N5089_BetaF = params.Q2N5089_BetaF;
    const auto Q2N5089_BetaR = params.Q2N5089_BetaR;
    
    const auto VCC = params.VCC;
    
    const auto vp = VCC;
    
    const auto gC1 = 2.0f * sample_rate * params.C1;
    
    const auto gR1 = 1.0f / params.R1;
    
    const auto gR2 = 1.0f / params.R2;
    
    const auto gRE = 1.0f / params.RE;
    
    const auto gRC = 1.0f / params.RC;
    
    const auto gC2 = 2.0f * sample_rate * params.C2;
    
    const auto gRL = 1.0f / params.RL;
    
    const auto vcrit_Q2N5089_vt = Q2N5089_vt * std::log(Q2N5089_vt / (std::sqrt(2.0) * Q2N5089_Is));
    
    const auto vi = vi_dc;

    float vBCQ1 = 0;
    float vBEQ1 = 0;

    // --- Newton-Raphson solve: Q1
    const auto _Q1_t3 = (gR1 * VCC);
    const auto _Q1_t15 = (gR1 + gR2);
    const auto _Q1_t16 = (1.0 / 1000000000.0);
    const auto _Q1_t20 = (1.0 / (gRC + _Q1_t16));
    const auto _Q1_t26 = (1.0 / (Q2N5089_vt * Q2N5089_BetaF));
    const auto _Q1_t28 = (1.0 / (gRE + _Q1_t16));
    const auto _Q1_t37 = (1.0 / (Q2N5089_vt * Q2N5089_BetaR));
    const auto _Q1_t14 = (1.0 / (_Q1_t15 + _Q1_t16));
    const auto _Q1_t43 = (VCC * gRC);
    for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
    {
        const auto _Q1_t9 = (vBEQ1 / Q2N5089_vt);
        const auto _Q1_t13 = (vBCQ1 / Q2N5089_vt);
        const auto _Q1_t8 = exp(_Q1_t9);
        const auto _Q1_t12 = exp(_Q1_t13);
        const auto _Q1_t19 = (_Q1_t8 - _Q1_t12);
        const auto _Q1_t25 = (Q2N5089_Is * _Q1_t8);
        const auto _Q1_t27 = (_Q1_t25 / Q2N5089_vt);
        const auto _Q1_t36 = (Q2N5089_Is * _Q1_t12);
        const auto _Q1_t40 = (_Q1_t12 / Q2N5089_vt);
        const auto _Q1_t41 = (_Q1_t40 / Q2N5089_BetaR);
        const auto _Q1_t7 = (_Q1_t8 - 1.0);
        const auto _Q1_t11 = (_Q1_t12 - 1.0);
        const auto _Q1_t24 = (_Q1_t25 * _Q1_t26);
        const auto _Q1_t35 = (_Q1_t36 * _Q1_t37);
        const auto _Q1_t39 = (_Q1_t40 + _Q1_t41);
        const auto _Q1_t6 = (_Q1_t7 / Q2N5089_BetaF);
        const auto _Q1_t10 = (_Q1_t11 / Q2N5089_BetaR);
        const auto _Q1_t18 = (_Q1_t19 - _Q1_t10);
        const auto _Q1_t23 = (_Q1_t24 * _Q1_t14);
        const auto _Q1_t29 = ((_Q1_t27 * _Q1_t20) - _Q1_t23);
        const auto _Q1_t34 = (_Q1_t35 * _Q1_t14);
        const auto _Q1_t38 = (Q2N5089_Is * _Q1_t39);
        const auto _Q1_t42 = (_Q1_t34 + ((((_Q1_t36 / Q2N5089_vt) / Q2N5089_BetaR) - _Q1_t38) * _Q1_t28));
        const auto _Q1_t5 = (_Q1_t6 + _Q1_t10);
        const auto _Q1_t17 = (Q2N5089_Is * _Q1_t18);
        const auto _Q1_t22 = ((_Q1_t23 + (((_Q1_t27 / Q2N5089_BetaF) + _Q1_t27) * _Q1_t28)) + 1.0);
        const auto _Q1_t33 = ((_Q1_t34 + (_Q1_t38 * _Q1_t20)) + 1.0);
        const auto _Q1_t4 = (Q2N5089_Is * _Q1_t5);
        const auto _Q1_t32 = ((_Q1_t33 * _Q1_t22) + (_Q1_t29 * _Q1_t42));
        const auto _Q1_t2 = (_Q1_t3 - _Q1_t4);
        const auto _Q1_t1 = (_Q1_t2 * _Q1_t14);
        const auto _Q1_t31 = (_Q1_t1 - ((_Q1_t4 + _Q1_t17) * _Q1_t28));
        const auto _Q1_t0 = (_Q1_t1 + ((_Q1_t17 - _Q1_t43) * _Q1_t20));
        const auto _Q1_t21 = (vBCQ1 - _Q1_t0);
        const auto _Q1_t30 = (vBEQ1 - _Q1_t31);
        const auto res_vBCQ1 = (_Q1_t0 - vBCQ1);
        const auto delta_vBCQ1 = (-(((_Q1_t21 * _Q1_t22) + (_Q1_t29 * _Q1_t30)) / _Q1_t32));
        const auto res_vBEQ1 = (_Q1_t31 - vBEQ1);
        const auto delta_vBEQ1 = (((_Q1_t21 * _Q1_t42) - (_Q1_t33 * _Q1_t30)) / _Q1_t32);
    
        auto residual_norm_sq = 0.0;
        residual_norm_sq += res_vBCQ1 * res_vBCQ1;
        residual_norm_sq += res_vBEQ1 * res_vBEQ1;
        auto step_norm_sq = 0.0;
        step_norm_sq += delta_vBCQ1 * delta_vBCQ1;
        step_norm_sq += delta_vBEQ1 * delta_vBEQ1;
    
        vBCQ1 = limit_junction_voltage(vBCQ1 + delta_vBCQ1, vBCQ1, Q2N5089_vt, vcrit_Q2N5089_vt);
        vBEQ1 = limit_junction_voltage(vBEQ1 + delta_vBEQ1, vBEQ1, Q2N5089_vt, vcrit_Q2N5089_vt);
    
        if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
            break;
        
    }
    const auto zC1 = (gC1 * (vi - (((gR1 * VCC) - (Q2N5089_Is * (((exp((vBEQ1 / Q2N5089_vt)) - 1.0) / Q2N5089_BetaF) + ((exp((vBCQ1 / Q2N5089_vt)) - 1.0) / Q2N5089_BetaR)))) / ((gR1 + gR2) + (1.0 / 1000000000.0)))));
    const auto zC2 = (-((gC2 * ((Q2N5089_Is * ((exp((vBEQ1 / Q2N5089_vt)) - exp((vBCQ1 / Q2N5089_vt))) - ((exp((vBCQ1 / Q2N5089_vt)) - 1.0) / Q2N5089_BetaR))) - (VCC * gRC))) / (gRC + (1.0 / 1000000000.0))));

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].vBCQ1 = vBCQ1;
        state[ch].vBEQ1 = vBEQ1;
        state[ch].zC1 = zC1;
        state[ch].zC2 = zC2;
    }
}
