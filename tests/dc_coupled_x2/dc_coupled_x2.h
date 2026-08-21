// Auto-generated with netlist_codegen version d1e5ccb.
// Command: netlist_codegen dc_coupled_x2.net dc_coupled_x2.h -type_name float

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


struct Params_stage1 {
    float Vbias = 5.0e+00f;
    float R1 = 1.0e+03f;
    float R2 = 1.0e+03f;
    float D1N914_Is = 2.52e-09f;
    float D1N914_vt = 4.52892e-02f;
    float R3 = 1.0e+04f;
    float C1 = 9.999999e-08f;
};

struct State_stage1 {
    float zR3C1 {};
    float vD1 {};
};

static void compute_stage1 (const float* const* input, float** output, int num_channels, int num_samples, Params_stage1 params, State_stage1* state, float sample_rate)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto D1N914_Is = params.D1N914_Is;
    const auto D1N914_vt = params.D1N914_vt;
    
    const auto Vbias = params.Vbias;
    
    const auto vbias = Vbias;
    
    const auto gR1 = 1.0f / params.R1;
    
    const auto gR2 = 1.0f / params.R2;
    
    const auto R3 = params.R3;
    const auto C1 = params.C1;
    const auto gR3C1 = 2.0f * sample_rate * C1 + (1.0f / R3);
    const auto gzR3C1 = 4.0f * sample_rate * C1;
    
    const auto vcrit_D1N914_vt = D1N914_vt * std::log(D1N914_vt / (std::sqrt(2.0) * D1N914_Is));
    
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zR3C1 = state[ch].zR3C1;
        auto vD1 = state[ch].vD1;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            // --- Newton-Raphson solve: D1
            const auto _D1_t3 = (1.0f / (gR1 + gR2));
            const auto _D1_t9 = (gR1 * vi);
            const auto _D1_t10 = (gR2 * Vbias);
            for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
            {
                const auto _D1_t2 = (vD1 / D1N914_vt);
                const auto _D1_t8 = math_exp_approx(_D1_t2);
                const auto _D1_t1 = (_D1_t8 - 1.0f);
                const auto _D1_t4 = (D1N914_Is * _D1_t1);
                const auto _D1_t6 = (D1N914_Is * _D1_t8);
                const auto _D1_t7 = (_D1_t6 / D1N914_vt);
                const auto _D1_t0 = ((((_D1_t4 - _D1_t9) - _D1_t10) * _D1_t3) + ((zR3C1 + _D1_t4) / gR3C1));
                const auto _D1_t5 = (_D1_t0 + vD1);
                const auto res_vD1 = (-_D1_t5);
                const auto delta_vD1 = (-(_D1_t5 / (((_D1_t7 * _D1_t3) + (_D1_t7 / gR3C1)) + 1.0f)));
            
                auto residual_norm_sq = 0.0;
                residual_norm_sq += res_vD1 * res_vD1;
                auto step_norm_sq = 0.0;
                step_norm_sq += delta_vD1 * delta_vD1;
            
                vD1 = limit_junction_voltage(vD1 + (delta_vD1), vD1, D1N914_vt, vcrit_D1N914_vt);
            
                if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
                    break;
                
            }

            const auto vo1 = ((zR3C1 + (D1N914_Is * (math_exp_approx((vD1 / D1N914_vt)) - 1.0f))) / gR3C1);
            const auto vR3C1 = (vo1 - 0);
            
            zR3C1 = gzR3C1 * vR3C1 - zR3C1; // RC parallel

            output[ch][n] = vo1;
        }
        state[ch].zR3C1 = zR3C1;
        state[ch].vD1 = vD1;
    }
}

static float reset_stage1 (Params_stage1 params, State_stage1* state, int num_channels, float sample_rate, float vi_dc = 0.0f)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto D1N914_Is = params.D1N914_Is;
    const auto D1N914_vt = params.D1N914_vt;
    
    const auto Vbias = params.Vbias;
    
    const auto vbias = Vbias;
    
    const auto gR1 = 1.0f / params.R1;
    
    const auto gR2 = 1.0f / params.R2;
    
    const auto R3 = params.R3;
    const auto C1 = params.C1;
    const auto gR3C1 = 2.0f * sample_rate * C1 + (1.0f / R3);
    const auto gzR3C1 = 4.0f * sample_rate * C1;
    
    const auto vcrit_D1N914_vt = D1N914_vt * std::log(D1N914_vt / (std::sqrt(2.0) * D1N914_Is));
    
    const auto vi = vi_dc;

    float vD1 = 0;

    // --- Newton-Raphson solve: D1
    const auto _D1_t2 = (1.0f / ((gR1 + gR2) + (1.0f / 1000000000.0f)));
    const auto _D1_t8 = (gR1 * vi);
    const auto _D1_t9 = ((gR2 * Vbias) / R3);
    for (int newton_iter = 0; newton_iter < 10000; ++newton_iter)
    {
        const auto _D1_t5 = (vD1 / D1N914_vt);
        const auto _D1_t4 = math_exp_approx(_D1_t5);
        const auto _D1_t7 = (D1N914_Is * _D1_t4);
        const auto _D1_t3 = (_D1_t4 - 1.0f);
        const auto _D1_t1 = (D1N914_Is * _D1_t3);
        const auto _D1_t0 = ((((((_D1_t1 - _D1_t8) / R3) - _D1_t9) * R3) * _D1_t2) + (_D1_t1 * R3));
        const auto _D1_t6 = (_D1_t0 + vD1);
        const auto res_vD1 = (-_D1_t6);
        const auto delta_vD1 = (-(_D1_t6 / ((((_D1_t7 / D1N914_vt) * _D1_t2) + ((_D1_t7 * R3) / D1N914_vt)) + 1.0f)));
    
        auto residual_norm_sq = 0.0;
        residual_norm_sq += res_vD1 * res_vD1;
        auto step_norm_sq = 0.0;
        step_norm_sq += delta_vD1 * delta_vD1;
    
        vD1 = limit_junction_voltage(vD1 + (delta_vD1), vD1, D1N914_vt, vcrit_D1N914_vt);
    
        if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
            break;
        
    }
    const auto zR3C1 = ((gzR3C1 * ((D1N914_Is * (math_exp_approx((vD1 / D1N914_vt)) - 1.0f)) * R3)) / 2.0f);

    const auto vo1_dc_out = ((D1N914_Is * (math_exp_approx((vD1 / D1N914_vt)) - 1.0f)) * R3);

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].vD1 = vD1;
        state[ch].zR3C1 = zR3C1;
    }
    return vo1_dc_out;
}

struct Params_stage2 {
    float R1x = 1.0e+03f;
    float D1N914_Is = 2.52e-09f;
    float D1N914_vt = 4.52892e-02f;
    float R3x = 1.0e+04f;
    float C1x = 9.999999e-08f;
};

struct State_stage2 {
    float zR3xC1x {};
    float vD1x {};
};

static void compute_stage2 (const float* const* input, float** output, int num_channels, int num_samples, Params_stage2 params, State_stage2* state, float sample_rate)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto D1N914_Is = params.D1N914_Is;
    const auto D1N914_vt = params.D1N914_vt;
    
    const auto gR1x = 1.0f / params.R1x;
    
    const auto R3x = params.R3x;
    const auto C1x = params.C1x;
    const auto gR3xC1x = 2.0f * sample_rate * C1x + (1.0f / R3x);
    const auto gzR3xC1x = 4.0f * sample_rate * C1x;
    
    const auto vcrit_D1N914_vt = D1N914_vt * std::log(D1N914_vt / (std::sqrt(2.0) * D1N914_Is));
    
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zR3xC1x = state[ch].zR3xC1x;
        auto vD1x = state[ch].vD1x;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vo1 = input[ch][n];

            // --- Newton-Raphson solve: D1x
            const auto _D1x_t8 = (gR1x * vo1);
            for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
            {
                const auto _D1x_t2 = (vD1x / D1N914_vt);
                const auto _D1x_t7 = math_exp_approx(_D1x_t2);
                const auto _D1x_t1 = (_D1x_t7 - 1.0f);
                const auto _D1x_t3 = (D1N914_Is * _D1x_t1);
                const auto _D1x_t6 = (D1N914_Is * _D1x_t7);
                const auto _D1x_t0 = (((_D1x_t3 - _D1x_t8) / gR1x) + ((zR3xC1x + _D1x_t3) / gR3xC1x));
                const auto _D1x_t4 = (_D1x_t0 + vD1x);
                const auto _D1x_t5 = (_D1x_t6 / D1N914_vt);
                const auto res_vD1x = (-_D1x_t4);
                const auto delta_vD1x = (-(_D1x_t4 / (((_D1x_t5 / gR1x) + (_D1x_t5 / gR3xC1x)) + 1.0f)));
            
                auto residual_norm_sq = 0.0;
                residual_norm_sq += res_vD1x * res_vD1x;
                auto step_norm_sq = 0.0;
                step_norm_sq += delta_vD1x * delta_vD1x;
            
                vD1x = limit_junction_voltage(vD1x + (delta_vD1x), vD1x, D1N914_vt, vcrit_D1N914_vt);
            
                if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
                    break;
                
            }

            const auto vo2 = ((zR3xC1x + (D1N914_Is * (math_exp_approx((vD1x / D1N914_vt)) - 1.0f))) / gR3xC1x);
            const auto vR3xC1x = (vo2 - 0);
            
            zR3xC1x = gzR3xC1x * vR3xC1x - zR3xC1x; // RC parallel

            output[ch][n] = vo2;
        }
        state[ch].zR3xC1x = zR3xC1x;
        state[ch].vD1x = vD1x;
    }
}

static float reset_stage2 (Params_stage2 params, State_stage2* state, int num_channels, float sample_rate, float vo1_dc = 0.0f)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto D1N914_Is = params.D1N914_Is;
    const auto D1N914_vt = params.D1N914_vt;
    
    const auto gR1x = 1.0f / params.R1x;
    
    const auto R3x = params.R3x;
    const auto C1x = params.C1x;
    const auto gR3xC1x = 2.0f * sample_rate * C1x + (1.0f / R3x);
    const auto gzR3xC1x = 4.0f * sample_rate * C1x;
    
    const auto vcrit_D1N914_vt = D1N914_vt * std::log(D1N914_vt / (std::sqrt(2.0) * D1N914_Is));
    
    const auto vo1 = vo1_dc;

    float vD1x = 0;

    // --- Newton-Raphson solve: D1x
    const auto _D1x_t3 = (1.0f / 1000000000.0f);
    const auto _D1x_t7 = (1.0f / ((1.0f / R3x) + _D1x_t3));
    const auto _D1x_t2 = (1.0f / (gR1x + _D1x_t3));
    const auto _D1x_t11 = (gR1x * vo1);
    for (int newton_iter = 0; newton_iter < 10000; ++newton_iter)
    {
        const auto _D1x_t1 = (vD1x / D1N914_vt);
        const auto _D1x_t6 = math_exp_approx(_D1x_t1);
        const auto _D1x_t10 = (D1N914_Is * _D1x_t6);
        const auto _D1x_t5 = (_D1x_t6 - 1.0f);
        const auto _D1x_t9 = (_D1x_t10 / D1N914_vt);
        const auto _D1x_t4 = (D1N914_Is * _D1x_t5);
        const auto _D1x_t0 = (((_D1x_t4 - _D1x_t11) * _D1x_t2) + (_D1x_t4 * _D1x_t7));
        const auto _D1x_t8 = (_D1x_t0 + vD1x);
        const auto res_vD1x = (-_D1x_t8);
        const auto delta_vD1x = (-(_D1x_t8 / (((_D1x_t9 * _D1x_t2) + (_D1x_t9 * _D1x_t7)) + 1.0f)));
    
        auto residual_norm_sq = 0.0;
        residual_norm_sq += res_vD1x * res_vD1x;
        auto step_norm_sq = 0.0;
        step_norm_sq += delta_vD1x * delta_vD1x;
    
        vD1x = limit_junction_voltage(vD1x + (delta_vD1x), vD1x, D1N914_vt, vcrit_D1N914_vt);
    
        if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
            break;
        
    }
    const auto zR3xC1x = (((gzR3xC1x * (D1N914_Is * (math_exp_approx((vD1x / D1N914_vt)) - 1.0f))) / ((1.0f / R3x) + (1.0f / 1000000000.0f))) / 2.0f);

    const auto vo2_dc_out = ((D1N914_Is * (math_exp_approx((vD1x / D1N914_vt)) - 1.0f)) / ((1.0f / R3x) + (1.0f / 1000000000.0f)));

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].vD1x = vD1x;
        state[ch].zR3xC1x = zR3xC1x;
    }
    return vo2_dc_out;
}

struct Params {
    Params_stage1 stage1;
    Params_stage2 stage2;
};

struct State {
    State_stage1 stage1;
    State_stage2 stage2;
};

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    for (int ch = 0; ch < num_channels; ++ch)
    {
        compute_stage1(&input[ch], &output[ch], 1, num_samples, params.stage1, &state[ch].stage1, sample_rate);
        compute_stage2(&output[ch], &output[ch], 1, num_samples, params.stage2, &state[ch].stage2, sample_rate);
    }
}

static void reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc = 0.0f)
{
    for (int ch = 0; ch < num_channels; ++ch)
    {
        const float stage1_dc = reset_stage1(params.stage1, &state[ch].stage1, 1, sample_rate, vi_dc);
        const float stage2_dc = reset_stage2(params.stage2, &state[ch].stage2, 1, sample_rate, stage1_dc);
    }
}
