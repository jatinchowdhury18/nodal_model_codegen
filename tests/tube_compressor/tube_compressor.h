// Auto-generated with netlist_codegen version 9b9cfe2.
// Command: netlist_codegen tube_compressor.net tube_compressor.h -type_name double

#pragma once

#include <cmath>

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

[[maybe_unused]] static auto limit_junction_voltage = [](auto v_new, auto v_old, auto vt, auto vcrit)
{
    if (v_new > vcrit && std::abs(v_new - v_old) > 2 * vt)
    {
        if (v_old > 0)
        {
            const auto arg = 1 + (v_new - v_old) / vt;
            v_new = arg > 0 ? v_old + vt * std::log(arg) : vcrit;
        }
        else
        {
            v_new = vt * std::log(v_new / vt);
        }
    }
    else if (v_new < -vcrit && std::abs(v_new - v_old) > 2 * vt)
    {
        if (v_old < 0)
        {
            const auto arg = 1 + (v_old - v_new) / vt;
            v_new = arg > 0 ? v_old - vt * std::log(arg) : -vcrit;
        }
        else
        {
            v_new = -vt * std::log(-v_new / vt);
        }
    }
    return v_new;
};

static constexpr auto newton_tol_sq = 1.0e-05;
static constexpr int newton_max_iter = 20;

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
    const auto _t4 = (gRp1 + gCc12);
    const auto _t6 = (gCc12 + gRg2);
    const auto _t9 = (gRp2 + gCout);
    const auto _t23 = (gCout * gCsc);
    const auto _t34 = ((gRfb + gRatt) + gRrelCenv);
    const auto _t44 = ((gCout + gRL) + gCsc);
    const auto _t22 = (gRk2Ck2 * _t23);
    const auto _t31 = (gRp2 * _t22);
    const auto _t43 = ((_t44 * gCsc) - (gCsc * gCsc));
    const auto _t30 = (gRatt * _t31);
    const auto _t33 = (_t6 * _t30);
    const auto _t42 = (gRk2Ck2 * _t43);
    const auto _t89 = (gRp2 * _t42);
    const auto _t29 = (gCc12 * _t30);
    const auto _t32 = (_t4 * _t33);
    const auto _t41 = ((_t9 * _t42) - (gCout * _t22));
    const auto _t88 = (gRatt * _t89);
    const auto _t91 = (_t6 * _t88);
    const auto _t28 = (gCc12 * _t29);
    const auto _t40 = (gRatt * _t41);
    const auto _t46 = (_t6 * _t40);
    const auto _t69 = (gRp1 * _t46);
    const auto _t87 = (gCc12 * _t88);
    const auto _t90 = (_t4 * _t91);
    const auto _t101 = (_t32 - _t28);
    const auto _t27 = (_t28 - _t32);
    const auto _t39 = (gCc12 * _t40);
    const auto _t45 = (_t4 * _t46);
    const auto _t68 = (VCC * _t69);
    const auto _t75 = (gRp1 * _t39);
    const auto _t86 = (gCc12 * _t87);
    const auto _t100 = (VCC * _t101);
    const auto _t26 = (VCC * _t27);
    const auto _t38 = (gCc12 * _t39);
    const auto _t74 = (VCC * _t75);
    const auto _t85 = (_t86 - _t90);
    const auto _t37 = (_t38 - _t45);
    const auto _t47 = (gRfb * _t37);
    const auto _t84 = (VCC * _t85);
    const auto _t36 = ((gRatt * _t37) - (_t34 * _t37));
    const auto _t35 = (1.0 / ((_t0 * _t36) + (gRfb * _t47)));
    const auto _t110 = (1.0 / (gCin + gRg1));
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
            const auto _X1_X2_D1_t1 = (gRk1Ck1 + gRfb);
            const auto _X1_X2_D1_t25 = (gCc12 + gRg2);
            const auto _X1_X2_D1_t29 = (gRp2 + gCout);
            const auto _X1_X2_D1_t34 = (gCout + gRL);
            const auto _X1_X2_D1_t35 = (gCsc * gCsc);
            const auto _X1_X2_D1_t38 = (gCout * gCsc);
            const auto _X1_X2_D1_t53 = (gRfb + gRatt);
            const auto _X1_X2_D1_t62 = (gRp1 + gCc12);
            const auto _X1_X2_D1_t89 = (gRfb * gRatt);
            const auto _X1_X2_D1_t119 = (_12AX7_DEMPWOLF_Gamma - 1.0);
            const auto _X1_X2_D1_t125 = (_12AX7_DEMPWOLF_Xi - 1.0);
            const auto _X1_X2_D1_t219 = (1.0 / (gCin + gRg1));
            const auto _X1_X2_D1_t317 = (zCout - zCsc);
            const auto _X1_X2_D1_t384 = (_12AX7_DEMPWOLF_Gk * _12AX7_DEMPWOLF_Gamma);
            const auto _X1_X2_D1_t33 = (_X1_X2_D1_t34 + gCsc);
            const auto _X1_X2_D1_t37 = (gRk2Ck2 * _X1_X2_D1_t38);
            const auto _X1_X2_D1_t52 = (_X1_X2_D1_t53 + gRrelCenv);
            const auto _X1_X2_D1_t316 = (_X1_X2_D1_t317 * gCsc);
            const auto _X1_X2_D1_t327 = (gRp2 * _X1_X2_D1_t37);
            const auto _X1_X2_D1_t32 = (_X1_X2_D1_t33 * gCsc);
            const auto _X1_X2_D1_t36 = (gCout * _X1_X2_D1_t37);
            const auto _X1_X2_D1_t326 = (gRatt * _X1_X2_D1_t327);
            const auto _X1_X2_D1_t329 = (gCc12 * _X1_X2_D1_t326);
            const auto _X1_X2_D1_t31 = (_X1_X2_D1_t32 - _X1_X2_D1_t35);
            const auto _X1_X2_D1_t325 = (_X1_X2_D1_t25 * _X1_X2_D1_t326);
            const auto _X1_X2_D1_t328 = (gCc12 * _X1_X2_D1_t329);
            const auto _X1_X2_D1_t30 = (gRk2Ck2 * _X1_X2_D1_t31);
            const auto _X1_X2_D1_t324 = (_X1_X2_D1_t62 * _X1_X2_D1_t325);
            const auto _X1_X2_D1_t353 = (gRp2 * _X1_X2_D1_t30);
            const auto _X1_X2_D1_t28 = (_X1_X2_D1_t29 * _X1_X2_D1_t30);
            const auto _X1_X2_D1_t323 = (_X1_X2_D1_t324 - _X1_X2_D1_t328);
            const auto _X1_X2_D1_t352 = (gRatt * _X1_X2_D1_t353);
            const auto _X1_X2_D1_t355 = (_X1_X2_D1_t25 * _X1_X2_D1_t352);
            const auto _X1_X2_D1_t27 = (_X1_X2_D1_t28 - _X1_X2_D1_t36);
            const auto _X1_X2_D1_t322 = (VCC * _X1_X2_D1_t323);
            const auto _X1_X2_D1_t351 = (gCc12 * _X1_X2_D1_t352);
            const auto _X1_X2_D1_t354 = (_X1_X2_D1_t62 * _X1_X2_D1_t355);
            const auto _X1_X2_D1_t26 = (gRatt * _X1_X2_D1_t27);
            const auto _X1_X2_D1_t60 = (gCc12 * _X1_X2_D1_t26);
            const auto _X1_X2_D1_t273 = (gRp1 * _X1_X2_D1_t60);
            const auto _X1_X2_D1_t350 = (gCc12 * _X1_X2_D1_t351);
            const auto _X1_X2_D1_t24 = (_X1_X2_D1_t25 * _X1_X2_D1_t26);
            const auto _X1_X2_D1_t51 = (gRp1 * _X1_X2_D1_t24);
            const auto _X1_X2_D1_t59 = (gCc12 * _X1_X2_D1_t60);
            const auto _X1_X2_D1_t61 = (_X1_X2_D1_t62 * _X1_X2_D1_t24);
            const auto _X1_X2_D1_t272 = (VCC * _X1_X2_D1_t273);
            const auto _X1_X2_D1_t349 = (_X1_X2_D1_t350 - _X1_X2_D1_t354);
            const auto _X1_X2_D1_t50 = (VCC * _X1_X2_D1_t51);
            const auto _X1_X2_D1_t58 = (_X1_X2_D1_t59 - _X1_X2_D1_t61);
            const auto _X1_X2_D1_t63 = (_X1_X2_D1_t52 * _X1_X2_D1_t58);
            const auto _X1_X2_D1_t65 = (gRfb * _X1_X2_D1_t58);
            const auto _X1_X2_D1_t84 = (zRrelCenv * _X1_X2_D1_t58);
            const auto _X1_X2_D1_t348 = (VCC * _X1_X2_D1_t349);
            const auto _X1_X2_D1_t57 = (gRatt * _X1_X2_D1_t58);
            const auto _X1_X2_D1_t64 = (gRfb * _X1_X2_D1_t65);
            const auto _X1_X2_D1_t56 = (_X1_X2_D1_t57 - _X1_X2_D1_t63);
            const auto _X1_X2_D1_t55 = (_X1_X2_D1_t1 * _X1_X2_D1_t56);
            const auto _X1_X2_D1_t54 = (1.0 / (_X1_X2_D1_t55 + _X1_X2_D1_t64));
            const auto _X1_X2_D1_t423 = (_12AX7_DEMPWOLF_Gg * _12AX7_DEMPWOLF_Xi);
            const auto _X1_X2_D1_t424 = ((gCin * vi) - zCin);
            for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
            {
                const auto _X1_X2_D1_t15 = (vPKX1 / _12AX7_DEMPWOLF_Mu);
                const auto _X1_X2_D1_t23 = (_12AX7_DEMPWOLF_Cg * vGKX1);
                const auto _X1_X2_D1_t49 = (_12AX7_DEMPWOLF_Cg * vGKX2);
                const auto _X1_X2_D1_t81 = (vD1 / D1N914_vt);
                const auto _X1_X2_D1_t118 = (vPKX2 / _12AX7_DEMPWOLF_Mu);
                const auto _X1_X2_D1_t14 = (_X1_X2_D1_t15 + vGKX1);
                const auto _X1_X2_D1_t22 = exp(_X1_X2_D1_t23);
                const auto _X1_X2_D1_t48 = exp(_X1_X2_D1_t49);
                const auto _X1_X2_D1_t80 = exp(_X1_X2_D1_t81);
                const auto _X1_X2_D1_t95 = (D1N914_Is * _X1_X2_D1_t80);
                const auto _X1_X2_D1_t117 = (_X1_X2_D1_t118 + vGKX2);
                const auto _X1_X2_D1_t233 = (gCout * _X1_X2_D1_t95);
                const auto _X1_X2_D1_t237 = (_X1_X2_D1_t33 * _X1_X2_D1_t95);
                const auto _X1_X2_D1_t250 = (gCsc * _X1_X2_D1_t95);
                const auto _X1_X2_D1_t13 = (_12AX7_DEMPWOLF_Ck * _X1_X2_D1_t14);
                const auto _X1_X2_D1_t21 = (1.0 + _X1_X2_D1_t22);
                const auto _X1_X2_D1_t47 = (1.0 + _X1_X2_D1_t48);
                const auto _X1_X2_D1_t79 = (_X1_X2_D1_t80 - 1.0);
                const auto _X1_X2_D1_t116 = (_12AX7_DEMPWOLF_Ck * _X1_X2_D1_t117);
                const auto _X1_X2_D1_t232 = (gRk2Ck2 * _X1_X2_D1_t233);
                const auto _X1_X2_D1_t236 = (gRk2Ck2 * _X1_X2_D1_t237);
                const auto _X1_X2_D1_t249 = (gRk2Ck2 * _X1_X2_D1_t250);
                const auto _X1_X2_D1_t12 = exp(_X1_X2_D1_t13);
                const auto _X1_X2_D1_t20 = log(_X1_X2_D1_t21);
                const auto _X1_X2_D1_t46 = log(_X1_X2_D1_t47);
                const auto _X1_X2_D1_t78 = (D1N914_Is * _X1_X2_D1_t79);
                const auto _X1_X2_D1_t115 = exp(_X1_X2_D1_t116);
                const auto _X1_X2_D1_t231 = (gCout * _X1_X2_D1_t232);
                const auto _X1_X2_D1_t235 = (_X1_X2_D1_t29 * _X1_X2_D1_t236);
                const auto _X1_X2_D1_t248 = (gCout * _X1_X2_D1_t249);
                const auto _X1_X2_D1_t311 = (zCsc + _X1_X2_D1_t78);
                const auto _X1_X2_D1_t315 = (_X1_X2_D1_t33 * _X1_X2_D1_t311);
                const auto _X1_X2_D1_t345 = (gCsc * _X1_X2_D1_t311);
                const auto _X1_X2_D1_t11 = (1.0 + _X1_X2_D1_t12);
                const auto _X1_X2_D1_t19 = (_X1_X2_D1_t20 / _12AX7_DEMPWOLF_Cg);
                const auto _X1_X2_D1_t45 = (_X1_X2_D1_t46 / _12AX7_DEMPWOLF_Cg);
                const auto _X1_X2_D1_t114 = (1.0 + _X1_X2_D1_t115);
                const auto _X1_X2_D1_t124 = pow(_X1_X2_D1_t45, _X1_X2_D1_t125);
                const auto _X1_X2_D1_t161 = pow(_X1_X2_D1_t19, _X1_X2_D1_t125);
                const auto _X1_X2_D1_t169 = (_12AX7_DEMPWOLF_Mu * _X1_X2_D1_t114);
                const auto _X1_X2_D1_t230 = (_X1_X2_D1_t231 / D1N914_vt);
                const auto _X1_X2_D1_t234 = (_X1_X2_D1_t235 / D1N914_vt);
                const auto _X1_X2_D1_t247 = (gRatt * _X1_X2_D1_t248);
                const auto _X1_X2_D1_t253 = (gCc12 * _X1_X2_D1_t247);
                const auto _X1_X2_D1_t310 = (gCout * _X1_X2_D1_t311);
                const auto _X1_X2_D1_t314 = (_X1_X2_D1_t315 + _X1_X2_D1_t316);
                const auto _X1_X2_D1_t344 = (_X1_X2_D1_t316 + _X1_X2_D1_t345);
                const auto _X1_X2_D1_t378 = (_12AX7_DEMPWOLF_Mu * _X1_X2_D1_t11);
                const auto _X1_X2_D1_t10 = log(_X1_X2_D1_t11);
                const auto _X1_X2_D1_t18 = pow(_X1_X2_D1_t19, _12AX7_DEMPWOLF_Xi);
                const auto _X1_X2_D1_t44 = pow(_X1_X2_D1_t45, _12AX7_DEMPWOLF_Xi);
                const auto _X1_X2_D1_t113 = log(_X1_X2_D1_t114);
                const auto _X1_X2_D1_t123 = (_X1_X2_D1_t124 * _X1_X2_D1_t48);
                const auto _X1_X2_D1_t160 = (_X1_X2_D1_t161 * _X1_X2_D1_t22);
                const auto _X1_X2_D1_t229 = (_X1_X2_D1_t230 - _X1_X2_D1_t234);
                const auto _X1_X2_D1_t246 = (_X1_X2_D1_t25 * _X1_X2_D1_t247);
                const auto _X1_X2_D1_t252 = (gCc12 * _X1_X2_D1_t253);
                const auto _X1_X2_D1_t309 = (gRk2Ck2 * _X1_X2_D1_t310);
                const auto _X1_X2_D1_t313 = (gRk2Ck2 * _X1_X2_D1_t314);
                const auto _X1_X2_D1_t343 = (gRk2Ck2 * _X1_X2_D1_t344);
                const auto _X1_X2_D1_t9 = (_X1_X2_D1_t10 / _12AX7_DEMPWOLF_Ck);
                const auto _X1_X2_D1_t17 = (_12AX7_DEMPWOLF_Gg * _X1_X2_D1_t18);
                const auto _X1_X2_D1_t43 = (_12AX7_DEMPWOLF_Gg * _X1_X2_D1_t44);
                const auto _X1_X2_D1_t77 = (_X1_X2_D1_t78 * _X1_X2_D1_t27);
                const auto _X1_X2_D1_t83 = (gCc12 * _X1_X2_D1_t77);
                const auto _X1_X2_D1_t94 = (_X1_X2_D1_t95 * _X1_X2_D1_t27);
                const auto _X1_X2_D1_t98 = (gCc12 * _X1_X2_D1_t94);
                const auto _X1_X2_D1_t112 = (_X1_X2_D1_t113 / _12AX7_DEMPWOLF_Ck);
                const auto _X1_X2_D1_t122 = (_12AX7_DEMPWOLF_Xi * _X1_X2_D1_t123);
                const auto _X1_X2_D1_t156 = pow(_X1_X2_D1_t9, _X1_X2_D1_t119);
                const auto _X1_X2_D1_t159 = (_12AX7_DEMPWOLF_Xi * _X1_X2_D1_t160);
                const auto _X1_X2_D1_t228 = (gRatt * _X1_X2_D1_t229);
                const auto _X1_X2_D1_t239 = (_X1_X2_D1_t25 * _X1_X2_D1_t228);
                const auto _X1_X2_D1_t245 = (_X1_X2_D1_t62 * _X1_X2_D1_t246);
                const auto _X1_X2_D1_t251 = (_X1_X2_D1_t252 / D1N914_vt);
                const auto _X1_X2_D1_t290 = pow(_X1_X2_D1_t112, _12AX7_DEMPWOLF_Gamma);
                const auto _X1_X2_D1_t308 = (gCout * _X1_X2_D1_t309);
                const auto _X1_X2_D1_t312 = (_X1_X2_D1_t29 * _X1_X2_D1_t313);
                const auto _X1_X2_D1_t342 = (gCout * _X1_X2_D1_t343);
                const auto _X1_X2_D1_t383 = (_X1_X2_D1_t384 * _X1_X2_D1_t156);
                const auto _X1_X2_D1_t8 = pow(_X1_X2_D1_t9, _12AX7_DEMPWOLF_Gamma);
                const auto _X1_X2_D1_t16 = (_X1_X2_D1_t17 + _12AX7_DEMPWOLF_Ig0);
                const auto _X1_X2_D1_t42 = (_X1_X2_D1_t43 + _12AX7_DEMPWOLF_Ig0);
                const auto _X1_X2_D1_t76 = (_X1_X2_D1_t25 * _X1_X2_D1_t77);
                const auto _X1_X2_D1_t82 = (gCc12 * _X1_X2_D1_t83);
                const auto _X1_X2_D1_t93 = (_X1_X2_D1_t25 * _X1_X2_D1_t94);
                const auto _X1_X2_D1_t97 = (gCc12 * _X1_X2_D1_t98);
                const auto _X1_X2_D1_t111 = pow(_X1_X2_D1_t112, _X1_X2_D1_t119);
                const auto _X1_X2_D1_t121 = (_12AX7_DEMPWOLF_Gg * _X1_X2_D1_t122);
                const auto _X1_X2_D1_t155 = (_X1_X2_D1_t156 * _X1_X2_D1_t12);
                const auto _X1_X2_D1_t158 = (_12AX7_DEMPWOLF_Gg * _X1_X2_D1_t159);
                const auto _X1_X2_D1_t217 = (_X1_X2_D1_t121 * _X1_X2_D1_t26);
                const auto _X1_X2_D1_t227 = (gCc12 * _X1_X2_D1_t228);
                const auto _X1_X2_D1_t238 = (_X1_X2_D1_t62 * _X1_X2_D1_t239);
                const auto _X1_X2_D1_t244 = (_X1_X2_D1_t245 / D1N914_vt);
                const auto _X1_X2_D1_t289 = (_12AX7_DEMPWOLF_Gk * _X1_X2_D1_t290);
                const auto _X1_X2_D1_t307 = (_X1_X2_D1_t308 - _X1_X2_D1_t312);
                const auto _X1_X2_D1_t361 = (gCc12 * _X1_X2_D1_t217);
                const auto _X1_X2_D1_t382 = (_X1_X2_D1_t383 * _X1_X2_D1_t12);
                const auto _X1_X2_D1_t7 = (_12AX7_DEMPWOLF_Gk * _X1_X2_D1_t8);
                const auto _X1_X2_D1_t41 = (zCc12 + _X1_X2_D1_t42);
                const auto _X1_X2_D1_t75 = (_X1_X2_D1_t62 * _X1_X2_D1_t76);
                const auto _X1_X2_D1_t92 = (_X1_X2_D1_t62 * _X1_X2_D1_t93);
                const auto _X1_X2_D1_t96 = (_X1_X2_D1_t97 / D1N914_vt);
                const auto _X1_X2_D1_t110 = (_X1_X2_D1_t111 * _X1_X2_D1_t115);
                const auto _X1_X2_D1_t120 = (_X1_X2_D1_t121 / _X1_X2_D1_t47);
                const auto _X1_X2_D1_t154 = (_12AX7_DEMPWOLF_Gamma * _X1_X2_D1_t155);
                const auto _X1_X2_D1_t157 = (_X1_X2_D1_t158 / _X1_X2_D1_t21);
                const auto _X1_X2_D1_t216 = (_X1_X2_D1_t62 * _X1_X2_D1_t217);
                const auto _X1_X2_D1_t226 = (gCc12 * _X1_X2_D1_t227);
                const auto _X1_X2_D1_t243 = (_X1_X2_D1_t244 - _X1_X2_D1_t251);
                const auto _X1_X2_D1_t288 = (_X1_X2_D1_t289 - _X1_X2_D1_t42);
                const auto _X1_X2_D1_t319 = (_X1_X2_D1_t288 - zCout);
                const auto _X1_X2_D1_t330 = (_X1_X2_D1_t82 - _X1_X2_D1_t75);
                const auto _X1_X2_D1_t341 = (_X1_X2_D1_t319 * _X1_X2_D1_t30);
                const auto _X1_X2_D1_t6 = (_X1_X2_D1_t7 - _X1_X2_D1_t16);
                const auto _X1_X2_D1_t40 = (_X1_X2_D1_t41 * _X1_X2_D1_t26);
                const auto _X1_X2_D1_t70 = (zRk1Ck1 + _X1_X2_D1_t6);
                const auto _X1_X2_D1_t74 = (_X1_X2_D1_t75 - _X1_X2_D1_t82);
                const auto _X1_X2_D1_t91 = (_X1_X2_D1_t92 / D1N914_vt);
                const auto _X1_X2_D1_t109 = (_12AX7_DEMPWOLF_Gamma * _X1_X2_D1_t110);
                const auto _X1_X2_D1_t153 = (_12AX7_DEMPWOLF_Gk * _X1_X2_D1_t154);
                const auto _X1_X2_D1_t225 = (_X1_X2_D1_t226 - _X1_X2_D1_t238);
                const auto _X1_X2_D1_t240 = (_X1_X2_D1_t96 - _X1_X2_D1_t91);
                const auto _X1_X2_D1_t270 = (_X1_X2_D1_t62 * _X1_X2_D1_t40);
                const auto _X1_X2_D1_t287 = (zRk2Ck2 + _X1_X2_D1_t288);
                const auto _X1_X2_D1_t318 = (_X1_X2_D1_t319 * _X1_X2_D1_t37);
                const auto _X1_X2_D1_t340 = (_X1_X2_D1_t341 + _X1_X2_D1_t342);
                const auto _X1_X2_D1_t377 = (_X1_X2_D1_t153 * _X1_X2_D1_t24);
                const auto _X1_X2_D1_t389 = (_X1_X2_D1_t153 * _X1_X2_D1_t60);
                const auto _X1_X2_D1_t5 = (_X1_X2_D1_t6 - zCc12);
                const auto _X1_X2_D1_t39 = (gCc12 * _X1_X2_D1_t40);
                const auto _X1_X2_D1_t69 = (_X1_X2_D1_t70 + _X1_X2_D1_t16);
                const auto _X1_X2_D1_t73 = (gRatt * _X1_X2_D1_t74);
                const auto _X1_X2_D1_t90 = (_X1_X2_D1_t91 - _X1_X2_D1_t96);
                const auto _X1_X2_D1_t108 = (_12AX7_DEMPWOLF_Gk * _X1_X2_D1_t109);
                const auto _X1_X2_D1_t152 = (_X1_X2_D1_t153 / _X1_X2_D1_t11);
                const auto _X1_X2_D1_t168 = (_X1_X2_D1_t108 * _X1_X2_D1_t37);
                const auto _X1_X2_D1_t185 = (_X1_X2_D1_t108 * _X1_X2_D1_t31);
                const auto _X1_X2_D1_t188 = (_X1_X2_D1_t108 * _X1_X2_D1_t38);
                const auto _X1_X2_D1_t202 = (_X1_X2_D1_t108 * _X1_X2_D1_t30);
                const auto _X1_X2_D1_t271 = (_X1_X2_D1_t5 * _X1_X2_D1_t60);
                const auto _X1_X2_D1_t286 = (_X1_X2_D1_t287 + _X1_X2_D1_t42);
                const auto _X1_X2_D1_t292 = (_X1_X2_D1_t286 * _X1_X2_D1_t38);
                const auto _X1_X2_D1_t306 = (_X1_X2_D1_t307 - _X1_X2_D1_t318);
                const auto _X1_X2_D1_t339 = (gRatt * _X1_X2_D1_t340);
                const auto _X1_X2_D1_t347 = (gCc12 * _X1_X2_D1_t339);
                const auto _X1_X2_D1_t4 = (_X1_X2_D1_t5 * _X1_X2_D1_t24);
                const auto _X1_X2_D1_t68 = (_X1_X2_D1_t69 * _X1_X2_D1_t56);
                const auto _X1_X2_D1_t72 = (_X1_X2_D1_t73 - _X1_X2_D1_t84);
                const auto _X1_X2_D1_t88 = (_X1_X2_D1_t89 * _X1_X2_D1_t90);
                const auto _X1_X2_D1_t107 = (_X1_X2_D1_t108 / _X1_X2_D1_t114);
                const auto _X1_X2_D1_t151 = (_X1_X2_D1_t152 - _X1_X2_D1_t157);
                const auto _X1_X2_D1_t167 = (gRatt * _X1_X2_D1_t168);
                const auto _X1_X2_D1_t172 = (gCc12 * _X1_X2_D1_t167);
                const auto _X1_X2_D1_t184 = (_X1_X2_D1_t29 * _X1_X2_D1_t185);
                const auto _X1_X2_D1_t187 = (gCout * _X1_X2_D1_t188);
                const auto _X1_X2_D1_t195 = (_X1_X2_D1_t151 + _X1_X2_D1_t157);
                const auto _X1_X2_D1_t201 = (gRatt * _X1_X2_D1_t202);
                const auto _X1_X2_D1_t205 = (gCc12 * _X1_X2_D1_t201);
                const auto _X1_X2_D1_t221 = (_X1_X2_D1_t195 * _X1_X2_D1_t56);
                const auto _X1_X2_D1_t256 = (_X1_X2_D1_t151 * _X1_X2_D1_t24);
                const auto _X1_X2_D1_t269 = (_X1_X2_D1_t270 + _X1_X2_D1_t271);
                const auto _X1_X2_D1_t285 = (_X1_X2_D1_t286 * _X1_X2_D1_t31);
                const auto _X1_X2_D1_t291 = (gCout * _X1_X2_D1_t292);
                const auto _X1_X2_D1_t305 = (gRatt * _X1_X2_D1_t306);
                const auto _X1_X2_D1_t321 = (_X1_X2_D1_t25 * _X1_X2_D1_t305);
                const auto _X1_X2_D1_t338 = (_X1_X2_D1_t25 * _X1_X2_D1_t339);
                const auto _X1_X2_D1_t346 = (gCc12 * _X1_X2_D1_t347);
                const auto _X1_X2_D1_t381 = (_X1_X2_D1_t382 * _X1_X2_D1_t56);
                const auto _X1_X2_D1_t3 = (_X1_X2_D1_t4 + _X1_X2_D1_t39);
                const auto _X1_X2_D1_t71 = (gRfb * _X1_X2_D1_t72);
                const auto _X1_X2_D1_t106 = (_X1_X2_D1_t107 - _X1_X2_D1_t120);
                const auto _X1_X2_D1_t140 = (_X1_X2_D1_t106 + _X1_X2_D1_t120);
                const auto _X1_X2_D1_t142 = (_X1_X2_D1_t140 * _X1_X2_D1_t38);
                const auto _X1_X2_D1_t150 = (_X1_X2_D1_t151 * _X1_X2_D1_t60);
                const auto _X1_X2_D1_t166 = (_X1_X2_D1_t25 * _X1_X2_D1_t167);
                const auto _X1_X2_D1_t171 = (gCc12 * _X1_X2_D1_t172);
                const auto _X1_X2_D1_t183 = (_X1_X2_D1_t184 / _X1_X2_D1_t169);
                const auto _X1_X2_D1_t186 = (_X1_X2_D1_t187 / _X1_X2_D1_t169);
                const auto _X1_X2_D1_t200 = (_X1_X2_D1_t25 * _X1_X2_D1_t201);
                const auto _X1_X2_D1_t204 = (gCc12 * _X1_X2_D1_t205);
                const auto _X1_X2_D1_t212 = (_X1_X2_D1_t106 * _X1_X2_D1_t37);
                const auto _X1_X2_D1_t268 = (_X1_X2_D1_t269 - _X1_X2_D1_t272);
                const auto _X1_X2_D1_t284 = (_X1_X2_D1_t29 * _X1_X2_D1_t285);
                const auto _X1_X2_D1_t304 = (gCc12 * _X1_X2_D1_t305);
                const auto _X1_X2_D1_t320 = (_X1_X2_D1_t62 * _X1_X2_D1_t321);
                const auto _X1_X2_D1_t337 = (_X1_X2_D1_t62 * _X1_X2_D1_t338);
                const auto _X1_X2_D1_t380 = (_X1_X2_D1_t381 / _X1_X2_D1_t378);
                const auto _X1_X2_D1_t2 = (_X1_X2_D1_t3 - _X1_X2_D1_t50);
                const auto _X1_X2_D1_t67 = (_X1_X2_D1_t68 + _X1_X2_D1_t71);
                const auto _X1_X2_D1_t87 = (_X1_X2_D1_t88 * _X1_X2_D1_t54);
                const auto _X1_X2_D1_t105 = (_X1_X2_D1_t106 * _X1_X2_D1_t30);
                const auto _X1_X2_D1_t139 = (_X1_X2_D1_t140 * _X1_X2_D1_t31);
                const auto _X1_X2_D1_t141 = (gCout * _X1_X2_D1_t142);
                const auto _X1_X2_D1_t149 = (((_X1_X2_D1_t1 * ((gRatt * _X1_X2_D1_t150) - (_X1_X2_D1_t52 * _X1_X2_D1_t150))) + (gRfb * (gRfb * _X1_X2_D1_t150))) * _X1_X2_D1_t54);
                const auto _X1_X2_D1_t165 = (_X1_X2_D1_t62 * _X1_X2_D1_t166);
                const auto _X1_X2_D1_t170 = (_X1_X2_D1_t171 / _X1_X2_D1_t169);
                const auto _X1_X2_D1_t182 = (_X1_X2_D1_t183 - _X1_X2_D1_t186);
                const auto _X1_X2_D1_t194 = (((_X1_X2_D1_t195 * gRfb) * _X1_X2_D1_t58) * _X1_X2_D1_t54);
                const auto _X1_X2_D1_t199 = (_X1_X2_D1_t62 * _X1_X2_D1_t200);
                const auto _X1_X2_D1_t203 = (_X1_X2_D1_t204 / _X1_X2_D1_t169);
                const auto _X1_X2_D1_t211 = (gRatt * _X1_X2_D1_t212);
                const auto _X1_X2_D1_t214 = (gCc12 * _X1_X2_D1_t211);
                const auto _X1_X2_D1_t220 = (_X1_X2_D1_t221 * _X1_X2_D1_t54);
                const auto _X1_X2_D1_t224 = (((((_X1_X2_D1_t1 * ((gRatt * _X1_X2_D1_t225) - (_X1_X2_D1_t52 * _X1_X2_D1_t225))) + (gRfb * (gRfb * _X1_X2_D1_t225))) * _X1_X2_D1_t54) + (((_X1_X2_D1_t1 * (_X1_X2_D1_t52 * _X1_X2_D1_t240)) - (gRfb * (gRfb * _X1_X2_D1_t240))) * _X1_X2_D1_t54)) - 1.0);
                const auto _X1_X2_D1_t242 = (((_X1_X2_D1_t1 * ((gRatt * _X1_X2_D1_t243) - (_X1_X2_D1_t52 * _X1_X2_D1_t243))) + (gRfb * (gRfb * _X1_X2_D1_t243))) * _X1_X2_D1_t54);
                const auto _X1_X2_D1_t255 = ((((_X1_X2_D1_t1 * ((gRatt * _X1_X2_D1_t256) - (_X1_X2_D1_t52 * _X1_X2_D1_t256))) + (gRfb * (gRfb * _X1_X2_D1_t256))) * _X1_X2_D1_t54) - _X1_X2_D1_t220);
                const auto _X1_X2_D1_t283 = (_X1_X2_D1_t284 - _X1_X2_D1_t291);
                const auto _X1_X2_D1_t303 = (gCc12 * _X1_X2_D1_t304);
                const auto _X1_X2_D1_t336 = (_X1_X2_D1_t337 - _X1_X2_D1_t346);
                const auto _X1_X2_D1_t360 = (((_X1_X2_D1_t1 * (((gRatt * _X1_X2_D1_t361) / _X1_X2_D1_t47) - ((_X1_X2_D1_t52 * _X1_X2_D1_t361) / _X1_X2_D1_t47))) + ((gRfb * (gRfb * _X1_X2_D1_t361)) / _X1_X2_D1_t47)) * _X1_X2_D1_t54);
                const auto _X1_X2_D1_t364 = (_X1_X2_D1_t149 * _X1_X2_D1_t224);
                const auto _X1_X2_D1_t379 = (_X1_X2_D1_t380 * _X1_X2_D1_t54);
                const auto _X1_X2_D1_t388 = (((_X1_X2_D1_t1 * (((gRatt * _X1_X2_D1_t389) / _X1_X2_D1_t378) - ((_X1_X2_D1_t52 * _X1_X2_D1_t389) / _X1_X2_D1_t378))) + ((gRfb * (gRfb * _X1_X2_D1_t389)) / _X1_X2_D1_t378)) * _X1_X2_D1_t54);
                const auto _X1_X2_D1_t390 = ((((_X1_X2_D1_t382 * gRfb) * _X1_X2_D1_t58) / _X1_X2_D1_t378) * _X1_X2_D1_t54);
                const auto _X1_X2_D1_t394 = (_X1_X2_D1_t388 * _X1_X2_D1_t224);
                const auto _X1_X2_D1_t396 = ((_X1_X2_D1_t388 * _X1_X2_D1_t194) - (_X1_X2_D1_t149 * _X1_X2_D1_t390));
                const auto _X1_X2_D1_t66 = (_X1_X2_D1_t67 * _X1_X2_D1_t54);
                const auto _X1_X2_D1_t104 = (gRatt * _X1_X2_D1_t105);
                const auto _X1_X2_D1_t127 = (gCc12 * _X1_X2_D1_t104);
                const auto _X1_X2_D1_t138 = (_X1_X2_D1_t29 * _X1_X2_D1_t139);
                const auto _X1_X2_D1_t164 = (_X1_X2_D1_t165 / _X1_X2_D1_t169);
                const auto _X1_X2_D1_t181 = (gRatt * _X1_X2_D1_t182);
                const auto _X1_X2_D1_t190 = (_X1_X2_D1_t25 * _X1_X2_D1_t181);
                const auto _X1_X2_D1_t198 = (_X1_X2_D1_t199 / _X1_X2_D1_t169);
                const auto _X1_X2_D1_t210 = (_X1_X2_D1_t25 * _X1_X2_D1_t211);
                const auto _X1_X2_D1_t213 = (gCc12 * _X1_X2_D1_t214);
                const auto _X1_X2_D1_t218 = ((((((_X1_X2_D1_t423 * _X1_X2_D1_t161) * _X1_X2_D1_t22) / _X1_X2_D1_t21) * _X1_X2_D1_t219) + _X1_X2_D1_t220) + 1.0);
                const auto _X1_X2_D1_t260 = (((_X1_X2_D1_t16 - _X1_X2_D1_t424) * _X1_X2_D1_t219) + _X1_X2_D1_t66);
                const auto _X1_X2_D1_t282 = (gRatt * _X1_X2_D1_t283);
                const auto _X1_X2_D1_t294 = (_X1_X2_D1_t25 * _X1_X2_D1_t282);
                const auto _X1_X2_D1_t302 = (_X1_X2_D1_t303 - _X1_X2_D1_t320);
                const auto _X1_X2_D1_t335 = (_X1_X2_D1_t336 + _X1_X2_D1_t348);
                const auto _X1_X2_D1_t376 = (((((_X1_X2_D1_t1 * (((gRatt * _X1_X2_D1_t377) / _X1_X2_D1_t378) - ((_X1_X2_D1_t52 * _X1_X2_D1_t377) / _X1_X2_D1_t378))) + ((gRfb * (gRfb * _X1_X2_D1_t377)) / _X1_X2_D1_t378)) * _X1_X2_D1_t54) - _X1_X2_D1_t379) - 1.0);
                const auto _X1_X2_D1_t0 = ((((_X1_X2_D1_t1 * ((gRatt * _X1_X2_D1_t2) - (_X1_X2_D1_t52 * _X1_X2_D1_t2))) + (gRfb * (gRfb * _X1_X2_D1_t2))) * _X1_X2_D1_t54) - _X1_X2_D1_t66);
                const auto _X1_X2_D1_t85 = (vPKX1 - _X1_X2_D1_t0);
                const auto _X1_X2_D1_t103 = (_X1_X2_D1_t25 * _X1_X2_D1_t104);
                const auto _X1_X2_D1_t126 = (gCc12 * _X1_X2_D1_t127);
                const auto _X1_X2_D1_t137 = (_X1_X2_D1_t138 - _X1_X2_D1_t141);
                const auto _X1_X2_D1_t163 = (_X1_X2_D1_t164 - _X1_X2_D1_t170);
                const auto _X1_X2_D1_t180 = (gCc12 * _X1_X2_D1_t181);
                const auto _X1_X2_D1_t189 = (_X1_X2_D1_t62 * _X1_X2_D1_t190);
                const auto _X1_X2_D1_t197 = (_X1_X2_D1_t198 - _X1_X2_D1_t203);
                const auto _X1_X2_D1_t209 = (_X1_X2_D1_t62 * _X1_X2_D1_t210);
                const auto _X1_X2_D1_t259 = (_X1_X2_D1_t260 + vGKX1);
                const auto _X1_X2_D1_t281 = (gCc12 * _X1_X2_D1_t282);
                const auto _X1_X2_D1_t293 = (_X1_X2_D1_t62 * _X1_X2_D1_t294);
                const auto _X1_X2_D1_t301 = (_X1_X2_D1_t302 - _X1_X2_D1_t322);
                const auto _X1_X2_D1_t102 = (_X1_X2_D1_t62 * _X1_X2_D1_t103);
                const auto _X1_X2_D1_t136 = (gRatt * _X1_X2_D1_t137);
                const auto _X1_X2_D1_t144 = (_X1_X2_D1_t25 * _X1_X2_D1_t136);
                const auto _X1_X2_D1_t162 = (((_X1_X2_D1_t1 * ((gRatt * _X1_X2_D1_t163) - (_X1_X2_D1_t52 * _X1_X2_D1_t163))) + (gRfb * (gRfb * _X1_X2_D1_t163))) * _X1_X2_D1_t54);
                const auto _X1_X2_D1_t179 = (gCc12 * _X1_X2_D1_t180);
                const auto _X1_X2_D1_t208 = (_X1_X2_D1_t209 - _X1_X2_D1_t213);
                const auto _X1_X2_D1_t280 = (gCc12 * _X1_X2_D1_t281);
                const auto _X1_X2_D1_t300 = ((((_X1_X2_D1_t1 * ((gRatt * _X1_X2_D1_t301) - (_X1_X2_D1_t52 * _X1_X2_D1_t301))) + (gRfb * (gRfb * _X1_X2_D1_t301))) * _X1_X2_D1_t54) + ((((_X1_X2_D1_t1 * ((_X1_X2_D1_t52 * _X1_X2_D1_t330) + _X1_X2_D1_t84)) - (gRfb * (gRfb * _X1_X2_D1_t330))) + (_X1_X2_D1_t69 * _X1_X2_D1_t65)) * _X1_X2_D1_t54));
                const auto _X1_X2_D1_t101 = (_X1_X2_D1_t102 - _X1_X2_D1_t126);
                const auto _X1_X2_D1_t135 = (gCc12 * _X1_X2_D1_t136);
                const auto _X1_X2_D1_t143 = (_X1_X2_D1_t62 * _X1_X2_D1_t144);
                const auto _X1_X2_D1_t178 = (_X1_X2_D1_t179 - _X1_X2_D1_t189);
                const auto _X1_X2_D1_t191 = (_X1_X2_D1_t52 * _X1_X2_D1_t178);
                const auto _X1_X2_D1_t193 = (gRfb * _X1_X2_D1_t178);
                const auto _X1_X2_D1_t207 = (((_X1_X2_D1_t1 * ((gRatt * _X1_X2_D1_t208) - (_X1_X2_D1_t52 * _X1_X2_D1_t208))) + (gRfb * (gRfb * _X1_X2_D1_t208))) * _X1_X2_D1_t54);
                const auto _X1_X2_D1_t279 = (_X1_X2_D1_t280 - _X1_X2_D1_t293);
                const auto _X1_X2_D1_t295 = (_X1_X2_D1_t52 * _X1_X2_D1_t279);
                const auto _X1_X2_D1_t297 = (gRfb * _X1_X2_D1_t279);
                const auto _X1_X2_D1_t299 = (vD1 - _X1_X2_D1_t300);
                const auto _X1_X2_D1_t373 = (_X1_X2_D1_t149 * _X1_X2_D1_t299);
                const auto _X1_X2_D1_t400 = (_X1_X2_D1_t388 * _X1_X2_D1_t299);
                const auto _X1_X2_D1_t134 = (gCc12 * _X1_X2_D1_t135);
                const auto _X1_X2_D1_t177 = (gRatt * _X1_X2_D1_t178);
                const auto _X1_X2_D1_t192 = (gRfb * _X1_X2_D1_t193);
                const auto _X1_X2_D1_t278 = (gRatt * _X1_X2_D1_t279);
                const auto _X1_X2_D1_t296 = (gRfb * _X1_X2_D1_t297);
                const auto _X1_X2_D1_t133 = (_X1_X2_D1_t134 - _X1_X2_D1_t143);
                const auto _X1_X2_D1_t145 = (_X1_X2_D1_t52 * _X1_X2_D1_t133);
                const auto _X1_X2_D1_t147 = (gRfb * _X1_X2_D1_t133);
                const auto _X1_X2_D1_t176 = (_X1_X2_D1_t177 - _X1_X2_D1_t191);
                const auto _X1_X2_D1_t277 = (_X1_X2_D1_t278 - _X1_X2_D1_t295);
                const auto _X1_X2_D1_t132 = (gRatt * _X1_X2_D1_t133);
                const auto _X1_X2_D1_t146 = (gRfb * _X1_X2_D1_t147);
                const auto _X1_X2_D1_t175 = (_X1_X2_D1_t1 * _X1_X2_D1_t176);
                const auto _X1_X2_D1_t276 = (_X1_X2_D1_t1 * _X1_X2_D1_t277);
                const auto _X1_X2_D1_t131 = (_X1_X2_D1_t132 - _X1_X2_D1_t145);
                const auto _X1_X2_D1_t174 = (_X1_X2_D1_t175 + _X1_X2_D1_t192);
                const auto _X1_X2_D1_t275 = (_X1_X2_D1_t276 + _X1_X2_D1_t296);
                const auto _X1_X2_D1_t130 = (_X1_X2_D1_t1 * _X1_X2_D1_t131);
                const auto _X1_X2_D1_t173 = (_X1_X2_D1_t174 * _X1_X2_D1_t54);
                const auto _X1_X2_D1_t196 = (((((_X1_X2_D1_t1 * ((gRatt * _X1_X2_D1_t197) - (_X1_X2_D1_t52 * _X1_X2_D1_t197))) + (gRfb * (gRfb * _X1_X2_D1_t197))) * _X1_X2_D1_t54) - _X1_X2_D1_t173) - 1.0);
                const auto _X1_X2_D1_t241 = (_X1_X2_D1_t173 * _X1_X2_D1_t224);
                const auto _X1_X2_D1_t274 = (_X1_X2_D1_t275 * _X1_X2_D1_t54);
                const auto _X1_X2_D1_t298 = (_X1_X2_D1_t173 * _X1_X2_D1_t299);
                const auto _X1_X2_D1_t334 = ((((_X1_X2_D1_t1 * ((gRatt * _X1_X2_D1_t335) - (_X1_X2_D1_t52 * _X1_X2_D1_t335))) + (gRfb * (gRfb * _X1_X2_D1_t335))) * _X1_X2_D1_t54) - _X1_X2_D1_t274);
                const auto _X1_X2_D1_t387 = ((_X1_X2_D1_t388 * _X1_X2_D1_t162) + (_X1_X2_D1_t173 * _X1_X2_D1_t390));
                const auto _X1_X2_D1_t393 = ((_X1_X2_D1_t242 * _X1_X2_D1_t387) - (_X1_X2_D1_t196 * _X1_X2_D1_t394));
                const auto _X1_X2_D1_t395 = (_X1_X2_D1_t196 * _X1_X2_D1_t396);
                const auto _X1_X2_D1_t403 = (_X1_X2_D1_t259 * _X1_X2_D1_t393);
                const auto _X1_X2_D1_t421 = (_X1_X2_D1_t259 * _X1_X2_D1_t395);
                const auto _X1_X2_D1_t129 = (_X1_X2_D1_t130 + _X1_X2_D1_t146);
                const auto _X1_X2_D1_t148 = ((_X1_X2_D1_t149 * _X1_X2_D1_t162) + (_X1_X2_D1_t173 * _X1_X2_D1_t194));
                const auto _X1_X2_D1_t267 = ((((_X1_X2_D1_t1 * ((gRatt * _X1_X2_D1_t268) - (_X1_X2_D1_t52 * _X1_X2_D1_t268))) + (gRfb * (gRfb * _X1_X2_D1_t268))) * _X1_X2_D1_t54) - _X1_X2_D1_t274);
                const auto _X1_X2_D1_t333 = (vPKX2 - _X1_X2_D1_t334);
                const auto _X1_X2_D1_t363 = ((_X1_X2_D1_t242 * _X1_X2_D1_t148) - (_X1_X2_D1_t196 * _X1_X2_D1_t364));
                const auto _X1_X2_D1_t368 = (_X1_X2_D1_t333 * _X1_X2_D1_t241);
                const auto _X1_X2_D1_t371 = (_X1_X2_D1_t333 * _X1_X2_D1_t148);
                const auto _X1_X2_D1_t392 = (((_X1_X2_D1_t218 * _X1_X2_D1_t393) - (_X1_X2_D1_t379 * _X1_X2_D1_t363)) + (_X1_X2_D1_t87 * _X1_X2_D1_t395));
                const auto _X1_X2_D1_t402 = (_X1_X2_D1_t333 * _X1_X2_D1_t387);
                const auto _X1_X2_D1_t128 = (_X1_X2_D1_t129 * _X1_X2_D1_t54);
                const auto _X1_X2_D1_t215 = (((((_X1_X2_D1_t1 * (((gRatt * _X1_X2_D1_t216) / _X1_X2_D1_t47) - ((_X1_X2_D1_t52 * _X1_X2_D1_t216) / _X1_X2_D1_t47))) + ((gRfb * (gRfb * _X1_X2_D1_t216)) / _X1_X2_D1_t47)) * _X1_X2_D1_t54) - _X1_X2_D1_t128) - 1.0);
                const auto _X1_X2_D1_t223 = (_X1_X2_D1_t215 * _X1_X2_D1_t224);
                const auto _X1_X2_D1_t254 = ((_X1_X2_D1_t173 * _X1_X2_D1_t207) + (_X1_X2_D1_t215 * _X1_X2_D1_t162));
                const auto _X1_X2_D1_t266 = (vGKX2 - _X1_X2_D1_t267);
                const auto _X1_X2_D1_t332 = (_X1_X2_D1_t333 * _X1_X2_D1_t254);
                const auto _X1_X2_D1_t358 = (_X1_X2_D1_t266 * _X1_X2_D1_t207);
                const auto _X1_X2_D1_t359 = (_X1_X2_D1_t215 * _X1_X2_D1_t299);
                const auto _X1_X2_D1_t362 = (_X1_X2_D1_t259 * _X1_X2_D1_t363);
                const auto _X1_X2_D1_t370 = (_X1_X2_D1_t266 * _X1_X2_D1_t224);
                const auto _X1_X2_D1_t372 = (_X1_X2_D1_t266 * _X1_X2_D1_t194);
                const auto _X1_X2_D1_t391 = ((_X1_X2_D1_t388 * _X1_X2_D1_t207) - (_X1_X2_D1_t215 * _X1_X2_D1_t390));
                const auto _X1_X2_D1_t401 = (_X1_X2_D1_t266 * _X1_X2_D1_t390);
                const auto _X1_X2_D1_t406 = (_X1_X2_D1_t373 - _X1_X2_D1_t372);
                const auto _X1_X2_D1_t415 = (_X1_X2_D1_t196 * _X1_X2_D1_t406);
                const auto _X1_X2_D1_t100 = ((((_X1_X2_D1_t1 * ((gRatt * _X1_X2_D1_t101) - (_X1_X2_D1_t52 * _X1_X2_D1_t101))) + (gRfb * (gRfb * _X1_X2_D1_t101))) * _X1_X2_D1_t54) - _X1_X2_D1_t128);
                const auto _X1_X2_D1_t206 = ((_X1_X2_D1_t149 * _X1_X2_D1_t207) - (_X1_X2_D1_t215 * _X1_X2_D1_t194));
                const auto _X1_X2_D1_t222 = (((_X1_X2_D1_t196 * _X1_X2_D1_t223) + (_X1_X2_D1_t100 * _X1_X2_D1_t241)) - (_X1_X2_D1_t242 * _X1_X2_D1_t254));
                const auto _X1_X2_D1_t258 = (_X1_X2_D1_t259 * _X1_X2_D1_t222);
                const auto _X1_X2_D1_t265 = (_X1_X2_D1_t266 * _X1_X2_D1_t162);
                const auto _X1_X2_D1_t357 = (_X1_X2_D1_t358 - _X1_X2_D1_t359);
                const auto _X1_X2_D1_t369 = (_X1_X2_D1_t196 * _X1_X2_D1_t370);
                const auto _X1_X2_D1_t386 = ((_X1_X2_D1_t100 * _X1_X2_D1_t387) - (_X1_X2_D1_t196 * _X1_X2_D1_t391));
                const auto _X1_X2_D1_t399 = (_X1_X2_D1_t400 - _X1_X2_D1_t401);
                const auto _X1_X2_D1_t404 = (_X1_X2_D1_t259 * _X1_X2_D1_t386);
                const auto _X1_X2_D1_t405 = ((_X1_X2_D1_t100 * _X1_X2_D1_t406) - (_X1_X2_D1_t333 * _X1_X2_D1_t206));
                const auto _X1_X2_D1_t407 = (((_X1_X2_D1_t333 * _X1_X2_D1_t223) - (_X1_X2_D1_t100 * _X1_X2_D1_t370)) + (_X1_X2_D1_t242 * _X1_X2_D1_t357));
                const auto _X1_X2_D1_t408 = ((_X1_X2_D1_t242 * _X1_X2_D1_t206) - (_X1_X2_D1_t100 * _X1_X2_D1_t364));
                const auto _X1_X2_D1_t409 = ((_X1_X2_D1_t100 * _X1_X2_D1_t399) - (_X1_X2_D1_t333 * _X1_X2_D1_t391));
                const auto _X1_X2_D1_t410 = ((_X1_X2_D1_t242 * _X1_X2_D1_t391) - (_X1_X2_D1_t100 * _X1_X2_D1_t394));
                const auto _X1_X2_D1_t411 = (_X1_X2_D1_t100 * _X1_X2_D1_t396);
                const auto _X1_X2_D1_t413 = (_X1_X2_D1_t298 + _X1_X2_D1_t265);
                const auto _X1_X2_D1_t414 = (_X1_X2_D1_t371 - _X1_X2_D1_t415);
                const auto _X1_X2_D1_t420 = (_X1_X2_D1_t379 * _X1_X2_D1_t414);
                const auto _X1_X2_D1_t422 = (((_X1_X2_D1_t196 * (_X1_X2_D1_t359 - _X1_X2_D1_t358)) + (_X1_X2_D1_t100 * _X1_X2_D1_t413)) - _X1_X2_D1_t332);
                const auto _X1_X2_D1_t99 = ((_X1_X2_D1_t100 * _X1_X2_D1_t148) - (_X1_X2_D1_t196 * _X1_X2_D1_t206));
                const auto _X1_X2_D1_t264 = (_X1_X2_D1_t265 + _X1_X2_D1_t298);
                const auto _X1_X2_D1_t356 = (_X1_X2_D1_t196 * _X1_X2_D1_t357);
                const auto _X1_X2_D1_t366 = (_X1_X2_D1_t242 * _X1_X2_D1_t264);
                const auto _X1_X2_D1_t367 = (_X1_X2_D1_t368 + _X1_X2_D1_t369);
                const auto _X1_X2_D1_t374 = (_X1_X2_D1_t259 * _X1_X2_D1_t99);
                const auto _X1_X2_D1_t385 = ((_X1_X2_D1_t87 * _X1_X2_D1_t386) - (_X1_X2_D1_t379 * _X1_X2_D1_t222));
                const auto _X1_X2_D1_t397 = ((_X1_X2_D1_t218 * _X1_X2_D1_t386) - (_X1_X2_D1_t379 * _X1_X2_D1_t99));
                const auto _X1_X2_D1_t398 = (_X1_X2_D1_t196 * _X1_X2_D1_t399);
                const auto _X1_X2_D1_t412 = ((_X1_X2_D1_t369 + _X1_X2_D1_t368) - (_X1_X2_D1_t242 * _X1_X2_D1_t413));
                const auto _X1_X2_D1_t416 = (_X1_X2_D1_t402 - _X1_X2_D1_t398);
                const auto _X1_X2_D1_t419 = (_X1_X2_D1_t218 * _X1_X2_D1_t416);
                const auto _X1_X2_D1_t86 = ((_X1_X2_D1_t87 * _X1_X2_D1_t99) - (_X1_X2_D1_t218 * _X1_X2_D1_t222));
                const auto _X1_X2_D1_t263 = (_X1_X2_D1_t100 * _X1_X2_D1_t264);
                const auto _X1_X2_D1_t331 = (_X1_X2_D1_t332 + _X1_X2_D1_t356);
                const auto _X1_X2_D1_t365 = (_X1_X2_D1_t366 - _X1_X2_D1_t367);
                const auto _X1_X2_D1_t375 = ((((_X1_X2_D1_t376 * _X1_X2_D1_t86) - (_X1_X2_D1_t255 * _X1_X2_D1_t385)) - (_X1_X2_D1_t360 * _X1_X2_D1_t392)) - (_X1_X2_D1_t87 * _X1_X2_D1_t397));
                const auto _X1_X2_D1_t418 = (_X1_X2_D1_t419 - _X1_X2_D1_t420);
                const auto _X1_X2_D1_t262 = (_X1_X2_D1_t263 - _X1_X2_D1_t331);
                const auto _X1_X2_D1_t417 = (_X1_X2_D1_t418 - _X1_X2_D1_t421);
                const auto _X1_X2_D1_t261 = (_X1_X2_D1_t87 * _X1_X2_D1_t262);
                const auto _X1_X2_D1_t257 = (_X1_X2_D1_t258 + _X1_X2_D1_t261);
                const auto res_vPKX1 = (_X1_X2_D1_t0 - vPKX1);
                const auto delta_vPKX1 = (((((_X1_X2_D1_t85 * _X1_X2_D1_t86) - (_X1_X2_D1_t255 * _X1_X2_D1_t257)) - (_X1_X2_D1_t360 * ((_X1_X2_D1_t362 + (_X1_X2_D1_t218 * _X1_X2_D1_t365)) + (_X1_X2_D1_t87 * (_X1_X2_D1_t371 + (_X1_X2_D1_t196 * (_X1_X2_D1_t372 - _X1_X2_D1_t373))))))) - (_X1_X2_D1_t87 * (_X1_X2_D1_t374 + (_X1_X2_D1_t218 * _X1_X2_D1_t262)))) / _X1_X2_D1_t375);
                const auto res_vGKX1 = (-_X1_X2_D1_t259);
                const auto delta_vGKX1 = (((((_X1_X2_D1_t376 * _X1_X2_D1_t257) - (_X1_X2_D1_t85 * _X1_X2_D1_t385)) - (_X1_X2_D1_t360 * ((_X1_X2_D1_t87 * (_X1_X2_D1_t398 - _X1_X2_D1_t402)) - ((_X1_X2_D1_t379 * _X1_X2_D1_t365) + _X1_X2_D1_t403)))) + (_X1_X2_D1_t87 * ((_X1_X2_D1_t379 * _X1_X2_D1_t262) + _X1_X2_D1_t404))) / _X1_X2_D1_t375);
                const auto res_vPKX2 = (_X1_X2_D1_t334 - vPKX2);
                const auto delta_vPKX2 = ((((((_X1_X2_D1_t376 * ((_X1_X2_D1_t87 * _X1_X2_D1_t405) - ((_X1_X2_D1_t218 * _X1_X2_D1_t407) + (_X1_X2_D1_t259 * _X1_X2_D1_t408)))) - (_X1_X2_D1_t255 * ((_X1_X2_D1_t87 * _X1_X2_D1_t409) - ((_X1_X2_D1_t379 * _X1_X2_D1_t407) + (_X1_X2_D1_t259 * _X1_X2_D1_t410))))) + (_X1_X2_D1_t85 * (((_X1_X2_D1_t218 * _X1_X2_D1_t410) - (_X1_X2_D1_t379 * _X1_X2_D1_t408)) + (_X1_X2_D1_t87 * _X1_X2_D1_t411)))) - (_X1_X2_D1_t360 * ((((_X1_X2_D1_t218 * ((_X1_X2_D1_t242 * _X1_X2_D1_t399) - (_X1_X2_D1_t333 * _X1_X2_D1_t394))) - (_X1_X2_D1_t379 * ((_X1_X2_D1_t242 * _X1_X2_D1_t406) - (_X1_X2_D1_t333 * _X1_X2_D1_t364)))) + (_X1_X2_D1_t259 * (_X1_X2_D1_t242 * _X1_X2_D1_t396))) + (_X1_X2_D1_t87 * (_X1_X2_D1_t333 * _X1_X2_D1_t396))))) - (_X1_X2_D1_t87 * (((_X1_X2_D1_t218 * _X1_X2_D1_t409) - (_X1_X2_D1_t379 * _X1_X2_D1_t405)) + (_X1_X2_D1_t259 * _X1_X2_D1_t411)))) / _X1_X2_D1_t375);
                const auto res_vGKX2 = (_X1_X2_D1_t267 - vGKX2);
                const auto delta_vGKX2 = (((((_X1_X2_D1_t376 * ((_X1_X2_D1_t362 - (_X1_X2_D1_t218 * _X1_X2_D1_t412)) + (_X1_X2_D1_t87 * _X1_X2_D1_t414))) - (_X1_X2_D1_t255 * ((_X1_X2_D1_t403 - (_X1_X2_D1_t379 * _X1_X2_D1_t412)) + (_X1_X2_D1_t87 * _X1_X2_D1_t416)))) - (_X1_X2_D1_t85 * _X1_X2_D1_t392)) - (_X1_X2_D1_t87 * _X1_X2_D1_t417)) / _X1_X2_D1_t375);
                const auto res_vD1 = (_X1_X2_D1_t300 - vD1);
                const auto delta_vD1 = (((((_X1_X2_D1_t255 * ((_X1_X2_D1_t379 * _X1_X2_D1_t422) + _X1_X2_D1_t404)) - (_X1_X2_D1_t376 * ((_X1_X2_D1_t218 * _X1_X2_D1_t422) + _X1_X2_D1_t374))) - (_X1_X2_D1_t360 * _X1_X2_D1_t417)) + (_X1_X2_D1_t85 * _X1_X2_D1_t397)) / _X1_X2_D1_t375);
            
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

            const auto _t13 = (zCout - zCsc);
            const auto _t16 = (D1N914_Is * (exp((vD1 / D1N914_vt)) - 1.0));
            const auto _t21 = (_12AX7_DEMPWOLF_Gg * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi));
            const auto _t49 = (_12AX7_DEMPWOLF_Gg * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi));
            const auto _t12 = (_t13 * gCsc);
            const auto _t15 = (zCsc + _t16);
            const auto _t20 = (_t21 + _12AX7_DEMPWOLF_Ig0);
            const auto _t48 = (_t49 + _12AX7_DEMPWOLF_Ig0);
            const auto _t51 = ((_12AX7_DEMPWOLF_Gk * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - _t48);
            const auto _t64 = (_t51 - zCc12);
            const auto _t67 = (zCc12 + _t20);
            const auto _t14 = (gCsc * _t15);
            const auto _t19 = ((_12AX7_DEMPWOLF_Gk * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Ck * ((vPKX2 / _12AX7_DEMPWOLF_Mu) + vGKX2))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - _t20);
            const auto _t50 = ((zRk1Ck1 + _t51) + _t48);
            const auto _t107 = ((zRk2Ck2 + _t19) + _t20);
            const auto _t11 = (_t12 + _t14);
            const auto _t18 = (_t19 - zCout);
            const auto _t57 = (_t16 * _t41);
            const auto _t59 = (gCc12 * _t57);
            const auto _t106 = ((_t9 * (_t107 * _t43)) - (gCout * (_t107 * _t23)));
            const auto _t10 = (gRk2Ck2 * _t11);
            const auto _t17 = (_t18 * _t22);
            const auto _t56 = (_t6 * _t57);
            const auto _t58 = (gCc12 * _t59);
            const auto _t63 = (_t64 * _t46);
            const auto _t66 = (_t67 * _t40);
            const auto _t72 = (_t4 * _t66);
            const auto _t81 = ((_t18 * _t42) + (gCout * _t10));
            const auto _t97 = (((gCout * (gRk2Ck2 * (gCout * _t15))) - (_t9 * (gRk2Ck2 * ((_t44 * _t15) + _t12)))) - _t17);
            const auto _t105 = (gRatt * _t106);
            const auto _t109 = (_t6 * _t105);
            const auto _t8 = ((_t9 * _t10) + _t17);
            const auto _t55 = (_t4 * _t56);
            const auto _t65 = (gCc12 * _t66);
            const auto _t73 = (_t64 * _t39);
            const auto _t80 = (gRatt * _t81);
            const auto _t83 = (gCc12 * _t80);
            const auto _t96 = (gRatt * _t97);
            const auto _t99 = (_t6 * _t96);
            const auto _t104 = (gCc12 * _t105);
            const auto _t108 = (_t4 * _t109);
            const auto _t7 = (gRatt * _t8);
            const auto _t25 = (gCc12 * _t7);
            const auto _t54 = (_t55 - _t58);
            const auto _t62 = (_t63 + _t65);
            const auto _t71 = (_t72 + _t73);
            const auto _t79 = (_t6 * _t80);
            const auto _t82 = (gCc12 * _t83);
            const auto _t95 = (gCc12 * _t96);
            const auto _t98 = (_t4 * _t99);
            const auto _t103 = (gCc12 * _t104);
            const auto _t5 = (_t6 * _t7);
            const auto _t24 = (gCc12 * _t25);
            const auto _t53 = (gRatt * _t54);
            const auto _t60 = (zRrelCenv * _t37);
            const auto _t61 = (_t62 - _t68);
            const auto _t70 = (_t71 - _t74);
            const auto _t78 = (_t4 * _t79);
            const auto _t94 = (gCc12 * _t95);
            const auto _t102 = (_t103 - _t108);
            const auto _t3 = (_t4 * _t5);
            const auto _t52 = (_t53 - _t60);
            const auto _t77 = (_t78 - _t82);
            const auto _t93 = (_t94 - _t98);
            const auto _t2 = (_t3 - _t24);
            const auto _t76 = (_t77 + _t84);
            const auto _t92 = (_t93 - _t100);
            const auto _t1 = (_t2 + _t26);
            const auto vo = (((_t0 * ((gRatt * _t1) - (_t34 * _t1))) + (gRfb * (gRfb * _t1))) * _t35);
            const auto vg1 = (-((_t48 - ((gCin * vi) - zCin)) * _t110));
            const auto tCin = (gCin * (vi - vg1));
            const auto vk1 = (((_t50 * _t36) + (gRfb * _t52)) * _t35);
            const auto vRk1Ck1 = (vk1 - 0);
            const auto vpl1 = (((_t0 * ((gRatt * _t61) - (_t34 * _t61))) + (gRfb * (gRfb * _t61))) * _t35);
            const auto vg2 = (((_t0 * ((gRatt * _t70) - (_t34 * _t70))) + (gRfb * (gRfb * _t70))) * _t35);
            const auto tCc12 = (gCc12 * (vpl1 - vg2));
            const auto venv = (-(((_t50 * _t47) - (_t0 * _t52)) * _t35));
            const auto vRrelCenv = (venv - 0);
            const auto vpl2 = (((_t0 * ((gRatt * _t76) - (_t34 * _t76))) + (gRfb * (gRfb * _t76))) * _t35);
            const auto tCout = (gCout * (vpl2 - vo));
            const auto vsc = (((_t0 * ((gRatt * _t92) - (_t34 * _t92))) + (gRfb * (gRfb * _t92))) * _t35);
            const auto tCsc = (gCsc * (vo - vsc));
            const auto vk2 = (((_t0 * ((gRatt * _t102) - (_t34 * _t102))) + (gRfb * (gRfb * _t102))) * _t35);
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

static void reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc = 0.0f)
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
    const auto _X1_D1_t3 = (1.0 / Rk1);
    const auto _X1_D1_t4 = (1.0 / 1000000000.0);
    const auto _X1_D1_t6 = (gRg2 + _X1_D1_t4);
    const auto _X1_D1_t9 = (gRp2 + _X1_D1_t4);
    const auto _X1_D1_t12 = (1.0 / Rk2);
    const auto _X1_D1_t13 = (gRL + _X1_D1_t4);
    const auto _X1_D1_t34 = (gRfb + gRatt);
    const auto _X1_D1_t35 = (1.0 / Rrel);
    const auto _X1_D1_t41 = (gRatt + _X1_D1_t4);
    const auto _X1_D1_t48 = (gRp1 + _X1_D1_t4);
    const auto _X1_D1_t75 = (gRfb * gRatt);
    const auto _X1_D1_t79 = (1.0 / (gRg1 + _X1_D1_t4));
    const auto _X1_D1_t96 = (_12AX7_DEMPWOLF_Gamma - 1.0);
    const auto _X1_D1_t102 = (_12AX7_DEMPWOLF_Xi - 1.0);
    const auto _X1_D1_t145 = (_12AX7_DEMPWOLF_Gk * _12AX7_DEMPWOLF_Gamma);
    const auto _X1_D1_t2 = (_X1_D1_t3 + gRfb);
    const auto _X1_D1_t11 = (_X1_D1_t12 + _X1_D1_t4);
    const auto _X1_D1_t33 = (_X1_D1_t34 + _X1_D1_t35);
    const auto _X1_D1_t74 = (_X1_D1_t75 * _X1_D1_t48);
    const auto _X1_D1_t1 = (_X1_D1_t2 + _X1_D1_t4);
    const auto _X1_D1_t10 = (_X1_D1_t11 * _X1_D1_t13);
    const auto _X1_D1_t32 = (_X1_D1_t33 + _X1_D1_t4);
    const auto _X1_D1_t73 = (_X1_D1_t74 * _X1_D1_t6);
    const auto _X1_D1_t8 = (_X1_D1_t9 * _X1_D1_t10);
    const auto _X1_D1_t40 = (_X1_D1_t41 * _X1_D1_t8);
    const auto _X1_D1_t72 = (_X1_D1_t73 * D1N914_Is);
    const auto _X1_D1_t7 = (gRatt * _X1_D1_t8);
    const auto _X1_D1_t39 = (_X1_D1_t6 * _X1_D1_t40);
    const auto _X1_D1_t50 = (_X1_D1_t48 * _X1_D1_t39);
    const auto _X1_D1_t52 = (gRfb * _X1_D1_t50);
    const auto _X1_D1_t5 = (_X1_D1_t6 * _X1_D1_t7);
    const auto _X1_D1_t38 = (gRp1 * _X1_D1_t39);
    const auto _X1_D1_t47 = (_X1_D1_t48 * _X1_D1_t5);
    const auto _X1_D1_t49 = (_X1_D1_t32 * _X1_D1_t50);
    const auto _X1_D1_t51 = (gRfb * _X1_D1_t52);
    const auto _X1_D1_t117 = (gRfb * _X1_D1_t47);
    const auto _X1_D1_t37 = (VCC * _X1_D1_t38);
    const auto _X1_D1_t46 = (gRatt * _X1_D1_t47);
    const auto _X1_D1_t45 = (_X1_D1_t46 - _X1_D1_t49);
    const auto _X1_D1_t44 = (_X1_D1_t1 * _X1_D1_t45);
    const auto _X1_D1_t43 = (1.0 / (_X1_D1_t44 + _X1_D1_t51));
    const auto _X1_D1_t163 = (VCC * (gRp1 * _X1_D1_t5));
    const auto _X1_D1_t164 = (_12AX7_DEMPWOLF_Gg * _12AX7_DEMPWOLF_Xi);
    for (int newton_iter = 0; newton_iter < 10000; ++newton_iter)
    {
        const auto _X1_D1_t23 = (vPKX1 / _12AX7_DEMPWOLF_Mu);
        const auto _X1_D1_t31 = (_12AX7_DEMPWOLF_Cg * vGKX1);
        const auto _X1_D1_t63 = (vD1 / D1N914_vt);
        const auto _X1_D1_t22 = (_X1_D1_t23 + vGKX1);
        const auto _X1_D1_t30 = exp(_X1_D1_t31);
        const auto _X1_D1_t62 = exp(_X1_D1_t63);
        const auto _X1_D1_t129 = (D1N914_Is * _X1_D1_t62);
        const auto _X1_D1_t21 = (_12AX7_DEMPWOLF_Ck * _X1_D1_t22);
        const auto _X1_D1_t29 = (1.0 + _X1_D1_t30);
        const auto _X1_D1_t61 = (_X1_D1_t62 - 1.0);
        const auto _X1_D1_t20 = exp(_X1_D1_t21);
        const auto _X1_D1_t28 = log(_X1_D1_t29);
        const auto _X1_D1_t60 = (D1N914_Is * _X1_D1_t61);
        const auto _X1_D1_t128 = (_X1_D1_t129 * _X1_D1_t8);
        const auto _X1_D1_t19 = (1.0 + _X1_D1_t20);
        const auto _X1_D1_t27 = (_X1_D1_t28 / _12AX7_DEMPWOLF_Cg);
        const auto _X1_D1_t59 = (_X1_D1_t60 * _X1_D1_t8);
        const auto _X1_D1_t71 = (_X1_D1_t72 * _X1_D1_t62);
        const auto _X1_D1_t101 = pow(_X1_D1_t27, _X1_D1_t102);
        const auto _X1_D1_t127 = (_X1_D1_t6 * _X1_D1_t128);
        const auto _X1_D1_t146 = (_12AX7_DEMPWOLF_Mu * _X1_D1_t19);
        const auto _X1_D1_t18 = log(_X1_D1_t19);
        const auto _X1_D1_t26 = pow(_X1_D1_t27, _12AX7_DEMPWOLF_Xi);
        const auto _X1_D1_t58 = (_X1_D1_t6 * _X1_D1_t59);
        const auto _X1_D1_t70 = (_X1_D1_t71 * _X1_D1_t9);
        const auto _X1_D1_t100 = (_X1_D1_t101 * _X1_D1_t30);
        const auto _X1_D1_t126 = (_X1_D1_t48 * _X1_D1_t127);
        const auto _X1_D1_t132 = (gRfb * _X1_D1_t126);
        const auto _X1_D1_t17 = (_X1_D1_t18 / _12AX7_DEMPWOLF_Ck);
        const auto _X1_D1_t25 = (_12AX7_DEMPWOLF_Gg * _X1_D1_t26);
        const auto _X1_D1_t57 = (_X1_D1_t48 * _X1_D1_t58);
        const auto _X1_D1_t69 = (_X1_D1_t70 * _X1_D1_t11);
        const auto _X1_D1_t95 = pow(_X1_D1_t17, _X1_D1_t96);
        const auto _X1_D1_t99 = (_12AX7_DEMPWOLF_Xi * _X1_D1_t100);
        const auto _X1_D1_t113 = (_X1_D1_t32 * _X1_D1_t57);
        const auto _X1_D1_t115 = (gRfb * _X1_D1_t57);
        const auto _X1_D1_t125 = (_X1_D1_t32 * _X1_D1_t126);
        const auto _X1_D1_t131 = (gRfb * _X1_D1_t132);
        const auto _X1_D1_t144 = (_X1_D1_t145 * _X1_D1_t95);
        const auto _X1_D1_t16 = pow(_X1_D1_t17, _12AX7_DEMPWOLF_Gamma);
        const auto _X1_D1_t24 = (_X1_D1_t25 + _12AX7_DEMPWOLF_Ig0);
        const auto _X1_D1_t56 = (gRatt * _X1_D1_t57);
        const auto _X1_D1_t68 = (_X1_D1_t69 * _X1_D1_t13);
        const auto _X1_D1_t94 = (_X1_D1_t95 * _X1_D1_t20);
        const auto _X1_D1_t98 = (_12AX7_DEMPWOLF_Gg * _X1_D1_t99);
        const auto _X1_D1_t112 = (_X1_D1_t1 * _X1_D1_t113);
        const auto _X1_D1_t114 = (gRfb * _X1_D1_t115);
        const auto _X1_D1_t124 = (_X1_D1_t1 * _X1_D1_t125);
        const auto _X1_D1_t130 = (_X1_D1_t131 / D1N914_vt);
        const auto _X1_D1_t143 = (_X1_D1_t144 * _X1_D1_t20);
        const auto _X1_D1_t156 = (_X1_D1_t143 * gRfb);
        const auto _X1_D1_t15 = (_12AX7_DEMPWOLF_Gk * _X1_D1_t16);
        const auto _X1_D1_t55 = (gRfb * _X1_D1_t56);
        const auto _X1_D1_t67 = (_X1_D1_t68 / D1N914_vt);
        const auto _X1_D1_t93 = (_12AX7_DEMPWOLF_Gamma * _X1_D1_t94);
        const auto _X1_D1_t97 = (_X1_D1_t98 / _X1_D1_t29);
        const auto _X1_D1_t111 = (_X1_D1_t112 - _X1_D1_t114);
        const auto _X1_D1_t123 = (_X1_D1_t124 / D1N914_vt);
        const auto _X1_D1_t142 = (_X1_D1_t143 * _X1_D1_t45);
        const auto _X1_D1_t155 = (_X1_D1_t156 * _X1_D1_t48);
        const auto _X1_D1_t14 = (_X1_D1_t15 - _X1_D1_t24);
        const auto _X1_D1_t42 = (_X1_D1_t14 * _X1_D1_t39);
        const auto _X1_D1_t65 = (_X1_D1_t14 + _X1_D1_t24);
        const auto _X1_D1_t66 = (_X1_D1_t67 * _X1_D1_t43);
        const auto _X1_D1_t92 = (_12AX7_DEMPWOLF_Gk * _X1_D1_t93);
        const auto _X1_D1_t116 = (_X1_D1_t65 * _X1_D1_t117);
        const auto _X1_D1_t122 = (_X1_D1_t123 - _X1_D1_t130);
        const auto _X1_D1_t141 = (_X1_D1_t142 / _X1_D1_t146);
        const auto _X1_D1_t154 = (_X1_D1_t155 * _X1_D1_t6);
        const auto _X1_D1_t158 = (_X1_D1_t92 * _X1_D1_t39);
        const auto _X1_D1_t36 = (_X1_D1_t37 - _X1_D1_t42);
        const auto _X1_D1_t64 = (_X1_D1_t65 * _X1_D1_t45);
        const auto _X1_D1_t91 = (_X1_D1_t92 / _X1_D1_t19);
        const auto _X1_D1_t110 = (_X1_D1_t111 + _X1_D1_t116);
        const auto _X1_D1_t121 = (_X1_D1_t122 * _X1_D1_t43);
        const auto _X1_D1_t140 = (_X1_D1_t141 * _X1_D1_t43);
        const auto _X1_D1_t153 = (_X1_D1_t154 * gRatt);
        const auto _X1_D1_t157 = (((((_X1_D1_t1 * (((_X1_D1_t32 * _X1_D1_t158) / _X1_D1_t146) - ((gRatt * (_X1_D1_t92 * _X1_D1_t5)) / _X1_D1_t146))) - ((gRfb * (gRfb * _X1_D1_t158)) / _X1_D1_t146)) * _X1_D1_t43) - _X1_D1_t140) - 1.0);
        const auto _X1_D1_t54 = (_X1_D1_t55 - _X1_D1_t64);
        const auto _X1_D1_t90 = (_X1_D1_t91 - _X1_D1_t97);
        const auto _X1_D1_t109 = (_X1_D1_t110 * _X1_D1_t43);
        const auto _X1_D1_t120 = ((_X1_D1_t121 - _X1_D1_t121) - 1.0);
        const auto _X1_D1_t134 = (_X1_D1_t90 * _X1_D1_t39);
        const auto _X1_D1_t152 = (_X1_D1_t153 * _X1_D1_t9);
        const auto _X1_D1_t53 = (_X1_D1_t54 * _X1_D1_t43);
        const auto _X1_D1_t78 = (_X1_D1_t53 - (_X1_D1_t24 * _X1_D1_t79));
        const auto _X1_D1_t89 = (_X1_D1_t90 + _X1_D1_t97);
        const auto _X1_D1_t106 = (_X1_D1_t89 * _X1_D1_t45);
        const auto _X1_D1_t108 = (_X1_D1_t109 - _X1_D1_t109);
        const auto _X1_D1_t151 = (_X1_D1_t152 * _X1_D1_t11);
        const auto _X1_D1_t0 = ((((_X1_D1_t1 * ((gRatt * (_X1_D1_t163 - (_X1_D1_t14 * _X1_D1_t5))) - (_X1_D1_t32 * _X1_D1_t36))) + (gRfb * (gRfb * _X1_D1_t36))) * _X1_D1_t43) + _X1_D1_t53);
        const auto _X1_D1_t77 = (vGKX1 - _X1_D1_t78);
        const auto _X1_D1_t88 = (_X1_D1_t89 * gRfb);
        const auto _X1_D1_t105 = (_X1_D1_t106 * _X1_D1_t43);
        const auto _X1_D1_t107 = (vD1 - _X1_D1_t108);
        const auto _X1_D1_t118 = (vPKX1 - _X1_D1_t0);
        const auto _X1_D1_t133 = ((((_X1_D1_t1 * ((_X1_D1_t32 * _X1_D1_t134) - (gRatt * (_X1_D1_t90 * _X1_D1_t5)))) - (gRfb * (gRfb * _X1_D1_t134))) * _X1_D1_t43) - _X1_D1_t105);
        const auto _X1_D1_t136 = (_X1_D1_t77 * _X1_D1_t120);
        const auto _X1_D1_t137 = (_X1_D1_t66 * _X1_D1_t107);
        const auto _X1_D1_t150 = (_X1_D1_t151 * _X1_D1_t13);
        const auto _X1_D1_t161 = (_X1_D1_t140 * _X1_D1_t107);
        const auto _X1_D1_t87 = (_X1_D1_t88 * _X1_D1_t48);
        const auto _X1_D1_t104 = ((_X1_D1_t105 + ((((_X1_D1_t164 * _X1_D1_t101) * _X1_D1_t30) / _X1_D1_t29) * _X1_D1_t79)) + 1.0);
        const auto _X1_D1_t135 = (_X1_D1_t136 - _X1_D1_t137);
        const auto _X1_D1_t149 = (_X1_D1_t150 / _X1_D1_t146);
        const auto _X1_D1_t86 = (_X1_D1_t87 * _X1_D1_t6);
        const auto _X1_D1_t103 = (_X1_D1_t104 * _X1_D1_t107);
        const auto _X1_D1_t148 = (_X1_D1_t149 * _X1_D1_t43);
        const auto _X1_D1_t85 = (_X1_D1_t86 * gRatt);
        const auto _X1_D1_t147 = (_X1_D1_t148 - _X1_D1_t148);
        const auto _X1_D1_t162 = (_X1_D1_t77 * _X1_D1_t147);
        const auto _X1_D1_t84 = (_X1_D1_t85 * _X1_D1_t9);
        const auto _X1_D1_t139 = ((_X1_D1_t140 * _X1_D1_t120) + (_X1_D1_t66 * _X1_D1_t147));
        const auto _X1_D1_t160 = (_X1_D1_t161 + _X1_D1_t162);
        const auto _X1_D1_t83 = (_X1_D1_t84 * _X1_D1_t11);
        const auto _X1_D1_t82 = (_X1_D1_t83 * _X1_D1_t13);
        const auto _X1_D1_t81 = (_X1_D1_t82 * _X1_D1_t43);
        const auto _X1_D1_t80 = (_X1_D1_t81 - _X1_D1_t81);
        const auto _X1_D1_t119 = ((_X1_D1_t104 * _X1_D1_t120) + (_X1_D1_t66 * _X1_D1_t80));
        const auto _X1_D1_t159 = ((_X1_D1_t104 * _X1_D1_t147) - (_X1_D1_t140 * _X1_D1_t80));
        const auto _X1_D1_t76 = (_X1_D1_t77 * _X1_D1_t80);
        const auto _X1_D1_t138 = (((_X1_D1_t133 * _X1_D1_t139) - (_X1_D1_t157 * _X1_D1_t119)) + (_X1_D1_t66 * _X1_D1_t159));
        const auto res_vPKX1 = (_X1_D1_t0 - vPKX1);
        const auto delta_vPKX1 = (((_X1_D1_t66 * (_X1_D1_t76 + _X1_D1_t103)) - ((_X1_D1_t118 * _X1_D1_t119) + (_X1_D1_t133 * _X1_D1_t135))) / _X1_D1_t138);
        const auto res_vGKX1 = (_X1_D1_t78 - vGKX1);
        const auto delta_vGKX1 = ((((_X1_D1_t157 * _X1_D1_t135) + (_X1_D1_t118 * _X1_D1_t139)) - (_X1_D1_t66 * _X1_D1_t160)) / _X1_D1_t138);
        const auto res_vD1 = (_X1_D1_t108 - vD1);
        const auto delta_vD1 = ((((_X1_D1_t133 * _X1_D1_t160) - (_X1_D1_t157 * (_X1_D1_t103 + _X1_D1_t76))) + (_X1_D1_t118 * _X1_D1_t159)) / _X1_D1_t138);
    
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
    const auto _X2_t3 = (1.0 / Rk1);
    const auto _X2_t4 = (1.0 / 1000000000.0);
    const auto _X2_t5 = (gRp1 + _X2_t4);
    const auto _X2_t6 = (gRg2 + _X2_t4);
    const auto _X2_t10 = (1.0 / Rk2);
    const auto _X2_t11 = (gRL + _X2_t4);
    const auto _X2_t33 = (gRfb + gRatt);
    const auto _X2_t34 = (1.0 / Rrel);
    const auto _X2_t40 = (gRatt + _X2_t4);
    const auto _X2_t52 = (gRp2 + _X2_t4);
    const auto _X2_t84 = (_12AX7_DEMPWOLF_Xi - 1.0);
    const auto _X2_t102 = (_12AX7_DEMPWOLF_Gamma - 1.0);
    const auto _X2_t2 = (_X2_t3 + gRfb);
    const auto _X2_t9 = (_X2_t10 + _X2_t4);
    const auto _X2_t32 = (_X2_t33 + _X2_t34);
    const auto _X2_t1 = (_X2_t2 + _X2_t4);
    const auto _X2_t8 = (_X2_t9 * _X2_t11);
    const auto _X2_t31 = (_X2_t32 + _X2_t4);
    const auto _X2_t51 = (_X2_t52 * _X2_t8);
    const auto _X2_t56 = (_X2_t40 * _X2_t51);
    const auto _X2_t7 = (gRp2 * _X2_t8);
    const auto _X2_t39 = (_X2_t40 * _X2_t7);
    const auto _X2_t50 = (gRatt * _X2_t51);
    const auto _X2_t55 = (_X2_t6 * _X2_t56);
    const auto _X2_t38 = (_X2_t6 * _X2_t39);
    const auto _X2_t49 = (_X2_t6 * _X2_t50);
    const auto _X2_t54 = (_X2_t5 * _X2_t55);
    const auto _X2_t58 = (gRfb * _X2_t54);
    const auto _X2_t37 = (_X2_t5 * _X2_t38);
    const auto _X2_t48 = (_X2_t5 * _X2_t49);
    const auto _X2_t53 = (_X2_t31 * _X2_t54);
    const auto _X2_t57 = (gRfb * _X2_t58);
    const auto _X2_t36 = (VCC * _X2_t37);
    const auto _X2_t47 = (gRatt * _X2_t48);
    const auto _X2_t46 = (_X2_t47 - _X2_t53);
    const auto _X2_t45 = (_X2_t1 * _X2_t46);
    const auto _X2_t44 = (1.0 / (_X2_t45 + _X2_t57));
    const auto _X2_t145 = (VCC * (_X2_t5 * (_X2_t6 * (gRatt * _X2_t7))));
    for (int newton_iter = 0; newton_iter < 10000; ++newton_iter)
    {
        const auto _X2_t22 = (vPKX2 / _12AX7_DEMPWOLF_Mu);
        const auto _X2_t30 = (_12AX7_DEMPWOLF_Cg * vGKX2);
        const auto _X2_t21 = (_X2_t22 + vGKX2);
        const auto _X2_t29 = exp(_X2_t30);
        const auto _X2_t20 = (_12AX7_DEMPWOLF_Ck * _X2_t21);
        const auto _X2_t28 = (1.0 + _X2_t29);
        const auto _X2_t19 = exp(_X2_t20);
        const auto _X2_t27 = log(_X2_t28);
        const auto _X2_t18 = (1.0 + _X2_t19);
        const auto _X2_t26 = (_X2_t27 / _12AX7_DEMPWOLF_Cg);
        const auto _X2_t83 = pow(_X2_t26, _X2_t84);
        const auto _X2_t131 = (_12AX7_DEMPWOLF_Mu * _X2_t18);
        const auto _X2_t17 = log(_X2_t18);
        const auto _X2_t25 = pow(_X2_t26, _12AX7_DEMPWOLF_Xi);
        const auto _X2_t82 = (_X2_t83 * _X2_t29);
        const auto _X2_t16 = (_X2_t17 / _12AX7_DEMPWOLF_Ck);
        const auto _X2_t24 = (_12AX7_DEMPWOLF_Gg * _X2_t25);
        const auto _X2_t81 = (_12AX7_DEMPWOLF_Xi * _X2_t82);
        const auto _X2_t101 = pow(_X2_t16, _X2_t102);
        const auto _X2_t15 = pow(_X2_t16, _12AX7_DEMPWOLF_Gamma);
        const auto _X2_t23 = (_X2_t24 + _12AX7_DEMPWOLF_Ig0);
        const auto _X2_t80 = (_12AX7_DEMPWOLF_Gg * _X2_t81);
        const auto _X2_t100 = (_X2_t101 * _X2_t19);
        const auto _X2_t103 = (_X2_t80 / _X2_t28);
        const auto _X2_t118 = (_X2_t23 * _X2_t56);
        const auto _X2_t14 = (_12AX7_DEMPWOLF_Gk * _X2_t15);
        const auto _X2_t79 = (_X2_t80 * _X2_t56);
        const auto _X2_t99 = (_12AX7_DEMPWOLF_Gamma * _X2_t100);
        const auto _X2_t117 = (_X2_t5 * _X2_t118);
        const auto _X2_t13 = (_X2_t14 - _X2_t23);
        const auto _X2_t69 = (_X2_t13 + _X2_t23);
        const auto _X2_t78 = (_X2_t5 * _X2_t79);
        const auto _X2_t98 = (_12AX7_DEMPWOLF_Gk * _X2_t99);
        const auto _X2_t130 = (_X2_t98 * _X2_t11);
        const auto _X2_t144 = (_X2_t98 * _X2_t8);
        const auto _X2_t12 = (_X2_t13 * _X2_t8);
        const auto _X2_t43 = (_X2_t40 * _X2_t12);
        const auto _X2_t68 = (_X2_t69 * _X2_t11);
        const auto _X2_t97 = (_X2_t98 / _X2_t18);
        const auto _X2_t129 = (_X2_t52 * _X2_t130);
        const auto _X2_t136 = (_X2_t40 * _X2_t129);
        const auto _X2_t143 = (_X2_t40 * _X2_t144);
        const auto _X2_t42 = (_X2_t6 * _X2_t43);
        const auto _X2_t67 = (_X2_t52 * _X2_t68);
        const auto _X2_t73 = (_X2_t40 * _X2_t67);
        const auto _X2_t96 = (_X2_t97 - _X2_t103);
        const auto _X2_t114 = (_X2_t96 * _X2_t8);
        const auto _X2_t128 = (gRatt * _X2_t129);
        const auto _X2_t135 = (_X2_t6 * _X2_t136);
        const auto _X2_t142 = (_X2_t6 * _X2_t143);
        const auto _X2_t41 = (_X2_t5 * _X2_t42);
        const auto _X2_t66 = (gRatt * _X2_t67);
        const auto _X2_t72 = (_X2_t6 * _X2_t73);
        const auto _X2_t95 = (_X2_t96 + _X2_t103);
        const auto _X2_t113 = (_X2_t40 * _X2_t114);
        const auto _X2_t127 = (_X2_t6 * _X2_t128);
        const auto _X2_t134 = (_X2_t5 * _X2_t135);
        const auto _X2_t139 = (gRfb * _X2_t134);
        const auto _X2_t141 = (_X2_t5 * _X2_t142);
        const auto _X2_t35 = (_X2_t36 - _X2_t41);
        const auto _X2_t65 = (_X2_t6 * _X2_t66);
        const auto _X2_t71 = (_X2_t5 * _X2_t72);
        const auto _X2_t75 = (gRfb * _X2_t71);
        const auto _X2_t94 = (_X2_t95 * _X2_t11);
        const auto _X2_t112 = (_X2_t6 * _X2_t113);
        const auto _X2_t126 = (_X2_t5 * _X2_t127);
        const auto _X2_t133 = (_X2_t31 * _X2_t134);
        const auto _X2_t138 = (gRfb * _X2_t139);
        const auto _X2_t64 = (_X2_t5 * _X2_t65);
        const auto _X2_t70 = (_X2_t31 * _X2_t71);
        const auto _X2_t74 = (gRfb * _X2_t75);
        const auto _X2_t93 = (_X2_t52 * _X2_t94);
        const auto _X2_t107 = (_X2_t40 * _X2_t93);
        const auto _X2_t111 = (_X2_t5 * _X2_t112);
        const auto _X2_t125 = (gRatt * _X2_t126);
        const auto _X2_t132 = (_X2_t133 / _X2_t131);
        const auto _X2_t137 = (_X2_t138 / _X2_t131);
        const auto _X2_t63 = (gRatt * _X2_t64);
        const auto _X2_t92 = (gRatt * _X2_t93);
        const auto _X2_t106 = (_X2_t6 * _X2_t107);
        const auto _X2_t124 = (_X2_t125 / _X2_t131);
        const auto _X2_t62 = (_X2_t63 - _X2_t70);
        const auto _X2_t91 = (_X2_t6 * _X2_t92);
        const auto _X2_t105 = (_X2_t5 * _X2_t106);
        const auto _X2_t109 = (gRfb * _X2_t105);
        const auto _X2_t123 = (_X2_t124 - _X2_t132);
        const auto _X2_t61 = (_X2_t1 * _X2_t62);
        const auto _X2_t90 = (_X2_t5 * _X2_t91);
        const auto _X2_t104 = (_X2_t31 * _X2_t105);
        const auto _X2_t108 = (gRfb * _X2_t109);
        const auto _X2_t122 = (_X2_t1 * _X2_t123);
        const auto _X2_t60 = (_X2_t61 + _X2_t74);
        const auto _X2_t89 = (gRatt * _X2_t90);
        const auto _X2_t121 = (_X2_t122 + _X2_t137);
        const auto _X2_t59 = (_X2_t60 * _X2_t44);
        const auto _X2_t88 = (_X2_t89 - _X2_t104);
        const auto _X2_t116 = ((((gRfb * (gRfb * _X2_t117)) - (_X2_t1 * ((_X2_t31 * _X2_t117) - (gRatt * (_X2_t5 * (_X2_t23 * _X2_t50)))))) * _X2_t44) + _X2_t59);
        const auto _X2_t120 = (_X2_t121 * _X2_t44);
        const auto _X2_t140 = (((((_X2_t1 * (((_X2_t31 * _X2_t141) / _X2_t131) - ((gRatt * (_X2_t5 * (_X2_t6 * (gRatt * _X2_t144)))) / _X2_t131))) - ((gRfb * (gRfb * _X2_t141)) / _X2_t131)) * _X2_t44) - _X2_t120) - 1.0);
        const auto _X2_t0 = ((((_X2_t1 * ((gRatt * (_X2_t145 - (_X2_t5 * (_X2_t6 * (gRatt * _X2_t12))))) - (_X2_t31 * _X2_t35))) + (gRfb * (gRfb * _X2_t35))) * _X2_t44) - _X2_t59);
        const auto _X2_t76 = (vPKX2 - _X2_t0);
        const auto _X2_t87 = (_X2_t1 * _X2_t88);
        const auto _X2_t115 = (_X2_t116 + vGKX2);
        const auto _X2_t86 = (_X2_t87 + _X2_t108);
        const auto _X2_t85 = (_X2_t86 * _X2_t44);
        const auto _X2_t110 = ((((_X2_t1 * ((_X2_t31 * _X2_t111) - (gRatt * (_X2_t5 * (_X2_t6 * (gRatt * _X2_t114)))))) - (gRfb * (gRfb * _X2_t111))) * _X2_t44) - _X2_t85);
        const auto _X2_t77 = ((((((gRfb * (gRfb * _X2_t78)) / _X2_t28) - (_X2_t1 * (((_X2_t31 * _X2_t78) / _X2_t28) - ((gRatt * (_X2_t5 * (_X2_t80 * _X2_t50))) / _X2_t28)))) * _X2_t44) + _X2_t85) + 1.0);
        const auto _X2_t119 = ((_X2_t110 * _X2_t120) - (_X2_t140 * _X2_t77));
        const auto res_vPKX2 = (_X2_t0 - vPKX2);
        const auto delta_vPKX2 = (-(((_X2_t76 * _X2_t77) + (_X2_t110 * _X2_t115)) / _X2_t119));
        const auto res_vGKX2 = (-_X2_t115);
        const auto delta_vGKX2 = (((_X2_t140 * _X2_t115) + (_X2_t76 * _X2_t120)) / _X2_t119);
    
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
    const auto zCin = (gCin * (vi + (((_12AX7_DEMPWOLF_Gg * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0) / (gRg1 + (1.0 / 1000000000.0)))));
    const auto zRk1Ck1 = (-(((gzRk1Ck1 * ((gRfb * (gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((D1N914_Is * (exp((vD1 / D1N914_vt)) - 1.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))) - ((((_12AX7_DEMPWOLF_Gk * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) + ((_12AX7_DEMPWOLF_Gg * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))))) / (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))) + (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) / 2.0));
    const auto zCc12 = (gCc12 * (((((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((VCC * (gRp1 * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - (((_12AX7_DEMPWOLF_Gk * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((VCC * (gRp1 * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - (((_12AX7_DEMPWOLF_Gk * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) + (gRfb * (gRfb * ((VCC * (gRp1 * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - (((_12AX7_DEMPWOLF_Gk * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) / (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))) + (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) + (((gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * (((_12AX7_DEMPWOLF_Gg * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))) - ((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * (((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * (((_12AX7_DEMPWOLF_Gg * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - (gRatt * ((gRp1 + (1.0 / 1000000000.0)) * (((_12AX7_DEMPWOLF_Gg * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) / (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))) + (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))))));
    const auto zRrelCenv = (-(((gzRrelCenv * (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * (gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((D1N914_Is * (exp((vD1 / D1N914_vt)) - 1.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))) + ((((_12AX7_DEMPWOLF_Gk * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) + ((_12AX7_DEMPWOLF_Gg * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) / (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))) + (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) / 2.0));
    const auto zCout = ((gCout * (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((VCC * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * (gRp2 * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * (((_12AX7_DEMPWOLF_Gk * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Ck * ((vPKX2 / _12AX7_DEMPWOLF_Mu) + vGKX2))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((VCC * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * (gRp2 * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * (((_12AX7_DEMPWOLF_Gk * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Ck * ((vPKX2 / _12AX7_DEMPWOLF_Mu) + vGKX2))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) + (gRfb * (gRfb * ((VCC * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * (gRp2 * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * (((_12AX7_DEMPWOLF_Gk * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Ck * ((vPKX2 / _12AX7_DEMPWOLF_Mu) + vGKX2))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))))) / (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))) + (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))));
    const auto zCsc = ((gCsc * ((((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((D1N914_Is * (exp((vD1 / D1N914_vt)) - 1.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))) - (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((D1N914_Is * (exp((vD1 / D1N914_vt)) - 1.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))) + ((((_12AX7_DEMPWOLF_Gk * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) + ((_12AX7_DEMPWOLF_Gg * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) / (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))) + (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))));
    const auto zRk2Ck2 = (((gzRk2Ck2 * (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * ((((_12AX7_DEMPWOLF_Gk * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Ck * ((vPKX2 / _12AX7_DEMPWOLF_Mu) + vGKX2))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) + ((_12AX7_DEMPWOLF_Gg * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * ((((_12AX7_DEMPWOLF_Gk * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Ck * ((vPKX2 / _12AX7_DEMPWOLF_Mu) + vGKX2))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) + ((_12AX7_DEMPWOLF_Gg * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * (gRL + (1.0 / 1000000000.0)))))))))) + (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * ((((_12AX7_DEMPWOLF_Gk * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Ck * ((vPKX2 / _12AX7_DEMPWOLF_Mu) + vGKX2))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) + ((_12AX7_DEMPWOLF_Gg * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Cg * vGKX2)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) * (gRL + (1.0 / 1000000000.0))))))))))) / (((((1.0 / Rk1) + gRfb) + (1.0 / 1000000000.0)) * ((gRatt * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * (gRatt * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))) - ((((gRfb + gRatt) + (1.0 / Rrel)) + (1.0 / 1000000000.0)) * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0)))))))))) + (gRfb * (gRfb * ((gRp1 + (1.0 / 1000000000.0)) * ((gRg2 + (1.0 / 1000000000.0)) * ((gRatt + (1.0 / 1000000000.0)) * ((gRp2 + (1.0 / 1000000000.0)) * (((1.0 / Rk2) + (1.0 / 1000000000.0)) * (gRL + (1.0 / 1000000000.0))))))))))) / 2.0);

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
}
