// Auto-generated with netlist_codegen version eaa29c0.
// Command: netlist_codegen tube_compressor.net tube_compressor.h -type_name double -instrument

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
    double X1_Cpg = 2.4e-12;
    double Cc12 = 4.7e-08;
    double Rg2 = 2.2e+05;
    double Ratt = 1.0e+04;
    double Rrel = 3.3e+05;
    double Cenv = 1.0e-06;
    double Rp2 = 4.7e+04;
    double X2_Cpg = 2.4e-12;
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
    double zX1_Cpg {};
    double zCc12 {};
    double zRrelCenv {};
    double zX2_Cpg {};
    double zCout {};
    double zCsc {};
    double zRk2Ck2 {};
    double vPKX1 {};
    double vPKX1_prev {};
    double vGKX1 {};
    double vGKX1_prev {};
    double vPKX2 {};
    double vPKX2_prev {};
    double vGKX2 {};
    double vGKX2_prev {};
    double vD1 {};
    double vD1_prev {};
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
    
    const auto gX1_Cpg = 2.0 * sample_rate * params.X1_Cpg;
    
    const auto gCc12 = 2.0 * sample_rate * params.Cc12;
    
    const auto gRg2 = 1.0 / params.Rg2;
    
    const auto gRatt = 1.0 / params.Ratt;
    
    const auto Rrel = params.Rrel;
    const auto Cenv = params.Cenv;
    const auto gRrelCenv = 2.0 * sample_rate * Cenv + (1.0 / Rrel);
    const auto gzRrelCenv = 4.0 * sample_rate * Cenv;
    
    const auto gRp2 = 1.0 / params.Rp2;
    
    const auto gX2_Cpg = 2.0 * sample_rate * params.X2_Cpg;
    
    const auto gCout = 2.0 * sample_rate * params.Cout;
    
    const auto gRL = 1.0 / params.RL;
    
    const auto gCsc = 2.0 * sample_rate * params.Csc;
    
    const auto Rk2 = params.Rk2;
    const auto Ck2 = params.Ck2;
    const auto gRk2Ck2 = 2.0 * sample_rate * Ck2 + (1.0 / Rk2);
    const auto gzRk2Ck2 = 4.0 * sample_rate * Ck2;
    
    const auto vcrit_D1N914_vt = D1N914_vt * std::log(D1N914_vt / (std::sqrt(2.0) * D1N914_Is));
    
    const auto _t0 = ((gCin + gRg1) + gX1_Cpg);
    const auto _t1 = (gRk1Ck1 + gRfb);
    const auto _t9 = (gCout * gCsc);
    const auto _t10 = ((gRp1 + gX1_Cpg) + gCc12);
    const auto _t18 = ((gCc12 + gRg2) + gX2_Cpg);
    const auto _t20 = ((gRp2 + gX2_Cpg) + gCout);
    const auto _t32 = ((gRfb + gRatt) + gRrelCenv);
    const auto _t59 = ((gCout + gRL) + gCsc);
    const auto _t8 = (gRk2Ck2 * _t9);
    const auto _t30 = (gRp2 * _t8);
    const auto _t58 = ((_t59 * gCsc) - (gCsc * gCsc));
    const auto _t7 = (gX2_Cpg * _t8);
    const auto _t29 = (gRatt * _t30);
    const auto _t31 = (_t18 * _t29);
    const auto _t47 = (gX1_Cpg * _t31);
    const auto _t57 = (gRk2Ck2 * _t58);
    const auto _t62 = (gX2_Cpg * _t57);
    const auto _t77 = (gRp2 * _t57);
    const auto _t158 = (gCc12 * _t29);
    const auto _t165 = (gRatt * _t7);
    const auto _t167 = (_t10 * _t31);
    const auto _t168 = (gCc12 * _t158);
    const auto _t170 = (VCC * _t47);
    const auto _t35 = (gCc12 * _t165);
    const auto _t56 = ((_t20 * _t57) - (gCout * _t8));
    const auto _t61 = (gRatt * _t62);
    const auto _t64 = (gRatt * _t56);
    const auto _t66 = (_t18 * _t64);
    const auto _t67 = (gX2_Cpg * _t61);
    const auto _t76 = (gRatt * _t77);
    const auto _t125 = (gCc12 * _t61);
    const auto _t133 = (_t18 * _t76);
    const auto _t141 = (gCc12 * _t76);
    const auto _t142 = (gRp1 * _t125);
    const auto _t146 = (gX1_Cpg * _t133);
    const auto _t147 = (gX1_Cpg * _t125);
    const auto _t157 = (gRp1 * _t35);
    const auto _t173 = (gX1_Cpg * _t35);
    const auto _t174 = (gRfb * _t173);
    const auto _t44 = (_t157 + _t167);
    const auto _t49 = (gRatt * _t173);
    const auto _t50 = (gRfb * _t174);
    const auto _t55 = (gCc12 * _t64);
    const auto _t60 = (_t66 - _t67);
    const auto _t65 = (_t10 * _t60);
    const auto _t71 = (gX1_Cpg * _t60);
    const auto _t72 = (gRfb * _t71);
    const auto _t75 = (gX2_Cpg * _t76);
    const auto _t97 = (gRp1 * _t60);
    const auto _t98 = (gCc12 * _t75);
    const auto _t107 = (gRp1 * _t55);
    const auto _t110 = (_t10 * _t75);
    const auto _t113 = (gX1_Cpg * _t75);
    const auto _t115 = (gX1_Cpg * _t55);
    const auto _t132 = (_t10 * _t133);
    const auto _t140 = (gCc12 * _t141);
    const auto _t145 = (VCC * _t146);
    const auto _t156 = (_t44 - _t168);
    const auto _t166 = (VCC * _t156);
    const auto _t172 = (_t32 * _t173);
    const auto _t28 = (_t168 - _t44);
    const auto _t43 = (VCC * _t28);
    const auto _t54 = (gCc12 * _t55);
    const auto _t63 = (_t54 - _t65);
    const auto _t68 = (gRfb * _t63);
    const auto _t70 = ((gRatt * _t71) - (_t32 * _t71));
    const auto _t96 = (_t97 + _t98);
    const auto _t109 = (_t107 + _t110);
    const auto _t112 = (VCC * _t113);
    const auto _t131 = (_t142 + _t132);
    const auto _t139 = (_t140 - _t131);
    const auto _t53 = ((gRatt * _t63) - (_t32 * _t63));
    const auto _t69 = ((_t1 * _t70) + (gRfb * _t72));
    const auto _t74 = (VCC * _t96);
    const auto _t108 = (VCC * _t109);
    const auto _t130 = (VCC * _t139);
    const auto _t52 = ((_t1 * _t53) + (gRfb * _t68));
    const auto _t51 = (1.0 / ((_t0 * _t52) + (gX1_Cpg * _t69)));
    const auto _t189 = ((_t1 * (_t172 - _t49)) - _t50);
    const auto _t190 = ((_t1 * ((_t32 * _t115) - (gRatt * _t115))) - (gRfb * (gRfb * _t115)));
    const auto _t191 = ((_t1 * ((_t32 * _t147) - (gRatt * _t147))) - (gRfb * (gRfb * _t147)));
    const auto _t192 = ((_t1 * (_t49 - _t172)) + _t50);
    const auto _X1_X2_D1_zt56 = (gCsc * gCsc);
    const auto _X1_X2_D1_zt61 = (gCin + gRg1);
    const auto _X1_X2_D1_zt64 = (_X1_X2_D1_zt61 + gX1_Cpg);
    const auto _X1_X2_D1_zt78 = (gCout * gCsc);
    const auto _X1_X2_D1_zt89 = (gRfb + gRatt);
    const auto _X1_X2_D1_zt102 = (gCsc * 1.0);
    const auto _X1_X2_D1_zt127 = (gCout + gRL);
    const auto _X1_X2_D1_zt145 = (gRk1Ck1 + gRfb);
    const auto _X1_X2_D1_zt146 = (gCc12 + gRg2);
    const auto _X1_X2_D1_zt190 = (gCout * 1.0);
    const auto _X1_X2_D1_zt192 = (_X1_X2_D1_zt89 + gRrelCenv);
    const auto _X1_X2_D1_zt208 = (gRp2 + gX2_Cpg);
    const auto _X1_X2_D1_zt260 = (_X1_X2_D1_zt127 + gCsc);
    const auto _X1_X2_D1_zt325 = (gRp1 + gX1_Cpg);
    const auto _X1_X2_D1_zt4 = (gRk2Ck2 * _X1_X2_D1_zt78);
    const auto _X1_X2_D1_zt38 = (_X1_X2_D1_zt146 + gX2_Cpg);
    const auto _X1_X2_D1_zt42 = (_X1_X2_D1_zt208 + gCout);
    const auto _X1_X2_D1_zt60 = (gCout * _X1_X2_D1_zt4);
    const auto _X1_X2_D1_zt76 = (_X1_X2_D1_zt78 * 1.0);
    const auto _X1_X2_D1_zt81 = (gCout * _X1_X2_D1_zt76);
    const auto _X1_X2_D1_zt83 = (_X1_X2_D1_zt260 * gCsc);
    const auto _X1_X2_D1_zt88 = (_X1_X2_D1_zt83 - _X1_X2_D1_zt56);
    const auto _X1_X2_D1_zt101 = (gRk2Ck2 * _X1_X2_D1_zt102);
    const auto _X1_X2_D1_zt116 = (gRk2Ck2 * _X1_X2_D1_zt88);
    const auto _X1_X2_D1_zt139 = (gCout * _X1_X2_D1_zt101);
    const auto _X1_X2_D1_zt140 = (gX2_Cpg * _X1_X2_D1_zt4);
    const auto _X1_X2_D1_zt141 = (gRatt * _X1_X2_D1_zt140);
    const auto _X1_X2_D1_zt143 = (gCc12 * _X1_X2_D1_zt141);
    const auto _X1_X2_D1_zt157 = (gX1_Cpg * _X1_X2_D1_zt143);
    const auto _X1_X2_D1_zt161 = (_X1_X2_D1_zt4 * 1.0);
    const auto _X1_X2_D1_zt170 = (_X1_X2_D1_zt141 * 1.0);
    const auto _X1_X2_D1_zt179 = (_X1_X2_D1_zt260 * 1.0);
    const auto _X1_X2_D1_zt187 = (gRk2Ck2 * _X1_X2_D1_zt190);
    const auto _X1_X2_D1_zt188 = (gRk2Ck2 * _X1_X2_D1_zt179);
    const auto _X1_X2_D1_zt189 = (gX2_Cpg * _X1_X2_D1_zt188);
    const auto _X1_X2_D1_zt195 = (_X1_X2_D1_zt325 + gCc12);
    const auto _X1_X2_D1_zt219 = (gX2_Cpg * _X1_X2_D1_zt116);
    const auto _X1_X2_D1_zt10 = (_X1_X2_D1_zt42 * _X1_X2_D1_zt116);
    const auto _X1_X2_D1_zt30 = (_X1_X2_D1_zt219 * 1.0);
    const auto _X1_X2_D1_zt37 = (_X1_X2_D1_zt10 - _X1_X2_D1_zt60);
    const auto _X1_X2_D1_zt39 = (gRatt * _X1_X2_D1_zt219);
    const auto _X1_X2_D1_zt41 = (gX2_Cpg * _X1_X2_D1_zt39);
    const auto _X1_X2_D1_zt59 = (gX2_Cpg * _X1_X2_D1_zt30);
    const auto _X1_X2_D1_zt63 = (gCc12 * _X1_X2_D1_zt39);
    const auto _X1_X2_D1_zt66 = (gX1_Cpg * _X1_X2_D1_zt63);
    const auto _X1_X2_D1_zt69 = (_X1_X2_D1_zt116 * 1.0);
    const auto _X1_X2_D1_zt86 = (_X1_X2_D1_zt88 * 1.0);
    const auto _X1_X2_D1_zt92 = (_X1_X2_D1_zt39 * 1.0);
    const auto _X1_X2_D1_zt107 = (gRatt * _X1_X2_D1_zt139);
    const auto _X1_X2_D1_zt108 = (_X1_X2_D1_zt38 * _X1_X2_D1_zt107);
    const auto _X1_X2_D1_zt113 = (gRatt * _X1_X2_D1_zt37);
    const auto _X1_X2_D1_zt124 = (gX2_Cpg * _X1_X2_D1_zt86);
    const auto _X1_X2_D1_zt132 = (_X1_X2_D1_zt42 * _X1_X2_D1_zt86);
    const auto _X1_X2_D1_zt137 = (gX2_Cpg * _X1_X2_D1_zt107);
    const auto _X1_X2_D1_zt138 = (gX1_Cpg * _X1_X2_D1_zt137);
    const auto _X1_X2_D1_zt142 = (_X1_X2_D1_zt143 * 1.0);
    const auto _X1_X2_D1_zt156 = ((_X1_X2_D1_zt145 * ((gRatt * _X1_X2_D1_zt157) - (_X1_X2_D1_zt192 * _X1_X2_D1_zt157))) + (gRfb * (gRfb * _X1_X2_D1_zt157)));
    const auto _X1_X2_D1_zt160 = (-_X1_X2_D1_zt161);
    const auto _X1_X2_D1_zt166 = (gRatt * _X1_X2_D1_zt160);
    const auto _X1_X2_D1_zt168 = (_X1_X2_D1_zt38 * _X1_X2_D1_zt166);
    const auto _X1_X2_D1_zt169 = (-_X1_X2_D1_zt170);
    const auto _X1_X2_D1_zt172 = (_X1_X2_D1_zt195 * _X1_X2_D1_zt169);
    const auto _X1_X2_D1_zt173 = (gX1_Cpg * _X1_X2_D1_zt169);
    const auto _X1_X2_D1_zt178 = (_X1_X2_D1_zt42 * _X1_X2_D1_zt188);
    const auto _X1_X2_D1_zt184 = (gCout * _X1_X2_D1_zt187);
    const auto _X1_X2_D1_zt185 = (gRatt * _X1_X2_D1_zt189);
    const auto _X1_X2_D1_zt199 = (_X1_X2_D1_zt37 * 1.0);
    const auto _X1_X2_D1_zt223 = (_X1_X2_D1_zt38 * _X1_X2_D1_zt113);
    const auto _X1_X2_D1_zt16 = (_X1_X2_D1_zt223 - _X1_X2_D1_zt41);
    const auto _X1_X2_D1_zt21 = (gCc12 * _X1_X2_D1_zt137);
    const auto _X1_X2_D1_zt32 = (gCc12 * _X1_X2_D1_zt199);
    const auto _X1_X2_D1_zt36 = (gCc12 * _X1_X2_D1_zt113);
    const auto _X1_X2_D1_zt44 = (gRatt * _X1_X2_D1_zt69);
    const auto _X1_X2_D1_zt46 = (gX2_Cpg * _X1_X2_D1_zt44);
    const auto _X1_X2_D1_zt50 = (_X1_X2_D1_zt113 * 1.0);
    const auto _X1_X2_D1_zt52 = (gRatt * _X1_X2_D1_zt21);
    const auto _X1_X2_D1_zt53 = (-_X1_X2_D1_zt21);
    const auto _X1_X2_D1_zt62 = (_X1_X2_D1_zt63 * 1.0);
    const auto _X1_X2_D1_zt65 = ((_X1_X2_D1_zt145 * ((_X1_X2_D1_zt192 * _X1_X2_D1_zt66) - (gRatt * _X1_X2_D1_zt66))) - (gRfb * (gRfb * _X1_X2_D1_zt66)));
    const auto _X1_X2_D1_zt68 = (gCc12 * _X1_X2_D1_zt44);
    const auto _X1_X2_D1_zt71 = (gCc12 * _X1_X2_D1_zt68);
    const auto _X1_X2_D1_zt72 = (_X1_X2_D1_zt38 * _X1_X2_D1_zt44);
    const auto _X1_X2_D1_zt73 = (gX1_Cpg * _X1_X2_D1_zt72);
    const auto _X1_X2_D1_zt85 = (gRatt * _X1_X2_D1_zt124);
    const auto _X1_X2_D1_zt90 = (_X1_X2_D1_zt195 * _X1_X2_D1_zt92);
    const auto _X1_X2_D1_zt91 = (gX1_Cpg * _X1_X2_D1_zt92);
    const auto _X1_X2_D1_zt100 = (gCc12 * _X1_X2_D1_zt107);
    const auto _X1_X2_D1_zt103 = (_X1_X2_D1_zt195 * _X1_X2_D1_zt108);
    const auto _X1_X2_D1_zt104 = (gCc12 * _X1_X2_D1_zt100);
    const auto _X1_X2_D1_zt105 = (_X1_X2_D1_zt103 - _X1_X2_D1_zt104);
    const auto _X1_X2_D1_zt106 = (gX1_Cpg * _X1_X2_D1_zt108);
    const auto _X1_X2_D1_zt109 = (_X1_X2_D1_zt36 * 1.0);
    const auto _X1_X2_D1_zt114 = (gX1_Cpg * _X1_X2_D1_zt36);
    const auto _X1_X2_D1_zt115 = (_X1_X2_D1_zt195 * _X1_X2_D1_zt46);
    const auto _X1_X2_D1_zt117 = (gX1_Cpg * _X1_X2_D1_zt46);
    const auto _X1_X2_D1_zt118 = (_X1_X2_D1_zt132 - _X1_X2_D1_zt81);
    const auto _X1_X2_D1_zt128 = (_X1_X2_D1_zt195 * _X1_X2_D1_zt50);
    const auto _X1_X2_D1_zt129 = (gX1_Cpg * _X1_X2_D1_zt50);
    const auto _X1_X2_D1_zt136 = (_X1_X2_D1_zt195 * _X1_X2_D1_zt137);
    const auto _X1_X2_D1_zt155 = (gCc12 * _X1_X2_D1_zt36);
    const auto _X1_X2_D1_zt163 = (gCc12 * _X1_X2_D1_zt166);
    const auto _X1_X2_D1_zt165 = (_X1_X2_D1_zt195 * _X1_X2_D1_zt168);
    const auto _X1_X2_D1_zt167 = (gX1_Cpg * _X1_X2_D1_zt168);
    const auto _X1_X2_D1_zt171 = (-_X1_X2_D1_zt172);
    const auto _X1_X2_D1_zt175 = (gX2_Cpg * _X1_X2_D1_zt185);
    const auto _X1_X2_D1_zt183 = (_X1_X2_D1_zt184 - _X1_X2_D1_zt178);
    const auto _X1_X2_D1_zt194 = (_X1_X2_D1_zt38 * _X1_X2_D1_zt199);
    const auto _X1_X2_D1_zt198 = (_X1_X2_D1_zt194 - _X1_X2_D1_zt59);
    const auto _X1_X2_D1_zt231 = (gX1_Cpg * _X1_X2_D1_zt16);
    const auto _X1_X2_D1_zt234 = (_X1_X2_D1_zt195 * _X1_X2_D1_zt16);
    const auto _X1_X2_D1_zt1 = (_X1_X2_D1_zt16 * 1.0);
    const auto _X1_X2_D1_zt15 = (gRatt * _X1_X2_D1_zt231);
    const auto _X1_X2_D1_zt20 = (gCc12 * _X1_X2_D1_zt46);
    const auto _X1_X2_D1_zt29 = (_X1_X2_D1_zt59 - _X1_X2_D1_zt194);
    const auto _X1_X2_D1_zt31 = (gCc12 * _X1_X2_D1_zt32);
    const auto _X1_X2_D1_zt33 = (-_X1_X2_D1_zt1);
    const auto _X1_X2_D1_zt40 = (gRfb * _X1_X2_D1_zt231);
    const auto _X1_X2_D1_zt45 = (gRatt * _X1_X2_D1_zt20);
    const auto _X1_X2_D1_zt47 = (-_X1_X2_D1_zt20);
    const auto _X1_X2_D1_zt49 = (gCc12 * _X1_X2_D1_zt50);
    const auto _X1_X2_D1_zt51 = (-_X1_X2_D1_zt49);
    const auto _X1_X2_D1_zt67 = (_X1_X2_D1_zt195 * _X1_X2_D1_zt72);
    const auto _X1_X2_D1_zt70 = (_X1_X2_D1_zt67 - _X1_X2_D1_zt71);
    const auto _X1_X2_D1_zt80 = (gRatt * _X1_X2_D1_zt118);
    const auto _X1_X2_D1_zt82 = (gX2_Cpg * _X1_X2_D1_zt85);
    const auto _X1_X2_D1_zt96 = (gCc12 * _X1_X2_D1_zt80);
    const auto _X1_X2_D1_zt99 = (_X1_X2_D1_zt38 * _X1_X2_D1_zt80);
    const auto _X1_X2_D1_zt111 = (_X1_X2_D1_zt192 * _X1_X2_D1_zt231);
    const auto _X1_X2_D1_zt112 = ((_X1_X2_D1_zt145 * ((_X1_X2_D1_zt192 * _X1_X2_D1_zt114) - (gRatt * _X1_X2_D1_zt114))) - (gRfb * (gRfb * _X1_X2_D1_zt114)));
    const auto _X1_X2_D1_zt123 = (_X1_X2_D1_zt99 - _X1_X2_D1_zt82);
    const auto _X1_X2_D1_zt133 = (gCc12 * _X1_X2_D1_zt96);
    const auto _X1_X2_D1_zt154 = (_X1_X2_D1_zt155 - _X1_X2_D1_zt234);
    const auto _X1_X2_D1_zt158 = (_X1_X2_D1_zt40 * 1.0);
    const auto _X1_X2_D1_zt162 = (gCc12 * _X1_X2_D1_zt163);
    const auto _X1_X2_D1_zt164 = (_X1_X2_D1_zt162 - _X1_X2_D1_zt165);
    const auto _X1_X2_D1_zt182 = (gRatt * _X1_X2_D1_zt183);
    const auto _X1_X2_D1_zt193 = (gX1_Cpg * _X1_X2_D1_zt198);
    const auto _X1_X2_D1_zt197 = (_X1_X2_D1_zt195 * _X1_X2_D1_zt198);
    const auto _X1_X2_D1_zt0 = (gRatt * _X1_X2_D1_zt1);
    const auto _X1_X2_D1_zt3 = (gRatt * _X1_X2_D1_zt154);
    const auto _X1_X2_D1_zt5 = (gRfb * _X1_X2_D1_zt40);
    const auto _X1_X2_D1_zt9 = (_X1_X2_D1_zt192 * _X1_X2_D1_zt154);
    const auto _X1_X2_D1_zt12 = (_X1_X2_D1_zt3 - _X1_X2_D1_zt9);
    const auto _X1_X2_D1_zt14 = (_X1_X2_D1_zt15 - _X1_X2_D1_zt111);
    const auto _X1_X2_D1_zt17 = (_X1_X2_D1_zt145 * _X1_X2_D1_zt14);
    const auto _X1_X2_D1_zt25 = (gX1_Cpg * _X1_X2_D1_zt29);
    const auto _X1_X2_D1_zt28 = (_X1_X2_D1_zt195 * _X1_X2_D1_zt29);
    const auto _X1_X2_D1_zt34 = (gRfb * _X1_X2_D1_zt154);
    const auto _X1_X2_D1_zt43 = (_X1_X2_D1_zt12 * 1.0);
    const auto _X1_X2_D1_zt48 = (gRatt * _X1_X2_D1_zt49);
    const auto _X1_X2_D1_zt77 = (_X1_X2_D1_zt195 * _X1_X2_D1_zt123);
    const auto _X1_X2_D1_zt79 = (_X1_X2_D1_zt133 - _X1_X2_D1_zt77);
    const auto _X1_X2_D1_zt95 = (gRatt * _X1_X2_D1_zt79);
    const auto _X1_X2_D1_zt97 = (_X1_X2_D1_zt192 * _X1_X2_D1_zt79);
    const auto _X1_X2_D1_zt98 = (gX1_Cpg * _X1_X2_D1_zt123);
    const auto _X1_X2_D1_zt110 = (_X1_X2_D1_zt145 * _X1_X2_D1_zt12);
    const auto _X1_X2_D1_zt120 = (gRfb * _X1_X2_D1_zt79);
    const auto _X1_X2_D1_zt122 = (gRatt * _X1_X2_D1_zt98);
    const auto _X1_X2_D1_zt126 = (gRfb * _X1_X2_D1_zt98);
    const auto _X1_X2_D1_zt131 = (_X1_X2_D1_zt95 - _X1_X2_D1_zt97);
    const auto _X1_X2_D1_zt147 = (gRfb * _X1_X2_D1_zt34);
    const auto _X1_X2_D1_zt148 = (_X1_X2_D1_zt17 + _X1_X2_D1_zt5);
    const auto _X1_X2_D1_zt152 = (-_X1_X2_D1_zt158);
    const auto _X1_X2_D1_zt153 = (_X1_X2_D1_zt34 * 1.0);
    const auto _X1_X2_D1_zt159 = (_X1_X2_D1_zt64 * _X1_X2_D1_zt153);
    const auto _X1_X2_D1_zt177 = (gCc12 * _X1_X2_D1_zt182);
    const auto _X1_X2_D1_zt180 = (_X1_X2_D1_zt38 * _X1_X2_D1_zt182);
    const auto _X1_X2_D1_zt191 = (_X1_X2_D1_zt180 + _X1_X2_D1_zt175);
    const auto _X1_X2_D1_zt196 = (_X1_X2_D1_zt31 - _X1_X2_D1_zt197);
    const auto _X1_X2_D1_zt7 = (_X1_X2_D1_zt14 * 1.0);
    const auto _X1_X2_D1_zt13 = (gX1_Cpg * _X1_X2_D1_zt148);
    const auto _X1_X2_D1_zt19 = (-_X1_X2_D1_zt43);
    const auto _X1_X2_D1_zt24 = (gRatt * _X1_X2_D1_zt25);
    const auto _X1_X2_D1_zt27 = (_X1_X2_D1_zt28 + _X1_X2_D1_zt31);
    const auto _X1_X2_D1_zt35 = (gX1_Cpg * _X1_X2_D1_zt7);
    const auto _X1_X2_D1_zt55 = (gRfb * _X1_X2_D1_zt24);
    const auto _X1_X2_D1_zt75 = (_X1_X2_D1_zt145 * _X1_X2_D1_zt131);
    const auto _X1_X2_D1_zt87 = (_X1_X2_D1_zt192 * _X1_X2_D1_zt98);
    const auto _X1_X2_D1_zt119 = (gRfb * _X1_X2_D1_zt120);
    const auto _X1_X2_D1_zt121 = (_X1_X2_D1_zt122 - _X1_X2_D1_zt87);
    const auto _X1_X2_D1_zt125 = (gRfb * _X1_X2_D1_zt126);
    const auto _X1_X2_D1_zt135 = (_X1_X2_D1_zt145 * _X1_X2_D1_zt121);
    const auto _X1_X2_D1_zt144 = (_X1_X2_D1_zt110 + _X1_X2_D1_zt147);
    const auto _X1_X2_D1_zt151 = (gX1_Cpg * _X1_X2_D1_zt152);
    const auto _X1_X2_D1_zt174 = (_X1_X2_D1_zt195 * _X1_X2_D1_zt191);
    const auto _X1_X2_D1_zt176 = (gCc12 * _X1_X2_D1_zt177);
    const auto _X1_X2_D1_zt181 = (_X1_X2_D1_zt176 - _X1_X2_D1_zt174);
    const auto _X1_X2_D1_zt186 = (gX1_Cpg * _X1_X2_D1_zt191);
    const auto _X1_X2_D1_zt8 = (_X1_X2_D1_zt64 * _X1_X2_D1_zt19);
    const auto _X1_X2_D1_zt11 = (_X1_X2_D1_zt64 * _X1_X2_D1_zt144);
    const auto _X1_X2_D1_zt23 = (gX1_Cpg * _X1_X2_D1_zt55);
    const auto _X1_X2_D1_zt26 = (gRatt * _X1_X2_D1_zt27);
    const auto _X1_X2_D1_zt58 = (gRfb * _X1_X2_D1_zt26);
    const auto _X1_X2_D1_zt94 = (_X1_X2_D1_zt75 + _X1_X2_D1_zt119);
    const auto _X1_X2_D1_zt130 = (_X1_X2_D1_zt64 * _X1_X2_D1_zt94);
    const auto _X1_X2_D1_zt134 = (_X1_X2_D1_zt135 + _X1_X2_D1_zt125);
    const auto _X1_X2_D1_zt150 = (_X1_X2_D1_zt151 - _X1_X2_D1_zt159);
    const auto _X1_X2_D1_zt2 = (_X1_X2_D1_zt11 + _X1_X2_D1_zt13);
    const auto _X1_X2_D1_zt6 = (_X1_X2_D1_zt35 - _X1_X2_D1_zt8);
    const auto _X1_X2_D1_zt18 = (_X1_X2_D1_zt6 / _X1_X2_D1_zt2);
    const auto _X1_X2_D1_zt57 = (_X1_X2_D1_zt64 * _X1_X2_D1_zt58);
    const auto _X1_X2_D1_zt84 = (gX1_Cpg * _X1_X2_D1_zt134);
    const auto _X1_X2_D1_zt93 = (_X1_X2_D1_zt130 + _X1_X2_D1_zt84);
    const auto _X1_X2_D1_zt149 = (_X1_X2_D1_zt150 / _X1_X2_D1_zt2);
    const auto _X1_X2_D1_zt22 = (_X1_X2_D1_zt23 - _X1_X2_D1_zt57);
    const auto _X1_X2_D1_zt54 = (_X1_X2_D1_zt22 / _X1_X2_D1_zt2);
    const auto _X1_X2_D1_zt74 = (_X1_X2_D1_zt93 / _X1_X2_D1_zt2);
    const auto _X1_X2_D1_Z0_0 = (((_X1_X2_D1_zt64 * ((_X1_X2_D1_zt145 * (_X1_X2_D1_zt0 - (_X1_X2_D1_zt192 * _X1_X2_D1_zt1))) + (gRfb * (gRfb * _X1_X2_D1_zt1)))) / _X1_X2_D1_zt2) - _X1_X2_D1_zt18);
    const auto _X1_X2_D1_Z0_1 = (((_X1_X2_D1_zt148 * 1.0) / _X1_X2_D1_zt2) - _X1_X2_D1_zt18);
    const auto _X1_X2_D1_Z0_2 = ((_X1_X2_D1_zt64 * ((_X1_X2_D1_zt145 * (_X1_X2_D1_zt45 - (_X1_X2_D1_zt192 * _X1_X2_D1_zt20))) + (gRfb * (gRfb * _X1_X2_D1_zt20)))) / _X1_X2_D1_zt2);
    const auto _X1_X2_D1_Z0_3 = ((_X1_X2_D1_zt64 * ((_X1_X2_D1_zt145 * (_X1_X2_D1_zt48 - (_X1_X2_D1_zt192 * _X1_X2_D1_zt49))) + (gRfb * (gRfb * _X1_X2_D1_zt49)))) / _X1_X2_D1_zt2);
    const auto _X1_X2_D1_Z0_4 = (((_X1_X2_D1_zt64 * ((_X1_X2_D1_zt145 * (_X1_X2_D1_zt52 - (_X1_X2_D1_zt192 * _X1_X2_D1_zt21))) + (gRfb * (gRfb * _X1_X2_D1_zt21)))) / _X1_X2_D1_zt2) - _X1_X2_D1_zt54);
    const auto _X1_X2_D1_Z1_0 = (((gX1_Cpg * ((_X1_X2_D1_zt145 * ((_X1_X2_D1_zt192 * _X1_X2_D1_zt33) + _X1_X2_D1_zt0)) - (gRfb * (gRfb * _X1_X2_D1_zt33)))) / _X1_X2_D1_zt2) - _X1_X2_D1_zt18);
    const auto _X1_X2_D1_Z1_1 = (((-(_X1_X2_D1_zt144 * 1.0)) / _X1_X2_D1_zt2) - _X1_X2_D1_zt18);
    const auto _X1_X2_D1_Z1_2 = ((gX1_Cpg * ((_X1_X2_D1_zt145 * ((_X1_X2_D1_zt192 * _X1_X2_D1_zt47) + _X1_X2_D1_zt45)) - (gRfb * (gRfb * _X1_X2_D1_zt47)))) / _X1_X2_D1_zt2);
    const auto _X1_X2_D1_Z1_3 = ((gX1_Cpg * ((_X1_X2_D1_zt145 * ((_X1_X2_D1_zt192 * _X1_X2_D1_zt51) + _X1_X2_D1_zt48)) - (gRfb * (gRfb * _X1_X2_D1_zt51)))) / _X1_X2_D1_zt2);
    const auto _X1_X2_D1_Z1_4 = (((gX1_Cpg * ((_X1_X2_D1_zt145 * ((_X1_X2_D1_zt192 * _X1_X2_D1_zt53) + _X1_X2_D1_zt52)) - (gRfb * (gRfb * _X1_X2_D1_zt53)))) / _X1_X2_D1_zt2) - _X1_X2_D1_zt54);
    const auto _X1_X2_D1_Z2_0 = ((_X1_X2_D1_zt64 * ((_X1_X2_D1_zt145 * ((gRatt * _X1_X2_D1_zt62) - (_X1_X2_D1_zt192 * _X1_X2_D1_zt62))) + (gRfb * (gRfb * _X1_X2_D1_zt62)))) / _X1_X2_D1_zt2);
    const auto _X1_X2_D1_Z2_1 = ((-(_X1_X2_D1_zt65 * 1.0)) / _X1_X2_D1_zt2);
    const auto _X1_X2_D1_Z2_2 = ((((_X1_X2_D1_zt64 * ((_X1_X2_D1_zt145 * ((gRatt * (-(_X1_X2_D1_zt71 - _X1_X2_D1_zt67))) - (_X1_X2_D1_zt192 * _X1_X2_D1_zt70))) + (gRfb * (gRfb * _X1_X2_D1_zt70)))) + (gX1_Cpg * ((_X1_X2_D1_zt145 * ((_X1_X2_D1_zt192 * _X1_X2_D1_zt73) + (gRatt * (-_X1_X2_D1_zt73)))) - (gRfb * (gRfb * _X1_X2_D1_zt73))))) / _X1_X2_D1_zt2) - _X1_X2_D1_zt74);
    const auto _X1_X2_D1_Z2_3 = ((((_X1_X2_D1_zt64 * ((_X1_X2_D1_zt145 * ((gRatt * (-(-_X1_X2_D1_zt90))) - (_X1_X2_D1_zt192 * _X1_X2_D1_zt90))) + (gRfb * (gRfb * _X1_X2_D1_zt90)))) + (gX1_Cpg * ((_X1_X2_D1_zt145 * ((_X1_X2_D1_zt192 * _X1_X2_D1_zt91) + (gRatt * (-_X1_X2_D1_zt91)))) - (gRfb * (gRfb * _X1_X2_D1_zt91))))) / _X1_X2_D1_zt2) - _X1_X2_D1_zt74);
    const auto _X1_X2_D1_Z2_4 = (((_X1_X2_D1_zt64 * ((_X1_X2_D1_zt145 * ((gRatt * (-(_X1_X2_D1_zt104 - _X1_X2_D1_zt103))) - (_X1_X2_D1_zt192 * _X1_X2_D1_zt105))) + (gRfb * (gRfb * _X1_X2_D1_zt105)))) + (gX1_Cpg * ((_X1_X2_D1_zt145 * ((_X1_X2_D1_zt192 * _X1_X2_D1_zt106) + (gRatt * (-_X1_X2_D1_zt106)))) - (gRfb * (gRfb * _X1_X2_D1_zt106))))) / _X1_X2_D1_zt2);
    const auto _X1_X2_D1_Z3_0 = ((_X1_X2_D1_zt64 * ((_X1_X2_D1_zt145 * ((gRatt * _X1_X2_D1_zt109) - (_X1_X2_D1_zt192 * _X1_X2_D1_zt109))) + (gRfb * (gRfb * _X1_X2_D1_zt109)))) / _X1_X2_D1_zt2);
    const auto _X1_X2_D1_Z3_1 = ((-(_X1_X2_D1_zt112 * 1.0)) / _X1_X2_D1_zt2);
    const auto _X1_X2_D1_Z3_2 = ((((_X1_X2_D1_zt64 * ((_X1_X2_D1_zt145 * ((gRatt * _X1_X2_D1_zt115) - (_X1_X2_D1_zt192 * _X1_X2_D1_zt115))) + (gRfb * (gRfb * _X1_X2_D1_zt115)))) + (gX1_Cpg * ((_X1_X2_D1_zt145 * ((_X1_X2_D1_zt192 * _X1_X2_D1_zt117) + (gRatt * (-_X1_X2_D1_zt117)))) - (gRfb * (gRfb * _X1_X2_D1_zt117))))) / _X1_X2_D1_zt2) - _X1_X2_D1_zt74);
    const auto _X1_X2_D1_Z3_3 = ((((_X1_X2_D1_zt64 * ((_X1_X2_D1_zt145 * ((gRatt * _X1_X2_D1_zt128) - (_X1_X2_D1_zt192 * _X1_X2_D1_zt128))) + (gRfb * (gRfb * _X1_X2_D1_zt128)))) + (gX1_Cpg * ((_X1_X2_D1_zt145 * ((_X1_X2_D1_zt192 * _X1_X2_D1_zt129) + (gRatt * (-_X1_X2_D1_zt129)))) - (gRfb * (gRfb * _X1_X2_D1_zt129))))) / _X1_X2_D1_zt2) - _X1_X2_D1_zt74);
    const auto _X1_X2_D1_Z3_4 = (((_X1_X2_D1_zt64 * ((_X1_X2_D1_zt145 * ((gRatt * _X1_X2_D1_zt136) - (_X1_X2_D1_zt192 * _X1_X2_D1_zt136))) + (gRfb * (gRfb * _X1_X2_D1_zt136)))) + (gX1_Cpg * ((_X1_X2_D1_zt145 * ((_X1_X2_D1_zt192 * _X1_X2_D1_zt138) + (gRatt * (-_X1_X2_D1_zt138)))) - (gRfb * (gRfb * _X1_X2_D1_zt138))))) / _X1_X2_D1_zt2);
    const auto _X1_X2_D1_Z4_0 = (((_X1_X2_D1_zt64 * ((_X1_X2_D1_zt145 * ((gRatt * _X1_X2_D1_zt142) - (_X1_X2_D1_zt192 * _X1_X2_D1_zt142))) + (gRfb * (gRfb * _X1_X2_D1_zt142)))) / _X1_X2_D1_zt2) - _X1_X2_D1_zt149);
    const auto _X1_X2_D1_Z4_1 = (((_X1_X2_D1_zt156 * 1.0) / _X1_X2_D1_zt2) - _X1_X2_D1_zt149);
    const auto _X1_X2_D1_Z4_2 = (((_X1_X2_D1_zt64 * ((_X1_X2_D1_zt145 * ((gRatt * (-(_X1_X2_D1_zt165 - _X1_X2_D1_zt162))) - (_X1_X2_D1_zt192 * _X1_X2_D1_zt164))) + (gRfb * (gRfb * _X1_X2_D1_zt164)))) + (gX1_Cpg * ((_X1_X2_D1_zt145 * ((gRatt * _X1_X2_D1_zt167) - (_X1_X2_D1_zt192 * _X1_X2_D1_zt167))) + (gRfb * (gRfb * _X1_X2_D1_zt167))))) / _X1_X2_D1_zt2);
    const auto _X1_X2_D1_Z4_3 = (((_X1_X2_D1_zt64 * ((_X1_X2_D1_zt145 * ((gRatt * _X1_X2_D1_zt171) - (_X1_X2_D1_zt192 * _X1_X2_D1_zt171))) + (gRfb * (gRfb * _X1_X2_D1_zt171)))) + (gX1_Cpg * ((_X1_X2_D1_zt145 * ((gRatt * _X1_X2_D1_zt173) - (_X1_X2_D1_zt192 * _X1_X2_D1_zt173))) + (gRfb * (gRfb * _X1_X2_D1_zt173))))) / _X1_X2_D1_zt2);
    const auto _X1_X2_D1_Z4_4 = ((((_X1_X2_D1_zt64 * ((_X1_X2_D1_zt145 * ((gRatt * (-(_X1_X2_D1_zt174 - _X1_X2_D1_zt176))) - (_X1_X2_D1_zt192 * _X1_X2_D1_zt181))) + (gRfb * (gRfb * _X1_X2_D1_zt181)))) + (gX1_Cpg * ((_X1_X2_D1_zt145 * ((gRatt * _X1_X2_D1_zt186) - (_X1_X2_D1_zt192 * _X1_X2_D1_zt186))) + (gRfb * (gRfb * _X1_X2_D1_zt186))))) / _X1_X2_D1_zt2) - (((gX1_Cpg * ((gRfb * (gRfb * _X1_X2_D1_zt193)) - (_X1_X2_D1_zt145 * (_X1_X2_D1_zt192 * _X1_X2_D1_zt193)))) - (_X1_X2_D1_zt64 * ((_X1_X2_D1_zt145 * (_X1_X2_D1_zt192 * _X1_X2_D1_zt196)) - (gRfb * (gRfb * _X1_X2_D1_zt196))))) / _X1_X2_D1_zt2));
    double c0__X1_X2_D1_voc2;
    double c__X1_X2_D1_voc2[10];
    double c0__X1_X2_D1_voc3;
    double c__X1_X2_D1_voc3[10];
    double c0__X1_X2_D1_voc4;
    double c__X1_X2_D1_voc4[10];
    double c0__X1_X2_D1_voc0;
    double c__X1_X2_D1_voc0[10];
    double c0__X1_X2_D1_voc1;
    double c__X1_X2_D1_voc1[10];
    double c0__X1_X2_D1_zt266;
    double c__X1_X2_D1_zt266[10];
    double c0__X1_X2_D1_zt329;
    double c__X1_X2_D1_zt329[10];
    double c0__X1_X2_D1_zt301;
    double c__X1_X2_D1_zt301[10];
    double c0__X1_X2_D1_zt334;
    double c__X1_X2_D1_zt334[10];
    double c0__X1_X2_D1_zt267;
    double c__X1_X2_D1_zt267[10];
    double c0__X1_X2_D1_zt200;
    double c__X1_X2_D1_zt200[10];
    double c0__X1_X2_D1_zt335;
    double c__X1_X2_D1_zt335[10];
    
    for (int _k = 0; _k <= 10; ++_k)
    {
        const auto vi = (_k == 1) ? 1.0 : 0.0;
        const auto zCin = (_k == 2) ? 1.0 : 0.0;
        const auto zRk1Ck1 = (_k == 3) ? 1.0 : 0.0;
        const auto zX1_Cpg = (_k == 4) ? 1.0 : 0.0;
        const auto zCc12 = (_k == 5) ? 1.0 : 0.0;
        const auto zRrelCenv = (_k == 6) ? 1.0 : 0.0;
        const auto zX2_Cpg = (_k == 7) ? 1.0 : 0.0;
        const auto zCout = (_k == 8) ? 1.0 : 0.0;
        const auto zCsc = (_k == 9) ? 1.0 : 0.0;
        const auto zRk2Ck2 = (_k == 10) ? 1.0 : 0.0;
        const auto _X1_X2_D1_zt236 = (zX1_Cpg + zCc12);
        const auto _X1_X2_D1_zt245 = (zX2_Cpg + zCout);
        const auto _X1_X2_D1_zt263 = (zCc12 + zX2_Cpg);
        const auto _X1_X2_D1_zt271 = (_X1_X2_D1_zt78 * zRk2Ck2);
        const auto _X1_X2_D1_zt288 = (-_X1_X2_D1_zt245);
        const auto _X1_X2_D1_zt293 = (zCout - zCsc);
        const auto _X1_X2_D1_zt294 = (gCsc * zCsc);
        const auto _X1_X2_D1_zt333 = (_X1_X2_D1_zt260 * zCsc);
        const auto _X1_X2_D1_zt348 = (gCout * zCsc);
        const auto _X1_X2_D1_zt359 = (gCin * vi);
        const auto _X1_X2_D1_zt202 = (_X1_X2_D1_zt116 * _X1_X2_D1_zt288);
        const auto _X1_X2_D1_zt211 = (_X1_X2_D1_zt359 - zCin);
        const auto _X1_X2_D1_zt228 = (-_X1_X2_D1_zt236);
        const auto _X1_X2_D1_zt256 = (gRp2 * _X1_X2_D1_zt116);
        const auto _X1_X2_D1_zt270 = (gCout * _X1_X2_D1_zt271);
        const auto _X1_X2_D1_zt292 = (_X1_X2_D1_zt293 * gCsc);
        const auto _X1_X2_D1_zt310 = (_X1_X2_D1_zt88 * zRk2Ck2);
        const auto _X1_X2_D1_zt321 = (_X1_X2_D1_zt333 + _X1_X2_D1_zt292);
        const auto _X1_X2_D1_zt339 = (gRk2Ck2 * _X1_X2_D1_zt348);
        const auto _X1_X2_D1_zt341 = (_X1_X2_D1_zt143 * _X1_X2_D1_zt228);
        const auto _X1_X2_D1_zt343 = (gRp1 * _X1_X2_D1_zt143);
        const auto _X1_X2_D1_zt345 = (gRp2 * _X1_X2_D1_zt4);
        const auto _X1_X2_D1_zt349 = (_X1_X2_D1_zt4 * _X1_X2_D1_zt288);
        const auto _X1_X2_D1_zt351 = (_X1_X2_D1_zt141 * _X1_X2_D1_zt263);
        const auto _X1_X2_D1_zt356 = (gRk2Ck2 * _X1_X2_D1_zt321);
        const auto _X1_X2_D1_zt357 = (gX2_Cpg * _X1_X2_D1_zt356);
        const auto _X1_X2_D1_zt204 = (gRatt * _X1_X2_D1_zt256);
        const auto _X1_X2_D1_zt207 = (_X1_X2_D1_zt113 * _X1_X2_D1_zt263);
        const auto _X1_X2_D1_zt210 = (_X1_X2_D1_zt211 - zX1_Cpg);
        const auto _X1_X2_D1_zt226 = (gX2_Cpg * _X1_X2_D1_zt204);
        const auto _X1_X2_D1_zt227 = (gCc12 * _X1_X2_D1_zt226);
        const auto _X1_X2_D1_zt235 = (_X1_X2_D1_zt63 * _X1_X2_D1_zt228);
        const auto _X1_X2_D1_zt244 = (_X1_X2_D1_zt292 + _X1_X2_D1_zt294);
        const auto _X1_X2_D1_zt247 = (gCc12 * _X1_X2_D1_zt204);
        const auto _X1_X2_D1_zt248 = (gRp1 * _X1_X2_D1_zt63);
        const auto _X1_X2_D1_zt258 = (_X1_X2_D1_zt39 * _X1_X2_D1_zt263);
        const auto _X1_X2_D1_zt261 = (_X1_X2_D1_zt38 * _X1_X2_D1_zt204);
        const auto _X1_X2_D1_zt284 = (_X1_X2_D1_zt195 * _X1_X2_D1_zt226);
        const auto _X1_X2_D1_zt291 = (gRk2Ck2 * _X1_X2_D1_zt244);
        const auto _X1_X2_D1_zt300 = (gX1_Cpg * _X1_X2_D1_zt226);
        const auto _X1_X2_D1_zt307 = (_X1_X2_D1_zt42 * _X1_X2_D1_zt310);
        const auto _X1_X2_D1_zt308 = (gX2_Cpg * _X1_X2_D1_zt310);
        const auto _X1_X2_D1_zt328 = (gRatt * _X1_X2_D1_zt345);
        const auto _X1_X2_D1_zt332 = (gCout * _X1_X2_D1_zt339);
        const auto _X1_X2_D1_zt340 = (_X1_X2_D1_zt42 * _X1_X2_D1_zt356);
        const auto _X1_X2_D1_zt350 = (gRatt * _X1_X2_D1_zt357);
        const auto _X1_X2_D1_zt352 = (_X1_X2_D1_zt38 * _X1_X2_D1_zt328);
        const auto _X1_X2_D1_zt354 = (gX1_Cpg * _X1_X2_D1_zt352);
        const auto _X1_X2_D1_zt358 = (VCC * _X1_X2_D1_zt354);
        const auto _X1_X2_D1_zt201 = (_X1_X2_D1_zt16 * _X1_X2_D1_zt228);
        const auto _X1_X2_D1_zt203 = (gRp1 * _X1_X2_D1_zt16);
        const auto _X1_X2_D1_zt209 = (-_X1_X2_D1_zt210);
        const auto _X1_X2_D1_zt222 = (_X1_X2_D1_zt203 + _X1_X2_D1_zt227);
        const auto _X1_X2_D1_zt225 = (VCC * _X1_X2_D1_zt222);
        const auto _X1_X2_D1_zt238 = (gCc12 * _X1_X2_D1_zt247);
        const auto _X1_X2_D1_zt239 = (_X1_X2_D1_zt195 * _X1_X2_D1_zt261);
        const auto _X1_X2_D1_zt252 = (gCout * _X1_X2_D1_zt291);
        const auto _X1_X2_D1_zt255 = (_X1_X2_D1_zt248 + _X1_X2_D1_zt239);
        const auto _X1_X2_D1_zt259 = (gX1_Cpg * _X1_X2_D1_zt261);
        const auto _X1_X2_D1_zt265 = (VCC * _X1_X2_D1_zt259);
        const auto _X1_X2_D1_zt269 = (_X1_X2_D1_zt307 - _X1_X2_D1_zt270);
        const auto _X1_X2_D1_zt276 = (gRatt * _X1_X2_D1_zt308);
        const auto _X1_X2_D1_zt279 = (gX2_Cpg * _X1_X2_D1_zt276);
        const auto _X1_X2_D1_zt289 = (_X1_X2_D1_zt36 * _X1_X2_D1_zt228);
        const auto _X1_X2_D1_zt295 = (gRp1 * _X1_X2_D1_zt36);
        const auto _X1_X2_D1_zt299 = (VCC * _X1_X2_D1_zt300);
        const auto _X1_X2_D1_zt320 = (gX2_Cpg * _X1_X2_D1_zt350);
        const auto _X1_X2_D1_zt327 = (gCc12 * _X1_X2_D1_zt328);
        const auto _X1_X2_D1_zt331 = (_X1_X2_D1_zt332 - _X1_X2_D1_zt340);
        const auto _X1_X2_D1_zt344 = (_X1_X2_D1_zt195 * _X1_X2_D1_zt352);
        const auto _X1_X2_D1_zt355 = (_X1_X2_D1_zt331 - _X1_X2_D1_zt349);
        const auto _X1_X2_D1_zt360 = (zRrelCenv * _X1_X2_D1_zt231);
        const auto _X1_X2_D1_zt218 = (zRrelCenv * _X1_X2_D1_zt154);
        const auto _X1_X2_D1_zt230 = (-_X1_X2_D1_zt360);
        const auto _X1_X2_D1_zt233 = (gRfb * _X1_X2_D1_zt218);
        const auto _X1_X2_D1_zt243 = (_X1_X2_D1_zt202 + _X1_X2_D1_zt252);
        const auto _X1_X2_D1_zt254 = (_X1_X2_D1_zt238 - _X1_X2_D1_zt255);
        const auto _X1_X2_D1_zt268 = (gRatt * _X1_X2_D1_zt269);
        const auto _X1_X2_D1_zt274 = (gCc12 * _X1_X2_D1_zt268);
        const auto _X1_X2_D1_zt283 = (_X1_X2_D1_zt295 + _X1_X2_D1_zt284);
        const auto _X1_X2_D1_zt306 = (gCc12 * _X1_X2_D1_zt274);
        const auto _X1_X2_D1_zt317 = (_X1_X2_D1_zt38 * _X1_X2_D1_zt268);
        const auto _X1_X2_D1_zt326 = (gCc12 * _X1_X2_D1_zt327);
        const auto _X1_X2_D1_zt337 = (gRatt * _X1_X2_D1_zt355);
        const auto _X1_X2_D1_zt338 = (_X1_X2_D1_zt38 * _X1_X2_D1_zt337);
        const auto _X1_X2_D1_zt342 = (_X1_X2_D1_zt343 + _X1_X2_D1_zt344);
        const auto _X1_X2_D1_zt215 = (_X1_X2_D1_zt14 * zRk1Ck1);
        const auto _X1_X2_D1_zt216 = (gRfb * _X1_X2_D1_zt230);
        const auto _X1_X2_D1_zt220 = (_X1_X2_D1_zt12 * zRk1Ck1);
        const auto _X1_X2_D1_zt229 = (_X1_X2_D1_zt215 + _X1_X2_D1_zt216);
        const auto _X1_X2_D1_zt232 = (_X1_X2_D1_zt233 - _X1_X2_D1_zt220);
        const auto _X1_X2_D1_zt242 = (gRatt * _X1_X2_D1_zt243);
        const auto _X1_X2_D1_zt246 = (VCC * _X1_X2_D1_zt254);
        const auto _X1_X2_D1_zt262 = (_X1_X2_D1_zt38 * _X1_X2_D1_zt242);
        const auto _X1_X2_D1_zt282 = (VCC * _X1_X2_D1_zt283);
        const auto _X1_X2_D1_zt287 = (gX2_Cpg * _X1_X2_D1_zt242);
        const auto _X1_X2_D1_zt296 = (_X1_X2_D1_zt207 + _X1_X2_D1_zt287);
        const auto _X1_X2_D1_zt298 = (gX1_Cpg * _X1_X2_D1_zt296);
        const auto _X1_X2_D1_zt312 = (_X1_X2_D1_zt317 - _X1_X2_D1_zt279);
        const auto _X1_X2_D1_zt316 = (gX1_Cpg * _X1_X2_D1_zt312);
        const auto _X1_X2_D1_zt318 = (gRfb * _X1_X2_D1_zt316);
        const auto _X1_X2_D1_zt319 = (_X1_X2_D1_zt338 + _X1_X2_D1_zt320);
        const auto _X1_X2_D1_zt322 = (gCc12 * _X1_X2_D1_zt337);
        const auto _X1_X2_D1_zt324 = (_X1_X2_D1_zt342 - _X1_X2_D1_zt326);
        const auto _X1_X2_D1_zt336 = (gCc12 * _X1_X2_D1_zt322);
        const auto _X1_X2_D1_zt353 = (_X1_X2_D1_zt319 - _X1_X2_D1_zt351);
        const auto _X1_X2_D1_zt206 = (gCc12 * _X1_X2_D1_zt296);
        const auto _X1_X2_D1_zt214 = (gX1_Cpg * _X1_X2_D1_zt229);
        const auto _X1_X2_D1_zt217 = (_X1_X2_D1_zt64 * _X1_X2_D1_zt232);
        const auto _X1_X2_D1_zt237 = (gCc12 * _X1_X2_D1_zt242);
        const auto _X1_X2_D1_zt241 = (_X1_X2_D1_zt262 + _X1_X2_D1_zt258);
        const auto _X1_X2_D1_zt253 = (gCc12 * _X1_X2_D1_zt237);
        const auto _X1_X2_D1_zt264 = (gX1_Cpg * _X1_X2_D1_zt241);
        const auto _X1_X2_D1_zt275 = (_X1_X2_D1_zt195 * _X1_X2_D1_zt312);
        const auto _X1_X2_D1_zt278 = (gRatt * _X1_X2_D1_zt316);
        const auto _X1_X2_D1_zt280 = (_X1_X2_D1_zt192 * _X1_X2_D1_zt316);
        const auto _X1_X2_D1_zt281 = (gRfb * _X1_X2_D1_zt318);
        const auto _X1_X2_D1_zt290 = (_X1_X2_D1_zt195 * _X1_X2_D1_zt296);
        const auto _X1_X2_D1_zt297 = (_X1_X2_D1_zt298 - _X1_X2_D1_zt299);
        const auto _X1_X2_D1_zt315 = (_X1_X2_D1_zt278 - _X1_X2_D1_zt280);
        const auto _X1_X2_D1_zt323 = (VCC * _X1_X2_D1_zt324);
        const auto _X1_X2_D1_zt330 = (_X1_X2_D1_zt195 * _X1_X2_D1_zt353);
        const auto _X1_X2_D1_zt335 = (_X1_X2_D1_zt336 - _X1_X2_D1_zt330);
        const auto _X1_X2_D1_zt347 = (gX1_Cpg * _X1_X2_D1_zt353);
        const auto _X1_X2_D1_zt205 = (_X1_X2_D1_zt201 + _X1_X2_D1_zt206);
        const auto _X1_X2_D1_zt213 = (_X1_X2_D1_zt214 - _X1_X2_D1_zt217);
        const auto _X1_X2_D1_zt221 = (_X1_X2_D1_zt225 - _X1_X2_D1_zt205);
        const auto _X1_X2_D1_zt224 = (_X1_X2_D1_zt205 - _X1_X2_D1_zt225);
        const auto _X1_X2_D1_zt240 = (_X1_X2_D1_zt195 * _X1_X2_D1_zt241);
        const auto _X1_X2_D1_zt251 = (_X1_X2_D1_zt240 - _X1_X2_D1_zt253);
        const auto _X1_X2_D1_zt257 = (_X1_X2_D1_zt264 - _X1_X2_D1_zt265);
        const auto _X1_X2_D1_zt272 = (_X1_X2_D1_zt306 - _X1_X2_D1_zt275);
        const auto _X1_X2_D1_zt273 = (gRfb * _X1_X2_D1_zt272);
        const auto _X1_X2_D1_zt277 = (_X1_X2_D1_zt145 * _X1_X2_D1_zt315);
        const auto _X1_X2_D1_zt286 = (_X1_X2_D1_zt290 + _X1_X2_D1_zt289);
        const auto _X1_X2_D1_zt305 = (gRatt * _X1_X2_D1_zt272);
        const auto _X1_X2_D1_zt309 = (_X1_X2_D1_zt192 * _X1_X2_D1_zt272);
        const auto _X1_X2_D1_zt311 = (gRfb * _X1_X2_D1_zt273);
        const auto _X1_X2_D1_zt314 = (_X1_X2_D1_zt277 + _X1_X2_D1_zt281);
        const auto _X1_X2_D1_zt334 = (_X1_X2_D1_zt335 + _X1_X2_D1_zt341);
        const auto _X1_X2_D1_zt346 = (_X1_X2_D1_zt347 + _X1_X2_D1_zt358);
        const auto _X1_X2_D1_zt200 = (gRatt * _X1_X2_D1_zt224);
        const auto _X1_X2_D1_zt212 = (_X1_X2_D1_zt213 / _X1_X2_D1_zt2);
        const auto _X1_X2_D1_zt250 = (_X1_X2_D1_zt251 + _X1_X2_D1_zt235);
        const auto _X1_X2_D1_zt285 = (_X1_X2_D1_zt286 - _X1_X2_D1_zt282);
        const auto _X1_X2_D1_zt304 = (_X1_X2_D1_zt305 - _X1_X2_D1_zt309);
        const auto _X1_X2_D1_zt313 = (gX1_Cpg * _X1_X2_D1_zt314);
        const auto _X1_X2_D1_zt329 = (_X1_X2_D1_zt334 - _X1_X2_D1_zt323);
        const auto _X1_X2_D1_zt249 = (_X1_X2_D1_zt250 + _X1_X2_D1_zt246);
        const auto _X1_X2_D1_zt303 = (_X1_X2_D1_zt145 * _X1_X2_D1_zt304);
        const auto _X1_X2_D1_zt302 = (_X1_X2_D1_zt303 + _X1_X2_D1_zt311);
        const auto _X1_X2_D1_zt267 = (_X1_X2_D1_zt64 * _X1_X2_D1_zt302);
        const auto _X1_X2_D1_zt301 = (_X1_X2_D1_zt267 + _X1_X2_D1_zt313);
        const auto _X1_X2_D1_zt266 = (_X1_X2_D1_zt301 / _X1_X2_D1_zt2);
        const auto _X1_X2_D1_voc0 = ((((_X1_X2_D1_zt64 * ((_X1_X2_D1_zt145 * (_X1_X2_D1_zt200 - (_X1_X2_D1_zt192 * _X1_X2_D1_zt224))) + (gRfb * (gRfb * _X1_X2_D1_zt224)))) + (_X1_X2_D1_zt148 * _X1_X2_D1_zt209)) / _X1_X2_D1_zt2) - _X1_X2_D1_zt212);
        const auto _X1_X2_D1_voc1 = ((((gX1_Cpg * ((_X1_X2_D1_zt145 * ((_X1_X2_D1_zt192 * _X1_X2_D1_zt221) + _X1_X2_D1_zt200)) - (gRfb * (gRfb * _X1_X2_D1_zt221)))) - (_X1_X2_D1_zt144 * _X1_X2_D1_zt209)) / _X1_X2_D1_zt2) - _X1_X2_D1_zt212);
        const auto _X1_X2_D1_voc2 = (((((_X1_X2_D1_zt64 * ((_X1_X2_D1_zt145 * ((gRatt * ((_X1_X2_D1_zt235 - (_X1_X2_D1_zt253 - _X1_X2_D1_zt240)) + _X1_X2_D1_zt246)) - (_X1_X2_D1_zt192 * _X1_X2_D1_zt249))) + (gRfb * (gRfb * _X1_X2_D1_zt249)))) + (gX1_Cpg * ((_X1_X2_D1_zt145 * ((_X1_X2_D1_zt192 * _X1_X2_D1_zt257) + (gRatt * (_X1_X2_D1_zt265 - _X1_X2_D1_zt264)))) - (gRfb * (gRfb * _X1_X2_D1_zt257))))) - (_X1_X2_D1_zt65 * _X1_X2_D1_zt209)) / _X1_X2_D1_zt2) - _X1_X2_D1_zt266);
        const auto _X1_X2_D1_voc3 = (((((_X1_X2_D1_zt64 * ((_X1_X2_D1_zt145 * ((gRatt * _X1_X2_D1_zt285) - (_X1_X2_D1_zt192 * _X1_X2_D1_zt285))) + (gRfb * (gRfb * _X1_X2_D1_zt285)))) + (gX1_Cpg * ((_X1_X2_D1_zt145 * ((_X1_X2_D1_zt192 * _X1_X2_D1_zt297) + (gRatt * (_X1_X2_D1_zt299 - _X1_X2_D1_zt298)))) - (gRfb * (gRfb * _X1_X2_D1_zt297))))) - (_X1_X2_D1_zt112 * _X1_X2_D1_zt209)) / _X1_X2_D1_zt2) - _X1_X2_D1_zt266);
        const auto _X1_X2_D1_voc4 = (((((_X1_X2_D1_zt64 * ((_X1_X2_D1_zt145 * ((gRatt * ((_X1_X2_D1_zt341 - (_X1_X2_D1_zt330 - _X1_X2_D1_zt336)) - _X1_X2_D1_zt323)) - (_X1_X2_D1_zt192 * _X1_X2_D1_zt329))) + (gRfb * (gRfb * _X1_X2_D1_zt329)))) + (gX1_Cpg * ((_X1_X2_D1_zt145 * ((gRatt * _X1_X2_D1_zt346) - (_X1_X2_D1_zt192 * _X1_X2_D1_zt346))) + (gRfb * (gRfb * _X1_X2_D1_zt346))))) + (_X1_X2_D1_zt156 * _X1_X2_D1_zt209)) / _X1_X2_D1_zt2) - (((gX1_Cpg * ((-(_X1_X2_D1_zt145 * _X1_X2_D1_zt360)) - (_X1_X2_D1_zt40 * zRk1Ck1))) - (_X1_X2_D1_zt64 * ((_X1_X2_D1_zt145 * _X1_X2_D1_zt218) + (_X1_X2_D1_zt34 * zRk1Ck1)))) / _X1_X2_D1_zt2));
        if (_k == 0) {
            c0__X1_X2_D1_voc2 = _X1_X2_D1_voc2;
            c0__X1_X2_D1_voc3 = _X1_X2_D1_voc3;
            c0__X1_X2_D1_voc4 = _X1_X2_D1_voc4;
            c0__X1_X2_D1_voc0 = _X1_X2_D1_voc0;
            c0__X1_X2_D1_voc1 = _X1_X2_D1_voc1;
            c0__X1_X2_D1_zt266 = _X1_X2_D1_zt266;
            c0__X1_X2_D1_zt329 = _X1_X2_D1_zt329;
            c0__X1_X2_D1_zt301 = _X1_X2_D1_zt301;
            c0__X1_X2_D1_zt334 = _X1_X2_D1_zt334;
            c0__X1_X2_D1_zt267 = _X1_X2_D1_zt267;
            c0__X1_X2_D1_zt200 = _X1_X2_D1_zt200;
            c0__X1_X2_D1_zt335 = _X1_X2_D1_zt335;
        } else {
            c__X1_X2_D1_voc2[_k - 1] = _X1_X2_D1_voc2 - c0__X1_X2_D1_voc2;
            c__X1_X2_D1_voc3[_k - 1] = _X1_X2_D1_voc3 - c0__X1_X2_D1_voc3;
            c__X1_X2_D1_voc4[_k - 1] = _X1_X2_D1_voc4 - c0__X1_X2_D1_voc4;
            c__X1_X2_D1_voc0[_k - 1] = _X1_X2_D1_voc0 - c0__X1_X2_D1_voc0;
            c__X1_X2_D1_voc1[_k - 1] = _X1_X2_D1_voc1 - c0__X1_X2_D1_voc1;
            c__X1_X2_D1_zt266[_k - 1] = _X1_X2_D1_zt266 - c0__X1_X2_D1_zt266;
            c__X1_X2_D1_zt329[_k - 1] = _X1_X2_D1_zt329 - c0__X1_X2_D1_zt329;
            c__X1_X2_D1_zt301[_k - 1] = _X1_X2_D1_zt301 - c0__X1_X2_D1_zt301;
            c__X1_X2_D1_zt334[_k - 1] = _X1_X2_D1_zt334 - c0__X1_X2_D1_zt334;
            c__X1_X2_D1_zt267[_k - 1] = _X1_X2_D1_zt267 - c0__X1_X2_D1_zt267;
            c__X1_X2_D1_zt200[_k - 1] = _X1_X2_D1_zt200 - c0__X1_X2_D1_zt200;
            c__X1_X2_D1_zt335[_k - 1] = _X1_X2_D1_zt335 - c0__X1_X2_D1_zt335;
        }
    }
    
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zCin = state[ch].zCin;
        auto zRk1Ck1 = state[ch].zRk1Ck1;
        auto zX1_Cpg = state[ch].zX1_Cpg;
        auto zCc12 = state[ch].zCc12;
        auto zRrelCenv = state[ch].zRrelCenv;
        auto zX2_Cpg = state[ch].zX2_Cpg;
        auto zCout = state[ch].zCout;
        auto zCsc = state[ch].zCsc;
        auto zRk2Ck2 = state[ch].zRk2Ck2;
        auto vPKX1 = state[ch].vPKX1;
        auto vPKX1_prev = state[ch].vPKX1_prev;
        auto vGKX1 = state[ch].vGKX1;
        auto vGKX1_prev = state[ch].vGKX1_prev;
        auto vPKX2 = state[ch].vPKX2;
        auto vPKX2_prev = state[ch].vPKX2_prev;
        auto vGKX2 = state[ch].vGKX2;
        auto vGKX2_prev = state[ch].vGKX2_prev;
        auto vD1 = state[ch].vD1;
        auto vD1_prev = state[ch].vD1_prev;
        long long nr_solves_X1_X2_D1 = state[ch].nr_solves_X1_X2_D1;
        long long nr_iters_X1_X2_D1 = state[ch].nr_iters_X1_X2_D1;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            { const auto _prev_step = vPKX1 - vPKX1_prev; vPKX1_prev = vPKX1;
vPKX1 = vPKX1 + (_prev_step);
            }
            { const auto _prev_step = vGKX1 - vGKX1_prev; vGKX1_prev = vGKX1;
vGKX1 = vGKX1 + (_prev_step);
            }
            { const auto _prev_step = vPKX2 - vPKX2_prev; vPKX2_prev = vPKX2;
vPKX2 = vPKX2 + (_prev_step);
            }
            { const auto _prev_step = vGKX2 - vGKX2_prev; vGKX2_prev = vGKX2;
vGKX2 = vGKX2 + (_prev_step);
            }
            { const auto _prev_step = vD1 - vD1_prev; vD1_prev = vD1;
vD1 = limit_junction_voltage(vD1 + (_prev_step), vD1, D1N914_vt, vcrit_D1N914_vt);
            }

            // --- Newton-Raphson solve (N-port): X1_X2_D1
            const auto _X1_X2_D1_voc2 = c0__X1_X2_D1_voc2 + c__X1_X2_D1_voc2[0] * vi + c__X1_X2_D1_voc2[1] * zCin + c__X1_X2_D1_voc2[2] * zRk1Ck1 + c__X1_X2_D1_voc2[3] * zX1_Cpg + c__X1_X2_D1_voc2[4] * zCc12 + c__X1_X2_D1_voc2[5] * zRrelCenv + c__X1_X2_D1_voc2[6] * zX2_Cpg + c__X1_X2_D1_voc2[7] * zCout + c__X1_X2_D1_voc2[8] * zCsc + c__X1_X2_D1_voc2[9] * zRk2Ck2;
            const auto _X1_X2_D1_voc3 = c0__X1_X2_D1_voc3 + c__X1_X2_D1_voc3[0] * vi + c__X1_X2_D1_voc3[1] * zCin + c__X1_X2_D1_voc3[2] * zRk1Ck1 + c__X1_X2_D1_voc3[3] * zX1_Cpg + c__X1_X2_D1_voc3[4] * zCc12 + c__X1_X2_D1_voc3[5] * zRrelCenv + c__X1_X2_D1_voc3[6] * zX2_Cpg + c__X1_X2_D1_voc3[7] * zCout + c__X1_X2_D1_voc3[8] * zCsc + c__X1_X2_D1_voc3[9] * zRk2Ck2;
            const auto _X1_X2_D1_voc4 = c0__X1_X2_D1_voc4 + c__X1_X2_D1_voc4[0] * vi + c__X1_X2_D1_voc4[1] * zCin + c__X1_X2_D1_voc4[2] * zRk1Ck1 + c__X1_X2_D1_voc4[3] * zX1_Cpg + c__X1_X2_D1_voc4[4] * zCc12 + c__X1_X2_D1_voc4[5] * zRrelCenv + c__X1_X2_D1_voc4[6] * zX2_Cpg + c__X1_X2_D1_voc4[7] * zCout + c__X1_X2_D1_voc4[8] * zCsc + c__X1_X2_D1_voc4[9] * zRk2Ck2;
            const auto _X1_X2_D1_voc0 = c0__X1_X2_D1_voc0 + c__X1_X2_D1_voc0[0] * vi + c__X1_X2_D1_voc0[1] * zCin + c__X1_X2_D1_voc0[2] * zRk1Ck1 + c__X1_X2_D1_voc0[3] * zX1_Cpg + c__X1_X2_D1_voc0[4] * zCc12 + c__X1_X2_D1_voc0[5] * zRrelCenv + c__X1_X2_D1_voc0[6] * zX2_Cpg + c__X1_X2_D1_voc0[7] * zCout + c__X1_X2_D1_voc0[8] * zCsc + c__X1_X2_D1_voc0[9] * zRk2Ck2;
            const auto _X1_X2_D1_voc1 = c0__X1_X2_D1_voc1 + c__X1_X2_D1_voc1[0] * vi + c__X1_X2_D1_voc1[1] * zCin + c__X1_X2_D1_voc1[2] * zRk1Ck1 + c__X1_X2_D1_voc1[3] * zX1_Cpg + c__X1_X2_D1_voc1[4] * zCc12 + c__X1_X2_D1_voc1[5] * zRrelCenv + c__X1_X2_D1_voc1[6] * zX2_Cpg + c__X1_X2_D1_voc1[7] * zCout + c__X1_X2_D1_voc1[8] * zCsc + c__X1_X2_D1_voc1[9] * zRk2Ck2;
            const auto _X1_X2_D1_zt266 = c0__X1_X2_D1_zt266 + c__X1_X2_D1_zt266[0] * vi + c__X1_X2_D1_zt266[1] * zCin + c__X1_X2_D1_zt266[2] * zRk1Ck1 + c__X1_X2_D1_zt266[3] * zX1_Cpg + c__X1_X2_D1_zt266[4] * zCc12 + c__X1_X2_D1_zt266[5] * zRrelCenv + c__X1_X2_D1_zt266[6] * zX2_Cpg + c__X1_X2_D1_zt266[7] * zCout + c__X1_X2_D1_zt266[8] * zCsc + c__X1_X2_D1_zt266[9] * zRk2Ck2;
            const auto _X1_X2_D1_zt329 = c0__X1_X2_D1_zt329 + c__X1_X2_D1_zt329[0] * vi + c__X1_X2_D1_zt329[1] * zCin + c__X1_X2_D1_zt329[2] * zRk1Ck1 + c__X1_X2_D1_zt329[3] * zX1_Cpg + c__X1_X2_D1_zt329[4] * zCc12 + c__X1_X2_D1_zt329[5] * zRrelCenv + c__X1_X2_D1_zt329[6] * zX2_Cpg + c__X1_X2_D1_zt329[7] * zCout + c__X1_X2_D1_zt329[8] * zCsc + c__X1_X2_D1_zt329[9] * zRk2Ck2;
            const auto _X1_X2_D1_zt301 = c0__X1_X2_D1_zt301 + c__X1_X2_D1_zt301[0] * vi + c__X1_X2_D1_zt301[1] * zCin + c__X1_X2_D1_zt301[2] * zRk1Ck1 + c__X1_X2_D1_zt301[3] * zX1_Cpg + c__X1_X2_D1_zt301[4] * zCc12 + c__X1_X2_D1_zt301[5] * zRrelCenv + c__X1_X2_D1_zt301[6] * zX2_Cpg + c__X1_X2_D1_zt301[7] * zCout + c__X1_X2_D1_zt301[8] * zCsc + c__X1_X2_D1_zt301[9] * zRk2Ck2;
            const auto _X1_X2_D1_zt334 = c0__X1_X2_D1_zt334 + c__X1_X2_D1_zt334[0] * vi + c__X1_X2_D1_zt334[1] * zCin + c__X1_X2_D1_zt334[2] * zRk1Ck1 + c__X1_X2_D1_zt334[3] * zX1_Cpg + c__X1_X2_D1_zt334[4] * zCc12 + c__X1_X2_D1_zt334[5] * zRrelCenv + c__X1_X2_D1_zt334[6] * zX2_Cpg + c__X1_X2_D1_zt334[7] * zCout + c__X1_X2_D1_zt334[8] * zCsc + c__X1_X2_D1_zt334[9] * zRk2Ck2;
            const auto _X1_X2_D1_zt267 = c0__X1_X2_D1_zt267 + c__X1_X2_D1_zt267[0] * vi + c__X1_X2_D1_zt267[1] * zCin + c__X1_X2_D1_zt267[2] * zRk1Ck1 + c__X1_X2_D1_zt267[3] * zX1_Cpg + c__X1_X2_D1_zt267[4] * zCc12 + c__X1_X2_D1_zt267[5] * zRrelCenv + c__X1_X2_D1_zt267[6] * zX2_Cpg + c__X1_X2_D1_zt267[7] * zCout + c__X1_X2_D1_zt267[8] * zCsc + c__X1_X2_D1_zt267[9] * zRk2Ck2;
            const auto _X1_X2_D1_zt200 = c0__X1_X2_D1_zt200 + c__X1_X2_D1_zt200[0] * vi + c__X1_X2_D1_zt200[1] * zCin + c__X1_X2_D1_zt200[2] * zRk1Ck1 + c__X1_X2_D1_zt200[3] * zX1_Cpg + c__X1_X2_D1_zt200[4] * zCc12 + c__X1_X2_D1_zt200[5] * zRrelCenv + c__X1_X2_D1_zt200[6] * zX2_Cpg + c__X1_X2_D1_zt200[7] * zCout + c__X1_X2_D1_zt200[8] * zCsc + c__X1_X2_D1_zt200[9] * zRk2Ck2;
            const auto _X1_X2_D1_zt335 = c0__X1_X2_D1_zt335 + c__X1_X2_D1_zt335[0] * vi + c__X1_X2_D1_zt335[1] * zCin + c__X1_X2_D1_zt335[2] * zRk1Ck1 + c__X1_X2_D1_zt335[3] * zX1_Cpg + c__X1_X2_D1_zt335[4] * zCc12 + c__X1_X2_D1_zt335[5] * zRrelCenv + c__X1_X2_D1_zt335[6] * zX2_Cpg + c__X1_X2_D1_zt335[7] * zCout + c__X1_X2_D1_zt335[8] * zCsc + c__X1_X2_D1_zt335[9] * zRk2Ck2;
            const auto _X1_X2_D1_zt236 = (zX1_Cpg + zCc12);
            const auto _X1_X2_D1_zt245 = (zX2_Cpg + zCout);
            const auto _X1_X2_D1_zt263 = (zCc12 + zX2_Cpg);
            const auto _X1_X2_D1_zt271 = (_X1_X2_D1_zt78 * zRk2Ck2);
            const auto _X1_X2_D1_zt288 = (-_X1_X2_D1_zt245);
            const auto _X1_X2_D1_zt293 = (zCout - zCsc);
            const auto _X1_X2_D1_zt294 = (gCsc * zCsc);
            const auto _X1_X2_D1_zt333 = (_X1_X2_D1_zt260 * zCsc);
            const auto _X1_X2_D1_zt348 = (gCout * zCsc);
            const auto _X1_X2_D1_zt359 = (gCin * vi);
            const auto _X1_X2_D1_zt202 = (_X1_X2_D1_zt116 * _X1_X2_D1_zt288);
            const auto _X1_X2_D1_zt211 = (_X1_X2_D1_zt359 - zCin);
            const auto _X1_X2_D1_zt228 = (-_X1_X2_D1_zt236);
            const auto _X1_X2_D1_zt256 = (gRp2 * _X1_X2_D1_zt116);
            const auto _X1_X2_D1_zt270 = (gCout * _X1_X2_D1_zt271);
            const auto _X1_X2_D1_zt292 = (_X1_X2_D1_zt293 * gCsc);
            const auto _X1_X2_D1_zt310 = (_X1_X2_D1_zt88 * zRk2Ck2);
            const auto _X1_X2_D1_zt321 = (_X1_X2_D1_zt333 + _X1_X2_D1_zt292);
            const auto _X1_X2_D1_zt339 = (gRk2Ck2 * _X1_X2_D1_zt348);
            const auto _X1_X2_D1_zt341 = (_X1_X2_D1_zt143 * _X1_X2_D1_zt228);
            const auto _X1_X2_D1_zt343 = (gRp1 * _X1_X2_D1_zt143);
            const auto _X1_X2_D1_zt345 = (gRp2 * _X1_X2_D1_zt4);
            const auto _X1_X2_D1_zt349 = (_X1_X2_D1_zt4 * _X1_X2_D1_zt288);
            const auto _X1_X2_D1_zt351 = (_X1_X2_D1_zt141 * _X1_X2_D1_zt263);
            const auto _X1_X2_D1_zt356 = (gRk2Ck2 * _X1_X2_D1_zt321);
            const auto _X1_X2_D1_zt357 = (gX2_Cpg * _X1_X2_D1_zt356);
            const auto _X1_X2_D1_zt204 = (gRatt * _X1_X2_D1_zt256);
            const auto _X1_X2_D1_zt207 = (_X1_X2_D1_zt113 * _X1_X2_D1_zt263);
            const auto _X1_X2_D1_zt210 = (_X1_X2_D1_zt211 - zX1_Cpg);
            const auto _X1_X2_D1_zt226 = (gX2_Cpg * _X1_X2_D1_zt204);
            const auto _X1_X2_D1_zt227 = (gCc12 * _X1_X2_D1_zt226);
            const auto _X1_X2_D1_zt235 = (_X1_X2_D1_zt63 * _X1_X2_D1_zt228);
            const auto _X1_X2_D1_zt244 = (_X1_X2_D1_zt292 + _X1_X2_D1_zt294);
            const auto _X1_X2_D1_zt247 = (gCc12 * _X1_X2_D1_zt204);
            const auto _X1_X2_D1_zt248 = (gRp1 * _X1_X2_D1_zt63);
            const auto _X1_X2_D1_zt258 = (_X1_X2_D1_zt39 * _X1_X2_D1_zt263);
            const auto _X1_X2_D1_zt261 = (_X1_X2_D1_zt38 * _X1_X2_D1_zt204);
            const auto _X1_X2_D1_zt284 = (_X1_X2_D1_zt195 * _X1_X2_D1_zt226);
            const auto _X1_X2_D1_zt291 = (gRk2Ck2 * _X1_X2_D1_zt244);
            const auto _X1_X2_D1_zt300 = (gX1_Cpg * _X1_X2_D1_zt226);
            const auto _X1_X2_D1_zt307 = (_X1_X2_D1_zt42 * _X1_X2_D1_zt310);
            const auto _X1_X2_D1_zt308 = (gX2_Cpg * _X1_X2_D1_zt310);
            const auto _X1_X2_D1_zt328 = (gRatt * _X1_X2_D1_zt345);
            const auto _X1_X2_D1_zt332 = (gCout * _X1_X2_D1_zt339);
            const auto _X1_X2_D1_zt340 = (_X1_X2_D1_zt42 * _X1_X2_D1_zt356);
            const auto _X1_X2_D1_zt350 = (gRatt * _X1_X2_D1_zt357);
            const auto _X1_X2_D1_zt352 = (_X1_X2_D1_zt38 * _X1_X2_D1_zt328);
            const auto _X1_X2_D1_zt354 = (gX1_Cpg * _X1_X2_D1_zt352);
            const auto _X1_X2_D1_zt358 = (VCC * _X1_X2_D1_zt354);
            const auto _X1_X2_D1_zt201 = (_X1_X2_D1_zt16 * _X1_X2_D1_zt228);
            const auto _X1_X2_D1_zt203 = (gRp1 * _X1_X2_D1_zt16);
            const auto _X1_X2_D1_zt209 = (-_X1_X2_D1_zt210);
            const auto _X1_X2_D1_zt222 = (_X1_X2_D1_zt203 + _X1_X2_D1_zt227);
            const auto _X1_X2_D1_zt225 = (VCC * _X1_X2_D1_zt222);
            const auto _X1_X2_D1_zt238 = (gCc12 * _X1_X2_D1_zt247);
            const auto _X1_X2_D1_zt239 = (_X1_X2_D1_zt195 * _X1_X2_D1_zt261);
            const auto _X1_X2_D1_zt252 = (gCout * _X1_X2_D1_zt291);
            const auto _X1_X2_D1_zt255 = (_X1_X2_D1_zt248 + _X1_X2_D1_zt239);
            const auto _X1_X2_D1_zt259 = (gX1_Cpg * _X1_X2_D1_zt261);
            const auto _X1_X2_D1_zt265 = (VCC * _X1_X2_D1_zt259);
            const auto _X1_X2_D1_zt269 = (_X1_X2_D1_zt307 - _X1_X2_D1_zt270);
            const auto _X1_X2_D1_zt276 = (gRatt * _X1_X2_D1_zt308);
            const auto _X1_X2_D1_zt279 = (gX2_Cpg * _X1_X2_D1_zt276);
            const auto _X1_X2_D1_zt289 = (_X1_X2_D1_zt36 * _X1_X2_D1_zt228);
            const auto _X1_X2_D1_zt295 = (gRp1 * _X1_X2_D1_zt36);
            const auto _X1_X2_D1_zt299 = (VCC * _X1_X2_D1_zt300);
            const auto _X1_X2_D1_zt320 = (gX2_Cpg * _X1_X2_D1_zt350);
            const auto _X1_X2_D1_zt327 = (gCc12 * _X1_X2_D1_zt328);
            const auto _X1_X2_D1_zt331 = (_X1_X2_D1_zt332 - _X1_X2_D1_zt340);
            const auto _X1_X2_D1_zt344 = (_X1_X2_D1_zt195 * _X1_X2_D1_zt352);
            const auto _X1_X2_D1_zt355 = (_X1_X2_D1_zt331 - _X1_X2_D1_zt349);
            const auto _X1_X2_D1_zt360 = (zRrelCenv * _X1_X2_D1_zt231);
            const auto _X1_X2_D1_zt218 = (zRrelCenv * _X1_X2_D1_zt154);
            const auto _X1_X2_D1_zt230 = (-_X1_X2_D1_zt360);
            const auto _X1_X2_D1_zt233 = (gRfb * _X1_X2_D1_zt218);
            const auto _X1_X2_D1_zt243 = (_X1_X2_D1_zt202 + _X1_X2_D1_zt252);
            const auto _X1_X2_D1_zt254 = (_X1_X2_D1_zt238 - _X1_X2_D1_zt255);
            const auto _X1_X2_D1_zt268 = (gRatt * _X1_X2_D1_zt269);
            const auto _X1_X2_D1_zt274 = (gCc12 * _X1_X2_D1_zt268);
            const auto _X1_X2_D1_zt283 = (_X1_X2_D1_zt295 + _X1_X2_D1_zt284);
            const auto _X1_X2_D1_zt306 = (gCc12 * _X1_X2_D1_zt274);
            const auto _X1_X2_D1_zt317 = (_X1_X2_D1_zt38 * _X1_X2_D1_zt268);
            const auto _X1_X2_D1_zt326 = (gCc12 * _X1_X2_D1_zt327);
            const auto _X1_X2_D1_zt337 = (gRatt * _X1_X2_D1_zt355);
            const auto _X1_X2_D1_zt338 = (_X1_X2_D1_zt38 * _X1_X2_D1_zt337);
            const auto _X1_X2_D1_zt342 = (_X1_X2_D1_zt343 + _X1_X2_D1_zt344);
            const auto _X1_X2_D1_zt215 = (_X1_X2_D1_zt14 * zRk1Ck1);
            const auto _X1_X2_D1_zt216 = (gRfb * _X1_X2_D1_zt230);
            const auto _X1_X2_D1_zt220 = (_X1_X2_D1_zt12 * zRk1Ck1);
            const auto _X1_X2_D1_zt229 = (_X1_X2_D1_zt215 + _X1_X2_D1_zt216);
            const auto _X1_X2_D1_zt232 = (_X1_X2_D1_zt233 - _X1_X2_D1_zt220);
            const auto _X1_X2_D1_zt242 = (gRatt * _X1_X2_D1_zt243);
            const auto _X1_X2_D1_zt246 = (VCC * _X1_X2_D1_zt254);
            const auto _X1_X2_D1_zt262 = (_X1_X2_D1_zt38 * _X1_X2_D1_zt242);
            const auto _X1_X2_D1_zt282 = (VCC * _X1_X2_D1_zt283);
            const auto _X1_X2_D1_zt287 = (gX2_Cpg * _X1_X2_D1_zt242);
            const auto _X1_X2_D1_zt296 = (_X1_X2_D1_zt207 + _X1_X2_D1_zt287);
            const auto _X1_X2_D1_zt298 = (gX1_Cpg * _X1_X2_D1_zt296);
            const auto _X1_X2_D1_zt312 = (_X1_X2_D1_zt317 - _X1_X2_D1_zt279);
            const auto _X1_X2_D1_zt316 = (gX1_Cpg * _X1_X2_D1_zt312);
            const auto _X1_X2_D1_zt318 = (gRfb * _X1_X2_D1_zt316);
            const auto _X1_X2_D1_zt319 = (_X1_X2_D1_zt338 + _X1_X2_D1_zt320);
            const auto _X1_X2_D1_zt322 = (gCc12 * _X1_X2_D1_zt337);
            const auto _X1_X2_D1_zt324 = (_X1_X2_D1_zt342 - _X1_X2_D1_zt326);
            const auto _X1_X2_D1_zt336 = (gCc12 * _X1_X2_D1_zt322);
            const auto _X1_X2_D1_zt353 = (_X1_X2_D1_zt319 - _X1_X2_D1_zt351);
            const auto _X1_X2_D1_zt206 = (gCc12 * _X1_X2_D1_zt296);
            const auto _X1_X2_D1_zt214 = (gX1_Cpg * _X1_X2_D1_zt229);
            const auto _X1_X2_D1_zt217 = (_X1_X2_D1_zt64 * _X1_X2_D1_zt232);
            const auto _X1_X2_D1_zt237 = (gCc12 * _X1_X2_D1_zt242);
            const auto _X1_X2_D1_zt241 = (_X1_X2_D1_zt262 + _X1_X2_D1_zt258);
            const auto _X1_X2_D1_zt253 = (gCc12 * _X1_X2_D1_zt237);
            const auto _X1_X2_D1_zt264 = (gX1_Cpg * _X1_X2_D1_zt241);
            const auto _X1_X2_D1_zt275 = (_X1_X2_D1_zt195 * _X1_X2_D1_zt312);
            const auto _X1_X2_D1_zt278 = (gRatt * _X1_X2_D1_zt316);
            const auto _X1_X2_D1_zt280 = (_X1_X2_D1_zt192 * _X1_X2_D1_zt316);
            const auto _X1_X2_D1_zt281 = (gRfb * _X1_X2_D1_zt318);
            const auto _X1_X2_D1_zt290 = (_X1_X2_D1_zt195 * _X1_X2_D1_zt296);
            const auto _X1_X2_D1_zt297 = (_X1_X2_D1_zt298 - _X1_X2_D1_zt299);
            const auto _X1_X2_D1_zt315 = (_X1_X2_D1_zt278 - _X1_X2_D1_zt280);
            const auto _X1_X2_D1_zt323 = (VCC * _X1_X2_D1_zt324);
            const auto _X1_X2_D1_zt330 = (_X1_X2_D1_zt195 * _X1_X2_D1_zt353);
            const auto _X1_X2_D1_zt347 = (gX1_Cpg * _X1_X2_D1_zt353);
            const auto _X1_X2_D1_zt205 = (_X1_X2_D1_zt201 + _X1_X2_D1_zt206);
            const auto _X1_X2_D1_zt213 = (_X1_X2_D1_zt214 - _X1_X2_D1_zt217);
            const auto _X1_X2_D1_zt221 = (_X1_X2_D1_zt225 - _X1_X2_D1_zt205);
            const auto _X1_X2_D1_zt224 = (_X1_X2_D1_zt205 - _X1_X2_D1_zt225);
            const auto _X1_X2_D1_zt240 = (_X1_X2_D1_zt195 * _X1_X2_D1_zt241);
            const auto _X1_X2_D1_zt251 = (_X1_X2_D1_zt240 - _X1_X2_D1_zt253);
            const auto _X1_X2_D1_zt257 = (_X1_X2_D1_zt264 - _X1_X2_D1_zt265);
            const auto _X1_X2_D1_zt272 = (_X1_X2_D1_zt306 - _X1_X2_D1_zt275);
            const auto _X1_X2_D1_zt273 = (gRfb * _X1_X2_D1_zt272);
            const auto _X1_X2_D1_zt277 = (_X1_X2_D1_zt145 * _X1_X2_D1_zt315);
            const auto _X1_X2_D1_zt286 = (_X1_X2_D1_zt290 + _X1_X2_D1_zt289);
            const auto _X1_X2_D1_zt305 = (gRatt * _X1_X2_D1_zt272);
            const auto _X1_X2_D1_zt309 = (_X1_X2_D1_zt192 * _X1_X2_D1_zt272);
            const auto _X1_X2_D1_zt311 = (gRfb * _X1_X2_D1_zt273);
            const auto _X1_X2_D1_zt314 = (_X1_X2_D1_zt277 + _X1_X2_D1_zt281);
            const auto _X1_X2_D1_zt346 = (_X1_X2_D1_zt347 + _X1_X2_D1_zt358);
            const auto _X1_X2_D1_zt212 = (_X1_X2_D1_zt213 / _X1_X2_D1_zt2);
            const auto _X1_X2_D1_zt250 = (_X1_X2_D1_zt251 + _X1_X2_D1_zt235);
            const auto _X1_X2_D1_zt285 = (_X1_X2_D1_zt286 - _X1_X2_D1_zt282);
            const auto _X1_X2_D1_zt304 = (_X1_X2_D1_zt305 - _X1_X2_D1_zt309);
            const auto _X1_X2_D1_zt313 = (gX1_Cpg * _X1_X2_D1_zt314);
            const auto _X1_X2_D1_zt249 = (_X1_X2_D1_zt250 + _X1_X2_D1_zt246);
            const auto _X1_X2_D1_zt303 = (_X1_X2_D1_zt145 * _X1_X2_D1_zt304);
            const auto _X1_X2_D1_zt302 = (_X1_X2_D1_zt303 + _X1_X2_D1_zt311);
            const auto _X1_X2_D1_pt3 = (_12AX7_DEMPWOLF_Gamma - 1.0);
            const auto _X1_X2_D1_pt10 = (_12AX7_DEMPWOLF_Xi - 1.0);
            ++nr_solves_X1_X2_D1;
            for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
            {
                ++nr_iters_X1_X2_D1;
                const auto _X1_X2_D1_pt2 = math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1)));
                const auto _X1_X2_D1_pt9 = (_12AX7_DEMPWOLF_Cg * vGKX1);
                const auto _X1_X2_D1_pt13 = math_exp_approx(_X1_X2_D1_pt9);
                const auto _X1_X2_D1_pt16 = math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX2 / _12AX7_DEMPWOLF_Mu) + vGKX2)));
                const auto _X1_X2_D1_pt25 = (_12AX7_DEMPWOLF_Cg * vGKX2);
                const auto _X1_X2_D1_pt26 = (vD1 / D1N914_vt);
                const auto _X1_X2_D1_pt43 = math_exp_approx(_X1_X2_D1_pt26);
                const auto _X1_X2_D1_pt1 = (1.0 + _X1_X2_D1_pt2);
                const auto _X1_X2_D1_pt8 = (1.0 + _X1_X2_D1_pt13);
                const auto _X1_X2_D1_pt12 = math_log_approx(_X1_X2_D1_pt8);
                const auto _X1_X2_D1_pt15 = (1.0 + _X1_X2_D1_pt16);
                const auto _X1_X2_D1_pt24 = math_exp_approx(_X1_X2_D1_pt25);
                const auto _X1_X2_D1_pt0 = (math_log_approx(_X1_X2_D1_pt1) / _12AX7_DEMPWOLF_Ck);
                const auto _X1_X2_D1_pt7 = (_X1_X2_D1_pt12 / _12AX7_DEMPWOLF_Cg);
                const auto _X1_X2_D1_pt14 = (math_log_approx(_X1_X2_D1_pt15) / _12AX7_DEMPWOLF_Ck);
                const auto _X1_X2_D1_pt23 = (1.0 + _X1_X2_D1_pt24);
                const auto _X1_X2_D1_pt30 = math_pow_approx(_X1_X2_D1_pt0, _X1_X2_D1_pt3);
                const auto _X1_X2_D1_pt38 = math_pow_approx(_X1_X2_D1_pt14, _X1_X2_D1_pt3);
                const auto _X1_X2_D1_pt6 = math_pow_approx(_X1_X2_D1_pt7, _X1_X2_D1_pt10);
                const auto _X1_X2_D1_pt22 = math_log_approx(_X1_X2_D1_pt23);
                const auto _X1_X2_D1_pt29 = (_X1_X2_D1_pt30 * _X1_X2_D1_pt2);
                const auto _X1_X2_D1_pt34 = (_X1_X2_D1_pt6 * _X1_X2_D1_pt13);
                const auto _X1_X2_D1_pt37 = (_X1_X2_D1_pt38 * _X1_X2_D1_pt16);
                const auto _X1_X2_D1_pt5 = (_X1_X2_D1_pt6 * _X1_X2_D1_pt7);
                const auto _X1_X2_D1_pt11 = (_12AX7_DEMPWOLF_Gg * _X1_X2_D1_pt5);
                const auto _X1_X2_D1_pt21 = (_X1_X2_D1_pt22 / _12AX7_DEMPWOLF_Cg);
                const auto _X1_X2_D1_pt28 = (_12AX7_DEMPWOLF_Gamma * _X1_X2_D1_pt29);
                const auto _X1_X2_D1_pt33 = (_12AX7_DEMPWOLF_Xi * _X1_X2_D1_pt34);
                const auto _X1_X2_D1_pt36 = (_12AX7_DEMPWOLF_Gamma * _X1_X2_D1_pt37);
                const auto _X1_X2_D1_pt4 = (_X1_X2_D1_pt11 + _12AX7_DEMPWOLF_Ig0);
                const auto _X1_X2_D1_pt19 = math_pow_approx(_X1_X2_D1_pt21, _X1_X2_D1_pt10);
                const auto _X1_X2_D1_pt27 = (_12AX7_DEMPWOLF_Gk * _X1_X2_D1_pt28);
                const auto _X1_X2_D1_pt32 = (_12AX7_DEMPWOLF_Gg * _X1_X2_D1_pt33);
                const auto _X1_X2_D1_pt35 = (_12AX7_DEMPWOLF_Gk * _X1_X2_D1_pt36);
                const auto _X1_X2_D1_pt40 = (_X1_X2_D1_pt19 * _X1_X2_D1_pt24);
                const auto _X1_X2_D1_pt42 = (_12AX7_DEMPWOLF_Xi * _X1_X2_D1_pt40);
                const auto _X1_X2_D1_pt18 = (_X1_X2_D1_pt19 * _X1_X2_D1_pt21);
                const auto _X1_X2_D1_pt20 = (_12AX7_DEMPWOLF_Gg * _X1_X2_D1_pt18);
                const auto _X1_X2_D1_pt31 = (_X1_X2_D1_pt32 / _X1_X2_D1_pt8);
                const auto _X1_X2_D1_pt39 = (_12AX7_DEMPWOLF_Gg * _X1_X2_D1_pt42);
                const auto _X1_X2_D1_pt41 = (_X1_X2_D1_pt39 / _X1_X2_D1_pt23);
                const auto _X1_X2_D1_pt17 = (_X1_X2_D1_pt20 + _12AX7_DEMPWOLF_Ig0);
                const auto _X1_X2_D1_i0 = ((_12AX7_DEMPWOLF_Gk * (_X1_X2_D1_pt30 * _X1_X2_D1_pt0)) - _X1_X2_D1_pt4);
                const auto _X1_X2_D1_i1 = _X1_X2_D1_pt4;
                const auto _X1_X2_D1_i2 = ((_12AX7_DEMPWOLF_Gk * (_X1_X2_D1_pt38 * _X1_X2_D1_pt14)) - _X1_X2_D1_pt17);
                const auto _X1_X2_D1_i3 = _X1_X2_D1_pt17;
                const auto _X1_X2_D1_i4 = (D1N914_Is * (_X1_X2_D1_pt43 - 1.0));
                const auto _X1_X2_D1_g0_0 = (_X1_X2_D1_pt27 / (_12AX7_DEMPWOLF_Mu * _X1_X2_D1_pt1));
                const auto _X1_X2_D1_g0_1 = ((_X1_X2_D1_pt27 / _X1_X2_D1_pt1) - _X1_X2_D1_pt31);
                const auto _X1_X2_D1_g1_0 = 0.0;
                const auto _X1_X2_D1_g1_1 = _X1_X2_D1_pt31;
                const auto _X1_X2_D1_g2_2 = (_X1_X2_D1_pt35 / (_12AX7_DEMPWOLF_Mu * _X1_X2_D1_pt15));
                const auto _X1_X2_D1_g2_3 = ((_X1_X2_D1_pt35 / _X1_X2_D1_pt15) - _X1_X2_D1_pt41);
                const auto _X1_X2_D1_g3_2 = 0.0;
                const auto _X1_X2_D1_g3_3 = _X1_X2_D1_pt41;
                const auto _X1_X2_D1_g4_4 = ((D1N914_Is * _X1_X2_D1_pt43) / D1N914_vt);
                const auto res_vPKX1 = ((((((_X1_X2_D1_voc0 + (_X1_X2_D1_Z0_0 * _X1_X2_D1_i0)) + (_X1_X2_D1_Z0_1 * _X1_X2_D1_i1)) + (_X1_X2_D1_Z0_2 * _X1_X2_D1_i2)) + (_X1_X2_D1_Z0_3 * _X1_X2_D1_i3)) + (_X1_X2_D1_Z0_4 * _X1_X2_D1_i4)) - vPKX1);
                const auto res_vGKX1 = ((((((_X1_X2_D1_voc1 + (_X1_X2_D1_Z1_0 * _X1_X2_D1_i0)) + (_X1_X2_D1_Z1_1 * _X1_X2_D1_i1)) + (_X1_X2_D1_Z1_2 * _X1_X2_D1_i2)) + (_X1_X2_D1_Z1_3 * _X1_X2_D1_i3)) + (_X1_X2_D1_Z1_4 * _X1_X2_D1_i4)) - vGKX1);
                const auto res_vPKX2 = ((((((_X1_X2_D1_voc2 + (_X1_X2_D1_Z2_0 * _X1_X2_D1_i0)) + (_X1_X2_D1_Z2_1 * _X1_X2_D1_i1)) + (_X1_X2_D1_Z2_2 * _X1_X2_D1_i2)) + (_X1_X2_D1_Z2_3 * _X1_X2_D1_i3)) + (_X1_X2_D1_Z2_4 * _X1_X2_D1_i4)) - vPKX2);
                const auto res_vGKX2 = ((((((_X1_X2_D1_voc3 + (_X1_X2_D1_Z3_0 * _X1_X2_D1_i0)) + (_X1_X2_D1_Z3_1 * _X1_X2_D1_i1)) + (_X1_X2_D1_Z3_2 * _X1_X2_D1_i2)) + (_X1_X2_D1_Z3_3 * _X1_X2_D1_i3)) + (_X1_X2_D1_Z3_4 * _X1_X2_D1_i4)) - vGKX2);
                const auto res_vD1 = ((((((_X1_X2_D1_voc4 + (_X1_X2_D1_Z4_0 * _X1_X2_D1_i0)) + (_X1_X2_D1_Z4_1 * _X1_X2_D1_i1)) + (_X1_X2_D1_Z4_2 * _X1_X2_D1_i2)) + (_X1_X2_D1_Z4_3 * _X1_X2_D1_i3)) + (_X1_X2_D1_Z4_4 * _X1_X2_D1_i4)) - vD1);
                const auto _X1_X2_D1_J0_0 = (((_X1_X2_D1_Z0_0 * _X1_X2_D1_g0_0) + (_X1_X2_D1_Z0_1 * _X1_X2_D1_g1_0)) - 1.0);
                const auto _X1_X2_D1_J0_1 = ((_X1_X2_D1_Z0_0 * _X1_X2_D1_g0_1) + (_X1_X2_D1_Z0_1 * _X1_X2_D1_g1_1));
                const auto _X1_X2_D1_J0_2 = ((_X1_X2_D1_Z0_2 * _X1_X2_D1_g2_2) + (_X1_X2_D1_Z0_3 * _X1_X2_D1_g3_2));
                const auto _X1_X2_D1_J0_3 = ((_X1_X2_D1_Z0_2 * _X1_X2_D1_g2_3) + (_X1_X2_D1_Z0_3 * _X1_X2_D1_g3_3));
                const auto _X1_X2_D1_J0_4 = (_X1_X2_D1_Z0_4 * _X1_X2_D1_g4_4);
                const auto _X1_X2_D1_J1_0 = ((_X1_X2_D1_Z1_0 * _X1_X2_D1_g0_0) + (_X1_X2_D1_Z1_1 * _X1_X2_D1_g1_0));
                const auto _X1_X2_D1_J1_1 = (((_X1_X2_D1_Z1_0 * _X1_X2_D1_g0_1) + (_X1_X2_D1_Z1_1 * _X1_X2_D1_g1_1)) - 1.0);
                const auto _X1_X2_D1_J1_2 = ((_X1_X2_D1_Z1_2 * _X1_X2_D1_g2_2) + (_X1_X2_D1_Z1_3 * _X1_X2_D1_g3_2));
                const auto _X1_X2_D1_J1_3 = ((_X1_X2_D1_Z1_2 * _X1_X2_D1_g2_3) + (_X1_X2_D1_Z1_3 * _X1_X2_D1_g3_3));
                const auto _X1_X2_D1_J1_4 = (_X1_X2_D1_Z1_4 * _X1_X2_D1_g4_4);
                const auto _X1_X2_D1_J2_0 = ((_X1_X2_D1_Z2_0 * _X1_X2_D1_g0_0) + (_X1_X2_D1_Z2_1 * _X1_X2_D1_g1_0));
                const auto _X1_X2_D1_J2_1 = ((_X1_X2_D1_Z2_0 * _X1_X2_D1_g0_1) + (_X1_X2_D1_Z2_1 * _X1_X2_D1_g1_1));
                const auto _X1_X2_D1_J2_2 = (((_X1_X2_D1_Z2_2 * _X1_X2_D1_g2_2) + (_X1_X2_D1_Z2_3 * _X1_X2_D1_g3_2)) - 1.0);
                const auto _X1_X2_D1_J2_3 = ((_X1_X2_D1_Z2_2 * _X1_X2_D1_g2_3) + (_X1_X2_D1_Z2_3 * _X1_X2_D1_g3_3));
                const auto _X1_X2_D1_J2_4 = (_X1_X2_D1_Z2_4 * _X1_X2_D1_g4_4);
                const auto _X1_X2_D1_J3_0 = ((_X1_X2_D1_Z3_0 * _X1_X2_D1_g0_0) + (_X1_X2_D1_Z3_1 * _X1_X2_D1_g1_0));
                const auto _X1_X2_D1_J3_1 = ((_X1_X2_D1_Z3_0 * _X1_X2_D1_g0_1) + (_X1_X2_D1_Z3_1 * _X1_X2_D1_g1_1));
                const auto _X1_X2_D1_J3_2 = ((_X1_X2_D1_Z3_2 * _X1_X2_D1_g2_2) + (_X1_X2_D1_Z3_3 * _X1_X2_D1_g3_2));
                const auto _X1_X2_D1_J3_3 = (((_X1_X2_D1_Z3_2 * _X1_X2_D1_g2_3) + (_X1_X2_D1_Z3_3 * _X1_X2_D1_g3_3)) - 1.0);
                const auto _X1_X2_D1_J3_4 = (_X1_X2_D1_Z3_4 * _X1_X2_D1_g4_4);
                const auto _X1_X2_D1_J4_0 = ((_X1_X2_D1_Z4_0 * _X1_X2_D1_g0_0) + (_X1_X2_D1_Z4_1 * _X1_X2_D1_g1_0));
                const auto _X1_X2_D1_J4_1 = ((_X1_X2_D1_Z4_0 * _X1_X2_D1_g0_1) + (_X1_X2_D1_Z4_1 * _X1_X2_D1_g1_1));
                const auto _X1_X2_D1_J4_2 = ((_X1_X2_D1_Z4_2 * _X1_X2_D1_g2_2) + (_X1_X2_D1_Z4_3 * _X1_X2_D1_g3_2));
                const auto _X1_X2_D1_J4_3 = ((_X1_X2_D1_Z4_2 * _X1_X2_D1_g2_3) + (_X1_X2_D1_Z4_3 * _X1_X2_D1_g3_3));
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
            
                vPKX1 = vPKX1 + (delta_vPKX1);
                vGKX1 = vGKX1 + (delta_vGKX1);
                vPKX2 = vPKX2 + (delta_vPKX2);
                vGKX2 = vGKX2 + (delta_vGKX2);
                vD1 = limit_junction_voltage(vD1 + (delta_vD1), vD1, D1N914_vt, vcrit_D1N914_vt);
            
                if (residual_norm_sq < newton_tol_sq)
                    break;
                
            }

            const auto _t6 = (_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi));
            const auto _t14 = (zCout - zCsc);
            const auto _t16 = (D1N914_Is * (math_exp_approx((vD1 / D1N914_vt)) - 1.0));
            const auto _t25 = (_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi));
            const auto _t80 = (_t14 * gCsc);
            const auto _t5 = (_t6 + _12AX7_DEMPWOLF_Ig0);
            const auto _t15 = (zCsc + _t16);
            const auto _t24 = (_t25 + _12AX7_DEMPWOLF_Ig0);
            const auto _t26 = ((zCc12 + zX2_Cpg) + _t24);
            const auto _t48 = (_t5 - (((gCin * vi) - zCin) - zX1_Cpg));
            const auto _t81 = (gCsc * _t15);
            const auto _t151 = (gRk2Ck2 * ((_t59 * _t15) + _t80));
            const auto _t153 = (gX2_Cpg * _t151);
            const auto _t4 = ((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - _t5);
            const auto _t13 = (_t80 + _t81);
            const auto _t23 = ((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX2 / _12AX7_DEMPWOLF_Mu) + vGKX2))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - _t24);
            const auto _t79 = (gRk2Ck2 * _t13);
            const auto _t82 = ((zRk1Ck1 + _t4) + _t5);
            const auto _t87 = (_t16 * _t62);
            const auto _t152 = (gRatt * _t153);
            const auto _t164 = (gX2_Cpg * _t152);
            const auto _t179 = ((zRk2Ck2 + _t23) + _t24);
            const auto _t3 = (_t4 - (zX1_Cpg + zCc12));
            const auto _t12 = (gX2_Cpg * _t79);
            const auto _t22 = (_t23 - (zX2_Cpg + zCout));
            const auto _t78 = (gCout * _t79);
            const auto _t86 = (gX2_Cpg * _t87);
            const auto _t101 = (_t26 * _t64);
            const auto _t118 = (_t16 * _t56);
            const auto _t123 = (gCc12 * _t118);
            const auto _t127 = (_t22 * _t57);
            const auto _t129 = (_t26 * _t61);
            const auto _t154 = (_t26 * _t165);
            const auto _t178 = (_t179 * _t58);
            const auto _t183 = (gX2_Cpg * _t178);
            const auto _t187 = (gRatt * _t183);
            const auto _t11 = (gRatt * _t12);
            const auto _t21 = (_t22 * _t8);
            const auto _t34 = (_t3 * _t35);
            const auto _t40 = (gX2_Cpg * _t11);
            const auto _t92 = (_t3 * _t60);
            const auto _t95 = (_t127 + _t78);
            const auto _t106 = (_t3 * _t55);
            const auto _t117 = (_t18 * _t118);
            const auto _t119 = (zRrelCenv * _t71);
            const auto _t122 = (gCc12 * _t123);
            const auto _t124 = (_t3 * _t125);
            const auto _t137 = (gRatt * _t95);
            const auto _t138 = (gCc12 * _t137);
            const auto _t150 = (((gCout * (gRk2Ck2 * (gCout * _t15))) - (_t20 * _t151)) - _t21);
            const auto _t161 = (gRatt * _t150);
            const auto _t163 = (_t18 * _t161);
            const auto _t177 = ((_t20 * _t178) - (gCout * (_t179 * _t9)));
            const auto _t182 = (gX2_Cpg * _t187);
            const auto _t185 = (gRatt * _t177);
            const auto _t186 = (_t18 * _t185);
            const auto _t19 = ((_t20 * _t79) + _t21);
            const auto _t41 = (gRatt * _t19);
            const auto _t105 = (gX2_Cpg * _t137);
            const auto _t116 = (_t86 - _t117);
            const auto _t121 = (zRrelCenv * _t63);
            const auto _t126 = (gCc12 * _t138);
            const auto _t128 = (_t18 * _t137);
            const auto _t136 = (_t128 + _t129);
            const auto _t144 = (gX1_Cpg * _t136);
            const auto _t160 = (gCc12 * _t161);
            const auto _t162 = (_t163 + _t164);
            const auto _t181 = (_t186 - _t182);
            const auto _t184 = (gCc12 * _t185);
            const auto _t188 = (gX1_Cpg * _t181);
            const auto _t17 = (_t18 * _t41);
            const auto _t27 = (gCc12 * _t41);
            const auto _t39 = (_t40 - _t17);
            const auto _t42 = (gCc12 * _t27);
            const auto _t85 = (gX1_Cpg * _t116);
            const auto _t90 = (_t10 * _t116);
            const auto _t94 = (_t101 + _t105);
            const auto _t104 = (_t10 * _t94);
            const auto _t114 = (gX1_Cpg * _t94);
            const auto _t135 = (_t10 * _t136);
            const auto _t143 = (_t144 - _t145);
            const auto _t149 = (_t162 - _t154);
            const auto _t155 = (gCc12 * _t160);
            const auto _t171 = (gX1_Cpg * _t149);
            const auto _t176 = (gCc12 * _t184);
            const auto _t180 = (_t10 * _t181);
            const auto _t38 = (_t39 - _t154);
            const auto _t46 = (gX1_Cpg * _t38);
            const auto _t84 = (gRatt * _t85);
            const auto _t89 = (_t90 + _t122);
            const auto _t93 = (gCc12 * _t94);
            const auto _t100 = (_t92 + _t93);
            const auto _t103 = (_t104 + _t106);
            const auto _t111 = (_t114 - _t112);
            const auto _t134 = (((_t135 - _t126) + _t124) + _t130);
            const auto _t148 = (_t10 * _t149);
            const auto _t159 = (((_t155 - _t148) + _t34) - _t166);
            const auto _t169 = (_t171 + _t170);
            const auto _t175 = (_t176 - _t180);
            const auto _t37 = (_t10 * _t38);
            const auto _t45 = (_t46 + _t170);
            const auto _t73 = (_t74 - _t100);
            const auto _t83 = (_t84 - _t119);
            const auto _t88 = (gRatt * _t89);
            const auto _t99 = (_t100 - _t74);
            const auto _t102 = (_t103 - _t108);
            const auto _t120 = (_t121 + _t88);
            const auto _t36 = (_t37 + _t42);
            const auto _t91 = (gRatt * _t99);
            const auto _t33 = (_t34 - _t36);
            const auto _t2 = (_t33 + _t43);
            const auto vo = ((((_t0 * ((_t1 * ((gRatt * _t2) - (_t32 * _t2))) + (gRfb * (gRfb * _t2)))) + (gX1_Cpg * ((_t1 * ((gRatt * _t45) - (_t32 * _t45))) + (gRfb * (gRfb * _t45))))) - (_t48 * _t189)) * _t51);
            const auto vg1 = (((gX1_Cpg * ((_t1 * ((_t32 * _t73) + _t91)) - (gRfb * (gRfb * _t73)))) - (_t48 * _t52)) * _t51);
            const auto tCin = (gCin * (vi - vg1));
            const auto vk1 = (((gX1_Cpg * ((_t82 * _t70) + (gRfb * _t83))) - (_t0 * ((gRfb * _t120) - (_t82 * _t53)))) * _t51);
            const auto vRk1Ck1 = (vk1 - 0);
            const auto vpl1 = (((_t0 * ((_t1 * (_t91 - (_t32 * _t99))) + (gRfb * (gRfb * _t99)))) + (_t48 * _t69)) * _t51);
            const auto tX1_Cpg = (gX1_Cpg * (vpl1 - vg1));
            const auto vg2 = ((((_t0 * ((_t1 * ((gRatt * _t102) - (_t32 * _t102))) + (gRfb * (gRfb * _t102)))) + (gX1_Cpg * ((_t1 * ((_t32 * _t111) + (gRatt * (_t112 - _t114)))) - (gRfb * (gRfb * _t111))))) - (_t48 * _t190)) * _t51);
            const auto tCc12 = (gCc12 * (vpl1 - vg2));
            const auto venv = (((gX1_Cpg * ((_t1 * _t83) - (_t82 * _t72))) - (_t0 * ((_t1 * _t120) + (_t82 * _t68)))) * _t51);
            const auto vRrelCenv = (venv - 0);
            const auto vpl2 = ((((_t0 * ((_t1 * ((gRatt * ((_t124 - (_t126 - _t135)) + _t130)) - (_t32 * _t134))) + (gRfb * (gRfb * _t134)))) + (gX1_Cpg * ((_t1 * ((_t32 * _t143) + (gRatt * (_t145 - _t144)))) - (gRfb * (gRfb * _t143))))) - (_t48 * _t191)) * _t51);
            const auto tX2_Cpg = (gX2_Cpg * (vpl2 - vg2));
            const auto tCout = (gCout * (vpl2 - vo));
            const auto vsc = ((((_t0 * ((_t1 * ((gRatt * ((_t34 - (_t148 - _t155)) - _t166)) - (_t32 * _t159))) + (gRfb * (gRfb * _t159)))) + (gX1_Cpg * ((_t1 * ((gRatt * _t169) - (_t32 * _t169))) + (gRfb * (gRfb * _t169))))) + (_t48 * _t192)) * _t51);
            const auto tCsc = (gCsc * (vo - vsc));
            const auto vk2 = (((_t0 * ((_t1 * ((gRatt * _t175) - (_t32 * _t175))) + (gRfb * (gRfb * _t175)))) + (gX1_Cpg * ((_t1 * ((gRatt * _t188) - (_t32 * _t188))) + (gRfb * (gRfb * _t188))))) * _t51);
            const auto vRk2Ck2 = (vk2 - 0);
            
            zCin = 2 * tCin - zCin;
            zRk1Ck1 = gzRk1Ck1 * vRk1Ck1 - zRk1Ck1; // RC parallel
            zX1_Cpg = 2 * tX1_Cpg - zX1_Cpg;
            zCc12 = 2 * tCc12 - zCc12;
            zRrelCenv = gzRrelCenv * vRrelCenv - zRrelCenv; // RC parallel
            zX2_Cpg = 2 * tX2_Cpg - zX2_Cpg;
            zCout = 2 * tCout - zCout;
            zCsc = 2 * tCsc - zCsc;
            zRk2Ck2 = gzRk2Ck2 * vRk2Ck2 - zRk2Ck2; // RC parallel

            output[ch][n] = vo;
        }
        state[ch].zCin = zCin;
        state[ch].zRk1Ck1 = zRk1Ck1;
        state[ch].zX1_Cpg = zX1_Cpg;
        state[ch].zCc12 = zCc12;
        state[ch].zRrelCenv = zRrelCenv;
        state[ch].zX2_Cpg = zX2_Cpg;
        state[ch].zCout = zCout;
        state[ch].zCsc = zCsc;
        state[ch].zRk2Ck2 = zRk2Ck2;
        state[ch].vPKX1 = vPKX1;
        state[ch].vPKX1_prev = vPKX1_prev;
        state[ch].vGKX1 = vGKX1;
        state[ch].vGKX1_prev = vGKX1_prev;
        state[ch].vPKX2 = vPKX2;
        state[ch].vPKX2_prev = vPKX2_prev;
        state[ch].vGKX2 = vGKX2;
        state[ch].vGKX2_prev = vGKX2_prev;
        state[ch].vD1 = vD1;
        state[ch].vD1_prev = vD1_prev;
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
    
    const auto gX1_Cpg = 2.0 * sample_rate * params.X1_Cpg;
    
    const auto gCc12 = 2.0 * sample_rate * params.Cc12;
    
    const auto gRg2 = 1.0 / params.Rg2;
    
    const auto gRatt = 1.0 / params.Ratt;
    
    const auto Rrel = params.Rrel;
    const auto Cenv = params.Cenv;
    const auto gRrelCenv = 2.0 * sample_rate * Cenv + (1.0 / Rrel);
    const auto gzRrelCenv = 4.0 * sample_rate * Cenv;
    
    const auto gRp2 = 1.0 / params.Rp2;
    
    const auto gX2_Cpg = 2.0 * sample_rate * params.X2_Cpg;
    
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
    const auto _X1_D1_pt13 = (_12AX7_DEMPWOLF_Xi - 1.0);
    const auto _X1_D1_pt18 = (_12AX7_DEMPWOLF_Gamma - 1.0);
    for (int newton_iter = 0; newton_iter < 10000; ++newton_iter)
    {
        const auto _X1_D1_pt3 = math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1)));
        const auto _X1_D1_pt12 = (_12AX7_DEMPWOLF_Cg * vGKX1);
        const auto _X1_D1_pt14 = (vD1 / D1N914_vt);
        const auto _X1_D1_pt23 = math_exp_approx(_X1_D1_pt14);
        const auto _X1_D1_pt2 = (1.0 + _X1_D1_pt3);
        const auto _X1_D1_pt7 = math_exp_approx(_X1_D1_pt12);
        const auto _X1_D1_pt1 = (math_log_approx(_X1_D1_pt2) / _12AX7_DEMPWOLF_Ck);
        const auto _X1_D1_pt6 = (1.0 + _X1_D1_pt7);
        const auto _X1_D1_pt11 = math_log_approx(_X1_D1_pt6);
        const auto _X1_D1_pt0 = math_pow_approx(_X1_D1_pt1, _X1_D1_pt18);
        const auto _X1_D1_pt5 = (_X1_D1_pt11 / _12AX7_DEMPWOLF_Cg);
        const auto _X1_D1_pt10 = math_pow_approx(_X1_D1_pt5, _X1_D1_pt13);
        const auto _X1_D1_pt16 = (_X1_D1_pt0 * _X1_D1_pt3);
        const auto _X1_D1_pt22 = (_X1_D1_pt10 * _X1_D1_pt7);
        const auto _X1_D1_pt4 = (_X1_D1_pt10 * _X1_D1_pt5);
        const auto _X1_D1_pt9 = (_12AX7_DEMPWOLF_Gg * _X1_D1_pt4);
        const auto _X1_D1_pt15 = (_12AX7_DEMPWOLF_Gamma * _X1_D1_pt16);
        const auto _X1_D1_pt17 = (_12AX7_DEMPWOLF_Gk * _X1_D1_pt15);
        const auto _X1_D1_pt20 = (_12AX7_DEMPWOLF_Xi * _X1_D1_pt22);
        const auto _X1_D1_pt8 = (_X1_D1_pt9 + _12AX7_DEMPWOLF_Ig0);
        const auto _X1_D1_pt19 = (_12AX7_DEMPWOLF_Gg * _X1_D1_pt20);
        const auto _X1_D1_pt21 = (_X1_D1_pt19 / _X1_D1_pt6);
        const auto _X1_D1_i0 = ((_12AX7_DEMPWOLF_Gk * (_X1_D1_pt0 * _X1_D1_pt1)) - _X1_D1_pt8);
        const auto _X1_D1_i1 = _X1_D1_pt8;
        const auto _X1_D1_i2 = (D1N914_Is * (_X1_D1_pt23 - 1.0));
        const auto _X1_D1_g0_0 = (_X1_D1_pt17 / (_12AX7_DEMPWOLF_Mu * _X1_D1_pt2));
        const auto _X1_D1_g0_1 = ((_X1_D1_pt17 / _X1_D1_pt2) - _X1_D1_pt21);
        const auto _X1_D1_g1_0 = 0.0;
        const auto _X1_D1_g1_1 = _X1_D1_pt21;
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
        const auto _X1_D1_pt35 = (vGKX1 - _X1_D1_pt25);
        const auto _X1_D1_pt36 = (vD1 - _X1_D1_pt26);
        const auto _X1_D1_pt37 = (_X1_D1_Z0_2 * _X1_D1_g2_2);
        const auto _X1_D1_pt38 = (_X1_D1_pt35 * _X1_D1_pt32);
        const auto _X1_D1_pt40 = (((_X1_D1_Z0_0 * _X1_D1_g0_0) + (_X1_D1_Z0_1 * _X1_D1_g1_0)) - 1.0);
        const auto _X1_D1_pt42 = ((_X1_D1_Z1_0 * _X1_D1_g0_0) + (_X1_D1_Z1_1 * _X1_D1_g1_0));
        const auto _X1_D1_pt43 = ((_X1_D1_Z2_0 * _X1_D1_g0_0) + (_X1_D1_Z2_1 * _X1_D1_g1_0));
        const auto _X1_D1_pt44 = ((_X1_D1_pt42 * _X1_D1_pt32) - (_X1_D1_pt29 * _X1_D1_pt43));
        const auto _X1_D1_pt46 = (_X1_D1_pt31 * _X1_D1_pt36);
        const auto _X1_D1_pt48 = (_X1_D1_pt42 * _X1_D1_pt36);
        const auto _X1_D1_pt49 = (_X1_D1_pt29 * _X1_D1_pt36);
        const auto _X1_D1_pt50 = (_X1_D1_pt35 * _X1_D1_pt43);
        const auto _X1_D1_pt28 = ((_X1_D1_pt29 * _X1_D1_pt30) - (_X1_D1_pt31 * _X1_D1_pt32));
        const auto _X1_D1_pt34 = (_X1_D1_pt35 * _X1_D1_pt30);
        const auto _X1_D1_pt41 = ((_X1_D1_pt42 * _X1_D1_pt30) - (_X1_D1_pt31 * _X1_D1_pt43));
        const auto _X1_D1_pt45 = (_X1_D1_pt34 - _X1_D1_pt46);
        const auto _X1_D1_pt47 = (_X1_D1_pt48 - _X1_D1_pt50);
        const auto _X1_D1_pt39 = (((_X1_D1_pt40 * _X1_D1_pt28) - (_X1_D1_pt33 * _X1_D1_pt41)) + (_X1_D1_pt37 * _X1_D1_pt44));
        const auto res_vPKX1 = (_X1_D1_pt24 - vPKX1);
        const auto res_vGKX1 = (_X1_D1_pt25 - vGKX1);
        const auto res_vD1 = (_X1_D1_pt26 - vD1);
        const auto delta_vPKX1 = ((((_X1_D1_pt27 * _X1_D1_pt28) - (_X1_D1_pt33 * _X1_D1_pt45)) + (_X1_D1_pt37 * (_X1_D1_pt38 - _X1_D1_pt49))) / _X1_D1_pt39);
        const auto delta_vGKX1 = ((((_X1_D1_pt40 * _X1_D1_pt45) - (_X1_D1_pt27 * _X1_D1_pt41)) + (_X1_D1_pt37 * _X1_D1_pt47)) / _X1_D1_pt39);
        const auto delta_vD1 = ((((_X1_D1_pt40 * (_X1_D1_pt49 - _X1_D1_pt38)) - (_X1_D1_pt33 * _X1_D1_pt47)) + (_X1_D1_pt27 * _X1_D1_pt44)) / _X1_D1_pt39);
    
        auto residual_norm_sq = 0.0;
        residual_norm_sq += res_vPKX1 * res_vPKX1;
        residual_norm_sq += res_vGKX1 * res_vGKX1;
        residual_norm_sq += res_vD1 * res_vD1;
    
        vPKX1 = vPKX1 + (delta_vPKX1);
        vGKX1 = vGKX1 + (delta_vGKX1);
        vD1 = limit_junction_voltage(vD1 + (delta_vD1), vD1, D1N914_vt, vcrit_D1N914_vt);
    
        if (residual_norm_sq < newton_tol_sq)
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
    const auto _X2_pt3 = (_12AX7_DEMPWOLF_Gamma - 1.0);
    const auto _X2_pt10 = (_12AX7_DEMPWOLF_Xi - 1.0);
    for (int newton_iter = 0; newton_iter < 10000; ++newton_iter)
    {
        const auto _X2_pt2 = math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX2 / _12AX7_DEMPWOLF_Mu) + vGKX2)));
        const auto _X2_pt9 = (_12AX7_DEMPWOLF_Cg * vGKX2);
        const auto _X2_pt13 = math_exp_approx(_X2_pt9);
        const auto _X2_pt1 = (1.0 + _X2_pt2);
        const auto _X2_pt8 = (1.0 + _X2_pt13);
        const auto _X2_pt12 = math_log_approx(_X2_pt8);
        const auto _X2_pt0 = (math_log_approx(_X2_pt1) / _12AX7_DEMPWOLF_Ck);
        const auto _X2_pt11 = (_X2_pt12 / _12AX7_DEMPWOLF_Cg);
        const auto _X2_pt17 = math_pow_approx(_X2_pt0, _X2_pt3);
        const auto _X2_pt7 = math_pow_approx(_X2_pt11, _X2_pt10);
        const auto _X2_pt16 = (_X2_pt17 * _X2_pt2);
        const auto _X2_pt20 = (_X2_pt7 * _X2_pt13);
        const auto _X2_pt6 = (_X2_pt7 * _X2_pt11);
        const auto _X2_pt15 = (_12AX7_DEMPWOLF_Gamma * _X2_pt16);
        const auto _X2_pt19 = (_12AX7_DEMPWOLF_Xi * _X2_pt20);
        const auto _X2_pt5 = (_12AX7_DEMPWOLF_Gg * _X2_pt6);
        const auto _X2_pt14 = (_12AX7_DEMPWOLF_Gk * _X2_pt15);
        const auto _X2_pt18 = (_12AX7_DEMPWOLF_Gg * _X2_pt19);
        const auto _X2_pt21 = (_X2_pt18 / _X2_pt8);
        const auto _X2_pt4 = (_X2_pt5 + _12AX7_DEMPWOLF_Ig0);
        const auto _X2_i0 = ((_12AX7_DEMPWOLF_Gk * (_X2_pt17 * _X2_pt0)) - _X2_pt4);
        const auto _X2_i1 = _X2_pt4;
        const auto _X2_g0_0 = (_X2_pt14 / (_12AX7_DEMPWOLF_Mu * _X2_pt1));
        const auto _X2_g0_1 = ((_X2_pt14 / _X2_pt1) - _X2_pt21);
        const auto _X2_g1_0 = 0.0;
        const auto _X2_g1_1 = _X2_pt21;
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
    
        vPKX2 = vPKX2 + (delta_vPKX2);
        vGKX2 = vGKX2 + (delta_vGKX2);
    
        if (residual_norm_sq < newton_tol_sq)
            break;
        
    }
    const auto zCin = (gCin * (vi + (((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0) / (gRg1 + (1.0 / 1000000000.0)))));
    const auto zRk1Ck1 = (-(((gzRk1Ck1 * ((gRfb * (gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((D1N914_Is * (math_exp_approx((vD1 / D1N914_vt)) - 1.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))) - ((((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) + ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))))) / (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))) + (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) / 2.0));
    const auto zX1_Cpg = (gX1_Cpg * (((((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((VCC * (gRp1 * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - (((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((VCC * (gRp1 * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - (((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) + (gRfb * (gRfb * ((VCC * (gRp1 * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - (((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) / (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))) + (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) + (((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0) / (gRg1 + (1.0 / 1000000000.0)))));
    const auto zCc12 = (gCc12 * (((((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((VCC * (gRp1 * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - (((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((VCC * (gRp1 * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - (((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) + (gRfb * (gRfb * ((VCC * (gRp1 * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - (((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) / (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))) + (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) + (((gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * (((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))) - ((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * (((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * (((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - (gRatt * ((gRp1 + (1.0 / 1000000000.0)) * (((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) / (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))) + (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))))));
    const auto zRrelCenv = (-(((gzRrelCenv * (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * (gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((D1N914_Is * (math_exp_approx((vD1 / D1N914_vt)) - 1.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))) + ((((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) + ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) / (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))) + (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) / 2.0));
    const auto zX2_Cpg = (gX2_Cpg * (((((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((VCC * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * (gRp2 * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * (((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX2 / _12AX7_DEMPWOLF_Mu) + vGKX2))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((VCC * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * (gRp2 * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * (((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX2 / _12AX7_DEMPWOLF_Mu) + vGKX2))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) + (gRfb * (gRfb * ((VCC * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * (gRp2 * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * (((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX2 / _12AX7_DEMPWOLF_Mu) + vGKX2))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) / (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))) + (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) + (((gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * (((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))) - ((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * (((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * (((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - (gRatt * ((gRp1 + (1.0 / 1000000000.0)) * (((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) / (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))) + (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))))));
    const auto zCout = ((gCout * (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((VCC * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * (gRp2 * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * (((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX2 / _12AX7_DEMPWOLF_Mu) + vGKX2))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((VCC * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * (gRp2 * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * (((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX2 / _12AX7_DEMPWOLF_Mu) + vGKX2))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) + (gRfb * (gRfb * ((VCC * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * (gRp2 * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * (((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX2 / _12AX7_DEMPWOLF_Mu) + vGKX2))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))))) / (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))) + (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))));
    const auto zCsc = ((gCsc * ((((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((D1N914_Is * (math_exp_approx((vD1 / D1N914_vt)) - 1.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))) - (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((D1N914_Is * (math_exp_approx((vD1 / D1N914_vt)) - 1.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))) + ((((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) + ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) / (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))) + (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))));
    const auto zRk2Ck2 = (((gzRk2Ck2 * (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * ((((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX2 / _12AX7_DEMPWOLF_Mu) + vGKX2))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) + ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * ((((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX2 / _12AX7_DEMPWOLF_Mu) + vGKX2))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) + ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * (gRL + (1.0 / 1000000000.0)))))))))) + (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * ((((_12AX7_DEMPWOLF_Gk * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Ck * ((vPKX2 / _12AX7_DEMPWOLF_Mu) + vGKX2))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) + ((_12AX7_DEMPWOLF_Gg * math_pow_approx((math_log_approx((1.0 + math_exp_approx((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * (gRL + (1.0 / 1000000000.0))))))))))) / (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))) + (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) / 2.0);

    const auto vo_dc_out = 0.0;

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].vPKX1 = vPKX1;
        state[ch].vPKX1_prev = vPKX1;
        state[ch].vGKX1 = vGKX1;
        state[ch].vGKX1_prev = vGKX1;
        state[ch].vD1 = vD1;
        state[ch].vD1_prev = vD1;
        state[ch].vPKX2 = vPKX2;
        state[ch].vPKX2_prev = vPKX2;
        state[ch].vGKX2 = vGKX2;
        state[ch].vGKX2_prev = vGKX2;
        state[ch].zCin = zCin;
        state[ch].zRk1Ck1 = zRk1Ck1;
        state[ch].zX1_Cpg = zX1_Cpg;
        state[ch].zCc12 = zCc12;
        state[ch].zRrelCenv = zRrelCenv;
        state[ch].zX2_Cpg = zX2_Cpg;
        state[ch].zCout = zCout;
        state[ch].zCsc = zCsc;
        state[ch].zRk2Ck2 = zRk2Ck2;
        state[ch].nr_solves_X1_X2_D1 = 0;
        state[ch].nr_iters_X1_X2_D1 = 0;
    }
    return vo_dc_out;
}

