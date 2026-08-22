// Auto-generated with netlist_codegen version ac50416.
// Command: netlist_codegen diode_clipper.net diode_clipper.h -opt_port_matrix

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


[[maybe_unused]] static auto limit_junction_voltage_sym = [](auto v_new, auto v_old, auto vt, auto vcrit)
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
    float R1 = 1.0e+03f;
    float C1 = 1.0e-06f;
    float D1N914_Is = 2.52e-09f;
    float D1N914_vt = 4.52892e-02f;
};

struct State {
    float zC1 {};
    float vD1D2 {};
};

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto D1N914_Is = params.D1N914_Is;
    const auto D1N914_vt = params.D1N914_vt;
    
    const auto gR1 = 1.0f / params.R1;
    
    const auto gC1 = 2.0f * sample_rate * params.C1;
    
    const auto vcrit_D1N914_vt = D1N914_vt * std::log(D1N914_vt / (std::sqrt(2.0) * D1N914_Is));
    
    const auto _t1 = (1.0f / (gR1 + gC1));
    const auto _D1D2_zt0 = (gR1 + gC1);
    const auto _D1D2_Z0_0 = (-(1.0f / _D1D2_zt0));
    float c0__D1D2_voc0;
    float c__D1D2_voc0[2];
    
    for (int _k = 0; _k <= 2; ++_k)
    {
        const auto vi = (_k == 1) ? 1.0f : 0.0f;
        const auto zC1 = (_k == 2) ? 1.0f : 0.0f;
        const auto _D1D2_voc0 = (-((-((gR1 * vi) + zC1)) / _D1D2_zt0));
        if (_k == 0) {
            c0__D1D2_voc0 = _D1D2_voc0;
        } else {
            c__D1D2_voc0[_k - 1] = _D1D2_voc0 - c0__D1D2_voc0;
        }
    }
    
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zC1 = state[ch].zC1;
        auto vD1D2 = state[ch].vD1D2;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            // --- Newton-Raphson solve (N-port): D1D2
            const auto _D1D2_voc0 = c0__D1D2_voc0 + c__D1D2_voc0[0] * vi + c__D1D2_voc0[1] * zC1;
            const auto _D1D2_pt3 = (1.0f / D1N914_vt);
            for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
            {
                const auto _D1D2_pt0 = (vD1D2 / D1N914_vt);
                const auto _D1D2_pt1 = math_exp_approx(_D1D2_pt0);
                const auto _D1D2_i0 = (D1N914_Is * (_D1D2_pt1 - (1.0f / _D1D2_pt1)));
                const auto _D1D2_g0_0 = (D1N914_Is * ((_D1D2_pt1 / D1N914_vt) + (_D1D2_pt3 / _D1D2_pt1)));
                const auto _D1D2_pt2 = (_D1D2_voc0 + (_D1D2_Z0_0 * _D1D2_i0));
                const auto res_vD1D2 = (_D1D2_pt2 - vD1D2);
                const auto delta_vD1D2 = ((vD1D2 - _D1D2_pt2) / ((_D1D2_Z0_0 * _D1D2_g0_0) - 1.0f));
            
                auto residual_norm_sq = 0.0;
                residual_norm_sq += res_vD1D2 * res_vD1D2;
                auto step_norm_sq = 0.0;
                step_norm_sq += delta_vD1D2 * delta_vD1D2;
            
                vD1D2 = limit_junction_voltage_sym(vD1D2 + (delta_vD1D2), vD1D2, D1N914_vt, vcrit_D1N914_vt);
            
                if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
                    break;
                
            }

            const auto _t0 = math_exp_approx((vD1D2 / D1N914_vt));
            const auto vo = (-(((D1N914_Is * (_t0 - (1.0f / _t0))) - ((gR1 * vi) + zC1)) * _t1));
            const auto tC1 = (gC1 * (vo - 0));
            
            zC1 = 2 * tC1 - zC1;

            output[ch][n] = vo;
        }
        state[ch].zC1 = zC1;
        state[ch].vD1D2 = vD1D2;
    }
}

static float reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc = 0.0f)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto D1N914_Is = params.D1N914_Is;
    const auto D1N914_vt = params.D1N914_vt;
    
    const auto gR1 = 1.0f / params.R1;
    
    const auto gC1 = 2.0f * sample_rate * params.C1;
    
    const auto vcrit_D1N914_vt = D1N914_vt * std::log(D1N914_vt / (std::sqrt(2.0) * D1N914_Is));
    
    const auto vi = vi_dc;

    float vD1D2 = 0;

    const auto _D1D2_zt0 = (gR1 + (1.0f / 1000000000.0f));
    const auto _D1D2_Z0_0 = (-(1.0f / _D1D2_zt0));
    // --- Newton-Raphson solve (N-port): D1D2
    const auto _D1D2_voc0 = (-((-(gR1 * vi)) / _D1D2_zt0));
    const auto _D1D2_pt3 = (1.0f / D1N914_vt);
    for (int newton_iter = 0; newton_iter < 10000; ++newton_iter)
    {
        const auto _D1D2_pt0 = (vD1D2 / D1N914_vt);
        const auto _D1D2_pt1 = math_exp_approx(_D1D2_pt0);
        const auto _D1D2_i0 = (D1N914_Is * (_D1D2_pt1 - (1.0f / _D1D2_pt1)));
        const auto _D1D2_g0_0 = (D1N914_Is * ((_D1D2_pt1 / D1N914_vt) + (_D1D2_pt3 / _D1D2_pt1)));
        const auto _D1D2_pt2 = (_D1D2_voc0 + (_D1D2_Z0_0 * _D1D2_i0));
        const auto res_vD1D2 = (_D1D2_pt2 - vD1D2);
        const auto delta_vD1D2 = ((vD1D2 - _D1D2_pt2) / ((_D1D2_Z0_0 * _D1D2_g0_0) - 1.0f));
    
        auto residual_norm_sq = 0.0;
        residual_norm_sq += res_vD1D2 * res_vD1D2;
        auto step_norm_sq = 0.0;
        step_norm_sq += delta_vD1D2 * delta_vD1D2;
    
        vD1D2 = limit_junction_voltage_sym(vD1D2 + (delta_vD1D2), vD1D2, D1N914_vt, vcrit_D1N914_vt);
    
        if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
            break;
        
    }
    const auto zC1 = (-((gC1 * ((D1N914_Is * (math_exp_approx((vD1D2 / D1N914_vt)) - (1.0f / math_exp_approx((vD1D2 / D1N914_vt))))) - (gR1 * vi))) / (gR1 + (1.0f / 1000000000.0f))));

    const auto vo_dc_out = (-(((D1N914_Is * (math_exp_approx((vD1D2 / D1N914_vt)) - (1.0f / math_exp_approx((vD1D2 / D1N914_vt))))) - (gR1 * vi)) / (gR1 + (1.0f / 1000000000.0f))));

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].vD1D2 = vD1D2;
        state[ch].zC1 = zC1;
    }
    return vo_dc_out;
}

