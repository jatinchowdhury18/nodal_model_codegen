// Auto-generated with netlist_codegen version d1e5ccb.
// Command: netlist_codegen common_emitter_pnp.net common_emitter_pnp_c.h -lang c -type_name double

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


static double limit_junction_voltage(double v_new, double v_old, double vt, double vcrit) {
    if (v_new > vcrit && fabs(v_new - v_old) > 2 * vt) {
        if (v_old > 0) {
            double arg = 1 + (v_new - v_old) / vt;
            v_new = arg > 0 ? v_old + vt * math_log_approx(arg) : vcrit;
        } else {
            v_new = vt * math_log_approx(v_new / vt);
        }
    } else if (v_new < -vcrit && fabs(v_new - v_old) > 2 * vt) {
        if (v_old < 0) {
            double arg = 1 + (v_old - v_new) / vt;
            v_new = arg > 0 ? v_old - vt * math_log_approx(arg) : -vcrit;
        } else {
            v_new = -vt * math_log_approx(-v_new / vt);
        }
    }
    return v_new;
}

static const double newton_tol_sq = 0.00001;
static const int newton_max_iter = 20;


typedef struct {
    double VEE; // = -9.0e+00
    double C1; // = 1.0e-07
    double R1; // = 4.7e+05
    double R2; // = 1.0e+05
    double RE; // = 2.7e+03
    double RC; // = 1.2e+04
    double Q2N5087_Is; // = 5.0e-14
    double Q2N5087_vt; // = 2.585e-02
    double Q2N5087_BetaF; // = 6.0e+02
    double Q2N5087_BetaR; // = 5.0e+01
    double C2; // = 1.0e-07
    double RL; // = 1.0e+05
} Params;

typedef struct {
    double zC1;
    double zC2;
    double vCBQ1;
    double vEBQ1;
} State;

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    #define sum(a, b) ((a) + (b))
    #define recip_sum(a, b) ((a) * (b) / ((a) + (b)))
    
    const double Q2N5087_Is = params.Q2N5087_Is;
    const double Q2N5087_vt = params.Q2N5087_vt;
    const double Q2N5087_BetaF = params.Q2N5087_BetaF;
    const double Q2N5087_BetaR = params.Q2N5087_BetaR;
    
    const double VEE = params.VEE;
    
    const double vn = VEE;
    
    const double gC1 = 2.0 * sample_rate * params.C1;
    
    const double gR1 = 1.0 / params.R1;
    
    const double gR2 = 1.0 / params.R2;
    
    const double gRE = 1.0 / params.RE;
    
    const double gRC = 1.0 / params.RC;
    
    const double gC2 = 2.0 * sample_rate * params.C2;
    
    const double gRL = 1.0 / params.RL;
    
    const double vcrit_Q2N5087_vt = Q2N5087_vt * log(Q2N5087_vt / (sqrt(2.0) * Q2N5087_Is));
    
    const double _t3 = (gRC + gC2);
    const double _t4 = (VEE * gRC);
    const double _t6 = (gC2 + gRL);
    const double _t5 = (1.0 / ((_t3 * _t6) - (gC2 * gC2)));
    const double _t9 = (_t4 * gC2);
    const double _t10 = (gR1 * VEE);
    const double _t11 = (1.0 / ((gC1 + gR1) + gR2));
    const double _t12 = (_t4 * _t6);
    for (int ch = 0; ch < num_channels; ++ch)
    {
        double zC1 = state[ch].zC1;
        double zC2 = state[ch].zC2;
        double vCBQ1 = state[ch].vCBQ1;
        double vEBQ1 = state[ch].vEBQ1;
        for (int n = 0; n < num_samples; ++n)
        {
            const double vi = input[ch][n];

            // --- Newton-Raphson solve: Q1
            const double _Q1_t5 = (gC2 + gRL);
            const double _Q1_t6 = (1.0 / (((gRC + gC2) * _Q1_t5) - (gC2 * gC2)));
            const double _Q1_t17 = (1.0 / (Q2N5087_vt * Q2N5087_BetaF));
            const double _Q1_t18 = (gC1 + gR1);
            const double _Q1_t28 = (gC1 * vi);
            const double _Q1_t32 = (gR1 * VEE);
            const double _Q1_t44 = (1.0 / (Q2N5087_vt * Q2N5087_BetaR));
            const double _Q1_t9 = (1.0 / (_Q1_t18 + gR2));
            const double _Q1_t27 = (_Q1_t28 - zC1);
            const double _Q1_t45 = (gC2 * zC2);
            const double _Q1_t46 = ((VEE * gRC) * _Q1_t5);
            for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
            {
                const double _Q1_t4 = (vCBQ1 / Q2N5087_vt);
                const double _Q1_t16 = (vEBQ1 / Q2N5087_vt);
                const double _Q1_t42 = math_exp_approx(_Q1_t4);
                const double _Q1_t14 = math_exp_approx(_Q1_t16);
                const double _Q1_t23 = (_Q1_t14 - 1.0);
                const double _Q1_t24 = (_Q1_t42 - 1.0);
                const double _Q1_t31 = (_Q1_t23 / Q2N5087_BetaF);
                const double _Q1_t35 = (_Q1_t42 / Q2N5087_vt);
                const double _Q1_t36 = (_Q1_t35 / Q2N5087_BetaR);
                const double _Q1_t38 = (Q2N5087_Is * _Q1_t42);
                const double _Q1_t41 = (_Q1_t35 + _Q1_t36);
                const double _Q1_t3 = (_Q1_t14 - _Q1_t42);
                const double _Q1_t8 = (_Q1_t24 / Q2N5087_BetaR);
                const double _Q1_t13 = (Q2N5087_Is * _Q1_t14);
                const double _Q1_t15 = (_Q1_t13 * _Q1_t17);
                const double _Q1_t20 = (_Q1_t15 * _Q1_t9);
                const double _Q1_t30 = (_Q1_t31 + _Q1_t8);
                const double _Q1_t37 = (_Q1_t38 * _Q1_t44);
                const double _Q1_t40 = (Q2N5087_Is * _Q1_t41);
                const double _Q1_t43 = (_Q1_t37 * _Q1_t9);
                const double _Q1_t2 = (_Q1_t3 - _Q1_t8);
                const double _Q1_t12 = (_Q1_t13 / Q2N5087_vt);
                const double _Q1_t19 = ((((_Q1_t13 * _Q1_t5) / Q2N5087_vt) * _Q1_t6) - _Q1_t20);
                const double _Q1_t29 = (Q2N5087_Is * _Q1_t30);
                const double _Q1_t34 = ((((_Q1_t40 * _Q1_t5) * _Q1_t6) + _Q1_t43) + 1.0);
                const double _Q1_t39 = (((((_Q1_t38 / Q2N5087_vt) / Q2N5087_BetaR) - _Q1_t40) / gRE) + _Q1_t43);
                const double _Q1_t1 = (Q2N5087_Is * _Q1_t2);
                const double _Q1_t11 = (((((_Q1_t12 / Q2N5087_BetaF) + _Q1_t12) / gRE) + _Q1_t20) + 1.0);
                const double _Q1_t26 = (_Q1_t27 + _Q1_t29);
                const double _Q1_t33 = ((_Q1_t34 * _Q1_t11) + (_Q1_t19 * _Q1_t39));
                const double _Q1_t25 = (_Q1_t26 + _Q1_t32);
                const double _Q1_t7 = (_Q1_t25 * _Q1_t9);
                const double _Q1_t22 = (((_Q1_t29 + _Q1_t1) / gRE) + _Q1_t7);
                const double _Q1_t0 = ((((((zC2 + _Q1_t1) * _Q1_t5) - _Q1_t45) + _Q1_t46) * _Q1_t6) - _Q1_t7);
                const double _Q1_t10 = (vCBQ1 - _Q1_t0);
                const double _Q1_t21 = (_Q1_t22 + vEBQ1);
                const double res_vCBQ1 = (_Q1_t0 - vCBQ1);
                const double delta_vCBQ1 = (-(((_Q1_t10 * _Q1_t11) + (_Q1_t19 * _Q1_t21)) / _Q1_t33));
                const double res_vEBQ1 = (-_Q1_t21);
                const double delta_vEBQ1 = (((_Q1_t10 * _Q1_t39) - (_Q1_t34 * _Q1_t21)) / _Q1_t33);
            
                double residual_norm_sq = 0.0;
                residual_norm_sq += res_vCBQ1 * res_vCBQ1;
                residual_norm_sq += res_vEBQ1 * res_vEBQ1;
                double step_norm_sq = 0.0;
                step_norm_sq += delta_vCBQ1 * delta_vCBQ1;
                step_norm_sq += delta_vEBQ1 * delta_vEBQ1;
            
                vCBQ1 = limit_junction_voltage(vCBQ1 + (delta_vCBQ1), vCBQ1, Q2N5087_vt, vcrit_Q2N5087_vt);
                vEBQ1 = limit_junction_voltage(vEBQ1 + (delta_vEBQ1), vEBQ1, Q2N5087_vt, vcrit_Q2N5087_vt);
            
                if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
                    break;
                
            }

            const double _t1 = math_exp_approx((vEBQ1 / Q2N5087_vt));
            const double _t2 = math_exp_approx((vCBQ1 / Q2N5087_vt));
            const double _t8 = (_t2 - 1.0);
            const double _t7 = (_t8 / Q2N5087_BetaR);
            const double _t0 = (zC2 + (Q2N5087_Is * ((_t1 - _t2) - _t7)));
            const double vo = ((((_t0 * gC2) - (_t3 * zC2)) + _t9) * _t5);
            const double vb = (((((gC1 * vi) - zC1) + (Q2N5087_Is * (((_t1 - 1.0) / Q2N5087_BetaF) + _t7))) + _t10) * _t11);
            const double tC1 = (gC1 * (vi - vb));
            const double vc = ((((_t0 * _t6) - (gC2 * zC2)) + _t12) * _t5);
            const double tC2 = (gC2 * (vc - vo));
            
            zC1 = 2 * tC1 - zC1;
            zC2 = 2 * tC2 - zC2;

            output[ch][n] = vo;
        }
        state[ch].zC1 = zC1;
        state[ch].zC2 = zC2;
        state[ch].vCBQ1 = vCBQ1;
        state[ch].vEBQ1 = vEBQ1;
    }
}

static float reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc)
{
    #define sum(a, b) ((a) + (b))
    #define recip_sum(a, b) ((a) * (b) / ((a) + (b)))
    
    const double Q2N5087_Is = params.Q2N5087_Is;
    const double Q2N5087_vt = params.Q2N5087_vt;
    const double Q2N5087_BetaF = params.Q2N5087_BetaF;
    const double Q2N5087_BetaR = params.Q2N5087_BetaR;
    
    const double VEE = params.VEE;
    
    const double vn = VEE;
    
    const double gC1 = 2.0 * sample_rate * params.C1;
    
    const double gR1 = 1.0 / params.R1;
    
    const double gR2 = 1.0 / params.R2;
    
    const double gRE = 1.0 / params.RE;
    
    const double gRC = 1.0 / params.RC;
    
    const double gC2 = 2.0 * sample_rate * params.C2;
    
    const double gRL = 1.0 / params.RL;
    
    const double vcrit_Q2N5087_vt = Q2N5087_vt * log(Q2N5087_vt / (sqrt(2.0) * Q2N5087_Is));
    
    const double vi = vi_dc;

    double vCBQ1 = 0;
    double vEBQ1 = 0;

    // --- Newton-Raphson solve: Q1
    const double _Q1_t10 = (gR1 + gR2);
    const double _Q1_t15 = (1.0 / 1000000000.0);
    const double _Q1_t20 = (1.0 / (Q2N5087_vt * Q2N5087_BetaF));
    const double _Q1_t31 = (gR1 * VEE);
    const double _Q1_t42 = (1.0 / (Q2N5087_vt * Q2N5087_BetaR));
    const double _Q1_t4 = (1.0 / (gRC + _Q1_t15));
    const double _Q1_t9 = (1.0 / (_Q1_t10 + _Q1_t15));
    const double _Q1_t14 = (1.0 / (gRE + _Q1_t15));
    const double _Q1_t43 = (VEE * gRC);
    for (int newton_iter = 0; newton_iter < 10000; ++newton_iter)
    {
        const double _Q1_t3 = (vEBQ1 / Q2N5087_vt);
        const double _Q1_t7 = math_exp_approx(_Q1_t3);
        const double _Q1_t8 = (vCBQ1 / Q2N5087_vt);
        const double _Q1_t13 = (Q2N5087_Is * _Q1_t7);
        const double _Q1_t17 = (_Q1_t13 / Q2N5087_vt);
        const double _Q1_t27 = math_exp_approx(_Q1_t8);
        const double _Q1_t28 = (_Q1_t27 - 1.0);
        const double _Q1_t30 = (_Q1_t7 - 1.0);
        const double _Q1_t37 = (Q2N5087_Is * _Q1_t27);
        const double _Q1_t39 = (_Q1_t27 / Q2N5087_vt);
        const double _Q1_t40 = (_Q1_t39 / Q2N5087_BetaR);
        const double _Q1_t6 = (_Q1_t30 / Q2N5087_BetaF);
        const double _Q1_t19 = (_Q1_t13 * _Q1_t20);
        const double _Q1_t25 = (_Q1_t28 / Q2N5087_BetaR);
        const double _Q1_t26 = (_Q1_t7 - _Q1_t27);
        const double _Q1_t34 = (_Q1_t39 + _Q1_t40);
        const double _Q1_t35 = (_Q1_t37 * _Q1_t42);
        const double _Q1_t38 = (Q2N5087_Is * _Q1_t34);
        const double _Q1_t41 = (_Q1_t35 * _Q1_t9);
        const double _Q1_t2 = (_Q1_t26 - _Q1_t25);
        const double _Q1_t18 = (_Q1_t19 * _Q1_t9);
        const double _Q1_t24 = (_Q1_t6 + _Q1_t25);
        const double _Q1_t33 = (((_Q1_t38 * _Q1_t4) + _Q1_t41) + 1.0);
        const double _Q1_t36 = (((((_Q1_t37 / Q2N5087_vt) / Q2N5087_BetaR) - _Q1_t38) * _Q1_t14) + _Q1_t41);
        const double _Q1_t1 = (Q2N5087_Is * _Q1_t2);
        const double _Q1_t12 = (((((_Q1_t17 / Q2N5087_BetaF) + _Q1_t17) * _Q1_t14) + _Q1_t18) + 1.0);
        const double _Q1_t16 = ((_Q1_t17 * _Q1_t4) - _Q1_t18);
        const double _Q1_t23 = (Q2N5087_Is * _Q1_t24);
        const double _Q1_t32 = ((_Q1_t33 * _Q1_t12) + (_Q1_t16 * _Q1_t36));
        const double _Q1_t5 = (_Q1_t23 + _Q1_t31);
        const double _Q1_t29 = (_Q1_t5 * _Q1_t9);
        const double _Q1_t0 = (((_Q1_t1 + _Q1_t43) * _Q1_t4) - _Q1_t29);
        const double _Q1_t11 = (vCBQ1 - _Q1_t0);
        const double _Q1_t22 = (((_Q1_t23 + _Q1_t1) * _Q1_t14) + _Q1_t29);
        const double _Q1_t21 = (_Q1_t22 + vEBQ1);
        const double res_vCBQ1 = (_Q1_t0 - vCBQ1);
        const double delta_vCBQ1 = (-(((_Q1_t11 * _Q1_t12) + (_Q1_t16 * _Q1_t21)) / _Q1_t32));
        const double res_vEBQ1 = (-_Q1_t21);
        const double delta_vEBQ1 = (((_Q1_t11 * _Q1_t36) - (_Q1_t33 * _Q1_t21)) / _Q1_t32);
    
        double residual_norm_sq = 0.0;
        residual_norm_sq += res_vCBQ1 * res_vCBQ1;
        residual_norm_sq += res_vEBQ1 * res_vEBQ1;
        double step_norm_sq = 0.0;
        step_norm_sq += delta_vCBQ1 * delta_vCBQ1;
        step_norm_sq += delta_vEBQ1 * delta_vEBQ1;
    
        vCBQ1 = limit_junction_voltage(vCBQ1 + (delta_vCBQ1), vCBQ1, Q2N5087_vt, vcrit_Q2N5087_vt);
        vEBQ1 = limit_junction_voltage(vEBQ1 + (delta_vEBQ1), vEBQ1, Q2N5087_vt, vcrit_Q2N5087_vt);
    
        if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
            break;
        
    }
    const double zC1 = (gC1 * (vi - (((Q2N5087_Is * (((math_exp_approx((vEBQ1 / Q2N5087_vt)) - 1.0) / Q2N5087_BetaF) + ((math_exp_approx((vCBQ1 / Q2N5087_vt)) - 1.0) / Q2N5087_BetaR))) + (gR1 * VEE)) / ((gR1 + gR2) + (1.0 / 1000000000.0)))));
    const double zC2 = ((gC2 * ((Q2N5087_Is * ((math_exp_approx((vEBQ1 / Q2N5087_vt)) - math_exp_approx((vCBQ1 / Q2N5087_vt))) - ((math_exp_approx((vCBQ1 / Q2N5087_vt)) - 1.0) / Q2N5087_BetaR))) + (VEE * gRC))) / (gRC + (1.0 / 1000000000.0)));

    const double vo_dc_out = 0.0;

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].vCBQ1 = vCBQ1;
        state[ch].vEBQ1 = vEBQ1;
        state[ch].zC1 = zC1;
        state[ch].zC2 = zC2;
    }
    return vo_dc_out;
}

