// Auto-generated with netlist_codegen version de49196.
// Command: netlist_codegen common_emitter_pnp.net common_emitter_pnp.h -opt_port_matrix

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
    const auto _t5 = (gC2 * 128.0f);
    const auto _t6 = ((gRC + gC2) * 128.0f);
    const auto _t8 = (gRC * 128.0f);
    const auto _t9 = (262144.0f * gRE);
    const auto _t11 = ((gC2 + gRL) * 128.0f);
    const auto _t10 = ((_t6 * _t11) - (_t5 * _t5));
    const auto _t13 = (1.0f / (_t9 * _t10));
    const auto _t14 = (VEE * (_t0 * (_t8 * _t5)));
    const auto _t15 = (gR1 * VEE);
    const auto _t16 = (1.0f / ((gC1 + gR1) + gR2));
    const auto _t17 = (VEE * (_t0 * (_t8 * _t11)));
    const auto _Q1_zt1 = (gC1 + gR1);
    const auto _Q1_zt4 = (_Q1_zt1 + gR2);
    const auto _Q1_zt5 = (1.0f / gRE);
    const auto _Q1_zt6 = (gC2 * 128.0f);
    const auto _Q1_zt7 = (gRE * 2048.0f);
    const auto _Q1_zt8 = (gC2 + gRL);
    const auto _Q1_zt0 = (1.0f / _Q1_zt4);
    const auto _Q1_zt2 = (_Q1_zt8 * 128.0f);
    const auto _Q1_zt3 = ((262144.0f * gRE) * ((((gRC + gC2) * 128.0f) * _Q1_zt2) - (_Q1_zt6 * _Q1_zt6)));
    const auto _Q1_Z0_0 = (-_Q1_zt0);
    const auto _Q1_Z0_1 = ((128.0f * (_Q1_zt7 * (_Q1_zt2 * (128.0f * 1.0f)))) / _Q1_zt3);
    const auto _Q1_Z1_0 = (-(_Q1_zt5 + _Q1_zt0));
    const auto _Q1_Z1_1 = (-_Q1_zt5);
    float c0__Q1_voc0;
    float c__Q1_voc0[3];
    float c0__Q1_voc1;
    float c__Q1_voc1[3];
    float c0__Q1_zt9;
    float c__Q1_zt9[3];
    float c0__Q1_zt10;
    float c__Q1_zt10[3];
    
    for (int _k = 0; _k <= 3; ++_k)
    {
        const auto vi = (_k == 1) ? 1.0f : 0.0f;
        const auto zC1 = (_k == 2) ? 1.0f : 0.0f;
        const auto zC2 = (_k == 3) ? 1.0f : 0.0f;
        const auto _Q1_zt12 = (gR1 * VEE);
        const auto _Q1_zt13 = (gC1 * vi);
        const auto _Q1_zt11 = (_Q1_zt13 - zC1);
        const auto _Q1_zt10 = (_Q1_zt11 + _Q1_zt12);
        const auto _Q1_zt9 = (_Q1_zt10 / _Q1_zt4);
        const auto _Q1_voc0 = (((128.0f * ((_Q1_zt7 * ((_Q1_zt2 * (128.0f * zC2)) - (_Q1_zt6 * (zC2 * 128.0f)))) + (VEE * (_Q1_zt7 * ((gRC * 128.0f) * _Q1_zt2))))) / _Q1_zt3) - _Q1_zt9);
        const auto _Q1_voc1 = (-_Q1_zt9);
        if (_k == 0) {
            c0__Q1_voc0 = _Q1_voc0;
            c0__Q1_voc1 = _Q1_voc1;
            c0__Q1_zt9 = _Q1_zt9;
            c0__Q1_zt10 = _Q1_zt10;
        } else {
            c__Q1_voc0[_k - 1] = _Q1_voc0 - c0__Q1_voc0;
            c__Q1_voc1[_k - 1] = _Q1_voc1 - c0__Q1_voc1;
            c__Q1_zt9[_k - 1] = _Q1_zt9 - c0__Q1_zt9;
            c__Q1_zt10[_k - 1] = _Q1_zt10 - c0__Q1_zt10;
        }
    }
    
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zC1 = state[ch].zC1;
        auto zC2 = state[ch].zC2;
        auto vCBQ1 = state[ch].vCBQ1;
        auto vEBQ1 = state[ch].vEBQ1;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            // --- Newton-Raphson solve (N-port): Q1
            const auto _Q1_voc0 = c0__Q1_voc0 + c__Q1_voc0[0] * vi + c__Q1_voc0[1] * zC1 + c__Q1_voc0[2] * zC2;
            const auto _Q1_voc1 = c0__Q1_voc1 + c__Q1_voc1[0] * vi + c__Q1_voc1[1] * zC1 + c__Q1_voc1[2] * zC2;
            const auto _Q1_zt9 = c0__Q1_zt9 + c__Q1_zt9[0] * vi + c__Q1_zt9[1] * zC1 + c__Q1_zt9[2] * zC2;
            const auto _Q1_zt10 = c0__Q1_zt10 + c__Q1_zt10[0] * vi + c__Q1_zt10[1] * zC1 + c__Q1_zt10[2] * zC2;
            const auto _Q1_zt12 = (gR1 * VEE);
            const auto _Q1_zt13 = (gC1 * vi);
            const auto _Q1_zt11 = (_Q1_zt13 - zC1);
            for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
            {
                const auto _Q1_pt0 = (vEBQ1 / Q2N5087_vt);
                const auto _Q1_pt2 = (vCBQ1 / Q2N5087_vt);
                const auto _Q1_pt5 = math_exp_approx(_Q1_pt0);
                const auto _Q1_pt6 = math_exp_approx(_Q1_pt2);
                const auto _Q1_pt7 = (_Q1_pt6 / Q2N5087_vt);
                const auto _Q1_pt8 = (Q2N5087_Is * _Q1_pt5);
                const auto _Q1_pt3 = (_Q1_pt6 - 1.0f);
                const auto _Q1_pt4 = (_Q1_pt8 / Q2N5087_vt);
                const auto _Q1_pt1 = (_Q1_pt3 / Q2N5087_BetaR);
                const auto _Q1_i0 = (Q2N5087_Is * (((_Q1_pt5 - 1.0f) / Q2N5087_BetaF) + _Q1_pt1));
                const auto _Q1_i1 = (Q2N5087_Is * ((_Q1_pt5 - _Q1_pt6) - _Q1_pt1));
                const auto _Q1_g0_0 = (((Q2N5087_Is * _Q1_pt6) / Q2N5087_vt) / Q2N5087_BetaR);
                const auto _Q1_g0_1 = (_Q1_pt4 / Q2N5087_BetaF);
                const auto _Q1_g1_0 = (-(Q2N5087_Is * (_Q1_pt7 + (_Q1_pt7 / Q2N5087_BetaR))));
                const auto _Q1_g1_1 = _Q1_pt4;
                const auto _Q1_pt9 = ((_Q1_voc0 + (_Q1_Z0_0 * _Q1_i0)) + (_Q1_Z0_1 * _Q1_i1));
                const auto _Q1_pt10 = ((_Q1_voc1 + (_Q1_Z1_0 * _Q1_i0)) + (_Q1_Z1_1 * _Q1_i1));
                const auto _Q1_pt11 = (vCBQ1 - _Q1_pt9);
                const auto _Q1_pt12 = (((_Q1_Z1_0 * _Q1_g0_1) + (_Q1_Z1_1 * _Q1_g1_1)) - 1.0f);
                const auto _Q1_pt13 = ((_Q1_Z0_0 * _Q1_g0_1) + (_Q1_Z0_1 * _Q1_g1_1));
                const auto _Q1_pt14 = (vEBQ1 - _Q1_pt10);
                const auto _Q1_pt16 = (((_Q1_Z0_0 * _Q1_g0_0) + (_Q1_Z0_1 * _Q1_g1_0)) - 1.0f);
                const auto _Q1_pt17 = ((_Q1_Z1_0 * _Q1_g0_0) + (_Q1_Z1_1 * _Q1_g1_0));
                const auto _Q1_pt15 = ((_Q1_pt16 * _Q1_pt12) - (_Q1_pt13 * _Q1_pt17));
                const auto res_vCBQ1 = (_Q1_pt9 - vCBQ1);
                const auto res_vEBQ1 = (_Q1_pt10 - vEBQ1);
                const auto delta_vCBQ1 = (((_Q1_pt11 * _Q1_pt12) - (_Q1_pt13 * _Q1_pt14)) / _Q1_pt15);
                const auto delta_vEBQ1 = (((_Q1_pt16 * _Q1_pt14) - (_Q1_pt11 * _Q1_pt17)) / _Q1_pt15);
            
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
            const auto _t4 = (_t3 - 1.0f);
            const auto _t7 = (zC2 * 128.0f);
            const auto _t12 = (_t4 / Q2N5087_BetaR);
            const auto _t1 = ((zC2 + (Q2N5087_Is * ((_t2 - _t3) - _t12))) * 128.0f);
            const auto vo = ((128.0f * ((_t0 * ((_t1 * _t5) - (_t6 * _t7))) + _t14)) * _t13);
            const auto vb = (((((gC1 * vi) - zC1) + (Q2N5087_Is * (((_t2 - 1.0f) / Q2N5087_BetaF) + _t12))) + _t15) * _t16);
            const auto tC1 = (gC1 * (vi - vb));
            const auto vc = ((128.0f * ((_t0 * ((_t1 * _t11) - (_t5 * _t7))) + _t17)) * _t13);
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

    const auto _Q1_zt0 = (gR1 + gR2);
    const auto _Q1_zt1 = (1.0f / 1000000000.0f);
    const auto _Q1_zt2 = (gRC + _Q1_zt1);
    const auto _Q1_zt4 = (gRE + _Q1_zt1);
    const auto _Q1_zt7 = (_Q1_zt0 + _Q1_zt1);
    const auto _Q1_zt8 = (1.0f / _Q1_zt4);
    const auto _Q1_zt3 = (-_Q1_zt8);
    const auto _Q1_zt6 = (1.0f / _Q1_zt7);
    const auto _Q1_zt5 = (-_Q1_zt6);
    const auto _Q1_Z0_0 = _Q1_zt5;
    const auto _Q1_Z0_1 = (1.0f / _Q1_zt2);
    const auto _Q1_Z1_0 = (_Q1_zt3 + _Q1_zt5);
    const auto _Q1_Z1_1 = _Q1_zt3;
    // --- Newton-Raphson solve (N-port): Q1
    const auto _Q1_zt9 = (gR1 * VEE);
    const auto _Q1_zt10 = (_Q1_zt9 / _Q1_zt7);
    const auto _Q1_voc0 = (((VEE * gRC) / _Q1_zt2) - _Q1_zt10);
    const auto _Q1_voc1 = (-_Q1_zt10);
    for (int newton_iter = 0; newton_iter < 10000; ++newton_iter)
    {
        const auto _Q1_pt0 = (vEBQ1 / Q2N5087_vt);
        const auto _Q1_pt2 = (vCBQ1 / Q2N5087_vt);
        const auto _Q1_pt5 = math_exp_approx(_Q1_pt0);
        const auto _Q1_pt6 = math_exp_approx(_Q1_pt2);
        const auto _Q1_pt7 = (_Q1_pt6 / Q2N5087_vt);
        const auto _Q1_pt8 = (Q2N5087_Is * _Q1_pt5);
        const auto _Q1_pt3 = (_Q1_pt6 - 1.0f);
        const auto _Q1_pt4 = (_Q1_pt8 / Q2N5087_vt);
        const auto _Q1_pt1 = (_Q1_pt3 / Q2N5087_BetaR);
        const auto _Q1_i0 = (Q2N5087_Is * (((_Q1_pt5 - 1.0f) / Q2N5087_BetaF) + _Q1_pt1));
        const auto _Q1_i1 = (Q2N5087_Is * ((_Q1_pt5 - _Q1_pt6) - _Q1_pt1));
        const auto _Q1_g0_0 = (((Q2N5087_Is * _Q1_pt6) / Q2N5087_vt) / Q2N5087_BetaR);
        const auto _Q1_g0_1 = (_Q1_pt4 / Q2N5087_BetaF);
        const auto _Q1_g1_0 = (-(Q2N5087_Is * (_Q1_pt7 + (_Q1_pt7 / Q2N5087_BetaR))));
        const auto _Q1_g1_1 = _Q1_pt4;
        const auto _Q1_pt9 = ((_Q1_voc0 + (_Q1_Z0_0 * _Q1_i0)) + (_Q1_Z0_1 * _Q1_i1));
        const auto _Q1_pt10 = ((_Q1_voc1 + (_Q1_Z1_0 * _Q1_i0)) + (_Q1_Z1_1 * _Q1_i1));
        const auto _Q1_pt11 = (vCBQ1 - _Q1_pt9);
        const auto _Q1_pt12 = (((_Q1_Z1_0 * _Q1_g0_1) + (_Q1_Z1_1 * _Q1_g1_1)) - 1.0f);
        const auto _Q1_pt13 = ((_Q1_Z0_0 * _Q1_g0_1) + (_Q1_Z0_1 * _Q1_g1_1));
        const auto _Q1_pt14 = (vEBQ1 - _Q1_pt10);
        const auto _Q1_pt16 = (((_Q1_Z0_0 * _Q1_g0_0) + (_Q1_Z0_1 * _Q1_g1_0)) - 1.0f);
        const auto _Q1_pt17 = ((_Q1_Z1_0 * _Q1_g0_0) + (_Q1_Z1_1 * _Q1_g1_0));
        const auto _Q1_pt15 = ((_Q1_pt16 * _Q1_pt12) - (_Q1_pt13 * _Q1_pt17));
        const auto res_vCBQ1 = (_Q1_pt9 - vCBQ1);
        const auto res_vEBQ1 = (_Q1_pt10 - vEBQ1);
        const auto delta_vCBQ1 = (((_Q1_pt11 * _Q1_pt12) - (_Q1_pt13 * _Q1_pt14)) / _Q1_pt15);
        const auto delta_vEBQ1 = (((_Q1_pt16 * _Q1_pt14) - (_Q1_pt11 * _Q1_pt17)) / _Q1_pt15);
    
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

