// Auto-generated with netlist_codegen version de49196.
// Command: netlist_codegen tube_compressor.net tube_compressor.h -opt_port_matrix -type_name double -instrument

#pragma once

#include <cmath>
#include <cstdint>

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


[[maybe_unused]] static auto limit_junction_voltage = [](auto v_new, auto v_old, auto vt, auto vcrit)
{
    if (v_new > vcrit && std::abs(v_new - v_old) > 2 * vt)
    {
        if (v_old > 0)
        {
            const auto arg = 1 + (v_new - v_old) / vt;
            v_new = arg > 0 ? v_old + vt * math_log_approx(arg) : vcrit;
        }
        else
        {
            v_new = vt * math_log_approx(v_new / vt);
        }
    }
    return v_new;
};

static constexpr auto newton_tol_sq = 0.00001;
static constexpr int newton_max_iter = 20;


struct Params {
    double VCC = 2.5e+02;
    double Cin = 1.0e-07;
    double Rg1 = 4.7e+05;
    double Rk1 = 2.2e+03;
    double Ck1 = 1.0e-05;
    double Rfb = 4.7e+04;
    double Rp1 = 1.0e+05;
    double _12AX7_DEMPWOLF_Gk = 2.242e-03;
    double _12AX7_DEMPWOLF_Mu = 1.032e+02;
    double _12AX7_DEMPWOLF_Gamma = 1.26e+00;
    double _12AX7_DEMPWOLF_Ck = 3.4e+00;
    double _12AX7_DEMPWOLF_Gg = 6.177e-04;
    double _12AX7_DEMPWOLF_Xi = 1.314e+00;
    double _12AX7_DEMPWOLF_Cg = 9.901e+00;
    double _12AX7_DEMPWOLF_Ig0 = 8.025e-08;
    double Cc12 = 4.7e-08;
    double Rg2 = 2.2e+05;
    double Ratt = 1.0e+04;
    double Rrel = 3.3e+05;
    double Cenv = 1.0e-06;
    double Rp2 = 4.7e+04;
    double Cout = 1.0e-07;
    double RL = 1.0e+05;
    double Csc = 1.0e-07;
    double D1N914_Is = 2.52e-09;
    double D1N914_vt = 4.52892e-02;
    double Rk2 = 1.5e+03;
    double Ck2 = 1.0e-05;
};

struct State {
    double zCin {};
    double zRk1Ck1 {};
    double zCc12 {};
    double zRrelCenv {};
    double zCout {};
    double zCsc {};
    double zRk2Ck2 {};
    double vPKX1 {};
    double vGKX1 {};
    double vPKX2 {};
    double vGKX2 {};
    double vD1 {};
    long long nr_solves_X1_X2_D1 {};
    long long nr_iters_X1_X2_D1 {};
};

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto _12AX7_DEMPWOLF_Gk = params._12AX7_DEMPWOLF_Gk;
    const auto _12AX7_DEMPWOLF_Mu = params._12AX7_DEMPWOLF_Mu;
    const auto _12AX7_DEMPWOLF_Gamma = params._12AX7_DEMPWOLF_Gamma;
    const auto _12AX7_DEMPWOLF_Ck = params._12AX7_DEMPWOLF_Ck;
    const auto _12AX7_DEMPWOLF_Gg = params._12AX7_DEMPWOLF_Gg;
    const auto _12AX7_DEMPWOLF_Xi = params._12AX7_DEMPWOLF_Xi;
    const auto _12AX7_DEMPWOLF_Cg = params._12AX7_DEMPWOLF_Cg;
    const auto _12AX7_DEMPWOLF_Ig0 = params._12AX7_DEMPWOLF_Ig0;
    const auto D1N914_Is = params.D1N914_Is;
    const auto D1N914_vt = params.D1N914_vt;
    
    const auto VCC = params.VCC;
    
    const auto vp = VCC;
    
    const auto gCin = 2.0 * sample_rate * params.Cin;
    
    const auto gRg1 = 1.0 / params.Rg1;
    
    const auto Rk1 = params.Rk1;
    const auto Ck1 = params.Ck1;
    const auto gRk1Ck1 = 2.0 * sample_rate * Ck1 + (1.0 / Rk1);
    const auto gzRk1Ck1 = 4.0 * sample_rate * Ck1;
    
    const auto gRfb = 1.0 / params.Rfb;
    
    const auto gRp1 = 1.0 / params.Rp1;
    
    const auto gCc12 = 2.0 * sample_rate * params.Cc12;
    
    const auto gRg2 = 1.0 / params.Rg2;
    
    const auto gRatt = 1.0 / params.Ratt;
    
    const auto Rrel = params.Rrel;
    const auto Cenv = params.Cenv;
    const auto gRrelCenv = 2.0 * sample_rate * Cenv + (1.0 / Rrel);
    const auto gzRrelCenv = 4.0 * sample_rate * Cenv;
    
    const auto gRp2 = 1.0 / params.Rp2;
    
    const auto gCout = 2.0 * sample_rate * params.Cout;
    
    const auto gRL = 1.0 / params.RL;
    
    const auto gCsc = 2.0 * sample_rate * params.Csc;
    
    const auto Rk2 = params.Rk2;
    const auto Ck2 = params.Ck2;
    const auto gRk2Ck2 = 2.0 * sample_rate * Ck2 + (1.0 / Rk2);
    const auto gzRk2Ck2 = 4.0 * sample_rate * Ck2;
    
    const auto vcrit_D1N914_vt = D1N914_vt * std::log(D1N914_vt / (std::sqrt(2.0) * D1N914_Is));
    
    const auto _t0 = (gRk1Ck1 + gRfb);
    const auto _t2 = (gRp1 + gCc12);
    const auto _t4 = (gCc12 + gRg2);
    const auto _t7 = (gRp2 + gCout);
    const auto _t17 = (gCout * gCsc);
    const auto _t20 = ((gRfb + gRatt) + gRrelCenv);
    const auto _t35 = ((gCout + gRL) + gCsc);
    const auto _t16 = (gRk2Ck2 * _t17);
    const auto _t34 = ((_t35 * gCsc) - (gCsc * gCsc));
    const auto _t94 = (gRp2 * _t16);
    const auto _t19 = (gRatt * _t94);
    const auto _t33 = (gRk2Ck2 * _t34);
    const auto _t78 = (gRp2 * _t33);
    const auto _t93 = (_t4 * _t19);
    const auto _t100 = (_t2 * _t93);
    const auto _t101 = (gCc12 * _t19);
    const auto _t18 = (gCc12 * _t101);
    const auto _t26 = (_t18 - _t100);
    const auto _t32 = ((_t7 * _t33) - (gCout * _t16));
    const auto _t36 = (gRatt * _t32);
    const auto _t38 = (_t4 * _t36);
    const auto _t57 = (gRp1 * _t38);
    const auto _t77 = (gRatt * _t78);
    const auto _t85 = (gCc12 * _t77);
    const auto _t86 = (_t4 * _t77);
    const auto _t92 = (_t100 - _t18);
    const auto _t25 = (VCC * _t26);
    const auto _t31 = (gCc12 * _t36);
    const auto _t37 = (_t2 * _t38);
    const auto _t56 = (VCC * _t57);
    const auto _t60 = (gRp1 * _t31);
    const auto _t64 = (VCC * _t60);
    const auto _t79 = (_t2 * _t86);
    const auto _t84 = (gCc12 * _t85);
    const auto _t91 = (VCC * _t92);
    const auto _t30 = (gCc12 * _t31);
    const auto _t83 = (_t84 - _t79);
    const auto _t29 = (_t30 - _t37);
    const auto _t39 = (gRfb * _t29);
    const auto _t82 = (VCC * _t83);
    const auto _t28 = ((gRatt * _t29) - (_t20 * _t29));
    const auto _t27 = (1.0 / ((_t0 * _t28) + (gRfb * _t39)));
    const auto _t110 = (1.0 / (gCin + gRg1));
    const auto _X1_X2_D1_zt3 = (gRfb + gRatt);
    const auto _X1_X2_D1_zt6 = (gRp2 + gCout);
    const auto _X1_X2_D1_zt14 = (gCin + gRg1);
    const auto _X1_X2_D1_zt34 = (gRk1Ck1 + gRfb);
    const auto _X1_X2_D1_zt51 = (gCsc * 1.0);
    const auto _X1_X2_D1_zt54 = (gRk2Ck2 * _X1_X2_D1_zt51);
    const auto _X1_X2_D1_zt59 = (gCout * gCsc);
    const auto _X1_X2_D1_zt63 = (_X1_X2_D1_zt3 + gRrelCenv);
    const auto _X1_X2_D1_zt64 = (gCsc * gCsc);
    const auto _X1_X2_D1_zt86 = (gCc12 + gRg2);
    const auto _X1_X2_D1_zt94 = (gCout * 1.0);
    const auto _X1_X2_D1_zt102 = (gRk2Ck2 * _X1_X2_D1_zt94);
    const auto _X1_X2_D1_zt103 = (gRp1 + gCc12);
    const auto _X1_X2_D1_zt155 = (gCout + gRL);
    const auto _X1_X2_D1_zt26 = (_X1_X2_D1_zt155 + gCsc);
    const auto _X1_X2_D1_zt31 = (gRk2Ck2 * _X1_X2_D1_zt59);
    const auto _X1_X2_D1_zt44 = (_X1_X2_D1_zt59 * 1.0);
    const auto _X1_X2_D1_zt50 = (gCout * _X1_X2_D1_zt54);
    const auto _X1_X2_D1_zt82 = (_X1_X2_D1_zt31 * 1.0);
    const auto _X1_X2_D1_zt89 = (gCout * _X1_X2_D1_zt31);
    const auto _X1_X2_D1_zt93 = (gCout * _X1_X2_D1_zt102);
    const auto _X1_X2_D1_zt96 = (_X1_X2_D1_zt26 * 1.0);
    const auto _X1_X2_D1_zt4 = (_X1_X2_D1_zt26 * gCsc);
    const auto _X1_X2_D1_zt38 = (gCout * _X1_X2_D1_zt44);
    const auto _X1_X2_D1_zt48 = (gRatt * _X1_X2_D1_zt50);
    const auto _X1_X2_D1_zt53 = (_X1_X2_D1_zt86 * _X1_X2_D1_zt48);
    const auto _X1_X2_D1_zt81 = (-_X1_X2_D1_zt82);
    const auto _X1_X2_D1_zt85 = (gRatt * _X1_X2_D1_zt81);
    const auto _X1_X2_D1_zt95 = (gRk2Ck2 * _X1_X2_D1_zt96);
    const auto _X1_X2_D1_zt2 = (_X1_X2_D1_zt4 - _X1_X2_D1_zt64);
    const auto _X1_X2_D1_zt47 = (gCc12 * _X1_X2_D1_zt48);
    const auto _X1_X2_D1_zt49 = (gCc12 * _X1_X2_D1_zt47);
    const auto _X1_X2_D1_zt52 = (_X1_X2_D1_zt103 * _X1_X2_D1_zt53);
    const auto _X1_X2_D1_zt71 = (_X1_X2_D1_zt2 * 1.0);
    const auto _X1_X2_D1_zt78 = (gCc12 * _X1_X2_D1_zt85);
    const auto _X1_X2_D1_zt80 = (_X1_X2_D1_zt86 * _X1_X2_D1_zt85);
    const auto _X1_X2_D1_zt84 = (gCc12 * _X1_X2_D1_zt78);
    const auto _X1_X2_D1_zt92 = (_X1_X2_D1_zt6 * _X1_X2_D1_zt95);
    const auto _X1_X2_D1_zt196 = (gRk2Ck2 * _X1_X2_D1_zt2);
    const auto _X1_X2_D1_zt29 = (_X1_X2_D1_zt196 * 1.0);
    const auto _X1_X2_D1_zt42 = (_X1_X2_D1_zt6 * _X1_X2_D1_zt71);
    const auto _X1_X2_D1_zt46 = (_X1_X2_D1_zt52 - _X1_X2_D1_zt49);
    const auto _X1_X2_D1_zt55 = (_X1_X2_D1_zt6 * _X1_X2_D1_zt196);
    const auto _X1_X2_D1_zt73 = (_X1_X2_D1_zt55 - _X1_X2_D1_zt89);
    const auto _X1_X2_D1_zt79 = (_X1_X2_D1_zt103 * _X1_X2_D1_zt80);
    const auto _X1_X2_D1_zt83 = (_X1_X2_D1_zt84 - _X1_X2_D1_zt79);
    const auto _X1_X2_D1_zt88 = (gRatt * _X1_X2_D1_zt73);
    const auto _X1_X2_D1_zt91 = (_X1_X2_D1_zt93 - _X1_X2_D1_zt92);
    const auto _X1_X2_D1_zt101 = (gRatt * _X1_X2_D1_zt91);
    const auto _X1_X2_D1_zt110 = (_X1_X2_D1_zt86 * _X1_X2_D1_zt88);
    const auto _X1_X2_D1_zt0 = (_X1_X2_D1_zt110 * 1.0);
    const auto _X1_X2_D1_zt1 = (_X1_X2_D1_zt103 * _X1_X2_D1_zt110);
    const auto _X1_X2_D1_zt9 = (_X1_X2_D1_zt73 * 1.0);
    const auto _X1_X2_D1_zt13 = (gCc12 * _X1_X2_D1_zt88);
    const auto _X1_X2_D1_zt22 = (gCc12 * _X1_X2_D1_zt9);
    const auto _X1_X2_D1_zt25 = (gRatt * _X1_X2_D1_zt29);
    const auto _X1_X2_D1_zt28 = (_X1_X2_D1_zt86 * _X1_X2_D1_zt25);
    const auto _X1_X2_D1_zt30 = (gCc12 * _X1_X2_D1_zt25);
    const auto _X1_X2_D1_zt36 = (_X1_X2_D1_zt42 - _X1_X2_D1_zt38);
    const auto _X1_X2_D1_zt45 = (gRatt * _X1_X2_D1_zt36);
    const auto _X1_X2_D1_zt56 = (_X1_X2_D1_zt13 * 1.0);
    const auto _X1_X2_D1_zt66 = (_X1_X2_D1_zt88 * 1.0);
    const auto _X1_X2_D1_zt70 = (gCc12 * _X1_X2_D1_zt45);
    const auto _X1_X2_D1_zt90 = (gCc12 * _X1_X2_D1_zt101);
    const auto _X1_X2_D1_zt99 = (gCc12 * _X1_X2_D1_zt90);
    const auto _X1_X2_D1_zt100 = (_X1_X2_D1_zt86 * _X1_X2_D1_zt101);
    const auto _X1_X2_D1_zt195 = (gCc12 * _X1_X2_D1_zt13);
    const auto _X1_X2_D1_zt5 = (gCc12 * _X1_X2_D1_zt66);
    const auto _X1_X2_D1_zt8 = (_X1_X2_D1_zt86 * _X1_X2_D1_zt9);
    const auto _X1_X2_D1_zt20 = (_X1_X2_D1_zt103 * _X1_X2_D1_zt8);
    const auto _X1_X2_D1_zt21 = (gCc12 * _X1_X2_D1_zt22);
    const auto _X1_X2_D1_zt23 = (_X1_X2_D1_zt103 * _X1_X2_D1_zt28);
    const auto _X1_X2_D1_zt24 = (gCc12 * _X1_X2_D1_zt30);
    const auto _X1_X2_D1_zt27 = (_X1_X2_D1_zt23 - _X1_X2_D1_zt24);
    const auto _X1_X2_D1_zt39 = (gCc12 * _X1_X2_D1_zt70);
    const auto _X1_X2_D1_zt43 = (_X1_X2_D1_zt86 * _X1_X2_D1_zt45);
    const auto _X1_X2_D1_zt65 = (_X1_X2_D1_zt103 * _X1_X2_D1_zt66);
    const auto _X1_X2_D1_zt87 = (_X1_X2_D1_zt195 - _X1_X2_D1_zt1);
    const auto _X1_X2_D1_zt97 = (_X1_X2_D1_zt103 * _X1_X2_D1_zt100);
    const auto _X1_X2_D1_zt98 = (_X1_X2_D1_zt99 - _X1_X2_D1_zt97);
    const auto _X1_X2_D1_zt104 = (_X1_X2_D1_zt21 - _X1_X2_D1_zt20);
    const auto _X1_X2_D1_zt19 = (_X1_X2_D1_zt20 - _X1_X2_D1_zt21);
    const auto _X1_X2_D1_zt40 = (gRfb * _X1_X2_D1_zt87);
    const auto _X1_X2_D1_zt41 = (_X1_X2_D1_zt103 * _X1_X2_D1_zt43);
    const auto _X1_X2_D1_zt62 = (gRatt * _X1_X2_D1_zt87);
    const auto _X1_X2_D1_zt72 = (_X1_X2_D1_zt40 * 1.0);
    const auto _X1_X2_D1_zt76 = (_X1_X2_D1_zt63 * _X1_X2_D1_zt87);
    const auto _X1_X2_D1_zt7 = (gRatt * _X1_X2_D1_zt19);
    const auto _X1_X2_D1_zt12 = (gRfb * _X1_X2_D1_zt40);
    const auto _X1_X2_D1_zt18 = (gRfb * _X1_X2_D1_zt7);
    const auto _X1_X2_D1_zt37 = (_X1_X2_D1_zt39 - _X1_X2_D1_zt41);
    const auto _X1_X2_D1_zt68 = (_X1_X2_D1_zt63 * _X1_X2_D1_zt37);
    const auto _X1_X2_D1_zt69 = (gRfb * _X1_X2_D1_zt37);
    const auto _X1_X2_D1_zt75 = (_X1_X2_D1_zt62 - _X1_X2_D1_zt76);
    const auto _X1_X2_D1_zt15 = (_X1_X2_D1_zt75 * 1.0);
    const auto _X1_X2_D1_zt35 = (gRatt * _X1_X2_D1_zt37);
    const auto _X1_X2_D1_zt60 = (gRfb * _X1_X2_D1_zt69);
    const auto _X1_X2_D1_zt61 = (_X1_X2_D1_zt34 * _X1_X2_D1_zt75);
    const auto _X1_X2_D1_zt67 = (_X1_X2_D1_zt35 - _X1_X2_D1_zt68);
    const auto _X1_X2_D1_zt74 = (_X1_X2_D1_zt61 + _X1_X2_D1_zt12);
    const auto _X1_X2_D1_zt77 = (_X1_X2_D1_zt72 / _X1_X2_D1_zt74);
    const auto _X1_X2_D1_zt11 = (_X1_X2_D1_zt15 / _X1_X2_D1_zt74);
    const auto _X1_X2_D1_zt17 = (_X1_X2_D1_zt18 / _X1_X2_D1_zt74);
    const auto _X1_X2_D1_zt58 = (_X1_X2_D1_zt34 * _X1_X2_D1_zt67);
    const auto _X1_X2_D1_zt10 = (-_X1_X2_D1_zt11);
    const auto _X1_X2_D1_zt16 = (-_X1_X2_D1_zt17);
    const auto _X1_X2_D1_zt33 = (_X1_X2_D1_zt58 + _X1_X2_D1_zt60);
    const auto _X1_X2_D1_zt32 = (_X1_X2_D1_zt33 / _X1_X2_D1_zt74);
    const auto _X1_X2_D1_zt57 = (-_X1_X2_D1_zt32);
    const auto _X1_X2_D1_Z0_0 = ((((_X1_X2_D1_zt34 * ((gRatt * _X1_X2_D1_zt0) - (_X1_X2_D1_zt63 * _X1_X2_D1_zt0))) + (gRfb * (gRfb * _X1_X2_D1_zt0))) / _X1_X2_D1_zt74) - _X1_X2_D1_zt11);
    const auto _X1_X2_D1_Z0_1 = _X1_X2_D1_zt10;
    const auto _X1_X2_D1_Z0_3 = (((_X1_X2_D1_zt34 * ((gRatt * _X1_X2_D1_zt5) - (_X1_X2_D1_zt63 * _X1_X2_D1_zt5))) + (gRfb * (gRfb * _X1_X2_D1_zt5))) / _X1_X2_D1_zt74);
    const auto _X1_X2_D1_Z0_4 = _X1_X2_D1_zt16;
    const auto _X1_X2_D1_Z1_0 = _X1_X2_D1_zt10;
    const auto _X1_X2_D1_Z1_1 = (-((1.0 / _X1_X2_D1_zt14) + _X1_X2_D1_zt11));
    const auto _X1_X2_D1_Z1_4 = _X1_X2_D1_zt16;
    const auto _X1_X2_D1_Z2_2 = ((((_X1_X2_D1_zt34 * ((gRatt * _X1_X2_D1_zt27) - (_X1_X2_D1_zt63 * _X1_X2_D1_zt27))) + (gRfb * (gRfb * _X1_X2_D1_zt27))) / _X1_X2_D1_zt74) - _X1_X2_D1_zt32);
    const auto _X1_X2_D1_Z2_3 = _X1_X2_D1_zt57;
    const auto _X1_X2_D1_Z2_4 = (((_X1_X2_D1_zt34 * ((gRatt * _X1_X2_D1_zt46) - (_X1_X2_D1_zt63 * _X1_X2_D1_zt46))) + (gRfb * (gRfb * _X1_X2_D1_zt46))) / _X1_X2_D1_zt74);
    const auto _X1_X2_D1_Z3_0 = (((_X1_X2_D1_zt34 * ((gRatt * _X1_X2_D1_zt56) - (_X1_X2_D1_zt63 * _X1_X2_D1_zt56))) + (gRfb * (gRfb * _X1_X2_D1_zt56))) / _X1_X2_D1_zt74);
    const auto _X1_X2_D1_Z3_2 = _X1_X2_D1_zt57;
    const auto _X1_X2_D1_Z3_3 = ((((_X1_X2_D1_zt34 * ((gRatt * _X1_X2_D1_zt65) - (_X1_X2_D1_zt63 * _X1_X2_D1_zt65))) + (gRfb * (gRfb * _X1_X2_D1_zt65))) / _X1_X2_D1_zt74) - _X1_X2_D1_zt32);
    const auto _X1_X2_D1_Z4_0 = _X1_X2_D1_zt77;
    const auto _X1_X2_D1_Z4_1 = _X1_X2_D1_zt77;
    const auto _X1_X2_D1_Z4_2 = (((_X1_X2_D1_zt34 * ((gRatt * _X1_X2_D1_zt83) - (_X1_X2_D1_zt63 * _X1_X2_D1_zt83))) + (gRfb * (gRfb * _X1_X2_D1_zt83))) / _X1_X2_D1_zt74);
    const auto _X1_X2_D1_Z4_4 = ((((_X1_X2_D1_zt34 * ((gRatt * _X1_X2_D1_zt98) - (_X1_X2_D1_zt63 * _X1_X2_D1_zt98))) + (gRfb * (gRfb * _X1_X2_D1_zt98))) / _X1_X2_D1_zt74) + (((_X1_X2_D1_zt34 * (_X1_X2_D1_zt63 * _X1_X2_D1_zt104)) - (gRfb * (gRfb * _X1_X2_D1_zt104))) / _X1_X2_D1_zt74));
    double c0__X1_X2_D1_voc2;
    double c__X1_X2_D1_voc2[8];
    double c0__X1_X2_D1_voc4;
    double c__X1_X2_D1_voc4[8];
    double c0__X1_X2_D1_voc3;
    double c__X1_X2_D1_voc3[8];
    double c0__X1_X2_D1_zt183;
    double c__X1_X2_D1_zt183[8];
    double c0__X1_X2_D1_voc0;
    double c__X1_X2_D1_voc0[8];
    double c0__X1_X2_D1_zt125;
    double c__X1_X2_D1_zt125[8];
    double c0__X1_X2_D1_zt141;
    double c__X1_X2_D1_zt141[8];
    double c0__X1_X2_D1_zt184;
    double c__X1_X2_D1_zt184[8];
    
    for (int _k = 0; _k <= 8; ++_k)
    {
        const auto vi = (_k == 1) ? 1.0 : 0.0;
        const auto zCin = (_k == 2) ? 1.0 : 0.0;
        const auto zRk1Ck1 = (_k == 3) ? 1.0 : 0.0;
        const auto zCc12 = (_k == 4) ? 1.0 : 0.0;
        const auto zRrelCenv = (_k == 5) ? 1.0 : 0.0;
        const auto zCout = (_k == 6) ? 1.0 : 0.0;
        const auto zCsc = (_k == 7) ? 1.0 : 0.0;
        const auto zRk2Ck2 = (_k == 8) ? 1.0 : 0.0;
        const auto _X1_X2_D1_zt128 = (gCsc * zCsc);
        const auto _X1_X2_D1_zt131 = (zCout - zCsc);
        const auto _X1_X2_D1_zt136 = (-zCout);
        const auto _X1_X2_D1_zt144 = (_X1_X2_D1_zt59 * zRk2Ck2);
        const auto _X1_X2_D1_zt146 = (gCout * _X1_X2_D1_zt144);
        const auto _X1_X2_D1_zt158 = (-zCc12);
        const auto _X1_X2_D1_zt178 = (_X1_X2_D1_zt131 * gCsc);
        const auto _X1_X2_D1_zt180 = (gCout * zCsc);
        const auto _X1_X2_D1_zt187 = (gRk2Ck2 * _X1_X2_D1_zt180);
        const auto _X1_X2_D1_zt135 = (_X1_X2_D1_zt178 + _X1_X2_D1_zt128);
        const auto _X1_X2_D1_zt171 = (gCout * _X1_X2_D1_zt187);
        const auto _X1_X2_D1_zt177 = (_X1_X2_D1_zt26 * zCsc);
        const auto _X1_X2_D1_zt181 = (gRp2 * _X1_X2_D1_zt31);
        const auto _X1_X2_D1_zt189 = (_X1_X2_D1_zt31 * _X1_X2_D1_zt136);
        const auto _X1_X2_D1_zt130 = (gRk2Ck2 * _X1_X2_D1_zt135);
        const auto _X1_X2_D1_zt172 = (_X1_X2_D1_zt177 + _X1_X2_D1_zt178);
        const auto _X1_X2_D1_zt175 = (gRatt * _X1_X2_D1_zt181);
        const auto _X1_X2_D1_zt182 = (gCc12 * _X1_X2_D1_zt175);
        const auto _X1_X2_D1_zt193 = (_X1_X2_D1_zt86 * _X1_X2_D1_zt175);
        const auto _X1_X2_D1_zt194 = (gCc12 * _X1_X2_D1_zt182);
        const auto _X1_X2_D1_zt120 = (gCout * _X1_X2_D1_zt130);
        const auto _X1_X2_D1_zt151 = (_X1_X2_D1_zt2 * zRk2Ck2);
        const auto _X1_X2_D1_zt165 = (_X1_X2_D1_zt6 * _X1_X2_D1_zt151);
        const auto _X1_X2_D1_zt169 = (gRk2Ck2 * _X1_X2_D1_zt172);
        const auto _X1_X2_D1_zt174 = (_X1_X2_D1_zt103 * _X1_X2_D1_zt193);
        const auto _X1_X2_D1_zt188 = (_X1_X2_D1_zt6 * _X1_X2_D1_zt169);
        const auto _X1_X2_D1_zt191 = (_X1_X2_D1_zt171 - _X1_X2_D1_zt188);
        const auto _X1_X2_D1_zt119 = (_X1_X2_D1_zt196 * _X1_X2_D1_zt136);
        const auto _X1_X2_D1_zt129 = (_X1_X2_D1_zt119 + _X1_X2_D1_zt120);
        const auto _X1_X2_D1_zt133 = (gRp2 * _X1_X2_D1_zt196);
        const auto _X1_X2_D1_zt134 = (gRatt * _X1_X2_D1_zt129);
        const auto _X1_X2_D1_zt145 = (_X1_X2_D1_zt165 - _X1_X2_D1_zt146);
        const auto _X1_X2_D1_zt150 = (gRatt * _X1_X2_D1_zt145);
        const auto _X1_X2_D1_zt152 = (_X1_X2_D1_zt88 * zCc12);
        const auto _X1_X2_D1_zt160 = (_X1_X2_D1_zt103 * _X1_X2_D1_zt152);
        const auto _X1_X2_D1_zt167 = (gCc12 * _X1_X2_D1_zt150);
        const auto _X1_X2_D1_zt173 = (_X1_X2_D1_zt174 - _X1_X2_D1_zt194);
        const auto _X1_X2_D1_zt190 = (_X1_X2_D1_zt191 - _X1_X2_D1_zt189);
        const auto _X1_X2_D1_zt192 = (VCC * _X1_X2_D1_zt173);
        const auto _X1_X2_D1_zt106 = (gRp1 * _X1_X2_D1_zt110);
        const auto _X1_X2_D1_zt108 = (_X1_X2_D1_zt110 * _X1_X2_D1_zt158);
        const auto _X1_X2_D1_zt109 = (VCC * _X1_X2_D1_zt106);
        const auto _X1_X2_D1_zt111 = (gCc12 * _X1_X2_D1_zt152);
        const auto _X1_X2_D1_zt122 = (gCc12 * _X1_X2_D1_zt134);
        const auto _X1_X2_D1_zt127 = (_X1_X2_D1_zt86 * _X1_X2_D1_zt134);
        const auto _X1_X2_D1_zt132 = (gRatt * _X1_X2_D1_zt133);
        const auto _X1_X2_D1_zt139 = (gCc12 * _X1_X2_D1_zt132);
        const auto _X1_X2_D1_zt147 = (_X1_X2_D1_zt86 * _X1_X2_D1_zt150);
        const auto _X1_X2_D1_zt153 = (_X1_X2_D1_zt13 * _X1_X2_D1_zt158);
        const auto _X1_X2_D1_zt157 = (_X1_X2_D1_zt160 + _X1_X2_D1_zt153);
        const auto _X1_X2_D1_zt159 = (gRp1 * _X1_X2_D1_zt13);
        const auto _X1_X2_D1_zt163 = (gCc12 * _X1_X2_D1_zt167);
        const auto _X1_X2_D1_zt168 = (_X1_X2_D1_zt103 * _X1_X2_D1_zt147);
        const auto _X1_X2_D1_zt186 = (gRatt * _X1_X2_D1_zt190);
        const auto _X1_X2_D1_zt105 = (_X1_X2_D1_zt108 + _X1_X2_D1_zt111);
        const auto _X1_X2_D1_zt107 = (_X1_X2_D1_zt105 - _X1_X2_D1_zt109);
        const auto _X1_X2_D1_zt117 = (zRrelCenv * _X1_X2_D1_zt87);
        const auto _X1_X2_D1_zt121 = (gCc12 * _X1_X2_D1_zt122);
        const auto _X1_X2_D1_zt123 = (gCc12 * _X1_X2_D1_zt139);
        const auto _X1_X2_D1_zt124 = (_X1_X2_D1_zt86 * _X1_X2_D1_zt132);
        const auto _X1_X2_D1_zt126 = (_X1_X2_D1_zt103 * _X1_X2_D1_zt127);
        const auto _X1_X2_D1_zt140 = (_X1_X2_D1_zt103 * _X1_X2_D1_zt124);
        const auto _X1_X2_D1_zt149 = (_X1_X2_D1_zt163 - _X1_X2_D1_zt168);
        const auto _X1_X2_D1_zt154 = (VCC * _X1_X2_D1_zt159);
        const auto _X1_X2_D1_zt156 = (_X1_X2_D1_zt157 - _X1_X2_D1_zt154);
        const auto _X1_X2_D1_zt162 = (gRatt * _X1_X2_D1_zt149);
        const auto _X1_X2_D1_zt164 = (_X1_X2_D1_zt63 * _X1_X2_D1_zt149);
        const auto _X1_X2_D1_zt170 = (_X1_X2_D1_zt86 * _X1_X2_D1_zt186);
        const auto _X1_X2_D1_zt179 = (_X1_X2_D1_zt103 * _X1_X2_D1_zt170);
        const auto _X1_X2_D1_zt185 = (gCc12 * _X1_X2_D1_zt186);
        const auto _X1_X2_D1_zt112 = (-_X1_X2_D1_zt117);
        const auto _X1_X2_D1_zt116 = (gRfb * _X1_X2_D1_zt112);
        const auto _X1_X2_D1_zt118 = (_X1_X2_D1_zt126 - _X1_X2_D1_zt121);
        const auto _X1_X2_D1_zt138 = (_X1_X2_D1_zt123 - _X1_X2_D1_zt140);
        const auto _X1_X2_D1_zt148 = (gRfb * _X1_X2_D1_zt149);
        const auto _X1_X2_D1_zt161 = (_X1_X2_D1_zt162 - _X1_X2_D1_zt164);
        const auto _X1_X2_D1_zt166 = (gRfb * _X1_X2_D1_zt148);
        const auto _X1_X2_D1_zt176 = (gCc12 * _X1_X2_D1_zt185);
        const auto _X1_X2_D1_zt184 = (_X1_X2_D1_zt176 - _X1_X2_D1_zt179);
        const auto _X1_X2_D1_zt115 = (_X1_X2_D1_zt75 * zRk1Ck1);
        const auto _X1_X2_D1_zt137 = (VCC * _X1_X2_D1_zt138);
        const auto _X1_X2_D1_zt143 = (_X1_X2_D1_zt34 * _X1_X2_D1_zt161);
        const auto _X1_X2_D1_zt183 = (_X1_X2_D1_zt184 - _X1_X2_D1_zt192);
        const auto _X1_X2_D1_zt114 = (_X1_X2_D1_zt115 + _X1_X2_D1_zt116);
        const auto _X1_X2_D1_zt125 = (_X1_X2_D1_zt118 + _X1_X2_D1_zt137);
        const auto _X1_X2_D1_zt142 = (_X1_X2_D1_zt143 + _X1_X2_D1_zt166);
        const auto _X1_X2_D1_zt113 = (_X1_X2_D1_zt114 / _X1_X2_D1_zt74);
        const auto _X1_X2_D1_zt141 = (_X1_X2_D1_zt142 / _X1_X2_D1_zt74);
        const auto _X1_X2_D1_voc0 = ((((_X1_X2_D1_zt34 * ((gRatt * _X1_X2_D1_zt107) - (_X1_X2_D1_zt63 * _X1_X2_D1_zt107))) + (gRfb * (gRfb * _X1_X2_D1_zt107))) / _X1_X2_D1_zt74) - _X1_X2_D1_zt113);
        const auto _X1_X2_D1_voc2 = ((((_X1_X2_D1_zt34 * ((gRatt * _X1_X2_D1_zt125) - (_X1_X2_D1_zt63 * _X1_X2_D1_zt125))) + (gRfb * (gRfb * _X1_X2_D1_zt125))) / _X1_X2_D1_zt74) - _X1_X2_D1_zt141);
        const auto _X1_X2_D1_voc3 = ((((_X1_X2_D1_zt34 * ((gRatt * _X1_X2_D1_zt156) - (_X1_X2_D1_zt63 * _X1_X2_D1_zt156))) + (gRfb * (gRfb * _X1_X2_D1_zt156))) / _X1_X2_D1_zt74) - _X1_X2_D1_zt141);
        const auto _X1_X2_D1_voc4 = ((((_X1_X2_D1_zt34 * ((gRatt * _X1_X2_D1_zt183) - (_X1_X2_D1_zt63 * _X1_X2_D1_zt183))) + (gRfb * (gRfb * _X1_X2_D1_zt183))) / _X1_X2_D1_zt74) + (((_X1_X2_D1_zt34 * _X1_X2_D1_zt117) + (_X1_X2_D1_zt40 * zRk1Ck1)) / _X1_X2_D1_zt74));
        if (_k == 0) {
            c0__X1_X2_D1_voc2 = _X1_X2_D1_voc2;
            c0__X1_X2_D1_voc4 = _X1_X2_D1_voc4;
            c0__X1_X2_D1_voc3 = _X1_X2_D1_voc3;
            c0__X1_X2_D1_zt183 = _X1_X2_D1_zt183;
            c0__X1_X2_D1_voc0 = _X1_X2_D1_voc0;
            c0__X1_X2_D1_zt125 = _X1_X2_D1_zt125;
            c0__X1_X2_D1_zt141 = _X1_X2_D1_zt141;
            c0__X1_X2_D1_zt184 = _X1_X2_D1_zt184;
        } else {
            c__X1_X2_D1_voc2[_k - 1] = _X1_X2_D1_voc2 - c0__X1_X2_D1_voc2;
            c__X1_X2_D1_voc4[_k - 1] = _X1_X2_D1_voc4 - c0__X1_X2_D1_voc4;
            c__X1_X2_D1_voc3[_k - 1] = _X1_X2_D1_voc3 - c0__X1_X2_D1_voc3;
            c__X1_X2_D1_zt183[_k - 1] = _X1_X2_D1_zt183 - c0__X1_X2_D1_zt183;
            c__X1_X2_D1_voc0[_k - 1] = _X1_X2_D1_voc0 - c0__X1_X2_D1_voc0;
            c__X1_X2_D1_zt125[_k - 1] = _X1_X2_D1_zt125 - c0__X1_X2_D1_zt125;
            c__X1_X2_D1_zt141[_k - 1] = _X1_X2_D1_zt141 - c0__X1_X2_D1_zt141;
            c__X1_X2_D1_zt184[_k - 1] = _X1_X2_D1_zt184 - c0__X1_X2_D1_zt184;
        }
    }
    
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zCin = state[ch].zCin;
        auto zRk1Ck1 = state[ch].zRk1Ck1;
        auto zCc12 = state[ch].zCc12;
        auto zRrelCenv = state[ch].zRrelCenv;
        auto zCout = state[ch].zCout;
        auto zCsc = state[ch].zCsc;
        auto zRk2Ck2 = state[ch].zRk2Ck2;
        auto vPKX1 = state[ch].vPKX1;
        auto vGKX1 = state[ch].vGKX1;
        auto vPKX2 = state[ch].vPKX2;
        auto vGKX2 = state[ch].vGKX2;
        auto vD1 = state[ch].vD1;
        long long nr_solves_X1_X2_D1 = state[ch].nr_solves_X1_X2_D1;
        long long nr_iters_X1_X2_D1 = state[ch].nr_iters_X1_X2_D1;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            // --- Newton-Raphson solve (N-port): X1_X2_D1
            const auto _X1_X2_D1_voc2 = c0__X1_X2_D1_voc2 + c__X1_X2_D1_voc2[0] * vi + c__X1_X2_D1_voc2[1] * zCin + c__X1_X2_D1_voc2[2] * zRk1Ck1 + c__X1_X2_D1_voc2[3] * zCc12 + c__X1_X2_D1_voc2[4] * zRrelCenv + c__X1_X2_D1_voc2[5] * zCout + c__X1_X2_D1_voc2[6] * zCsc + c__X1_X2_D1_voc2[7] * zRk2Ck2;
            const auto _X1_X2_D1_voc4 = c0__X1_X2_D1_voc4 + c__X1_X2_D1_voc4[0] * vi + c__X1_X2_D1_voc4[1] * zCin + c__X1_X2_D1_voc4[2] * zRk1Ck1 + c__X1_X2_D1_voc4[3] * zCc12 + c__X1_X2_D1_voc4[4] * zRrelCenv + c__X1_X2_D1_voc4[5] * zCout + c__X1_X2_D1_voc4[6] * zCsc + c__X1_X2_D1_voc4[7] * zRk2Ck2;
            const auto _X1_X2_D1_voc3 = c0__X1_X2_D1_voc3 + c__X1_X2_D1_voc3[0] * vi + c__X1_X2_D1_voc3[1] * zCin + c__X1_X2_D1_voc3[2] * zRk1Ck1 + c__X1_X2_D1_voc3[3] * zCc12 + c__X1_X2_D1_voc3[4] * zRrelCenv + c__X1_X2_D1_voc3[5] * zCout + c__X1_X2_D1_voc3[6] * zCsc + c__X1_X2_D1_voc3[7] * zRk2Ck2;
            const auto _X1_X2_D1_zt183 = c0__X1_X2_D1_zt183 + c__X1_X2_D1_zt183[0] * vi + c__X1_X2_D1_zt183[1] * zCin + c__X1_X2_D1_zt183[2] * zRk1Ck1 + c__X1_X2_D1_zt183[3] * zCc12 + c__X1_X2_D1_zt183[4] * zRrelCenv + c__X1_X2_D1_zt183[5] * zCout + c__X1_X2_D1_zt183[6] * zCsc + c__X1_X2_D1_zt183[7] * zRk2Ck2;
            const auto _X1_X2_D1_voc0 = c0__X1_X2_D1_voc0 + c__X1_X2_D1_voc0[0] * vi + c__X1_X2_D1_voc0[1] * zCin + c__X1_X2_D1_voc0[2] * zRk1Ck1 + c__X1_X2_D1_voc0[3] * zCc12 + c__X1_X2_D1_voc0[4] * zRrelCenv + c__X1_X2_D1_voc0[5] * zCout + c__X1_X2_D1_voc0[6] * zCsc + c__X1_X2_D1_voc0[7] * zRk2Ck2;
            const auto _X1_X2_D1_zt125 = c0__X1_X2_D1_zt125 + c__X1_X2_D1_zt125[0] * vi + c__X1_X2_D1_zt125[1] * zCin + c__X1_X2_D1_zt125[2] * zRk1Ck1 + c__X1_X2_D1_zt125[3] * zCc12 + c__X1_X2_D1_zt125[4] * zRrelCenv + c__X1_X2_D1_zt125[5] * zCout + c__X1_X2_D1_zt125[6] * zCsc + c__X1_X2_D1_zt125[7] * zRk2Ck2;
            const auto _X1_X2_D1_zt141 = c0__X1_X2_D1_zt141 + c__X1_X2_D1_zt141[0] * vi + c__X1_X2_D1_zt141[1] * zCin + c__X1_X2_D1_zt141[2] * zRk1Ck1 + c__X1_X2_D1_zt141[3] * zCc12 + c__X1_X2_D1_zt141[4] * zRrelCenv + c__X1_X2_D1_zt141[5] * zCout + c__X1_X2_D1_zt141[6] * zCsc + c__X1_X2_D1_zt141[7] * zRk2Ck2;
            const auto _X1_X2_D1_zt184 = c0__X1_X2_D1_zt184 + c__X1_X2_D1_zt184[0] * vi + c__X1_X2_D1_zt184[1] * zCin + c__X1_X2_D1_zt184[2] * zRk1Ck1 + c__X1_X2_D1_zt184[3] * zCc12 + c__X1_X2_D1_zt184[4] * zRrelCenv + c__X1_X2_D1_zt184[5] * zCout + c__X1_X2_D1_zt184[6] * zCsc + c__X1_X2_D1_zt184[7] * zRk2Ck2;
            const auto _X1_X2_D1_zt128 = (gCsc * zCsc);
            const auto _X1_X2_D1_zt131 = (zCout - zCsc);
            const auto _X1_X2_D1_zt136 = (-zCout);
            const auto _X1_X2_D1_zt144 = (_X1_X2_D1_zt59 * zRk2Ck2);
            const auto _X1_X2_D1_zt146 = (gCout * _X1_X2_D1_zt144);
            const auto _X1_X2_D1_zt158 = (-zCc12);
            const auto _X1_X2_D1_zt178 = (_X1_X2_D1_zt131 * gCsc);
            const auto _X1_X2_D1_zt180 = (gCout * zCsc);
            const auto _X1_X2_D1_zt187 = (gRk2Ck2 * _X1_X2_D1_zt180);
            const auto _X1_X2_D1_zt135 = (_X1_X2_D1_zt178 + _X1_X2_D1_zt128);
            const auto _X1_X2_D1_zt171 = (gCout * _X1_X2_D1_zt187);
            const auto _X1_X2_D1_zt177 = (_X1_X2_D1_zt26 * zCsc);
            const auto _X1_X2_D1_zt181 = (gRp2 * _X1_X2_D1_zt31);
            const auto _X1_X2_D1_zt189 = (_X1_X2_D1_zt31 * _X1_X2_D1_zt136);
            const auto _X1_X2_D1_zt130 = (gRk2Ck2 * _X1_X2_D1_zt135);
            const auto _X1_X2_D1_zt172 = (_X1_X2_D1_zt177 + _X1_X2_D1_zt178);
            const auto _X1_X2_D1_zt175 = (gRatt * _X1_X2_D1_zt181);
            const auto _X1_X2_D1_zt182 = (gCc12 * _X1_X2_D1_zt175);
            const auto _X1_X2_D1_zt193 = (_X1_X2_D1_zt86 * _X1_X2_D1_zt175);
            const auto _X1_X2_D1_zt194 = (gCc12 * _X1_X2_D1_zt182);
            const auto _X1_X2_D1_zt120 = (gCout * _X1_X2_D1_zt130);
            const auto _X1_X2_D1_zt151 = (_X1_X2_D1_zt2 * zRk2Ck2);
            const auto _X1_X2_D1_zt165 = (_X1_X2_D1_zt6 * _X1_X2_D1_zt151);
            const auto _X1_X2_D1_zt169 = (gRk2Ck2 * _X1_X2_D1_zt172);
            const auto _X1_X2_D1_zt174 = (_X1_X2_D1_zt103 * _X1_X2_D1_zt193);
            const auto _X1_X2_D1_zt188 = (_X1_X2_D1_zt6 * _X1_X2_D1_zt169);
            const auto _X1_X2_D1_zt191 = (_X1_X2_D1_zt171 - _X1_X2_D1_zt188);
            const auto _X1_X2_D1_zt119 = (_X1_X2_D1_zt196 * _X1_X2_D1_zt136);
            const auto _X1_X2_D1_zt129 = (_X1_X2_D1_zt119 + _X1_X2_D1_zt120);
            const auto _X1_X2_D1_zt133 = (gRp2 * _X1_X2_D1_zt196);
            const auto _X1_X2_D1_zt134 = (gRatt * _X1_X2_D1_zt129);
            const auto _X1_X2_D1_zt145 = (_X1_X2_D1_zt165 - _X1_X2_D1_zt146);
            const auto _X1_X2_D1_zt150 = (gRatt * _X1_X2_D1_zt145);
            const auto _X1_X2_D1_zt152 = (_X1_X2_D1_zt88 * zCc12);
            const auto _X1_X2_D1_zt160 = (_X1_X2_D1_zt103 * _X1_X2_D1_zt152);
            const auto _X1_X2_D1_zt167 = (gCc12 * _X1_X2_D1_zt150);
            const auto _X1_X2_D1_zt173 = (_X1_X2_D1_zt174 - _X1_X2_D1_zt194);
            const auto _X1_X2_D1_zt190 = (_X1_X2_D1_zt191 - _X1_X2_D1_zt189);
            const auto _X1_X2_D1_zt192 = (VCC * _X1_X2_D1_zt173);
            const auto _X1_X2_D1_zt106 = (gRp1 * _X1_X2_D1_zt110);
            const auto _X1_X2_D1_zt108 = (_X1_X2_D1_zt110 * _X1_X2_D1_zt158);
            const auto _X1_X2_D1_zt109 = (VCC * _X1_X2_D1_zt106);
            const auto _X1_X2_D1_zt111 = (gCc12 * _X1_X2_D1_zt152);
            const auto _X1_X2_D1_zt122 = (gCc12 * _X1_X2_D1_zt134);
            const auto _X1_X2_D1_zt127 = (_X1_X2_D1_zt86 * _X1_X2_D1_zt134);
            const auto _X1_X2_D1_zt132 = (gRatt * _X1_X2_D1_zt133);
            const auto _X1_X2_D1_zt139 = (gCc12 * _X1_X2_D1_zt132);
            const auto _X1_X2_D1_zt147 = (_X1_X2_D1_zt86 * _X1_X2_D1_zt150);
            const auto _X1_X2_D1_zt153 = (_X1_X2_D1_zt13 * _X1_X2_D1_zt158);
            const auto _X1_X2_D1_zt157 = (_X1_X2_D1_zt160 + _X1_X2_D1_zt153);
            const auto _X1_X2_D1_zt159 = (gRp1 * _X1_X2_D1_zt13);
            const auto _X1_X2_D1_zt163 = (gCc12 * _X1_X2_D1_zt167);
            const auto _X1_X2_D1_zt168 = (_X1_X2_D1_zt103 * _X1_X2_D1_zt147);
            const auto _X1_X2_D1_zt186 = (gRatt * _X1_X2_D1_zt190);
            const auto _X1_X2_D1_zt105 = (_X1_X2_D1_zt108 + _X1_X2_D1_zt111);
            const auto _X1_X2_D1_zt107 = (_X1_X2_D1_zt105 - _X1_X2_D1_zt109);
            const auto _X1_X2_D1_zt117 = (zRrelCenv * _X1_X2_D1_zt87);
            const auto _X1_X2_D1_zt121 = (gCc12 * _X1_X2_D1_zt122);
            const auto _X1_X2_D1_zt123 = (gCc12 * _X1_X2_D1_zt139);
            const auto _X1_X2_D1_zt124 = (_X1_X2_D1_zt86 * _X1_X2_D1_zt132);
            const auto _X1_X2_D1_zt126 = (_X1_X2_D1_zt103 * _X1_X2_D1_zt127);
            const auto _X1_X2_D1_zt140 = (_X1_X2_D1_zt103 * _X1_X2_D1_zt124);
            const auto _X1_X2_D1_zt149 = (_X1_X2_D1_zt163 - _X1_X2_D1_zt168);
            const auto _X1_X2_D1_zt154 = (VCC * _X1_X2_D1_zt159);
            const auto _X1_X2_D1_zt156 = (_X1_X2_D1_zt157 - _X1_X2_D1_zt154);
            const auto _X1_X2_D1_zt162 = (gRatt * _X1_X2_D1_zt149);
            const auto _X1_X2_D1_zt164 = (_X1_X2_D1_zt63 * _X1_X2_D1_zt149);
            const auto _X1_X2_D1_zt170 = (_X1_X2_D1_zt86 * _X1_X2_D1_zt186);
            const auto _X1_X2_D1_zt179 = (_X1_X2_D1_zt103 * _X1_X2_D1_zt170);
            const auto _X1_X2_D1_zt185 = (gCc12 * _X1_X2_D1_zt186);
            const auto _X1_X2_D1_zt112 = (-_X1_X2_D1_zt117);
            const auto _X1_X2_D1_zt116 = (gRfb * _X1_X2_D1_zt112);
            const auto _X1_X2_D1_zt118 = (_X1_X2_D1_zt126 - _X1_X2_D1_zt121);
            const auto _X1_X2_D1_zt138 = (_X1_X2_D1_zt123 - _X1_X2_D1_zt140);
            const auto _X1_X2_D1_zt148 = (gRfb * _X1_X2_D1_zt149);
            const auto _X1_X2_D1_zt161 = (_X1_X2_D1_zt162 - _X1_X2_D1_zt164);
            const auto _X1_X2_D1_zt166 = (gRfb * _X1_X2_D1_zt148);
            const auto _X1_X2_D1_zt176 = (gCc12 * _X1_X2_D1_zt185);
            const auto _X1_X2_D1_zt115 = (_X1_X2_D1_zt75 * zRk1Ck1);
            const auto _X1_X2_D1_zt137 = (VCC * _X1_X2_D1_zt138);
            const auto _X1_X2_D1_zt143 = (_X1_X2_D1_zt34 * _X1_X2_D1_zt161);
            const auto _X1_X2_D1_zt114 = (_X1_X2_D1_zt115 + _X1_X2_D1_zt116);
            const auto _X1_X2_D1_zt142 = (_X1_X2_D1_zt143 + _X1_X2_D1_zt166);
            const auto _X1_X2_D1_zt113 = (_X1_X2_D1_zt114 / _X1_X2_D1_zt74);
            const auto _X1_X2_D1_voc1 = (-(((-((gCin * vi) - zCin)) / _X1_X2_D1_zt14) + _X1_X2_D1_zt113));
            const auto _X1_X2_D1_pt37 = (_12AX7_DEMPWOLF_Gamma - 1.0);
            const auto _X1_X2_D1_pt40 = (_12AX7_DEMPWOLF_Xi - 1.0);
            ++nr_solves_X1_X2_D1;
            for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
            {
                ++nr_iters_X1_X2_D1;
                const auto _X1_X2_D1_pt2 = math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1)));
                const auto _X1_X2_D1_pt10 = (_12AX7_DEMPWOLF_Cg * vGKX1);
                const auto _X1_X2_D1_pt13 = math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX2 / _12AX7_DEMPWOLF_Mu) + vGKX2)));
                const auto _X1_X2_D1_pt21 = (_12AX7_DEMPWOLF_Cg * vGKX2);
                const auto _X1_X2_D1_pt23 = (vD1 / D1N914_vt);
                const auto _X1_X2_D1_pt1 = (1.0 + _X1_X2_D1_pt2);
                const auto _X1_X2_D1_pt7 = math_exp_approx(_X1_X2_D1_pt10);
                const auto _X1_X2_D1_pt12 = (1.0 + _X1_X2_D1_pt13);
                const auto _X1_X2_D1_pt20 = math_exp_approx(_X1_X2_D1_pt21);
                const auto _X1_X2_D1_pt22 = math_exp_approx(_X1_X2_D1_pt23);
                const auto _X1_X2_D1_pt0 = (math_log_approx(_X1_X2_D1_pt1) / _12AX7_DEMPWOLF_Ck);
                const auto _X1_X2_D1_pt6 = (1.0 + _X1_X2_D1_pt7);
                const auto _X1_X2_D1_pt11 = (math_log_approx(_X1_X2_D1_pt12) / _12AX7_DEMPWOLF_Ck);
                const auto _X1_X2_D1_pt19 = (1.0 + _X1_X2_D1_pt20);
                const auto _X1_X2_D1_pt27 = math_pow_approx(_X1_X2_D1_pt0, _X1_X2_D1_pt37);
                const auto _X1_X2_D1_pt36 = math_pow_approx(_X1_X2_D1_pt11, _X1_X2_D1_pt37);
                const auto _X1_X2_D1_pt5 = math_log_approx(_X1_X2_D1_pt6);
                const auto _X1_X2_D1_pt16 = math_log_approx(_X1_X2_D1_pt19);
                const auto _X1_X2_D1_pt26 = (_X1_X2_D1_pt27 * _X1_X2_D1_pt2);
                const auto _X1_X2_D1_pt35 = (_X1_X2_D1_pt36 * _X1_X2_D1_pt13);
                const auto _X1_X2_D1_pt4 = (_X1_X2_D1_pt5 / _12AX7_DEMPWOLF_Cg);
                const auto _X1_X2_D1_pt15 = (_X1_X2_D1_pt16 / _12AX7_DEMPWOLF_Cg);
                const auto _X1_X2_D1_pt25 = (_12AX7_DEMPWOLF_Gamma * _X1_X2_D1_pt26);
                const auto _X1_X2_D1_pt30 = math_pow_approx(_X1_X2_D1_pt4, _X1_X2_D1_pt40);
                const auto _X1_X2_D1_pt34 = (_12AX7_DEMPWOLF_Gamma * _X1_X2_D1_pt35);
                const auto _X1_X2_D1_pt43 = math_pow_approx(_X1_X2_D1_pt15, _X1_X2_D1_pt40);
                const auto _X1_X2_D1_pt3 = math_pow_approx(_X1_X2_D1_pt4, _12AX7_DEMPWOLF_Xi);
                const auto _X1_X2_D1_pt9 = (_12AX7_DEMPWOLF_Gg * _X1_X2_D1_pt3);
                const auto _X1_X2_D1_pt14 = math_pow_approx(_X1_X2_D1_pt15, _12AX7_DEMPWOLF_Xi);
                const auto _X1_X2_D1_pt18 = (_12AX7_DEMPWOLF_Gg * _X1_X2_D1_pt14);
                const auto _X1_X2_D1_pt24 = (_12AX7_DEMPWOLF_Gk * _X1_X2_D1_pt25);
                const auto _X1_X2_D1_pt29 = (_X1_X2_D1_pt30 * _X1_X2_D1_pt7);
                const auto _X1_X2_D1_pt32 = (_12AX7_DEMPWOLF_Xi * _X1_X2_D1_pt29);
                const auto _X1_X2_D1_pt33 = (_12AX7_DEMPWOLF_Gk * _X1_X2_D1_pt34);
                const auto _X1_X2_D1_pt39 = (_X1_X2_D1_pt43 * _X1_X2_D1_pt20);
                const auto _X1_X2_D1_pt8 = (_X1_X2_D1_pt9 + _12AX7_DEMPWOLF_Ig0);
                const auto _X1_X2_D1_pt17 = (_X1_X2_D1_pt18 + _12AX7_DEMPWOLF_Ig0);
                const auto _X1_X2_D1_pt31 = (_12AX7_DEMPWOLF_Gg * _X1_X2_D1_pt32);
                const auto _X1_X2_D1_pt38 = (_12AX7_DEMPWOLF_Xi * _X1_X2_D1_pt39);
                const auto _X1_X2_D1_pt42 = (_12AX7_DEMPWOLF_Gg * _X1_X2_D1_pt38);
                const auto _X1_X2_D1_pt28 = (_X1_X2_D1_pt31 / _X1_X2_D1_pt6);
                const auto _X1_X2_D1_pt41 = (_X1_X2_D1_pt42 / _X1_X2_D1_pt19);
                const auto _X1_X2_D1_i0 = ((_12AX7_DEMPWOLF_Gk * math_pow_approx(_X1_X2_D1_pt0, _12AX7_DEMPWOLF_Gamma)) - _X1_X2_D1_pt8);
                const auto _X1_X2_D1_i1 = _X1_X2_D1_pt8;
                const auto _X1_X2_D1_i2 = ((_12AX7_DEMPWOLF_Gk * math_pow_approx(_X1_X2_D1_pt11, _12AX7_DEMPWOLF_Gamma)) - _X1_X2_D1_pt17);
                const auto _X1_X2_D1_i3 = _X1_X2_D1_pt17;
                const auto _X1_X2_D1_i4 = (D1N914_Is * (_X1_X2_D1_pt22 - 1.0));
                const auto _X1_X2_D1_g0_0 = (_X1_X2_D1_pt24 / (_12AX7_DEMPWOLF_Mu * _X1_X2_D1_pt1));
                const auto _X1_X2_D1_g0_1 = ((_X1_X2_D1_pt24 / _X1_X2_D1_pt1) - _X1_X2_D1_pt28);
                const auto _X1_X2_D1_g1_0 = 0.0;
                const auto _X1_X2_D1_g1_1 = _X1_X2_D1_pt28;
                const auto _X1_X2_D1_g2_2 = (_X1_X2_D1_pt33 / (_12AX7_DEMPWOLF_Mu * _X1_X2_D1_pt12));
                const auto _X1_X2_D1_g2_3 = ((_X1_X2_D1_pt33 / _X1_X2_D1_pt12) - _X1_X2_D1_pt41);
                const auto _X1_X2_D1_g3_2 = 0.0;
                const auto _X1_X2_D1_g3_3 = _X1_X2_D1_pt41;
                const auto _X1_X2_D1_g4_4 = ((D1N914_Is * _X1_X2_D1_pt22) / D1N914_vt);
                const auto res_vPKX1 = (((((_X1_X2_D1_voc0 + (_X1_X2_D1_Z0_0 * _X1_X2_D1_i0)) + (_X1_X2_D1_Z0_1 * _X1_X2_D1_i1)) + (_X1_X2_D1_Z0_3 * _X1_X2_D1_i3)) + (_X1_X2_D1_Z0_4 * _X1_X2_D1_i4)) - vPKX1);
                const auto res_vGKX1 = ((((_X1_X2_D1_voc1 + (_X1_X2_D1_Z1_0 * _X1_X2_D1_i0)) + (_X1_X2_D1_Z1_1 * _X1_X2_D1_i1)) + (_X1_X2_D1_Z1_4 * _X1_X2_D1_i4)) - vGKX1);
                const auto res_vPKX2 = ((((_X1_X2_D1_voc2 + (_X1_X2_D1_Z2_2 * _X1_X2_D1_i2)) + (_X1_X2_D1_Z2_3 * _X1_X2_D1_i3)) + (_X1_X2_D1_Z2_4 * _X1_X2_D1_i4)) - vPKX2);
                const auto res_vGKX2 = ((((_X1_X2_D1_voc3 + (_X1_X2_D1_Z3_0 * _X1_X2_D1_i0)) + (_X1_X2_D1_Z3_2 * _X1_X2_D1_i2)) + (_X1_X2_D1_Z3_3 * _X1_X2_D1_i3)) - vGKX2);
                const auto res_vD1 = (((((_X1_X2_D1_voc4 + (_X1_X2_D1_Z4_0 * _X1_X2_D1_i0)) + (_X1_X2_D1_Z4_1 * _X1_X2_D1_i1)) + (_X1_X2_D1_Z4_2 * _X1_X2_D1_i2)) + (_X1_X2_D1_Z4_4 * _X1_X2_D1_i4)) - vD1);
                const auto _X1_X2_D1_J0_0 = (((_X1_X2_D1_Z0_0 * _X1_X2_D1_g0_0) + (_X1_X2_D1_Z0_1 * _X1_X2_D1_g1_0)) - 1.0);
                const auto _X1_X2_D1_J0_1 = ((_X1_X2_D1_Z0_0 * _X1_X2_D1_g0_1) + (_X1_X2_D1_Z0_1 * _X1_X2_D1_g1_1));
                const auto _X1_X2_D1_J0_2 = (_X1_X2_D1_Z0_3 * _X1_X2_D1_g3_2);
                const auto _X1_X2_D1_J0_3 = (_X1_X2_D1_Z0_3 * _X1_X2_D1_g3_3);
                const auto _X1_X2_D1_J0_4 = (_X1_X2_D1_Z0_4 * _X1_X2_D1_g4_4);
                const auto _X1_X2_D1_J1_0 = ((_X1_X2_D1_Z1_0 * _X1_X2_D1_g0_0) + (_X1_X2_D1_Z1_1 * _X1_X2_D1_g1_0));
                const auto _X1_X2_D1_J1_1 = (((_X1_X2_D1_Z1_0 * _X1_X2_D1_g0_1) + (_X1_X2_D1_Z1_1 * _X1_X2_D1_g1_1)) - 1.0);
                const auto _X1_X2_D1_J1_2 = 0.0;
                const auto _X1_X2_D1_J1_3 = 0.0;
                const auto _X1_X2_D1_J1_4 = (_X1_X2_D1_Z1_4 * _X1_X2_D1_g4_4);
                const auto _X1_X2_D1_J2_0 = 0.0;
                const auto _X1_X2_D1_J2_1 = 0.0;
                const auto _X1_X2_D1_J2_2 = (((_X1_X2_D1_Z2_2 * _X1_X2_D1_g2_2) + (_X1_X2_D1_Z2_3 * _X1_X2_D1_g3_2)) - 1.0);
                const auto _X1_X2_D1_J2_3 = ((_X1_X2_D1_Z2_2 * _X1_X2_D1_g2_3) + (_X1_X2_D1_Z2_3 * _X1_X2_D1_g3_3));
                const auto _X1_X2_D1_J2_4 = (_X1_X2_D1_Z2_4 * _X1_X2_D1_g4_4);
                const auto _X1_X2_D1_J3_0 = (_X1_X2_D1_Z3_0 * _X1_X2_D1_g0_0);
                const auto _X1_X2_D1_J3_1 = (_X1_X2_D1_Z3_0 * _X1_X2_D1_g0_1);
                const auto _X1_X2_D1_J3_2 = ((_X1_X2_D1_Z3_2 * _X1_X2_D1_g2_2) + (_X1_X2_D1_Z3_3 * _X1_X2_D1_g3_2));
                const auto _X1_X2_D1_J3_3 = (((_X1_X2_D1_Z3_2 * _X1_X2_D1_g2_3) + (_X1_X2_D1_Z3_3 * _X1_X2_D1_g3_3)) - 1.0);
                const auto _X1_X2_D1_J3_4 = 0.0;
                const auto _X1_X2_D1_J4_0 = ((_X1_X2_D1_Z4_0 * _X1_X2_D1_g0_0) + (_X1_X2_D1_Z4_1 * _X1_X2_D1_g1_0));
                const auto _X1_X2_D1_J4_1 = ((_X1_X2_D1_Z4_0 * _X1_X2_D1_g0_1) + (_X1_X2_D1_Z4_1 * _X1_X2_D1_g1_1));
                const auto _X1_X2_D1_J4_2 = (_X1_X2_D1_Z4_2 * _X1_X2_D1_g2_2);
                const auto _X1_X2_D1_J4_3 = (_X1_X2_D1_Z4_2 * _X1_X2_D1_g2_3);
                const auto _X1_X2_D1_J4_4 = ((_X1_X2_D1_Z4_4 * _X1_X2_D1_g4_4) - 1.0);
            
                double _J[5][5];
                _J[0][0] = _X1_X2_D1_J0_0;
                _J[0][1] = _X1_X2_D1_J0_1;
                _J[0][2] = _X1_X2_D1_J0_2;
                _J[0][3] = _X1_X2_D1_J0_3;
                _J[0][4] = _X1_X2_D1_J0_4;
                _J[1][0] = _X1_X2_D1_J1_0;
                _J[1][1] = _X1_X2_D1_J1_1;
                _J[1][2] = _X1_X2_D1_J1_2;
                _J[1][3] = _X1_X2_D1_J1_3;
                _J[1][4] = _X1_X2_D1_J1_4;
                _J[2][0] = _X1_X2_D1_J2_0;
                _J[2][1] = _X1_X2_D1_J2_1;
                _J[2][2] = _X1_X2_D1_J2_2;
                _J[2][3] = _X1_X2_D1_J2_3;
                _J[2][4] = _X1_X2_D1_J2_4;
                _J[3][0] = _X1_X2_D1_J3_0;
                _J[3][1] = _X1_X2_D1_J3_1;
                _J[3][2] = _X1_X2_D1_J3_2;
                _J[3][3] = _X1_X2_D1_J3_3;
                _J[3][4] = _X1_X2_D1_J3_4;
                _J[4][0] = _X1_X2_D1_J4_0;
                _J[4][1] = _X1_X2_D1_J4_1;
                _J[4][2] = _X1_X2_D1_J4_2;
                _J[4][3] = _X1_X2_D1_J4_3;
                _J[4][4] = _X1_X2_D1_J4_4;
                double _b[5] = { -res_vPKX1, -res_vGKX1, -res_vPKX2, -res_vGKX2, -res_vD1 };
                {
                    int _p = 0;
                    double _bv = std::abs(_J[0][0]);
                    { const double _m = std::abs(_J[1][0]); if (_m > _bv) { _bv = _m; _p = 1; } }
                    { const double _m = std::abs(_J[2][0]); if (_m > _bv) { _bv = _m; _p = 2; } }
                    { const double _m = std::abs(_J[3][0]); if (_m > _bv) { _bv = _m; _p = 3; } }
                    { const double _m = std::abs(_J[4][0]); if (_m > _bv) { _bv = _m; _p = 4; } }
                    if (_p == 1) {
                        { double _t = _J[0][0]; _J[0][0] = _J[1][0]; _J[1][0] = _t; }
                        { double _t = _J[0][1]; _J[0][1] = _J[1][1]; _J[1][1] = _t; }
                        { double _t = _J[0][2]; _J[0][2] = _J[1][2]; _J[1][2] = _t; }
                        { double _t = _J[0][3]; _J[0][3] = _J[1][3]; _J[1][3] = _t; }
                        { double _t = _J[0][4]; _J[0][4] = _J[1][4]; _J[1][4] = _t; }
                        { double _t = _b[0]; _b[0] = _b[1]; _b[1] = _t; }
                    }
                    else if (_p == 2) {
                        { double _t = _J[0][0]; _J[0][0] = _J[2][0]; _J[2][0] = _t; }
                        { double _t = _J[0][1]; _J[0][1] = _J[2][1]; _J[2][1] = _t; }
                        { double _t = _J[0][2]; _J[0][2] = _J[2][2]; _J[2][2] = _t; }
                        { double _t = _J[0][3]; _J[0][3] = _J[2][3]; _J[2][3] = _t; }
                        { double _t = _J[0][4]; _J[0][4] = _J[2][4]; _J[2][4] = _t; }
                        { double _t = _b[0]; _b[0] = _b[2]; _b[2] = _t; }
                    }
                    else if (_p == 3) {
                        { double _t = _J[0][0]; _J[0][0] = _J[3][0]; _J[3][0] = _t; }
                        { double _t = _J[0][1]; _J[0][1] = _J[3][1]; _J[3][1] = _t; }
                        { double _t = _J[0][2]; _J[0][2] = _J[3][2]; _J[3][2] = _t; }
                        { double _t = _J[0][3]; _J[0][3] = _J[3][3]; _J[3][3] = _t; }
                        { double _t = _J[0][4]; _J[0][4] = _J[3][4]; _J[3][4] = _t; }
                        { double _t = _b[0]; _b[0] = _b[3]; _b[3] = _t; }
                    }
                    else if (_p == 4) {
                        { double _t = _J[0][0]; _J[0][0] = _J[4][0]; _J[4][0] = _t; }
                        { double _t = _J[0][1]; _J[0][1] = _J[4][1]; _J[4][1] = _t; }
                        { double _t = _J[0][2]; _J[0][2] = _J[4][2]; _J[4][2] = _t; }
                        { double _t = _J[0][3]; _J[0][3] = _J[4][3]; _J[4][3] = _t; }
                        { double _t = _J[0][4]; _J[0][4] = _J[4][4]; _J[4][4] = _t; }
                        { double _t = _b[0]; _b[0] = _b[4]; _b[4] = _t; }
                    }
                }
                const double _r0 = (double)1 / _J[0][0];
                {
                    const double _f = _J[1][0] * _r0;
                    _J[1][1] -= _f * _J[0][1];
                    _J[1][2] -= _f * _J[0][2];
                    _J[1][3] -= _f * _J[0][3];
                    _J[1][4] -= _f * _J[0][4];
                    _b[1] -= _f * _b[0];
                }
                {
                    const double _f = _J[2][0] * _r0;
                    _J[2][1] -= _f * _J[0][1];
                    _J[2][2] -= _f * _J[0][2];
                    _J[2][3] -= _f * _J[0][3];
                    _J[2][4] -= _f * _J[0][4];
                    _b[2] -= _f * _b[0];
                }
                {
                    const double _f = _J[3][0] * _r0;
                    _J[3][1] -= _f * _J[0][1];
                    _J[3][2] -= _f * _J[0][2];
                    _J[3][3] -= _f * _J[0][3];
                    _J[3][4] -= _f * _J[0][4];
                    _b[3] -= _f * _b[0];
                }
                {
                    const double _f = _J[4][0] * _r0;
                    _J[4][1] -= _f * _J[0][1];
                    _J[4][2] -= _f * _J[0][2];
                    _J[4][3] -= _f * _J[0][3];
                    _J[4][4] -= _f * _J[0][4];
                    _b[4] -= _f * _b[0];
                }
                {
                    int _p = 1;
                    double _bv = std::abs(_J[1][1]);
                    { const double _m = std::abs(_J[2][1]); if (_m > _bv) { _bv = _m; _p = 2; } }
                    { const double _m = std::abs(_J[3][1]); if (_m > _bv) { _bv = _m; _p = 3; } }
                    { const double _m = std::abs(_J[4][1]); if (_m > _bv) { _bv = _m; _p = 4; } }
                    if (_p == 2) {
                        { double _t = _J[1][1]; _J[1][1] = _J[2][1]; _J[2][1] = _t; }
                        { double _t = _J[1][2]; _J[1][2] = _J[2][2]; _J[2][2] = _t; }
                        { double _t = _J[1][3]; _J[1][3] = _J[2][3]; _J[2][3] = _t; }
                        { double _t = _J[1][4]; _J[1][4] = _J[2][4]; _J[2][4] = _t; }
                        { double _t = _b[1]; _b[1] = _b[2]; _b[2] = _t; }
                    }
                    else if (_p == 3) {
                        { double _t = _J[1][1]; _J[1][1] = _J[3][1]; _J[3][1] = _t; }
                        { double _t = _J[1][2]; _J[1][2] = _J[3][2]; _J[3][2] = _t; }
                        { double _t = _J[1][3]; _J[1][3] = _J[3][3]; _J[3][3] = _t; }
                        { double _t = _J[1][4]; _J[1][4] = _J[3][4]; _J[3][4] = _t; }
                        { double _t = _b[1]; _b[1] = _b[3]; _b[3] = _t; }
                    }
                    else if (_p == 4) {
                        { double _t = _J[1][1]; _J[1][1] = _J[4][1]; _J[4][1] = _t; }
                        { double _t = _J[1][2]; _J[1][2] = _J[4][2]; _J[4][2] = _t; }
                        { double _t = _J[1][3]; _J[1][3] = _J[4][3]; _J[4][3] = _t; }
                        { double _t = _J[1][4]; _J[1][4] = _J[4][4]; _J[4][4] = _t; }
                        { double _t = _b[1]; _b[1] = _b[4]; _b[4] = _t; }
                    }
                }
                const double _r1 = (double)1 / _J[1][1];
                {
                    const double _f = _J[2][1] * _r1;
                    _J[2][2] -= _f * _J[1][2];
                    _J[2][3] -= _f * _J[1][3];
                    _J[2][4] -= _f * _J[1][4];
                    _b[2] -= _f * _b[1];
                }
                {
                    const double _f = _J[3][1] * _r1;
                    _J[3][2] -= _f * _J[1][2];
                    _J[3][3] -= _f * _J[1][3];
                    _J[3][4] -= _f * _J[1][4];
                    _b[3] -= _f * _b[1];
                }
                {
                    const double _f = _J[4][1] * _r1;
                    _J[4][2] -= _f * _J[1][2];
                    _J[4][3] -= _f * _J[1][3];
                    _J[4][4] -= _f * _J[1][4];
                    _b[4] -= _f * _b[1];
                }
                {
                    int _p = 2;
                    double _bv = std::abs(_J[2][2]);
                    { const double _m = std::abs(_J[3][2]); if (_m > _bv) { _bv = _m; _p = 3; } }
                    { const double _m = std::abs(_J[4][2]); if (_m > _bv) { _bv = _m; _p = 4; } }
                    if (_p == 3) {
                        { double _t = _J[2][2]; _J[2][2] = _J[3][2]; _J[3][2] = _t; }
                        { double _t = _J[2][3]; _J[2][3] = _J[3][3]; _J[3][3] = _t; }
                        { double _t = _J[2][4]; _J[2][4] = _J[3][4]; _J[3][4] = _t; }
                        { double _t = _b[2]; _b[2] = _b[3]; _b[3] = _t; }
                    }
                    else if (_p == 4) {
                        { double _t = _J[2][2]; _J[2][2] = _J[4][2]; _J[4][2] = _t; }
                        { double _t = _J[2][3]; _J[2][3] = _J[4][3]; _J[4][3] = _t; }
                        { double _t = _J[2][4]; _J[2][4] = _J[4][4]; _J[4][4] = _t; }
                        { double _t = _b[2]; _b[2] = _b[4]; _b[4] = _t; }
                    }
                }
                const double _r2 = (double)1 / _J[2][2];
                {
                    const double _f = _J[3][2] * _r2;
                    _J[3][3] -= _f * _J[2][3];
                    _J[3][4] -= _f * _J[2][4];
                    _b[3] -= _f * _b[2];
                }
                {
                    const double _f = _J[4][2] * _r2;
                    _J[4][3] -= _f * _J[2][3];
                    _J[4][4] -= _f * _J[2][4];
                    _b[4] -= _f * _b[2];
                }
                {
                    int _p = 3;
                    double _bv = std::abs(_J[3][3]);
                    { const double _m = std::abs(_J[4][3]); if (_m > _bv) { _bv = _m; _p = 4; } }
                    if (_p == 4) {
                        { double _t = _J[3][3]; _J[3][3] = _J[4][3]; _J[4][3] = _t; }
                        { double _t = _J[3][4]; _J[3][4] = _J[4][4]; _J[4][4] = _t; }
                        { double _t = _b[3]; _b[3] = _b[4]; _b[4] = _t; }
                    }
                }
                const double _r3 = (double)1 / _J[3][3];
                {
                    const double _f = _J[4][3] * _r3;
                    _J[4][4] -= _f * _J[3][4];
                    _b[4] -= _f * _b[3];
                }
                const double _rb4 = (double)1 / _J[4][4];
                const double _rb0 = _r0;
                const double _rb1 = _r1;
                const double _rb2 = _r2;
                const double _rb3 = _r3;
                _b[4] = (_b[4]) * _rb4;
                _b[3] = (_b[3] - _J[3][4] * _b[4]) * _rb3;
                _b[2] = (_b[2] - _J[2][3] * _b[3] - _J[2][4] * _b[4]) * _rb2;
                _b[1] = (_b[1] - _J[1][2] * _b[2] - _J[1][3] * _b[3] - _J[1][4] * _b[4]) * _rb1;
                _b[0] = (_b[0] - _J[0][1] * _b[1] - _J[0][2] * _b[2] - _J[0][3] * _b[3] - _J[0][4] * _b[4]) * _rb0;
                const auto delta_vPKX1 = _b[0];
                const auto delta_vGKX1 = _b[1];
                const auto delta_vPKX2 = _b[2];
                const auto delta_vGKX2 = _b[3];
                const auto delta_vD1 = _b[4];
            
                auto residual_norm_sq = 0.0;
                residual_norm_sq += res_vPKX1 * res_vPKX1;
                residual_norm_sq += res_vGKX1 * res_vGKX1;
                residual_norm_sq += res_vPKX2 * res_vPKX2;
                residual_norm_sq += res_vGKX2 * res_vGKX2;
                residual_norm_sq += res_vD1 * res_vD1;
                auto step_norm_sq = 0.0;
                step_norm_sq += delta_vPKX1 * delta_vPKX1;
                step_norm_sq += delta_vGKX1 * delta_vGKX1;
                step_norm_sq += delta_vPKX2 * delta_vPKX2;
                step_norm_sq += delta_vGKX2 * delta_vGKX2;
                step_norm_sq += delta_vD1 * delta_vD1;
            
                vPKX1 = vPKX1 + (delta_vPKX1);
                vGKX1 = vGKX1 + (delta_vGKX1);
                vPKX2 = vPKX2 + (delta_vPKX2);
                vGKX2 = vGKX2 + (delta_vGKX2);
                vD1 = limit_junction_voltage(vD1 + (delta_vD1), vD1, D1N914_vt, vcrit_D1N914_vt);
            
                if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
                    break;
                
            }

            const auto _t9 = (zCout - zCsc);
            const auto _t11 = (D1N914_Is * (math_exp_approx((vD1 / D1N914_vt)) - 1.0));
            const auto _t15 = (_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi));
            const auto _t41 = (_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi));
            const auto _t73 = (_t9 * gCsc);
            const auto _t10 = (zCsc + _t11);
            const auto _t14 = (_t15 + _12AX7_DEMPWOLF_Ig0);
            const auto _t40 = (_t41 + _12AX7_DEMPWOLF_Ig0);
            const auto _t43 = ((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - _t40);
            const auto _t53 = (_t43 - zCc12);
            const auto _t55 = (zCc12 + _t14);
            const auto _t74 = (gCsc * _t10);
            const auto _t8 = (_t73 + _t74);
            const auto _t13 = ((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX2 / _12AX7_DEMPWOLF_Mu) + vGKX2))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - _t14);
            const auto _t42 = ((zRk1Ck1 + _t43) + _t40);
            const auto _t72 = (gRk2Ck2 * _t8);
            const auto _t106 = ((zRk2Ck2 + _t13) + _t14);
            const auto _t12 = (_t13 - zCout);
            const auto _t46 = (_t11 * _t32);
            const auto _t48 = (gCc12 * _t46);
            const auto _t52 = (_t53 * _t38);
            const auto _t59 = (_t55 * _t36);
            const auto _t67 = (_t4 * _t46);
            const auto _t71 = ((_t12 * _t33) + (gCout * _t72));
            const auto _t90 = (_t12 * _t16);
            const auto _t105 = ((_t7 * (_t106 * _t34)) - (gCout * (_t106 * _t17)));
            const auto _t6 = ((_t7 * _t72) + _t90);
            const auto _t47 = (gCc12 * _t48);
            const auto _t54 = (gCc12 * _t59);
            const auto _t58 = (_t2 * _t59);
            const auto _t63 = (_t53 * _t31);
            const auto _t66 = (_t2 * _t67);
            const auto _t70 = (gRatt * _t71);
            const auto _t76 = (gCc12 * _t70);
            const auto _t81 = (_t4 * _t70);
            const auto _t89 = (((gCout * (gRk2Ck2 * (gCout * _t10))) - (_t7 * (gRk2Ck2 * ((_t35 * _t10) + _t73)))) - _t90);
            const auto _t104 = (gRatt * _t105);
            const auto _t108 = (_t4 * _t104);
            const auto _t109 = (gCc12 * _t104);
            const auto _t5 = (gRatt * _t6);
            const auto _t24 = (gCc12 * _t5);
            const auto _t45 = (_t66 - _t47);
            const auto _t51 = (_t52 + _t54);
            const auto _t62 = (_t58 + _t63);
            const auto _t65 = (gRatt * _t45);
            const auto _t75 = (gCc12 * _t76);
            const auto _t80 = (_t2 * _t81);
            const auto _t88 = (gRatt * _t89);
            const auto _t99 = (_t4 * _t88);
            const auto _t103 = (gCc12 * _t109);
            const auto _t107 = (_t2 * _t108);
            const auto _t3 = (_t4 * _t5);
            const auto _t22 = (_t2 * _t3);
            const auto _t23 = (gCc12 * _t24);
            const auto _t49 = (zRrelCenv * _t29);
            const auto _t50 = (_t51 - _t56);
            const auto _t61 = (_t62 - _t64);
            const auto _t69 = (_t80 - _t75);
            const auto _t87 = (gCc12 * _t88);
            const auto _t97 = (gCc12 * _t87);
            const auto _t98 = (_t2 * _t99);
            const auto _t102 = (_t103 - _t107);
            const auto _t1 = (_t22 - _t23);
            const auto _t21 = (_t1 + _t25);
            const auto _t44 = (_t65 - _t49);
            const auto _t68 = (_t69 + _t82);
            const auto _t96 = (_t97 - _t98);
            const auto _t95 = (_t96 - _t91);
            const auto vo = (((_t0 * ((gRatt * _t21) - (_t20 * _t21))) + (gRfb * (gRfb * _t21))) * _t27);
            const auto vg1 = (-((_t40 - ((gCin * vi) - zCin)) * _t110));
            const auto tCin = (gCin * (vi - vg1));
            const auto vk1 = (((_t42 * _t28) + (gRfb * _t44)) * _t27);
            const auto vRk1Ck1 = (vk1 - 0);
            const auto vpl1 = (((_t0 * ((gRatt * _t50) - (_t20 * _t50))) + (gRfb * (gRfb * _t50))) * _t27);
            const auto vg2 = (((_t0 * ((gRatt * _t61) - (_t20 * _t61))) + (gRfb * (gRfb * _t61))) * _t27);
            const auto tCc12 = (gCc12 * (vpl1 - vg2));
            const auto venv = (-(((_t42 * _t39) - (_t0 * _t44)) * _t27));
            const auto vRrelCenv = (venv - 0);
            const auto vpl2 = (((_t0 * ((gRatt * _t68) - (_t20 * _t68))) + (gRfb * (gRfb * _t68))) * _t27);
            const auto tCout = (gCout * (vpl2 - vo));
            const auto vsc = (((_t0 * ((gRatt * _t95) - (_t20 * _t95))) + (gRfb * (gRfb * _t95))) * _t27);
            const auto tCsc = (gCsc * (vo - vsc));
            const auto vk2 = (((_t0 * ((gRatt * _t102) - (_t20 * _t102))) + (gRfb * (gRfb * _t102))) * _t27);
            const auto vRk2Ck2 = (vk2 - 0);
            
            zCin = 2 * tCin - zCin;
            zRk1Ck1 = gzRk1Ck1 * vRk1Ck1 - zRk1Ck1; // RC parallel
            zCc12 = 2 * tCc12 - zCc12;
            zRrelCenv = gzRrelCenv * vRrelCenv - zRrelCenv; // RC parallel
            zCout = 2 * tCout - zCout;
            zCsc = 2 * tCsc - zCsc;
            zRk2Ck2 = gzRk2Ck2 * vRk2Ck2 - zRk2Ck2; // RC parallel

            output[ch][n] = vo;
        }
        state[ch].zCin = zCin;
        state[ch].zRk1Ck1 = zRk1Ck1;
        state[ch].zCc12 = zCc12;
        state[ch].zRrelCenv = zRrelCenv;
        state[ch].zCout = zCout;
        state[ch].zCsc = zCsc;
        state[ch].zRk2Ck2 = zRk2Ck2;
        state[ch].vPKX1 = vPKX1;
        state[ch].vGKX1 = vGKX1;
        state[ch].vPKX2 = vPKX2;
        state[ch].vGKX2 = vGKX2;
        state[ch].vD1 = vD1;
        state[ch].nr_solves_X1_X2_D1 = nr_solves_X1_X2_D1;
        state[ch].nr_iters_X1_X2_D1 = nr_iters_X1_X2_D1;
    }
}

static float reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc = 0.0f)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto _12AX7_DEMPWOLF_Gk = params._12AX7_DEMPWOLF_Gk;
    const auto _12AX7_DEMPWOLF_Mu = params._12AX7_DEMPWOLF_Mu;
    const auto _12AX7_DEMPWOLF_Gamma = params._12AX7_DEMPWOLF_Gamma;
    const auto _12AX7_DEMPWOLF_Ck = params._12AX7_DEMPWOLF_Ck;
    const auto _12AX7_DEMPWOLF_Gg = params._12AX7_DEMPWOLF_Gg;
    const auto _12AX7_DEMPWOLF_Xi = params._12AX7_DEMPWOLF_Xi;
    const auto _12AX7_DEMPWOLF_Cg = params._12AX7_DEMPWOLF_Cg;
    const auto _12AX7_DEMPWOLF_Ig0 = params._12AX7_DEMPWOLF_Ig0;
    const auto D1N914_Is = params.D1N914_Is;
    const auto D1N914_vt = params.D1N914_vt;
    
    const auto VCC = params.VCC;
    
    const auto vp = VCC;
    
    const auto gCin = 2.0 * sample_rate * params.Cin;
    
    const auto gRg1 = 1.0 / params.Rg1;
    
    const auto Rk1 = params.Rk1;
    const auto Ck1 = params.Ck1;
    const auto gRk1Ck1 = 2.0 * sample_rate * Ck1 + (1.0 / Rk1);
    const auto gzRk1Ck1 = 4.0 * sample_rate * Ck1;
    
    const auto gRfb = 1.0 / params.Rfb;
    
    const auto gRp1 = 1.0 / params.Rp1;
    
    const auto gCc12 = 2.0 * sample_rate * params.Cc12;
    
    const auto gRg2 = 1.0 / params.Rg2;
    
    const auto gRatt = 1.0 / params.Ratt;
    
    const auto Rrel = params.Rrel;
    const auto Cenv = params.Cenv;
    const auto gRrelCenv = 2.0 * sample_rate * Cenv + (1.0 / Rrel);
    const auto gzRrelCenv = 4.0 * sample_rate * Cenv;
    
    const auto gRp2 = 1.0 / params.Rp2;
    
    const auto gCout = 2.0 * sample_rate * params.Cout;
    
    const auto gRL = 1.0 / params.RL;
    
    const auto gCsc = 2.0 * sample_rate * params.Csc;
    
    const auto Rk2 = params.Rk2;
    const auto Ck2 = params.Ck2;
    const auto gRk2Ck2 = 2.0 * sample_rate * Ck2 + (1.0 / Rk2);
    const auto gzRk2Ck2 = 4.0 * sample_rate * Ck2;
    
    const auto vcrit_D1N914_vt = D1N914_vt * std::log(D1N914_vt / (std::sqrt(2.0) * D1N914_Is));
    
    const auto vi = vi_dc;

    double vPKX1 = 0;
    double vGKX1 = 0;
    double vD1 = 0;
    double vPKX2 = 0;
    double vGKX2 = 0;

    const auto _X1_D1_zt5 = (gRfb + gRatt);
    const auto _X1_D1_zt14 = (1.0 / Rrel);
    const auto _X1_D1_zt22 = (1.0 / Rk2);
    const auto _X1_D1_zt31 = (1.0 / 1000000000.0);
    const auto _X1_D1_zt35 = (1.0 / Rk1);
    const auto _X1_D1_zt37 = (_X1_D1_zt5 + _X1_D1_zt14);
    const auto _X1_D1_zt43 = (_X1_D1_zt35 + gRfb);
    const auto _X1_D1_zt51 = (_X1_D1_zt22 + _X1_D1_zt31);
    const auto _X1_D1_zt52 = (gRL + _X1_D1_zt31);
    const auto _X1_D1_zt4 = (gRatt + _X1_D1_zt31);
    const auto _X1_D1_zt13 = (_X1_D1_zt37 + _X1_D1_zt31);
    const auto _X1_D1_zt18 = (_X1_D1_zt51 * _X1_D1_zt52);
    const auto _X1_D1_zt21 = (gRp1 + _X1_D1_zt31);
    const auto _X1_D1_zt25 = (gRg2 + _X1_D1_zt31);
    const auto _X1_D1_zt28 = (_X1_D1_zt43 + _X1_D1_zt31);
    const auto _X1_D1_zt29 = (gRp2 + _X1_D1_zt31);
    const auto _X1_D1_zt39 = (_X1_D1_zt29 * _X1_D1_zt18);
    const auto _X1_D1_zt48 = (_X1_D1_zt4 * _X1_D1_zt39);
    const auto _X1_D1_zt2 = (_X1_D1_zt25 * _X1_D1_zt48);
    const auto _X1_D1_zt3 = (_X1_D1_zt21 * _X1_D1_zt2);
    const auto _X1_D1_zt7 = (_X1_D1_zt2 * 1.0);
    const auto _X1_D1_zt17 = (_X1_D1_zt13 * _X1_D1_zt3);
    const auto _X1_D1_zt20 = (gRfb * _X1_D1_zt3);
    const auto _X1_D1_zt32 = (gRatt * _X1_D1_zt39);
    const auto _X1_D1_zt38 = (_X1_D1_zt39 * 1.0);
    const auto _X1_D1_zt46 = (_X1_D1_zt25 * _X1_D1_zt38);
    const auto _X1_D1_zt47 = (_X1_D1_zt25 * _X1_D1_zt32);
    const auto _X1_D1_zt1 = (_X1_D1_zt21 * _X1_D1_zt47);
    const auto _X1_D1_zt6 = (-_X1_D1_zt7);
    const auto _X1_D1_zt12 = (gRatt * _X1_D1_zt1);
    const auto _X1_D1_zt16 = (_X1_D1_zt12 - _X1_D1_zt17);
    const auto _X1_D1_zt19 = (gRfb * _X1_D1_zt20);
    const auto _X1_D1_zt27 = (gRfb * _X1_D1_zt1);
    const auto _X1_D1_zt44 = (_X1_D1_zt21 * _X1_D1_zt46);
    const auto _X1_D1_zt45 = (gRfb * _X1_D1_zt44);
    const auto _X1_D1_zt9 = (gRatt * _X1_D1_zt44);
    const auto _X1_D1_zt11 = (_X1_D1_zt16 * 1.0);
    const auto _X1_D1_zt15 = (_X1_D1_zt28 * _X1_D1_zt16);
    const auto _X1_D1_zt24 = (gRfb * _X1_D1_zt9);
    const auto _X1_D1_zt26 = (_X1_D1_zt27 * 1.0);
    const auto _X1_D1_zt36 = (_X1_D1_zt13 * _X1_D1_zt44);
    const auto _X1_D1_zt40 = (gRfb * _X1_D1_zt45);
    const auto _X1_D1_zt8 = (_X1_D1_zt15 + _X1_D1_zt19);
    const auto _X1_D1_zt10 = (-_X1_D1_zt11);
    const auto _X1_D1_zt23 = (_X1_D1_zt24 / _X1_D1_zt8);
    const auto _X1_D1_zt30 = (_X1_D1_zt26 / _X1_D1_zt8);
    const auto _X1_D1_zt33 = (_X1_D1_zt30 - _X1_D1_zt30);
    const auto _X1_D1_zt34 = (_X1_D1_zt28 * _X1_D1_zt36);
    const auto _X1_D1_zt42 = (_X1_D1_zt34 - _X1_D1_zt40);
    const auto _X1_D1_zt0 = (_X1_D1_zt10 / _X1_D1_zt8);
    const auto _X1_D1_zt41 = (_X1_D1_zt42 / _X1_D1_zt8);
    const auto _X1_D1_Z0_0 = (_X1_D1_zt0 + (((_X1_D1_zt28 * ((gRatt * (-(_X1_D1_zt47 * 1.0))) - (_X1_D1_zt13 * _X1_D1_zt6))) + (gRfb * (gRfb * _X1_D1_zt6))) / _X1_D1_zt8));
    const auto _X1_D1_Z0_1 = _X1_D1_zt0;
    const auto _X1_D1_Z0_2 = _X1_D1_zt23;
    const auto _X1_D1_Z1_0 = _X1_D1_zt0;
    const auto _X1_D1_Z1_1 = (_X1_D1_zt0 + (-(1.0 / (gRg1 + _X1_D1_zt31))));
    const auto _X1_D1_Z1_2 = _X1_D1_zt23;
    const auto _X1_D1_Z2_0 = _X1_D1_zt33;
    const auto _X1_D1_Z2_1 = _X1_D1_zt33;
    const auto _X1_D1_Z2_2 = (_X1_D1_zt41 - _X1_D1_zt41);
    // --- Newton-Raphson solve (N-port): X1_D1
    const auto _X1_D1_zt50 = (gRp1 * _X1_D1_zt2);
    const auto _X1_D1_zt49 = (VCC * _X1_D1_zt50);
    const auto _X1_D1_voc0 = (((_X1_D1_zt28 * ((gRatt * (VCC * (gRp1 * _X1_D1_zt47))) - (_X1_D1_zt13 * _X1_D1_zt49))) + (gRfb * (gRfb * _X1_D1_zt49))) / _X1_D1_zt8);
    const auto _X1_D1_pt16 = (_12AX7_DEMPWOLF_Gamma - 1.0);
    const auto _X1_D1_pt18 = (_12AX7_DEMPWOLF_Xi - 1.0);
    for (int newton_iter = 0; newton_iter < 10000; ++newton_iter)
    {
        const auto _X1_D1_pt2 = math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1)));
        const auto _X1_D1_pt6 = (_12AX7_DEMPWOLF_Cg * vGKX1);
        const auto _X1_D1_pt11 = (vD1 / D1N914_vt);
        const auto _X1_D1_pt23 = math_exp_approx(_X1_D1_pt11);
        const auto _X1_D1_pt1 = (1.0 + _X1_D1_pt2);
        const auto _X1_D1_pt5 = math_exp_approx(_X1_D1_pt6);
        const auto _X1_D1_pt10 = (1.0 + _X1_D1_pt5);
        const auto _X1_D1_pt0 = (math_log_approx(_X1_D1_pt1) / _12AX7_DEMPWOLF_Ck);
        const auto _X1_D1_pt4 = math_log_approx(_X1_D1_pt10);
        const auto _X1_D1_pt9 = (_X1_D1_pt4 / _12AX7_DEMPWOLF_Cg);
        const auto _X1_D1_pt15 = math_pow_approx(_X1_D1_pt0, _X1_D1_pt16);
        const auto _X1_D1_pt22 = math_pow_approx(_X1_D1_pt9, _X1_D1_pt18);
        const auto _X1_D1_pt8 = math_pow_approx(_X1_D1_pt9, _12AX7_DEMPWOLF_Xi);
        const auto _X1_D1_pt13 = (_X1_D1_pt15 * _X1_D1_pt2);
        const auto _X1_D1_pt14 = (_12AX7_DEMPWOLF_Gamma * _X1_D1_pt13);
        const auto _X1_D1_pt21 = (_X1_D1_pt22 * _X1_D1_pt5);
        const auto _X1_D1_pt3 = (_12AX7_DEMPWOLF_Gg * _X1_D1_pt8);
        const auto _X1_D1_pt7 = (_X1_D1_pt3 + _12AX7_DEMPWOLF_Ig0);
        const auto _X1_D1_pt12 = (_12AX7_DEMPWOLF_Gk * _X1_D1_pt14);
        const auto _X1_D1_pt17 = (_12AX7_DEMPWOLF_Xi * _X1_D1_pt21);
        const auto _X1_D1_pt20 = (_12AX7_DEMPWOLF_Gg * _X1_D1_pt17);
        const auto _X1_D1_pt19 = (_X1_D1_pt20 / _X1_D1_pt10);
        const auto _X1_D1_i0 = ((_12AX7_DEMPWOLF_Gk * math_pow_approx(_X1_D1_pt0, _12AX7_DEMPWOLF_Gamma)) - _X1_D1_pt7);
        const auto _X1_D1_i1 = _X1_D1_pt7;
        const auto _X1_D1_i2 = (D1N914_Is * (_X1_D1_pt23 - 1.0));
        const auto _X1_D1_g0_0 = (_X1_D1_pt12 / (_12AX7_DEMPWOLF_Mu * _X1_D1_pt1));
        const auto _X1_D1_g0_1 = ((_X1_D1_pt12 / _X1_D1_pt1) - _X1_D1_pt19);
        const auto _X1_D1_g1_0 = 0.0;
        const auto _X1_D1_g1_1 = _X1_D1_pt19;
        const auto _X1_D1_g2_2 = ((D1N914_Is * _X1_D1_pt23) / D1N914_vt);
        const auto _X1_D1_pt24 = (((_X1_D1_voc0 + (_X1_D1_Z0_0 * _X1_D1_i0)) + (_X1_D1_Z0_1 * _X1_D1_i1)) + (_X1_D1_Z0_2 * _X1_D1_i2));
        const auto _X1_D1_pt25 = (((_X1_D1_Z1_0 * _X1_D1_i0) + (_X1_D1_Z1_1 * _X1_D1_i1)) + (_X1_D1_Z1_2 * _X1_D1_i2));
        const auto _X1_D1_pt26 = (((_X1_D1_Z2_0 * _X1_D1_i0) + (_X1_D1_Z2_1 * _X1_D1_i1)) + (_X1_D1_Z2_2 * _X1_D1_i2));
        const auto _X1_D1_pt27 = (vPKX1 - _X1_D1_pt24);
        const auto _X1_D1_pt29 = (((_X1_D1_Z1_0 * _X1_D1_g0_1) + (_X1_D1_Z1_1 * _X1_D1_g1_1)) - 1.0);
        const auto _X1_D1_pt30 = ((_X1_D1_Z2_2 * _X1_D1_g2_2) - 1.0);
        const auto _X1_D1_pt31 = (_X1_D1_Z1_2 * _X1_D1_g2_2);
        const auto _X1_D1_pt32 = ((_X1_D1_Z2_0 * _X1_D1_g0_1) + (_X1_D1_Z2_1 * _X1_D1_g1_1));
        const auto _X1_D1_pt33 = ((_X1_D1_Z0_0 * _X1_D1_g0_1) + (_X1_D1_Z0_1 * _X1_D1_g1_1));
        const auto _X1_D1_pt36 = (vGKX1 - _X1_D1_pt25);
        const auto _X1_D1_pt37 = (vD1 - _X1_D1_pt26);
        const auto _X1_D1_pt38 = (_X1_D1_Z0_2 * _X1_D1_g2_2);
        const auto _X1_D1_pt39 = (_X1_D1_pt36 * _X1_D1_pt32);
        const auto _X1_D1_pt41 = (((_X1_D1_Z0_0 * _X1_D1_g0_0) + (_X1_D1_Z0_1 * _X1_D1_g1_0)) - 1.0);
        const auto _X1_D1_pt43 = ((_X1_D1_Z1_0 * _X1_D1_g0_0) + (_X1_D1_Z1_1 * _X1_D1_g1_0));
        const auto _X1_D1_pt44 = ((_X1_D1_Z2_0 * _X1_D1_g0_0) + (_X1_D1_Z2_1 * _X1_D1_g1_0));
        const auto _X1_D1_pt45 = ((_X1_D1_pt43 * _X1_D1_pt32) - (_X1_D1_pt29 * _X1_D1_pt44));
        const auto _X1_D1_pt46 = (_X1_D1_pt31 * _X1_D1_pt37);
        const auto _X1_D1_pt47 = (_X1_D1_pt36 * _X1_D1_pt44);
        const auto _X1_D1_pt48 = (_X1_D1_pt29 * _X1_D1_pt37);
        const auto _X1_D1_pt50 = (_X1_D1_pt43 * _X1_D1_pt37);
        const auto _X1_D1_pt28 = ((_X1_D1_pt29 * _X1_D1_pt30) - (_X1_D1_pt31 * _X1_D1_pt32));
        const auto _X1_D1_pt35 = (_X1_D1_pt36 * _X1_D1_pt30);
        const auto _X1_D1_pt42 = ((_X1_D1_pt43 * _X1_D1_pt30) - (_X1_D1_pt31 * _X1_D1_pt44));
        const auto _X1_D1_pt49 = (_X1_D1_pt50 - _X1_D1_pt47);
        const auto _X1_D1_pt34 = (_X1_D1_pt35 - _X1_D1_pt46);
        const auto _X1_D1_pt40 = (((_X1_D1_pt41 * _X1_D1_pt28) - (_X1_D1_pt33 * _X1_D1_pt42)) + (_X1_D1_pt38 * _X1_D1_pt45));
        const auto res_vPKX1 = (_X1_D1_pt24 - vPKX1);
        const auto res_vGKX1 = (_X1_D1_pt25 - vGKX1);
        const auto res_vD1 = (_X1_D1_pt26 - vD1);
        const auto delta_vPKX1 = ((((_X1_D1_pt27 * _X1_D1_pt28) - (_X1_D1_pt33 * _X1_D1_pt34)) + (_X1_D1_pt38 * (_X1_D1_pt39 - _X1_D1_pt48))) / _X1_D1_pt40);
        const auto delta_vGKX1 = ((((_X1_D1_pt41 * _X1_D1_pt34) - (_X1_D1_pt27 * _X1_D1_pt42)) + (_X1_D1_pt38 * _X1_D1_pt49)) / _X1_D1_pt40);
        const auto delta_vD1 = ((((_X1_D1_pt41 * (_X1_D1_pt48 - _X1_D1_pt39)) - (_X1_D1_pt33 * _X1_D1_pt49)) + (_X1_D1_pt27 * _X1_D1_pt45)) / _X1_D1_pt40);
    
        auto residual_norm_sq = 0.0;
        residual_norm_sq += res_vPKX1 * res_vPKX1;
        residual_norm_sq += res_vGKX1 * res_vGKX1;
        residual_norm_sq += res_vD1 * res_vD1;
        auto step_norm_sq = 0.0;
        step_norm_sq += delta_vPKX1 * delta_vPKX1;
        step_norm_sq += delta_vGKX1 * delta_vGKX1;
        step_norm_sq += delta_vD1 * delta_vD1;
    
        vPKX1 = vPKX1 + (delta_vPKX1);
        vGKX1 = vGKX1 + (delta_vGKX1);
        vD1 = limit_junction_voltage(vD1 + (delta_vD1), vD1, D1N914_vt, vcrit_D1N914_vt);
    
        if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
            break;
        
    }
    const auto _X2_zt0 = (1.0 / Rk1);
    const auto _X2_zt41 = (_X2_zt0 + gRfb);
    const auto _X2_zt43 = (1.0 / 1000000000.0);
    const auto _X2_zt47 = (gRfb + gRatt);
    const auto _X2_zt48 = (1.0 / Rrel);
    const auto _X2_zt51 = (1.0 / Rk2);
    const auto _X2_zt52 = (gRatt + _X2_zt43);
    const auto _X2_zt8 = (_X2_zt47 + _X2_zt48);
    const auto _X2_zt19 = (gRL + _X2_zt43);
    const auto _X2_zt24 = (_X2_zt19 * 1.0);
    const auto _X2_zt27 = (gRg2 + _X2_zt43);
    const auto _X2_zt33 = (_X2_zt41 + _X2_zt43);
    const auto _X2_zt34 = (gRp1 + _X2_zt43);
    const auto _X2_zt35 = (gRp2 + _X2_zt43);
    const auto _X2_zt42 = (_X2_zt8 + _X2_zt43);
    const auto _X2_zt49 = (_X2_zt51 + _X2_zt43);
    const auto _X2_zt58 = (_X2_zt49 * _X2_zt19);
    const auto _X2_zt5 = (_X2_zt58 * 1.0);
    const auto _X2_zt9 = (_X2_zt35 * _X2_zt58);
    const auto _X2_zt12 = (_X2_zt35 * _X2_zt24);
    const auto _X2_zt15 = (gRatt * _X2_zt9);
    const auto _X2_zt17 = (_X2_zt52 * _X2_zt9);
    const auto _X2_zt28 = (gRatt * _X2_zt12);
    const auto _X2_zt38 = (_X2_zt52 * _X2_zt12);
    const auto _X2_zt40 = (_X2_zt17 * 1.0);
    const auto _X2_zt46 = (_X2_zt27 * _X2_zt15);
    const auto _X2_zt4 = (_X2_zt52 * _X2_zt5);
    const auto _X2_zt14 = (_X2_zt34 * _X2_zt46);
    const auto _X2_zt16 = (_X2_zt27 * _X2_zt17);
    const auto _X2_zt23 = (_X2_zt27 * _X2_zt28);
    const auto _X2_zt31 = (_X2_zt27 * _X2_zt38);
    const auto _X2_zt39 = (_X2_zt34 * _X2_zt40);
    const auto _X2_zt45 = (gRatt * _X2_zt14);
    const auto _X2_zt3 = (_X2_zt27 * _X2_zt4);
    const auto _X2_zt10 = (_X2_zt34 * _X2_zt16);
    const auto _X2_zt18 = (gRfb * _X2_zt10);
    const auto _X2_zt22 = (_X2_zt34 * _X2_zt23);
    const auto _X2_zt26 = (gRatt * _X2_zt22);
    const auto _X2_zt29 = (_X2_zt34 * _X2_zt31);
    const auto _X2_zt37 = (_X2_zt42 * _X2_zt29);
    const auto _X2_zt50 = (gRfb * _X2_zt18);
    const auto _X2_zt2 = (_X2_zt34 * _X2_zt3);
    const auto _X2_zt7 = (_X2_zt42 * _X2_zt10);
    const auto _X2_zt13 = (gRfb * _X2_zt29);
    const auto _X2_zt21 = (_X2_zt26 - _X2_zt37);
    const auto _X2_zt30 = (gRfb * _X2_zt13);
    const auto _X2_zt44 = (_X2_zt45 - _X2_zt7);
    const auto _X2_zt1 = (-_X2_zt2);
    const auto _X2_zt6 = (_X2_zt33 * _X2_zt44);
    const auto _X2_zt11 = (_X2_zt33 * _X2_zt21);
    const auto _X2_zt25 = (_X2_zt11 + _X2_zt30);
    const auto _X2_zt32 = (_X2_zt6 + _X2_zt50);
    const auto _X2_zt20 = (_X2_zt25 / _X2_zt32);
    const auto _X2_zt36 = (-_X2_zt20);
    const auto _X2_Z0_0 = ((((_X2_zt33 * ((gRatt * (-(_X2_zt34 * (_X2_zt27 * (gRatt * _X2_zt5))))) - (_X2_zt42 * _X2_zt1))) + (gRfb * (gRfb * _X2_zt1))) / _X2_zt32) + _X2_zt36);
    const auto _X2_Z0_1 = _X2_zt36;
    const auto _X2_Z1_0 = _X2_zt36;
    const auto _X2_Z1_1 = (_X2_zt36 + (-(((gRfb * (gRfb * _X2_zt39)) - (_X2_zt33 * ((_X2_zt42 * _X2_zt39) - (gRatt * (_X2_zt34 * (_X2_zt15 * 1.0)))))) / _X2_zt32)));
    // --- Newton-Raphson solve (N-port): X2
    const auto _X2_zt53 = (gRp2 * _X2_zt58);
    const auto _X2_zt57 = (_X2_zt52 * _X2_zt53);
    const auto _X2_zt56 = (_X2_zt27 * _X2_zt57);
    const auto _X2_zt55 = (_X2_zt34 * _X2_zt56);
    const auto _X2_zt54 = (VCC * _X2_zt55);
    const auto _X2_voc0 = (((_X2_zt33 * ((gRatt * (VCC * (_X2_zt34 * (_X2_zt27 * (gRatt * _X2_zt53))))) - (_X2_zt42 * _X2_zt54))) + (gRfb * (gRfb * _X2_zt54))) / _X2_zt32);
    const auto _X2_pt14 = (_12AX7_DEMPWOLF_Gamma - 1.0);
    const auto _X2_pt21 = (_12AX7_DEMPWOLF_Xi - 1.0);
    for (int newton_iter = 0; newton_iter < 10000; ++newton_iter)
    {
        const auto _X2_pt2 = math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX2 / _12AX7_DEMPWOLF_Mu) + vGKX2)));
        const auto _X2_pt7 = (_12AX7_DEMPWOLF_Cg * vGKX2);
        const auto _X2_pt10 = math_exp_approx(_X2_pt7);
        const auto _X2_pt1 = (1.0 + _X2_pt2);
        const auto _X2_pt6 = (1.0 + _X2_pt10);
        const auto _X2_pt9 = math_log_approx(_X2_pt6);
        const auto _X2_pt0 = (math_log_approx(_X2_pt1) / _12AX7_DEMPWOLF_Ck);
        const auto _X2_pt5 = (_X2_pt9 / _12AX7_DEMPWOLF_Cg);
        const auto _X2_pt8 = math_pow_approx(_X2_pt5, _12AX7_DEMPWOLF_Xi);
        const auto _X2_pt13 = math_pow_approx(_X2_pt0, _X2_pt14);
        const auto _X2_pt19 = math_pow_approx(_X2_pt5, _X2_pt21);
        const auto _X2_pt4 = (_12AX7_DEMPWOLF_Gg * _X2_pt8);
        const auto _X2_pt12 = (_X2_pt13 * _X2_pt2);
        const auto _X2_pt18 = (_X2_pt19 * _X2_pt10);
        const auto _X2_pt20 = (_12AX7_DEMPWOLF_Xi * _X2_pt18);
        const auto _X2_pt3 = (_X2_pt4 + _12AX7_DEMPWOLF_Ig0);
        const auto _X2_pt11 = (_12AX7_DEMPWOLF_Gamma * _X2_pt12);
        const auto _X2_pt15 = (_12AX7_DEMPWOLF_Gk * _X2_pt11);
        const auto _X2_pt17 = (_12AX7_DEMPWOLF_Gg * _X2_pt20);
        const auto _X2_pt16 = (_X2_pt17 / _X2_pt6);
        const auto _X2_i0 = ((_12AX7_DEMPWOLF_Gk * math_pow_approx(_X2_pt0, _12AX7_DEMPWOLF_Gamma)) - _X2_pt3);
        const auto _X2_i1 = _X2_pt3;
        const auto _X2_g0_0 = (_X2_pt15 / (_12AX7_DEMPWOLF_Mu * _X2_pt1));
        const auto _X2_g0_1 = ((_X2_pt15 / _X2_pt1) - _X2_pt16);
        const auto _X2_g1_0 = 0.0;
        const auto _X2_g1_1 = _X2_pt16;
        const auto _X2_pt22 = ((_X2_voc0 + (_X2_Z0_0 * _X2_i0)) + (_X2_Z0_1 * _X2_i1));
        const auto _X2_pt23 = ((_X2_Z1_0 * _X2_i0) + (_X2_Z1_1 * _X2_i1));
        const auto _X2_pt24 = (vPKX2 - _X2_pt22);
        const auto _X2_pt25 = (((_X2_Z1_0 * _X2_g0_1) + (_X2_Z1_1 * _X2_g1_1)) - 1.0);
        const auto _X2_pt26 = ((_X2_Z0_0 * _X2_g0_1) + (_X2_Z0_1 * _X2_g1_1));
        const auto _X2_pt27 = (vGKX2 - _X2_pt23);
        const auto _X2_pt29 = (((_X2_Z0_0 * _X2_g0_0) + (_X2_Z0_1 * _X2_g1_0)) - 1.0);
        const auto _X2_pt30 = ((_X2_Z1_0 * _X2_g0_0) + (_X2_Z1_1 * _X2_g1_0));
        const auto _X2_pt28 = ((_X2_pt29 * _X2_pt25) - (_X2_pt26 * _X2_pt30));
        const auto res_vPKX2 = (_X2_pt22 - vPKX2);
        const auto res_vGKX2 = (_X2_pt23 - vGKX2);
        const auto delta_vPKX2 = (((_X2_pt24 * _X2_pt25) - (_X2_pt26 * _X2_pt27)) / _X2_pt28);
        const auto delta_vGKX2 = (((_X2_pt29 * _X2_pt27) - (_X2_pt24 * _X2_pt30)) / _X2_pt28);
    
        auto residual_norm_sq = 0.0;
        residual_norm_sq += res_vPKX2 * res_vPKX2;
        residual_norm_sq += res_vGKX2 * res_vGKX2;
        auto step_norm_sq = 0.0;
        step_norm_sq += delta_vPKX2 * delta_vPKX2;
        step_norm_sq += delta_vGKX2 * delta_vGKX2;
    
        vPKX2 = vPKX2 + (delta_vPKX2);
        vGKX2 = vGKX2 + (delta_vGKX2);
    
        if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
            break;
        
    }
    const auto zCin = (gCin * (vi + (((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0) / (gRg1 + (1.0 / 1000000000.0)))));
    const auto zRk1Ck1 = (-(((gzRk1Ck1 * ((gRfb * (gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((D1N914_Is * (math_exp_approx((vD1 / D1N914_vt)) - 1.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))) - ((((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) + ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))))) / (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))) + (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) / 2.0));
    const auto zCc12 = (gCc12 * (((((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((VCC * (gRp1 * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - (((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((VCC * (gRp1 * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - (((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) + (gRfb * (gRfb * ((VCC * (gRp1 * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - (((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) / (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))) + (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) + (((gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * (((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))) - ((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * (((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * (((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - (gRatt * ((gRp1 + (1.0 / 1000000000.0)) * (((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) / (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))) + (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))))));
    const auto zRrelCenv = (-(((gzRrelCenv * (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * (gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((D1N914_Is * (math_exp_approx((vD1 / D1N914_vt)) - 1.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))) + ((((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) + ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) / (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))) + (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) / 2.0));
    const auto zCout = ((gCout * (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((VCC * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * (gRp2 * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * (((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX2 / _12AX7_DEMPWOLF_Mu) + vGKX2))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((VCC * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * (gRp2 * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * (((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX2 / _12AX7_DEMPWOLF_Mu) + vGKX2))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) + (gRfb * (gRfb * ((VCC * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * (gRp2 * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * (((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX2 / _12AX7_DEMPWOLF_Mu) + vGKX2))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))))) / (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))) + (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))));
    const auto zCsc = ((gCsc * ((((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((D1N914_Is * (math_exp_approx((vD1 / D1N914_vt)) - 1.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))) - (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((D1N914_Is * (math_exp_approx((vD1 / D1N914_vt)) - 1.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))) + ((((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) + ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) / (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))) + (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))));
    const auto zRk2Ck2 = (((gzRk2Ck2 * (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * ((((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX2 / _12AX7_DEMPWOLF_Mu) + vGKX2))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) + ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * ((((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX2 / _12AX7_DEMPWOLF_Mu) + vGKX2))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) + ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * (gRL + (1.0 / 1000000000.0)))))))))) + (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * ((((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX2 / _12AX7_DEMPWOLF_Mu) + vGKX2))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) + ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * (gRL + (1.0 / 1000000000.0))))))))))) / (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))) + (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) / 2.0);

    const auto vo_dc_out = 0.0;

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].vPKX1 = vPKX1;
        state[ch].vGKX1 = vGKX1;
        state[ch].vD1 = vD1;
        state[ch].vPKX2 = vPKX2;
        state[ch].vGKX2 = vGKX2;
        state[ch].zCin = zCin;
        state[ch].zRk1Ck1 = zRk1Ck1;
        state[ch].zCc12 = zCc12;
        state[ch].zRrelCenv = zRrelCenv;
        state[ch].zCout = zCout;
        state[ch].zCsc = zCsc;
        state[ch].zRk2Ck2 = zRk2Ck2;
        state[ch].nr_solves_X1_X2_D1 = 0;
        state[ch].nr_iters_X1_X2_D1 = 0;
    }
    return vo_dc_out;
}

