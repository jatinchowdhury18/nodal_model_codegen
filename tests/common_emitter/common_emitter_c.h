// Auto-generated with netlist_codegen version 5608cd2.
// Command: netlist_codegen common_emitter.net common_emitter_c.h -opt_port_matrix -lang c

#pragma once

#include <math.h>
#include <stdint.h>

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


static float limit_junction_voltage(float v_new, float v_old, float vt, float vcrit) {
    if (v_new > vcrit && fabsf(v_new - v_old) > 2 * vt) {
        if (v_old > 0) {
            float arg = 1 + (v_new - v_old) / vt;
            v_new = arg > 0 ? v_old + vt * math_log_approx(arg) : vcrit;
        } else {
            v_new = vt * math_log_approx(v_new / vt);
        }
    }
    return v_new;
}

static const float newton_tol_sq = 0.00001;
static const int newton_max_iter = 20;


typedef struct {
    float VCC; // = 9.0e+00
    float C1; // = 1.0e-07
    float R1; // = 4.7e+05
    float R2; // = 1.0e+05
    float RE; // = 2.7e+03
    float RC; // = 1.2e+04
    float Q2N5089_Is; // = 5.0e-14
    float Q2N5089_vt; // = 2.585e-02
    float Q2N5089_BetaF; // = 6.0e+02
    float Q2N5089_BetaR; // = 5.0e+01
    float C2; // = 1.0e-07
    float RL; // = 1.0e+05
} Params;

typedef struct {
    float zC1;
    float zC2;
    float vBCQ1;
    float vBCQ1_prev;
    float vBEQ1;
    float vBEQ1_prev;
} State;

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    #define sum(a, b) ((a) + (b))
    #define recip_sum(a, b) ((a) * (b) / ((a) + (b)))
    
    const float Q2N5089_Is = params.Q2N5089_Is;
    const float Q2N5089_vt = params.Q2N5089_vt;
    const float Q2N5089_BetaF = params.Q2N5089_BetaF;
    const float Q2N5089_BetaR = params.Q2N5089_BetaR;
    
    const float VCC = params.VCC;
    
    const float vp = VCC;
    
    const float gC1 = 2.0f * sample_rate * params.C1;
    
    const float gR1 = 1.0f / params.R1;
    
    const float gR2 = 1.0f / params.R2;
    
    const float gRE = 1.0f / params.RE;
    
    const float gRC = 1.0f / params.RC;
    
    const float gC2 = 2.0f * sample_rate * params.C2;
    
    const float gRL = 1.0f / params.RL;
    
    const float vcrit_Q2N5089_vt = Q2N5089_vt * logf(Q2N5089_vt / (sqrtf(2.0) * Q2N5089_Is));
    
    const float _t0 = (gRE * 2048.0f);
    const float _t1 = ((gRC + gC2) * 128.0f);
    const float _t7 = (gC2 * 128.0f);
    const float _t8 = (gRC * 128.0f);
    const float _t9 = (262144.0f * gRE);
    const float _t11 = ((gC2 + gRL) * 128.0f);
    const float _t10 = ((_t1 * _t11) - (_t7 * _t7));
    const float _t13 = (1.0f / (_t9 * _t10));
    const float _t14 = (VCC * (_t0 * (_t8 * _t7)));
    const float _t15 = (gR1 * VCC);
    const float _t16 = (1.0f / ((gC1 + gR1) + gR2));
    const float _t17 = (VCC * (_t0 * (_t8 * _t11)));
    const float _Q1_zt1 = (gC1 + gR1);
    const float _Q1_zt2 = (gRE * 2048.0f);
    const float _Q1_zt5 = (gC2 + gRL);
    const float _Q1_zt6 = (gC2 * 128.0f);
    const float _Q1_zt8 = (-1.0f);
    const float _Q1_zt10 = (1.0f / gRE);
    const float _Q1_zt0 = (_Q1_zt1 + gR2);
    const float _Q1_zt4 = (_Q1_zt5 * 128.0f);
    const float _Q1_zt7 = (_Q1_zt8 / _Q1_zt0);
    const float _Q1_zt9 = (-_Q1_zt10);
    const float _Q1_zt3 = ((262144.0f * gRE) * ((((gRC + gC2) * 128.0f) * _Q1_zt4) - (_Q1_zt6 * _Q1_zt6)));
    const float _Q1_Z0_0 = _Q1_zt7;
    const float _Q1_Z0_1 = ((128.0f * (_Q1_zt2 * (_Q1_zt4 * (128.0f * 1.0f)))) / _Q1_zt3);
    const float _Q1_Z1_0 = (_Q1_zt7 + _Q1_zt9);
    const float _Q1_Z1_1 = _Q1_zt9;
    float c0__Q1_voc0;
    float c__Q1_voc0[3];
    float c0__Q1_zt13;
    float c__Q1_zt13[3];
    float c0__Q1_voc1;
    float c__Q1_voc1[3];
    float c0__Q1_zt14;
    float c__Q1_zt14[3];
    
    for (int _k = 0; _k <= 3; ++_k)
    {
        const float vi = (_k == 1) ? 1.0f : 0.0f;
        const float zC1 = (_k == 2) ? 1.0f : 0.0f;
        const float zC2 = (_k == 3) ? 1.0f : 0.0f;
        const float _Q1_zt12 = (gC1 * vi);
        const float _Q1_zt15 = (gR1 * VCC);
        const float _Q1_zt16 = (_Q1_zt12 - zC1);
        const float _Q1_zt11 = (-_Q1_zt16);
        const float _Q1_zt14 = (_Q1_zt15 - _Q1_zt11);
        const float _Q1_zt13 = (_Q1_zt14 / _Q1_zt0);
        const float _Q1_voc0 = (_Q1_zt13 + ((128.0f * ((_Q1_zt2 * ((_Q1_zt4 * (128.0f * (-zC2))) + (_Q1_zt6 * (zC2 * 128.0f)))) - (VCC * (_Q1_zt2 * ((gRC * 128.0f) * _Q1_zt4))))) / _Q1_zt3));
        const float _Q1_voc1 = _Q1_zt13;
        if (_k == 0) {
            c0__Q1_voc0 = _Q1_voc0;
            c0__Q1_zt13 = _Q1_zt13;
            c0__Q1_voc1 = _Q1_voc1;
            c0__Q1_zt14 = _Q1_zt14;
        } else {
            c__Q1_voc0[_k - 1] = _Q1_voc0 - c0__Q1_voc0;
            c__Q1_zt13[_k - 1] = _Q1_zt13 - c0__Q1_zt13;
            c__Q1_voc1[_k - 1] = _Q1_voc1 - c0__Q1_voc1;
            c__Q1_zt14[_k - 1] = _Q1_zt14 - c0__Q1_zt14;
        }
    }
    
    for (int ch = 0; ch < num_channels; ++ch)
    {
        float zC1 = state[ch].zC1;
        float zC2 = state[ch].zC2;
        float vBCQ1 = state[ch].vBCQ1;
        float vBCQ1_prev = state[ch].vBCQ1_prev;
        float vBEQ1 = state[ch].vBEQ1;
        float vBEQ1_prev = state[ch].vBEQ1_prev;
        for (int n = 0; n < num_samples; ++n)
        {
            const float vi = input[ch][n];

            { const float _prev_step = vBCQ1 - vBCQ1_prev; vBCQ1_prev = vBCQ1;
vBCQ1 = limit_junction_voltage(vBCQ1 + (_prev_step), vBCQ1, Q2N5089_vt, vcrit_Q2N5089_vt);
            }
            { const float _prev_step = vBEQ1 - vBEQ1_prev; vBEQ1_prev = vBEQ1;
vBEQ1 = limit_junction_voltage(vBEQ1 + (_prev_step), vBEQ1, Q2N5089_vt, vcrit_Q2N5089_vt);
            }

            // --- Newton-Raphson solve (N-port): Q1
            const float _Q1_voc0 = c0__Q1_voc0 + c__Q1_voc0[0] * vi + c__Q1_voc0[1] * zC1 + c__Q1_voc0[2] * zC2;
            const float _Q1_zt13 = c0__Q1_zt13 + c__Q1_zt13[0] * vi + c__Q1_zt13[1] * zC1 + c__Q1_zt13[2] * zC2;
            const float _Q1_voc1 = c0__Q1_voc1 + c__Q1_voc1[0] * vi + c__Q1_voc1[1] * zC1 + c__Q1_voc1[2] * zC2;
            const float _Q1_zt14 = c0__Q1_zt14 + c__Q1_zt14[0] * vi + c__Q1_zt14[1] * zC1 + c__Q1_zt14[2] * zC2;
            const float _Q1_zt12 = (gC1 * vi);
            const float _Q1_zt15 = (gR1 * VCC);
            const float _Q1_zt16 = (_Q1_zt12 - zC1);
            const float _Q1_zt11 = (-_Q1_zt16);
            for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
            {
                const float _Q1_pt0 = (vBCQ1 / Q2N5089_vt);
                const float _Q1_pt1 = (vBEQ1 / Q2N5089_vt);
                const float _Q1_pt4 = math_exp_approx(_Q1_pt0);
                const float _Q1_pt6 = (_Q1_pt4 / Q2N5089_vt);
                const float _Q1_pt8 = math_exp_approx(_Q1_pt1);
                const float _Q1_pt3 = (_Q1_pt4 - 1.0f);
                const float _Q1_pt5 = (Q2N5089_Is * _Q1_pt8);
                const float _Q1_pt7 = (_Q1_pt5 / Q2N5089_vt);
                const float _Q1_pt2 = (_Q1_pt3 / Q2N5089_BetaR);
                const float _Q1_i0 = (Q2N5089_Is * (((_Q1_pt8 - 1.0f) / Q2N5089_BetaF) + _Q1_pt2));
                const float _Q1_i1 = (Q2N5089_Is * ((_Q1_pt8 - _Q1_pt4) - _Q1_pt2));
                const float _Q1_g0_0 = (((Q2N5089_Is * _Q1_pt4) / Q2N5089_vt) / Q2N5089_BetaR);
                const float _Q1_g0_1 = (_Q1_pt7 / Q2N5089_BetaF);
                const float _Q1_g1_0 = (-(Q2N5089_Is * (_Q1_pt6 + (_Q1_pt6 / Q2N5089_BetaR))));
                const float _Q1_g1_1 = _Q1_pt7;
                const float _Q1_pt9 = ((_Q1_voc0 + (_Q1_Z0_0 * _Q1_i0)) + (_Q1_Z0_1 * _Q1_i1));
                const float _Q1_pt10 = ((_Q1_voc1 + (_Q1_Z1_0 * _Q1_i0)) + (_Q1_Z1_1 * _Q1_i1));
                const float _Q1_pt11 = (vBCQ1 - _Q1_pt9);
                const float _Q1_pt12 = (((_Q1_Z1_0 * _Q1_g0_1) + (_Q1_Z1_1 * _Q1_g1_1)) - 1.0f);
                const float _Q1_pt13 = ((_Q1_Z0_0 * _Q1_g0_1) + (_Q1_Z0_1 * _Q1_g1_1));
                const float _Q1_pt14 = (vBEQ1 - _Q1_pt10);
                const float _Q1_pt16 = (((_Q1_Z0_0 * _Q1_g0_0) + (_Q1_Z0_1 * _Q1_g1_0)) - 1.0f);
                const float _Q1_pt17 = ((_Q1_Z1_0 * _Q1_g0_0) + (_Q1_Z1_1 * _Q1_g1_0));
                const float _Q1_pt15 = ((_Q1_pt16 * _Q1_pt12) - (_Q1_pt13 * _Q1_pt17));
                const float res_vBCQ1 = (_Q1_pt9 - vBCQ1);
                const float res_vBEQ1 = (_Q1_pt10 - vBEQ1);
                const float delta_vBCQ1 = (((_Q1_pt11 * _Q1_pt12) - (_Q1_pt13 * _Q1_pt14)) / _Q1_pt15);
                const float delta_vBEQ1 = (((_Q1_pt16 * _Q1_pt14) - (_Q1_pt11 * _Q1_pt17)) / _Q1_pt15);
            
                float residual_norm_sq = 0.0;
                residual_norm_sq += res_vBCQ1 * res_vBCQ1;
                residual_norm_sq += res_vBEQ1 * res_vBEQ1;
            
                vBCQ1 = limit_junction_voltage(vBCQ1 + (delta_vBCQ1), vBCQ1, Q2N5089_vt, vcrit_Q2N5089_vt);
                vBEQ1 = limit_junction_voltage(vBEQ1 + (delta_vBEQ1), vBEQ1, Q2N5089_vt, vcrit_Q2N5089_vt);
            
                if (residual_norm_sq < newton_tol_sq)
                    break;
                
            }

            const float _t2 = (zC2 * 128.0f);
            const float _t4 = math_exp_approx((vBEQ1 / Q2N5089_vt));
            const float _t5 = math_exp_approx((vBCQ1 / Q2N5089_vt));
            const float _t12 = (_t5 - 1.0f);
            const float _t6 = (_t12 / Q2N5089_BetaR);
            const float _t3 = (((Q2N5089_Is * ((_t4 - _t5) - _t6)) - zC2) * 128.0f);
            const float vo = (-((128.0f * ((_t0 * ((_t1 * _t2) + (_t3 * _t7))) - _t14)) * _t13));
            const float vb = ((_t15 - ((Q2N5089_Is * (((_t4 - 1.0f) / Q2N5089_BetaF) + _t6)) - ((gC1 * vi) - zC1))) * _t16);
            const float tC1 = (gC1 * (vi - vb));
            const float vc = (-((128.0f * ((_t0 * ((_t3 * _t11) + (_t7 * _t2))) - _t17)) * _t13));
            const float tC2 = (gC2 * (vc - vo));
            
            zC1 = 2 * tC1 - zC1;
            zC2 = 2 * tC2 - zC2;

            output[ch][n] = vo;
        }
        state[ch].zC1 = zC1;
        state[ch].zC2 = zC2;
        state[ch].vBCQ1 = vBCQ1;
        state[ch].vBCQ1_prev = vBCQ1_prev;
        state[ch].vBEQ1 = vBEQ1;
        state[ch].vBEQ1_prev = vBEQ1_prev;
    }
}

static float reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc)
{
    #define sum(a, b) ((a) + (b))
    #define recip_sum(a, b) ((a) * (b) / ((a) + (b)))
    
    const float Q2N5089_Is = params.Q2N5089_Is;
    const float Q2N5089_vt = params.Q2N5089_vt;
    const float Q2N5089_BetaF = params.Q2N5089_BetaF;
    const float Q2N5089_BetaR = params.Q2N5089_BetaR;
    
    const float VCC = params.VCC;
    
    const float vp = VCC;
    
    const float gC1 = 2.0f * sample_rate * params.C1;
    
    const float gR1 = 1.0f / params.R1;
    
    const float gR2 = 1.0f / params.R2;
    
    const float gRE = 1.0f / params.RE;
    
    const float gRC = 1.0f / params.RC;
    
    const float gC2 = 2.0f * sample_rate * params.C2;
    
    const float gRL = 1.0f / params.RL;
    
    const float vcrit_Q2N5089_vt = Q2N5089_vt * logf(Q2N5089_vt / (sqrtf(2.0) * Q2N5089_Is));
    
    const float vi = vi_dc;

    float vBCQ1 = 0;
    float vBEQ1 = 0;

    const float _Q1_zt0 = (-1.0f);
    const float _Q1_zt2 = (gR1 + gR2);
    const float _Q1_zt3 = (1.0f / 1000000000.0f);
    const float _Q1_zt4 = (gRC + _Q1_zt3);
    const float _Q1_zt6 = (gRE + _Q1_zt3);
    const float _Q1_zt8 = (1.0f / _Q1_zt6);
    const float _Q1_zt1 = (_Q1_zt2 + _Q1_zt3);
    const float _Q1_zt5 = (_Q1_zt0 / _Q1_zt1);
    const float _Q1_zt7 = (-_Q1_zt8);
    const float _Q1_Z0_0 = _Q1_zt5;
    const float _Q1_Z0_1 = (1.0f / _Q1_zt4);
    const float _Q1_Z1_0 = (_Q1_zt5 + _Q1_zt7);
    const float _Q1_Z1_1 = _Q1_zt7;
    // --- Newton-Raphson solve (N-port): Q1
    const float _Q1_zt9 = (gR1 * VCC);
    const float _Q1_zt10 = (_Q1_zt9 / _Q1_zt1);
    const float _Q1_voc0 = (_Q1_zt10 + ((-(VCC * gRC)) / _Q1_zt4));
    const float _Q1_voc1 = _Q1_zt10;
    for (int newton_iter = 0; newton_iter < 10000; ++newton_iter)
    {
        const float _Q1_pt0 = (vBCQ1 / Q2N5089_vt);
        const float _Q1_pt1 = (vBEQ1 / Q2N5089_vt);
        const float _Q1_pt4 = math_exp_approx(_Q1_pt0);
        const float _Q1_pt6 = (_Q1_pt4 / Q2N5089_vt);
        const float _Q1_pt8 = math_exp_approx(_Q1_pt1);
        const float _Q1_pt3 = (_Q1_pt4 - 1.0f);
        const float _Q1_pt5 = (Q2N5089_Is * _Q1_pt8);
        const float _Q1_pt7 = (_Q1_pt5 / Q2N5089_vt);
        const float _Q1_pt2 = (_Q1_pt3 / Q2N5089_BetaR);
        const float _Q1_i0 = (Q2N5089_Is * (((_Q1_pt8 - 1.0f) / Q2N5089_BetaF) + _Q1_pt2));
        const float _Q1_i1 = (Q2N5089_Is * ((_Q1_pt8 - _Q1_pt4) - _Q1_pt2));
        const float _Q1_g0_0 = (((Q2N5089_Is * _Q1_pt4) / Q2N5089_vt) / Q2N5089_BetaR);
        const float _Q1_g0_1 = (_Q1_pt7 / Q2N5089_BetaF);
        const float _Q1_g1_0 = (-(Q2N5089_Is * (_Q1_pt6 + (_Q1_pt6 / Q2N5089_BetaR))));
        const float _Q1_g1_1 = _Q1_pt7;
        const float _Q1_pt9 = ((_Q1_voc0 + (_Q1_Z0_0 * _Q1_i0)) + (_Q1_Z0_1 * _Q1_i1));
        const float _Q1_pt10 = ((_Q1_voc1 + (_Q1_Z1_0 * _Q1_i0)) + (_Q1_Z1_1 * _Q1_i1));
        const float _Q1_pt11 = (vBCQ1 - _Q1_pt9);
        const float _Q1_pt12 = (((_Q1_Z1_0 * _Q1_g0_1) + (_Q1_Z1_1 * _Q1_g1_1)) - 1.0f);
        const float _Q1_pt13 = ((_Q1_Z0_0 * _Q1_g0_1) + (_Q1_Z0_1 * _Q1_g1_1));
        const float _Q1_pt14 = (vBEQ1 - _Q1_pt10);
        const float _Q1_pt16 = (((_Q1_Z0_0 * _Q1_g0_0) + (_Q1_Z0_1 * _Q1_g1_0)) - 1.0f);
        const float _Q1_pt17 = ((_Q1_Z1_0 * _Q1_g0_0) + (_Q1_Z1_1 * _Q1_g1_0));
        const float _Q1_pt15 = ((_Q1_pt16 * _Q1_pt12) - (_Q1_pt13 * _Q1_pt17));
        const float res_vBCQ1 = (_Q1_pt9 - vBCQ1);
        const float res_vBEQ1 = (_Q1_pt10 - vBEQ1);
        const float delta_vBCQ1 = (((_Q1_pt11 * _Q1_pt12) - (_Q1_pt13 * _Q1_pt14)) / _Q1_pt15);
        const float delta_vBEQ1 = (((_Q1_pt16 * _Q1_pt14) - (_Q1_pt11 * _Q1_pt17)) / _Q1_pt15);
    
        float residual_norm_sq = 0.0;
        residual_norm_sq += res_vBCQ1 * res_vBCQ1;
        residual_norm_sq += res_vBEQ1 * res_vBEQ1;
    
        vBCQ1 = limit_junction_voltage(vBCQ1 + (delta_vBCQ1), vBCQ1, Q2N5089_vt, vcrit_Q2N5089_vt);
        vBEQ1 = limit_junction_voltage(vBEQ1 + (delta_vBEQ1), vBEQ1, Q2N5089_vt, vcrit_Q2N5089_vt);
    
        if (residual_norm_sq < newton_tol_sq)
            break;
        
    }
    const float zC1 = (gC1 * (vi - (((gR1 * VCC) - (Q2N5089_Is * (((math_exp_approx((vBEQ1 / Q2N5089_vt)) - 1.0f) / Q2N5089_BetaF) + ((math_exp_approx((vBCQ1 / Q2N5089_vt)) - 1.0f) / Q2N5089_BetaR)))) / ((gR1 + gR2) + (1.0f / 1000000000.0f)))));
    const float zC2 = (-((gC2 * ((Q2N5089_Is * ((math_exp_approx((vBEQ1 / Q2N5089_vt)) - math_exp_approx((vBCQ1 / Q2N5089_vt))) - ((math_exp_approx((vBCQ1 / Q2N5089_vt)) - 1.0f) / Q2N5089_BetaR))) - (VCC * gRC))) / (gRC + (1.0f / 1000000000.0f))));

    const float vo_dc_out = 0.0f;

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].vBCQ1 = vBCQ1;
        state[ch].vBCQ1_prev = vBCQ1;
        state[ch].vBEQ1 = vBEQ1;
        state[ch].vBEQ1_prev = vBEQ1;
        state[ch].zC1 = zC1;
        state[ch].zC2 = zC2;
    }
    return vo_dc_out;
}

