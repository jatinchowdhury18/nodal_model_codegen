// Auto-generated with netlist_codegen version 58e1e0e.
// Command: netlist_codegen common_drain.net common_drain.h -type_name double

#pragma once

#include <cmath>
#include <cstdint>

[[maybe_unused]] static auto limit_jfet_vgs = [](auto v_new, auto vp)
{
    if (v_new < vp) return vp;
    return v_new;
};

static constexpr auto newton_tol_sq = 0.00001;
static constexpr int newton_max_iter = 20;


struct Params {
    double VCC = 9.0e+00;
    double C12 = 2.2e-08;
    double R11 = 1.0e+06;
    double R13 = 1.0e+04;
    double C13 = 1.0e-05;
    double RL = 1.0e+05;
    double _2N5485_Beta = 2.0e-03;
    double _2N5485_vp = -1.0e+00;
};

struct State {
    double zC12 {};
    double zC13 {};
    double vGSJ1 {};
    double vGSJ1_prev {};
};

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto _2N5485_Beta = params._2N5485_Beta;
    const auto _2N5485_vp = params._2N5485_vp;
    
    const auto VCC = params.VCC;
    
    const auto vd = VCC;
    
    const auto gC12 = 2.0 * sample_rate * params.C12;
    
    const auto gR11 = 1.0 / params.R11;
    
    const auto gR13 = 1.0 / params.R13;
    
    const auto gC13 = 2.0 * sample_rate * params.C13;
    
    const auto gRL = 1.0 / params.RL;
    
    const auto _t2 = (gR13 + gC13);
    const auto _t4 = (gC13 + gRL);
    const auto _t3 = (1.0 / ((_t2 * _t4) - (gC13 * gC13)));
    const auto _t5 = (1.0 / (gC12 + gR11));
    const auto _t6 = (1.0 / 1000.0);
    double c0_tC12;
    double c_tC12[3];
    double c0_vg;
    double c_vg[3];
    
    for (int _k = 0; _k <= 3; ++_k)
    {
        const auto vi = (_k == 1) ? 1.0 : 0.0;
        const auto zC12 = (_k == 2) ? 1.0 : 0.0;
        const auto zC13 = (_k == 3) ? 1.0 : 0.0;
        const auto vg = (-((zC12 - (gC12 * vi)) * _t5));
        const auto tC12 = (gC12 * (vi - vg));
        if (_k == 0) {
            c0_tC12 = tC12;
            c0_vg = vg;
        } else {
            c_tC12[_k - 1] = tC12 - c0_tC12;
            c_vg[_k - 1] = vg - c0_vg;
        }
    }
    
    const auto _J1_zt1 = (gC13 + gRL);
    const auto _J1_zt0 = (((gR13 + gC13) * _J1_zt1) - (gC13 * gC13));
    const auto _J1_Z0_0 = (-((_J1_zt1 * 1.0) / _J1_zt0));
    double c0__J1_voc0;
    double c__J1_voc0[3];
    
    for (int _k = 0; _k <= 3; ++_k)
    {
        const auto vi = (_k == 1) ? 1.0 : 0.0;
        const auto zC12 = (_k == 2) ? 1.0 : 0.0;
        const auto zC13 = (_k == 3) ? 1.0 : 0.0;
        const auto _J1_voc0 = (-(((zC12 - (gC12 * vi)) / (gC12 + gR11)) + (((_J1_zt1 * zC13) - (gC13 * zC13)) / _J1_zt0)));
        if (_k == 0) {
            c0__J1_voc0 = _J1_voc0;
        } else {
            c__J1_voc0[_k - 1] = _J1_voc0 - c0__J1_voc0;
        }
    }
    
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zC12 = state[ch].zC12;
        auto zC13 = state[ch].zC13;
        auto vGSJ1 = state[ch].vGSJ1;
        auto vGSJ1_prev = state[ch].vGSJ1_prev;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            { const auto _prev_step = vGSJ1 - vGSJ1_prev; vGSJ1_prev = vGSJ1;
vGSJ1 = limit_jfet_vgs(vGSJ1 + (_prev_step), _2N5485_vp);
            }

            // --- Newton-Raphson solve (N-port): J1
            const auto _J1_voc0 = c0__J1_voc0 + c__J1_voc0[0] * vi + c__J1_voc0[1] * zC12 + c__J1_voc0[2] * zC13;
            const auto _J1_pt2 = (1.0 / 1000.0);
            for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
            {
                const auto _J1_pt1 = (vGSJ1 - _2N5485_vp);
                const auto _J1_pt0 = (_J1_pt1 + _J1_pt2);
                const auto _J1_i0 = (_2N5485_Beta * (_J1_pt0 * _J1_pt0));
                const auto _J1_g0_0 = (_2N5485_Beta * (_J1_pt0 + _J1_pt0));
                const auto _J1_pt3 = (_J1_voc0 + (_J1_Z0_0 * _J1_i0));
                const auto res_vGSJ1 = (_J1_pt3 - vGSJ1);
                const auto delta_vGSJ1 = ((vGSJ1 - _J1_pt3) / ((_J1_Z0_0 * _J1_g0_0) - 1.0));
            
                auto residual_norm_sq = 0.0;
                residual_norm_sq += res_vGSJ1 * res_vGSJ1;
            
                vGSJ1 = limit_jfet_vgs(vGSJ1 + (delta_vGSJ1), _2N5485_vp);
            
                if (residual_norm_sq < newton_tol_sq)
                    break;
                
            }

            const auto tC12 = c0_tC12 + c_tC12[0] * vi + c_tC12[1] * zC12 + c_tC12[2] * zC13;
            const auto vg = c0_vg + c_vg[0] * vi + c_vg[1] * zC12 + c_vg[2] * zC13;
            const auto _t1 = ((vGSJ1 - _2N5485_vp) + _t6);
            const auto _t0 = ((_2N5485_Beta * (_t1 * _t1)) + zC13);
            const auto vo = (((_t0 * gC13) - (_t2 * zC13)) * _t3);
            const auto vs = (((_t0 * _t4) - (gC13 * zC13)) * _t3);
            const auto tC13 = (gC13 * (vs - vo));
            
            zC12 = 2 * tC12 - zC12;
            zC13 = 2 * tC13 - zC13;

            output[ch][n] = vo;
        }
        state[ch].zC12 = zC12;
        state[ch].zC13 = zC13;
        state[ch].vGSJ1 = vGSJ1;
        state[ch].vGSJ1_prev = vGSJ1_prev;
    }
}

static float reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc = 0.0f)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto _2N5485_Beta = params._2N5485_Beta;
    const auto _2N5485_vp = params._2N5485_vp;
    
    const auto VCC = params.VCC;
    
    const auto vd = VCC;
    
    const auto gC12 = 2.0 * sample_rate * params.C12;
    
    const auto gR11 = 1.0 / params.R11;
    
    const auto gR13 = 1.0 / params.R13;
    
    const auto gC13 = 2.0 * sample_rate * params.C13;
    
    const auto gRL = 1.0 / params.RL;
    
    const auto vi = vi_dc;

    double vGSJ1 = 0;

    const auto _J1_Z0_0 = (-(1.0 / (gR13 + (1.0 / 1000000000.0))));
    // --- Newton-Raphson solve (N-port): J1
    const auto _J1_pt1 = (1.0 / 1000.0);
    for (int newton_iter = 0; newton_iter < 10000; ++newton_iter)
    {
        const auto _J1_pt0 = (vGSJ1 - _2N5485_vp);
        const auto _J1_pt2 = (_J1_pt0 + _J1_pt1);
        const auto _J1_i0 = (_2N5485_Beta * (_J1_pt2 * _J1_pt2));
        const auto _J1_g0_0 = (_2N5485_Beta * (_J1_pt2 + _J1_pt2));
        const auto _J1_pt3 = (_J1_Z0_0 * _J1_i0);
        const auto res_vGSJ1 = (_J1_pt3 - vGSJ1);
        const auto delta_vGSJ1 = ((vGSJ1 - _J1_pt3) / ((_J1_Z0_0 * _J1_g0_0) - 1.0));
    
        auto residual_norm_sq = 0.0;
        residual_norm_sq += res_vGSJ1 * res_vGSJ1;
    
        vGSJ1 = limit_jfet_vgs(vGSJ1 + (delta_vGSJ1), _2N5485_vp);
    
        if (residual_norm_sq < newton_tol_sq)
            break;
        
    }
    const auto zC12 = (gC12 * vi);
    const auto zC13 = ((gC13 * ((_2N5485_Beta * ((vGSJ1 - _2N5485_vp) + (1.0 / 1000.0))) * ((vGSJ1 - _2N5485_vp) + (1.0 / 1000.0)))) / (gR13 + (1.0 / 1000000000.0)));

    const auto vo_dc_out = 0.0;

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].vGSJ1 = vGSJ1;
        state[ch].vGSJ1_prev = vGSJ1;
        state[ch].zC12 = zC12;
        state[ch].zC13 = zC13;
    }
    return vo_dc_out;
}

