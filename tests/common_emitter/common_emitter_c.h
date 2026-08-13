// Auto-generated with netlist_codegen version 03d2306.
// Command: netlist_codegen common_emitter.net common_emitter_c.h -lang c

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
    } else if (v_new < -vcrit && fabsf(v_new - v_old) > 2 * vt) {
        if (v_old < 0) {
            float arg = 1 + (v_old - v_new) / vt;
            v_new = arg > 0 ? v_old - vt * math_log_approx(arg) : -vcrit;
        } else {
            v_new = -vt * math_log_approx(-v_new / vt);
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
    float vBEQ1;
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
    const float _t8 = (gC2 * 128.0f);
    const float _t9 = (gRC * 128.0f);
    const float _t11 = (262144.0f * gRE);
    const float _t13 = ((gC2 + gRL) * 128.0f);
    const float _t12 = ((_t1 * _t13) - (_t8 * _t8));
    const float _t10 = (1.0f / (_t11 * _t12));
    const float _t14 = (VCC * (_t0 * (_t9 * _t8)));
    const float _t15 = (gR1 * VCC);
    const float _t16 = (1.0f / ((gC1 + gR1) + gR2));
    const float _t17 = (VCC * (_t0 * (_t9 * _t13)));
    for (int ch = 0; ch < num_channels; ++ch)
    {
        float zC1 = state[ch].zC1;
        float zC2 = state[ch].zC2;
        float vBCQ1 = state[ch].vBCQ1;
        float vBEQ1 = state[ch].vBEQ1;
        for (int n = 0; n < num_samples; ++n)
        {
            const float vi = input[ch][n];

            // --- Newton-Raphson solve: Q1
            const float _Q1_t3 = (gR1 * VCC);
            const float _Q1_t16 = (gC1 * vi);
            const float _Q1_t18 = (gC1 + gR1);
            const float _Q1_t19 = (gRE * 2048.0f);
            const float _Q1_t24 = (gC2 + gRL);
            const float _Q1_t25 = (gC2 * 128.0f);
            const float _Q1_t32 = (1.0f / (Q2N5089_vt * Q2N5089_BetaF));
            const float _Q1_t35 = (16384.0f * Q2N5089_Is);
            const float _Q1_t43 = (1.0f / (Q2N5089_vt * Q2N5089_BetaR));
            const float _Q1_t15 = (_Q1_t16 - zC1);
            const float _Q1_t17 = (1.0f / (_Q1_t18 + gR2));
            const float _Q1_t23 = (_Q1_t24 * 128.0f);
            const float _Q1_t26 = ((((gRC + gC2) * 128.0f) * _Q1_t23) - (_Q1_t25 * _Q1_t25));
            const float _Q1_t48 = (_Q1_t25 * (zC2 * 128.0f));
            const float _Q1_t49 = (VCC * (_Q1_t19 * ((gRC * 128.0f) * _Q1_t23)));
            const float _Q1_t50 = (1.0f / ((262144.0f * gRE) * _Q1_t26));
            const float _Q1_t51 = (1.0f / _Q1_t26);
            for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
            {
                const float _Q1_t10 = (vBEQ1 / Q2N5089_vt);
                const float _Q1_t14 = (vBCQ1 / Q2N5089_vt);
                const float _Q1_t9 = math_exp_approx(_Q1_t10);
                const float _Q1_t13 = math_exp_approx(_Q1_t14);
                const float _Q1_t22 = (_Q1_t9 - _Q1_t13);
                const float _Q1_t31 = (Q2N5089_Is * _Q1_t9);
                const float _Q1_t33 = (_Q1_t31 / Q2N5089_vt);
                const float _Q1_t42 = (Q2N5089_Is * _Q1_t13);
                const float _Q1_t45 = (_Q1_t13 / Q2N5089_vt);
                const float _Q1_t46 = (_Q1_t45 / Q2N5089_BetaR);
                const float _Q1_t8 = (_Q1_t9 - 1.0f);
                const float _Q1_t12 = (_Q1_t13 - 1.0f);
                const float _Q1_t30 = (_Q1_t31 * _Q1_t32);
                const float _Q1_t41 = (_Q1_t42 * _Q1_t43);
                const float _Q1_t44 = (_Q1_t45 + _Q1_t46);
                const float _Q1_t7 = (_Q1_t8 / Q2N5089_BetaF);
                const float _Q1_t11 = (_Q1_t12 / Q2N5089_BetaR);
                const float _Q1_t21 = (_Q1_t22 - _Q1_t11);
                const float _Q1_t29 = (_Q1_t30 * _Q1_t17);
                const float _Q1_t34 = (((((_Q1_t35 * _Q1_t9) * _Q1_t24) / Q2N5089_vt) * _Q1_t51) - _Q1_t29);
                const float _Q1_t40 = (_Q1_t41 * _Q1_t17);
                const float _Q1_t47 = (_Q1_t40 + ((((_Q1_t42 / Q2N5089_vt) / Q2N5089_BetaR) - (Q2N5089_Is * _Q1_t44)) / gRE));
                const float _Q1_t6 = (_Q1_t7 + _Q1_t11);
                const float _Q1_t20 = (Q2N5089_Is * _Q1_t21);
                const float _Q1_t28 = ((_Q1_t29 + (((_Q1_t33 / Q2N5089_BetaF) + _Q1_t33) / gRE)) + 1.0f);
                const float _Q1_t39 = ((_Q1_t40 + (((_Q1_t35 * _Q1_t44) * _Q1_t24) * _Q1_t51)) + 1.0f);
                const float _Q1_t5 = (Q2N5089_Is * _Q1_t6);
                const float _Q1_t38 = ((_Q1_t39 * _Q1_t28) + (_Q1_t34 * _Q1_t47));
                const float _Q1_t4 = (_Q1_t5 - _Q1_t15);
                const float _Q1_t2 = (_Q1_t3 - _Q1_t4);
                const float _Q1_t1 = (_Q1_t2 * _Q1_t17);
                const float _Q1_t37 = (_Q1_t1 - ((_Q1_t5 + _Q1_t20) / gRE));
                const float _Q1_t0 = (_Q1_t1 + ((128.0f * ((_Q1_t19 * ((((_Q1_t20 - zC2) * 128.0f) * _Q1_t23) + _Q1_t48)) - _Q1_t49)) * _Q1_t50));
                const float _Q1_t27 = (vBCQ1 - _Q1_t0);
                const float _Q1_t36 = (vBEQ1 - _Q1_t37);
                const float res_vBCQ1 = (_Q1_t0 - vBCQ1);
                const float delta_vBCQ1 = (-(((_Q1_t27 * _Q1_t28) + (_Q1_t34 * _Q1_t36)) / _Q1_t38));
                const float res_vBEQ1 = (_Q1_t37 - vBEQ1);
                const float delta_vBEQ1 = (((_Q1_t27 * _Q1_t47) - (_Q1_t39 * _Q1_t36)) / _Q1_t38);
            
                float residual_norm_sq = 0.0;
                residual_norm_sq += res_vBCQ1 * res_vBCQ1;
                residual_norm_sq += res_vBEQ1 * res_vBEQ1;
                float step_norm_sq = 0.0;
                step_norm_sq += delta_vBCQ1 * delta_vBCQ1;
                step_norm_sq += delta_vBEQ1 * delta_vBEQ1;
            
                vBCQ1 = limit_junction_voltage(vBCQ1 + (delta_vBCQ1), vBCQ1, Q2N5089_vt, vcrit_Q2N5089_vt);
                vBEQ1 = limit_junction_voltage(vBEQ1 + (delta_vBEQ1), vBEQ1, Q2N5089_vt, vcrit_Q2N5089_vt);
            
                if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
                    break;
                
            }

            const float _t2 = (zC2 * 128.0f);
            const float _t4 = math_exp_approx((vBEQ1 / Q2N5089_vt));
            const float _t5 = math_exp_approx((vBCQ1 / Q2N5089_vt));
            const float _t7 = (_t5 - 1.0f);
            const float _t6 = (_t7 / Q2N5089_BetaR);
            const float _t3 = (((Q2N5089_Is * ((_t4 - _t5) - _t6)) - zC2) * 128.0f);
            const float vo = (-((128.0f * ((_t0 * ((_t1 * _t2) + (_t3 * _t8))) - _t14)) * _t10));
            const float vb = ((_t15 - ((Q2N5089_Is * (((_t4 - 1.0f) / Q2N5089_BetaF) + _t6)) - ((gC1 * vi) - zC1))) * _t16);
            const float tC1 = (gC1 * (vi - vb));
            const float vc = (-((128.0f * ((_t0 * ((_t3 * _t13) + (_t8 * _t2))) - _t17)) * _t10));
            const float tC2 = (gC2 * (vc - vo));
            
            zC1 = 2 * tC1 - zC1;
            zC2 = 2 * tC2 - zC2;

            output[ch][n] = vo;
        }
        state[ch].zC1 = zC1;
        state[ch].zC2 = zC2;
        state[ch].vBCQ1 = vBCQ1;
        state[ch].vBEQ1 = vBEQ1;
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

    // --- Newton-Raphson solve: Q1
    const float _Q1_t3 = (gR1 * VCC);
    const float _Q1_t15 = (gR1 + gR2);
    const float _Q1_t16 = (1.0f / 1000000000.0f);
    const float _Q1_t20 = (1.0f / (gRC + _Q1_t16));
    const float _Q1_t26 = (1.0f / (Q2N5089_vt * Q2N5089_BetaF));
    const float _Q1_t28 = (1.0f / (gRE + _Q1_t16));
    const float _Q1_t37 = (1.0f / (Q2N5089_vt * Q2N5089_BetaR));
    const float _Q1_t14 = (1.0f / (_Q1_t15 + _Q1_t16));
    const float _Q1_t43 = (VCC * gRC);
    for (int newton_iter = 0; newton_iter < 10000; ++newton_iter)
    {
        const float _Q1_t9 = (vBEQ1 / Q2N5089_vt);
        const float _Q1_t13 = (vBCQ1 / Q2N5089_vt);
        const float _Q1_t8 = math_exp_approx(_Q1_t9);
        const float _Q1_t12 = math_exp_approx(_Q1_t13);
        const float _Q1_t19 = (_Q1_t8 - _Q1_t12);
        const float _Q1_t25 = (Q2N5089_Is * _Q1_t8);
        const float _Q1_t27 = (_Q1_t25 / Q2N5089_vt);
        const float _Q1_t36 = (Q2N5089_Is * _Q1_t12);
        const float _Q1_t40 = (_Q1_t12 / Q2N5089_vt);
        const float _Q1_t41 = (_Q1_t40 / Q2N5089_BetaR);
        const float _Q1_t7 = (_Q1_t8 - 1.0f);
        const float _Q1_t11 = (_Q1_t12 - 1.0f);
        const float _Q1_t24 = (_Q1_t25 * _Q1_t26);
        const float _Q1_t35 = (_Q1_t36 * _Q1_t37);
        const float _Q1_t39 = (_Q1_t40 + _Q1_t41);
        const float _Q1_t6 = (_Q1_t7 / Q2N5089_BetaF);
        const float _Q1_t10 = (_Q1_t11 / Q2N5089_BetaR);
        const float _Q1_t18 = (_Q1_t19 - _Q1_t10);
        const float _Q1_t23 = (_Q1_t24 * _Q1_t14);
        const float _Q1_t29 = ((_Q1_t27 * _Q1_t20) - _Q1_t23);
        const float _Q1_t34 = (_Q1_t35 * _Q1_t14);
        const float _Q1_t38 = (Q2N5089_Is * _Q1_t39);
        const float _Q1_t42 = (_Q1_t34 + ((((_Q1_t36 / Q2N5089_vt) / Q2N5089_BetaR) - _Q1_t38) * _Q1_t28));
        const float _Q1_t5 = (_Q1_t6 + _Q1_t10);
        const float _Q1_t17 = (Q2N5089_Is * _Q1_t18);
        const float _Q1_t22 = ((_Q1_t23 + (((_Q1_t27 / Q2N5089_BetaF) + _Q1_t27) * _Q1_t28)) + 1.0f);
        const float _Q1_t33 = ((_Q1_t34 + (_Q1_t38 * _Q1_t20)) + 1.0f);
        const float _Q1_t4 = (Q2N5089_Is * _Q1_t5);
        const float _Q1_t32 = ((_Q1_t33 * _Q1_t22) + (_Q1_t29 * _Q1_t42));
        const float _Q1_t2 = (_Q1_t3 - _Q1_t4);
        const float _Q1_t1 = (_Q1_t2 * _Q1_t14);
        const float _Q1_t31 = (_Q1_t1 - ((_Q1_t4 + _Q1_t17) * _Q1_t28));
        const float _Q1_t0 = (_Q1_t1 + ((_Q1_t17 - _Q1_t43) * _Q1_t20));
        const float _Q1_t21 = (vBCQ1 - _Q1_t0);
        const float _Q1_t30 = (vBEQ1 - _Q1_t31);
        const float res_vBCQ1 = (_Q1_t0 - vBCQ1);
        const float delta_vBCQ1 = (-(((_Q1_t21 * _Q1_t22) + (_Q1_t29 * _Q1_t30)) / _Q1_t32));
        const float res_vBEQ1 = (_Q1_t31 - vBEQ1);
        const float delta_vBEQ1 = (((_Q1_t21 * _Q1_t42) - (_Q1_t33 * _Q1_t30)) / _Q1_t32);
    
        float residual_norm_sq = 0.0;
        residual_norm_sq += res_vBCQ1 * res_vBCQ1;
        residual_norm_sq += res_vBEQ1 * res_vBEQ1;
        float step_norm_sq = 0.0;
        step_norm_sq += delta_vBCQ1 * delta_vBCQ1;
        step_norm_sq += delta_vBEQ1 * delta_vBEQ1;
    
        vBCQ1 = limit_junction_voltage(vBCQ1 + (delta_vBCQ1), vBCQ1, Q2N5089_vt, vcrit_Q2N5089_vt);
        vBEQ1 = limit_junction_voltage(vBEQ1 + (delta_vBEQ1), vBEQ1, Q2N5089_vt, vcrit_Q2N5089_vt);
    
        if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
            break;
        
    }
    const float zC1 = (gC1 * (vi - (((gR1 * VCC) - (Q2N5089_Is * (((math_exp_approx((vBEQ1 / Q2N5089_vt)) - 1.0f) / Q2N5089_BetaF) + ((math_exp_approx((vBCQ1 / Q2N5089_vt)) - 1.0f) / Q2N5089_BetaR)))) / ((gR1 + gR2) + (1.0f / 1000000000.0f)))));
    const float zC2 = (-((gC2 * ((Q2N5089_Is * ((math_exp_approx((vBEQ1 / Q2N5089_vt)) - math_exp_approx((vBCQ1 / Q2N5089_vt))) - ((math_exp_approx((vBCQ1 / Q2N5089_vt)) - 1.0f) / Q2N5089_BetaR))) - (VCC * gRC))) / (gRC + (1.0f / 1000000000.0f))));

    const float vo_dc_out = 0.0f;

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].vBCQ1 = vBCQ1;
        state[ch].vBEQ1 = vBEQ1;
        state[ch].zC1 = zC1;
        state[ch].zC2 = zC2;
    }
    return vo_dc_out;
}

