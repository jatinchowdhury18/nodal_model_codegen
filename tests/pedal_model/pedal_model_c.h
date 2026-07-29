// Auto-generated with netlist_codegen version a65dc78.
// Command: netlist_codegen pedal_model.net pedal_model_c.h -lang c -type_name double

#pragma once

#include <math.h>

typedef struct {
    double Vpp; // = 9.0e+00
    double Vp; // = 4.5e+00
    double R5; // = 1.0e+06
    double RL; // = 1.0e+05
    double R4; // = 1.0e+06
    double _2N5485_Beta; // = 2.0e-03
    double _2N5485_vp; // = -1.0e+00
    double C4; // = 1.0e-09
    double Eamp_Aol; // = 3.0e+05
    double Eamp_Vos; // = 2.0e-03
    double Eamp_Ios; // = 1.0e-09
    double Eamp_Ibias; // = 7.0e-09
    double Eamp_Rin; // = 4.0e+07
    double Eamp_Rcomp; // = 1.0e+00
    double Eamp_Ccomp; // = 3.0e-11
    double Eamp_Rout; // = 2.0e+02
    double R11; // = 1.0e+06
    double C12; // = 2.2e-08
    double D1N914_Is; // = 2.52e-09
    double D1N914_vt; // = 4.52892e-02
    double R7; // = 5.6e+02
    double C6; // = 4.7e-06
    double C13; // = 1.0e-05
    double R6; // = 1.0e+03
    double C3; // = 2.2e-08
    double C11; // = 3.3e-09
    double R8; // = 4.7e+01
    double C7; // = 2.2e-06
    double R13; // = 1.0e+04
    double Rd; // = 1.0e+05
    double C5; // = 1.0e-10
    double R9; // = 1.0e+03
    double C9; // = 4.7e-06
    double Rf; // = 1.0e+05
    double R10; // = 1.6e+03
} Params;

typedef struct {
    double zC4;
    double zEamp_Ccomp;
    double zC12;
    double zR7C6;
    double zC13;
    double zR6C3;
    double zC11;
    double zR8C7;
    double zRdC5;
    double zR9C9;
    double vGSJ1;
    double vD1D2;
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

static double limit_jfet_vgs(double v_new, double vp) {
    if (v_new < vp) return vp;
    return v_new;
}

static const double newton_tol_sq = 1.0e-06;
static const int newton_max_iter = 20;

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    #define sum(a, b) ((a) + (b))
    #define recip_sum(a, b) ((a) * (b) / ((a) + (b)))
    
    const double _2N5485_Beta = params._2N5485_Beta;
    const double _2N5485_vp = params._2N5485_vp;
    const double Eamp_Aol = params.Eamp_Aol;
    const double Eamp_Vos = params.Eamp_Vos;
    const double Eamp_Ios = params.Eamp_Ios;
    const double Eamp_Ibias = params.Eamp_Ibias;
    const double D1N914_Is = params.D1N914_Is;
    const double D1N914_vt = params.D1N914_vt;
    
    const double Vpp = params.Vpp;
    
    const double vpp = Vpp;
    
    const double Vp = params.Vp;
    
    const double vp = Vp;
    
    const double gR5 = 1.0 / params.R5;
    
    const double gRL = 1.0 / params.RL;
    
    const double gR4 = 1.0 / params.R4;
    
    const double gC4 = 2.0 * sample_rate * params.C4;
    
    const double gEamp_Rin = 1.0 / params.Eamp_Rin;
    
    const double gEamp_Rcomp = 1.0 / params.Eamp_Rcomp;
    
    const double gEamp_Ccomp = 2.0 * sample_rate * params.Eamp_Ccomp;
    
    const double gEamp_Rout = 1.0 / params.Eamp_Rout;
    
    const double gR11 = 1.0 / params.R11;
    
    const double gC12 = 2.0 * sample_rate * params.C12;
    
    const double R7 = params.R7;
    const double C6 = params.C6;
    const double gR7C6 = (2.0 * sample_rate * C6) / (1.0 + 2.0 * sample_rate * R7 * C6);
    const double gnR7C6 = gR7C6 * (2.0 * sample_rate * R7 * C6 - 1.0) / (2.0 * sample_rate * C6);
    
    const double gC13 = 2.0 * sample_rate * params.C13;
    
    const double R6 = params.R6;
    const double C3 = params.C3;
    const double gR6C3 = (2.0 * sample_rate * C3) / (1.0 + 2.0 * sample_rate * R6 * C3);
    const double gnR6C3 = gR6C3 * (2.0 * sample_rate * R6 * C3 - 1.0) / (2.0 * sample_rate * C3);
    
    const double gC11 = 2.0 * sample_rate * params.C11;
    
    const double R8 = params.R8;
    const double C7 = params.C7;
    const double gR8C7 = (2.0 * sample_rate * C7) / (1.0 + 2.0 * sample_rate * R8 * C7);
    const double gnR8C7 = gR8C7 * (2.0 * sample_rate * R8 * C7 - 1.0) / (2.0 * sample_rate * C7);
    
    const double gR13 = 1.0 / params.R13;
    
    const double Rd = params.Rd;
    const double C5 = params.C5;
    const double gRdC5 = 2.0 * sample_rate * C5 + (1.0 / Rd);
    const double gzRdC5 = 4.0 * sample_rate * C5;
    
    const double R9 = params.R9;
    const double C9 = params.C9;
    const double gR9C9 = (2.0 * sample_rate * C9) / (1.0 + 2.0 * sample_rate * R9 * C9);
    const double gnR9C9 = gR9C9 * (2.0 * sample_rate * R9 * C9 - 1.0) / (2.0 * sample_rate * C9);
    
    const double RfR10 = sum(params.Rf, params.R10);
    const double gRfR10 = 1.0 / RfR10;
    
    const double vcrit_D1N914_vt = D1N914_vt * log(D1N914_vt / (sqrt(2.0) * D1N914_Is));
    
    const double _t1 = (gR11 + gC12);
    const double _t3 = (gC13 + gR13);
    const double _t8 = ((gEamp_Rcomp + gEamp_Ccomp) + gEamp_Rout);
    const double _t10 = ((gC12 + gC11) + gRfR10);
    const double _t11 = ((gEamp_Rout + gRdC5) + gR9C9);
    const double _t15 = (gR9C9 + gRfR10);
    const double _t21 = ((gC4 + gEamp_Rin) + gR6C3);
    const double _t25 = (((gEamp_Rin + gR7C6) + gR8C7) + gRdC5);
    const double _t33 = (gC12 * _t15);
    const double _t38 = (gEamp_Rcomp * _t33);
    const double _t39 = (gEamp_Rout * _t33);
    const double _t42 = (gC12 * gR9C9);
    const double _t57 = (gRL + gC13);
    const double _t60 = (Eamp_Ios / 2.0);
    const double _t66 = (Eamp_Aol * Eamp_Vos);
    const double _t85 = (gC12 * gRfR10);
    const double _t93 = (_t66 * _t38);
    const double _t120 = (gEamp_Rout * _t85);
    const double _t123 = (gEamp_Rcomp * _t85);
    const double _t125 = (_t25 * _t120);
    const double _t137 = (gEamp_Rcomp * _t42);
    const double _t142 = (Eamp_Aol * _t137);
    const double _t150 = (_t25 * _t142);
    const double _t151 = (gEamp_Rin * _t142);
    const double _t156 = (gRfR10 * gR9C9);
    const double _t168 = (gEamp_Rcomp * _t156);
    const double _t173 = (Eamp_Aol * _t168);
    const double _t191 = (_t25 * _t173);
    const double _t192 = (gEamp_Rin * _t173);
    const double _t9 = (_t10 * gR9C9);
    const double _t14 = ((_t10 * _t15) - (gRfR10 * gRfR10));
    const double _t19 = (gEamp_Rcomp * _t14);
    const double _t20 = (gEamp_Rout * _t14);
    const double _t32 = (_t8 * _t33);
    const double _t34 = (gRdC5 * _t32);
    const double _t37 = (Eamp_Aol * _t38);
    const double _t41 = (_t8 * _t42);
    const double _t44 = (gEamp_Rin * _t32);
    const double _t46 = (gEamp_Rin * _t41);
    const double _t65 = (_t66 * _t19);
    const double _t84 = (_t8 * _t85);
    const double _t106 = (gEamp_Rcomp * _t9);
    const double _t115 = (_t66 * _t106);
    const double _t119 = (gEamp_Rin * _t120);
    const double _t122 = (Eamp_Aol * _t123);
    const double _t136 = (_t66 * _t137);
    const double _t147 = (gEamp_Rin * _t37);
    const double _t148 = (_t25 * _t37);
    const double _t149 = (_t150 - _t151);
    const double _t155 = (_t8 * _t156);
    const double _t167 = (_t66 * _t168);
    const double _t183 = (_t25 * _t155);
    const double _t190 = (_t191 - _t192);
    const double _t202 = (_t25 * _t84);
    const double _t203 = (gEamp_Rin * _t84);
    const double _t7 = (_t8 * _t9);
    const double _t13 = (_t8 * _t14);
    const double _t18 = (Eamp_Aol * _t19);
    const double _t23 = (gRdC5 * _t13);
    const double _t24 = (_t25 * _t13);
    const double _t27 = (_t25 * _t7);
    const double _t31 = (_t25 * _t32);
    const double _t36 = (gRdC5 * _t37);
    const double _t40 = (_t25 * _t41);
    const double _t45 = (_t36 - (gEamp_Rin * _t39));
    const double _t83 = (gRdC5 * _t84);
    const double _t101 = (gEamp_Rin * _t18);
    const double _t102 = (_t25 * _t18);
    const double _t105 = (Eamp_Aol * _t106);
    const double _t107 = (_t25 * _t105);
    const double _t121 = (gRdC5 * _t122);
    const double _t124 = (_t125 - _t121);
    const double _t146 = (_t147 - _t148);
    const double _t154 = (gEamp_Rin * _t155);
    const double _t188 = (gRdC5 * _t154);
    const double _t189 = (gEamp_Rout * _t190);
    const double _t201 = ((_t21 * (gR9C9 * _t202)) - (gEamp_Rin * (gR9C9 * _t203)));
    const double _t6 = (gEamp_Rin * _t7);
    const double _t12 = (gEamp_Rin * _t13);
    const double _t17 = (gRdC5 * _t18);
    const double _t26 = (_t17 - (_t25 * _t20));
    const double _t35 = (_t36 - (_t25 * _t39));
    const double _t43 = (((_t11 * _t44) + (gEamp_Rout * _t45)) - (gR9C9 * _t46));
    const double _t82 = (gR9C9 * _t83);
    const double _t100 = (_t101 - _t102);
    const double _t104 = (gEamp_Rin * _t105);
    const double _t118 = (_t119 - _t121);
    const double _t187 = (_t188 + _t189);
    const double _t196 = (_t21 * _t82);
    const double _t215 = ((_t21 * (((gRdC5 * _t83) - (_t11 * _t202)) + (gEamp_Rout * _t124))) - (gEamp_Rin * ((gEamp_Rout * _t118) - (_t11 * _t203))));
    const double _t16 = (_t17 - (gEamp_Rin * _t20));
    const double _t22 = ((((gRdC5 * _t23) - (_t11 * _t24)) - (gEamp_Rout * _t26)) + (gR9C9 * _t27));
    const double _t30 = ((((_t11 * _t31) - (gRdC5 * _t34)) + (gEamp_Rout * _t35)) - (gR9C9 * _t40));
    const double _t81 = (gEamp_Rin * _t82);
    const double _t103 = (_t104 - _t107);
    const double _t117 = ((gEamp_Rin * (gR9C9 * _t118)) - (_t21 * (gR9C9 * _t124)));
    const double _t5 = ((gR9C9 * _t6) - ((_t11 * _t12) + (gEamp_Rout * _t16)));
    const double _t29 = ((_t21 * _t30) - (gEamp_Rin * _t43));
    const double _t56 = (gC13 * _t29);
    const double _t4 = ((gEamp_Rin * _t5) - (_t21 * _t22));
    const double _t28 = (_t3 * _t29);
    const double _t55 = (gC13 * _t4);
    const double _t2 = (_t3 * _t4);
    const double _t54 = ((_t1 * _t55) - (gC12 * _t56));
    const double _t0 = ((_t1 * _t2) - (gC12 * _t28));
    const double _t53 = (1.0 / ((gC13 * _t54) - (_t57 * _t0)));
    const double _t216 = (gC13 * _t81);
    const double _t217 = (_t3 * _t81);
    const double _t218 = (gC13 * _t117);
    const double _t219 = (_t3 * _t117);
    const double _t220 = (gC13 * _t196);
    const double _t221 = (_t3 * _t196);
    const double _t222 = (gC13 * _t201);
    const double _t223 = (_t3 * _t201);
    const double _t224 = (gC13 * _t215);
    const double _t225 = (_t3 * _t215);
    const double _t226 = (1.0 / 1000.0);
    const double _t227 = (Eamp_Ibias - _t60);
    const double _t228 = (Eamp_Ibias + _t60);
    const double _t229 = ((gEamp_Rout * _t146) - (gRdC5 * _t44));
    const double _t230 = ((gRdC5 * _t12) - (gEamp_Rout * _t100));
    const double _t231 = ((gEamp_Rout * _t103) - (gRdC5 * _t6));
    const double _t232 = (((_t11 * _t100) + (gRdC5 * _t16)) - (gR9C9 * _t103));
    const double _t233 = ((gRdC5 * _t46) + (gEamp_Rout * _t149));
    const double _t234 = (((_t11 * _t146) + (gRdC5 * _t45)) + (gR9C9 * _t149));
    for (int ch = 0; ch < num_channels; ++ch)
    {
        double zC4 = state[ch].zC4;
        double zEamp_Ccomp = state[ch].zEamp_Ccomp;
        double zC12 = state[ch].zC12;
        double zR7C6 = state[ch].zR7C6;
        double zC13 = state[ch].zC13;
        double zR6C3 = state[ch].zR6C3;
        double zC11 = state[ch].zC11;
        double zR8C7 = state[ch].zR8C7;
        double zRdC5 = state[ch].zRdC5;
        double zR9C9 = state[ch].zR9C9;
        double vGSJ1 = state[ch].vGSJ1;
        double vD1D2 = state[ch].vD1D2;
        for (int n = 0; n < num_samples; ++n)
        {
            const double vi = input[ch][n];

            // --- Newton-Raphson solve: J1_D1D2
            const double _J1_D1D2_t1 = (gRL + gC13);
            const double _J1_D1D2_t3 = (gC13 + gR13);
            const double _J1_D1D2_t11 = (gEamp_Rcomp + gEamp_Ccomp);
            const double _J1_D1D2_t14 = (gC12 + gC11);
            const double _J1_D1D2_t18 = (gEamp_Rout + gRdC5);
            const double _J1_D1D2_t23 = (gR9C9 + gRfR10);
            const double _J1_D1D2_t24 = (gRfR10 * gRfR10);
            const double _J1_D1D2_t34 = (gC4 + gEamp_Rin);
            const double _J1_D1D2_t44 = (gEamp_Rin + gR7C6);
            const double _J1_D1D2_t55 = (Eamp_Ios / 2.0);
            const double _J1_D1D2_t58 = (gR6C3 * vi);
            const double _J1_D1D2_t63 = (gRfR10 * gR9C9);
            const double _J1_D1D2_t68 = (gEamp_Rcomp * _J1_D1D2_t63);
            const double _J1_D1D2_t76 = (zRdC5 - zR9C9);
            const double _J1_D1D2_t83 = (zC12 + zC11);
            const double _J1_D1D2_t95 = (Eamp_Ibias - _J1_D1D2_t55);
            const double _J1_D1D2_t97 = (zR7C6 + zR8C7);
            const double _J1_D1D2_t109 = (Eamp_Aol * Eamp_Vos);
            const double _J1_D1D2_t111 = (zEamp_Ccomp * _J1_D1D2_t63);
            const double _J1_D1D2_t116 = (_J1_D1D2_t83 * gR9C9);
            const double _J1_D1D2_t135 = (gR11 + gC12);
            const double _J1_D1D2_t146 = (gC12 * _J1_D1D2_t23);
            const double _J1_D1D2_t153 = (gEamp_Rcomp * _J1_D1D2_t146);
            const double _J1_D1D2_t155 = (gEamp_Rout * _J1_D1D2_t146);
            const double _J1_D1D2_t159 = (gC12 * gR9C9);
            const double _J1_D1D2_t167 = (gEamp_Rin * _J1_D1D2_t155);
            const double _J1_D1D2_t180 = (1.0 / 1000.0);
            const double _J1_D1D2_t194 = (1.0 / D1N914_vt);
            const double _J1_D1D2_t271 = (_J1_D1D2_t83 * gRfR10);
            const double _J1_D1D2_t331 = (zEamp_Ccomp * _J1_D1D2_t159);
            const double _J1_D1D2_t333 = (gEamp_Rcomp * _J1_D1D2_t159);
            const double _J1_D1D2_t338 = (Eamp_Aol * _J1_D1D2_t333);
            const double _J1_D1D2_t356 = (gEamp_Rin * _J1_D1D2_t338);
            const double _J1_D1D2_t364 = (gC12 * gRfR10);
            const double _J1_D1D2_t370 = (gEamp_Rout * _J1_D1D2_t364);
            const double _J1_D1D2_t373 = (gEamp_Rcomp * _J1_D1D2_t364);
            const double _J1_D1D2_t378 = (gEamp_Rin * _J1_D1D2_t370);
            const double _J1_D1D2_t10 = (_J1_D1D2_t11 + gEamp_Rout);
            const double _J1_D1D2_t13 = (_J1_D1D2_t14 + gRfR10);
            const double _J1_D1D2_t17 = (_J1_D1D2_t18 + gR9C9);
            const double _J1_D1D2_t22 = (_J1_D1D2_t13 * _J1_D1D2_t23);
            const double _J1_D1D2_t33 = (_J1_D1D2_t34 + gR6C3);
            const double _J1_D1D2_t43 = (_J1_D1D2_t44 + gR8C7);
            const double _J1_D1D2_t54 = (Eamp_Ibias + _J1_D1D2_t55);
            const double _J1_D1D2_t57 = (_J1_D1D2_t58 + zR6C3);
            const double _J1_D1D2_t62 = (_J1_D1D2_t10 * _J1_D1D2_t63);
            const double _J1_D1D2_t67 = (Eamp_Aol * _J1_D1D2_t68);
            const double _J1_D1D2_t69 = (gEamp_Rin * _J1_D1D2_t67);
            const double _J1_D1D2_t82 = (_J1_D1D2_t83 * _J1_D1D2_t23);
            const double _J1_D1D2_t96 = (_J1_D1D2_t97 + zRdC5);
            const double _J1_D1D2_t108 = (_J1_D1D2_t109 * _J1_D1D2_t68);
            const double _J1_D1D2_t115 = (_J1_D1D2_t10 * _J1_D1D2_t116);
            const double _J1_D1D2_t129 = (gEamp_Rin * _J1_D1D2_t115);
            const double _J1_D1D2_t145 = (_J1_D1D2_t10 * _J1_D1D2_t146);
            const double _J1_D1D2_t148 = (gRdC5 * _J1_D1D2_t145);
            const double _J1_D1D2_t152 = (Eamp_Aol * _J1_D1D2_t153);
            const double _J1_D1D2_t158 = (_J1_D1D2_t10 * _J1_D1D2_t159);
            const double _J1_D1D2_t164 = (gEamp_Rin * _J1_D1D2_t145);
            const double _J1_D1D2_t169 = (gEamp_Rin * _J1_D1D2_t158);
            const double _J1_D1D2_t332 = (_J1_D1D2_t109 * _J1_D1D2_t333);
            const double _J1_D1D2_t349 = (_J1_D1D2_t76 * _J1_D1D2_t169);
            const double _J1_D1D2_t352 = (gRdC5 * _J1_D1D2_t169);
            const double _J1_D1D2_t363 = (_J1_D1D2_t10 * _J1_D1D2_t364);
            const double _J1_D1D2_t372 = (Eamp_Aol * _J1_D1D2_t373);
            const double _J1_D1D2_t380 = (gEamp_Rin * _J1_D1D2_t363);
            const double _J1_D1D2_t12 = (_J1_D1D2_t13 * gR9C9);
            const double _J1_D1D2_t21 = (_J1_D1D2_t22 - _J1_D1D2_t24);
            const double _J1_D1D2_t29 = (gEamp_Rcomp * _J1_D1D2_t21);
            const double _J1_D1D2_t31 = (gEamp_Rout * _J1_D1D2_t21);
            const double _J1_D1D2_t42 = (_J1_D1D2_t43 + gRdC5);
            const double _J1_D1D2_t47 = (_J1_D1D2_t42 * _J1_D1D2_t31);
            const double _J1_D1D2_t56 = (zC4 + _J1_D1D2_t57);
            const double _J1_D1D2_t61 = (gEamp_Rin * _J1_D1D2_t62);
            const double _J1_D1D2_t66 = (_J1_D1D2_t42 * _J1_D1D2_t67);
            const double _J1_D1D2_t77 = (_J1_D1D2_t42 * _J1_D1D2_t62);
            const double _J1_D1D2_t94 = (_J1_D1D2_t95 - _J1_D1D2_t96);
            const double _J1_D1D2_t102 = (_J1_D1D2_t94 * _J1_D1D2_t67);
            const double _J1_D1D2_t114 = (_J1_D1D2_t42 * _J1_D1D2_t115);
            const double _J1_D1D2_t121 = (_J1_D1D2_t76 * _J1_D1D2_t61);
            const double _J1_D1D2_t128 = (gR9C9 * _J1_D1D2_t129);
            const double _J1_D1D2_t144 = (_J1_D1D2_t42 * _J1_D1D2_t145);
            const double _J1_D1D2_t147 = (gRdC5 * _J1_D1D2_t148);
            const double _J1_D1D2_t151 = (gRdC5 * _J1_D1D2_t152);
            const double _J1_D1D2_t154 = (_J1_D1D2_t42 * _J1_D1D2_t155);
            const double _J1_D1D2_t157 = (_J1_D1D2_t42 * _J1_D1D2_t158);
            const double _J1_D1D2_t163 = (_J1_D1D2_t17 * _J1_D1D2_t164);
            const double _J1_D1D2_t166 = (_J1_D1D2_t151 - _J1_D1D2_t167);
            const double _J1_D1D2_t168 = (gR9C9 * _J1_D1D2_t169);
            const double _J1_D1D2_t279 = (zEamp_Ccomp * _J1_D1D2_t12);
            const double _J1_D1D2_t282 = (gEamp_Rcomp * _J1_D1D2_t12);
            const double _J1_D1D2_t287 = (Eamp_Aol * _J1_D1D2_t282);
            const double _J1_D1D2_t307 = (gEamp_Rin * _J1_D1D2_t287);
            const double _J1_D1D2_t308 = (_J1_D1D2_t42 * _J1_D1D2_t287);
            const double _J1_D1D2_t321 = (_J1_D1D2_t94 * _J1_D1D2_t158);
            const double _J1_D1D2_t337 = (_J1_D1D2_t94 * _J1_D1D2_t338);
            const double _J1_D1D2_t339 = (_J1_D1D2_t76 * _J1_D1D2_t157);
            const double _J1_D1D2_t355 = (_J1_D1D2_t42 * _J1_D1D2_t338);
            const double _J1_D1D2_t362 = (gRdC5 * _J1_D1D2_t363);
            const double _J1_D1D2_t366 = (_J1_D1D2_t42 * _J1_D1D2_t363);
            const double _J1_D1D2_t369 = (_J1_D1D2_t42 * _J1_D1D2_t370);
            const double _J1_D1D2_t371 = (gRdC5 * _J1_D1D2_t372);
            const double _J1_D1D2_t377 = (_J1_D1D2_t378 - _J1_D1D2_t371);
            const double _J1_D1D2_t379 = (_J1_D1D2_t17 * _J1_D1D2_t380);
            const double _J1_D1D2_t9 = (_J1_D1D2_t10 * _J1_D1D2_t12);
            const double _J1_D1D2_t20 = (_J1_D1D2_t10 * _J1_D1D2_t21);
            const double _J1_D1D2_t28 = (Eamp_Aol * _J1_D1D2_t29);
            const double _J1_D1D2_t30 = (gEamp_Rin * _J1_D1D2_t31);
            const double _J1_D1D2_t39 = (gRdC5 * _J1_D1D2_t20);
            const double _J1_D1D2_t41 = (_J1_D1D2_t42 * _J1_D1D2_t20);
            const double _J1_D1D2_t49 = (_J1_D1D2_t42 * _J1_D1D2_t9);
            const double _J1_D1D2_t53 = (_J1_D1D2_t54 - _J1_D1D2_t56);
            const double _J1_D1D2_t60 = (gRdC5 * _J1_D1D2_t61);
            const double _J1_D1D2_t65 = (_J1_D1D2_t66 - _J1_D1D2_t69);
            const double _J1_D1D2_t75 = (_J1_D1D2_t76 * _J1_D1D2_t77);
            const double _J1_D1D2_t93 = (_J1_D1D2_t94 * _J1_D1D2_t62);
            const double _J1_D1D2_t113 = (gR9C9 * _J1_D1D2_t114);
            const double _J1_D1D2_t143 = (_J1_D1D2_t17 * _J1_D1D2_t144);
            const double _J1_D1D2_t150 = (_J1_D1D2_t151 - _J1_D1D2_t154);
            const double _J1_D1D2_t156 = (gR9C9 * _J1_D1D2_t157);
            const double _J1_D1D2_t165 = (gEamp_Rout * _J1_D1D2_t166);
            const double _J1_D1D2_t281 = (_J1_D1D2_t109 * _J1_D1D2_t282);
            const double _J1_D1D2_t286 = (_J1_D1D2_t94 * _J1_D1D2_t287);
            const double _J1_D1D2_t295 = (_J1_D1D2_t94 * _J1_D1D2_t9);
            const double _J1_D1D2_t302 = (_J1_D1D2_t76 * _J1_D1D2_t49);
            const double _J1_D1D2_t306 = (_J1_D1D2_t307 - _J1_D1D2_t308);
            const double _J1_D1D2_t354 = (_J1_D1D2_t355 - _J1_D1D2_t356);
            const double _J1_D1D2_t361 = (gRdC5 * _J1_D1D2_t362);
            const double _J1_D1D2_t365 = (_J1_D1D2_t17 * _J1_D1D2_t366);
            const double _J1_D1D2_t368 = (_J1_D1D2_t369 - _J1_D1D2_t371);
            const double _J1_D1D2_t376 = (gEamp_Rout * _J1_D1D2_t377);
            const double _J1_D1D2_t8 = (gEamp_Rin * _J1_D1D2_t9);
            const double _J1_D1D2_t19 = (gEamp_Rin * _J1_D1D2_t20);
            const double _J1_D1D2_t27 = (gRdC5 * _J1_D1D2_t28);
            const double _J1_D1D2_t38 = (gRdC5 * _J1_D1D2_t39);
            const double _J1_D1D2_t40 = (_J1_D1D2_t17 * _J1_D1D2_t41);
            const double _J1_D1D2_t46 = (_J1_D1D2_t27 - _J1_D1D2_t47);
            const double _J1_D1D2_t48 = (gR9C9 * _J1_D1D2_t49);
            const double _J1_D1D2_t64 = (gEamp_Rout * _J1_D1D2_t65);
            const double _J1_D1D2_t142 = (_J1_D1D2_t143 - _J1_D1D2_t147);
            const double _J1_D1D2_t149 = (gEamp_Rout * _J1_D1D2_t150);
            const double _J1_D1D2_t162 = (_J1_D1D2_t163 + _J1_D1D2_t165);
            const double _J1_D1D2_t265 = (_J1_D1D2_t76 * _J1_D1D2_t8);
            const double _J1_D1D2_t305 = (gEamp_Rout * _J1_D1D2_t306);
            const double _J1_D1D2_t309 = (gRdC5 * _J1_D1D2_t8);
            const double _J1_D1D2_t353 = (gEamp_Rout * _J1_D1D2_t354);
            const double _J1_D1D2_t360 = (_J1_D1D2_t361 - _J1_D1D2_t365);
            const double _J1_D1D2_t367 = (gEamp_Rout * _J1_D1D2_t368);
            const double _J1_D1D2_t375 = (_J1_D1D2_t376 - _J1_D1D2_t379);
            const double _J1_D1D2_t7 = (gR9C9 * _J1_D1D2_t8);
            const double _J1_D1D2_t16 = (_J1_D1D2_t17 * _J1_D1D2_t19);
            const double _J1_D1D2_t26 = (_J1_D1D2_t27 - _J1_D1D2_t30);
            const double _J1_D1D2_t37 = (_J1_D1D2_t38 - _J1_D1D2_t40);
            const double _J1_D1D2_t45 = (gEamp_Rout * _J1_D1D2_t46);
            const double _J1_D1D2_t59 = (_J1_D1D2_t60 + _J1_D1D2_t64);
            const double _J1_D1D2_t141 = (_J1_D1D2_t142 + _J1_D1D2_t149);
            const double _J1_D1D2_t161 = (_J1_D1D2_t162 - _J1_D1D2_t168);
            const double _J1_D1D2_t304 = (_J1_D1D2_t305 - _J1_D1D2_t309);
            const double _J1_D1D2_t351 = (_J1_D1D2_t352 + _J1_D1D2_t353);
            const double _J1_D1D2_t359 = (_J1_D1D2_t360 + _J1_D1D2_t367);
            const double _J1_D1D2_t374 = (gEamp_Rin * _J1_D1D2_t375);
            const double _J1_D1D2_t25 = (gEamp_Rout * _J1_D1D2_t26);
            const double _J1_D1D2_t36 = (_J1_D1D2_t37 - _J1_D1D2_t45);
            const double _J1_D1D2_t52 = (_J1_D1D2_t53 * _J1_D1D2_t59);
            const double _J1_D1D2_t140 = (_J1_D1D2_t141 - _J1_D1D2_t156);
            const double _J1_D1D2_t160 = (gEamp_Rin * _J1_D1D2_t161);
            const double _J1_D1D2_t303 = (_J1_D1D2_t53 * _J1_D1D2_t304);
            const double _J1_D1D2_t350 = (_J1_D1D2_t53 * _J1_D1D2_t351);
            const double _J1_D1D2_t358 = (_J1_D1D2_t33 * _J1_D1D2_t359);
            const double _J1_D1D2_t15 = (_J1_D1D2_t16 + _J1_D1D2_t25);
            const double _J1_D1D2_t35 = (_J1_D1D2_t36 + _J1_D1D2_t48);
            const double _J1_D1D2_t139 = (_J1_D1D2_t33 * _J1_D1D2_t140);
            const double _J1_D1D2_t357 = (_J1_D1D2_t358 - _J1_D1D2_t374);
            const double _J1_D1D2_t6 = (_J1_D1D2_t7 - _J1_D1D2_t15);
            const double _J1_D1D2_t32 = (_J1_D1D2_t33 * _J1_D1D2_t35);
            const double _J1_D1D2_t138 = (_J1_D1D2_t139 - _J1_D1D2_t160);
            const double _J1_D1D2_t174 = (_J1_D1D2_t3 * _J1_D1D2_t138);
            const double _J1_D1D2_t5 = (gEamp_Rin * _J1_D1D2_t6);
            const double _J1_D1D2_t137 = (gC13 * _J1_D1D2_t138);
            const double _J1_D1D2_t173 = (gC12 * _J1_D1D2_t174);
            const double _J1_D1D2_t4 = (_J1_D1D2_t5 - _J1_D1D2_t32);
            const double _J1_D1D2_t130 = (gC13 * _J1_D1D2_t4);
            const double _J1_D1D2_t134 = (_J1_D1D2_t135 * _J1_D1D2_t130);
            const double _J1_D1D2_t136 = (gC12 * _J1_D1D2_t137);
            const double _J1_D1D2_t2 = (_J1_D1D2_t3 * _J1_D1D2_t4);
            const double _J1_D1D2_t133 = (_J1_D1D2_t134 - _J1_D1D2_t136);
            const double _J1_D1D2_t172 = (_J1_D1D2_t135 * _J1_D1D2_t2);
            const double _J1_D1D2_t132 = (gC13 * _J1_D1D2_t133);
            const double _J1_D1D2_t171 = (_J1_D1D2_t172 - _J1_D1D2_t173);
            const double _J1_D1D2_t170 = (_J1_D1D2_t1 * _J1_D1D2_t171);
            const double _J1_D1D2_t131 = (1.0 / (_J1_D1D2_t132 - _J1_D1D2_t170));
            const double _J1_D1D2_t385 = (zC12 * _J1_D1D2_t2);
            const double _J1_D1D2_t386 = (zC12 * _J1_D1D2_t130);
            const double _J1_D1D2_t387 = (zC13 * _J1_D1D2_t133);
            const double _J1_D1D2_t388 = (zC12 * (gC13 * _J1_D1D2_t357));
            const double _J1_D1D2_t389 = (zC12 * (_J1_D1D2_t3 * _J1_D1D2_t357));
            for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
            {
                const double _J1_D1D2_t89 = (vD1D2 / D1N914_vt);
                const double _J1_D1D2_t179 = (vGSJ1 - _2N5485_vp);
                const double _J1_D1D2_t88 = exp(_J1_D1D2_t89);
                const double _J1_D1D2_t90 = (1.0 / _J1_D1D2_t88);
                const double _J1_D1D2_t178 = (_J1_D1D2_t179 + _J1_D1D2_t180);
                const double _J1_D1D2_t192 = (_J1_D1D2_t88 / D1N914_vt);
                const double _J1_D1D2_t193 = (_J1_D1D2_t194 / _J1_D1D2_t88);
                const double _J1_D1D2_t384 = (_J1_D1D2_t178 + _J1_D1D2_t178);
                const double _J1_D1D2_t87 = (_J1_D1D2_t88 - _J1_D1D2_t90);
                const double _J1_D1D2_t177 = (_J1_D1D2_t178 * _J1_D1D2_t178);
                const double _J1_D1D2_t191 = (_J1_D1D2_t192 + _J1_D1D2_t193);
                const double _J1_D1D2_t383 = (_2N5485_Beta * _J1_D1D2_t384);
                const double _J1_D1D2_t86 = (D1N914_Is * _J1_D1D2_t87);
                const double _J1_D1D2_t176 = (_2N5485_Beta * _J1_D1D2_t177);
                const double _J1_D1D2_t190 = (D1N914_Is * _J1_D1D2_t191);
                const double _J1_D1D2_t218 = (gC12 * _J1_D1D2_t190);
                const double _J1_D1D2_t224 = (gEamp_Rout * _J1_D1D2_t218);
                const double _J1_D1D2_t227 = (gEamp_Rcomp * _J1_D1D2_t218);
                const double _J1_D1D2_t232 = (gEamp_Rin * _J1_D1D2_t224);
                const double _J1_D1D2_t242 = (gRfR10 * _J1_D1D2_t190);
                const double _J1_D1D2_t245 = (gEamp_Rcomp * _J1_D1D2_t242);
                const double _J1_D1D2_t248 = (_J1_D1D2_t10 * _J1_D1D2_t242);
                const double _J1_D1D2_t253 = (gRdC5 * _J1_D1D2_t248);
                const double _J1_D1D2_t255 = (_J1_D1D2_t42 * _J1_D1D2_t248);
                const double _J1_D1D2_t85 = (zR9C9 + _J1_D1D2_t86);
                const double _J1_D1D2_t175 = (zC13 + _J1_D1D2_t176);
                const double _J1_D1D2_t189 = (_J1_D1D2_t13 * _J1_D1D2_t190);
                const double _J1_D1D2_t197 = (gEamp_Rcomp * _J1_D1D2_t189);
                const double _J1_D1D2_t200 = (_J1_D1D2_t10 * _J1_D1D2_t189);
                const double _J1_D1D2_t205 = (_J1_D1D2_t42 * _J1_D1D2_t200);
                const double _J1_D1D2_t207 = (gRdC5 * _J1_D1D2_t200);
                const double _J1_D1D2_t217 = (_J1_D1D2_t10 * _J1_D1D2_t218);
                const double _J1_D1D2_t220 = (_J1_D1D2_t42 * _J1_D1D2_t217);
                const double _J1_D1D2_t223 = (_J1_D1D2_t42 * _J1_D1D2_t224);
                const double _J1_D1D2_t226 = (Eamp_Aol * _J1_D1D2_t227);
                const double _J1_D1D2_t234 = (gEamp_Rin * _J1_D1D2_t217);
                const double _J1_D1D2_t241 = (gEamp_Rout * _J1_D1D2_t242);
                const double _J1_D1D2_t244 = (Eamp_Aol * _J1_D1D2_t245);
                const double _J1_D1D2_t247 = (gEamp_Rin * _J1_D1D2_t248);
                const double _J1_D1D2_t252 = (gRdC5 * _J1_D1D2_t253);
                const double _J1_D1D2_t254 = (_J1_D1D2_t17 * _J1_D1D2_t255);
                const double _J1_D1D2_t258 = (_J1_D1D2_t42 * _J1_D1D2_t241);
                const double _J1_D1D2_t272 = (_J1_D1D2_t13 * _J1_D1D2_t85);
                const double _J1_D1D2_t320 = (gC12 * _J1_D1D2_t85);
                const double _J1_D1D2_t330 = (gEamp_Rout * _J1_D1D2_t320);
                const double _J1_D1D2_t336 = (gEamp_Rcomp * _J1_D1D2_t320);
                const double _J1_D1D2_t84 = (gRfR10 * _J1_D1D2_t85);
                const double _J1_D1D2_t188 = (gEamp_Rout * _J1_D1D2_t189);
                const double _J1_D1D2_t196 = (Eamp_Aol * _J1_D1D2_t197);
                const double _J1_D1D2_t199 = (gEamp_Rin * _J1_D1D2_t200);
                const double _J1_D1D2_t204 = (_J1_D1D2_t17 * _J1_D1D2_t205);
                const double _J1_D1D2_t206 = (gRdC5 * _J1_D1D2_t207);
                const double _J1_D1D2_t210 = (_J1_D1D2_t42 * _J1_D1D2_t188);
                const double _J1_D1D2_t216 = (gRdC5 * _J1_D1D2_t217);
                const double _J1_D1D2_t219 = (_J1_D1D2_t17 * _J1_D1D2_t220);
                const double _J1_D1D2_t225 = (gRdC5 * _J1_D1D2_t226);
                const double _J1_D1D2_t231 = (_J1_D1D2_t232 - _J1_D1D2_t225);
                const double _J1_D1D2_t233 = (_J1_D1D2_t17 * _J1_D1D2_t234);
                const double _J1_D1D2_t240 = (gEamp_Rin * _J1_D1D2_t241);
                const double _J1_D1D2_t243 = (gRdC5 * _J1_D1D2_t244);
                const double _J1_D1D2_t246 = (_J1_D1D2_t17 * _J1_D1D2_t247);
                const double _J1_D1D2_t251 = (_J1_D1D2_t252 - _J1_D1D2_t254);
                const double _J1_D1D2_t257 = (_J1_D1D2_t258 - _J1_D1D2_t243);
                const double _J1_D1D2_t270 = (_J1_D1D2_t271 - _J1_D1D2_t272);
                const double _J1_D1D2_t280 = (gEamp_Rout * _J1_D1D2_t270);
                const double _J1_D1D2_t285 = (gEamp_Rcomp * _J1_D1D2_t270);
                const double _J1_D1D2_t319 = (_J1_D1D2_t10 * _J1_D1D2_t320);
                const double _J1_D1D2_t323 = (_J1_D1D2_t42 * _J1_D1D2_t319);
                const double _J1_D1D2_t329 = (_J1_D1D2_t330 + _J1_D1D2_t331);
                const double _J1_D1D2_t335 = (Eamp_Aol * _J1_D1D2_t336);
                const double _J1_D1D2_t348 = (gEamp_Rin * _J1_D1D2_t319);
                const double _J1_D1D2_t81 = (_J1_D1D2_t82 - _J1_D1D2_t84);
                const double _J1_D1D2_t105 = (gEamp_Rcomp * _J1_D1D2_t81);
                const double _J1_D1D2_t112 = (gEamp_Rout * _J1_D1D2_t81);
                const double _J1_D1D2_t187 = (gEamp_Rin * _J1_D1D2_t188);
                const double _J1_D1D2_t195 = (gRdC5 * _J1_D1D2_t196);
                const double _J1_D1D2_t198 = (_J1_D1D2_t17 * _J1_D1D2_t199);
                const double _J1_D1D2_t203 = (_J1_D1D2_t204 - _J1_D1D2_t206);
                const double _J1_D1D2_t209 = (_J1_D1D2_t210 - _J1_D1D2_t195);
                const double _J1_D1D2_t215 = (gRdC5 * _J1_D1D2_t216);
                const double _J1_D1D2_t222 = (_J1_D1D2_t223 - _J1_D1D2_t225);
                const double _J1_D1D2_t230 = (gEamp_Rout * _J1_D1D2_t231);
                const double _J1_D1D2_t239 = (_J1_D1D2_t240 - _J1_D1D2_t243);
                const double _J1_D1D2_t256 = (gEamp_Rout * _J1_D1D2_t257);
                const double _J1_D1D2_t269 = (_J1_D1D2_t10 * _J1_D1D2_t270);
                const double _J1_D1D2_t278 = (_J1_D1D2_t279 - _J1_D1D2_t280);
                const double _J1_D1D2_t284 = (Eamp_Aol * _J1_D1D2_t285);
                const double _J1_D1D2_t294 = (gRdC5 * _J1_D1D2_t269);
                const double _J1_D1D2_t297 = (_J1_D1D2_t42 * _J1_D1D2_t269);
                const double _J1_D1D2_t318 = (gRdC5 * _J1_D1D2_t319);
                const double _J1_D1D2_t322 = (_J1_D1D2_t17 * _J1_D1D2_t323);
                const double _J1_D1D2_t328 = (_J1_D1D2_t329 + _J1_D1D2_t332);
                const double _J1_D1D2_t334 = (gRdC5 * _J1_D1D2_t335);
                const double _J1_D1D2_t346 = (gEamp_Rin * _J1_D1D2_t328);
                const double _J1_D1D2_t347 = (_J1_D1D2_t17 * _J1_D1D2_t348);
                const double _J1_D1D2_t80 = (_J1_D1D2_t10 * _J1_D1D2_t81);
                const double _J1_D1D2_t98 = (gRdC5 * _J1_D1D2_t80);
                const double _J1_D1D2_t104 = (Eamp_Aol * _J1_D1D2_t105);
                const double _J1_D1D2_t110 = (_J1_D1D2_t111 - _J1_D1D2_t112);
                const double _J1_D1D2_t123 = (gEamp_Rin * _J1_D1D2_t80);
                const double _J1_D1D2_t186 = (_J1_D1D2_t187 - _J1_D1D2_t195);
                const double _J1_D1D2_t208 = (gEamp_Rout * _J1_D1D2_t209);
                const double _J1_D1D2_t214 = (_J1_D1D2_t215 - _J1_D1D2_t219);
                const double _J1_D1D2_t221 = (gEamp_Rout * _J1_D1D2_t222);
                const double _J1_D1D2_t229 = (_J1_D1D2_t230 - _J1_D1D2_t233);
                const double _J1_D1D2_t238 = (gEamp_Rout * _J1_D1D2_t239);
                const double _J1_D1D2_t250 = (_J1_D1D2_t251 + _J1_D1D2_t256);
                const double _J1_D1D2_t268 = (gEamp_Rin * _J1_D1D2_t269);
                const double _J1_D1D2_t277 = (_J1_D1D2_t278 + _J1_D1D2_t281);
                const double _J1_D1D2_t283 = (gRdC5 * _J1_D1D2_t284);
                const double _J1_D1D2_t293 = (_J1_D1D2_t294 + _J1_D1D2_t295);
                const double _J1_D1D2_t296 = (_J1_D1D2_t17 * _J1_D1D2_t297);
                const double _J1_D1D2_t301 = (_J1_D1D2_t42 * _J1_D1D2_t277);
                const double _J1_D1D2_t317 = (_J1_D1D2_t318 - _J1_D1D2_t321);
                const double _J1_D1D2_t327 = (_J1_D1D2_t42 * _J1_D1D2_t328);
                const double _J1_D1D2_t345 = (_J1_D1D2_t346 - _J1_D1D2_t334);
                const double _J1_D1D2_t79 = (_J1_D1D2_t42 * _J1_D1D2_t80);
                const double _J1_D1D2_t92 = (_J1_D1D2_t93 + _J1_D1D2_t98);
                const double _J1_D1D2_t103 = (gRdC5 * _J1_D1D2_t104);
                const double _J1_D1D2_t107 = (_J1_D1D2_t108 + _J1_D1D2_t110);
                const double _J1_D1D2_t122 = (_J1_D1D2_t17 * _J1_D1D2_t123);
                const double _J1_D1D2_t127 = (gEamp_Rin * _J1_D1D2_t107);
                const double _J1_D1D2_t185 = (gEamp_Rout * _J1_D1D2_t186);
                const double _J1_D1D2_t202 = (_J1_D1D2_t203 - _J1_D1D2_t208);
                const double _J1_D1D2_t213 = (_J1_D1D2_t214 + _J1_D1D2_t221);
                const double _J1_D1D2_t228 = (gEamp_Rin * _J1_D1D2_t229);
                const double _J1_D1D2_t237 = (_J1_D1D2_t238 - _J1_D1D2_t246);
                const double _J1_D1D2_t249 = (_J1_D1D2_t33 * _J1_D1D2_t250);
                const double _J1_D1D2_t267 = (_J1_D1D2_t17 * _J1_D1D2_t268);
                const double _J1_D1D2_t276 = (gEamp_Rin * _J1_D1D2_t277);
                const double _J1_D1D2_t292 = (gRdC5 * _J1_D1D2_t293);
                const double _J1_D1D2_t300 = (_J1_D1D2_t283 + _J1_D1D2_t301);
                const double _J1_D1D2_t316 = (gRdC5 * _J1_D1D2_t317);
                const double _J1_D1D2_t326 = (_J1_D1D2_t327 - _J1_D1D2_t334);
                const double _J1_D1D2_t344 = (_J1_D1D2_t345 + _J1_D1D2_t337);
                const double _J1_D1D2_t78 = (_J1_D1D2_t17 * _J1_D1D2_t79);
                const double _J1_D1D2_t91 = (gRdC5 * _J1_D1D2_t92);
                const double _J1_D1D2_t101 = (_J1_D1D2_t102 + _J1_D1D2_t103);
                const double _J1_D1D2_t106 = (_J1_D1D2_t42 * _J1_D1D2_t107);
                const double _J1_D1D2_t120 = (_J1_D1D2_t121 - _J1_D1D2_t122);
                const double _J1_D1D2_t126 = (_J1_D1D2_t102 + _J1_D1D2_t127);
                const double _J1_D1D2_t184 = (_J1_D1D2_t185 - _J1_D1D2_t198);
                const double _J1_D1D2_t201 = (_J1_D1D2_t33 * _J1_D1D2_t202);
                const double _J1_D1D2_t212 = (_J1_D1D2_t33 * _J1_D1D2_t213);
                const double _J1_D1D2_t236 = (gEamp_Rin * _J1_D1D2_t237);
                const double _J1_D1D2_t275 = (_J1_D1D2_t276 + _J1_D1D2_t283);
                const double _J1_D1D2_t291 = (_J1_D1D2_t292 - _J1_D1D2_t296);
                const double _J1_D1D2_t299 = (_J1_D1D2_t300 + _J1_D1D2_t286);
                const double _J1_D1D2_t315 = (_J1_D1D2_t316 - _J1_D1D2_t322);
                const double _J1_D1D2_t325 = (_J1_D1D2_t326 + _J1_D1D2_t337);
                const double _J1_D1D2_t343 = (gEamp_Rout * _J1_D1D2_t344);
                const double _J1_D1D2_t74 = (_J1_D1D2_t75 - _J1_D1D2_t78);
                const double _J1_D1D2_t100 = (_J1_D1D2_t101 + _J1_D1D2_t106);
                const double _J1_D1D2_t125 = (_J1_D1D2_t126 + _J1_D1D2_t103);
                const double _J1_D1D2_t183 = (gEamp_Rin * _J1_D1D2_t184);
                const double _J1_D1D2_t211 = (_J1_D1D2_t212 - _J1_D1D2_t228);
                const double _J1_D1D2_t235 = (_J1_D1D2_t236 - _J1_D1D2_t249);
                const double _J1_D1D2_t274 = (_J1_D1D2_t275 + _J1_D1D2_t286);
                const double _J1_D1D2_t298 = (gEamp_Rout * _J1_D1D2_t299);
                const double _J1_D1D2_t324 = (gEamp_Rout * _J1_D1D2_t325);
                const double _J1_D1D2_t342 = (_J1_D1D2_t343 - _J1_D1D2_t347);
                const double _J1_D1D2_t73 = (_J1_D1D2_t74 + _J1_D1D2_t91);
                const double _J1_D1D2_t99 = (gEamp_Rout * _J1_D1D2_t100);
                const double _J1_D1D2_t124 = (gEamp_Rout * _J1_D1D2_t125);
                const double _J1_D1D2_t182 = (_J1_D1D2_t183 + _J1_D1D2_t201);
                const double _J1_D1D2_t273 = (gEamp_Rout * _J1_D1D2_t274);
                const double _J1_D1D2_t290 = (_J1_D1D2_t291 - _J1_D1D2_t298);
                const double _J1_D1D2_t314 = (_J1_D1D2_t315 + _J1_D1D2_t324);
                const double _J1_D1D2_t341 = (_J1_D1D2_t342 - _J1_D1D2_t349);
                const double _J1_D1D2_t72 = (_J1_D1D2_t73 - _J1_D1D2_t99);
                const double _J1_D1D2_t119 = (_J1_D1D2_t120 - _J1_D1D2_t124);
                const double _J1_D1D2_t266 = (_J1_D1D2_t267 + _J1_D1D2_t273);
                const double _J1_D1D2_t289 = (_J1_D1D2_t290 + _J1_D1D2_t302);
                const double _J1_D1D2_t313 = (_J1_D1D2_t314 - _J1_D1D2_t339);
                const double _J1_D1D2_t340 = (gEamp_Rin * _J1_D1D2_t341);
                const double _J1_D1D2_t71 = (_J1_D1D2_t72 + _J1_D1D2_t113);
                const double _J1_D1D2_t118 = (_J1_D1D2_t119 + _J1_D1D2_t128);
                const double _J1_D1D2_t264 = (_J1_D1D2_t265 - _J1_D1D2_t266);
                const double _J1_D1D2_t288 = (_J1_D1D2_t33 * _J1_D1D2_t289);
                const double _J1_D1D2_t312 = (_J1_D1D2_t33 * _J1_D1D2_t313);
                const double _J1_D1D2_t70 = (_J1_D1D2_t33 * _J1_D1D2_t71);
                const double _J1_D1D2_t117 = (gEamp_Rin * _J1_D1D2_t118);
                const double _J1_D1D2_t181 = ((((_J1_D1D2_t1 * ((_J1_D1D2_t135 * (_J1_D1D2_t3 * _J1_D1D2_t182)) + (gC12 * (_J1_D1D2_t3 * _J1_D1D2_t211)))) - (gC13 * ((_J1_D1D2_t135 * (gC13 * _J1_D1D2_t182)) + (gC12 * (gC13 * _J1_D1D2_t211))))) * _J1_D1D2_t131) - 1.0);
                const double _J1_D1D2_t263 = (gEamp_Rin * _J1_D1D2_t264);
                const double _J1_D1D2_t311 = (_J1_D1D2_t312 - _J1_D1D2_t340);
                const double _J1_D1D2_t382 = (((_J1_D1D2_t1 * ((_J1_D1D2_t135 * (_J1_D1D2_t383 * _J1_D1D2_t4)) - (gC12 * (_J1_D1D2_t383 * _J1_D1D2_t138)))) * _J1_D1D2_t131) - 1.0);
                const double _J1_D1D2_t51 = (_J1_D1D2_t52 + _J1_D1D2_t70);
                const double _J1_D1D2_t262 = (_J1_D1D2_t263 - _J1_D1D2_t288);
                const double _J1_D1D2_t310 = (_J1_D1D2_t311 - _J1_D1D2_t350);
                const double _J1_D1D2_t381 = (_J1_D1D2_t382 * _J1_D1D2_t181);
                const double _J1_D1D2_t50 = (_J1_D1D2_t51 - _J1_D1D2_t117);
                const double _J1_D1D2_t261 = (_J1_D1D2_t262 + _J1_D1D2_t303);
                const double _J1_D1D2_t0 = ((((_J1_D1D2_t1 * (_J1_D1D2_t385 + (gC12 * (_J1_D1D2_t3 * _J1_D1D2_t50)))) - (gC13 * (_J1_D1D2_t386 + (gC12 * (gC13 * _J1_D1D2_t50))))) * _J1_D1D2_t131) - ((_J1_D1D2_t387 - (_J1_D1D2_t1 * ((_J1_D1D2_t135 * (_J1_D1D2_t175 * _J1_D1D2_t4)) - (gC12 * (_J1_D1D2_t175 * _J1_D1D2_t138))))) * _J1_D1D2_t131));
                const double _J1_D1D2_t260 = (((gC13 * (((_J1_D1D2_t135 * (gC13 * _J1_D1D2_t261)) - (gC12 * (gC13 * _J1_D1D2_t310))) + _J1_D1D2_t388)) - (_J1_D1D2_t1 * (((_J1_D1D2_t135 * (_J1_D1D2_t3 * _J1_D1D2_t261)) - (gC12 * (_J1_D1D2_t3 * _J1_D1D2_t310))) + _J1_D1D2_t389))) * _J1_D1D2_t131);
                const double _J1_D1D2_t259 = (vD1D2 - _J1_D1D2_t260);
                const double res_vGSJ1 = (_J1_D1D2_t0 - vGSJ1);
                const double delta_vGSJ1 = ((((vGSJ1 - _J1_D1D2_t0) * _J1_D1D2_t181) - ((((_J1_D1D2_t1 * (gC12 * (_J1_D1D2_t3 * _J1_D1D2_t235))) - (gC13 * (gC12 * (gC13 * _J1_D1D2_t235)))) * _J1_D1D2_t131) * _J1_D1D2_t259)) / _J1_D1D2_t381);
                const double res_vD1D2 = (_J1_D1D2_t260 - vD1D2);
                const double delta_vD1D2 = ((_J1_D1D2_t382 * _J1_D1D2_t259) / _J1_D1D2_t381);
            
                double residual_norm_sq = 0.0;
                residual_norm_sq += res_vGSJ1 * res_vGSJ1;
                residual_norm_sq += res_vD1D2 * res_vD1D2;
                double step_norm_sq = 0.0;
                step_norm_sq += delta_vGSJ1 * delta_vGSJ1;
                step_norm_sq += delta_vD1D2 * delta_vD1D2;
            
                vGSJ1 = limit_jfet_vgs(vGSJ1 + (delta_vGSJ1), _2N5485_vp);
                vD1D2 = limit_junction_voltage(vD1D2 + (delta_vD1D2), vD1D2, D1N914_vt, vcrit_D1N914_vt);
            
                if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
                    break;
                
            }

            const double _t50 = ((vGSJ1 - _2N5485_vp) + _t226);
            const double _t62 = (_t227 - ((zR7C6 + zR8C7) + zRdC5));
            const double _t69 = (zRdC5 - zR9C9);
            const double _t77 = exp((vD1D2 / D1N914_vt));
            const double _t79 = (zC12 + zC11);
            const double _t90 = (_t62 * _t39);
            const double _t94 = (zEamp_Ccomp * _t33);
            const double _t127 = (_t94 + _t93);
            const double _t135 = (zEamp_Ccomp * _t42);
            const double _t161 = (_t79 * _t15);
            const double _t165 = (zEamp_Ccomp * _t156);
            const double _t177 = (_t79 * gR9C9);
            const double _t49 = (zC13 + (_2N5485_Beta * (_t50 * _t50)));
            const double _t59 = (_t228 - (zC4 + ((gR6C3 * vi) + zR6C3)));
            const double _t61 = (_t62 * _t20);
            const double _t67 = (zEamp_Ccomp * _t14);
            const double _t76 = (zR9C9 + (D1N914_Is * (_t77 - (1.0 / _t77))));
            const double _t78 = (_t79 * gRfR10);
            const double _t87 = (_t69 * _t34);
            const double _t89 = (_t62 * _t32);
            const double _t92 = (_t93 + _t94);
            const double _t95 = (_t62 * _t41);
            const double _t98 = (gC12 * _t76);
            const double _t108 = (_t67 + _t65);
            const double _t114 = (zEamp_Ccomp * _t9);
            const double _t128 = (_t62 * _t37);
            const double _t134 = (gEamp_Rout * _t98);
            const double _t140 = (gEamp_Rcomp * _t98);
            const double _t141 = (_t62 * _t142);
            const double _t162 = (gRfR10 * _t76);
            const double _t172 = (_t62 * _t173);
            const double _t176 = (_t8 * _t177);
            const double _t179 = (_t62 * _t155);
            const double _t185 = (_t25 * _t176);
            const double _t64 = (_t65 + _t67);
            const double _t68 = (_t69 * _t23);
            const double _t71 = (_t62 * _t13);
            const double _t75 = (_t10 * _t76);
            const double _t80 = (_t62 * _t7);
            const double _t88 = (_t11 * _t89);
            const double _t91 = (gRdC5 * _t92);
            const double _t97 = (_t8 * _t98);
            const double _t109 = (_t62 * _t18);
            const double _t116 = (_t62 * _t105);
            const double _t133 = (_t134 + _t135);
            const double _t139 = (Eamp_Aol * _t140);
            const double _t160 = (_t161 - _t162);
            const double _t166 = (gEamp_Rout * _t160);
            const double _t171 = (gEamp_Rcomp * _t160);
            const double _t175 = (gEamp_Rin * _t176);
            const double _t182 = (_t69 * _t183);
            const double _t184 = (gR9C9 * _t185);
            const double _t205 = (_t25 * _t97);
            const double _t206 = (gEamp_Rin * _t97);
            const double _t209 = (_t78 - _t75);
            const double _t210 = ((_t114 - (gEamp_Rout * _t209)) + _t115);
            const double _t213 = (gEamp_Rcomp * _t209);
            const double _t63 = (gRdC5 * _t64);
            const double _t70 = (_t11 * _t71);
            const double _t74 = (_t75 - _t78);
            const double _t96 = (gRdC5 * _t97);
            const double _t112 = (gEamp_Rcomp * _t74);
            const double _t113 = (((gEamp_Rout * _t74) + _t114) + _t115);
            const double _t132 = (_t133 + _t136);
            const double _t138 = (gRdC5 * _t139);
            const double _t145 = (gEamp_Rin * _t132);
            const double _t153 = (_t69 * _t154);
            const double _t159 = (_t8 * _t160);
            const double _t164 = (_t165 - _t166);
            const double _t170 = (Eamp_Aol * _t171);
            const double _t174 = (gR9C9 * _t175);
            const double _t178 = (gRdC5 * _t159);
            const double _t181 = (_t25 * _t159);
            const double _t194 = (_t167 + _t164);
            const double _t198 = (_t96 - _t95);
            const double _t204 = (((_t21 * (((gEamp_Rout * (_t128 + (_t25 * _t92))) - ((_t69 * _t31) + (gRdC5 * _t89))) - (gR9C9 * _t205))) + (_t59 * _t229)) - (gEamp_Rin * (((gEamp_Rout * ((gEamp_Rin * _t92) + _t128)) - (_t69 * _t44)) - (gR9C9 * _t206))));
            const double _t208 = (_t8 * _t209);
            const double _t212 = (Eamp_Aol * _t213);
            const double _t73 = (_t8 * _t74);
            const double _t86 = ((_t59 * _t30) + (gEamp_Rin * (((_t87 + _t88) - (gEamp_Rout * (_t90 + _t91))) - (gR9C9 * (_t95 - _t96)))));
            const double _t111 = (Eamp_Aol * _t112);
            const double _t131 = (_t25 * _t132);
            const double _t144 = (_t145 - _t138);
            const double _t158 = (gEamp_Rin * _t159);
            const double _t163 = (_t164 + _t167);
            const double _t169 = (gRdC5 * _t170);
            const double _t180 = (_t11 * _t181);
            const double _t186 = (_t59 * _t187);
            const double _t197 = ((_t21 * (((gEamp_Rout * (_t91 + _t90)) - (_t88 + _t87)) - (gR9C9 * _t198))) - (_t59 * _t43));
            const double _t200 = ((gEamp_Rin * (((_t69 * _t12) - (gEamp_Rout * ((gEamp_Rin * _t64) + _t109))) + (gR9C9 * (gEamp_Rin * _t73)))) - ((_t21 * ((((_t69 * _t24) + (gRdC5 * _t71)) - (gEamp_Rout * (_t109 + (_t25 * _t64)))) + (gR9C9 * (_t25 * _t73)))) + (_t59 * _t230)));
            const double _t211 = (gRdC5 * _t212);
            const double _t48 = (_t49 * _t29);
            const double _t72 = (gRdC5 * _t73);
            const double _t110 = (gRdC5 * _t111);
            const double _t130 = (_t131 - _t138);
            const double _t143 = (_t144 + _t141);
            const double _t157 = (_t11 * _t158);
            const double _t193 = ((_t186 + (_t21 * ((((_t182 - _t180) + (gRdC5 * (_t179 + _t178))) - (gEamp_Rout * ((_t172 + _t169) + (_t25 * _t194)))) + _t184))) - (gEamp_Rin * (((_t153 - _t157) - (gEamp_Rout * ((_t172 + (gEamp_Rin * _t194)) + _t169))) + _t174)));
            const double _t195 = ((_t59 * _t5) - (_t21 * (((_t70 + _t68) - (gEamp_Rout * (_t63 + _t61))) - (gR9C9 * (_t80 - _t72)))));
            const double _t207 = (((gEamp_Rin * ((_t69 * _t6) - ((_t11 * (gEamp_Rin * _t208)) + (gEamp_Rout * (((gEamp_Rin * _t210) + _t211) + _t116))))) - (_t21 * ((((gRdC5 * ((gRdC5 * _t208) + _t80)) - (_t11 * (_t25 * _t208))) - (gEamp_Rout * ((_t211 + (_t25 * _t210)) + _t116))) + (_t69 * _t27)))) + (_t59 * _t231));
            const double _t47 = (gC12 * _t48);
            const double _t52 = (_t49 * _t4);
            const double _t58 = ((_t59 * _t22) + (gEamp_Rin * (((gEamp_Rout * (_t61 + _t63)) - (_t68 + _t70)) - (gR9C9 * (_t72 - _t80)))));
            const double _t99 = ((_t59 * _t232) - ((_t21 * ((((gRdC5 * ((gRdC5 * _t108) + _t61)) - (_t11 * ((_t25 * _t108) + _t109))) - (_t69 * _t26)) - (gR9C9 * ((_t110 - (_t25 * _t113)) - _t116)))) + (gEamp_Rin * (((_t11 * ((gEamp_Rin * _t108) + _t109)) + (_t69 * _t16)) + (gR9C9 * ((_t110 - (gEamp_Rin * _t113)) - _t116))))));
            const double _t129 = (_t130 + _t141);
            const double _t152 = (((gEamp_Rin * ((_t153 - (_t157 + (gEamp_Rout * (((gEamp_Rin * _t163) + _t169) + _t172)))) + _t174)) - (_t21 * (((((gRdC5 * (_t178 + _t179)) - _t180) - (gEamp_Rout * ((_t169 + (_t25 * _t163)) + _t172))) + _t182) + _t184))) - _t186);
            const double _t214 = (((_t21 * ((((gRdC5 * _t198) - (_t11 * _t205)) + (gEamp_Rout * _t129)) - (_t69 * _t40))) - (gEamp_Rin * (((gEamp_Rout * _t143) - (_t11 * _t206)) - (_t69 * _t46)))) - (_t59 * _t233));
            const double _t51 = (_t1 * _t52);
            const double _t126 = (((_t21 * ((((_t11 * ((_t25 * _t127) + _t128)) - (gRdC5 * ((gRdC5 * _t127) + _t90))) + (_t69 * _t35)) - (gR9C9 * _t129))) - (gEamp_Rin * (((_t11 * ((gEamp_Rin * _t127) + _t128)) + (_t69 * _t45)) - (gR9C9 * _t143)))) + (_t59 * _t234));
            const double vo = (((zC13 * _t0) + (gC13 * (_t47 - _t51))) * _t53);
            const double vpos = (((gC13 * (((_t1 * (gC13 * _t58)) - (zC12 * _t216)) + (gC12 * (gC13 * _t86)))) - (_t57 * (((_t1 * (_t3 * _t58)) - (zC12 * _t217)) + (gC12 * (_t3 * _t86))))) * _t53);
            const double tC4 = (gC4 * (vpos - 0));
            const double vbw_Eamp = (((gC13 * (((_t1 * (gC13 * _t99)) + (zC12 * _t218)) - (gC12 * (gC13 * _t126)))) - (_t57 * (((_t1 * (_t3 * _t99)) + (zC12 * _t219)) - (gC12 * (_t3 * _t126))))) * _t53);
            const double tEamp_Ccomp = (gEamp_Ccomp * (vbw_Eamp - 0));
            const double vf = (((gC13 * ((_t1 * (gC13 * _t152)) - (zC12 * _t56))) - (_t57 * ((_t1 * (_t3 * _t152)) - (zC12 * _t28)))) * _t53);
            const double vg = (((_t57 * ((zC12 * _t2) + (gC12 * (_t3 * _t193)))) - (gC13 * ((zC12 * _t55) + (gC12 * (gC13 * _t193))))) * _t53);
            const double tC12 = (gC12 * (vf - vg));
            const double vneg = (((gC13 * (((_t1 * (gC13 * _t195)) - (zC12 * _t220)) - (gC12 * (gC13 * _t197)))) - (_t57 * (((_t1 * (_t3 * _t195)) - (zC12 * _t221)) - (gC12 * (_t3 * _t197))))) * _t53);
            const double vs = (((zC13 * _t54) - (_t57 * (_t51 - _t47))) * _t53);
            const double tC13 = (gC13 * (vs - vo));
            const double vR6C3 = (vpos - vi);
            const double tC11 = (gC11 * (vf - 0));
            const double v2 = (((gC13 * (((_t1 * (gC13 * _t200)) - (zC12 * _t222)) - (gC12 * (gC13 * _t204)))) - (_t57 * (((_t1 * (_t3 * _t200)) - (zC12 * _t223)) - (gC12 * (_t3 * _t204))))) * _t53);
            const double vRdC5 = (vneg - v2);
            const double vdp = (((gC13 * (((_t1 * (gC13 * _t207)) - (gC12 * (gC13 * _t214))) + (zC12 * _t224))) - (_t57 * (((_t1 * (_t3 * _t207)) - (gC12 * (_t3 * _t214))) + (zC12 * _t225)))) * _t53);
            const double vR9C9 = (v2 - vdp);
            const double _t199 = (vneg - 0);
            const double vR7C6 = _t199;
            const double vR8C7 = _t199;
            
            zC4 = 2 * tC4 - zC4;
            zEamp_Ccomp = 2 * tEamp_Ccomp - zEamp_Ccomp;
            zC12 = 2 * tC12 - zC12;
            zR7C6 = gR7C6 * (1 - gnR7C6) * vR7C6 + gnR7C6 * zR7C6; // RC series
            zC13 = 2 * tC13 - zC13;
            zR6C3 = gR6C3 * (1 - gnR6C3) * vR6C3 + gnR6C3 * zR6C3; // RC series
            zC11 = 2 * tC11 - zC11;
            zR8C7 = gR8C7 * (1 - gnR8C7) * vR8C7 + gnR8C7 * zR8C7; // RC series
            zRdC5 = gzRdC5 * vRdC5 - zRdC5; // RC parallel
            zR9C9 = gR9C9 * (1 - gnR9C9) * vR9C9 + gnR9C9 * zR9C9; // RC series

            output[ch][n] = vo;
        }
        state[ch].zC4 = zC4;
        state[ch].zEamp_Ccomp = zEamp_Ccomp;
        state[ch].zC12 = zC12;
        state[ch].zR7C6 = zR7C6;
        state[ch].zC13 = zC13;
        state[ch].zR6C3 = zR6C3;
        state[ch].zC11 = zC11;
        state[ch].zR8C7 = zR8C7;
        state[ch].zRdC5 = zRdC5;
        state[ch].zR9C9 = zR9C9;
        state[ch].vGSJ1 = vGSJ1;
        state[ch].vD1D2 = vD1D2;
    }
}

static void reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc)
{
    #define sum(a, b) ((a) + (b))
    #define recip_sum(a, b) ((a) * (b) / ((a) + (b)))
    
    const double _2N5485_Beta = params._2N5485_Beta;
    const double _2N5485_vp = params._2N5485_vp;
    const double Eamp_Aol = params.Eamp_Aol;
    const double Eamp_Vos = params.Eamp_Vos;
    const double Eamp_Ios = params.Eamp_Ios;
    const double Eamp_Ibias = params.Eamp_Ibias;
    const double D1N914_Is = params.D1N914_Is;
    const double D1N914_vt = params.D1N914_vt;
    
    const double Vpp = params.Vpp;
    
    const double vpp = Vpp;
    
    const double Vp = params.Vp;
    
    const double vp = Vp;
    
    const double gR5 = 1.0 / params.R5;
    
    const double gRL = 1.0 / params.RL;
    
    const double gR4 = 1.0 / params.R4;
    
    const double gC4 = 2.0 * sample_rate * params.C4;
    
    const double gEamp_Rin = 1.0 / params.Eamp_Rin;
    
    const double gEamp_Rcomp = 1.0 / params.Eamp_Rcomp;
    
    const double gEamp_Ccomp = 2.0 * sample_rate * params.Eamp_Ccomp;
    
    const double gEamp_Rout = 1.0 / params.Eamp_Rout;
    
    const double gR11 = 1.0 / params.R11;
    
    const double gC12 = 2.0 * sample_rate * params.C12;
    
    const double R7 = params.R7;
    const double C6 = params.C6;
    const double gR7C6 = (2.0 * sample_rate * C6) / (1.0 + 2.0 * sample_rate * R7 * C6);
    const double gnR7C6 = gR7C6 * (2.0 * sample_rate * R7 * C6 - 1.0) / (2.0 * sample_rate * C6);
    
    const double gC13 = 2.0 * sample_rate * params.C13;
    
    const double R6 = params.R6;
    const double C3 = params.C3;
    const double gR6C3 = (2.0 * sample_rate * C3) / (1.0 + 2.0 * sample_rate * R6 * C3);
    const double gnR6C3 = gR6C3 * (2.0 * sample_rate * R6 * C3 - 1.0) / (2.0 * sample_rate * C3);
    
    const double gC11 = 2.0 * sample_rate * params.C11;
    
    const double R8 = params.R8;
    const double C7 = params.C7;
    const double gR8C7 = (2.0 * sample_rate * C7) / (1.0 + 2.0 * sample_rate * R8 * C7);
    const double gnR8C7 = gR8C7 * (2.0 * sample_rate * R8 * C7 - 1.0) / (2.0 * sample_rate * C7);
    
    const double gR13 = 1.0 / params.R13;
    
    const double Rd = params.Rd;
    const double C5 = params.C5;
    const double gRdC5 = 2.0 * sample_rate * C5 + (1.0 / Rd);
    const double gzRdC5 = 4.0 * sample_rate * C5;
    
    const double R9 = params.R9;
    const double C9 = params.C9;
    const double gR9C9 = (2.0 * sample_rate * C9) / (1.0 + 2.0 * sample_rate * R9 * C9);
    const double gnR9C9 = gR9C9 * (2.0 * sample_rate * R9 * C9 - 1.0) / (2.0 * sample_rate * C9);
    
    const double RfR10 = sum(params.Rf, params.R10);
    const double gRfR10 = 1.0 / RfR10;
    
    const double vcrit_D1N914_vt = D1N914_vt * log(D1N914_vt / (sqrt(2.0) * D1N914_Is));
    
    const double vi = vi_dc;

    double vGSJ1 = 0;
    double vD1D2 = 0;

    // --- Newton-Raphson solve: J1
    const double _J1_t4 = (1.0 / 1000.0);
    const double _J1_t5 = (1.0 / (gR13 + (1.0 / 1000000000.0)));
    for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
    {
        const double _J1_t3 = (vGSJ1 - _2N5485_vp);
        const double _J1_t2 = (_J1_t3 + _J1_t4);
        const double _J1_t1 = ((_2N5485_Beta * (_J1_t2 * _J1_t2)) * _J1_t5);
        const double _J1_t0 = (_J1_t1 + vGSJ1);
        const double res_vGSJ1 = (-_J1_t0);
        const double delta_vGSJ1 = (-(_J1_t0 / (((_2N5485_Beta * (_J1_t2 + _J1_t2)) * _J1_t5) + 1.0)));
    
        double residual_norm_sq = 0.0;
        residual_norm_sq += res_vGSJ1 * res_vGSJ1;
        double step_norm_sq = 0.0;
        step_norm_sq += delta_vGSJ1 * delta_vGSJ1;
    
        vGSJ1 = limit_jfet_vgs(vGSJ1 + (delta_vGSJ1), _2N5485_vp);
    
        if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
            break;
        
    }
    // --- Newton-Raphson solve: D1D2
    const double _D1D2_t4 = (-1.0 / Rd);
    const double _D1D2_t5 = (1.0 / 1000000000.0);
    const double _D1D2_t8 = (gEamp_Rcomp + gEamp_Rout);
    const double _D1D2_t10 = (gRfR10 + _D1D2_t5);
    const double _D1D2_t20 = (gEamp_Rin + _D1D2_t5);
    const double _D1D2_t23 = (1.0 / Rd);
    const double _D1D2_t27 = (_D1D2_t10 * _D1D2_t10);
    const double _D1D2_t28 = (gRfR10 * gRfR10);
    const double _D1D2_t39 = (1.0 / D1N914_vt);
    const double _D1D2_t45 = (Eamp_Aol * gEamp_Rcomp);
    const double _D1D2_t3 = (_D1D2_t4 - gEamp_Rout);
    const double _D1D2_t7 = (_D1D2_t8 + _D1D2_t5);
    const double _D1D2_t22 = (gEamp_Rin + _D1D2_t23);
    const double _D1D2_t26 = (_D1D2_t27 - _D1D2_t28);
    const double _D1D2_t29 = (gEamp_Rout * _D1D2_t26);
    const double _D1D2_t32 = (gEamp_Rcomp * _D1D2_t26);
    const double _D1D2_t44 = (_D1D2_t45 * _D1D2_t10);
    const double _D1D2_t2 = (_D1D2_t3 - _D1D2_t5);
    const double _D1D2_t21 = (_D1D2_t22 + _D1D2_t5);
    const double _D1D2_t25 = (_D1D2_t7 * _D1D2_t26);
    const double _D1D2_t31 = (Eamp_Aol * _D1D2_t32);
    const double _D1D2_t43 = (_D1D2_t44 * D1N914_Is);
    const double _D1D2_t30 = (_D1D2_t31 / Rd);
    const double _D1D2_t24 = (1.0 / ((gEamp_Rin * ((_D1D2_t2 * (gEamp_Rin * _D1D2_t25)) + (gEamp_Rout * ((gEamp_Rin * _D1D2_t29) - _D1D2_t30)))) - (_D1D2_t20 * (((_D1D2_t2 * (_D1D2_t21 * _D1D2_t25)) + ((_D1D2_t25 / Rd) / Rd)) + (gEamp_Rout * ((_D1D2_t21 * _D1D2_t29) - _D1D2_t30))))));
    const double _D1D2_t46 = ((_D1D2_t7 * _D1D2_t10) * D1N914_Is);
    const double _D1D2_t47 = (1.0 / (Rd * Rd));
    for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
    {
        const double _D1D2_t14 = (vD1D2 / D1N914_vt);
        const double _D1D2_t13 = exp(_D1D2_t14);
        const double _D1D2_t15 = (1.0 / _D1D2_t13);
        const double _D1D2_t37 = (_D1D2_t13 / D1N914_vt);
        const double _D1D2_t38 = (_D1D2_t39 / _D1D2_t13);
        const double _D1D2_t12 = (_D1D2_t13 - _D1D2_t15);
        const double _D1D2_t36 = (_D1D2_t37 + _D1D2_t38);
        const double _D1D2_t11 = (D1N914_Is * _D1D2_t12);
        const double _D1D2_t35 = (D1N914_Is * _D1D2_t36);
        const double _D1D2_t42 = (_D1D2_t43 * _D1D2_t36);
        const double _D1D2_t9 = (_D1D2_t10 * _D1D2_t11);
        const double _D1D2_t16 = (gEamp_Rout * _D1D2_t9);
        const double _D1D2_t19 = (gEamp_Rcomp * _D1D2_t9);
        const double _D1D2_t34 = (_D1D2_t10 * _D1D2_t35);
        const double _D1D2_t40 = (gEamp_Rout * _D1D2_t34);
        const double _D1D2_t41 = (_D1D2_t42 / Rd);
        const double _D1D2_t6 = (_D1D2_t7 * _D1D2_t9);
        const double _D1D2_t18 = (Eamp_Aol * _D1D2_t19);
        const double _D1D2_t33 = (_D1D2_t7 * _D1D2_t34);
        const double _D1D2_t17 = (_D1D2_t18 / Rd);
        const double _D1D2_t1 = (((gEamp_Rin * ((_D1D2_t2 * (gEamp_Rin * _D1D2_t6)) + (gEamp_Rout * ((gEamp_Rin * _D1D2_t16) - _D1D2_t17)))) - (_D1D2_t20 * ((((_D1D2_t6 / Rd) / Rd) + (_D1D2_t2 * (_D1D2_t21 * _D1D2_t6))) + (gEamp_Rout * ((_D1D2_t21 * _D1D2_t16) - _D1D2_t17))))) * _D1D2_t24);
        const double _D1D2_t0 = (_D1D2_t1 + vD1D2);
        const double res_vD1D2 = (-_D1D2_t0);
        const double delta_vD1D2 = (-(_D1D2_t0 / ((((gEamp_Rin * ((_D1D2_t2 * (gEamp_Rin * _D1D2_t33)) + (gEamp_Rout * ((gEamp_Rin * _D1D2_t40) - _D1D2_t41)))) - (_D1D2_t20 * ((((_D1D2_t46 * _D1D2_t36) * _D1D2_t47) + (_D1D2_t2 * (_D1D2_t21 * _D1D2_t33))) + (gEamp_Rout * ((_D1D2_t21 * _D1D2_t40) - _D1D2_t41))))) * _D1D2_t24) + 1.0)));
    
        double residual_norm_sq = 0.0;
        residual_norm_sq += res_vD1D2 * res_vD1D2;
        double step_norm_sq = 0.0;
        step_norm_sq += delta_vD1D2 * delta_vD1D2;
    
        vD1D2 = limit_junction_voltage(vD1D2 + (delta_vD1D2), vD1D2, D1N914_vt, vcrit_D1N914_vt);
    
        if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
            break;
        
    }
    const double zC4 = ((gC4 * (((Eamp_Ibias + (Eamp_Ios / 2.0)) * ((((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))) / Rd) / Rd)) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) + (gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * ((Eamp_Ibias - (Eamp_Ios / 2.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((((Eamp_Aol * Eamp_Vos) * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd) + ((Eamp_Ibias - (Eamp_Ios / 2.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))))))))) / ((gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * ((((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))) / Rd) / Rd)) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd)))))));
    const double zEamp_Ccomp = ((gEamp_Ccomp * (((gEamp_Rin * ((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * ((gEamp_Rin * ((Eamp_Aol * Eamp_Vos) * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + ((Eamp_Ibias - (Eamp_Ios / 2.0)) * (Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * ((Eamp_Aol * Eamp_Vos) * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + ((Eamp_Ibias - (Eamp_Ios / 2.0)) * (Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))))) + (((((Eamp_Aol * Eamp_Vos) * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd) + ((Eamp_Ibias - (Eamp_Ios / 2.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) / Rd)))) - ((Eamp_Ibias + (Eamp_Ios / 2.0)) * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * ((gEamp_Rin * (Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) - (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))))) + (((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd)) / Rd))))) / ((gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * ((((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))) / Rd) / Rd)) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd)))))));
    const double zC12 = (-((gC12 * ((gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (gRfR10 * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt))))))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (gRfR10 * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))) - ((Eamp_Aol * (gEamp_Rcomp * (gRfR10 * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))) / Rd))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * (((((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (gRfR10 * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt))))))) / Rd) / Rd) + ((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (gRfR10 * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))))) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (gRfR10 * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))) - ((Eamp_Aol * (gEamp_Rcomp * (gRfR10 * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))) / Rd))))))) / ((gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * ((((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))) / Rd) / Rd)) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))))));
    const double zR7C6 = ((gR7C6 * (((gEamp_Rin + (1.0 / 1000000000.0)) * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * ((Eamp_Ibias - (Eamp_Ios / 2.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * (((Eamp_Ibias - (Eamp_Ios / 2.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) + (((Eamp_Aol * Eamp_Vos) * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) + ((Eamp_Ibias + (Eamp_Ios / 2.0)) * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))))) / ((gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * ((((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))) / Rd) / Rd)) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd)))))));
    const double zC13 = ((gC13 * ((_2N5485_Beta * ((vGSJ1 - _2N5485_vp) + (1.0 / 1000.0))) * ((vGSJ1 - _2N5485_vp) + (1.0 / 1000.0)))) / (gR13 + (1.0 / 1000000000.0)));
    const double zR6C3 = (gR6C3 * (((((Eamp_Ibias + (Eamp_Ios / 2.0)) * ((((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))) / Rd) / Rd)) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) + (gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * ((Eamp_Ibias - (Eamp_Ios / 2.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((((Eamp_Aol * Eamp_Vos) * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd) + ((Eamp_Ibias - (Eamp_Ios / 2.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))))))) / ((gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * ((((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))) / Rd) / Rd)) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))))) - vi));
    const double zC11 = (-((gC11 * ((gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (gRfR10 * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt))))))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (gRfR10 * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))) - ((Eamp_Aol * (gEamp_Rcomp * (gRfR10 * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))) / Rd))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * (((((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (gRfR10 * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt))))))) / Rd) / Rd) + ((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (gRfR10 * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))))) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (gRfR10 * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))) - ((Eamp_Aol * (gEamp_Rcomp * (gRfR10 * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))) / Rd))))))) / ((gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * ((((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))) / Rd) / Rd)) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))))));
    const double zR8C7 = ((gR8C7 * (((gEamp_Rin + (1.0 / 1000000000.0)) * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * ((Eamp_Ibias - (Eamp_Ios / 2.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * (((Eamp_Ibias - (Eamp_Ios / 2.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) + (((Eamp_Aol * Eamp_Vos) * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) + ((Eamp_Ibias + (Eamp_Ios / 2.0)) * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))))) / ((gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * ((((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))) / Rd) / Rd)) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd)))))));
    const double zRdC5 = ((gzRdC5 * (((((gEamp_Rin + (1.0 / 1000000000.0)) * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * ((Eamp_Ibias - (Eamp_Ios / 2.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * (((Eamp_Ibias - (Eamp_Ios / 2.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) + (((Eamp_Aol * Eamp_Vos) * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) + ((Eamp_Ibias + (Eamp_Ios / 2.0)) * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd)))))) / ((gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * ((((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))) / Rd) / Rd)) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))))) + (((((gEamp_Rin + (1.0 / 1000000000.0)) * ((((Eamp_Ibias - (Eamp_Ios / 2.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd) - (gEamp_Rout * (((Eamp_Ibias - (Eamp_Ios / 2.0)) * (Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * ((Eamp_Aol * Eamp_Vos) * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))))))) + ((Eamp_Ibias + (Eamp_Ios / 2.0)) * (((gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd) - (gEamp_Rout * ((gEamp_Rin * (Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) - (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))))))))) + (gEamp_Rin * (gEamp_Rout * ((gEamp_Rin * ((Eamp_Aol * Eamp_Vos) * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + ((Eamp_Ibias - (Eamp_Ios / 2.0)) * (Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))))))) / ((gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * ((((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))) / Rd) / Rd)) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))))))) / 2.0);
    const double zR9C9 = (gR9C9 * ((((gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * ((gRfR10 + (1.0 / 1000000000.0)) * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt))))))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * ((gRfR10 + (1.0 / 1000000000.0)) * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))) - ((Eamp_Aol * (gEamp_Rcomp * ((gRfR10 + (1.0 / 1000000000.0)) * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))) / Rd))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * (((((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * ((gRfR10 + (1.0 / 1000000000.0)) * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt))))))) / Rd) / Rd) + ((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * ((gRfR10 + (1.0 / 1000000000.0)) * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))))) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * ((gRfR10 + (1.0 / 1000000000.0)) * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))) - ((Eamp_Aol * (gEamp_Rcomp * ((gRfR10 + (1.0 / 1000000000.0)) * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))) / Rd)))))) / ((gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * ((((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))) / Rd) / Rd)) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))))) - (((((gEamp_Rin + (1.0 / 1000000000.0)) * ((((Eamp_Ibias - (Eamp_Ios / 2.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd) - (gEamp_Rout * (((Eamp_Ibias - (Eamp_Ios / 2.0)) * (Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * ((Eamp_Aol * Eamp_Vos) * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))))))) + ((Eamp_Ibias + (Eamp_Ios / 2.0)) * (((gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd) - (gEamp_Rout * ((gEamp_Rin * (Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) - (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))))))))) + (gEamp_Rin * (gEamp_Rout * ((gEamp_Rin * ((Eamp_Aol * Eamp_Vos) * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + ((Eamp_Ibias - (Eamp_Ios / 2.0)) * (Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))))))) / ((gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * ((((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))) / Rd) / Rd)) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd)))))))));

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].vGSJ1 = vGSJ1;
        state[ch].vD1D2 = vD1D2;
        state[ch].zC4 = zC4;
        state[ch].zEamp_Ccomp = zEamp_Ccomp;
        state[ch].zC12 = zC12;
        state[ch].zR7C6 = zR7C6;
        state[ch].zC13 = zC13;
        state[ch].zR6C3 = zR6C3;
        state[ch].zC11 = zC11;
        state[ch].zR8C7 = zR8C7;
        state[ch].zRdC5 = zRdC5;
        state[ch].zR9C9 = zR9C9;
    }
}
