// Auto-generated with netlist_codegen version ea0eac5.
// Command: netlist_codegen opamp_complete.net opamp_complete.h

#pragma once

#include <cmath>

struct Params {
    float R1 = 1.0e+03f;
    float RF = 1.0e+05f;
    float Eraw_Aol = 2.0e+05f;
    float Eraw_VsatP = 1.35e+01f;
    float Eraw_VsatN = -1.35e+01f;
    float Eraw_Vos = 2.0e-03f;
    float Eraw_Ios = 2.0e-08f;
    float Eraw_Ibias = 8.0e-08f;
    float Eraw_Rin = 2.0e+06f;
    float Eraw_Rcomp = 1.0e+00f;
    float Eraw_Ccomp = 3.183099e-02f;
    float Eraw_Rout = 7.5e+01f;
};

struct State {
    float zEraw_Ccomp {};
    float vclip_Eraw {};
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
    
    const auto Eraw_Aol = params.Eraw_Aol;
    const auto Eraw_VsatP = params.Eraw_VsatP;
    const auto Eraw_VsatN = params.Eraw_VsatN;
    const auto Eraw_Vos = params.Eraw_Vos;
    const auto Eraw_Ios = params.Eraw_Ios;
    const auto Eraw_Ibias = params.Eraw_Ibias;
    
    const auto gR1 = 1.0f / params.R1;
    
    const auto gRF = 1.0f / params.RF;
    
    const auto gEraw_Rin = 1.0f / params.Eraw_Rin;
    
    const auto gEraw_Rcomp = 1.0f / params.Eraw_Rcomp;
    
    const auto gEraw_Ccomp = 2.0f * sample_rate * params.Eraw_Ccomp;
    
    const auto gEraw_Rout = 1.0f / params.Eraw_Rout;
    
    const auto _t2 = (gEraw_Rcomp + gEraw_Ccomp);
    const auto _t3 = ((gR1 + gRF) + gEraw_Rin);
    const auto _t6 = (gRF + gEraw_Rout);
    const auto _t8 = (Eraw_Aol * gEraw_Rcomp);
    const auto _t1 = (gRF * _t2);
    const auto _t5 = (1.0 / ((gRF * _t1) - (_t3 * (_t6 * _t2))));
    const auto _t9 = (_t6 * _t8);
    const auto _t10 = (Eraw_Ibias - (Eraw_Ios / 2.0));
    const auto _t11 = ((Eraw_Aol * Eraw_Vos) * gEraw_Rcomp);
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zEraw_Ccomp = state[ch].zEraw_Ccomp;
        auto vclip_Eraw = state[ch].vclip_Eraw;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            // --- Newton-Raphson solve: Eraw
            const auto _Eraw_t3 = (Eraw_Aol * Eraw_Vos);
            const auto _Eraw_t4 = (Eraw_Aol * gEraw_Rcomp);
            const auto _Eraw_t6 = (gR1 + gRF);
            const auto _Eraw_t7 = (gRF + gEraw_Rout);
            const auto _Eraw_t9 = (gEraw_Rcomp + gEraw_Ccomp);
            const auto _Eraw_t2 = (_Eraw_t3 * gEraw_Rcomp);
            const auto _Eraw_t5 = (_Eraw_t6 + gEraw_Rin);
            const auto _Eraw_t8 = ((gRF * (gRF * _Eraw_t9)) - (_Eraw_t5 * (_Eraw_t7 * _Eraw_t9)));
            const auto _Eraw_t1 = (zEraw_Ccomp + _Eraw_t2);
            const auto _Eraw_t10 = (1.0 / (((((gRF * gEraw_Rout) * Eraw_Aol) * gEraw_Rcomp) / _Eraw_t8) - 1.0));
            const auto _Eraw_t11 = (gRF * _Eraw_t1);
            const auto _Eraw_t12 = (_Eraw_t5 * (_Eraw_t7 * _Eraw_t1));
            const auto _Eraw_t13 = (((Eraw_Ibias - (Eraw_Ios / 2.0)) - (gR1 * vi)) * (_Eraw_t7 * _Eraw_t4));
            const auto _Eraw_t14 = (1.0 / _Eraw_t8);
            for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
            {
                const auto _Eraw_t0 = ((((gRF * (_Eraw_t11 + ((gEraw_Rout * vclip_Eraw) * _Eraw_t4))) - _Eraw_t12) - _Eraw_t13) * _Eraw_t14);
                const auto res_vclip_Eraw = (_Eraw_t0 - vclip_Eraw);
                const auto delta_vclip_Eraw = ((vclip_Eraw - _Eraw_t0) * _Eraw_t10);
            
                const float _natural = (float)(vclip_Eraw + delta_vclip_Eraw);
                auto _v = _natural;
                if (_v - _natural > 1.0e30)  _v = _natural + 1.0e30;
                if (_v - _natural < -1.0e30) _v = _natural - 1.0e30;
                auto res_vclip_Eraw_active = 0.0, delta_vclip_Eraw_active = 0.0;
                if (_v > Eraw_VsatP) {
                    delta_vclip_Eraw_active = Eraw_VsatP - vclip_Eraw;
                    vclip_Eraw = Eraw_VsatP;
                } else if (_v < Eraw_VsatN) {
                    delta_vclip_Eraw_active = Eraw_VsatN - vclip_Eraw;
                    vclip_Eraw = Eraw_VsatN;
                } else {
                    res_vclip_Eraw_active = res_vclip_Eraw; delta_vclip_Eraw_active = _v - vclip_Eraw;
                    vclip_Eraw = _v;
                }
            
                auto residual_norm_sq = res_vclip_Eraw_active * res_vclip_Eraw_active;
                auto step_norm_sq = delta_vclip_Eraw_active * delta_vclip_Eraw_active;
            
                
                if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
                    break;
                
            }

            const auto _t0 = (_t10 - (gR1 * vi));
            const auto _t4 = (gEraw_Rout * vclip_Eraw);
            const auto _t7 = (zEraw_Ccomp + _t11);
            const auto vo = (((_t0 * _t1) - (_t3 * (_t4 * _t2))) * _t5);
            const auto vbw_Eraw = ((((gRF * ((gRF * _t7) + (_t4 * _t8))) - (_t3 * (_t6 * _t7))) - (_t0 * _t9)) * _t5);
            const auto tEraw_Ccomp = (gEraw_Ccomp * (vbw_Eraw - 0));
            
            zEraw_Ccomp = 2 * tEraw_Ccomp - zEraw_Ccomp;

            output[ch][n] = vo;
        }
        state[ch].zEraw_Ccomp = zEraw_Ccomp;
        state[ch].vclip_Eraw = vclip_Eraw;
    }
}

static void reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc = 0.0f)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto Eraw_Aol = params.Eraw_Aol;
    const auto Eraw_VsatP = params.Eraw_VsatP;
    const auto Eraw_VsatN = params.Eraw_VsatN;
    const auto Eraw_Vos = params.Eraw_Vos;
    const auto Eraw_Ios = params.Eraw_Ios;
    const auto Eraw_Ibias = params.Eraw_Ibias;
    
    const auto gR1 = 1.0f / params.R1;
    
    const auto gRF = 1.0f / params.RF;
    
    const auto gEraw_Rin = 1.0f / params.Eraw_Rin;
    
    const auto gEraw_Rcomp = 1.0f / params.Eraw_Rcomp;
    
    const auto gEraw_Ccomp = 2.0f * sample_rate * params.Eraw_Ccomp;
    
    const auto gEraw_Rout = 1.0f / params.Eraw_Rout;
    
    const auto vi = vi_dc;

    float vclip_Eraw = 0;

    // --- Newton-Raphson solve: Eraw
    const auto _Eraw_t2 = (Eraw_Aol * Eraw_Vos);
    const auto _Eraw_t3 = (Eraw_Aol * gEraw_Rcomp);
    const auto _Eraw_t6 = (gR1 + gRF);
    const auto _Eraw_t7 = (1.0 / 1000000000.0);
    const auto _Eraw_t9 = (gRF + gEraw_Rout);
    const auto _Eraw_t11 = (gEraw_Rcomp + _Eraw_t7);
    const auto _Eraw_t1 = (_Eraw_t2 * gEraw_Rcomp);
    const auto _Eraw_t5 = (_Eraw_t6 + gEraw_Rin);
    const auto _Eraw_t8 = (_Eraw_t9 + _Eraw_t7);
    const auto _Eraw_t4 = (_Eraw_t5 + _Eraw_t7);
    const auto _Eraw_t10 = ((gRF * (gRF * _Eraw_t11)) - (_Eraw_t4 * (_Eraw_t8 * _Eraw_t11)));
    const auto _Eraw_t12 = (1.0 / (((((gRF * gEraw_Rout) * Eraw_Aol) * gEraw_Rcomp) / _Eraw_t10) - 1.0));
    const auto _Eraw_t13 = (gRF * _Eraw_t1);
    const auto _Eraw_t14 = (_Eraw_t4 * (_Eraw_t8 * _Eraw_t1));
    const auto _Eraw_t15 = (((Eraw_Ibias - (Eraw_Ios / 2.0)) - (gR1 * vi)) * (_Eraw_t8 * _Eraw_t3));
    const auto _Eraw_t16 = (1.0 / _Eraw_t10);
    for (int newton_iter = 0; newton_iter < 10000; ++newton_iter)
    {
        const auto _Eraw_t0 = ((((gRF * (_Eraw_t13 + ((gEraw_Rout * vclip_Eraw) * _Eraw_t3))) - _Eraw_t14) - _Eraw_t15) * _Eraw_t16);
        const auto res_vclip_Eraw = (_Eraw_t0 - vclip_Eraw);
        const auto delta_vclip_Eraw = ((vclip_Eraw - _Eraw_t0) * _Eraw_t12);
    
        const float _natural = (float)(vclip_Eraw + delta_vclip_Eraw);
        auto _v = _natural;
        if (_v - _natural > 1.0e30)  _v = _natural + 1.0e30;
        if (_v - _natural < -1.0e30) _v = _natural - 1.0e30;
        auto res_vclip_Eraw_active = 0.0, delta_vclip_Eraw_active = 0.0;
        if (_v > Eraw_VsatP) {
            delta_vclip_Eraw_active = Eraw_VsatP - vclip_Eraw;
            vclip_Eraw = Eraw_VsatP;
        } else if (_v < Eraw_VsatN) {
            delta_vclip_Eraw_active = Eraw_VsatN - vclip_Eraw;
            vclip_Eraw = Eraw_VsatN;
        } else {
            res_vclip_Eraw_active = res_vclip_Eraw; delta_vclip_Eraw_active = _v - vclip_Eraw;
            vclip_Eraw = _v;
        }
    
        auto residual_norm_sq = res_vclip_Eraw_active * res_vclip_Eraw_active;
        auto step_norm_sq = delta_vclip_Eraw_active * delta_vclip_Eraw_active;
    
        
        if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
            break;
        
    }
    const auto zEraw_Ccomp = ((gEraw_Ccomp * (((gRF * ((gRF * ((Eraw_Aol * Eraw_Vos) * gEraw_Rcomp)) + ((gEraw_Rout * vclip_Eraw) * (Eraw_Aol * gEraw_Rcomp)))) - ((((gR1 + gRF) + gEraw_Rin) + (1.0 / 1000000000.0)) * (((gRF + gEraw_Rout) + (1.0 / 1000000000.0)) * ((Eraw_Aol * Eraw_Vos) * gEraw_Rcomp)))) - (((Eraw_Ibias - (Eraw_Ios / 2.0)) - (gR1 * vi)) * (((gRF + gEraw_Rout) + (1.0 / 1000000000.0)) * (Eraw_Aol * gEraw_Rcomp))))) / ((gRF * (gRF * (gEraw_Rcomp + (1.0 / 1000000000.0)))) - ((((gR1 + gRF) + gEraw_Rin) + (1.0 / 1000000000.0)) * (((gRF + gEraw_Rout) + (1.0 / 1000000000.0)) * (gEraw_Rcomp + (1.0 / 1000000000.0))))));

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].vclip_Eraw = vclip_Eraw;
        state[ch].zEraw_Ccomp = zEraw_Ccomp;
    }
}
