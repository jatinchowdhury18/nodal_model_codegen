// Auto-generated with netlist_codegen version ac50416.
// Command: netlist_codegen opamp_clipping.net opamp_clipping.h -opt_port_matrix

#pragma once

#include <cmath>
#include <cstdint>

[[maybe_unused]] static auto clamp_opamp_output = [](auto v_new, auto v_prev, auto slew_max_step, auto vsat_n, auto vsat_p)
{
    if (v_new - v_prev > slew_max_step)  v_new = v_prev + slew_max_step;
    if (v_new - v_prev < -slew_max_step) v_new = v_prev - slew_max_step;
    if (v_new > vsat_p) return vsat_p;
    if (v_new < vsat_n) return vsat_n;
    return v_new;
};

static constexpr auto newton_tol_sq = 0.00001;
static constexpr int newton_max_iter = 20;


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

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto Eop_Aol = params.Eop_Aol;
    const auto Eop_VsatP = params.Eop_VsatP;
    const auto Eop_VsatN = params.Eop_VsatN;
    
    const auto gR1 = 1.0f / params.R1;
    
    const auto gRF = 1.0f / params.RF;
    
    float c0__Eop_t4;
    float c__Eop_t4[1];
    
    for (int _k = 0; _k <= 1; ++_k)
    {
        const auto vi = (_k == 1) ? 1.0f : 0.0f;
        const auto _Eop_t4 = ((gR1 * vi) * Eop_Aol);
        if (_k == 0) {
            c0__Eop_t4 = _Eop_t4;
        } else {
            c__Eop_t4[_k - 1] = _Eop_t4 - c0__Eop_t4;
        }
    }
    
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto vclip_Eop = state[ch].vclip_Eop;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            // --- Newton-Raphson solve: Eop
            const auto _Eop_t4 = c0__Eop_t4 + c__Eop_t4[0] * vi;
            const auto _Eop_t2 = (gR1 + gRF);
            const auto _Eop_t3 = (1.0f / (((gRF * Eop_Aol) / _Eop_t2) + 1.0f));
            const auto _Eop_t5 = (1.0f / _Eop_t2);
            for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
            {
                const auto _Eop_t1 = ((((gRF * vclip_Eop) * Eop_Aol) + _Eop_t4) * _Eop_t5);
                const auto _Eop_t0 = (_Eop_t1 + vclip_Eop);
                const auto res_vclip_Eop = (-_Eop_t0);
                const auto delta_vclip_Eop = (-(_Eop_t0 * _Eop_t3));
            
                const float _natural = (float)(vclip_Eop + delta_vclip_Eop);
                auto _v = _natural;
                if (_v - _natural > 1.0e30)  _v = _natural + 1.0e30;
                if (_v - _natural < -1.0e30) _v = _natural - 1.0e30;
                auto res_vclip_Eop_active = 0.0, delta_vclip_Eop_active = 0.0;
                if (_v > Eop_VsatP) {
                    delta_vclip_Eop_active = Eop_VsatP - vclip_Eop;
                    vclip_Eop = Eop_VsatP;
                } else if (_v < Eop_VsatN) {
                    delta_vclip_Eop_active = Eop_VsatN - vclip_Eop;
                    vclip_Eop = Eop_VsatN;
                } else {
                    res_vclip_Eop_active = res_vclip_Eop; delta_vclip_Eop_active = _v - vclip_Eop;
                    vclip_Eop = _v;
                }
            
                auto residual_norm_sq = res_vclip_Eop_active * res_vclip_Eop_active;
                auto step_norm_sq = delta_vclip_Eop_active * delta_vclip_Eop_active;
            
            
                if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
                    break;
                
            }

            const auto vo = vclip_Eop;
            

            output[ch][n] = vo;
        }
        state[ch].vclip_Eop = vclip_Eop;
    }
}

static float reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc = 0.0f)
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
    const auto _Eop_t1 = ((gR1 + gRF) + (1.0f / 1000000000.0f));
    const auto _Eop_t3 = (1.0f / (((gRF * Eop_Aol) / _Eop_t1) + 1.0f));
    const auto _Eop_t4 = ((gR1 * vi) * Eop_Aol);
    const auto _Eop_t5 = (1.0f / _Eop_t1);
    for (int newton_iter = 0; newton_iter < 10000; ++newton_iter)
    {
        const auto _Eop_t0 = (((gRF * (vclip_Eop * Eop_Aol)) + _Eop_t4) * _Eop_t5);
        const auto _Eop_t2 = (_Eop_t0 + vclip_Eop);
        const auto res_vclip_Eop = (-_Eop_t2);
        const auto delta_vclip_Eop = (-(_Eop_t2 * _Eop_t3));
    
        const float _natural = (float)(vclip_Eop + delta_vclip_Eop);
        auto _v = _natural;
        if (_v - _natural > 1.0e30)  _v = _natural + 1.0e30;
        if (_v - _natural < -1.0e30) _v = _natural - 1.0e30;
        auto res_vclip_Eop_active = 0.0, delta_vclip_Eop_active = 0.0;
        if (_v > Eop_VsatP) {
            delta_vclip_Eop_active = Eop_VsatP - vclip_Eop;
            vclip_Eop = Eop_VsatP;
        } else if (_v < Eop_VsatN) {
            delta_vclip_Eop_active = Eop_VsatN - vclip_Eop;
            vclip_Eop = Eop_VsatN;
        } else {
            res_vclip_Eop_active = res_vclip_Eop; delta_vclip_Eop_active = _v - vclip_Eop;
            vclip_Eop = _v;
        }
    
        auto residual_norm_sq = res_vclip_Eop_active * res_vclip_Eop_active;
        auto step_norm_sq = delta_vclip_Eop_active * delta_vclip_Eop_active;
    
    
        if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
            break;
        
    }
    const auto vo_dc_out = vclip_Eop;

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].vclip_Eop = vclip_Eop;
    }
    return vo_dc_out;
}

