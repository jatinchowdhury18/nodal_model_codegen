// Auto-generated with netlist_codegen version 5608cd2.
// Command: netlist_codegen common_cathode.net common_cathode_c.h -opt_port_matrix -lang c -type_name double

#pragma once

#include <math.h>
#include <stdint.h>

static int64_t math_bits_from_float(double x) { union { double f; int64_t i; } u; u.f = x; return u.i; }
static double math_float_from_bits(int64_t i) { union { int64_t i; double f; } u; u.i = i; return u.f; }

static double math_exp_approx(double x) {
    x *= 1.4426950408889634074;
    if (x < -1022.0) x = -1022.0;
    const int64_t xi = (int64_t) x;
    const int64_t l = x < (double) 0 ? xi - 1 : xi;
    const double f = x - (double) l;
    const double f_sq = f * f;
    const int64_t vi = (l + 1023) << 52;
    return math_float_from_bits(vi) * ((((1.0 + 0.69314718056000002 * f) + f_sq * ((0.24022825068600001 + 0.0554875633068 * f))) + (f_sq * f_sq) * (((0.00967475272129 + 0.00124453797252 * f) + f_sq * (0.000217714753229)))));
}

static double math_log_approx(double x) {
    const int64_t vi = math_bits_from_float(x);
    const int64_t ex = vi & 0x7ff0000000000000LL;
    const int64_t e = (ex >> 52) - 1023;
    const int64_t vfi = (vi - ex) | 0x3ff0000000000000LL;
    const double vf = math_float_from_bits(vfi);
    const double vf_sq = vf * vf;
    return 0.69314718055994530942 * ((double) e + ((((-3.06081857306000015 + 6.19242937535999972 * vf) + vf_sq * ((-5.46521465639999971 + 3.38542517474999994 * vf))) + (vf_sq * vf_sq) * (((-1.3100709077499999 + 0.28479443750200001 * vf) + vf_sq * (-0.0265448504094))))));
}

static double math_pow_approx(double x, double y) {
    return math_exp_approx(y * math_log_approx(x));
}


static const double newton_tol_sq = 0.00001;
static const int newton_max_iter = 20;


typedef struct {
    double VCC; // = 2.5e+02
    double Cin; // = 2.0e-08
    double Rg; // = 1.0e+06
    double Cout; // = 2.0e-08
    double RL; // = 1.0e+06
    double Rp; // = 1.0e+05
    double _12AX7_DEMPWOLF_Gk; // = 2.242e-03
    double _12AX7_DEMPWOLF_Mu; // = 1.032e+02
    double _12AX7_DEMPWOLF_Gamma; // = 1.26e+00
    double _12AX7_DEMPWOLF_Ck; // = 3.4e+00
    double _12AX7_DEMPWOLF_Gg; // = 6.177e-04
    double _12AX7_DEMPWOLF_Xi; // = 1.314e+00
    double _12AX7_DEMPWOLF_Cg; // = 9.901e+00
    double _12AX7_DEMPWOLF_Ig0; // = 8.025e-08
    double Rk; // = 1.5e+03
    double Ck; // = 2.5e-05
} Params;

typedef struct {
    double zCin;
    double zCout;
    double zRkCk;
    double vPKX1;
    double vPKX1_prev;
    double vGKX1;
    double vGKX1_prev;
} State;

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    #define sum(a, b) ((a) + (b))
    #define recip_sum(a, b) ((a) * (b) / ((a) + (b)))
    
    const double _12AX7_DEMPWOLF_Gk = params._12AX7_DEMPWOLF_Gk;
    const double _12AX7_DEMPWOLF_Mu = params._12AX7_DEMPWOLF_Mu;
    const double _12AX7_DEMPWOLF_Gamma = params._12AX7_DEMPWOLF_Gamma;
    const double _12AX7_DEMPWOLF_Ck = params._12AX7_DEMPWOLF_Ck;
    const double _12AX7_DEMPWOLF_Gg = params._12AX7_DEMPWOLF_Gg;
    const double _12AX7_DEMPWOLF_Xi = params._12AX7_DEMPWOLF_Xi;
    const double _12AX7_DEMPWOLF_Cg = params._12AX7_DEMPWOLF_Cg;
    const double _12AX7_DEMPWOLF_Ig0 = params._12AX7_DEMPWOLF_Ig0;
    
    const double VCC = params.VCC;
    
    const double vp = VCC;
    
    const double gCin = 2.0 * sample_rate * params.Cin;
    
    const double gRg = 1.0 / params.Rg;
    
    const double gCout = 2.0 * sample_rate * params.Cout;
    
    const double gRL = 1.0 / params.RL;
    
    const double gRp = 1.0 / params.Rp;
    
    const double Rk = params.Rk;
    const double Ck = params.Ck;
    const double gRkCk = 2.0 * sample_rate * Ck + (1.0 / Rk);
    const double gzRkCk = 4.0 * sample_rate * Ck;
    
    const double _t0 = (gCout + gRp);
    const double _t4 = (gCout * gRkCk);
    const double _t5 = (VCC * gRkCk);
    const double _t8 = (gCout + gRL);
    const double _t7 = (_t8 * gRkCk);
    const double _t6 = (1.0 / ((gCout * _t4) - (_t0 * _t7)));
    const double _t12 = (gRp * (gCout * _t5));
    const double _t13 = (1.0 / (gCin + gRg));
    const double _t14 = (gRp * (_t8 * _t5));
    const double _X1_zt4 = (gCout * 1.0);
    const double _X1_zt10 = (gCout + gRp);
    const double _X1_zt12 = (gCout * gRkCk);
    const double _X1_zt13 = (gCin + gRg);
    const double _X1_zt14 = (gCout + gRL);
    const double _X1_zt0 = (_X1_zt14 * gRkCk);
    const double _X1_zt1 = (gCout * _X1_zt12);
    const double _X1_zt2 = (_X1_zt10 * _X1_zt0);
    const double _X1_zt3 = (gCout * _X1_zt4);
    const double _X1_zt6 = (_X1_zt14 * 1.0);
    const double _X1_zt11 = (_X1_zt1 - _X1_zt2);
    const double _X1_zt5 = (_X1_zt10 * _X1_zt6);
    const double _X1_zt9 = (_X1_zt3 - _X1_zt5);
    const double _X1_zt7 = (_X1_zt9 / _X1_zt11);
    const double _X1_zt8 = (-_X1_zt7);
    const double _X1_Z0_0 = (((_X1_zt0 * 1.0) / _X1_zt11) - _X1_zt7);
    const double _X1_Z0_1 = _X1_zt8;
    const double _X1_Z1_0 = _X1_zt8;
    const double _X1_Z1_1 = (-((1.0 / _X1_zt13) + _X1_zt7));
    double c0__X1_voc0;
    double c__X1_voc0[4];
    double c0__X1_voc1;
    double c__X1_voc1[4];
    double c0__X1_zt15;
    double c__X1_zt15[4];
    
    for (int _k = 0; _k <= 4; ++_k)
    {
        const double vi = (_k == 1) ? 1.0 : 0.0;
        const double zCin = (_k == 2) ? 1.0 : 0.0;
        const double zCout = (_k == 3) ? 1.0 : 0.0;
        const double zRkCk = (_k == 4) ? 1.0 : 0.0;
        const double _X1_zt16 = (gCout * zRkCk);
        const double _X1_zt18 = (gCout * _X1_zt16);
        const double _X1_zt20 = (_X1_zt14 * zRkCk);
        const double _X1_zt19 = (_X1_zt10 * _X1_zt20);
        const double _X1_zt17 = (_X1_zt18 - _X1_zt19);
        const double _X1_zt15 = (_X1_zt17 / _X1_zt11);
        const double _X1_voc0 = (((((_X1_zt0 * (-zCout)) + (gCout * (zCout * gRkCk))) - (gRp * (_X1_zt14 * (VCC * gRkCk)))) / _X1_zt11) - _X1_zt15);
        const double _X1_voc1 = (-(((-((gCin * vi) - zCin)) / _X1_zt13) + _X1_zt15));
        if (_k == 0) {
            c0__X1_voc0 = _X1_voc0;
            c0__X1_voc1 = _X1_voc1;
            c0__X1_zt15 = _X1_zt15;
        } else {
            c__X1_voc0[_k - 1] = _X1_voc0 - c0__X1_voc0;
            c__X1_voc1[_k - 1] = _X1_voc1 - c0__X1_voc1;
            c__X1_zt15[_k - 1] = _X1_zt15 - c0__X1_zt15;
        }
    }
    
    for (int ch = 0; ch < num_channels; ++ch)
    {
        double zCin = state[ch].zCin;
        double zCout = state[ch].zCout;
        double zRkCk = state[ch].zRkCk;
        double vPKX1 = state[ch].vPKX1;
        double vPKX1_prev = state[ch].vPKX1_prev;
        double vGKX1 = state[ch].vGKX1;
        double vGKX1_prev = state[ch].vGKX1_prev;
        for (int n = 0; n < num_samples; ++n)
        {
            const double vi = input[ch][n];

            { const double _prev_step = vPKX1 - vPKX1_prev; vPKX1_prev = vPKX1;
vPKX1 = vPKX1 + (_prev_step);
            }
            { const double _prev_step = vGKX1 - vGKX1_prev; vGKX1_prev = vGKX1;
vGKX1 = vGKX1 + (_prev_step);
            }

            // --- Newton-Raphson solve (N-port): X1
            const double _X1_voc0 = c0__X1_voc0 + c__X1_voc0[0] * vi + c__X1_voc0[1] * zCin + c__X1_voc0[2] * zCout + c__X1_voc0[3] * zRkCk;
            const double _X1_voc1 = c0__X1_voc1 + c__X1_voc1[0] * vi + c__X1_voc1[1] * zCin + c__X1_voc1[2] * zCout + c__X1_voc1[3] * zRkCk;
            const double _X1_zt15 = c0__X1_zt15 + c__X1_zt15[0] * vi + c__X1_zt15[1] * zCin + c__X1_zt15[2] * zCout + c__X1_zt15[3] * zRkCk;
            const double _X1_zt16 = (gCout * zRkCk);
            const double _X1_zt18 = (gCout * _X1_zt16);
            const double _X1_zt20 = (_X1_zt14 * zRkCk);
            const double _X1_zt19 = (_X1_zt10 * _X1_zt20);
            const double _X1_zt17 = (_X1_zt18 - _X1_zt19);
            const double _X1_pt12 = (_12AX7_DEMPWOLF_Xi - 1.0);
            const double _X1_pt17 = (_12AX7_DEMPWOLF_Gamma - 1.0);
            for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
            {
                const double _X1_pt2 = math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1)));
                const double _X1_pt5 = (_12AX7_DEMPWOLF_Cg * vGKX1);
                const double _X1_pt1 = (1.0 + _X1_pt2);
                const double _X1_pt4 = math_exp_approx(_X1_pt5);
                const double _X1_pt11 = (1.0 + _X1_pt4);
                const double _X1_pt0 = (math_log_approx(_X1_pt1) / _12AX7_DEMPWOLF_Ck);
                const double _X1_pt10 = math_log_approx(_X1_pt11);
                const double _X1_pt16 = math_pow_approx(_X1_pt0, _X1_pt17);
                const double _X1_pt3 = (_X1_pt10 / _12AX7_DEMPWOLF_Cg);
                const double _X1_pt9 = math_pow_approx(_X1_pt3, _X1_pt12);
                const double _X1_pt14 = (_X1_pt16 * _X1_pt2);
                const double _X1_pt19 = (_X1_pt9 * _X1_pt4);
                const double _X1_pt21 = (_12AX7_DEMPWOLF_Xi * _X1_pt19);
                const double _X1_pt8 = (_X1_pt9 * _X1_pt3);
                const double _X1_pt13 = (_12AX7_DEMPWOLF_Gamma * _X1_pt14);
                const double _X1_pt15 = (_12AX7_DEMPWOLF_Gk * _X1_pt13);
                const double _X1_pt20 = (_12AX7_DEMPWOLF_Gg * _X1_pt21);
                const double _X1_pt7 = (_12AX7_DEMPWOLF_Gg * _X1_pt8);
                const double _X1_pt18 = (_X1_pt20 / _X1_pt11);
                const double _X1_pt6 = (_X1_pt7 + _12AX7_DEMPWOLF_Ig0);
                const double _X1_i0 = ((_12AX7_DEMPWOLF_Gk * (_X1_pt16 * _X1_pt0)) - _X1_pt6);
                const double _X1_i1 = _X1_pt6;
                const double _X1_g0_0 = (_X1_pt15 / (_12AX7_DEMPWOLF_Mu * _X1_pt1));
                const double _X1_g0_1 = ((_X1_pt15 / _X1_pt1) - _X1_pt18);
                const double _X1_g1_0 = 0.0;
                const double _X1_g1_1 = _X1_pt18;
                const double _X1_pt22 = ((_X1_voc0 + (_X1_Z0_0 * _X1_i0)) + (_X1_Z0_1 * _X1_i1));
                const double _X1_pt23 = ((_X1_voc1 + (_X1_Z1_0 * _X1_i0)) + (_X1_Z1_1 * _X1_i1));
                const double _X1_pt24 = (vPKX1 - _X1_pt22);
                const double _X1_pt25 = (((_X1_Z1_0 * _X1_g0_1) + (_X1_Z1_1 * _X1_g1_1)) - 1.0);
                const double _X1_pt26 = ((_X1_Z0_0 * _X1_g0_1) + (_X1_Z0_1 * _X1_g1_1));
                const double _X1_pt27 = (vGKX1 - _X1_pt23);
                const double _X1_pt29 = (((_X1_Z0_0 * _X1_g0_0) + (_X1_Z0_1 * _X1_g1_0)) - 1.0);
                const double _X1_pt30 = ((_X1_Z1_0 * _X1_g0_0) + (_X1_Z1_1 * _X1_g1_0));
                const double _X1_pt28 = ((_X1_pt29 * _X1_pt25) - (_X1_pt26 * _X1_pt30));
                const double res_vPKX1 = (_X1_pt22 - vPKX1);
                const double res_vGKX1 = (_X1_pt23 - vGKX1);
                const double delta_vPKX1 = (((_X1_pt24 * _X1_pt25) - (_X1_pt26 * _X1_pt27)) / _X1_pt28);
                const double delta_vGKX1 = (((_X1_pt29 * _X1_pt27) - (_X1_pt24 * _X1_pt30)) / _X1_pt28);
            
                double residual_norm_sq = 0.0;
                residual_norm_sq += res_vPKX1 * res_vPKX1;
                residual_norm_sq += res_vGKX1 * res_vGKX1;
            
                vPKX1 = vPKX1 + (delta_vPKX1);
                vGKX1 = vGKX1 + (delta_vGKX1);
            
                if (residual_norm_sq < newton_tol_sq)
                    break;
                
            }

            const double _t3 = (_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi));
            const double _t9 = (_t3 + _12AX7_DEMPWOLF_Ig0);
            const double _t10 = (zCout * gRkCk);
            const double _t2 = ((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - _t9);
            const double _t11 = ((zRkCk + _t2) + _t9);
            const double _t1 = (_t2 - zCout);
            const double vo = ((((_t0 * _t10) + (_t1 * _t4)) - _t12) * _t6);
            const double vg = (-((_t9 - ((gCin * vi) - zCin)) * _t13));
            const double tCin = (gCin * (vi - vg));
            const double vpl = ((((_t1 * _t7) + (gCout * _t10)) - _t14) * _t6);
            const double tCout = (gCout * (vpl - vo));
            const double vk = (((gCout * (gCout * _t11)) - (_t0 * (_t8 * _t11))) * _t6);
            const double vRkCk = (vk - 0);
            
            zCin = 2 * tCin - zCin;
            zCout = 2 * tCout - zCout;
            zRkCk = gzRkCk * vRkCk - zRkCk; // RC parallel

            output[ch][n] = vo;
        }
        state[ch].zCin = zCin;
        state[ch].zCout = zCout;
        state[ch].zRkCk = zRkCk;
        state[ch].vPKX1 = vPKX1;
        state[ch].vPKX1_prev = vPKX1_prev;
        state[ch].vGKX1 = vGKX1;
        state[ch].vGKX1_prev = vGKX1_prev;
    }
}

static float reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc)
{
    #define sum(a, b) ((a) + (b))
    #define recip_sum(a, b) ((a) * (b) / ((a) + (b)))
    
    const double _12AX7_DEMPWOLF_Gk = params._12AX7_DEMPWOLF_Gk;
    const double _12AX7_DEMPWOLF_Mu = params._12AX7_DEMPWOLF_Mu;
    const double _12AX7_DEMPWOLF_Gamma = params._12AX7_DEMPWOLF_Gamma;
    const double _12AX7_DEMPWOLF_Ck = params._12AX7_DEMPWOLF_Ck;
    const double _12AX7_DEMPWOLF_Gg = params._12AX7_DEMPWOLF_Gg;
    const double _12AX7_DEMPWOLF_Xi = params._12AX7_DEMPWOLF_Xi;
    const double _12AX7_DEMPWOLF_Cg = params._12AX7_DEMPWOLF_Cg;
    const double _12AX7_DEMPWOLF_Ig0 = params._12AX7_DEMPWOLF_Ig0;
    
    const double VCC = params.VCC;
    
    const double vp = VCC;
    
    const double gCin = 2.0 * sample_rate * params.Cin;
    
    const double gRg = 1.0 / params.Rg;
    
    const double gCout = 2.0 * sample_rate * params.Cout;
    
    const double gRL = 1.0 / params.RL;
    
    const double gRp = 1.0 / params.Rp;
    
    const double Rk = params.Rk;
    const double Ck = params.Ck;
    const double gRkCk = 2.0 * sample_rate * Ck + (1.0 / Rk);
    const double gzRkCk = 4.0 * sample_rate * Ck;
    
    const double vi = vi_dc;

    double vPKX1 = 0;
    double vGKX1 = 0;

    const double _X1_zt1 = (1.0 / 1000000000.0);
    const double _X1_zt2 = (1.0 / Rk);
    const double _X1_zt3 = (_X1_zt2 + _X1_zt1);
    const double _X1_zt5 = (1.0 / _X1_zt3);
    const double _X1_zt0 = (gRp + _X1_zt1);
    const double _X1_zt4 = (-_X1_zt5);
    const double _X1_Z0_0 = ((-(1.0 / _X1_zt0)) + _X1_zt4);
    const double _X1_Z0_1 = _X1_zt4;
    const double _X1_Z1_0 = _X1_zt4;
    const double _X1_Z1_1 = (_X1_zt4 + (-(1.0 / (gRg + _X1_zt1))));
    // --- Newton-Raphson solve (N-port): X1
    const double _X1_voc0 = (-((-(gRp * VCC)) / _X1_zt0));
    const double _X1_pt13 = (_12AX7_DEMPWOLF_Xi - 1.0);
    const double _X1_pt17 = (_12AX7_DEMPWOLF_Gamma - 1.0);
    for (int newton_iter = 0; newton_iter < 10000; ++newton_iter)
    {
        const double _X1_pt3 = math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1)));
        const double _X1_pt12 = (_12AX7_DEMPWOLF_Cg * vGKX1);
        const double _X1_pt2 = (1.0 + _X1_pt3);
        const double _X1_pt11 = math_exp_approx(_X1_pt12);
        const double _X1_pt1 = (math_log_approx(_X1_pt2) / _12AX7_DEMPWOLF_Ck);
        const double _X1_pt10 = (1.0 + _X1_pt11);
        const double _X1_pt0 = math_pow_approx(_X1_pt1, _X1_pt17);
        const double _X1_pt9 = math_log_approx(_X1_pt10);
        const double _X1_pt14 = (_X1_pt0 * _X1_pt3);
        const double _X1_pt16 = (_12AX7_DEMPWOLF_Gamma * _X1_pt14);
        const double _X1_pt8 = (_X1_pt9 / _12AX7_DEMPWOLF_Cg);
        const double _X1_pt15 = (_12AX7_DEMPWOLF_Gk * _X1_pt16);
        const double _X1_pt7 = math_pow_approx(_X1_pt8, _X1_pt13);
        const double _X1_pt21 = (_X1_pt7 * _X1_pt11);
        const double _X1_pt6 = (_X1_pt7 * _X1_pt8);
        const double _X1_pt18 = (_12AX7_DEMPWOLF_Xi * _X1_pt21);
        const double _X1_pt20 = (_12AX7_DEMPWOLF_Gg * _X1_pt18);
        const double _X1_pt5 = (_12AX7_DEMPWOLF_Gg * _X1_pt6);
        const double _X1_pt19 = (_X1_pt20 / _X1_pt10);
        const double _X1_pt4 = (_X1_pt5 + _12AX7_DEMPWOLF_Ig0);
        const double _X1_i0 = ((_12AX7_DEMPWOLF_Gk * (_X1_pt0 * _X1_pt1)) - _X1_pt4);
        const double _X1_i1 = _X1_pt4;
        const double _X1_g0_0 = (_X1_pt15 / (_12AX7_DEMPWOLF_Mu * _X1_pt2));
        const double _X1_g0_1 = ((_X1_pt15 / _X1_pt2) - _X1_pt19);
        const double _X1_g1_0 = 0.0;
        const double _X1_g1_1 = _X1_pt19;
        const double _X1_pt22 = ((_X1_voc0 + (_X1_Z0_0 * _X1_i0)) + (_X1_Z0_1 * _X1_i1));
        const double _X1_pt23 = ((_X1_Z1_0 * _X1_i0) + (_X1_Z1_1 * _X1_i1));
        const double _X1_pt24 = (vPKX1 - _X1_pt22);
        const double _X1_pt25 = (((_X1_Z1_0 * _X1_g0_1) + (_X1_Z1_1 * _X1_g1_1)) - 1.0);
        const double _X1_pt26 = ((_X1_Z0_0 * _X1_g0_1) + (_X1_Z0_1 * _X1_g1_1));
        const double _X1_pt27 = (vGKX1 - _X1_pt23);
        const double _X1_pt29 = (((_X1_Z0_0 * _X1_g0_0) + (_X1_Z0_1 * _X1_g1_0)) - 1.0);
        const double _X1_pt30 = ((_X1_Z1_0 * _X1_g0_0) + (_X1_Z1_1 * _X1_g1_0));
        const double _X1_pt28 = ((_X1_pt29 * _X1_pt25) - (_X1_pt26 * _X1_pt30));
        const double res_vPKX1 = (_X1_pt22 - vPKX1);
        const double res_vGKX1 = (_X1_pt23 - vGKX1);
        const double delta_vPKX1 = (((_X1_pt24 * _X1_pt25) - (_X1_pt26 * _X1_pt27)) / _X1_pt28);
        const double delta_vGKX1 = (((_X1_pt29 * _X1_pt27) - (_X1_pt24 * _X1_pt30)) / _X1_pt28);
    
        double residual_norm_sq = 0.0;
        residual_norm_sq += res_vPKX1 * res_vPKX1;
        residual_norm_sq += res_vGKX1 * res_vGKX1;
    
        vPKX1 = vPKX1 + (delta_vPKX1);
        vGKX1 = vGKX1 + (delta_vGKX1);
    
        if (residual_norm_sq < newton_tol_sq)
            break;
        
    }
    const double zCin = (gCin * (vi + (((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0) / (gRg + (1.0 / 1000000000.0)))));
    const double zCout = (-((gCout * (((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) - (gRp * VCC))) / (gRp + (1.0 / 1000000000.0))));
    const double zRkCk = (((gzRkCk * (((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) + ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0))) / ((1.0 / Rk) + (1.0 / 1000000000.0))) / 2.0);

    const double vo_dc_out = 0.0;

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].vPKX1 = vPKX1;
        state[ch].vPKX1_prev = vPKX1;
        state[ch].vGKX1 = vGKX1;
        state[ch].vGKX1_prev = vGKX1;
        state[ch].zCin = zCin;
        state[ch].zCout = zCout;
        state[ch].zRkCk = zRkCk;
    }
    return vo_dc_out;
}

