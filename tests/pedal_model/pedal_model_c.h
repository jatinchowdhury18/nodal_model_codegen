// Auto-generated with netlist_codegen version 58e1e0e.
// Command: netlist_codegen pedal_model.net pedal_model_c.h -lang c -type_name double

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


static double limit_junction_voltage_sym(double v_new, double v_old, double vt, double vcrit) {
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

static double limit_jfet_vgs(double v_new, double vp) {
    if (v_new < vp) return vp;
    return v_new;
}

static const double newton_tol_sq = 0.000001;
static const int newton_max_iter = 20;


typedef struct {
    double Vpp; // = 9.0e+00
    double Vp; // = 4.5e+00
    double R5; // = 1.0e+06
    double RL; // = 1.0e+05
    double R4; // = 1.0e+06
    double _2N5485_Beta; // = 2.0e-03
    double _2N5485_vp; // = -1.0e+00
    double C4; // = 1.0e-09
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
    double zC12;
    double zR7C6;
    double zC13;
    double zR6C3;
    double zC11;
    double zR8C7;
    double zRdC5;
    double zR9C9;
    double vGSJ1;
    double vGSJ1_prev;
    double vD1D2;
    double vD1D2_prev;
} State;

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    #define sum(a, b) ((a) + (b))
    #define recip_sum(a, b) ((a) * (b) / ((a) + (b)))
    
    const double _2N5485_Beta = params._2N5485_Beta;
    const double _2N5485_vp = params._2N5485_vp;
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
    
    const double _t2 = (gC13 + gR13);
    const double _t4 = (gC4 + gR6C3);
    const double _t7 = (gR9C9 + gRfR10);
    const double _t8 = (gR11 + gC12);
    const double _t13 = ((gC12 + gC11) + gRfR10);
    const double _t23 = (gRL + gC13);
    const double _t37 = ((gR7C6 + gR8C7) + gRdC5);
    const double _t41 = (gRfR10 * gR9C9);
    const double _t48 = (_t4 * (gRdC5 * (gC12 * gRfR10)));
    const double _t50 = (_t13 * gR9C9);
    const double _t52 = (gC12 * gR9C9);
    const double _t6 = (gC12 * _t7);
    const double _t12 = ((_t13 * _t7) - (gRfR10 * gRfR10));
    const double _t40 = (_t37 * _t41);
    const double _t5 = (gRdC5 * _t6);
    const double _t11 = (gRdC5 * _t12);
    const double _t3 = (_t4 * _t5);
    const double _t10 = (_t4 * _t11);
    const double _t21 = (gC13 * _t3);
    const double _t22 = (gC13 * _t10);
    const double _t1 = (_t2 * _t3);
    const double _t9 = (_t2 * _t10);
    const double _t20 = ((gC12 * _t21) - (_t8 * _t22));
    const double _t0 = ((gC12 * _t1) - (_t8 * _t9));
    const double _t19 = (1.0 / ((gC13 * _t20) - (_t23 * _t0)));
    const double _t53 = (gC13 * _t48);
    const double _t54 = (_t2 * _t48);
    const double _t55 = (1.0 / 1000.0);
    const double _t56 = (_t37 * _t12);
    const double _t57 = (_t37 * _t6);
    const double _t58 = (_t37 * _t50);
    const double _t59 = (_t37 * _t52);
    double c0_vRdC5;
    double c_vRdC5[10];
    
    for (int _k = 0; _k <= 10; ++_k)
    {
        const double vi = (_k == 1) ? 1.0 : 0.0;
        const double zC4 = (_k == 2) ? 1.0 : 0.0;
        const double zC12 = (_k == 3) ? 1.0 : 0.0;
        const double zR7C6 = (_k == 4) ? 1.0 : 0.0;
        const double zC13 = (_k == 5) ? 1.0 : 0.0;
        const double zR6C3 = (_k == 6) ? 1.0 : 0.0;
        const double zC11 = (_k == 7) ? 1.0 : 0.0;
        const double zR8C7 = (_k == 8) ? 1.0 : 0.0;
        const double zRdC5 = (_k == 9) ? 1.0 : 0.0;
        const double zR9C9 = (_k == 10) ? 1.0 : 0.0;
        const double _t25 = (zC4 + ((gR6C3 * vi) + zR6C3));
        const double _t36 = ((zR7C6 + zR8C7) + zRdC5);
        const double _t46 = ((_t4 * (_t36 * _t12)) - (_t25 * _t56));
        const double _t47 = ((_t4 * (_t36 * _t6)) - (_t25 * _t57));
        const double _t24 = (_t25 * _t5);
        const double _t26 = (_t25 * _t11);
        const double vneg = (((gC13 * ((gC12 * (gC13 * _t24)) - (_t8 * (gC13 * _t26)))) - (_t23 * ((gC12 * (_t2 * _t24)) - (_t8 * (_t2 * _t26))))) * _t19);
        const double v2 = (((gC13 * ((_t8 * (gC13 * _t46)) - (gC12 * (gC13 * _t47)))) - (_t23 * ((_t8 * (_t2 * _t46)) - (gC12 * (_t2 * _t47))))) * _t19);
        const double vRdC5 = (vneg - v2);
        if (_k == 0) {
            c0_vRdC5 = vRdC5;
        } else {
            c_vRdC5[_k - 1] = vRdC5 - c0_vRdC5;
        }
    }
    
    const double _J1_D1D2_zt4 = (gC12 + gC11);
    const double _J1_D1D2_zt11 = (gRfR10 * 1.0);
    const double _J1_D1D2_zt12 = (-_J1_D1D2_zt11);
    const double _J1_D1D2_zt16 = (gRfR10 * gRfR10);
    const double _J1_D1D2_zt23 = (gRL + gC13);
    const double _J1_D1D2_zt25 = (_J1_D1D2_zt4 + gRfR10);
    const double _J1_D1D2_zt26 = (gC12 * 1.0);
    const double _J1_D1D2_zt31 = (gR9C9 + gRfR10);
    const double _J1_D1D2_zt42 = (gC13 + gR13);
    const double _J1_D1D2_zt43 = (gC4 + gR6C3);
    const double _J1_D1D2_zt52 = (gR11 + gC12);
    const double _J1_D1D2_zt6 = (gC12 * _J1_D1D2_zt31);
    const double _J1_D1D2_zt10 = (gRdC5 * _J1_D1D2_zt12);
    const double _J1_D1D2_zt22 = (gRdC5 * _J1_D1D2_zt26);
    const double _J1_D1D2_zt24 = (_J1_D1D2_zt25 * 1.0);
    const double _J1_D1D2_zt30 = (_J1_D1D2_zt25 * _J1_D1D2_zt31);
    const double _J1_D1D2_zt34 = (_J1_D1D2_zt30 - _J1_D1D2_zt16);
    const double _J1_D1D2_zt0 = (gRdC5 * _J1_D1D2_zt6);
    const double _J1_D1D2_zt1 = (gRdC5 * _J1_D1D2_zt34);
    const double _J1_D1D2_zt3 = (_J1_D1D2_zt43 * _J1_D1D2_zt0);
    const double _J1_D1D2_zt9 = (_J1_D1D2_zt43 * _J1_D1D2_zt10);
    const double _J1_D1D2_zt20 = (-_J1_D1D2_zt24);
    const double _J1_D1D2_zt21 = (_J1_D1D2_zt43 * _J1_D1D2_zt22);
    const double _J1_D1D2_zt29 = (_J1_D1D2_zt43 * _J1_D1D2_zt1);
    const double _J1_D1D2_zt33 = (_J1_D1D2_zt42 * _J1_D1D2_zt3);
    const double _J1_D1D2_zt51 = (gC13 * _J1_D1D2_zt3);
    const double _J1_D1D2_zt8 = (_J1_D1D2_zt42 * _J1_D1D2_zt29);
    const double _J1_D1D2_zt14 = (gC12 * _J1_D1D2_zt51);
    const double _J1_D1D2_zt15 = (gC13 * _J1_D1D2_zt29);
    const double _J1_D1D2_zt17 = (gC12 * _J1_D1D2_zt33);
    const double _J1_D1D2_zt19 = (gRdC5 * _J1_D1D2_zt20);
    const double _J1_D1D2_zt28 = (_J1_D1D2_zt52 * _J1_D1D2_zt15);
    const double _J1_D1D2_zt7 = (_J1_D1D2_zt52 * _J1_D1D2_zt8);
    const double _J1_D1D2_zt18 = (_J1_D1D2_zt43 * _J1_D1D2_zt19);
    const double _J1_D1D2_zt27 = (_J1_D1D2_zt14 - _J1_D1D2_zt28);
    const double _J1_D1D2_zt2 = (gC13 * _J1_D1D2_zt27);
    const double _J1_D1D2_zt5 = (_J1_D1D2_zt17 - _J1_D1D2_zt7);
    const double _J1_D1D2_zt32 = (_J1_D1D2_zt23 * _J1_D1D2_zt5);
    const double _J1_D1D2_zt13 = (_J1_D1D2_zt2 - _J1_D1D2_zt32);
    const double _J1_D1D2_Z0_0 = (-((-(_J1_D1D2_zt23 * ((gC12 * (_J1_D1D2_zt3 * 1.0)) - (_J1_D1D2_zt52 * (_J1_D1D2_zt29 * 1.0))))) / _J1_D1D2_zt13));
    const double _J1_D1D2_Z0_1 = (((gC13 * (-(gC12 * (gC13 * _J1_D1D2_zt9)))) - (_J1_D1D2_zt23 * (-(gC12 * (_J1_D1D2_zt42 * _J1_D1D2_zt9))))) / _J1_D1D2_zt13);
    const double _J1_D1D2_Z1_1 = (((gC13 * (-((_J1_D1D2_zt52 * (gC13 * _J1_D1D2_zt18)) + (gC12 * (gC13 * _J1_D1D2_zt21))))) - (_J1_D1D2_zt23 * (-((_J1_D1D2_zt52 * (_J1_D1D2_zt42 * _J1_D1D2_zt18)) + (gC12 * (_J1_D1D2_zt42 * _J1_D1D2_zt21)))))) / _J1_D1D2_zt13);
    double c0__J1_D1D2_voc0;
    double c__J1_D1D2_voc0[10];
    double c0__J1_D1D2_voc1;
    double c__J1_D1D2_voc1[10];
    double c0__J1_D1D2_zt72;
    double c__J1_D1D2_zt72[10];
    double c0__J1_D1D2_zt44;
    double c__J1_D1D2_zt44[10];
    
    for (int _k = 0; _k <= 10; ++_k)
    {
        const double vi = (_k == 1) ? 1.0 : 0.0;
        const double zC4 = (_k == 2) ? 1.0 : 0.0;
        const double zC12 = (_k == 3) ? 1.0 : 0.0;
        const double zR7C6 = (_k == 4) ? 1.0 : 0.0;
        const double zC13 = (_k == 5) ? 1.0 : 0.0;
        const double zR6C3 = (_k == 6) ? 1.0 : 0.0;
        const double zC11 = (_k == 7) ? 1.0 : 0.0;
        const double zR8C7 = (_k == 8) ? 1.0 : 0.0;
        const double zRdC5 = (_k == 9) ? 1.0 : 0.0;
        const double zR9C9 = (_k == 10) ? 1.0 : 0.0;
        const double _J1_D1D2_zt36 = (gR6C3 * vi);
        const double _J1_D1D2_zt38 = (gRfR10 * gR9C9);
        const double _J1_D1D2_zt41 = (gRfR10 * zR9C9);
        const double _J1_D1D2_zt57 = (zC12 + zC11);
        const double _J1_D1D2_zt58 = (_J1_D1D2_zt25 * zR9C9);
        const double _J1_D1D2_zt59 = (_J1_D1D2_zt25 * gR9C9);
        const double _J1_D1D2_zt69 = (gC12 * gR9C9);
        const double _J1_D1D2_zt71 = (gC12 * gRfR10);
        const double _J1_D1D2_zt77 = (zR7C6 + zR8C7);
        const double _J1_D1D2_zt78 = (gC12 * zR9C9);
        const double _J1_D1D2_zt80 = (gR7C6 + gR8C7);
        const double _J1_D1D2_zt35 = (_J1_D1D2_zt36 + zR6C3);
        const double _J1_D1D2_zt37 = (_J1_D1D2_zt80 + gRdC5);
        const double _J1_D1D2_zt46 = (_J1_D1D2_zt37 * _J1_D1D2_zt38);
        const double _J1_D1D2_zt49 = (_J1_D1D2_zt57 * _J1_D1D2_zt31);
        const double _J1_D1D2_zt54 = (gRdC5 * _J1_D1D2_zt71);
        const double _J1_D1D2_zt56 = (_J1_D1D2_zt57 * gRfR10);
        const double _J1_D1D2_zt61 = (zC4 + _J1_D1D2_zt35);
        const double _J1_D1D2_zt62 = (_J1_D1D2_zt37 * _J1_D1D2_zt59);
        const double _J1_D1D2_zt66 = (gRdC5 * _J1_D1D2_zt78);
        const double _J1_D1D2_zt68 = (_J1_D1D2_zt77 + zRdC5);
        const double _J1_D1D2_zt75 = (_J1_D1D2_zt56 - _J1_D1D2_zt58);
        const double _J1_D1D2_zt76 = (_J1_D1D2_zt68 * _J1_D1D2_zt59);
        const double _J1_D1D2_zt79 = (_J1_D1D2_zt37 * _J1_D1D2_zt69);
        const double _J1_D1D2_zt45 = (_J1_D1D2_zt61 * _J1_D1D2_zt46);
        const double _J1_D1D2_zt48 = (_J1_D1D2_zt49 - _J1_D1D2_zt41);
        const double _J1_D1D2_zt50 = (_J1_D1D2_zt68 * _J1_D1D2_zt38);
        const double _J1_D1D2_zt53 = (_J1_D1D2_zt43 * _J1_D1D2_zt54);
        const double _J1_D1D2_zt55 = (gRdC5 * _J1_D1D2_zt75);
        const double _J1_D1D2_zt60 = (_J1_D1D2_zt61 * _J1_D1D2_zt62);
        const double _J1_D1D2_zt67 = (_J1_D1D2_zt68 * _J1_D1D2_zt69);
        const double _J1_D1D2_zt70 = (_J1_D1D2_zt61 * _J1_D1D2_zt79);
        const double _J1_D1D2_zt74 = (_J1_D1D2_zt55 - _J1_D1D2_zt76);
        const double _J1_D1D2_zt40 = (gRdC5 * _J1_D1D2_zt48);
        const double _J1_D1D2_zt65 = (_J1_D1D2_zt66 + _J1_D1D2_zt67);
        const double _J1_D1D2_zt73 = (_J1_D1D2_zt43 * _J1_D1D2_zt74);
        const double _J1_D1D2_zt39 = (_J1_D1D2_zt40 - _J1_D1D2_zt50);
        const double _J1_D1D2_zt47 = (_J1_D1D2_zt43 * _J1_D1D2_zt39);
        const double _J1_D1D2_zt64 = (_J1_D1D2_zt43 * _J1_D1D2_zt65);
        const double _J1_D1D2_zt72 = (_J1_D1D2_zt73 + _J1_D1D2_zt60);
        const double _J1_D1D2_zt44 = (_J1_D1D2_zt45 + _J1_D1D2_zt47);
        const double _J1_D1D2_zt63 = (_J1_D1D2_zt64 - _J1_D1D2_zt70);
        const double _J1_D1D2_voc0 = ((((gC13 * ((zC12 * _J1_D1D2_zt15) - (gC12 * (gC13 * _J1_D1D2_zt44)))) - (_J1_D1D2_zt23 * ((zC12 * _J1_D1D2_zt8) - (gC12 * (_J1_D1D2_zt42 * _J1_D1D2_zt44))))) / _J1_D1D2_zt13) - (((zC13 * _J1_D1D2_zt27) - (_J1_D1D2_zt23 * ((gC12 * (_J1_D1D2_zt3 * zC13)) - (_J1_D1D2_zt52 * (_J1_D1D2_zt29 * zC13))))) / _J1_D1D2_zt13));
        const double _J1_D1D2_voc1 = (((gC13 * ((zC12 * (gC13 * _J1_D1D2_zt53)) - ((_J1_D1D2_zt52 * (gC13 * _J1_D1D2_zt72)) + (gC12 * (gC13 * _J1_D1D2_zt63))))) - (_J1_D1D2_zt23 * ((zC12 * (_J1_D1D2_zt42 * _J1_D1D2_zt53)) - ((_J1_D1D2_zt52 * (_J1_D1D2_zt42 * _J1_D1D2_zt72)) + (gC12 * (_J1_D1D2_zt42 * _J1_D1D2_zt63)))))) / _J1_D1D2_zt13);
        if (_k == 0) {
            c0__J1_D1D2_voc0 = _J1_D1D2_voc0;
            c0__J1_D1D2_voc1 = _J1_D1D2_voc1;
            c0__J1_D1D2_zt72 = _J1_D1D2_zt72;
            c0__J1_D1D2_zt44 = _J1_D1D2_zt44;
        } else {
            c__J1_D1D2_voc0[_k - 1] = _J1_D1D2_voc0 - c0__J1_D1D2_voc0;
            c__J1_D1D2_voc1[_k - 1] = _J1_D1D2_voc1 - c0__J1_D1D2_voc1;
            c__J1_D1D2_zt72[_k - 1] = _J1_D1D2_zt72 - c0__J1_D1D2_zt72;
            c__J1_D1D2_zt44[_k - 1] = _J1_D1D2_zt44 - c0__J1_D1D2_zt44;
        }
    }
    
    for (int ch = 0; ch < num_channels; ++ch)
    {
        double zC4 = state[ch].zC4;
        double zC12 = state[ch].zC12;
        double zR7C6 = state[ch].zR7C6;
        double zC13 = state[ch].zC13;
        double zR6C3 = state[ch].zR6C3;
        double zC11 = state[ch].zC11;
        double zR8C7 = state[ch].zR8C7;
        double zRdC5 = state[ch].zRdC5;
        double zR9C9 = state[ch].zR9C9;
        double vGSJ1 = state[ch].vGSJ1;
        double vGSJ1_prev = state[ch].vGSJ1_prev;
        double vD1D2 = state[ch].vD1D2;
        double vD1D2_prev = state[ch].vD1D2_prev;
        for (int n = 0; n < num_samples; ++n)
        {
            const double vi = input[ch][n];

            { const double _prev_step = vGSJ1 - vGSJ1_prev; vGSJ1_prev = vGSJ1;
vGSJ1 = limit_jfet_vgs(vGSJ1 + (_prev_step), _2N5485_vp);
            }
            { const double _prev_step = vD1D2 - vD1D2_prev; vD1D2_prev = vD1D2;
vD1D2 = limit_junction_voltage_sym(vD1D2 + (_prev_step), vD1D2, D1N914_vt, vcrit_D1N914_vt);
            }

            // --- Newton-Raphson solve (N-port): J1_D1D2
            const double _J1_D1D2_voc0 = c0__J1_D1D2_voc0 + c__J1_D1D2_voc0[0] * vi + c__J1_D1D2_voc0[1] * zC4 + c__J1_D1D2_voc0[2] * zC12 + c__J1_D1D2_voc0[3] * zR7C6 + c__J1_D1D2_voc0[4] * zC13 + c__J1_D1D2_voc0[5] * zR6C3 + c__J1_D1D2_voc0[6] * zC11 + c__J1_D1D2_voc0[7] * zR8C7 + c__J1_D1D2_voc0[8] * zRdC5 + c__J1_D1D2_voc0[9] * zR9C9;
            const double _J1_D1D2_voc1 = c0__J1_D1D2_voc1 + c__J1_D1D2_voc1[0] * vi + c__J1_D1D2_voc1[1] * zC4 + c__J1_D1D2_voc1[2] * zC12 + c__J1_D1D2_voc1[3] * zR7C6 + c__J1_D1D2_voc1[4] * zC13 + c__J1_D1D2_voc1[5] * zR6C3 + c__J1_D1D2_voc1[6] * zC11 + c__J1_D1D2_voc1[7] * zR8C7 + c__J1_D1D2_voc1[8] * zRdC5 + c__J1_D1D2_voc1[9] * zR9C9;
            const double _J1_D1D2_zt72 = c0__J1_D1D2_zt72 + c__J1_D1D2_zt72[0] * vi + c__J1_D1D2_zt72[1] * zC4 + c__J1_D1D2_zt72[2] * zC12 + c__J1_D1D2_zt72[3] * zR7C6 + c__J1_D1D2_zt72[4] * zC13 + c__J1_D1D2_zt72[5] * zR6C3 + c__J1_D1D2_zt72[6] * zC11 + c__J1_D1D2_zt72[7] * zR8C7 + c__J1_D1D2_zt72[8] * zRdC5 + c__J1_D1D2_zt72[9] * zR9C9;
            const double _J1_D1D2_zt44 = c0__J1_D1D2_zt44 + c__J1_D1D2_zt44[0] * vi + c__J1_D1D2_zt44[1] * zC4 + c__J1_D1D2_zt44[2] * zC12 + c__J1_D1D2_zt44[3] * zR7C6 + c__J1_D1D2_zt44[4] * zC13 + c__J1_D1D2_zt44[5] * zR6C3 + c__J1_D1D2_zt44[6] * zC11 + c__J1_D1D2_zt44[7] * zR8C7 + c__J1_D1D2_zt44[8] * zRdC5 + c__J1_D1D2_zt44[9] * zR9C9;
            const double _J1_D1D2_zt36 = (gR6C3 * vi);
            const double _J1_D1D2_zt38 = (gRfR10 * gR9C9);
            const double _J1_D1D2_zt41 = (gRfR10 * zR9C9);
            const double _J1_D1D2_zt57 = (zC12 + zC11);
            const double _J1_D1D2_zt58 = (_J1_D1D2_zt25 * zR9C9);
            const double _J1_D1D2_zt59 = (_J1_D1D2_zt25 * gR9C9);
            const double _J1_D1D2_zt69 = (gC12 * gR9C9);
            const double _J1_D1D2_zt71 = (gC12 * gRfR10);
            const double _J1_D1D2_zt77 = (zR7C6 + zR8C7);
            const double _J1_D1D2_zt78 = (gC12 * zR9C9);
            const double _J1_D1D2_zt80 = (gR7C6 + gR8C7);
            const double _J1_D1D2_zt35 = (_J1_D1D2_zt36 + zR6C3);
            const double _J1_D1D2_zt37 = (_J1_D1D2_zt80 + gRdC5);
            const double _J1_D1D2_zt46 = (_J1_D1D2_zt37 * _J1_D1D2_zt38);
            const double _J1_D1D2_zt49 = (_J1_D1D2_zt57 * _J1_D1D2_zt31);
            const double _J1_D1D2_zt54 = (gRdC5 * _J1_D1D2_zt71);
            const double _J1_D1D2_zt56 = (_J1_D1D2_zt57 * gRfR10);
            const double _J1_D1D2_zt61 = (zC4 + _J1_D1D2_zt35);
            const double _J1_D1D2_zt62 = (_J1_D1D2_zt37 * _J1_D1D2_zt59);
            const double _J1_D1D2_zt66 = (gRdC5 * _J1_D1D2_zt78);
            const double _J1_D1D2_zt68 = (_J1_D1D2_zt77 + zRdC5);
            const double _J1_D1D2_zt75 = (_J1_D1D2_zt56 - _J1_D1D2_zt58);
            const double _J1_D1D2_zt76 = (_J1_D1D2_zt68 * _J1_D1D2_zt59);
            const double _J1_D1D2_zt79 = (_J1_D1D2_zt37 * _J1_D1D2_zt69);
            const double _J1_D1D2_zt45 = (_J1_D1D2_zt61 * _J1_D1D2_zt46);
            const double _J1_D1D2_zt48 = (_J1_D1D2_zt49 - _J1_D1D2_zt41);
            const double _J1_D1D2_zt50 = (_J1_D1D2_zt68 * _J1_D1D2_zt38);
            const double _J1_D1D2_zt53 = (_J1_D1D2_zt43 * _J1_D1D2_zt54);
            const double _J1_D1D2_zt55 = (gRdC5 * _J1_D1D2_zt75);
            const double _J1_D1D2_zt60 = (_J1_D1D2_zt61 * _J1_D1D2_zt62);
            const double _J1_D1D2_zt67 = (_J1_D1D2_zt68 * _J1_D1D2_zt69);
            const double _J1_D1D2_zt70 = (_J1_D1D2_zt61 * _J1_D1D2_zt79);
            const double _J1_D1D2_zt74 = (_J1_D1D2_zt55 - _J1_D1D2_zt76);
            const double _J1_D1D2_zt40 = (gRdC5 * _J1_D1D2_zt48);
            const double _J1_D1D2_zt65 = (_J1_D1D2_zt66 + _J1_D1D2_zt67);
            const double _J1_D1D2_zt73 = (_J1_D1D2_zt43 * _J1_D1D2_zt74);
            const double _J1_D1D2_zt39 = (_J1_D1D2_zt40 - _J1_D1D2_zt50);
            const double _J1_D1D2_zt47 = (_J1_D1D2_zt43 * _J1_D1D2_zt39);
            const double _J1_D1D2_zt64 = (_J1_D1D2_zt43 * _J1_D1D2_zt65);
            const double _J1_D1D2_zt63 = (_J1_D1D2_zt64 - _J1_D1D2_zt70);
            const double _J1_D1D2_pt0 = (1.0 / 1000.0);
            const double _J1_D1D2_pt11 = (1.0 / D1N914_vt);
            for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
            {
                const double _J1_D1D2_pt2 = (vGSJ1 - _2N5485_vp);
                const double _J1_D1D2_pt4 = (vD1D2 / D1N914_vt);
                const double _J1_D1D2_pt1 = (_J1_D1D2_pt2 + _J1_D1D2_pt0);
                const double _J1_D1D2_pt3 = math_exp_approx(_J1_D1D2_pt4);
                const double _J1_D1D2_i0 = (_2N5485_Beta * (_J1_D1D2_pt1 * _J1_D1D2_pt1));
                const double _J1_D1D2_i1 = (D1N914_Is * (_J1_D1D2_pt3 - (1.0 / _J1_D1D2_pt3)));
                const double _J1_D1D2_g0_0 = (_2N5485_Beta * (_J1_D1D2_pt1 + _J1_D1D2_pt1));
                const double _J1_D1D2_g1_1 = (D1N914_Is * ((_J1_D1D2_pt3 / D1N914_vt) + (_J1_D1D2_pt11 / _J1_D1D2_pt3)));
                const double _J1_D1D2_pt5 = ((_J1_D1D2_voc0 + (_J1_D1D2_Z0_0 * _J1_D1D2_i0)) + (_J1_D1D2_Z0_1 * _J1_D1D2_i1));
                const double _J1_D1D2_pt6 = (_J1_D1D2_voc1 + (_J1_D1D2_Z1_1 * _J1_D1D2_i1));
                const double _J1_D1D2_pt7 = ((_J1_D1D2_Z1_1 * _J1_D1D2_g1_1) - 1.0);
                const double _J1_D1D2_pt8 = (vD1D2 - _J1_D1D2_pt6);
                const double _J1_D1D2_pt10 = ((_J1_D1D2_Z0_0 * _J1_D1D2_g0_0) - 1.0);
                const double _J1_D1D2_pt9 = (_J1_D1D2_pt10 * _J1_D1D2_pt7);
                const double res_vGSJ1 = (_J1_D1D2_pt5 - vGSJ1);
                const double res_vD1D2 = (_J1_D1D2_pt6 - vD1D2);
                const double delta_vGSJ1 = ((((vGSJ1 - _J1_D1D2_pt5) * _J1_D1D2_pt7) - ((_J1_D1D2_Z0_1 * _J1_D1D2_g1_1) * _J1_D1D2_pt8)) / _J1_D1D2_pt9);
                const double delta_vD1D2 = ((_J1_D1D2_pt10 * _J1_D1D2_pt8) / _J1_D1D2_pt9);
            
                double residual_norm_sq = 0.0;
                residual_norm_sq += res_vGSJ1 * res_vGSJ1;
                residual_norm_sq += res_vD1D2 * res_vD1D2;
            
                vGSJ1 = limit_jfet_vgs(vGSJ1 + (delta_vGSJ1), _2N5485_vp);
                vD1D2 = limit_junction_voltage_sym(vD1D2 + (delta_vD1D2), vD1D2, D1N914_vt, vcrit_D1N914_vt);
            
                if (residual_norm_sq < newton_tol_sq)
                    break;
                
            }

            const double vRdC5 = c0_vRdC5 + c_vRdC5[0] * vi + c_vRdC5[1] * zC4 + c_vRdC5[2] * zC12 + c_vRdC5[3] * zR7C6 + c_vRdC5[4] * zC13 + c_vRdC5[5] * zR6C3 + c_vRdC5[6] * zC11 + c_vRdC5[7] * zR8C7 + c_vRdC5[8] * zRdC5 + c_vRdC5[9] * zR9C9;
            const double _t16 = ((vGSJ1 - _2N5485_vp) + _t55);
            const double _t25 = (zC4 + ((gR6C3 * vi) + zR6C3));
            const double _t31 = (zC12 + zC11);
            const double _t34 = math_exp_approx((vD1D2 / D1N914_vt));
            const double _t36 = ((zR7C6 + zR8C7) + zRdC5);
            const double _t43 = (_t31 * _t7);
            const double _t15 = (zC13 + (_2N5485_Beta * (_t16 * _t16)));
            const double _t33 = (zR9C9 + (D1N914_Is * (_t34 - (1.0 / _t34))));
            const double _t35 = (_t36 * _t41);
            const double _t46 = ((_t4 * (_t36 * _t12)) - (_t25 * _t56));
            const double _t47 = ((_t4 * (_t36 * _t6)) - (_t25 * _t57));
            const double _t49 = ((_t4 * ((gRdC5 * ((_t31 * gRfR10) - (_t13 * _t33))) - (_t36 * _t50))) + (_t25 * _t58));
            const double _t51 = ((_t4 * ((gRdC5 * (gC12 * _t33)) + (_t36 * _t52))) - (_t25 * _t59));
            const double _t24 = (_t25 * _t5);
            const double _t26 = (_t25 * _t11);
            const double _t32 = (gRfR10 * _t33);
            const double _t39 = (_t25 * _t40);
            const double _t42 = (_t43 - _t32);
            const double _t18 = (_t15 * _t3);
            const double _t30 = (gRdC5 * _t42);
            const double _t44 = (_t15 * _t10);
            const double _t14 = (_t8 * _t44);
            const double _t17 = (gC12 * _t18);
            const double _t29 = (_t30 - _t35);
            const double _t28 = (_t4 * _t29);
            const double _t38 = (_t39 + _t28);
            const double _t27 = (_t28 + _t39);
            const double vo = (((zC13 * _t0) + (gC13 * (_t14 - _t17))) * _t19);
            const double vneg = (((gC13 * ((gC12 * (gC13 * _t24)) - (_t8 * (gC13 * _t26)))) - (_t23 * ((gC12 * (_t2 * _t24)) - (_t8 * (_t2 * _t26))))) * _t19);
            const double vf = (((gC13 * ((zC12 * _t21) - (_t8 * (gC13 * _t27)))) - (_t23 * ((zC12 * _t1) - (_t8 * (_t2 * _t27))))) * _t19);
            const double vg = (((gC13 * ((zC12 * _t22) - (gC12 * (gC13 * _t38)))) - (_t23 * ((zC12 * _t9) - (gC12 * (_t2 * _t38))))) * _t19);
            const double tC12 = (gC12 * (vf - vg));
            const double vs = (((zC13 * _t20) - (_t23 * (_t17 - _t14))) * _t19);
            const double tC13 = (gC13 * (vs - vo));
            const double vR6C3 = (vneg - vi);
            const double tC11 = (gC11 * (vf - 0));
            const double v2 = (((gC13 * ((_t8 * (gC13 * _t46)) - (gC12 * (gC13 * _t47)))) - (_t23 * ((_t8 * (_t2 * _t46)) - (gC12 * (_t2 * _t47))))) * _t19);
            const double vdp = (((gC13 * ((zC12 * _t53) - ((_t8 * (gC13 * _t49)) + (gC12 * (gC13 * _t51))))) - (_t23 * ((zC12 * _t54) - ((_t8 * (_t2 * _t49)) + (gC12 * (_t2 * _t51)))))) * _t19);
            const double vR9C9 = (v2 - vdp);
            const double _t45 = (vneg - 0);
            const double tC4 = (gC4 * _t45);
            const double vR7C6 = _t45;
            const double vR8C7 = _t45;
            
            zC4 = 2 * tC4 - zC4;
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
        state[ch].zC12 = zC12;
        state[ch].zR7C6 = zR7C6;
        state[ch].zC13 = zC13;
        state[ch].zR6C3 = zR6C3;
        state[ch].zC11 = zC11;
        state[ch].zR8C7 = zR8C7;
        state[ch].zRdC5 = zRdC5;
        state[ch].zR9C9 = zR9C9;
        state[ch].vGSJ1 = vGSJ1;
        state[ch].vGSJ1_prev = vGSJ1_prev;
        state[ch].vD1D2 = vD1D2;
        state[ch].vD1D2_prev = vD1D2_prev;
    }
}

static float reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc)
{
    #define sum(a, b) ((a) + (b))
    #define recip_sum(a, b) ((a) * (b) / ((a) + (b)))
    
    const double _2N5485_Beta = params._2N5485_Beta;
    const double _2N5485_vp = params._2N5485_vp;
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

    const double _J1_Z0_0 = (-(1.0 / (gR13 + (1.0 / 1000000000.0))));
    // --- Newton-Raphson solve (N-port): J1
    const double _J1_pt1 = (1.0 / 1000.0);
    for (int newton_iter = 0; newton_iter < 10000; ++newton_iter)
    {
        const double _J1_pt0 = (vGSJ1 - _2N5485_vp);
        const double _J1_pt2 = (_J1_pt0 + _J1_pt1);
        const double _J1_i0 = (_2N5485_Beta * (_J1_pt2 * _J1_pt2));
        const double _J1_g0_0 = (_2N5485_Beta * (_J1_pt2 + _J1_pt2));
        const double _J1_pt3 = (_J1_Z0_0 * _J1_i0);
        const double res_vGSJ1 = (_J1_pt3 - vGSJ1);
        const double delta_vGSJ1 = ((vGSJ1 - _J1_pt3) / ((_J1_Z0_0 * _J1_g0_0) - 1.0));
    
        double residual_norm_sq = 0.0;
        residual_norm_sq += res_vGSJ1 * res_vGSJ1;
    
        vGSJ1 = limit_jfet_vgs(vGSJ1 + (delta_vGSJ1), _2N5485_vp);
    
        if (residual_norm_sq < newton_tol_sq)
            break;
        
    }
    const double _D1D2_zt1 = (1.0 / 1000000000.0);
    const double _D1D2_zt0 = (gRfR10 + _D1D2_zt1);
    const double _D1D2_Z0_0 = (-((_D1D2_zt0 * 1.0) / ((_D1D2_zt0 * _D1D2_zt0) - (gRfR10 * gRfR10))));
    // --- Newton-Raphson solve (N-port): D1D2
    const double _D1D2_pt3 = (1.0 / D1N914_vt);
    for (int newton_iter = 0; newton_iter < 10000; ++newton_iter)
    {
        const double _D1D2_pt0 = (vD1D2 / D1N914_vt);
        const double _D1D2_pt1 = math_exp_approx(_D1D2_pt0);
        const double _D1D2_i0 = (D1N914_Is * (_D1D2_pt1 - (1.0 / _D1D2_pt1)));
        const double _D1D2_g0_0 = (D1N914_Is * ((_D1D2_pt1 / D1N914_vt) + (_D1D2_pt3 / _D1D2_pt1)));
        const double _D1D2_pt2 = (_D1D2_Z0_0 * _D1D2_i0);
        const double res_vD1D2 = (_D1D2_pt2 - vD1D2);
        const double delta_vD1D2 = ((vD1D2 - _D1D2_pt2) / ((_D1D2_Z0_0 * _D1D2_g0_0) - 1.0));
    
        double residual_norm_sq = 0.0;
        residual_norm_sq += res_vD1D2 * res_vD1D2;
    
        vD1D2 = limit_junction_voltage_sym(vD1D2 + (delta_vD1D2), vD1D2, D1N914_vt, vcrit_D1N914_vt);
    
        if (residual_norm_sq < newton_tol_sq)
            break;
        
    }
    const double zC4 = 0.0;
    const double zC12 = (-((gC12 * ((gRfR10 * D1N914_Is) * (math_exp_approx((vD1D2 / D1N914_vt)) - (1.0 / math_exp_approx((vD1D2 / D1N914_vt)))))) / (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))));
    const double zR7C6 = 0.0;
    const double zC13 = ((gC13 * ((_2N5485_Beta * ((vGSJ1 - _2N5485_vp) + (1.0 / 1000.0))) * ((vGSJ1 - _2N5485_vp) + (1.0 / 1000.0)))) / (gR13 + (1.0 / 1000000000.0)));
    const double zR6C3 = (-(gR6C3 * vi));
    const double zC11 = (-((gC11 * ((gRfR10 * D1N914_Is) * (math_exp_approx((vD1D2 / D1N914_vt)) - (1.0 / math_exp_approx((vD1D2 / D1N914_vt)))))) / (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))));
    const double zR8C7 = 0.0;
    const double zRdC5 = 0.0;
    const double zR9C9 = ((gR9C9 * (((gRfR10 + (1.0 / 1000000000.0)) * D1N914_Is) * (math_exp_approx((vD1D2 / D1N914_vt)) - (1.0 / math_exp_approx((vD1D2 / D1N914_vt)))))) / (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)));

    const double vo_dc_out = 0.0;

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].vGSJ1 = vGSJ1;
        state[ch].vGSJ1_prev = vGSJ1;
        state[ch].vD1D2 = vD1D2;
        state[ch].vD1D2_prev = vD1D2;
        state[ch].zC4 = zC4;
        state[ch].zC12 = zC12;
        state[ch].zR7C6 = zR7C6;
        state[ch].zC13 = zC13;
        state[ch].zR6C3 = zR6C3;
        state[ch].zC11 = zC11;
        state[ch].zR8C7 = zR8C7;
        state[ch].zRdC5 = zRdC5;
        state[ch].zR9C9 = zR9C9;
    }
    return vo_dc_out;
}

