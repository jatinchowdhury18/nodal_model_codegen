// Auto-generated with netlist_codegen version d1e5ccb.
// Command: netlist_codegen tube_compressor.net tube_compressor.h -type_name double

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
    else if (v_new < -vcrit && std::abs(v_new - v_old) > 2 * vt)
    {
        if (v_old < 0)
        {
            const auto arg = 1 + (v_old - v_new) / vt;
            v_new = arg > 0 ? v_old - vt * math_log_approx(arg) : -vcrit;
        }
        else
        {
            v_new = -vt * math_log_approx(-v_new / vt);
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
    double c0__X1_X2_D1_t27;
    double c__X1_X2_D1_t27[8];
    
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
        const auto _X1_X2_D1_t25 = (gCout * gCsc);
        const auto _X1_X2_D1_t68 = (gRp2 + gCout);
        const auto _X1_X2_D1_t78 = (gCout + gRL);
        const auto _X1_X2_D1_t89 = (gCc12 + gRg2);
        const auto _X1_X2_D1_t90 = (gCsc * gCsc);
        const auto _X1_X2_D1_t180 = (gRp1 + gCc12);
        const auto _X1_X2_D1_t181 = (gRk2Ck2 * _X1_X2_D1_t25);
        const auto _X1_X2_D1_t16 = (_X1_X2_D1_t78 + gCsc);
        const auto _X1_X2_D1_t37 = (_X1_X2_D1_t16 * gCsc);
        const auto _X1_X2_D1_t102 = (gCout * _X1_X2_D1_t181);
        const auto _X1_X2_D1_t103 = (_X1_X2_D1_t37 - _X1_X2_D1_t90);
        const auto _X1_X2_D1_t2 = (gRk2Ck2 * _X1_X2_D1_t103);
        const auto _X1_X2_D1_t122 = (_X1_X2_D1_t68 * _X1_X2_D1_t2);
        const auto _X1_X2_D1_t15 = (_X1_X2_D1_t122 - _X1_X2_D1_t102);
        const auto _X1_X2_D1_t120 = (gRatt * _X1_X2_D1_t15);
        const auto _X1_X2_D1_t13 = (_X1_X2_D1_t89 * _X1_X2_D1_t120);
        const auto _X1_X2_D1_t18 = (gCc12 * _X1_X2_D1_t120);
        const auto _X1_X2_D1_t35 = (gCc12 * _X1_X2_D1_t18);
        const auto _X1_X2_D1_t53 = (_X1_X2_D1_t180 * _X1_X2_D1_t13);
        const auto _X1_X2_D1_t179 = (_X1_X2_D1_t35 - _X1_X2_D1_t53);
        const auto _X1_X2_D1_t27 = (zRrelCenv * _X1_X2_D1_t179);
        if (_k == 0) {
            c0__X1_X2_D1_t27 = _X1_X2_D1_t27;
        } else {
            c__X1_X2_D1_t27[_k - 1] = _X1_X2_D1_t27 - c0__X1_X2_D1_t27;
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
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            // --- Newton-Raphson solve: X1_X2_D1
            const auto _X1_X2_D1_t27 = c0__X1_X2_D1_t27 + c__X1_X2_D1_t27[0] * vi + c__X1_X2_D1_t27[1] * zCin + c__X1_X2_D1_t27[2] * zRk1Ck1 + c__X1_X2_D1_t27[3] * zCc12 + c__X1_X2_D1_t27[4] * zRrelCenv + c__X1_X2_D1_t27[5] * zCout + c__X1_X2_D1_t27[6] * zCsc + c__X1_X2_D1_t27[7] * zRk2Ck2;
            const auto _X1_X2_D1_t25 = (gCout * gCsc);
            const auto _X1_X2_D1_t64 = (_12AX7_DEMPWOLF_Xi - 1.0);
            const auto _X1_X2_D1_t68 = (gRp2 + gCout);
            const auto _X1_X2_D1_t78 = (gCout + gRL);
            const auto _X1_X2_D1_t88 = (gRk1Ck1 + gRfb);
            const auto _X1_X2_D1_t89 = (gCc12 + gRg2);
            const auto _X1_X2_D1_t90 = (gCsc * gCsc);
            const auto _X1_X2_D1_t116 = (gRfb + gRatt);
            const auto _X1_X2_D1_t133 = (_12AX7_DEMPWOLF_Gamma - 1.0);
            const auto _X1_X2_D1_t174 = (1.0 / (gCin + gRg1));
            const auto _X1_X2_D1_t180 = (gRp1 + gCc12);
            const auto _X1_X2_D1_t181 = (gRk2Ck2 * _X1_X2_D1_t25);
            const auto _X1_X2_D1_t287 = (gRp2 * _X1_X2_D1_t181);
            const auto _X1_X2_D1_t316 = (zCout - zCsc);
            const auto _X1_X2_D1_t352 = (gRfb * gRatt);
            const auto _X1_X2_D1_t375 = (_12AX7_DEMPWOLF_Gk * _12AX7_DEMPWOLF_Gamma);
            const auto _X1_X2_D1_t16 = (_X1_X2_D1_t78 + gCsc);
            const auto _X1_X2_D1_t36 = (_X1_X2_D1_t116 + gRrelCenv);
            const auto _X1_X2_D1_t37 = (_X1_X2_D1_t16 * gCsc);
            const auto _X1_X2_D1_t102 = (gCout * _X1_X2_D1_t181);
            const auto _X1_X2_D1_t103 = (_X1_X2_D1_t37 - _X1_X2_D1_t90);
            const auto _X1_X2_D1_t286 = (gRatt * _X1_X2_D1_t287);
            const auto _X1_X2_D1_t315 = (_X1_X2_D1_t316 * gCsc);
            const auto _X1_X2_D1_t2 = (gRk2Ck2 * _X1_X2_D1_t103);
            const auto _X1_X2_D1_t122 = (_X1_X2_D1_t68 * _X1_X2_D1_t2);
            const auto _X1_X2_D1_t275 = (gCc12 * _X1_X2_D1_t286);
            const auto _X1_X2_D1_t285 = (_X1_X2_D1_t89 * _X1_X2_D1_t286);
            const auto _X1_X2_D1_t295 = (gCc12 * _X1_X2_D1_t275);
            const auto _X1_X2_D1_t311 = (gRp2 * _X1_X2_D1_t2);
            const auto _X1_X2_D1_t312 = (gRatt * _X1_X2_D1_t311);
            const auto _X1_X2_D1_t327 = (gCc12 * _X1_X2_D1_t312);
            const auto _X1_X2_D1_t328 = (_X1_X2_D1_t89 * _X1_X2_D1_t312);
            const auto _X1_X2_D1_t15 = (_X1_X2_D1_t122 - _X1_X2_D1_t102);
            const auto _X1_X2_D1_t120 = (gRatt * _X1_X2_D1_t15);
            const auto _X1_X2_D1_t274 = (_X1_X2_D1_t180 * _X1_X2_D1_t285);
            const auto _X1_X2_D1_t284 = (_X1_X2_D1_t274 - _X1_X2_D1_t295);
            const auto _X1_X2_D1_t322 = (gCc12 * _X1_X2_D1_t327);
            const auto _X1_X2_D1_t323 = (_X1_X2_D1_t180 * _X1_X2_D1_t328);
            const auto _X1_X2_D1_t13 = (_X1_X2_D1_t89 * _X1_X2_D1_t120);
            const auto _X1_X2_D1_t18 = (gCc12 * _X1_X2_D1_t120);
            const auto _X1_X2_D1_t35 = (gCc12 * _X1_X2_D1_t18);
            const auto _X1_X2_D1_t53 = (_X1_X2_D1_t180 * _X1_X2_D1_t13);
            const auto _X1_X2_D1_t179 = (_X1_X2_D1_t35 - _X1_X2_D1_t53);
            const auto _X1_X2_D1_t252 = (gRp1 * _X1_X2_D1_t18);
            const auto _X1_X2_D1_t273 = (VCC * _X1_X2_D1_t284);
            const auto _X1_X2_D1_t310 = (_X1_X2_D1_t322 - _X1_X2_D1_t323);
            const auto _X1_X2_D1_t321 = (VCC * _X1_X2_D1_t310);
            const auto _X1_X2_D1_t4 = (gRp1 * _X1_X2_D1_t13);
            const auto _X1_X2_D1_t14 = (VCC * _X1_X2_D1_t4);
            const auto _X1_X2_D1_t17 = (gRfb * _X1_X2_D1_t179);
            const auto _X1_X2_D1_t77 = (gRatt * _X1_X2_D1_t179);
            const auto _X1_X2_D1_t115 = (_X1_X2_D1_t36 * _X1_X2_D1_t179);
            const auto _X1_X2_D1_t121 = (gRfb * _X1_X2_D1_t17);
            const auto _X1_X2_D1_t178 = (_X1_X2_D1_t77 - _X1_X2_D1_t115);
            const auto _X1_X2_D1_t251 = (VCC * _X1_X2_D1_t252);
            const auto _X1_X2_D1_t101 = (_X1_X2_D1_t88 * _X1_X2_D1_t178);
            const auto _X1_X2_D1_t114 = (1.0 / (_X1_X2_D1_t101 + _X1_X2_D1_t121));
            const auto _X1_X2_D1_t423 = ((gCin * vi) - zCin);
            const auto _X1_X2_D1_t424 = (_12AX7_DEMPWOLF_Gg * _12AX7_DEMPWOLF_Xi);
            for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
            {
                const auto _X1_X2_D1_t12 = (vPKX1 / _12AX7_DEMPWOLF_Mu);
                const auto _X1_X2_D1_t21 = (_12AX7_DEMPWOLF_Cg * vGKX1);
                const auto _X1_X2_D1_t69 = (_12AX7_DEMPWOLF_Cg * vGKX2);
                const auto _X1_X2_D1_t71 = (vPKX2 / _12AX7_DEMPWOLF_Mu);
                const auto _X1_X2_D1_t85 = (_X1_X2_D1_t12 + vGKX1);
                const auto _X1_X2_D1_t175 = (_12AX7_DEMPWOLF_Ck * _X1_X2_D1_t85);
                const auto _X1_X2_D1_t177 = math_exp_approx(_X1_X2_D1_t21);
                const auto _X1_X2_D1_t212 = (vD1 / D1N914_vt);
                const auto _X1_X2_D1_t20 = math_exp_approx(_X1_X2_D1_t175);
                const auto _X1_X2_D1_t24 = math_exp_approx(_X1_X2_D1_t212);
                const auto _X1_X2_D1_t34 = (D1N914_Is * _X1_X2_D1_t24);
                const auto _X1_X2_D1_t45 = (_X1_X2_D1_t71 + vGKX2);
                const auto _X1_X2_D1_t52 = math_exp_approx(_X1_X2_D1_t69);
                const auto _X1_X2_D1_t73 = (_12AX7_DEMPWOLF_Ck * _X1_X2_D1_t45);
                const auto _X1_X2_D1_t83 = (1.0 + _X1_X2_D1_t20);
                const auto _X1_X2_D1_t87 = (1.0 + _X1_X2_D1_t177);
                const auto _X1_X2_D1_t118 = math_log_approx(_X1_X2_D1_t87);
                const auto _X1_X2_D1_t169 = (1.0 + _X1_X2_D1_t52);
                const auto _X1_X2_D1_t192 = (_X1_X2_D1_t16 * _X1_X2_D1_t34);
                const auto _X1_X2_D1_t199 = (gCout * _X1_X2_D1_t34);
                const auto _X1_X2_D1_t208 = (gCsc * _X1_X2_D1_t34);
                const auto _X1_X2_D1_t211 = (gRk2Ck2 * _X1_X2_D1_t208);
                const auto _X1_X2_D1_t217 = (gCout * _X1_X2_D1_t211);
                const auto _X1_X2_D1_t290 = (_X1_X2_D1_t24 - 1.0);
                const auto _X1_X2_D1_t360 = (_12AX7_DEMPWOLF_Mu * _X1_X2_D1_t83);
                const auto _X1_X2_D1_t5 = math_log_approx(_X1_X2_D1_t83);
                const auto _X1_X2_D1_t7 = math_log_approx(_X1_X2_D1_t169);
                const auto _X1_X2_D1_t46 = (_X1_X2_D1_t7 / _12AX7_DEMPWOLF_Cg);
                const auto _X1_X2_D1_t50 = math_exp_approx(_X1_X2_D1_t73);
                const auto _X1_X2_D1_t82 = (_X1_X2_D1_t5 / _12AX7_DEMPWOLF_Ck);
                const auto _X1_X2_D1_t86 = (_X1_X2_D1_t118 / _12AX7_DEMPWOLF_Cg);
                const auto _X1_X2_D1_t158 = math_pow_approx(_X1_X2_D1_t46, _X1_X2_D1_t64);
                const auto _X1_X2_D1_t186 = (gRk2Ck2 * _X1_X2_D1_t192);
                const auto _X1_X2_D1_t191 = (_X1_X2_D1_t68 * _X1_X2_D1_t186);
                const auto _X1_X2_D1_t196 = (gRk2Ck2 * _X1_X2_D1_t199);
                const auto _X1_X2_D1_t207 = (gRatt * _X1_X2_D1_t217);
                const auto _X1_X2_D1_t210 = (_X1_X2_D1_t89 * _X1_X2_D1_t207);
                const auto _X1_X2_D1_t216 = (gCc12 * _X1_X2_D1_t207);
                const auto _X1_X2_D1_t228 = math_pow_approx(_X1_X2_D1_t86, _X1_X2_D1_t64);
                const auto _X1_X2_D1_t231 = math_pow_approx(_X1_X2_D1_t86, _12AX7_DEMPWOLF_Xi);
                const auto _X1_X2_D1_t246 = math_pow_approx(_X1_X2_D1_t82, _12AX7_DEMPWOLF_Gamma);
                const auto _X1_X2_D1_t259 = math_pow_approx(_X1_X2_D1_t46, _12AX7_DEMPWOLF_Xi);
                const auto _X1_X2_D1_t262 = (_12AX7_DEMPWOLF_Gg * _X1_X2_D1_t259);
                const auto _X1_X2_D1_t263 = (_X1_X2_D1_t262 + _12AX7_DEMPWOLF_Ig0);
                const auto _X1_X2_D1_t281 = (D1N914_Is * _X1_X2_D1_t290);
                const auto _X1_X2_D1_t293 = (zCsc + _X1_X2_D1_t281);
                const auto _X1_X2_D1_t301 = (_12AX7_DEMPWOLF_Gg * _X1_X2_D1_t231);
                const auto _X1_X2_D1_t317 = (gCsc * _X1_X2_D1_t293);
                const auto _X1_X2_D1_t370 = math_pow_approx(_X1_X2_D1_t82, _X1_X2_D1_t133);
                const auto _X1_X2_D1_t374 = (_X1_X2_D1_t375 * _X1_X2_D1_t370);
                const auto _X1_X2_D1_t6 = (zCc12 + _X1_X2_D1_t263);
                const auto _X1_X2_D1_t11 = (_12AX7_DEMPWOLF_Gk * _X1_X2_D1_t246);
                const auto _X1_X2_D1_t33 = (_X1_X2_D1_t34 * _X1_X2_D1_t15);
                const auto _X1_X2_D1_t41 = (1.0 + _X1_X2_D1_t50);
                const auto _X1_X2_D1_t76 = (_X1_X2_D1_t158 * _X1_X2_D1_t52);
                const auto _X1_X2_D1_t81 = (_X1_X2_D1_t370 * _X1_X2_D1_t20);
                const auto _X1_X2_D1_t104 = math_log_approx(_X1_X2_D1_t41);
                const auto _X1_X2_D1_t111 = (_X1_X2_D1_t104 / _12AX7_DEMPWOLF_Ck);
                const auto _X1_X2_D1_t141 = (_12AX7_DEMPWOLF_Mu * _X1_X2_D1_t41);
                const auto _X1_X2_D1_t148 = math_pow_approx(_X1_X2_D1_t111, _X1_X2_D1_t133);
                const auto _X1_X2_D1_t161 = (_X1_X2_D1_t148 * _X1_X2_D1_t50);
                const auto _X1_X2_D1_t185 = (gCout * _X1_X2_D1_t196);
                const auto _X1_X2_D1_t188 = (_X1_X2_D1_t191 / D1N914_vt);
                const auto _X1_X2_D1_t195 = (_X1_X2_D1_t185 / D1N914_vt);
                const auto _X1_X2_D1_t198 = (_X1_X2_D1_t195 - _X1_X2_D1_t188);
                const auto _X1_X2_D1_t202 = (gCc12 * _X1_X2_D1_t33);
                const auto _X1_X2_D1_t209 = (_X1_X2_D1_t180 * _X1_X2_D1_t210);
                const auto _X1_X2_D1_t214 = (gCc12 * _X1_X2_D1_t216);
                const auto _X1_X2_D1_t215 = (_X1_X2_D1_t209 / D1N914_vt);
                const auto _X1_X2_D1_t220 = (_X1_X2_D1_t228 * _X1_X2_D1_t177);
                const auto _X1_X2_D1_t238 = (_X1_X2_D1_t281 * _X1_X2_D1_t15);
                const auto _X1_X2_D1_t245 = (_X1_X2_D1_t6 * _X1_X2_D1_t120);
                const auto _X1_X2_D1_t249 = (_X1_X2_D1_t180 * _X1_X2_D1_t245);
                const auto _X1_X2_D1_t269 = math_pow_approx(_X1_X2_D1_t111, _12AX7_DEMPWOLF_Gamma);
                const auto _X1_X2_D1_t271 = (gCout * _X1_X2_D1_t293);
                const auto _X1_X2_D1_t283 = (_X1_X2_D1_t16 * _X1_X2_D1_t293);
                const auto _X1_X2_D1_t289 = (gRk2Ck2 * _X1_X2_D1_t271);
                const auto _X1_X2_D1_t300 = (_X1_X2_D1_t301 + _12AX7_DEMPWOLF_Ig0);
                const auto _X1_X2_D1_t309 = (_X1_X2_D1_t315 + _X1_X2_D1_t317);
                const auto _X1_X2_D1_t326 = (gRk2Ck2 * _X1_X2_D1_t309);
                const auto _X1_X2_D1_t340 = (_12AX7_DEMPWOLF_Gk * _X1_X2_D1_t269);
                const auto _X1_X2_D1_t369 = (_12AX7_DEMPWOLF_Gamma * _X1_X2_D1_t81);
                const auto _X1_X2_D1_t373 = (_X1_X2_D1_t374 * _X1_X2_D1_t20);
                const auto _X1_X2_D1_t3 = (gCc12 * _X1_X2_D1_t245);
                const auto _X1_X2_D1_t10 = (_X1_X2_D1_t11 - _X1_X2_D1_t300);
                const auto _X1_X2_D1_t23 = (_X1_X2_D1_t89 * _X1_X2_D1_t238);
                const auto _X1_X2_D1_t26 = (gCc12 * _X1_X2_D1_t238);
                const auto _X1_X2_D1_t32 = (_X1_X2_D1_t89 * _X1_X2_D1_t33);
                const auto _X1_X2_D1_t61 = (_12AX7_DEMPWOLF_Xi * _X1_X2_D1_t76);
                const auto _X1_X2_D1_t74 = (_12AX7_DEMPWOLF_Gg * _X1_X2_D1_t61);
                const auto _X1_X2_D1_t119 = (_12AX7_DEMPWOLF_Xi * _X1_X2_D1_t220);
                const auto _X1_X2_D1_t153 = (_12AX7_DEMPWOLF_Gamma * _X1_X2_D1_t161);
                const auto _X1_X2_D1_t162 = (_X1_X2_D1_t74 / _X1_X2_D1_t169);
                const auto _X1_X2_D1_t165 = (_X1_X2_D1_t74 * _X1_X2_D1_t120);
                const auto _X1_X2_D1_t176 = (_12AX7_DEMPWOLF_Gg * _X1_X2_D1_t119);
                const auto _X1_X2_D1_t190 = (gRatt * _X1_X2_D1_t198);
                const auto _X1_X2_D1_t201 = (gCc12 * _X1_X2_D1_t202);
                const auto _X1_X2_D1_t213 = (_X1_X2_D1_t214 / D1N914_vt);
                const auto _X1_X2_D1_t227 = (_X1_X2_D1_t176 / _X1_X2_D1_t87);
                const auto _X1_X2_D1_t237 = (gCc12 * _X1_X2_D1_t26);
                const auto _X1_X2_D1_t247 = (_X1_X2_D1_t10 - zCc12);
                const auto _X1_X2_D1_t250 = (_X1_X2_D1_t247 * _X1_X2_D1_t18);
                const auto _X1_X2_D1_t258 = (_X1_X2_D1_t340 - _X1_X2_D1_t263);
                const auto _X1_X2_D1_t282 = (_X1_X2_D1_t283 + _X1_X2_D1_t315);
                const auto _X1_X2_D1_t288 = (gCout * _X1_X2_D1_t289);
                const auto _X1_X2_D1_t297 = (_X1_X2_D1_t180 * _X1_X2_D1_t23);
                const auto _X1_X2_D1_t299 = (zRk1Ck1 + _X1_X2_D1_t10);
                const auto _X1_X2_D1_t306 = (gCout * _X1_X2_D1_t326);
                const auto _X1_X2_D1_t308 = (_X1_X2_D1_t258 - zCout);
                const auto _X1_X2_D1_t346 = (gCc12 * _X1_X2_D1_t165);
                const auto _X1_X2_D1_t354 = (_X1_X2_D1_t180 * _X1_X2_D1_t32);
                const auto _X1_X2_D1_t355 = (_X1_X2_D1_t201 / D1N914_vt);
                const auto _X1_X2_D1_t368 = (_12AX7_DEMPWOLF_Gk * _X1_X2_D1_t369);
                const auto _X1_X2_D1_t371 = (_X1_X2_D1_t368 * _X1_X2_D1_t18);
                const auto _X1_X2_D1_t9 = (_X1_X2_D1_t247 * _X1_X2_D1_t13);
                const auto _X1_X2_D1_t70 = (_12AX7_DEMPWOLF_Gk * _X1_X2_D1_t153);
                const auto _X1_X2_D1_t100 = (_X1_X2_D1_t70 * _X1_X2_D1_t181);
                const auto _X1_X2_D1_t108 = (_X1_X2_D1_t70 * _X1_X2_D1_t25);
                const auto _X1_X2_D1_t129 = (_X1_X2_D1_t70 * _X1_X2_D1_t2);
                const auto _X1_X2_D1_t140 = (_X1_X2_D1_t70 * _X1_X2_D1_t103);
                const auto _X1_X2_D1_t143 = (gCout * _X1_X2_D1_t108);
                const auto _X1_X2_D1_t146 = (_X1_X2_D1_t143 / _X1_X2_D1_t141);
                const auto _X1_X2_D1_t154 = (_X1_X2_D1_t70 / _X1_X2_D1_t41);
                const auto _X1_X2_D1_t164 = (_X1_X2_D1_t180 * _X1_X2_D1_t165);
                const auto _X1_X2_D1_t187 = (_X1_X2_D1_t89 * _X1_X2_D1_t190);
                const auto _X1_X2_D1_t189 = (gCc12 * _X1_X2_D1_t190);
                const auto _X1_X2_D1_t194 = (gCc12 * _X1_X2_D1_t189);
                const auto _X1_X2_D1_t197 = (_X1_X2_D1_t180 * _X1_X2_D1_t187);
                const auto _X1_X2_D1_t203 = (_X1_X2_D1_t354 / D1N914_vt);
                const auto _X1_X2_D1_t206 = (_X1_X2_D1_t215 - _X1_X2_D1_t213);
                const auto _X1_X2_D1_t226 = (_X1_X2_D1_t368 / _X1_X2_D1_t83);
                const auto _X1_X2_D1_t236 = (_X1_X2_D1_t297 - _X1_X2_D1_t237);
                const auto _X1_X2_D1_t248 = (_X1_X2_D1_t249 + _X1_X2_D1_t250);
                const auto _X1_X2_D1_t256 = (zRk2Ck2 + _X1_X2_D1_t258);
                const auto _X1_X2_D1_t272 = (gRk2Ck2 * _X1_X2_D1_t282);
                const auto _X1_X2_D1_t291 = (_X1_X2_D1_t308 * _X1_X2_D1_t181);
                const auto _X1_X2_D1_t294 = (_X1_X2_D1_t68 * _X1_X2_D1_t272);
                const auto _X1_X2_D1_t296 = (_X1_X2_D1_t237 - _X1_X2_D1_t297);
                const auto _X1_X2_D1_t298 = (_X1_X2_D1_t299 + _X1_X2_D1_t300);
                const auto _X1_X2_D1_t307 = (_X1_X2_D1_t308 * _X1_X2_D1_t2);
                const auto _X1_X2_D1_t320 = (_X1_X2_D1_t307 + _X1_X2_D1_t306);
                const auto _X1_X2_D1_t325 = (gRatt * _X1_X2_D1_t320);
                const auto _X1_X2_D1_t337 = (_X1_X2_D1_t256 + _X1_X2_D1_t263);
                const auto _X1_X2_D1_t353 = (_X1_X2_D1_t203 - _X1_X2_D1_t355);
                const auto _X1_X2_D1_t359 = (_X1_X2_D1_t368 * _X1_X2_D1_t13);
                const auto _X1_X2_D1_t363 = (_X1_X2_D1_t373 * _X1_X2_D1_t178);
                const auto _X1_X2_D1_t1 = (_X1_X2_D1_t9 + _X1_X2_D1_t3);
                const auto _X1_X2_D1_t8 = (_X1_X2_D1_t1 - _X1_X2_D1_t14);
                const auto _X1_X2_D1_t22 = (gRatt * _X1_X2_D1_t236);
                const auto _X1_X2_D1_t31 = (_X1_X2_D1_t352 * _X1_X2_D1_t353);
                const auto _X1_X2_D1_t44 = (_X1_X2_D1_t154 - _X1_X2_D1_t162);
                const auto _X1_X2_D1_t62 = (_X1_X2_D1_t44 + _X1_X2_D1_t162);
                const auto _X1_X2_D1_t63 = (_X1_X2_D1_t62 * _X1_X2_D1_t25);
                const auto _X1_X2_D1_t75 = (gCout * _X1_X2_D1_t63);
                const auto _X1_X2_D1_t96 = (gRatt * _X1_X2_D1_t100);
                const auto _X1_X2_D1_t98 = (gCc12 * _X1_X2_D1_t96);
                const auto _X1_X2_D1_t107 = (_X1_X2_D1_t68 * _X1_X2_D1_t140);
                const auto _X1_X2_D1_t126 = (gRatt * _X1_X2_D1_t129);
                const auto _X1_X2_D1_t132 = (_X1_X2_D1_t89 * _X1_X2_D1_t126);
                const auto _X1_X2_D1_t160 = (_X1_X2_D1_t44 * _X1_X2_D1_t181);
                const auto _X1_X2_D1_t193 = (_X1_X2_D1_t194 - _X1_X2_D1_t197);
                const auto _X1_X2_D1_t200 = (_X1_X2_D1_t355 - _X1_X2_D1_t203);
                const auto _X1_X2_D1_t205 = (((_X1_X2_D1_t88 * ((gRatt * _X1_X2_D1_t206) - (_X1_X2_D1_t36 * _X1_X2_D1_t206))) + (gRfb * (gRfb * _X1_X2_D1_t206))) * _X1_X2_D1_t114);
                const auto _X1_X2_D1_t225 = (_X1_X2_D1_t226 - _X1_X2_D1_t227);
                const auto _X1_X2_D1_t233 = (_X1_X2_D1_t298 * _X1_X2_D1_t178);
                const auto _X1_X2_D1_t235 = (_X1_X2_D1_t22 - _X1_X2_D1_t27);
                const auto _X1_X2_D1_t244 = (_X1_X2_D1_t248 - _X1_X2_D1_t251);
                const auto _X1_X2_D1_t255 = (_X1_X2_D1_t337 * _X1_X2_D1_t103);
                const auto _X1_X2_D1_t257 = (_X1_X2_D1_t337 * _X1_X2_D1_t25);
                const auto _X1_X2_D1_t268 = (_X1_X2_D1_t288 - _X1_X2_D1_t294);
                const auto _X1_X2_D1_t280 = (_X1_X2_D1_t268 - _X1_X2_D1_t291);
                const auto _X1_X2_D1_t314 = (_X1_X2_D1_t89 * _X1_X2_D1_t325);
                const auto _X1_X2_D1_t319 = (gCc12 * _X1_X2_D1_t325);
                const auto _X1_X2_D1_t324 = (_X1_X2_D1_t180 * _X1_X2_D1_t314);
                const auto _X1_X2_D1_t336 = (_X1_X2_D1_t68 * _X1_X2_D1_t255);
                const auto _X1_X2_D1_t338 = (gCout * _X1_X2_D1_t257);
                const auto _X1_X2_D1_t342 = (_X1_X2_D1_t336 - _X1_X2_D1_t338);
                const auto _X1_X2_D1_t345 = (((_X1_X2_D1_t88 * (((gRatt * _X1_X2_D1_t346) / _X1_X2_D1_t169) - ((_X1_X2_D1_t36 * _X1_X2_D1_t346) / _X1_X2_D1_t169))) + ((gRfb * (gRfb * _X1_X2_D1_t346)) / _X1_X2_D1_t169)) * _X1_X2_D1_t114);
                const auto _X1_X2_D1_t362 = (_X1_X2_D1_t363 / _X1_X2_D1_t360);
                const auto _X1_X2_D1_t367 = (((_X1_X2_D1_t88 * (((gRatt * _X1_X2_D1_t371) / _X1_X2_D1_t360) - ((_X1_X2_D1_t36 * _X1_X2_D1_t371) / _X1_X2_D1_t360))) + ((gRfb * (gRfb * _X1_X2_D1_t371)) / _X1_X2_D1_t360)) * _X1_X2_D1_t114);
                const auto _X1_X2_D1_t372 = ((((_X1_X2_D1_t373 * gRfb) * _X1_X2_D1_t179) / _X1_X2_D1_t360) * _X1_X2_D1_t114);
                const auto _X1_X2_D1_t30 = (_X1_X2_D1_t31 * _X1_X2_D1_t114);
                const auto _X1_X2_D1_t43 = (_X1_X2_D1_t44 * _X1_X2_D1_t2);
                const auto _X1_X2_D1_t48 = (gRatt * _X1_X2_D1_t43);
                const auto _X1_X2_D1_t51 = (gCc12 * _X1_X2_D1_t48);
                const auto _X1_X2_D1_t59 = (_X1_X2_D1_t62 * _X1_X2_D1_t103);
                const auto _X1_X2_D1_t84 = (_X1_X2_D1_t225 * _X1_X2_D1_t18);
                const auto _X1_X2_D1_t93 = (gCc12 * _X1_X2_D1_t98);
                const auto _X1_X2_D1_t95 = (_X1_X2_D1_t89 * _X1_X2_D1_t96);
                const auto _X1_X2_D1_t97 = (_X1_X2_D1_t93 / _X1_X2_D1_t141);
                const auto _X1_X2_D1_t99 = (_X1_X2_D1_t180 * _X1_X2_D1_t95);
                const auto _X1_X2_D1_t105 = (_X1_X2_D1_t107 / _X1_X2_D1_t141);
                const auto _X1_X2_D1_t113 = (_X1_X2_D1_t105 - _X1_X2_D1_t146);
                const auto _X1_X2_D1_t125 = (gCc12 * _X1_X2_D1_t126);
                const auto _X1_X2_D1_t131 = (_X1_X2_D1_t180 * _X1_X2_D1_t132);
                const auto _X1_X2_D1_t145 = (gRatt * _X1_X2_D1_t113);
                const auto _X1_X2_D1_t152 = (gRatt * _X1_X2_D1_t160);
                const auto _X1_X2_D1_t155 = (_X1_X2_D1_t89 * _X1_X2_D1_t152);
                const auto _X1_X2_D1_t157 = (gCc12 * _X1_X2_D1_t152);
                const auto _X1_X2_D1_t184 = (((((_X1_X2_D1_t88 * ((gRatt * _X1_X2_D1_t193) - (_X1_X2_D1_t36 * _X1_X2_D1_t193))) + (gRfb * (gRfb * _X1_X2_D1_t193))) * _X1_X2_D1_t114) + (((_X1_X2_D1_t88 * (_X1_X2_D1_t36 * _X1_X2_D1_t200)) - (gRfb * (gRfb * _X1_X2_D1_t200))) * _X1_X2_D1_t114)) - 1.0);
                const auto _X1_X2_D1_t221 = (_X1_X2_D1_t225 * _X1_X2_D1_t13);
                const auto _X1_X2_D1_t224 = (_X1_X2_D1_t225 + _X1_X2_D1_t227);
                const auto _X1_X2_D1_t234 = (gRfb * _X1_X2_D1_t235);
                const auto _X1_X2_D1_t279 = (gRatt * _X1_X2_D1_t280);
                const auto _X1_X2_D1_t292 = (_X1_X2_D1_t89 * _X1_X2_D1_t279);
                const auto _X1_X2_D1_t318 = (gCc12 * _X1_X2_D1_t319);
                const auto _X1_X2_D1_t341 = (gRatt * _X1_X2_D1_t342);
                const auto _X1_X2_D1_t361 = (_X1_X2_D1_t362 * _X1_X2_D1_t114);
                const auto _X1_X2_D1_t379 = (_X1_X2_D1_t367 * _X1_X2_D1_t184);
                const auto _X1_X2_D1_t40 = (_X1_X2_D1_t89 * _X1_X2_D1_t48);
                const auto _X1_X2_D1_t42 = (gCc12 * _X1_X2_D1_t51);
                const auto _X1_X2_D1_t49 = (_X1_X2_D1_t180 * _X1_X2_D1_t40);
                const auto _X1_X2_D1_t58 = (_X1_X2_D1_t68 * _X1_X2_D1_t59);
                const auto _X1_X2_D1_t80 = (((_X1_X2_D1_t88 * ((gRatt * _X1_X2_D1_t84) - (_X1_X2_D1_t36 * _X1_X2_D1_t84))) + (gRfb * (gRfb * _X1_X2_D1_t84))) * _X1_X2_D1_t114);
                const auto _X1_X2_D1_t94 = (_X1_X2_D1_t99 / _X1_X2_D1_t141);
                const auto _X1_X2_D1_t110 = (gCc12 * _X1_X2_D1_t145);
                const auto _X1_X2_D1_t112 = (_X1_X2_D1_t89 * _X1_X2_D1_t145);
                const auto _X1_X2_D1_t117 = (((_X1_X2_D1_t224 * gRfb) * _X1_X2_D1_t179) * _X1_X2_D1_t114);
                const auto _X1_X2_D1_t124 = (gCc12 * _X1_X2_D1_t125);
                const auto _X1_X2_D1_t128 = (_X1_X2_D1_t131 / _X1_X2_D1_t141);
                const auto _X1_X2_D1_t130 = (_X1_X2_D1_t124 / _X1_X2_D1_t141);
                const auto _X1_X2_D1_t144 = (_X1_X2_D1_t180 * _X1_X2_D1_t112);
                const auto _X1_X2_D1_t151 = (_X1_X2_D1_t180 * _X1_X2_D1_t155);
                const auto _X1_X2_D1_t156 = (gCc12 * _X1_X2_D1_t157);
                const auto _X1_X2_D1_t159 = (_X1_X2_D1_t151 - _X1_X2_D1_t156);
                const auto _X1_X2_D1_t172 = (_X1_X2_D1_t58 - _X1_X2_D1_t75);
                const auto _X1_X2_D1_t223 = (_X1_X2_D1_t224 * _X1_X2_D1_t178);
                const auto _X1_X2_D1_t232 = (_X1_X2_D1_t233 + _X1_X2_D1_t234);
                const auto _X1_X2_D1_t267 = (gCc12 * _X1_X2_D1_t279);
                const auto _X1_X2_D1_t270 = (_X1_X2_D1_t180 * _X1_X2_D1_t292);
                const auto _X1_X2_D1_t278 = (gCc12 * _X1_X2_D1_t267);
                const auto _X1_X2_D1_t305 = (_X1_X2_D1_t324 - _X1_X2_D1_t318);
                const auto _X1_X2_D1_t313 = (_X1_X2_D1_t305 + _X1_X2_D1_t321);
                const auto _X1_X2_D1_t334 = (gCc12 * _X1_X2_D1_t341);
                const auto _X1_X2_D1_t335 = (_X1_X2_D1_t89 * _X1_X2_D1_t341);
                const auto _X1_X2_D1_t349 = (_X1_X2_D1_t80 * _X1_X2_D1_t184);
                const auto _X1_X2_D1_t358 = (((((_X1_X2_D1_t88 * (((gRatt * _X1_X2_D1_t359) / _X1_X2_D1_t360) - ((_X1_X2_D1_t36 * _X1_X2_D1_t359) / _X1_X2_D1_t360))) + ((gRfb * (gRfb * _X1_X2_D1_t359)) / _X1_X2_D1_t360)) * _X1_X2_D1_t114) - _X1_X2_D1_t361) - 1.0);
                const auto _X1_X2_D1_t381 = ((_X1_X2_D1_t367 * _X1_X2_D1_t117) - (_X1_X2_D1_t80 * _X1_X2_D1_t372));
                const auto _X1_X2_D1_t19 = (_X1_X2_D1_t232 * _X1_X2_D1_t114);
                const auto _X1_X2_D1_t47 = (_X1_X2_D1_t49 - _X1_X2_D1_t42);
                const auto _X1_X2_D1_t60 = (gRatt * _X1_X2_D1_t172);
                const auto _X1_X2_D1_t92 = (_X1_X2_D1_t94 - _X1_X2_D1_t97);
                const auto _X1_X2_D1_t106 = (gCc12 * _X1_X2_D1_t110);
                const auto _X1_X2_D1_t127 = (_X1_X2_D1_t128 - _X1_X2_D1_t130);
                const auto _X1_X2_D1_t139 = (_X1_X2_D1_t106 - _X1_X2_D1_t144);
                const auto _X1_X2_D1_t142 = (_X1_X2_D1_t36 * _X1_X2_D1_t139);
                const auto _X1_X2_D1_t150 = (((_X1_X2_D1_t88 * ((gRatt * _X1_X2_D1_t159) - (_X1_X2_D1_t36 * _X1_X2_D1_t159))) + (gRfb * (gRfb * _X1_X2_D1_t159))) * _X1_X2_D1_t114);
                const auto _X1_X2_D1_t167 = (gCc12 * _X1_X2_D1_t60);
                const auto _X1_X2_D1_t170 = (_X1_X2_D1_t89 * _X1_X2_D1_t60);
                const auto _X1_X2_D1_t171 = (gCc12 * _X1_X2_D1_t167);
                const auto _X1_X2_D1_t222 = (_X1_X2_D1_t223 * _X1_X2_D1_t114);
                const auto _X1_X2_D1_t230 = (((_X1_X2_D1_t300 - _X1_X2_D1_t423) * _X1_X2_D1_t174) + _X1_X2_D1_t19);
                const auto _X1_X2_D1_t254 = (gCc12 * _X1_X2_D1_t334);
                const auto _X1_X2_D1_t264 = (_X1_X2_D1_t180 * _X1_X2_D1_t335);
                const auto _X1_X2_D1_t277 = (_X1_X2_D1_t278 - _X1_X2_D1_t270);
                const auto _X1_X2_D1_t383 = (_X1_X2_D1_t230 + vGKX1);
                const auto _X1_X2_D1_t0 = ((((_X1_X2_D1_t88 * ((gRatt * _X1_X2_D1_t8) - (_X1_X2_D1_t36 * _X1_X2_D1_t8))) + (gRfb * (gRfb * _X1_X2_D1_t8))) * _X1_X2_D1_t114) - _X1_X2_D1_t19);
                const auto _X1_X2_D1_t28 = (vPKX1 - _X1_X2_D1_t0);
                const auto _X1_X2_D1_t72 = (_X1_X2_D1_t180 * _X1_X2_D1_t170);
                const auto _X1_X2_D1_t91 = (((_X1_X2_D1_t88 * ((gRatt * _X1_X2_D1_t92) - (_X1_X2_D1_t36 * _X1_X2_D1_t92))) + (gRfb * (gRfb * _X1_X2_D1_t92))) * _X1_X2_D1_t114);
                const auto _X1_X2_D1_t109 = (gRfb * _X1_X2_D1_t139);
                const auto _X1_X2_D1_t138 = (gRatt * _X1_X2_D1_t139);
                const auto _X1_X2_D1_t147 = (gRfb * _X1_X2_D1_t109);
                const auto _X1_X2_D1_t173 = ((((((_X1_X2_D1_t424 * _X1_X2_D1_t228) * _X1_X2_D1_t177) / _X1_X2_D1_t87) * _X1_X2_D1_t174) + _X1_X2_D1_t222) + 1.0);
                const auto _X1_X2_D1_t219 = ((((_X1_X2_D1_t88 * ((gRatt * _X1_X2_D1_t221) - (_X1_X2_D1_t36 * _X1_X2_D1_t221))) + (gRfb * (gRfb * _X1_X2_D1_t221))) * _X1_X2_D1_t114) - _X1_X2_D1_t222);
                const auto _X1_X2_D1_t260 = (_X1_X2_D1_t254 - _X1_X2_D1_t264);
                const auto _X1_X2_D1_t276 = (_X1_X2_D1_t277 - _X1_X2_D1_t273);
                const auto _X1_X2_D1_t332 = (gRatt * _X1_X2_D1_t260);
                const auto _X1_X2_D1_t333 = (_X1_X2_D1_t36 * _X1_X2_D1_t260);
                const auto _X1_X2_D1_t339 = (gRfb * _X1_X2_D1_t260);
                const auto _X1_X2_D1_t67 = (_X1_X2_D1_t171 - _X1_X2_D1_t72);
                const auto _X1_X2_D1_t137 = (_X1_X2_D1_t138 - _X1_X2_D1_t142);
                const auto _X1_X2_D1_t168 = (_X1_X2_D1_t36 * _X1_X2_D1_t67);
                const auto _X1_X2_D1_t261 = (gRfb * _X1_X2_D1_t339);
                const auto _X1_X2_D1_t266 = ((((_X1_X2_D1_t88 * ((gRatt * _X1_X2_D1_t276) - (_X1_X2_D1_t36 * _X1_X2_D1_t276))) + (gRfb * (gRfb * _X1_X2_D1_t276))) * _X1_X2_D1_t114) + ((((_X1_X2_D1_t88 * ((_X1_X2_D1_t36 * _X1_X2_D1_t296) + _X1_X2_D1_t27)) - (gRfb * (gRfb * _X1_X2_D1_t296))) + (_X1_X2_D1_t298 * _X1_X2_D1_t17)) * _X1_X2_D1_t114));
                const auto _X1_X2_D1_t331 = (_X1_X2_D1_t332 - _X1_X2_D1_t333);
                const auto _X1_X2_D1_t57 = (gRatt * _X1_X2_D1_t67);
                const auto _X1_X2_D1_t66 = (gRfb * _X1_X2_D1_t67);
                const auto _X1_X2_D1_t136 = (_X1_X2_D1_t88 * _X1_X2_D1_t137);
                const auto _X1_X2_D1_t253 = (_X1_X2_D1_t88 * _X1_X2_D1_t331);
                const auto _X1_X2_D1_t265 = (vD1 - _X1_X2_D1_t266);
                const auto _X1_X2_D1_t330 = (_X1_X2_D1_t253 + _X1_X2_D1_t261);
                const auto _X1_X2_D1_t389 = (_X1_X2_D1_t367 * _X1_X2_D1_t265);
                const auto _X1_X2_D1_t395 = (_X1_X2_D1_t80 * _X1_X2_D1_t265);
                const auto _X1_X2_D1_t56 = (_X1_X2_D1_t57 - _X1_X2_D1_t168);
                const auto _X1_X2_D1_t65 = (gRfb * _X1_X2_D1_t66);
                const auto _X1_X2_D1_t135 = (_X1_X2_D1_t136 + _X1_X2_D1_t147);
                const auto _X1_X2_D1_t166 = (_X1_X2_D1_t88 * _X1_X2_D1_t56);
                const auto _X1_X2_D1_t329 = (_X1_X2_D1_t330 * _X1_X2_D1_t114);
                const auto _X1_X2_D1_t55 = (_X1_X2_D1_t166 + _X1_X2_D1_t65);
                const auto _X1_X2_D1_t134 = (_X1_X2_D1_t135 * _X1_X2_D1_t114);
                const auto _X1_X2_D1_t204 = (_X1_X2_D1_t134 * _X1_X2_D1_t184);
                const auto _X1_X2_D1_t243 = ((((_X1_X2_D1_t88 * ((gRatt * _X1_X2_D1_t244) - (_X1_X2_D1_t36 * _X1_X2_D1_t244))) + (gRfb * (gRfb * _X1_X2_D1_t244))) * _X1_X2_D1_t114) - _X1_X2_D1_t329);
                const auto _X1_X2_D1_t304 = ((((_X1_X2_D1_t88 * ((gRatt * _X1_X2_D1_t313) - (_X1_X2_D1_t36 * _X1_X2_D1_t313))) + (gRfb * (gRfb * _X1_X2_D1_t313))) * _X1_X2_D1_t114) - _X1_X2_D1_t329);
                const auto _X1_X2_D1_t366 = ((_X1_X2_D1_t367 * _X1_X2_D1_t91) + (_X1_X2_D1_t134 * _X1_X2_D1_t372));
                const auto _X1_X2_D1_t387 = (_X1_X2_D1_t134 * _X1_X2_D1_t265);
                const auto _X1_X2_D1_t54 = (_X1_X2_D1_t55 * _X1_X2_D1_t114);
                const auto _X1_X2_D1_t79 = ((_X1_X2_D1_t80 * _X1_X2_D1_t91) + (_X1_X2_D1_t134 * _X1_X2_D1_t117));
                const auto _X1_X2_D1_t123 = (((((_X1_X2_D1_t88 * ((gRatt * _X1_X2_D1_t127) - (_X1_X2_D1_t36 * _X1_X2_D1_t127))) + (gRfb * (gRfb * _X1_X2_D1_t127))) * _X1_X2_D1_t114) - _X1_X2_D1_t134) - 1.0);
                const auto _X1_X2_D1_t163 = (((((_X1_X2_D1_t88 * (((gRatt * _X1_X2_D1_t164) / _X1_X2_D1_t169) - ((_X1_X2_D1_t36 * _X1_X2_D1_t164) / _X1_X2_D1_t169))) + ((gRfb * (gRfb * _X1_X2_D1_t164)) / _X1_X2_D1_t169)) * _X1_X2_D1_t114) - _X1_X2_D1_t54) - 1.0);
                const auto _X1_X2_D1_t183 = (_X1_X2_D1_t163 * _X1_X2_D1_t184);
                const auto _X1_X2_D1_t218 = ((_X1_X2_D1_t134 * _X1_X2_D1_t150) + (_X1_X2_D1_t163 * _X1_X2_D1_t91));
                const auto _X1_X2_D1_t242 = (vGKX2 - _X1_X2_D1_t243);
                const auto _X1_X2_D1_t303 = (vPKX2 - _X1_X2_D1_t304);
                const auto _X1_X2_D1_t344 = (_X1_X2_D1_t163 * _X1_X2_D1_t265);
                const auto _X1_X2_D1_t348 = ((_X1_X2_D1_t205 * _X1_X2_D1_t79) - (_X1_X2_D1_t123 * _X1_X2_D1_t349));
                const auto _X1_X2_D1_t351 = (_X1_X2_D1_t303 * _X1_X2_D1_t79);
                const auto _X1_X2_D1_t376 = ((_X1_X2_D1_t367 * _X1_X2_D1_t150) - (_X1_X2_D1_t163 * _X1_X2_D1_t372));
                const auto _X1_X2_D1_t378 = ((_X1_X2_D1_t205 * _X1_X2_D1_t366) - (_X1_X2_D1_t123 * _X1_X2_D1_t379));
                const auto _X1_X2_D1_t380 = (_X1_X2_D1_t123 * _X1_X2_D1_t381);
                const auto _X1_X2_D1_t386 = (_X1_X2_D1_t242 * _X1_X2_D1_t91);
                const auto _X1_X2_D1_t390 = (_X1_X2_D1_t303 * _X1_X2_D1_t366);
                const auto _X1_X2_D1_t396 = (_X1_X2_D1_t242 * _X1_X2_D1_t117);
                const auto _X1_X2_D1_t405 = (_X1_X2_D1_t242 * _X1_X2_D1_t184);
                const auto _X1_X2_D1_t406 = (_X1_X2_D1_t303 * _X1_X2_D1_t204);
                const auto _X1_X2_D1_t408 = (_X1_X2_D1_t383 * _X1_X2_D1_t378);
                const auto _X1_X2_D1_t412 = (_X1_X2_D1_t242 * _X1_X2_D1_t372);
                const auto _X1_X2_D1_t415 = (_X1_X2_D1_t242 * _X1_X2_D1_t150);
                const auto _X1_X2_D1_t416 = (_X1_X2_D1_t387 + _X1_X2_D1_t386);
                const auto _X1_X2_D1_t422 = (_X1_X2_D1_t383 * _X1_X2_D1_t380);
                const auto _X1_X2_D1_t39 = ((((_X1_X2_D1_t88 * ((gRatt * _X1_X2_D1_t47) - (_X1_X2_D1_t36 * _X1_X2_D1_t47))) + (gRfb * (gRfb * _X1_X2_D1_t47))) * _X1_X2_D1_t114) - _X1_X2_D1_t54);
                const auto _X1_X2_D1_t149 = ((_X1_X2_D1_t80 * _X1_X2_D1_t150) - (_X1_X2_D1_t163 * _X1_X2_D1_t117));
                const auto _X1_X2_D1_t182 = (((_X1_X2_D1_t123 * _X1_X2_D1_t183) + (_X1_X2_D1_t39 * _X1_X2_D1_t204)) - (_X1_X2_D1_t205 * _X1_X2_D1_t218));
                const auto _X1_X2_D1_t229 = (_X1_X2_D1_t383 * _X1_X2_D1_t182);
                const auto _X1_X2_D1_t241 = (_X1_X2_D1_t386 + _X1_X2_D1_t387);
                const auto _X1_X2_D1_t302 = (_X1_X2_D1_t303 * _X1_X2_D1_t218);
                const auto _X1_X2_D1_t347 = (_X1_X2_D1_t383 * _X1_X2_D1_t348);
                const auto _X1_X2_D1_t365 = ((_X1_X2_D1_t39 * _X1_X2_D1_t366) - (_X1_X2_D1_t123 * _X1_X2_D1_t376));
                const auto _X1_X2_D1_t377 = (((_X1_X2_D1_t173 * _X1_X2_D1_t378) - (_X1_X2_D1_t361 * _X1_X2_D1_t348)) + (_X1_X2_D1_t30 * _X1_X2_D1_t380));
                const auto _X1_X2_D1_t392 = (_X1_X2_D1_t205 * _X1_X2_D1_t241);
                const auto _X1_X2_D1_t394 = (_X1_X2_D1_t395 - _X1_X2_D1_t396);
                const auto _X1_X2_D1_t398 = (_X1_X2_D1_t415 - _X1_X2_D1_t344);
                const auto _X1_X2_D1_t399 = ((_X1_X2_D1_t205 * _X1_X2_D1_t149) - (_X1_X2_D1_t39 * _X1_X2_D1_t349));
                const auto _X1_X2_D1_t401 = ((_X1_X2_D1_t205 * _X1_X2_D1_t376) - (_X1_X2_D1_t39 * _X1_X2_D1_t379));
                const auto _X1_X2_D1_t402 = (_X1_X2_D1_t39 * _X1_X2_D1_t381);
                const auto _X1_X2_D1_t404 = (_X1_X2_D1_t123 * _X1_X2_D1_t405);
                const auto _X1_X2_D1_t407 = (_X1_X2_D1_t123 * _X1_X2_D1_t394);
                const auto _X1_X2_D1_t411 = (_X1_X2_D1_t389 - _X1_X2_D1_t412);
                const auto _X1_X2_D1_t414 = (((_X1_X2_D1_t123 * (_X1_X2_D1_t344 - _X1_X2_D1_t415)) + (_X1_X2_D1_t39 * _X1_X2_D1_t416)) - _X1_X2_D1_t302);
                const auto _X1_X2_D1_t417 = (_X1_X2_D1_t383 * _X1_X2_D1_t365);
                const auto _X1_X2_D1_t421 = (_X1_X2_D1_t351 - _X1_X2_D1_t407);
                const auto _X1_X2_D1_t38 = ((_X1_X2_D1_t39 * _X1_X2_D1_t79) - (_X1_X2_D1_t123 * _X1_X2_D1_t149));
                const auto _X1_X2_D1_t240 = (_X1_X2_D1_t39 * _X1_X2_D1_t241);
                const auto _X1_X2_D1_t343 = (_X1_X2_D1_t123 * _X1_X2_D1_t398);
                const auto _X1_X2_D1_t350 = (_X1_X2_D1_t406 + _X1_X2_D1_t404);
                const auto _X1_X2_D1_t356 = (_X1_X2_D1_t383 * _X1_X2_D1_t38);
                const auto _X1_X2_D1_t364 = ((_X1_X2_D1_t30 * _X1_X2_D1_t365) - (_X1_X2_D1_t361 * _X1_X2_D1_t182));
                const auto _X1_X2_D1_t382 = ((_X1_X2_D1_t173 * _X1_X2_D1_t365) - (_X1_X2_D1_t361 * _X1_X2_D1_t38));
                const auto _X1_X2_D1_t388 = (_X1_X2_D1_t302 + _X1_X2_D1_t343);
                const auto _X1_X2_D1_t391 = (_X1_X2_D1_t392 - _X1_X2_D1_t350);
                const auto _X1_X2_D1_t393 = ((_X1_X2_D1_t39 * _X1_X2_D1_t394) - (_X1_X2_D1_t303 * _X1_X2_D1_t149));
                const auto _X1_X2_D1_t397 = (((_X1_X2_D1_t303 * _X1_X2_D1_t183) - (_X1_X2_D1_t39 * _X1_X2_D1_t405)) + (_X1_X2_D1_t205 * _X1_X2_D1_t398));
                const auto _X1_X2_D1_t400 = ((_X1_X2_D1_t39 * _X1_X2_D1_t411) - (_X1_X2_D1_t303 * _X1_X2_D1_t376));
                const auto _X1_X2_D1_t403 = ((_X1_X2_D1_t404 + _X1_X2_D1_t406) - (_X1_X2_D1_t205 * _X1_X2_D1_t416));
                const auto _X1_X2_D1_t410 = (_X1_X2_D1_t123 * _X1_X2_D1_t411);
                const auto _X1_X2_D1_t420 = (_X1_X2_D1_t361 * _X1_X2_D1_t421);
                const auto _X1_X2_D1_t29 = ((_X1_X2_D1_t30 * _X1_X2_D1_t38) - (_X1_X2_D1_t173 * _X1_X2_D1_t182));
                const auto _X1_X2_D1_t239 = (_X1_X2_D1_t240 - _X1_X2_D1_t388);
                const auto _X1_X2_D1_t357 = ((((_X1_X2_D1_t358 * _X1_X2_D1_t29) - (_X1_X2_D1_t219 * _X1_X2_D1_t364)) - (_X1_X2_D1_t345 * _X1_X2_D1_t377)) - (_X1_X2_D1_t30 * _X1_X2_D1_t382));
                const auto _X1_X2_D1_t385 = (_X1_X2_D1_t30 * _X1_X2_D1_t239);
                const auto _X1_X2_D1_t409 = (_X1_X2_D1_t390 - _X1_X2_D1_t410);
                const auto _X1_X2_D1_t419 = (_X1_X2_D1_t173 * _X1_X2_D1_t409);
                const auto _X1_X2_D1_t384 = (_X1_X2_D1_t229 + _X1_X2_D1_t385);
                const auto _X1_X2_D1_t418 = (_X1_X2_D1_t419 - _X1_X2_D1_t420);
                const auto _X1_X2_D1_t413 = (_X1_X2_D1_t418 - _X1_X2_D1_t422);
                const auto res_vPKX1 = (_X1_X2_D1_t0 - vPKX1);
                const auto delta_vPKX1 = (((((_X1_X2_D1_t28 * _X1_X2_D1_t29) - (_X1_X2_D1_t219 * _X1_X2_D1_t384)) - (_X1_X2_D1_t345 * ((_X1_X2_D1_t347 + (_X1_X2_D1_t173 * _X1_X2_D1_t391)) + (_X1_X2_D1_t30 * (_X1_X2_D1_t351 + (_X1_X2_D1_t123 * (_X1_X2_D1_t396 - _X1_X2_D1_t395))))))) - (_X1_X2_D1_t30 * (_X1_X2_D1_t356 + (_X1_X2_D1_t173 * _X1_X2_D1_t239)))) / _X1_X2_D1_t357);
                const auto res_vGKX1 = (-_X1_X2_D1_t383);
                const auto delta_vGKX1 = (((((_X1_X2_D1_t358 * _X1_X2_D1_t384) - (_X1_X2_D1_t28 * _X1_X2_D1_t364)) - (_X1_X2_D1_t345 * ((_X1_X2_D1_t30 * (_X1_X2_D1_t410 - _X1_X2_D1_t390)) - ((_X1_X2_D1_t361 * _X1_X2_D1_t391) + _X1_X2_D1_t408)))) + (_X1_X2_D1_t30 * ((_X1_X2_D1_t361 * _X1_X2_D1_t239) + _X1_X2_D1_t417))) / _X1_X2_D1_t357);
                const auto res_vPKX2 = (_X1_X2_D1_t304 - vPKX2);
                const auto delta_vPKX2 = ((((((_X1_X2_D1_t358 * ((_X1_X2_D1_t30 * _X1_X2_D1_t393) - ((_X1_X2_D1_t173 * _X1_X2_D1_t397) + (_X1_X2_D1_t383 * _X1_X2_D1_t399)))) - (_X1_X2_D1_t219 * ((_X1_X2_D1_t30 * _X1_X2_D1_t400) - ((_X1_X2_D1_t361 * _X1_X2_D1_t397) + (_X1_X2_D1_t383 * _X1_X2_D1_t401))))) + (_X1_X2_D1_t28 * (((_X1_X2_D1_t173 * _X1_X2_D1_t401) - (_X1_X2_D1_t361 * _X1_X2_D1_t399)) + (_X1_X2_D1_t30 * _X1_X2_D1_t402)))) - (_X1_X2_D1_t345 * ((((_X1_X2_D1_t173 * ((_X1_X2_D1_t205 * _X1_X2_D1_t411) - (_X1_X2_D1_t303 * _X1_X2_D1_t379))) - (_X1_X2_D1_t361 * ((_X1_X2_D1_t205 * _X1_X2_D1_t394) - (_X1_X2_D1_t303 * _X1_X2_D1_t349)))) + (_X1_X2_D1_t383 * (_X1_X2_D1_t205 * _X1_X2_D1_t381))) + (_X1_X2_D1_t30 * (_X1_X2_D1_t303 * _X1_X2_D1_t381))))) - (_X1_X2_D1_t30 * (((_X1_X2_D1_t173 * _X1_X2_D1_t400) - (_X1_X2_D1_t361 * _X1_X2_D1_t393)) + (_X1_X2_D1_t383 * _X1_X2_D1_t402)))) / _X1_X2_D1_t357);
                const auto res_vGKX2 = (_X1_X2_D1_t243 - vGKX2);
                const auto delta_vGKX2 = (((((_X1_X2_D1_t358 * ((_X1_X2_D1_t347 - (_X1_X2_D1_t173 * _X1_X2_D1_t403)) + (_X1_X2_D1_t30 * _X1_X2_D1_t421))) - (_X1_X2_D1_t219 * ((_X1_X2_D1_t408 - (_X1_X2_D1_t361 * _X1_X2_D1_t403)) + (_X1_X2_D1_t30 * _X1_X2_D1_t409)))) - (_X1_X2_D1_t28 * _X1_X2_D1_t377)) - (_X1_X2_D1_t30 * _X1_X2_D1_t413)) / _X1_X2_D1_t357);
                const auto res_vD1 = (_X1_X2_D1_t266 - vD1);
                const auto delta_vD1 = (((((_X1_X2_D1_t219 * ((_X1_X2_D1_t361 * _X1_X2_D1_t414) + _X1_X2_D1_t417)) - (_X1_X2_D1_t358 * ((_X1_X2_D1_t173 * _X1_X2_D1_t414) + _X1_X2_D1_t356))) - (_X1_X2_D1_t345 * _X1_X2_D1_t413)) + (_X1_X2_D1_t28 * _X1_X2_D1_t382)) / _X1_X2_D1_t357);
            
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

    // --- Newton-Raphson solve: X1_D1
    const auto _X1_D1_t2 = (gRfb + gRatt);
    const auto _X1_D1_t8 = (1.0 / Rk2);
    const auto _X1_D1_t35 = (gRfb * gRatt);
    const auto _X1_D1_t48 = (1.0 / Rk1);
    const auto _X1_D1_t72 = (1.0 / Rrel);
    const auto _X1_D1_t87 = (_X1_D1_t2 + _X1_D1_t72);
    const auto _X1_D1_t91 = (1.0 / 1000000000.0);
    const auto _X1_D1_t124 = (_12AX7_DEMPWOLF_Gamma - 1.0);
    const auto _X1_D1_t131 = (_12AX7_DEMPWOLF_Xi - 1.0);
    const auto _X1_D1_t136 = (_12AX7_DEMPWOLF_Gk * _12AX7_DEMPWOLF_Gamma);
    const auto _X1_D1_t18 = (_X1_D1_t8 + _X1_D1_t91);
    const auto _X1_D1_t28 = (_X1_D1_t87 + _X1_D1_t91);
    const auto _X1_D1_t30 = (gRatt + _X1_D1_t91);
    const auto _X1_D1_t46 = (gRp2 + _X1_D1_t91);
    const auto _X1_D1_t47 = (_X1_D1_t48 + gRfb);
    const auto _X1_D1_t49 = (gRp1 + _X1_D1_t91);
    const auto _X1_D1_t51 = (1.0 / (gRg1 + _X1_D1_t91));
    const auto _X1_D1_t62 = (_X1_D1_t47 + _X1_D1_t91);
    const auto _X1_D1_t74 = (gRL + _X1_D1_t91);
    const auto _X1_D1_t88 = (gRg2 + _X1_D1_t91);
    const auto _X1_D1_t122 = (_X1_D1_t35 * _X1_D1_t49);
    const auto _X1_D1_t27 = (_X1_D1_t18 * _X1_D1_t74);
    const auto _X1_D1_t121 = (_X1_D1_t122 * _X1_D1_t88);
    const auto _X1_D1_t16 = (_X1_D1_t46 * _X1_D1_t27);
    const auto _X1_D1_t34 = (_X1_D1_t121 * D1N914_Is);
    const auto _X1_D1_t73 = (_X1_D1_t30 * _X1_D1_t16);
    const auto _X1_D1_t15 = (gRatt * _X1_D1_t16);
    const auto _X1_D1_t29 = (_X1_D1_t88 * _X1_D1_t73);
    const auto _X1_D1_t97 = (_X1_D1_t88 * _X1_D1_t15);
    const auto _X1_D1_t10 = (gRp1 * _X1_D1_t29);
    const auto _X1_D1_t17 = (_X1_D1_t49 * _X1_D1_t29);
    const auto _X1_D1_t50 = (gRfb * _X1_D1_t17);
    const auto _X1_D1_t71 = (_X1_D1_t28 * _X1_D1_t17);
    const auto _X1_D1_t75 = (gRfb * _X1_D1_t50);
    const auto _X1_D1_t96 = (_X1_D1_t49 * _X1_D1_t97);
    const auto _X1_D1_t3 = (VCC * _X1_D1_t10);
    const auto _X1_D1_t70 = (gRatt * _X1_D1_t96);
    const auto _X1_D1_t95 = (gRfb * _X1_D1_t96);
    const auto _X1_D1_t26 = (_X1_D1_t70 - _X1_D1_t71);
    const auto _X1_D1_t69 = (_X1_D1_t62 * _X1_D1_t26);
    const auto _X1_D1_t68 = (1.0 / (_X1_D1_t69 + _X1_D1_t75));
    const auto _X1_D1_t163 = (_12AX7_DEMPWOLF_Gg * _12AX7_DEMPWOLF_Xi);
    const auto _X1_D1_t164 = (VCC * (gRp1 * _X1_D1_t97));
    for (int newton_iter = 0; newton_iter < 10000; ++newton_iter)
    {
        const auto _X1_D1_t12 = (vPKX1 / _12AX7_DEMPWOLF_Mu);
        const auto _X1_D1_t61 = (_12AX7_DEMPWOLF_Cg * vGKX1);
        const auto _X1_D1_t90 = (vD1 / D1N914_vt);
        const auto _X1_D1_t11 = (_X1_D1_t12 + vGKX1);
        const auto _X1_D1_t24 = (_12AX7_DEMPWOLF_Ck * _X1_D1_t11);
        const auto _X1_D1_t42 = math_exp_approx(_X1_D1_t90);
        const auto _X1_D1_t45 = math_exp_approx(_X1_D1_t61);
        const auto _X1_D1_t113 = (D1N914_Is * _X1_D1_t42);
        const auto _X1_D1_t7 = (1.0 + _X1_D1_t45);
        const auto _X1_D1_t21 = (_X1_D1_t42 - 1.0);
        const auto _X1_D1_t23 = math_exp_approx(_X1_D1_t24);
        const auto _X1_D1_t44 = (1.0 + _X1_D1_t23);
        const auto _X1_D1_t89 = (D1N914_Is * _X1_D1_t21);
        const auto _X1_D1_t151 = (_12AX7_DEMPWOLF_Mu * _X1_D1_t44);
        const auto _X1_D1_t5 = math_log_approx(_X1_D1_t44);
        const auto _X1_D1_t6 = math_log_approx(_X1_D1_t7);
        const auto _X1_D1_t14 = (_X1_D1_t6 / _12AX7_DEMPWOLF_Cg);
        const auto _X1_D1_t41 = (_X1_D1_t89 * _X1_D1_t16);
        const auto _X1_D1_t59 = (_X1_D1_t5 / _12AX7_DEMPWOLF_Ck);
        const auto _X1_D1_t80 = math_pow_approx(_X1_D1_t14, _X1_D1_t131);
        const auto _X1_D1_t115 = (_X1_D1_t113 * _X1_D1_t16);
        const auto _X1_D1_t117 = (_X1_D1_t88 * _X1_D1_t115);
        const auto _X1_D1_t150 = math_pow_approx(_X1_D1_t59, _X1_D1_t124);
        const auto _X1_D1_t1 = math_pow_approx(_X1_D1_t59, _12AX7_DEMPWOLF_Gamma);
        const auto _X1_D1_t13 = math_pow_approx(_X1_D1_t14, _12AX7_DEMPWOLF_Xi);
        const auto _X1_D1_t25 = (_12AX7_DEMPWOLF_Gg * _X1_D1_t13);
        const auto _X1_D1_t33 = (_X1_D1_t34 * _X1_D1_t42);
        const auto _X1_D1_t40 = (_X1_D1_t88 * _X1_D1_t41);
        const auto _X1_D1_t58 = (_X1_D1_t150 * _X1_D1_t23);
        const auto _X1_D1_t60 = (_X1_D1_t80 * _X1_D1_t45);
        const auto _X1_D1_t78 = (_12AX7_DEMPWOLF_Gamma * _X1_D1_t58);
        const auto _X1_D1_t79 = (_12AX7_DEMPWOLF_Xi * _X1_D1_t60);
        const auto _X1_D1_t94 = (_12AX7_DEMPWOLF_Gk * _X1_D1_t1);
        const auto _X1_D1_t99 = (_X1_D1_t49 * _X1_D1_t40);
        const auto _X1_D1_t101 = (gRfb * _X1_D1_t99);
        const auto _X1_D1_t102 = (_X1_D1_t25 + _12AX7_DEMPWOLF_Ig0);
        const auto _X1_D1_t112 = (_X1_D1_t49 * _X1_D1_t117);
        const auto _X1_D1_t116 = (gRfb * _X1_D1_t112);
        const auto _X1_D1_t120 = (_X1_D1_t33 * _X1_D1_t46);
        const auto _X1_D1_t127 = (_12AX7_DEMPWOLF_Gk * _X1_D1_t78);
        const auto _X1_D1_t129 = (_12AX7_DEMPWOLF_Gg * _X1_D1_t79);
        const auto _X1_D1_t144 = (_X1_D1_t136 * _X1_D1_t150);
        const auto _X1_D1_t148 = (_X1_D1_t144 * _X1_D1_t23);
        const auto _X1_D1_t152 = (_X1_D1_t127 * _X1_D1_t29);
        const auto _X1_D1_t39 = (gRatt * _X1_D1_t99);
        const auto _X1_D1_t67 = (_X1_D1_t129 / _X1_D1_t7);
        const auto _X1_D1_t93 = (_X1_D1_t94 - _X1_D1_t102);
        const auto _X1_D1_t98 = (_X1_D1_t28 * _X1_D1_t99);
        const auto _X1_D1_t100 = (gRfb * _X1_D1_t101);
        const auto _X1_D1_t109 = (_X1_D1_t28 * _X1_D1_t112);
        const auto _X1_D1_t111 = (gRfb * _X1_D1_t116);
        const auto _X1_D1_t119 = (_X1_D1_t120 * _X1_D1_t18);
        const auto _X1_D1_t126 = (_X1_D1_t127 / _X1_D1_t44);
        const auto _X1_D1_t147 = (_X1_D1_t148 * gRfb);
        const auto _X1_D1_t4 = (_X1_D1_t93 * _X1_D1_t29);
        const auto _X1_D1_t9 = (_X1_D1_t3 - _X1_D1_t4);
        const auto _X1_D1_t20 = (gRfb * _X1_D1_t39);
        const auto _X1_D1_t22 = (_X1_D1_t93 + _X1_D1_t102);
        const auto _X1_D1_t32 = (_X1_D1_t119 * _X1_D1_t74);
        const auto _X1_D1_t86 = (_X1_D1_t62 * _X1_D1_t98);
        const auto _X1_D1_t108 = (_X1_D1_t62 * _X1_D1_t109);
        const auto _X1_D1_t110 = (_X1_D1_t111 / D1N914_vt);
        const auto _X1_D1_t118 = (_X1_D1_t32 / D1N914_vt);
        const auto _X1_D1_t125 = (_X1_D1_t126 - _X1_D1_t67);
        const auto _X1_D1_t128 = (_X1_D1_t125 * _X1_D1_t29);
        const auto _X1_D1_t143 = (_X1_D1_t147 * _X1_D1_t49);
        const auto _X1_D1_t43 = (_X1_D1_t22 * _X1_D1_t26);
        const auto _X1_D1_t57 = (_X1_D1_t125 + _X1_D1_t67);
        const auto _X1_D1_t66 = (_X1_D1_t57 * gRfb);
        const auto _X1_D1_t77 = (_X1_D1_t57 * _X1_D1_t26);
        const auto _X1_D1_t85 = (_X1_D1_t86 - _X1_D1_t100);
        const auto _X1_D1_t92 = (_X1_D1_t22 * _X1_D1_t95);
        const auto _X1_D1_t107 = (_X1_D1_t108 / D1N914_vt);
        const auto _X1_D1_t142 = (_X1_D1_t143 * _X1_D1_t88);
        const auto _X1_D1_t154 = (_X1_D1_t148 * _X1_D1_t26);
        const auto _X1_D1_t38 = (_X1_D1_t20 - _X1_D1_t43);
        const auto _X1_D1_t56 = (_X1_D1_t66 * _X1_D1_t49);
        const auto _X1_D1_t84 = (_X1_D1_t85 + _X1_D1_t92);
        const auto _X1_D1_t106 = (_X1_D1_t107 - _X1_D1_t110);
        const auto _X1_D1_t114 = (_X1_D1_t106 * _X1_D1_t68);
        const auto _X1_D1_t130 = (_X1_D1_t77 * _X1_D1_t68);
        const auto _X1_D1_t135 = (_X1_D1_t154 / _X1_D1_t151);
        const auto _X1_D1_t141 = (_X1_D1_t142 * gRatt);
        const auto _X1_D1_t153 = (_X1_D1_t135 * _X1_D1_t68);
        const auto _X1_D1_t19 = (_X1_D1_t38 * _X1_D1_t68);
        const auto _X1_D1_t31 = (_X1_D1_t118 * _X1_D1_t68);
        const auto _X1_D1_t37 = (_X1_D1_t19 - (_X1_D1_t102 * _X1_D1_t51));
        const auto _X1_D1_t55 = (_X1_D1_t56 * _X1_D1_t88);
        const auto _X1_D1_t65 = (_X1_D1_t55 * gRatt);
        const auto _X1_D1_t76 = ((_X1_D1_t130 + ((((_X1_D1_t163 * _X1_D1_t80) * _X1_D1_t45) / _X1_D1_t7) * _X1_D1_t51)) + 1.0);
        const auto _X1_D1_t83 = (_X1_D1_t84 * _X1_D1_t68);
        const auto _X1_D1_t105 = ((_X1_D1_t114 - _X1_D1_t114) - 1.0);
        const auto _X1_D1_t123 = ((((_X1_D1_t62 * ((_X1_D1_t28 * _X1_D1_t128) - (gRatt * (_X1_D1_t125 * _X1_D1_t97)))) - (gRfb * (gRfb * _X1_D1_t128))) * _X1_D1_t68) - _X1_D1_t130);
        const auto _X1_D1_t140 = (_X1_D1_t141 * _X1_D1_t46);
        const auto _X1_D1_t149 = (((((_X1_D1_t62 * (((_X1_D1_t28 * _X1_D1_t152) / _X1_D1_t151) - ((gRatt * (_X1_D1_t127 * _X1_D1_t97)) / _X1_D1_t151))) - ((gRfb * (gRfb * _X1_D1_t152)) / _X1_D1_t151)) * _X1_D1_t68) - _X1_D1_t153) - 1.0);
        const auto _X1_D1_t0 = ((((_X1_D1_t62 * ((gRatt * (_X1_D1_t164 - (_X1_D1_t93 * _X1_D1_t97))) - (_X1_D1_t28 * _X1_D1_t9))) + (gRfb * (gRfb * _X1_D1_t9))) * _X1_D1_t68) + _X1_D1_t19);
        const auto _X1_D1_t36 = (vGKX1 - _X1_D1_t37);
        const auto _X1_D1_t54 = (_X1_D1_t65 * _X1_D1_t46);
        const auto _X1_D1_t82 = (_X1_D1_t83 - _X1_D1_t83);
        const auto _X1_D1_t103 = (vPKX1 - _X1_D1_t0);
        const auto _X1_D1_t139 = (_X1_D1_t140 * _X1_D1_t18);
        const auto _X1_D1_t146 = (_X1_D1_t139 * _X1_D1_t74);
        const auto _X1_D1_t157 = (_X1_D1_t36 * _X1_D1_t105);
        const auto _X1_D1_t53 = (_X1_D1_t54 * _X1_D1_t18);
        const auto _X1_D1_t64 = (_X1_D1_t53 * _X1_D1_t74);
        const auto _X1_D1_t81 = (vD1 - _X1_D1_t82);
        const auto _X1_D1_t132 = (_X1_D1_t31 * _X1_D1_t81);
        const auto _X1_D1_t138 = (_X1_D1_t146 / _X1_D1_t151);
        const auto _X1_D1_t145 = (_X1_D1_t138 * _X1_D1_t68);
        const auto _X1_D1_t156 = (_X1_D1_t157 - _X1_D1_t132);
        const auto _X1_D1_t159 = (_X1_D1_t153 * _X1_D1_t81);
        const auto _X1_D1_t161 = (_X1_D1_t76 * _X1_D1_t81);
        const auto _X1_D1_t63 = (_X1_D1_t64 * _X1_D1_t68);
        const auto _X1_D1_t137 = (_X1_D1_t145 - _X1_D1_t145);
        const auto _X1_D1_t160 = (_X1_D1_t36 * _X1_D1_t137);
        const auto _X1_D1_t52 = (_X1_D1_t63 - _X1_D1_t63);
        const auto _X1_D1_t104 = ((_X1_D1_t76 * _X1_D1_t105) + (_X1_D1_t31 * _X1_D1_t52));
        const auto _X1_D1_t134 = ((_X1_D1_t153 * _X1_D1_t105) + (_X1_D1_t31 * _X1_D1_t137));
        const auto _X1_D1_t155 = ((_X1_D1_t76 * _X1_D1_t137) - (_X1_D1_t153 * _X1_D1_t52));
        const auto _X1_D1_t158 = (_X1_D1_t159 + _X1_D1_t160);
        const auto _X1_D1_t162 = (_X1_D1_t36 * _X1_D1_t52);
        const auto _X1_D1_t133 = (((_X1_D1_t123 * _X1_D1_t134) - (_X1_D1_t149 * _X1_D1_t104)) + (_X1_D1_t31 * _X1_D1_t155));
        const auto res_vPKX1 = (_X1_D1_t0 - vPKX1);
        const auto delta_vPKX1 = (((_X1_D1_t31 * (_X1_D1_t162 + _X1_D1_t161)) - ((_X1_D1_t103 * _X1_D1_t104) + (_X1_D1_t123 * _X1_D1_t156))) / _X1_D1_t133);
        const auto res_vGKX1 = (_X1_D1_t37 - vGKX1);
        const auto delta_vGKX1 = ((((_X1_D1_t149 * _X1_D1_t156) + (_X1_D1_t103 * _X1_D1_t134)) - (_X1_D1_t31 * _X1_D1_t158)) / _X1_D1_t133);
        const auto res_vD1 = (_X1_D1_t82 - vD1);
        const auto delta_vD1 = ((((_X1_D1_t123 * _X1_D1_t158) - (_X1_D1_t149 * (_X1_D1_t161 + _X1_D1_t162))) + (_X1_D1_t103 * _X1_D1_t155)) / _X1_D1_t133);
    
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
    // --- Newton-Raphson solve: X2
    const auto _X2_t20 = (1.0 / Rk2);
    const auto _X2_t50 = (gRfb + gRatt);
    const auto _X2_t54 = (1.0 / 1000000000.0);
    const auto _X2_t58 = (1.0 / Rk1);
    const auto _X2_t66 = (gRatt + _X2_t54);
    const auto _X2_t69 = (_12AX7_DEMPWOLF_Gamma - 1.0);
    const auto _X2_t74 = (gRL + _X2_t54);
    const auto _X2_t78 = (1.0 / Rrel);
    const auto _X2_t98 = (_12AX7_DEMPWOLF_Xi - 1.0);
    const auto _X2_t5 = (gRp1 + _X2_t54);
    const auto _X2_t23 = (_X2_t58 + gRfb);
    const auto _X2_t37 = (gRp2 + _X2_t54);
    const auto _X2_t43 = (gRg2 + _X2_t54);
    const auto _X2_t48 = (_X2_t20 + _X2_t54);
    const auto _X2_t64 = (_X2_t50 + _X2_t78);
    const auto _X2_t13 = (_X2_t48 * _X2_t74);
    const auto _X2_t22 = (_X2_t23 + _X2_t54);
    const auto _X2_t44 = (_X2_t37 * _X2_t13);
    const auto _X2_t49 = (_X2_t64 + _X2_t54);
    const auto _X2_t53 = (gRatt * _X2_t44);
    const auto _X2_t79 = (_X2_t66 * _X2_t44);
    const auto _X2_t12 = (gRp2 * _X2_t13);
    const auto _X2_t19 = (_X2_t43 * _X2_t79);
    const auto _X2_t41 = (_X2_t43 * _X2_t53);
    const auto _X2_t80 = (_X2_t5 * _X2_t19);
    const auto _X2_t6 = (_X2_t66 * _X2_t12);
    const auto _X2_t11 = (_X2_t43 * _X2_t6);
    const auto _X2_t18 = (_X2_t5 * _X2_t41);
    const auto _X2_t42 = (_X2_t49 * _X2_t80);
    const auto _X2_t56 = (gRfb * _X2_t80);
    const auto _X2_t77 = (gRatt * _X2_t18);
    const auto _X2_t10 = (_X2_t5 * _X2_t11);
    const auto _X2_t55 = (gRfb * _X2_t56);
    const auto _X2_t76 = (_X2_t77 - _X2_t42);
    const auto _X2_t4 = (VCC * _X2_t10);
    const auto _X2_t40 = (_X2_t22 * _X2_t76);
    const auto _X2_t75 = (1.0 / (_X2_t40 + _X2_t55));
    const auto _X2_t145 = (VCC * (_X2_t5 * (_X2_t43 * (gRatt * _X2_t12))));
    for (int newton_iter = 0; newton_iter < 10000; ++newton_iter)
    {
        const auto _X2_t28 = (vPKX2 / _12AX7_DEMPWOLF_Mu);
        const auto _X2_t33 = (_12AX7_DEMPWOLF_Cg * vGKX2);
        const auto _X2_t39 = (_X2_t28 + vGKX2);
        const auto _X2_t17 = math_exp_approx(_X2_t33);
        const auto _X2_t27 = (_12AX7_DEMPWOLF_Ck * _X2_t39);
        const auto _X2_t2 = math_exp_approx(_X2_t27);
        const auto _X2_t8 = (1.0 + _X2_t17);
        const auto _X2_t26 = (1.0 + _X2_t2);
        const auto _X2_t73 = math_log_approx(_X2_t8);
        const auto _X2_t123 = (_12AX7_DEMPWOLF_Mu * _X2_t26);
        const auto _X2_t7 = math_log_approx(_X2_t26);
        const auto _X2_t68 = (_X2_t7 / _12AX7_DEMPWOLF_Ck);
        const auto _X2_t71 = (_X2_t73 / _12AX7_DEMPWOLF_Cg);
        const auto _X2_t93 = math_pow_approx(_X2_t68, _X2_t69);
        const auto _X2_t94 = math_pow_approx(_X2_t71, _X2_t98);
        const auto _X2_t113 = math_pow_approx(_X2_t71, _12AX7_DEMPWOLF_Xi);
        const auto _X2_t116 = math_pow_approx(_X2_t68, _12AX7_DEMPWOLF_Gamma);
        const auto _X2_t143 = (_X2_t93 * _X2_t2);
        const auto _X2_t32 = (_12AX7_DEMPWOLF_Gk * _X2_t116);
        const auto _X2_t70 = (_X2_t94 * _X2_t17);
        const auto _X2_t102 = (_12AX7_DEMPWOLF_Gamma * _X2_t143);
        const auto _X2_t106 = (_12AX7_DEMPWOLF_Gg * _X2_t113);
        const auto _X2_t3 = (_X2_t106 + _12AX7_DEMPWOLF_Ig0);
        const auto _X2_t52 = (_12AX7_DEMPWOLF_Xi * _X2_t70);
        const auto _X2_t63 = (_12AX7_DEMPWOLF_Gg * _X2_t52);
        const auto _X2_t67 = (_12AX7_DEMPWOLF_Gk * _X2_t102);
        const auto _X2_t101 = (_X2_t67 / _X2_t26);
        const auto _X2_t105 = (_X2_t3 * _X2_t79);
        const auto _X2_t112 = (_X2_t32 - _X2_t3);
        const auto _X2_t115 = (_X2_t112 + _X2_t3);
        const auto _X2_t122 = (_X2_t67 * _X2_t74);
        const auto _X2_t130 = (_X2_t67 * _X2_t13);
        const auto _X2_t138 = (_X2_t37 * _X2_t122);
        const auto _X2_t1 = (_X2_t112 * _X2_t13);
        const auto _X2_t16 = (_X2_t66 * _X2_t1);
        const auto _X2_t38 = (_X2_t115 * _X2_t74);
        const auto _X2_t51 = (_X2_t63 * _X2_t79);
        const auto _X2_t62 = (_X2_t63 / _X2_t8);
        const auto _X2_t92 = (_X2_t101 - _X2_t62);
        const auto _X2_t97 = (_X2_t92 + _X2_t62);
        const auto _X2_t104 = (_X2_t5 * _X2_t105);
        const auto _X2_t114 = (_X2_t37 * _X2_t38);
        const auto _X2_t121 = (_X2_t66 * _X2_t138);
        const auto _X2_t128 = (_X2_t66 * _X2_t130);
        const auto _X2_t137 = (gRatt * _X2_t138);
        const auto _X2_t140 = (_X2_t43 * _X2_t121);
        const auto _X2_t15 = (_X2_t43 * _X2_t16);
        const auto _X2_t31 = (_X2_t66 * _X2_t114);
        const auto _X2_t36 = (_X2_t43 * _X2_t31);
        const auto _X2_t47 = (_X2_t5 * _X2_t51);
        const auto _X2_t83 = (_X2_t92 * _X2_t13);
        const auto _X2_t85 = (_X2_t66 * _X2_t83);
        const auto _X2_t91 = (_X2_t97 * _X2_t74);
        const auto _X2_t111 = (gRatt * _X2_t114);
        const auto _X2_t119 = (_X2_t43 * _X2_t137);
        const auto _X2_t125 = (_X2_t5 * _X2_t140);
        const auto _X2_t127 = (_X2_t43 * _X2_t128);
        const auto _X2_t129 = (_X2_t5 * _X2_t127);
        const auto _X2_t136 = (_X2_t5 * _X2_t119);
        const auto _X2_t14 = (_X2_t5 * _X2_t15);
        const auto _X2_t30 = (_X2_t5 * _X2_t36);
        const auto _X2_t35 = (gRfb * _X2_t30);
        const auto _X2_t61 = (_X2_t37 * _X2_t91);
        const auto _X2_t65 = (_X2_t66 * _X2_t61);
        const auto _X2_t82 = (_X2_t43 * _X2_t85);
        const auto _X2_t84 = (_X2_t5 * _X2_t82);
        const auto _X2_t90 = (gRatt * _X2_t61);
        const auto _X2_t96 = (_X2_t43 * _X2_t65);
        const auto _X2_t100 = (_X2_t5 * _X2_t96);
        const auto _X2_t110 = (_X2_t43 * _X2_t111);
        const auto _X2_t120 = (_X2_t49 * _X2_t125);
        const auto _X2_t124 = (gRfb * _X2_t125);
        const auto _X2_t135 = (gRatt * _X2_t136);
        const auto _X2_t139 = (_X2_t120 / _X2_t123);
        const auto _X2_t142 = (gRfb * _X2_t124);
        const auto _X2_t9 = (_X2_t4 - _X2_t14);
        const auto _X2_t25 = (_X2_t5 * _X2_t110);
        const auto _X2_t29 = (_X2_t49 * _X2_t30);
        const auto _X2_t34 = (gRfb * _X2_t35);
        const auto _X2_t72 = (gRfb * _X2_t100);
        const auto _X2_t89 = (_X2_t43 * _X2_t90);
        const auto _X2_t95 = (_X2_t49 * _X2_t100);
        const auto _X2_t99 = (gRfb * _X2_t72);
        const auto _X2_t109 = (gRatt * _X2_t25);
        const auto _X2_t134 = (_X2_t135 / _X2_t123);
        const auto _X2_t141 = (_X2_t142 / _X2_t123);
        const auto _X2_t24 = (_X2_t109 - _X2_t29);
        const auto _X2_t60 = (_X2_t5 * _X2_t89);
        const auto _X2_t133 = (_X2_t134 - _X2_t139);
        const auto _X2_t21 = (_X2_t22 * _X2_t24);
        const auto _X2_t59 = (gRatt * _X2_t60);
        const auto _X2_t88 = (_X2_t59 - _X2_t95);
        const auto _X2_t108 = (_X2_t21 + _X2_t34);
        const auto _X2_t132 = (_X2_t22 * _X2_t133);
        const auto _X2_t87 = (_X2_t22 * _X2_t88);
        const auto _X2_t107 = (_X2_t108 * _X2_t75);
        const auto _X2_t131 = (_X2_t132 + _X2_t141);
        const auto _X2_t0 = ((((_X2_t22 * ((gRatt * (_X2_t145 - (_X2_t5 * (_X2_t43 * (gRatt * _X2_t1))))) - (_X2_t49 * _X2_t9))) + (gRfb * (gRfb * _X2_t9))) * _X2_t75) - _X2_t107);
        const auto _X2_t45 = (vPKX2 - _X2_t0);
        const auto _X2_t57 = (_X2_t87 + _X2_t99);
        const auto _X2_t86 = (_X2_t57 * _X2_t75);
        const auto _X2_t103 = ((((gRfb * (gRfb * _X2_t104)) - (_X2_t22 * ((_X2_t49 * _X2_t104) - (gRatt * (_X2_t5 * (_X2_t3 * _X2_t53)))))) * _X2_t75) + _X2_t107);
        const auto _X2_t118 = (_X2_t131 * _X2_t75);
        const auto _X2_t126 = (((((_X2_t22 * (((_X2_t49 * _X2_t129) / _X2_t123) - ((gRatt * (_X2_t5 * (_X2_t43 * (gRatt * _X2_t130)))) / _X2_t123))) - ((gRfb * (gRfb * _X2_t129)) / _X2_t123)) * _X2_t75) - _X2_t118) - 1.0);
        const auto _X2_t144 = (_X2_t103 + vGKX2);
        const auto _X2_t46 = ((((((gRfb * (gRfb * _X2_t47)) / _X2_t8) - (_X2_t22 * (((_X2_t49 * _X2_t47) / _X2_t8) - ((gRatt * (_X2_t5 * (_X2_t63 * _X2_t53))) / _X2_t8)))) * _X2_t75) + _X2_t86) + 1.0);
        const auto _X2_t81 = ((((_X2_t22 * ((_X2_t49 * _X2_t84) - (gRatt * (_X2_t5 * (_X2_t43 * (gRatt * _X2_t83)))))) - (gRfb * (gRfb * _X2_t84))) * _X2_t75) - _X2_t86);
        const auto _X2_t117 = ((_X2_t81 * _X2_t118) - (_X2_t126 * _X2_t46));
        const auto res_vPKX2 = (_X2_t0 - vPKX2);
        const auto delta_vPKX2 = (-(((_X2_t45 * _X2_t46) + (_X2_t81 * _X2_t144)) / _X2_t117));
        const auto res_vGKX2 = (-_X2_t144);
        const auto delta_vGKX2 = (((_X2_t126 * _X2_t144) + (_X2_t45 * _X2_t118)) / _X2_t117);
    
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
    }
    return vo_dc_out;
}

