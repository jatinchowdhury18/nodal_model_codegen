// Auto-generated with netlist_codegen version 50a08af.
// Command: netlist_codegen opamp_clipping.net opamp_clipping.h

#pragma once

#include <cmath>

struct Params {
    float R1 = 1.0e+03f;
    float RF = 1.0e+04f;
    float Eop_Aol = 1.0e+05f;
    float Eop_VsatP = 4.5e+00f;
    float Eop_VsatN = -4.5e+00f;
};

struct State {
    float vclip_Eop {};
};

[[maybe_unused]] static auto clamp_opamp_output = [](auto v_new, auto v_prev, auto slew_max_step, auto vsat_n, auto vsat_p)
{
    if (v_new - v_prev > slew_max_step)  v_new = v_prev + slew_max_step;
    if (v_new - v_prev < -slew_max_step) v_new = v_prev - slew_max_step;
    if (v_new > vsat_p) return vsat_p;
    if (v_new < vsat_n) return vsat_n;
    return v_new;
};

static constexpr auto newton_tol_sq = 1.0e-05;
static constexpr int newton_max_iter = 20;

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto Eop_Aol = params.Eop_Aol;
    const auto Eop_VsatP = params.Eop_VsatP;
    const auto Eop_VsatN = params.Eop_VsatN;
    
    const auto gR1 = 1.0f / params.R1;
    
    const auto gRF = 1.0f / params.RF;
    
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto vclip_Eop = state[ch].vclip_Eop;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            // --- Newton-Raphson solve: Eop
            const auto _Eop_t2 = (gR1 + gRF);
            const auto _Eop_t3 = (1.0 / (((gRF * Eop_Aol) / _Eop_t2) + 1.0));
            const auto _Eop_t4 = ((gR1 * vi) * Eop_Aol);
            const auto _Eop_t5 = (1.0 / _Eop_t2);
            for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
            {
                const auto _Eop_t1 = (((gRF * (vclip_Eop * Eop_Aol)) + _Eop_t4) * _Eop_t5);
                const auto _Eop_t0 = (_Eop_t1 + vclip_Eop);
                const auto res_vclip_Eop = (-_Eop_t0);
                const auto delta_vclip_Eop = (-(_Eop_t0 * _Eop_t3));
            
                auto residual_norm_sq = 0.0;
                residual_norm_sq += res_vclip_Eop * res_vclip_Eop;
                auto step_norm_sq = 0.0;
                step_norm_sq += delta_vclip_Eop * delta_vclip_Eop;
            
                const auto _v = (float)(vclip_Eop + delta_vclip_Eop);
                vclip_Eop = clamp_opamp_output(_v, _v, 1.0e30, Eop_VsatN, Eop_VsatP);
            
                if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
                    break;
                
            }

            const auto vo = vclip_Eop;
            

            output[ch][n] = vo;
        }
        state[ch].vclip_Eop = vclip_Eop;
    }
}

static void reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc = 0.0f)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto Eop_Aol = params.Eop_Aol;
    const auto Eop_VsatP = params.Eop_VsatP;
    const auto Eop_VsatN = params.Eop_VsatN;
    
    const auto gR1 = 1.0f / params.R1;
    
    const auto gRF = 1.0f / params.RF;
    
    const auto vi = vi_dc;

    float vclip_Eop = 0;

    // --- Newton-Raphson solve: Eop
    const auto _Eop_t2 = ((gR1 + gRF) + (1.0 / 1000000000.0));
    const auto _Eop_t3 = (1.0 / (((gRF * Eop_Aol) / _Eop_t2) + 1.0));
    const auto _Eop_t4 = ((gR1 * vi) * Eop_Aol);
    const auto _Eop_t5 = (1.0 / _Eop_t2);
    for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
    {
        const auto _Eop_t1 = (((gRF * (vclip_Eop * Eop_Aol)) + _Eop_t4) * _Eop_t5);
        const auto _Eop_t0 = (_Eop_t1 + vclip_Eop);
        const auto res_vclip_Eop = (-_Eop_t0);
        const auto delta_vclip_Eop = (-(_Eop_t0 * _Eop_t3));
    
        auto residual_norm_sq = 0.0;
        residual_norm_sq += res_vclip_Eop * res_vclip_Eop;
        auto step_norm_sq = 0.0;
        step_norm_sq += delta_vclip_Eop * delta_vclip_Eop;
    
        const auto _v = (float)(vclip_Eop + delta_vclip_Eop);
        vclip_Eop = clamp_opamp_output(_v, _v, 1.0e30, Eop_VsatN, Eop_VsatP);
    
        if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
            break;
        
    }
    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].vclip_Eop = vclip_Eop;
    }
}
