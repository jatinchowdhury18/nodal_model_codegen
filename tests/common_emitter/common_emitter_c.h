// Auto-generated with netlist_codegen version 5c2e3ae.
// Command: netlist_codegen common_emitter.net common_emitter_c.h -lang c -type_name double

#pragma once

#include <math.h>

typedef struct {
    double VCC; // = 9.0e+00
    double C1; // = 1.0e-07
    double R1; // = 4.7e+05
    double R2; // = 1.0e+05
    double RE; // = 2.7e+03
    double RC; // = 1.2e+04
    double Q2N5089_Is; // = 5.0e-14
    double Q2N5089_vt; // = 2.585e-02
    double Q2N5089_BetaF; // = 6.0e+02
    double Q2N5089_BetaR; // = 5.0e+01
    double C2; // = 1.0e-07
    double RL; // = 1.0e+05
} Params;

typedef struct {
    double zC1;
    double zC2;
    double vBCQ1;
    double vBEQ1;
} State;

static double limit_junction_voltage(double v_new, double v_old, double vt, double vcrit) {
    if (v_new > vcrit && fabs(v_new - v_old) > 2 * vt) {
        if (v_old > 0) {
            double arg = 1 + (v_new - v_old) / vt;
            v_new = arg > 0 ? v_old + vt * log(arg) : vcrit;
        } else {
            v_new = vt * log(v_new / vt);
        }
    } else if (v_new < -vcrit && fabs(v_new - v_old) > 2 * vt) {
        if (v_old < 0) {
            double arg = 1 + (v_old - v_new) / vt;
            v_new = arg > 0 ? v_old - vt * log(arg) : -vcrit;
        } else {
            v_new = -vt * log(-v_new / vt);
        }
    }
    return v_new;
}

static const double newton_tol_sq = 1.0e-05;
static const int newton_max_iter = 20;

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    #define sum(a, b) ((a) + (b))
    #define recip_sum(a, b) ((a) * (b) / ((a) + (b)))
    
    const double Q2N5089_Is = params.Q2N5089_Is;
    const double Q2N5089_vt = params.Q2N5089_vt;
    const double Q2N5089_BetaF = params.Q2N5089_BetaF;
    const double Q2N5089_BetaR = params.Q2N5089_BetaR;
    
    const double VCC = params.VCC;
    
    const double vp = VCC;
    
    const double gC1 = 2.0 * sample_rate * params.C1;
    
    const double gR1 = 1.0 / params.R1;
    
    const double gR2 = 1.0 / params.R2;
    
    const double gRE = 1.0 / params.RE;
    
    const double gRC = 1.0 / params.RC;
    
    const double gC2 = 2.0 * sample_rate * params.C2;
    
    const double gRL = 1.0 / params.RL;
    
    const double vcrit_Q2N5089_vt = Q2N5089_vt * log(Q2N5089_vt / (sqrt(2.0) * Q2N5089_Is));
    
    const double _t0 = (gRC + gC2);
    const double _t6 = (VCC * gRC);
    const double _t8 = (gC2 + gRL);
    const double _t7 = (1.0 / ((_t0 * _t8) - (gC2 * gC2)));
    const double _t9 = (_t6 * gC2);
    const double _t10 = (gR1 * VCC);
    const double _t11 = (1.0 / ((gC1 + gR1) + gR2));
    const double _t12 = (_t6 * _t8);
    for (int ch = 0; ch < num_channels; ++ch)
    {
        double zC1 = state[ch].zC1;
        double zC2 = state[ch].zC2;
        double vBCQ1 = state[ch].vBCQ1;
        double vBEQ1 = state[ch].vBEQ1;
        for (int n = 0; n < num_samples; ++n)
        {
            const double vi = input[ch][n];

            // --- Newton-Raphson solve: Q1
            const double _Q1_t3 = (gR1 * VCC);
            const double _Q1_t16 = (gC1 * vi);
            const double _Q1_t18 = (gC1 + gR1);
            const double _Q1_t22 = (gC2 + gRL);
            const double _Q1_t23 = (1.0 / (((gRC + gC2) * _Q1_t22) - (gC2 * gC2)));
            const double _Q1_t29 = (1.0 / (Q2N5089_vt * Q2N5089_BetaF));
            const double _Q1_t39 = (1.0 / (Q2N5089_vt * Q2N5089_BetaR));
            const double _Q1_t15 = (_Q1_t16 - zC1);
            const double _Q1_t17 = (1.0 / (_Q1_t18 + gR2));
            const double _Q1_t45 = (gC2 * zC2);
            const double _Q1_t46 = ((VCC * gRC) * _Q1_t22);
            for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
            {
                const double _Q1_t10 = (vBEQ1 / Q2N5089_vt);
                const double _Q1_t14 = (vBCQ1 / Q2N5089_vt);
                const double _Q1_t9 = exp(_Q1_t10);
                const double _Q1_t13 = exp(_Q1_t14);
                const double _Q1_t21 = (_Q1_t9 - _Q1_t13);
                const double _Q1_t28 = (Q2N5089_Is * _Q1_t9);
                const double _Q1_t30 = (_Q1_t28 / Q2N5089_vt);
                const double _Q1_t38 = (Q2N5089_Is * _Q1_t13);
                const double _Q1_t42 = (_Q1_t13 / Q2N5089_vt);
                const double _Q1_t43 = (_Q1_t42 / Q2N5089_BetaR);
                const double _Q1_t8 = (_Q1_t9 - 1.0);
                const double _Q1_t12 = (_Q1_t13 - 1.0);
                const double _Q1_t27 = (_Q1_t28 * _Q1_t29);
                const double _Q1_t37 = (_Q1_t38 * _Q1_t39);
                const double _Q1_t41 = (_Q1_t42 + _Q1_t43);
                const double _Q1_t7 = (_Q1_t8 / Q2N5089_BetaF);
                const double _Q1_t11 = (_Q1_t12 / Q2N5089_BetaR);
                const double _Q1_t20 = (_Q1_t21 - _Q1_t11);
                const double _Q1_t26 = (_Q1_t27 * _Q1_t17);
                const double _Q1_t31 = ((((_Q1_t28 * _Q1_t22) / Q2N5089_vt) * _Q1_t23) - _Q1_t26);
                const double _Q1_t36 = (_Q1_t37 * _Q1_t17);
                const double _Q1_t40 = (Q2N5089_Is * _Q1_t41);
                const double _Q1_t44 = (_Q1_t36 + ((((_Q1_t38 / Q2N5089_vt) / Q2N5089_BetaR) - _Q1_t40) / gRE));
                const double _Q1_t6 = (_Q1_t7 + _Q1_t11);
                const double _Q1_t19 = (Q2N5089_Is * _Q1_t20);
                const double _Q1_t25 = ((_Q1_t26 + (((_Q1_t30 / Q2N5089_BetaF) + _Q1_t30) / gRE)) + 1.0);
                const double _Q1_t35 = ((_Q1_t36 + ((_Q1_t40 * _Q1_t22) * _Q1_t23)) + 1.0);
                const double _Q1_t5 = (Q2N5089_Is * _Q1_t6);
                const double _Q1_t34 = ((_Q1_t35 * _Q1_t25) + (_Q1_t31 * _Q1_t44));
                const double _Q1_t4 = (_Q1_t5 - _Q1_t15);
                const double _Q1_t2 = (_Q1_t3 - _Q1_t4);
                const double _Q1_t1 = (_Q1_t2 * _Q1_t17);
                const double _Q1_t33 = (_Q1_t1 - ((_Q1_t5 + _Q1_t19) / gRE));
                const double _Q1_t0 = (_Q1_t1 + (((((_Q1_t19 - zC2) * _Q1_t22) + _Q1_t45) - _Q1_t46) * _Q1_t23));
                const double _Q1_t24 = (vBCQ1 - _Q1_t0);
                const double _Q1_t32 = (vBEQ1 - _Q1_t33);
                const double res_vBCQ1 = (_Q1_t0 - vBCQ1);
                const double delta_vBCQ1 = (-(((_Q1_t24 * _Q1_t25) + (_Q1_t31 * _Q1_t32)) / _Q1_t34));
                const double res_vBEQ1 = (_Q1_t33 - vBEQ1);
                const double delta_vBEQ1 = (((_Q1_t24 * _Q1_t44) - (_Q1_t35 * _Q1_t32)) / _Q1_t34);
            
                double residual_norm_sq = 0.0;
                residual_norm_sq += res_vBCQ1 * res_vBCQ1;
                residual_norm_sq += res_vBEQ1 * res_vBEQ1;
                double step_norm_sq = 0.0;
                step_norm_sq += delta_vBCQ1 * delta_vBCQ1;
                step_norm_sq += delta_vBEQ1 * delta_vBEQ1;
            
                vBCQ1 = limit_junction_voltage(vBCQ1 + delta_vBCQ1, vBCQ1, Q2N5089_vt, vcrit_Q2N5089_vt);
                vBEQ1 = limit_junction_voltage(vBEQ1 + delta_vBEQ1, vBEQ1, Q2N5089_vt, vcrit_Q2N5089_vt);
            
                if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
                    break;
                
            }

            const double _t2 = exp((vBEQ1 / Q2N5089_vt));
            const double _t3 = exp((vBCQ1 / Q2N5089_vt));
            const double _t5 = (_t3 - 1.0);
            const double _t4 = (_t5 / Q2N5089_BetaR);
            const double _t1 = ((Q2N5089_Is * ((_t2 - _t3) - _t4)) - zC2);
            const double vo = (-((((_t0 * zC2) + (_t1 * gC2)) - _t9) * _t7));
            const double vb = ((_t10 - ((Q2N5089_Is * (((_t2 - 1.0) / Q2N5089_BetaF) + _t4)) - ((gC1 * vi) - zC1))) * _t11);
            const double tC1 = (gC1 * (vi - vb));
            const double vc = (-((((_t1 * _t8) + (gC2 * zC2)) - _t12) * _t7));
            const double tC2 = (gC2 * (vc - vo));
            
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

static void reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc)
{
    #define sum(a, b) ((a) + (b))
    #define recip_sum(a, b) ((a) * (b) / ((a) + (b)))
    
    const double Q2N5089_Is = params.Q2N5089_Is;
    const double Q2N5089_vt = params.Q2N5089_vt;
    const double Q2N5089_BetaF = params.Q2N5089_BetaF;
    const double Q2N5089_BetaR = params.Q2N5089_BetaR;
    
    const double VCC = params.VCC;
    
    const double vp = VCC;
    
    const double gC1 = 2.0 * sample_rate * params.C1;
    
    const double gR1 = 1.0 / params.R1;
    
    const double gR2 = 1.0 / params.R2;
    
    const double gRE = 1.0 / params.RE;
    
    const double gRC = 1.0 / params.RC;
    
    const double gC2 = 2.0 * sample_rate * params.C2;
    
    const double gRL = 1.0 / params.RL;
    
    const double vcrit_Q2N5089_vt = Q2N5089_vt * log(Q2N5089_vt / (sqrt(2.0) * Q2N5089_Is));
    
    const double vi = vi_dc;

    double vBCQ1 = 0;
    double vBEQ1 = 0;

    // --- Newton-Raphson solve: Q1
    const double _Q1_t3 = (gR1 * VCC);
    const double _Q1_t15 = (gR1 + gR2);
    const double _Q1_t16 = (1.0 / 1000000000.0);
    const double _Q1_t20 = (1.0 / (gRC + _Q1_t16));
    const double _Q1_t26 = (1.0 / (Q2N5089_vt * Q2N5089_BetaF));
    const double _Q1_t28 = (1.0 / (gRE + _Q1_t16));
    const double _Q1_t37 = (1.0 / (Q2N5089_vt * Q2N5089_BetaR));
    const double _Q1_t14 = (1.0 / (_Q1_t15 + _Q1_t16));
    const double _Q1_t43 = (VCC * gRC);
    for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
    {
        const double _Q1_t9 = (vBEQ1 / Q2N5089_vt);
        const double _Q1_t13 = (vBCQ1 / Q2N5089_vt);
        const double _Q1_t8 = exp(_Q1_t9);
        const double _Q1_t12 = exp(_Q1_t13);
        const double _Q1_t19 = (_Q1_t8 - _Q1_t12);
        const double _Q1_t25 = (Q2N5089_Is * _Q1_t8);
        const double _Q1_t27 = (_Q1_t25 / Q2N5089_vt);
        const double _Q1_t36 = (Q2N5089_Is * _Q1_t12);
        const double _Q1_t40 = (_Q1_t12 / Q2N5089_vt);
        const double _Q1_t41 = (_Q1_t40 / Q2N5089_BetaR);
        const double _Q1_t7 = (_Q1_t8 - 1.0);
        const double _Q1_t11 = (_Q1_t12 - 1.0);
        const double _Q1_t24 = (_Q1_t25 * _Q1_t26);
        const double _Q1_t35 = (_Q1_t36 * _Q1_t37);
        const double _Q1_t39 = (_Q1_t40 + _Q1_t41);
        const double _Q1_t6 = (_Q1_t7 / Q2N5089_BetaF);
        const double _Q1_t10 = (_Q1_t11 / Q2N5089_BetaR);
        const double _Q1_t18 = (_Q1_t19 - _Q1_t10);
        const double _Q1_t23 = (_Q1_t24 * _Q1_t14);
        const double _Q1_t29 = ((_Q1_t27 * _Q1_t20) - _Q1_t23);
        const double _Q1_t34 = (_Q1_t35 * _Q1_t14);
        const double _Q1_t38 = (Q2N5089_Is * _Q1_t39);
        const double _Q1_t42 = (_Q1_t34 + ((((_Q1_t36 / Q2N5089_vt) / Q2N5089_BetaR) - _Q1_t38) * _Q1_t28));
        const double _Q1_t5 = (_Q1_t6 + _Q1_t10);
        const double _Q1_t17 = (Q2N5089_Is * _Q1_t18);
        const double _Q1_t22 = ((_Q1_t23 + (((_Q1_t27 / Q2N5089_BetaF) + _Q1_t27) * _Q1_t28)) + 1.0);
        const double _Q1_t33 = ((_Q1_t34 + (_Q1_t38 * _Q1_t20)) + 1.0);
        const double _Q1_t4 = (Q2N5089_Is * _Q1_t5);
        const double _Q1_t32 = ((_Q1_t33 * _Q1_t22) + (_Q1_t29 * _Q1_t42));
        const double _Q1_t2 = (_Q1_t3 - _Q1_t4);
        const double _Q1_t1 = (_Q1_t2 * _Q1_t14);
        const double _Q1_t31 = (_Q1_t1 - ((_Q1_t4 + _Q1_t17) * _Q1_t28));
        const double _Q1_t0 = (_Q1_t1 + ((_Q1_t17 - _Q1_t43) * _Q1_t20));
        const double _Q1_t21 = (vBCQ1 - _Q1_t0);
        const double _Q1_t30 = (vBEQ1 - _Q1_t31);
        const double res_vBCQ1 = (_Q1_t0 - vBCQ1);
        const double delta_vBCQ1 = (-(((_Q1_t21 * _Q1_t22) + (_Q1_t29 * _Q1_t30)) / _Q1_t32));
        const double res_vBEQ1 = (_Q1_t31 - vBEQ1);
        const double delta_vBEQ1 = (((_Q1_t21 * _Q1_t42) - (_Q1_t33 * _Q1_t30)) / _Q1_t32);
    
        double residual_norm_sq = 0.0;
        residual_norm_sq += res_vBCQ1 * res_vBCQ1;
        residual_norm_sq += res_vBEQ1 * res_vBEQ1;
        double step_norm_sq = 0.0;
        step_norm_sq += delta_vBCQ1 * delta_vBCQ1;
        step_norm_sq += delta_vBEQ1 * delta_vBEQ1;
    
        vBCQ1 = limit_junction_voltage(vBCQ1 + delta_vBCQ1, vBCQ1, Q2N5089_vt, vcrit_Q2N5089_vt);
        vBEQ1 = limit_junction_voltage(vBEQ1 + delta_vBEQ1, vBEQ1, Q2N5089_vt, vcrit_Q2N5089_vt);
    
        if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
            break;
        
    }
    const double zC1 = (gC1 * (vi - (((gR1 * VCC) - (Q2N5089_Is * (((exp((vBEQ1 / Q2N5089_vt)) - 1.0) / Q2N5089_BetaF) + ((exp((vBCQ1 / Q2N5089_vt)) - 1.0) / Q2N5089_BetaR)))) / ((gR1 + gR2) + (1.0 / 1000000000.0)))));
    const double zC2 = (-((gC2 * ((Q2N5089_Is * ((exp((vBEQ1 / Q2N5089_vt)) - exp((vBCQ1 / Q2N5089_vt))) - ((exp((vBCQ1 / Q2N5089_vt)) - 1.0) / Q2N5089_BetaR))) - (VCC * gRC))) / (gRC + (1.0 / 1000000000.0))));

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].vBCQ1 = vBCQ1;
        state[ch].vBEQ1 = vBEQ1;
        state[ch].zC1 = zC1;
        state[ch].zC2 = zC2;
    }
}
