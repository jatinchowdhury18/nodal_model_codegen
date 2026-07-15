// Auto-generated with netlist_codegen version 1e4a320.
// Command: netlist_codegen diode_circuit.net diode_circuit.h

#pragma once

#include <cmath>

struct Params {
    float R1 = 1.0e+03f;
    float C1 = 1.0e-06f;
    float D1N914_Is = 2.52e-09f;
    float D1N914_vt = 4.52892e-02f;
};

struct State {
    float zC1 {};
    float vD1 {};
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
    return v_new;
};

static constexpr auto newton_tol_sq = 1.0e-05;
static constexpr int newton_max_iter = 20;

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto D1N914_Is = params.D1N914_Is;
    const auto D1N914_vt = params.D1N914_vt;
    
    const auto gR1 = 1.0f / params.R1;
    
    const auto gC1 = 2.0f * sample_rate * params.C1;
    
    const auto vcrit_D1N914_vt = D1N914_vt * std::log(D1N914_vt / (std::sqrt(2.0) * D1N914_Is));
    
    const auto _t0 = (1 / (gR1 + gC1));
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zC1 = state[ch].zC1;
        auto vD1 = state[ch].vD1;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            // --- Newton-Raphson solve: D1
            const auto _D1_t4 = (1 / (gR1 + gC1));
            const auto _D1_t5 = ((gR1 * vi) + zC1);
            for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
            {
                const auto _D1_t3 = (vD1 / D1N914_vt);
                const auto _D1_t2 = exp(_D1_t3);
                const auto _D1_t1 = (((D1N914_Is * (_D1_t2 - 1)) - _D1_t5) * _D1_t4);
                const auto _D1_t0 = (_D1_t1 + vD1);
                const auto res_vD1 = (-_D1_t0);
                const auto delta_vD1 = (-(_D1_t0 / ((((D1N914_Is * _D1_t2) / D1N914_vt) * _D1_t4) + 1)));
            
                auto residual_norm_sq = 0;
                residual_norm_sq += res_vD1 * res_vD1;
                auto step_norm_sq = 0;
                step_norm_sq += delta_vD1 * delta_vD1;
            
                vD1 = limit_junction_voltage(vD1 + delta_vD1, vD1, D1N914_vt, vcrit_D1N914_vt);
            
                if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
                    break;
                
            }

            const auto vo = (-(((D1N914_Is * (exp((vD1 / D1N914_vt)) - 1)) - ((gR1 * vi) + zC1)) * _t0));
            const auto tC1 = (gC1 * (vo - 0));
            
            zC1 = 2 * tC1 - zC1;

            output[ch][n] = vo;
        }
        state[ch].zC1 = zC1;
        state[ch].vD1 = vD1;
    }
}

static void reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc = 0.0f)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto D1N914_Is = params.D1N914_Is;
    const auto D1N914_vt = params.D1N914_vt;
    
    const auto gR1 = 1.0f / params.R1;
    
    const auto gC1 = 2.0f * sample_rate * params.C1;
    
    const auto vcrit_D1N914_vt = D1N914_vt * std::log(D1N914_vt / (std::sqrt(2.0) * D1N914_Is));
    
    const auto vi = vi_dc;

    float vD1 = 0;

    // --- Newton-Raphson solve: D1
    const auto _D1_t4 = (gR1 * vi);
    for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
    {
        const auto _D1_t3 = (vD1 / D1N914_vt);
        const auto _D1_t2 = exp(_D1_t3);
        const auto _D1_t1 = (((D1N914_Is * (_D1_t2 - 1)) - _D1_t4) / gR1);
        const auto _D1_t0 = (_D1_t1 + vD1);
        const auto res_vD1 = (-_D1_t0);
        const auto delta_vD1 = (-(_D1_t0 / ((((D1N914_Is * _D1_t2) / D1N914_vt) / gR1) + 1)));
    
        auto residual_norm_sq = 0;
        residual_norm_sq += res_vD1 * res_vD1;
        auto step_norm_sq = 0;
        step_norm_sq += delta_vD1 * delta_vD1;
    
        vD1 = limit_junction_voltage(vD1 + delta_vD1, vD1, D1N914_vt, vcrit_D1N914_vt);
    
        if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
            break;
        
    }
    const auto zC1 = (-((gC1 * ((D1N914_Is * (exp((vD1 / D1N914_vt)) - 1)) - (gR1 * vi))) / gR1));

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].vD1 = vD1;
        state[ch].zC1 = zC1;
    }
}
