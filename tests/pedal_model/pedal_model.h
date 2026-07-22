// Auto-generated with netlist_codegen version 25c9320.
// Command: netlist_codegen pedal_model.net pedal_model.h -type_name double

#pragma once

#include <cmath>

struct Params {
    double Vpp = 9.0e+00;
    double Vp = 4.5e+00;
    double R5 = 1.0e+06;
    double RL = 1.0e+05;
    double R4 = 1.0e+06;
    double _2N5485_Beta = 2.0e-03;
    double _2N5485_vp = -1.0e+00;
    double C4 = 1.0e-09;
    double Eamp_Aol = 3.0e+05;
    double Eamp_Vos = 2.0e-03;
    double Eamp_Ios = 1.0e-09;
    double Eamp_Ibias = 7.0e-09;
    double Eamp_Rin = 4.0e+07;
    double Eamp_Rcomp = 1.0e+00;
    double Eamp_Ccomp = 3.0e-11;
    double Eamp_Rout = 2.0e+02;
    double R11 = 1.0e+06;
    double C12 = 2.2e-08;
    double D1N914_Is = 2.52e-09;
    double D1N914_vt = 4.52892e-02;
    double R7 = 5.6e+02;
    double C6 = 4.7e-06;
    double C13 = 1.0e-05;
    double R6 = 1.0e+03;
    double C3 = 2.2e-08;
    double C11 = 3.3e-09;
    double R8 = 4.7e+01;
    double C7 = 2.2e-06;
    double R13 = 1.0e+04;
    double Rd = 1.0e+05;
    double C5 = 1.0e-10;
    double R9 = 1.0e+03;
    double C9 = 4.7e-06;
    double Rf = 1.0e+05;
    double R10 = 1.6e+03;
};

struct State {
    double zC4 {};
    double zEamp_Ccomp {};
    double zC12 {};
    double zR7C6 {};
    double zC13 {};
    double zR6C3 {};
    double zC11 {};
    double zR8C7 {};
    double zRdC5 {};
    double zR9C9 {};
    double vGSJ1 {};
    double vD1D2 {};
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

[[maybe_unused]] static auto limit_jfet_vgs = [](auto v_new, auto vp)
{
    if (v_new < vp) return vp;
    return v_new;
};

static constexpr auto newton_tol_sq = 1.0e-06;
static constexpr int newton_max_iter = 20;

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto _2N5485_Beta = params._2N5485_Beta;
    const auto _2N5485_vp = params._2N5485_vp;
    const auto Eamp_Aol = params.Eamp_Aol;
    const auto Eamp_Vos = params.Eamp_Vos;
    const auto Eamp_Ios = params.Eamp_Ios;
    const auto Eamp_Ibias = params.Eamp_Ibias;
    const auto D1N914_Is = params.D1N914_Is;
    const auto D1N914_vt = params.D1N914_vt;
    
    const auto Vpp = params.Vpp;
    
    const auto vpp = Vpp;
    
    const auto Vp = params.Vp;
    
    const auto vp = Vp;
    
    const auto gR5 = 1.0 / params.R5;
    
    const auto gRL = 1.0 / params.RL;
    
    const auto gR4 = 1.0 / params.R4;
    
    const auto gC4 = 2.0 * sample_rate * params.C4;
    
    const auto gEamp_Rin = 1.0 / params.Eamp_Rin;
    
    const auto gEamp_Rcomp = 1.0 / params.Eamp_Rcomp;
    
    const auto gEamp_Ccomp = 2.0 * sample_rate * params.Eamp_Ccomp;
    
    const auto gEamp_Rout = 1.0 / params.Eamp_Rout;
    
    const auto gR11 = 1.0 / params.R11;
    
    const auto gC12 = 2.0 * sample_rate * params.C12;
    
    const auto R7 = params.R7;
    const auto C6 = params.C6;
    const auto gR7C6 = (2.0 * sample_rate * C6) / (1.0 + 2.0 * sample_rate * R7 * C6);
    const auto gnR7C6 = gR7C6 * (2.0 * sample_rate * R7 * C6 - 1.0) / (2.0 * sample_rate * C6);
    
    const auto gC13 = 2.0 * sample_rate * params.C13;
    
    const auto R6 = params.R6;
    const auto C3 = params.C3;
    const auto gR6C3 = (2.0 * sample_rate * C3) / (1.0 + 2.0 * sample_rate * R6 * C3);
    const auto gnR6C3 = gR6C3 * (2.0 * sample_rate * R6 * C3 - 1.0) / (2.0 * sample_rate * C3);
    
    const auto gC11 = 2.0 * sample_rate * params.C11;
    
    const auto R8 = params.R8;
    const auto C7 = params.C7;
    const auto gR8C7 = (2.0 * sample_rate * C7) / (1.0 + 2.0 * sample_rate * R8 * C7);
    const auto gnR8C7 = gR8C7 * (2.0 * sample_rate * R8 * C7 - 1.0) / (2.0 * sample_rate * C7);
    
    const auto gR13 = 1.0 / params.R13;
    
    const auto Rd = params.Rd;
    const auto C5 = params.C5;
    const auto gRdC5 = 2.0 * sample_rate * C5 + (1.0 / Rd);
    const auto gzRdC5 = 4.0 * sample_rate * C5;
    
    const auto R9 = params.R9;
    const auto C9 = params.C9;
    const auto gR9C9 = (2.0 * sample_rate * C9) / (1.0 + 2.0 * sample_rate * R9 * C9);
    const auto gnR9C9 = gR9C9 * (2.0 * sample_rate * R9 * C9 - 1.0) / (2.0 * sample_rate * C9);
    
    const auto RfR10 = sum(params.Rf, params.R10);
    const auto gRfR10 = 1.0 / RfR10;
    
    const auto vcrit_D1N914_vt = D1N914_vt * std::log(D1N914_vt / (std::sqrt(2.0) * D1N914_Is));
    
    const auto _t1 = (gR11 + gC12);
    const auto _t3 = (gC13 + gR13);
    const auto _t8 = ((gEamp_Rcomp + gEamp_Ccomp) + gEamp_Rout);
    const auto _t10 = ((gC12 + gC11) + gRfR10);
    const auto _t11 = ((gEamp_Rout + gRdC5) + gR9C9);
    const auto _t15 = (gR9C9 + gRfR10);
    const auto _t21 = ((gC4 + gEamp_Rin) + gR6C3);
    const auto _t25 = (((gEamp_Rin + gR7C6) + gR8C7) + gRdC5);
    const auto _t33 = (gC12 * _t15);
    const auto _t38 = (gEamp_Rcomp * _t33);
    const auto _t39 = (gEamp_Rout * _t33);
    const auto _t42 = (gC12 * gR9C9);
    const auto _t57 = (gRL + gC13);
    const auto _t60 = (Eamp_Ios / 2.0);
    const auto _t66 = (Eamp_Aol * Eamp_Vos);
    const auto _t85 = (gC12 * gRfR10);
    const auto _t93 = (_t66 * _t38);
    const auto _t120 = (gEamp_Rout * _t85);
    const auto _t123 = (gEamp_Rcomp * _t85);
    const auto _t125 = (_t25 * _t120);
    const auto _t137 = (gEamp_Rcomp * _t42);
    const auto _t142 = (Eamp_Aol * _t137);
    const auto _t150 = (_t25 * _t142);
    const auto _t151 = (gEamp_Rin * _t142);
    const auto _t156 = (gRfR10 * gR9C9);
    const auto _t168 = (gEamp_Rcomp * _t156);
    const auto _t173 = (Eamp_Aol * _t168);
    const auto _t191 = (_t25 * _t173);
    const auto _t192 = (gEamp_Rin * _t173);
    const auto _t9 = (_t10 * gR9C9);
    const auto _t14 = ((_t10 * _t15) - (gRfR10 * gRfR10));
    const auto _t19 = (gEamp_Rcomp * _t14);
    const auto _t20 = (gEamp_Rout * _t14);
    const auto _t32 = (_t8 * _t33);
    const auto _t34 = (gRdC5 * _t32);
    const auto _t37 = (Eamp_Aol * _t38);
    const auto _t41 = (_t8 * _t42);
    const auto _t44 = (gEamp_Rin * _t32);
    const auto _t46 = (gEamp_Rin * _t41);
    const auto _t65 = (_t66 * _t19);
    const auto _t84 = (_t8 * _t85);
    const auto _t106 = (gEamp_Rcomp * _t9);
    const auto _t115 = (_t66 * _t106);
    const auto _t119 = (gEamp_Rin * _t120);
    const auto _t122 = (Eamp_Aol * _t123);
    const auto _t136 = (_t66 * _t137);
    const auto _t147 = (gEamp_Rin * _t37);
    const auto _t148 = (_t25 * _t37);
    const auto _t149 = (_t150 - _t151);
    const auto _t155 = (_t8 * _t156);
    const auto _t167 = (_t66 * _t168);
    const auto _t183 = (_t25 * _t155);
    const auto _t190 = (_t191 - _t192);
    const auto _t202 = (_t25 * _t84);
    const auto _t203 = (gEamp_Rin * _t84);
    const auto _t7 = (_t8 * _t9);
    const auto _t13 = (_t8 * _t14);
    const auto _t18 = (Eamp_Aol * _t19);
    const auto _t23 = (gRdC5 * _t13);
    const auto _t24 = (_t25 * _t13);
    const auto _t27 = (_t25 * _t7);
    const auto _t31 = (_t25 * _t32);
    const auto _t36 = (gRdC5 * _t37);
    const auto _t40 = (_t25 * _t41);
    const auto _t45 = (_t36 - (gEamp_Rin * _t39));
    const auto _t83 = (gRdC5 * _t84);
    const auto _t101 = (gEamp_Rin * _t18);
    const auto _t102 = (_t25 * _t18);
    const auto _t105 = (Eamp_Aol * _t106);
    const auto _t107 = (_t25 * _t105);
    const auto _t121 = (gRdC5 * _t122);
    const auto _t124 = (_t125 - _t121);
    const auto _t146 = (_t147 - _t148);
    const auto _t154 = (gEamp_Rin * _t155);
    const auto _t188 = (gRdC5 * _t154);
    const auto _t189 = (gEamp_Rout * _t190);
    const auto _t201 = ((_t21 * (gR9C9 * _t202)) - (gEamp_Rin * (gR9C9 * _t203)));
    const auto _t6 = (gEamp_Rin * _t7);
    const auto _t12 = (gEamp_Rin * _t13);
    const auto _t17 = (gRdC5 * _t18);
    const auto _t26 = (_t17 - (_t25 * _t20));
    const auto _t35 = (_t36 - (_t25 * _t39));
    const auto _t43 = (((_t11 * _t44) + (gEamp_Rout * _t45)) - (gR9C9 * _t46));
    const auto _t82 = (gR9C9 * _t83);
    const auto _t100 = (_t101 - _t102);
    const auto _t104 = (gEamp_Rin * _t105);
    const auto _t118 = (_t119 - _t121);
    const auto _t187 = (_t188 + _t189);
    const auto _t196 = (_t21 * _t82);
    const auto _t215 = ((_t21 * (((gRdC5 * _t83) - (_t11 * _t202)) + (gEamp_Rout * _t124))) - (gEamp_Rin * ((gEamp_Rout * _t118) - (_t11 * _t203))));
    const auto _t16 = (_t17 - (gEamp_Rin * _t20));
    const auto _t22 = ((((gRdC5 * _t23) - (_t11 * _t24)) - (gEamp_Rout * _t26)) + (gR9C9 * _t27));
    const auto _t30 = ((((_t11 * _t31) - (gRdC5 * _t34)) + (gEamp_Rout * _t35)) - (gR9C9 * _t40));
    const auto _t81 = (gEamp_Rin * _t82);
    const auto _t103 = (_t104 - _t107);
    const auto _t117 = ((gEamp_Rin * (gR9C9 * _t118)) - (_t21 * (gR9C9 * _t124)));
    const auto _t5 = ((gR9C9 * _t6) - ((_t11 * _t12) + (gEamp_Rout * _t16)));
    const auto _t29 = ((_t21 * _t30) - (gEamp_Rin * _t43));
    const auto _t56 = (gC13 * _t29);
    const auto _t4 = ((gEamp_Rin * _t5) - (_t21 * _t22));
    const auto _t28 = (_t3 * _t29);
    const auto _t55 = (gC13 * _t4);
    const auto _t2 = (_t3 * _t4);
    const auto _t54 = ((_t1 * _t55) - (gC12 * _t56));
    const auto _t0 = ((_t1 * _t2) - (gC12 * _t28));
    const auto _t53 = (1.0 / ((gC13 * _t54) - (_t57 * _t0)));
    const auto _t216 = (gC13 * _t81);
    const auto _t217 = (_t3 * _t81);
    const auto _t218 = (gC13 * _t117);
    const auto _t219 = (_t3 * _t117);
    const auto _t220 = (gC13 * _t196);
    const auto _t221 = (_t3 * _t196);
    const auto _t222 = (gC13 * _t201);
    const auto _t223 = (_t3 * _t201);
    const auto _t224 = (gC13 * _t215);
    const auto _t225 = (_t3 * _t215);
    const auto _t226 = (1.0 / 1000.0);
    const auto _t227 = (Eamp_Ibias - _t60);
    const auto _t228 = (Eamp_Ibias + _t60);
    const auto _t229 = ((gEamp_Rout * _t146) - (gRdC5 * _t44));
    const auto _t230 = ((gRdC5 * _t12) - (gEamp_Rout * _t100));
    const auto _t231 = ((gEamp_Rout * _t103) - (gRdC5 * _t6));
    const auto _t232 = (((_t11 * _t100) + (gRdC5 * _t16)) - (gR9C9 * _t103));
    const auto _t233 = ((gRdC5 * _t46) + (gEamp_Rout * _t149));
    const auto _t234 = (((_t11 * _t146) + (gRdC5 * _t45)) + (gR9C9 * _t149));
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zC4 = state[ch].zC4;
        auto zEamp_Ccomp = state[ch].zEamp_Ccomp;
        auto zC12 = state[ch].zC12;
        auto zR7C6 = state[ch].zR7C6;
        auto zC13 = state[ch].zC13;
        auto zR6C3 = state[ch].zR6C3;
        auto zC11 = state[ch].zC11;
        auto zR8C7 = state[ch].zR8C7;
        auto zRdC5 = state[ch].zRdC5;
        auto zR9C9 = state[ch].zR9C9;
        auto vGSJ1 = state[ch].vGSJ1;
        auto vD1D2 = state[ch].vD1D2;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            // --- Newton-Raphson solve: J1_D1D2
            const auto _J1_D1D2_t1 = (gRL + gC13);
            const auto _J1_D1D2_t3 = (gC13 + gR13);
            const auto _J1_D1D2_t11 = (gEamp_Rcomp + gEamp_Ccomp);
            const auto _J1_D1D2_t14 = (gC12 + gC11);
            const auto _J1_D1D2_t18 = (gEamp_Rout + gRdC5);
            const auto _J1_D1D2_t23 = (gR9C9 + gRfR10);
            const auto _J1_D1D2_t24 = (gRfR10 * gRfR10);
            const auto _J1_D1D2_t34 = (gC4 + gEamp_Rin);
            const auto _J1_D1D2_t44 = (gEamp_Rin + gR7C6);
            const auto _J1_D1D2_t55 = (Eamp_Ios / 2.0);
            const auto _J1_D1D2_t58 = (gR6C3 * vi);
            const auto _J1_D1D2_t63 = (gRfR10 * gR9C9);
            const auto _J1_D1D2_t68 = (gEamp_Rcomp * _J1_D1D2_t63);
            const auto _J1_D1D2_t76 = (zRdC5 - zR9C9);
            const auto _J1_D1D2_t83 = (zC12 + zC11);
            const auto _J1_D1D2_t95 = (Eamp_Ibias - _J1_D1D2_t55);
            const auto _J1_D1D2_t97 = (zR7C6 + zR8C7);
            const auto _J1_D1D2_t109 = (Eamp_Aol * Eamp_Vos);
            const auto _J1_D1D2_t111 = (zEamp_Ccomp * _J1_D1D2_t63);
            const auto _J1_D1D2_t116 = (_J1_D1D2_t83 * gR9C9);
            const auto _J1_D1D2_t135 = (gR11 + gC12);
            const auto _J1_D1D2_t146 = (gC12 * _J1_D1D2_t23);
            const auto _J1_D1D2_t153 = (gEamp_Rcomp * _J1_D1D2_t146);
            const auto _J1_D1D2_t155 = (gEamp_Rout * _J1_D1D2_t146);
            const auto _J1_D1D2_t159 = (gC12 * gR9C9);
            const auto _J1_D1D2_t167 = (gEamp_Rin * _J1_D1D2_t155);
            const auto _J1_D1D2_t180 = (1.0 / 1000.0);
            const auto _J1_D1D2_t194 = (1.0 / D1N914_vt);
            const auto _J1_D1D2_t271 = (_J1_D1D2_t83 * gRfR10);
            const auto _J1_D1D2_t331 = (zEamp_Ccomp * _J1_D1D2_t159);
            const auto _J1_D1D2_t333 = (gEamp_Rcomp * _J1_D1D2_t159);
            const auto _J1_D1D2_t338 = (Eamp_Aol * _J1_D1D2_t333);
            const auto _J1_D1D2_t356 = (gEamp_Rin * _J1_D1D2_t338);
            const auto _J1_D1D2_t364 = (gC12 * gRfR10);
            const auto _J1_D1D2_t370 = (gEamp_Rout * _J1_D1D2_t364);
            const auto _J1_D1D2_t373 = (gEamp_Rcomp * _J1_D1D2_t364);
            const auto _J1_D1D2_t378 = (gEamp_Rin * _J1_D1D2_t370);
            const auto _J1_D1D2_t10 = (_J1_D1D2_t11 + gEamp_Rout);
            const auto _J1_D1D2_t13 = (_J1_D1D2_t14 + gRfR10);
            const auto _J1_D1D2_t17 = (_J1_D1D2_t18 + gR9C9);
            const auto _J1_D1D2_t22 = (_J1_D1D2_t13 * _J1_D1D2_t23);
            const auto _J1_D1D2_t33 = (_J1_D1D2_t34 + gR6C3);
            const auto _J1_D1D2_t43 = (_J1_D1D2_t44 + gR8C7);
            const auto _J1_D1D2_t54 = (Eamp_Ibias + _J1_D1D2_t55);
            const auto _J1_D1D2_t57 = (_J1_D1D2_t58 + zR6C3);
            const auto _J1_D1D2_t62 = (_J1_D1D2_t10 * _J1_D1D2_t63);
            const auto _J1_D1D2_t67 = (Eamp_Aol * _J1_D1D2_t68);
            const auto _J1_D1D2_t69 = (gEamp_Rin * _J1_D1D2_t67);
            const auto _J1_D1D2_t82 = (_J1_D1D2_t83 * _J1_D1D2_t23);
            const auto _J1_D1D2_t96 = (_J1_D1D2_t97 + zRdC5);
            const auto _J1_D1D2_t108 = (_J1_D1D2_t109 * _J1_D1D2_t68);
            const auto _J1_D1D2_t115 = (_J1_D1D2_t10 * _J1_D1D2_t116);
            const auto _J1_D1D2_t129 = (gEamp_Rin * _J1_D1D2_t115);
            const auto _J1_D1D2_t145 = (_J1_D1D2_t10 * _J1_D1D2_t146);
            const auto _J1_D1D2_t148 = (gRdC5 * _J1_D1D2_t145);
            const auto _J1_D1D2_t152 = (Eamp_Aol * _J1_D1D2_t153);
            const auto _J1_D1D2_t158 = (_J1_D1D2_t10 * _J1_D1D2_t159);
            const auto _J1_D1D2_t164 = (gEamp_Rin * _J1_D1D2_t145);
            const auto _J1_D1D2_t169 = (gEamp_Rin * _J1_D1D2_t158);
            const auto _J1_D1D2_t332 = (_J1_D1D2_t109 * _J1_D1D2_t333);
            const auto _J1_D1D2_t349 = (_J1_D1D2_t76 * _J1_D1D2_t169);
            const auto _J1_D1D2_t352 = (gRdC5 * _J1_D1D2_t169);
            const auto _J1_D1D2_t363 = (_J1_D1D2_t10 * _J1_D1D2_t364);
            const auto _J1_D1D2_t372 = (Eamp_Aol * _J1_D1D2_t373);
            const auto _J1_D1D2_t380 = (gEamp_Rin * _J1_D1D2_t363);
            const auto _J1_D1D2_t12 = (_J1_D1D2_t13 * gR9C9);
            const auto _J1_D1D2_t21 = (_J1_D1D2_t22 - _J1_D1D2_t24);
            const auto _J1_D1D2_t29 = (gEamp_Rcomp * _J1_D1D2_t21);
            const auto _J1_D1D2_t31 = (gEamp_Rout * _J1_D1D2_t21);
            const auto _J1_D1D2_t42 = (_J1_D1D2_t43 + gRdC5);
            const auto _J1_D1D2_t47 = (_J1_D1D2_t42 * _J1_D1D2_t31);
            const auto _J1_D1D2_t56 = (zC4 + _J1_D1D2_t57);
            const auto _J1_D1D2_t61 = (gEamp_Rin * _J1_D1D2_t62);
            const auto _J1_D1D2_t66 = (_J1_D1D2_t42 * _J1_D1D2_t67);
            const auto _J1_D1D2_t77 = (_J1_D1D2_t42 * _J1_D1D2_t62);
            const auto _J1_D1D2_t94 = (_J1_D1D2_t95 - _J1_D1D2_t96);
            const auto _J1_D1D2_t102 = (_J1_D1D2_t94 * _J1_D1D2_t67);
            const auto _J1_D1D2_t114 = (_J1_D1D2_t42 * _J1_D1D2_t115);
            const auto _J1_D1D2_t121 = (_J1_D1D2_t76 * _J1_D1D2_t61);
            const auto _J1_D1D2_t128 = (gR9C9 * _J1_D1D2_t129);
            const auto _J1_D1D2_t144 = (_J1_D1D2_t42 * _J1_D1D2_t145);
            const auto _J1_D1D2_t147 = (gRdC5 * _J1_D1D2_t148);
            const auto _J1_D1D2_t151 = (gRdC5 * _J1_D1D2_t152);
            const auto _J1_D1D2_t154 = (_J1_D1D2_t42 * _J1_D1D2_t155);
            const auto _J1_D1D2_t157 = (_J1_D1D2_t42 * _J1_D1D2_t158);
            const auto _J1_D1D2_t163 = (_J1_D1D2_t17 * _J1_D1D2_t164);
            const auto _J1_D1D2_t166 = (_J1_D1D2_t151 - _J1_D1D2_t167);
            const auto _J1_D1D2_t168 = (gR9C9 * _J1_D1D2_t169);
            const auto _J1_D1D2_t279 = (zEamp_Ccomp * _J1_D1D2_t12);
            const auto _J1_D1D2_t282 = (gEamp_Rcomp * _J1_D1D2_t12);
            const auto _J1_D1D2_t287 = (Eamp_Aol * _J1_D1D2_t282);
            const auto _J1_D1D2_t307 = (gEamp_Rin * _J1_D1D2_t287);
            const auto _J1_D1D2_t308 = (_J1_D1D2_t42 * _J1_D1D2_t287);
            const auto _J1_D1D2_t321 = (_J1_D1D2_t94 * _J1_D1D2_t158);
            const auto _J1_D1D2_t337 = (_J1_D1D2_t94 * _J1_D1D2_t338);
            const auto _J1_D1D2_t339 = (_J1_D1D2_t76 * _J1_D1D2_t157);
            const auto _J1_D1D2_t355 = (_J1_D1D2_t42 * _J1_D1D2_t338);
            const auto _J1_D1D2_t362 = (gRdC5 * _J1_D1D2_t363);
            const auto _J1_D1D2_t366 = (_J1_D1D2_t42 * _J1_D1D2_t363);
            const auto _J1_D1D2_t369 = (_J1_D1D2_t42 * _J1_D1D2_t370);
            const auto _J1_D1D2_t371 = (gRdC5 * _J1_D1D2_t372);
            const auto _J1_D1D2_t377 = (_J1_D1D2_t378 - _J1_D1D2_t371);
            const auto _J1_D1D2_t379 = (_J1_D1D2_t17 * _J1_D1D2_t380);
            const auto _J1_D1D2_t9 = (_J1_D1D2_t10 * _J1_D1D2_t12);
            const auto _J1_D1D2_t20 = (_J1_D1D2_t10 * _J1_D1D2_t21);
            const auto _J1_D1D2_t28 = (Eamp_Aol * _J1_D1D2_t29);
            const auto _J1_D1D2_t30 = (gEamp_Rin * _J1_D1D2_t31);
            const auto _J1_D1D2_t39 = (gRdC5 * _J1_D1D2_t20);
            const auto _J1_D1D2_t41 = (_J1_D1D2_t42 * _J1_D1D2_t20);
            const auto _J1_D1D2_t49 = (_J1_D1D2_t42 * _J1_D1D2_t9);
            const auto _J1_D1D2_t53 = (_J1_D1D2_t54 - _J1_D1D2_t56);
            const auto _J1_D1D2_t60 = (gRdC5 * _J1_D1D2_t61);
            const auto _J1_D1D2_t65 = (_J1_D1D2_t66 - _J1_D1D2_t69);
            const auto _J1_D1D2_t75 = (_J1_D1D2_t76 * _J1_D1D2_t77);
            const auto _J1_D1D2_t93 = (_J1_D1D2_t94 * _J1_D1D2_t62);
            const auto _J1_D1D2_t113 = (gR9C9 * _J1_D1D2_t114);
            const auto _J1_D1D2_t143 = (_J1_D1D2_t17 * _J1_D1D2_t144);
            const auto _J1_D1D2_t150 = (_J1_D1D2_t151 - _J1_D1D2_t154);
            const auto _J1_D1D2_t156 = (gR9C9 * _J1_D1D2_t157);
            const auto _J1_D1D2_t165 = (gEamp_Rout * _J1_D1D2_t166);
            const auto _J1_D1D2_t281 = (_J1_D1D2_t109 * _J1_D1D2_t282);
            const auto _J1_D1D2_t286 = (_J1_D1D2_t94 * _J1_D1D2_t287);
            const auto _J1_D1D2_t295 = (_J1_D1D2_t94 * _J1_D1D2_t9);
            const auto _J1_D1D2_t302 = (_J1_D1D2_t76 * _J1_D1D2_t49);
            const auto _J1_D1D2_t306 = (_J1_D1D2_t307 - _J1_D1D2_t308);
            const auto _J1_D1D2_t354 = (_J1_D1D2_t355 - _J1_D1D2_t356);
            const auto _J1_D1D2_t361 = (gRdC5 * _J1_D1D2_t362);
            const auto _J1_D1D2_t365 = (_J1_D1D2_t17 * _J1_D1D2_t366);
            const auto _J1_D1D2_t368 = (_J1_D1D2_t369 - _J1_D1D2_t371);
            const auto _J1_D1D2_t376 = (gEamp_Rout * _J1_D1D2_t377);
            const auto _J1_D1D2_t8 = (gEamp_Rin * _J1_D1D2_t9);
            const auto _J1_D1D2_t19 = (gEamp_Rin * _J1_D1D2_t20);
            const auto _J1_D1D2_t27 = (gRdC5 * _J1_D1D2_t28);
            const auto _J1_D1D2_t38 = (gRdC5 * _J1_D1D2_t39);
            const auto _J1_D1D2_t40 = (_J1_D1D2_t17 * _J1_D1D2_t41);
            const auto _J1_D1D2_t46 = (_J1_D1D2_t27 - _J1_D1D2_t47);
            const auto _J1_D1D2_t48 = (gR9C9 * _J1_D1D2_t49);
            const auto _J1_D1D2_t64 = (gEamp_Rout * _J1_D1D2_t65);
            const auto _J1_D1D2_t142 = (_J1_D1D2_t143 - _J1_D1D2_t147);
            const auto _J1_D1D2_t149 = (gEamp_Rout * _J1_D1D2_t150);
            const auto _J1_D1D2_t162 = (_J1_D1D2_t163 + _J1_D1D2_t165);
            const auto _J1_D1D2_t265 = (_J1_D1D2_t76 * _J1_D1D2_t8);
            const auto _J1_D1D2_t305 = (gEamp_Rout * _J1_D1D2_t306);
            const auto _J1_D1D2_t309 = (gRdC5 * _J1_D1D2_t8);
            const auto _J1_D1D2_t353 = (gEamp_Rout * _J1_D1D2_t354);
            const auto _J1_D1D2_t360 = (_J1_D1D2_t361 - _J1_D1D2_t365);
            const auto _J1_D1D2_t367 = (gEamp_Rout * _J1_D1D2_t368);
            const auto _J1_D1D2_t375 = (_J1_D1D2_t376 - _J1_D1D2_t379);
            const auto _J1_D1D2_t7 = (gR9C9 * _J1_D1D2_t8);
            const auto _J1_D1D2_t16 = (_J1_D1D2_t17 * _J1_D1D2_t19);
            const auto _J1_D1D2_t26 = (_J1_D1D2_t27 - _J1_D1D2_t30);
            const auto _J1_D1D2_t37 = (_J1_D1D2_t38 - _J1_D1D2_t40);
            const auto _J1_D1D2_t45 = (gEamp_Rout * _J1_D1D2_t46);
            const auto _J1_D1D2_t59 = (_J1_D1D2_t60 + _J1_D1D2_t64);
            const auto _J1_D1D2_t141 = (_J1_D1D2_t142 + _J1_D1D2_t149);
            const auto _J1_D1D2_t161 = (_J1_D1D2_t162 - _J1_D1D2_t168);
            const auto _J1_D1D2_t304 = (_J1_D1D2_t305 - _J1_D1D2_t309);
            const auto _J1_D1D2_t351 = (_J1_D1D2_t352 + _J1_D1D2_t353);
            const auto _J1_D1D2_t359 = (_J1_D1D2_t360 + _J1_D1D2_t367);
            const auto _J1_D1D2_t374 = (gEamp_Rin * _J1_D1D2_t375);
            const auto _J1_D1D2_t25 = (gEamp_Rout * _J1_D1D2_t26);
            const auto _J1_D1D2_t36 = (_J1_D1D2_t37 - _J1_D1D2_t45);
            const auto _J1_D1D2_t52 = (_J1_D1D2_t53 * _J1_D1D2_t59);
            const auto _J1_D1D2_t140 = (_J1_D1D2_t141 - _J1_D1D2_t156);
            const auto _J1_D1D2_t160 = (gEamp_Rin * _J1_D1D2_t161);
            const auto _J1_D1D2_t303 = (_J1_D1D2_t53 * _J1_D1D2_t304);
            const auto _J1_D1D2_t350 = (_J1_D1D2_t53 * _J1_D1D2_t351);
            const auto _J1_D1D2_t358 = (_J1_D1D2_t33 * _J1_D1D2_t359);
            const auto _J1_D1D2_t15 = (_J1_D1D2_t16 + _J1_D1D2_t25);
            const auto _J1_D1D2_t35 = (_J1_D1D2_t36 + _J1_D1D2_t48);
            const auto _J1_D1D2_t139 = (_J1_D1D2_t33 * _J1_D1D2_t140);
            const auto _J1_D1D2_t357 = (_J1_D1D2_t358 - _J1_D1D2_t374);
            const auto _J1_D1D2_t6 = (_J1_D1D2_t7 - _J1_D1D2_t15);
            const auto _J1_D1D2_t32 = (_J1_D1D2_t33 * _J1_D1D2_t35);
            const auto _J1_D1D2_t138 = (_J1_D1D2_t139 - _J1_D1D2_t160);
            const auto _J1_D1D2_t174 = (_J1_D1D2_t3 * _J1_D1D2_t138);
            const auto _J1_D1D2_t5 = (gEamp_Rin * _J1_D1D2_t6);
            const auto _J1_D1D2_t137 = (gC13 * _J1_D1D2_t138);
            const auto _J1_D1D2_t173 = (gC12 * _J1_D1D2_t174);
            const auto _J1_D1D2_t4 = (_J1_D1D2_t5 - _J1_D1D2_t32);
            const auto _J1_D1D2_t130 = (gC13 * _J1_D1D2_t4);
            const auto _J1_D1D2_t134 = (_J1_D1D2_t135 * _J1_D1D2_t130);
            const auto _J1_D1D2_t136 = (gC12 * _J1_D1D2_t137);
            const auto _J1_D1D2_t2 = (_J1_D1D2_t3 * _J1_D1D2_t4);
            const auto _J1_D1D2_t133 = (_J1_D1D2_t134 - _J1_D1D2_t136);
            const auto _J1_D1D2_t172 = (_J1_D1D2_t135 * _J1_D1D2_t2);
            const auto _J1_D1D2_t132 = (gC13 * _J1_D1D2_t133);
            const auto _J1_D1D2_t171 = (_J1_D1D2_t172 - _J1_D1D2_t173);
            const auto _J1_D1D2_t170 = (_J1_D1D2_t1 * _J1_D1D2_t171);
            const auto _J1_D1D2_t131 = (1.0 / (_J1_D1D2_t132 - _J1_D1D2_t170));
            const auto _J1_D1D2_t385 = (zC12 * _J1_D1D2_t2);
            const auto _J1_D1D2_t386 = (zC12 * _J1_D1D2_t130);
            const auto _J1_D1D2_t387 = (zC13 * _J1_D1D2_t133);
            const auto _J1_D1D2_t388 = (zC12 * (gC13 * _J1_D1D2_t357));
            const auto _J1_D1D2_t389 = (zC12 * (_J1_D1D2_t3 * _J1_D1D2_t357));
            for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
            {
                const auto _J1_D1D2_t89 = (vD1D2 / D1N914_vt);
                const auto _J1_D1D2_t179 = (vGSJ1 - _2N5485_vp);
                const auto _J1_D1D2_t88 = exp(_J1_D1D2_t89);
                const auto _J1_D1D2_t90 = (1.0 / _J1_D1D2_t88);
                const auto _J1_D1D2_t178 = (_J1_D1D2_t179 + _J1_D1D2_t180);
                const auto _J1_D1D2_t192 = (_J1_D1D2_t88 / D1N914_vt);
                const auto _J1_D1D2_t193 = (_J1_D1D2_t194 / _J1_D1D2_t88);
                const auto _J1_D1D2_t384 = (_J1_D1D2_t178 + _J1_D1D2_t178);
                const auto _J1_D1D2_t87 = (_J1_D1D2_t88 - _J1_D1D2_t90);
                const auto _J1_D1D2_t177 = (_J1_D1D2_t178 * _J1_D1D2_t178);
                const auto _J1_D1D2_t191 = (_J1_D1D2_t192 + _J1_D1D2_t193);
                const auto _J1_D1D2_t383 = (_2N5485_Beta * _J1_D1D2_t384);
                const auto _J1_D1D2_t86 = (D1N914_Is * _J1_D1D2_t87);
                const auto _J1_D1D2_t176 = (_2N5485_Beta * _J1_D1D2_t177);
                const auto _J1_D1D2_t190 = (D1N914_Is * _J1_D1D2_t191);
                const auto _J1_D1D2_t218 = (gC12 * _J1_D1D2_t190);
                const auto _J1_D1D2_t224 = (gEamp_Rout * _J1_D1D2_t218);
                const auto _J1_D1D2_t227 = (gEamp_Rcomp * _J1_D1D2_t218);
                const auto _J1_D1D2_t232 = (gEamp_Rin * _J1_D1D2_t224);
                const auto _J1_D1D2_t242 = (gRfR10 * _J1_D1D2_t190);
                const auto _J1_D1D2_t245 = (gEamp_Rcomp * _J1_D1D2_t242);
                const auto _J1_D1D2_t248 = (_J1_D1D2_t10 * _J1_D1D2_t242);
                const auto _J1_D1D2_t253 = (gRdC5 * _J1_D1D2_t248);
                const auto _J1_D1D2_t255 = (_J1_D1D2_t42 * _J1_D1D2_t248);
                const auto _J1_D1D2_t85 = (zR9C9 + _J1_D1D2_t86);
                const auto _J1_D1D2_t175 = (zC13 + _J1_D1D2_t176);
                const auto _J1_D1D2_t189 = (_J1_D1D2_t13 * _J1_D1D2_t190);
                const auto _J1_D1D2_t197 = (gEamp_Rcomp * _J1_D1D2_t189);
                const auto _J1_D1D2_t200 = (_J1_D1D2_t10 * _J1_D1D2_t189);
                const auto _J1_D1D2_t205 = (_J1_D1D2_t42 * _J1_D1D2_t200);
                const auto _J1_D1D2_t207 = (gRdC5 * _J1_D1D2_t200);
                const auto _J1_D1D2_t217 = (_J1_D1D2_t10 * _J1_D1D2_t218);
                const auto _J1_D1D2_t220 = (_J1_D1D2_t42 * _J1_D1D2_t217);
                const auto _J1_D1D2_t223 = (_J1_D1D2_t42 * _J1_D1D2_t224);
                const auto _J1_D1D2_t226 = (Eamp_Aol * _J1_D1D2_t227);
                const auto _J1_D1D2_t234 = (gEamp_Rin * _J1_D1D2_t217);
                const auto _J1_D1D2_t241 = (gEamp_Rout * _J1_D1D2_t242);
                const auto _J1_D1D2_t244 = (Eamp_Aol * _J1_D1D2_t245);
                const auto _J1_D1D2_t247 = (gEamp_Rin * _J1_D1D2_t248);
                const auto _J1_D1D2_t252 = (gRdC5 * _J1_D1D2_t253);
                const auto _J1_D1D2_t254 = (_J1_D1D2_t17 * _J1_D1D2_t255);
                const auto _J1_D1D2_t258 = (_J1_D1D2_t42 * _J1_D1D2_t241);
                const auto _J1_D1D2_t272 = (_J1_D1D2_t13 * _J1_D1D2_t85);
                const auto _J1_D1D2_t320 = (gC12 * _J1_D1D2_t85);
                const auto _J1_D1D2_t330 = (gEamp_Rout * _J1_D1D2_t320);
                const auto _J1_D1D2_t336 = (gEamp_Rcomp * _J1_D1D2_t320);
                const auto _J1_D1D2_t84 = (gRfR10 * _J1_D1D2_t85);
                const auto _J1_D1D2_t188 = (gEamp_Rout * _J1_D1D2_t189);
                const auto _J1_D1D2_t196 = (Eamp_Aol * _J1_D1D2_t197);
                const auto _J1_D1D2_t199 = (gEamp_Rin * _J1_D1D2_t200);
                const auto _J1_D1D2_t204 = (_J1_D1D2_t17 * _J1_D1D2_t205);
                const auto _J1_D1D2_t206 = (gRdC5 * _J1_D1D2_t207);
                const auto _J1_D1D2_t210 = (_J1_D1D2_t42 * _J1_D1D2_t188);
                const auto _J1_D1D2_t216 = (gRdC5 * _J1_D1D2_t217);
                const auto _J1_D1D2_t219 = (_J1_D1D2_t17 * _J1_D1D2_t220);
                const auto _J1_D1D2_t225 = (gRdC5 * _J1_D1D2_t226);
                const auto _J1_D1D2_t231 = (_J1_D1D2_t232 - _J1_D1D2_t225);
                const auto _J1_D1D2_t233 = (_J1_D1D2_t17 * _J1_D1D2_t234);
                const auto _J1_D1D2_t240 = (gEamp_Rin * _J1_D1D2_t241);
                const auto _J1_D1D2_t243 = (gRdC5 * _J1_D1D2_t244);
                const auto _J1_D1D2_t246 = (_J1_D1D2_t17 * _J1_D1D2_t247);
                const auto _J1_D1D2_t251 = (_J1_D1D2_t252 - _J1_D1D2_t254);
                const auto _J1_D1D2_t257 = (_J1_D1D2_t258 - _J1_D1D2_t243);
                const auto _J1_D1D2_t270 = (_J1_D1D2_t271 - _J1_D1D2_t272);
                const auto _J1_D1D2_t280 = (gEamp_Rout * _J1_D1D2_t270);
                const auto _J1_D1D2_t285 = (gEamp_Rcomp * _J1_D1D2_t270);
                const auto _J1_D1D2_t319 = (_J1_D1D2_t10 * _J1_D1D2_t320);
                const auto _J1_D1D2_t323 = (_J1_D1D2_t42 * _J1_D1D2_t319);
                const auto _J1_D1D2_t329 = (_J1_D1D2_t330 + _J1_D1D2_t331);
                const auto _J1_D1D2_t335 = (Eamp_Aol * _J1_D1D2_t336);
                const auto _J1_D1D2_t348 = (gEamp_Rin * _J1_D1D2_t319);
                const auto _J1_D1D2_t81 = (_J1_D1D2_t82 - _J1_D1D2_t84);
                const auto _J1_D1D2_t105 = (gEamp_Rcomp * _J1_D1D2_t81);
                const auto _J1_D1D2_t112 = (gEamp_Rout * _J1_D1D2_t81);
                const auto _J1_D1D2_t187 = (gEamp_Rin * _J1_D1D2_t188);
                const auto _J1_D1D2_t195 = (gRdC5 * _J1_D1D2_t196);
                const auto _J1_D1D2_t198 = (_J1_D1D2_t17 * _J1_D1D2_t199);
                const auto _J1_D1D2_t203 = (_J1_D1D2_t204 - _J1_D1D2_t206);
                const auto _J1_D1D2_t209 = (_J1_D1D2_t210 - _J1_D1D2_t195);
                const auto _J1_D1D2_t215 = (gRdC5 * _J1_D1D2_t216);
                const auto _J1_D1D2_t222 = (_J1_D1D2_t223 - _J1_D1D2_t225);
                const auto _J1_D1D2_t230 = (gEamp_Rout * _J1_D1D2_t231);
                const auto _J1_D1D2_t239 = (_J1_D1D2_t240 - _J1_D1D2_t243);
                const auto _J1_D1D2_t256 = (gEamp_Rout * _J1_D1D2_t257);
                const auto _J1_D1D2_t269 = (_J1_D1D2_t10 * _J1_D1D2_t270);
                const auto _J1_D1D2_t278 = (_J1_D1D2_t279 - _J1_D1D2_t280);
                const auto _J1_D1D2_t284 = (Eamp_Aol * _J1_D1D2_t285);
                const auto _J1_D1D2_t294 = (gRdC5 * _J1_D1D2_t269);
                const auto _J1_D1D2_t297 = (_J1_D1D2_t42 * _J1_D1D2_t269);
                const auto _J1_D1D2_t318 = (gRdC5 * _J1_D1D2_t319);
                const auto _J1_D1D2_t322 = (_J1_D1D2_t17 * _J1_D1D2_t323);
                const auto _J1_D1D2_t328 = (_J1_D1D2_t329 + _J1_D1D2_t332);
                const auto _J1_D1D2_t334 = (gRdC5 * _J1_D1D2_t335);
                const auto _J1_D1D2_t346 = (gEamp_Rin * _J1_D1D2_t328);
                const auto _J1_D1D2_t347 = (_J1_D1D2_t17 * _J1_D1D2_t348);
                const auto _J1_D1D2_t80 = (_J1_D1D2_t10 * _J1_D1D2_t81);
                const auto _J1_D1D2_t98 = (gRdC5 * _J1_D1D2_t80);
                const auto _J1_D1D2_t104 = (Eamp_Aol * _J1_D1D2_t105);
                const auto _J1_D1D2_t110 = (_J1_D1D2_t111 - _J1_D1D2_t112);
                const auto _J1_D1D2_t123 = (gEamp_Rin * _J1_D1D2_t80);
                const auto _J1_D1D2_t186 = (_J1_D1D2_t187 - _J1_D1D2_t195);
                const auto _J1_D1D2_t208 = (gEamp_Rout * _J1_D1D2_t209);
                const auto _J1_D1D2_t214 = (_J1_D1D2_t215 - _J1_D1D2_t219);
                const auto _J1_D1D2_t221 = (gEamp_Rout * _J1_D1D2_t222);
                const auto _J1_D1D2_t229 = (_J1_D1D2_t230 - _J1_D1D2_t233);
                const auto _J1_D1D2_t238 = (gEamp_Rout * _J1_D1D2_t239);
                const auto _J1_D1D2_t250 = (_J1_D1D2_t251 + _J1_D1D2_t256);
                const auto _J1_D1D2_t268 = (gEamp_Rin * _J1_D1D2_t269);
                const auto _J1_D1D2_t277 = (_J1_D1D2_t278 + _J1_D1D2_t281);
                const auto _J1_D1D2_t283 = (gRdC5 * _J1_D1D2_t284);
                const auto _J1_D1D2_t293 = (_J1_D1D2_t294 + _J1_D1D2_t295);
                const auto _J1_D1D2_t296 = (_J1_D1D2_t17 * _J1_D1D2_t297);
                const auto _J1_D1D2_t301 = (_J1_D1D2_t42 * _J1_D1D2_t277);
                const auto _J1_D1D2_t317 = (_J1_D1D2_t318 - _J1_D1D2_t321);
                const auto _J1_D1D2_t327 = (_J1_D1D2_t42 * _J1_D1D2_t328);
                const auto _J1_D1D2_t345 = (_J1_D1D2_t346 - _J1_D1D2_t334);
                const auto _J1_D1D2_t79 = (_J1_D1D2_t42 * _J1_D1D2_t80);
                const auto _J1_D1D2_t92 = (_J1_D1D2_t93 + _J1_D1D2_t98);
                const auto _J1_D1D2_t103 = (gRdC5 * _J1_D1D2_t104);
                const auto _J1_D1D2_t107 = (_J1_D1D2_t108 + _J1_D1D2_t110);
                const auto _J1_D1D2_t122 = (_J1_D1D2_t17 * _J1_D1D2_t123);
                const auto _J1_D1D2_t127 = (gEamp_Rin * _J1_D1D2_t107);
                const auto _J1_D1D2_t185 = (gEamp_Rout * _J1_D1D2_t186);
                const auto _J1_D1D2_t202 = (_J1_D1D2_t203 - _J1_D1D2_t208);
                const auto _J1_D1D2_t213 = (_J1_D1D2_t214 + _J1_D1D2_t221);
                const auto _J1_D1D2_t228 = (gEamp_Rin * _J1_D1D2_t229);
                const auto _J1_D1D2_t237 = (_J1_D1D2_t238 - _J1_D1D2_t246);
                const auto _J1_D1D2_t249 = (_J1_D1D2_t33 * _J1_D1D2_t250);
                const auto _J1_D1D2_t267 = (_J1_D1D2_t17 * _J1_D1D2_t268);
                const auto _J1_D1D2_t276 = (gEamp_Rin * _J1_D1D2_t277);
                const auto _J1_D1D2_t292 = (gRdC5 * _J1_D1D2_t293);
                const auto _J1_D1D2_t300 = (_J1_D1D2_t283 + _J1_D1D2_t301);
                const auto _J1_D1D2_t316 = (gRdC5 * _J1_D1D2_t317);
                const auto _J1_D1D2_t326 = (_J1_D1D2_t327 - _J1_D1D2_t334);
                const auto _J1_D1D2_t344 = (_J1_D1D2_t345 + _J1_D1D2_t337);
                const auto _J1_D1D2_t78 = (_J1_D1D2_t17 * _J1_D1D2_t79);
                const auto _J1_D1D2_t91 = (gRdC5 * _J1_D1D2_t92);
                const auto _J1_D1D2_t101 = (_J1_D1D2_t102 + _J1_D1D2_t103);
                const auto _J1_D1D2_t106 = (_J1_D1D2_t42 * _J1_D1D2_t107);
                const auto _J1_D1D2_t120 = (_J1_D1D2_t121 - _J1_D1D2_t122);
                const auto _J1_D1D2_t126 = (_J1_D1D2_t102 + _J1_D1D2_t127);
                const auto _J1_D1D2_t184 = (_J1_D1D2_t185 - _J1_D1D2_t198);
                const auto _J1_D1D2_t201 = (_J1_D1D2_t33 * _J1_D1D2_t202);
                const auto _J1_D1D2_t212 = (_J1_D1D2_t33 * _J1_D1D2_t213);
                const auto _J1_D1D2_t236 = (gEamp_Rin * _J1_D1D2_t237);
                const auto _J1_D1D2_t275 = (_J1_D1D2_t276 + _J1_D1D2_t283);
                const auto _J1_D1D2_t291 = (_J1_D1D2_t292 - _J1_D1D2_t296);
                const auto _J1_D1D2_t299 = (_J1_D1D2_t300 + _J1_D1D2_t286);
                const auto _J1_D1D2_t315 = (_J1_D1D2_t316 - _J1_D1D2_t322);
                const auto _J1_D1D2_t325 = (_J1_D1D2_t326 + _J1_D1D2_t337);
                const auto _J1_D1D2_t343 = (gEamp_Rout * _J1_D1D2_t344);
                const auto _J1_D1D2_t74 = (_J1_D1D2_t75 - _J1_D1D2_t78);
                const auto _J1_D1D2_t100 = (_J1_D1D2_t101 + _J1_D1D2_t106);
                const auto _J1_D1D2_t125 = (_J1_D1D2_t126 + _J1_D1D2_t103);
                const auto _J1_D1D2_t183 = (gEamp_Rin * _J1_D1D2_t184);
                const auto _J1_D1D2_t211 = (_J1_D1D2_t212 - _J1_D1D2_t228);
                const auto _J1_D1D2_t235 = (_J1_D1D2_t236 - _J1_D1D2_t249);
                const auto _J1_D1D2_t274 = (_J1_D1D2_t275 + _J1_D1D2_t286);
                const auto _J1_D1D2_t298 = (gEamp_Rout * _J1_D1D2_t299);
                const auto _J1_D1D2_t324 = (gEamp_Rout * _J1_D1D2_t325);
                const auto _J1_D1D2_t342 = (_J1_D1D2_t343 - _J1_D1D2_t347);
                const auto _J1_D1D2_t73 = (_J1_D1D2_t74 + _J1_D1D2_t91);
                const auto _J1_D1D2_t99 = (gEamp_Rout * _J1_D1D2_t100);
                const auto _J1_D1D2_t124 = (gEamp_Rout * _J1_D1D2_t125);
                const auto _J1_D1D2_t182 = (_J1_D1D2_t183 + _J1_D1D2_t201);
                const auto _J1_D1D2_t273 = (gEamp_Rout * _J1_D1D2_t274);
                const auto _J1_D1D2_t290 = (_J1_D1D2_t291 - _J1_D1D2_t298);
                const auto _J1_D1D2_t314 = (_J1_D1D2_t315 + _J1_D1D2_t324);
                const auto _J1_D1D2_t341 = (_J1_D1D2_t342 - _J1_D1D2_t349);
                const auto _J1_D1D2_t72 = (_J1_D1D2_t73 - _J1_D1D2_t99);
                const auto _J1_D1D2_t119 = (_J1_D1D2_t120 - _J1_D1D2_t124);
                const auto _J1_D1D2_t266 = (_J1_D1D2_t267 + _J1_D1D2_t273);
                const auto _J1_D1D2_t289 = (_J1_D1D2_t290 + _J1_D1D2_t302);
                const auto _J1_D1D2_t313 = (_J1_D1D2_t314 - _J1_D1D2_t339);
                const auto _J1_D1D2_t340 = (gEamp_Rin * _J1_D1D2_t341);
                const auto _J1_D1D2_t71 = (_J1_D1D2_t72 + _J1_D1D2_t113);
                const auto _J1_D1D2_t118 = (_J1_D1D2_t119 + _J1_D1D2_t128);
                const auto _J1_D1D2_t264 = (_J1_D1D2_t265 - _J1_D1D2_t266);
                const auto _J1_D1D2_t288 = (_J1_D1D2_t33 * _J1_D1D2_t289);
                const auto _J1_D1D2_t312 = (_J1_D1D2_t33 * _J1_D1D2_t313);
                const auto _J1_D1D2_t70 = (_J1_D1D2_t33 * _J1_D1D2_t71);
                const auto _J1_D1D2_t117 = (gEamp_Rin * _J1_D1D2_t118);
                const auto _J1_D1D2_t181 = ((((_J1_D1D2_t1 * ((_J1_D1D2_t135 * (_J1_D1D2_t3 * _J1_D1D2_t182)) + (gC12 * (_J1_D1D2_t3 * _J1_D1D2_t211)))) - (gC13 * ((_J1_D1D2_t135 * (gC13 * _J1_D1D2_t182)) + (gC12 * (gC13 * _J1_D1D2_t211))))) * _J1_D1D2_t131) - 1.0);
                const auto _J1_D1D2_t263 = (gEamp_Rin * _J1_D1D2_t264);
                const auto _J1_D1D2_t311 = (_J1_D1D2_t312 - _J1_D1D2_t340);
                const auto _J1_D1D2_t382 = (((_J1_D1D2_t1 * ((_J1_D1D2_t135 * (_J1_D1D2_t383 * _J1_D1D2_t4)) - (gC12 * (_J1_D1D2_t383 * _J1_D1D2_t138)))) * _J1_D1D2_t131) - 1.0);
                const auto _J1_D1D2_t51 = (_J1_D1D2_t52 + _J1_D1D2_t70);
                const auto _J1_D1D2_t262 = (_J1_D1D2_t263 - _J1_D1D2_t288);
                const auto _J1_D1D2_t310 = (_J1_D1D2_t311 - _J1_D1D2_t350);
                const auto _J1_D1D2_t381 = (_J1_D1D2_t382 * _J1_D1D2_t181);
                const auto _J1_D1D2_t50 = (_J1_D1D2_t51 - _J1_D1D2_t117);
                const auto _J1_D1D2_t261 = (_J1_D1D2_t262 + _J1_D1D2_t303);
                const auto _J1_D1D2_t0 = ((((_J1_D1D2_t1 * (_J1_D1D2_t385 + (gC12 * (_J1_D1D2_t3 * _J1_D1D2_t50)))) - (gC13 * (_J1_D1D2_t386 + (gC12 * (gC13 * _J1_D1D2_t50))))) * _J1_D1D2_t131) - ((_J1_D1D2_t387 - (_J1_D1D2_t1 * ((_J1_D1D2_t135 * (_J1_D1D2_t175 * _J1_D1D2_t4)) - (gC12 * (_J1_D1D2_t175 * _J1_D1D2_t138))))) * _J1_D1D2_t131));
                const auto _J1_D1D2_t260 = (((gC13 * (((_J1_D1D2_t135 * (gC13 * _J1_D1D2_t261)) - (gC12 * (gC13 * _J1_D1D2_t310))) + _J1_D1D2_t388)) - (_J1_D1D2_t1 * (((_J1_D1D2_t135 * (_J1_D1D2_t3 * _J1_D1D2_t261)) - (gC12 * (_J1_D1D2_t3 * _J1_D1D2_t310))) + _J1_D1D2_t389))) * _J1_D1D2_t131);
                const auto _J1_D1D2_t259 = (vD1D2 - _J1_D1D2_t260);
                const auto res_vGSJ1 = (_J1_D1D2_t0 - vGSJ1);
                const auto delta_vGSJ1 = ((((vGSJ1 - _J1_D1D2_t0) * _J1_D1D2_t181) - ((((_J1_D1D2_t1 * (gC12 * (_J1_D1D2_t3 * _J1_D1D2_t235))) - (gC13 * (gC12 * (gC13 * _J1_D1D2_t235)))) * _J1_D1D2_t131) * _J1_D1D2_t259)) / _J1_D1D2_t381);
                const auto res_vD1D2 = (_J1_D1D2_t260 - vD1D2);
                const auto delta_vD1D2 = ((_J1_D1D2_t382 * _J1_D1D2_t259) / _J1_D1D2_t381);
            
                auto residual_norm_sq = 0.0;
                residual_norm_sq += res_vGSJ1 * res_vGSJ1;
                residual_norm_sq += res_vD1D2 * res_vD1D2;
                auto step_norm_sq = 0.0;
                step_norm_sq += delta_vGSJ1 * delta_vGSJ1;
                step_norm_sq += delta_vD1D2 * delta_vD1D2;
            
                vGSJ1 = limit_jfet_vgs(vGSJ1 + (delta_vGSJ1), _2N5485_vp);
                vD1D2 = limit_junction_voltage(vD1D2 + (delta_vD1D2), vD1D2, D1N914_vt, vcrit_D1N914_vt);
            
                if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
                    break;
                
            }

            const auto _t50 = ((vGSJ1 - _2N5485_vp) + _t226);
            const auto _t62 = (_t227 - ((zR7C6 + zR8C7) + zRdC5));
            const auto _t69 = (zRdC5 - zR9C9);
            const auto _t77 = exp((vD1D2 / D1N914_vt));
            const auto _t79 = (zC12 + zC11);
            const auto _t90 = (_t62 * _t39);
            const auto _t94 = (zEamp_Ccomp * _t33);
            const auto _t127 = (_t94 + _t93);
            const auto _t135 = (zEamp_Ccomp * _t42);
            const auto _t161 = (_t79 * _t15);
            const auto _t165 = (zEamp_Ccomp * _t156);
            const auto _t177 = (_t79 * gR9C9);
            const auto _t49 = (zC13 + (_2N5485_Beta * (_t50 * _t50)));
            const auto _t59 = (_t228 - (zC4 + ((gR6C3 * vi) + zR6C3)));
            const auto _t61 = (_t62 * _t20);
            const auto _t67 = (zEamp_Ccomp * _t14);
            const auto _t76 = (zR9C9 + (D1N914_Is * (_t77 - (1.0 / _t77))));
            const auto _t78 = (_t79 * gRfR10);
            const auto _t87 = (_t69 * _t34);
            const auto _t89 = (_t62 * _t32);
            const auto _t92 = (_t93 + _t94);
            const auto _t95 = (_t62 * _t41);
            const auto _t98 = (gC12 * _t76);
            const auto _t108 = (_t67 + _t65);
            const auto _t114 = (zEamp_Ccomp * _t9);
            const auto _t128 = (_t62 * _t37);
            const auto _t134 = (gEamp_Rout * _t98);
            const auto _t140 = (gEamp_Rcomp * _t98);
            const auto _t141 = (_t62 * _t142);
            const auto _t162 = (gRfR10 * _t76);
            const auto _t172 = (_t62 * _t173);
            const auto _t176 = (_t8 * _t177);
            const auto _t179 = (_t62 * _t155);
            const auto _t185 = (_t25 * _t176);
            const auto _t64 = (_t65 + _t67);
            const auto _t68 = (_t69 * _t23);
            const auto _t71 = (_t62 * _t13);
            const auto _t75 = (_t10 * _t76);
            const auto _t80 = (_t62 * _t7);
            const auto _t88 = (_t11 * _t89);
            const auto _t91 = (gRdC5 * _t92);
            const auto _t97 = (_t8 * _t98);
            const auto _t109 = (_t62 * _t18);
            const auto _t116 = (_t62 * _t105);
            const auto _t133 = (_t134 + _t135);
            const auto _t139 = (Eamp_Aol * _t140);
            const auto _t160 = (_t161 - _t162);
            const auto _t166 = (gEamp_Rout * _t160);
            const auto _t171 = (gEamp_Rcomp * _t160);
            const auto _t175 = (gEamp_Rin * _t176);
            const auto _t182 = (_t69 * _t183);
            const auto _t184 = (gR9C9 * _t185);
            const auto _t205 = (_t25 * _t97);
            const auto _t206 = (gEamp_Rin * _t97);
            const auto _t209 = (_t78 - _t75);
            const auto _t210 = ((_t114 - (gEamp_Rout * _t209)) + _t115);
            const auto _t213 = (gEamp_Rcomp * _t209);
            const auto _t63 = (gRdC5 * _t64);
            const auto _t70 = (_t11 * _t71);
            const auto _t74 = (_t75 - _t78);
            const auto _t96 = (gRdC5 * _t97);
            const auto _t112 = (gEamp_Rcomp * _t74);
            const auto _t113 = (((gEamp_Rout * _t74) + _t114) + _t115);
            const auto _t132 = (_t133 + _t136);
            const auto _t138 = (gRdC5 * _t139);
            const auto _t145 = (gEamp_Rin * _t132);
            const auto _t153 = (_t69 * _t154);
            const auto _t159 = (_t8 * _t160);
            const auto _t164 = (_t165 - _t166);
            const auto _t170 = (Eamp_Aol * _t171);
            const auto _t174 = (gR9C9 * _t175);
            const auto _t178 = (gRdC5 * _t159);
            const auto _t181 = (_t25 * _t159);
            const auto _t194 = (_t167 + _t164);
            const auto _t198 = (_t96 - _t95);
            const auto _t204 = (((_t21 * (((gEamp_Rout * (_t128 + (_t25 * _t92))) - ((_t69 * _t31) + (gRdC5 * _t89))) - (gR9C9 * _t205))) + (_t59 * _t229)) - (gEamp_Rin * (((gEamp_Rout * ((gEamp_Rin * _t92) + _t128)) - (_t69 * _t44)) - (gR9C9 * _t206))));
            const auto _t208 = (_t8 * _t209);
            const auto _t212 = (Eamp_Aol * _t213);
            const auto _t73 = (_t8 * _t74);
            const auto _t86 = ((_t59 * _t30) + (gEamp_Rin * (((_t87 + _t88) - (gEamp_Rout * (_t90 + _t91))) - (gR9C9 * (_t95 - _t96)))));
            const auto _t111 = (Eamp_Aol * _t112);
            const auto _t131 = (_t25 * _t132);
            const auto _t144 = (_t145 - _t138);
            const auto _t158 = (gEamp_Rin * _t159);
            const auto _t163 = (_t164 + _t167);
            const auto _t169 = (gRdC5 * _t170);
            const auto _t180 = (_t11 * _t181);
            const auto _t186 = (_t59 * _t187);
            const auto _t197 = ((_t21 * (((gEamp_Rout * (_t91 + _t90)) - (_t88 + _t87)) - (gR9C9 * _t198))) - (_t59 * _t43));
            const auto _t200 = ((gEamp_Rin * (((_t69 * _t12) - (gEamp_Rout * ((gEamp_Rin * _t64) + _t109))) + (gR9C9 * (gEamp_Rin * _t73)))) - ((_t21 * ((((_t69 * _t24) + (gRdC5 * _t71)) - (gEamp_Rout * (_t109 + (_t25 * _t64)))) + (gR9C9 * (_t25 * _t73)))) + (_t59 * _t230)));
            const auto _t211 = (gRdC5 * _t212);
            const auto _t48 = (_t49 * _t29);
            const auto _t72 = (gRdC5 * _t73);
            const auto _t110 = (gRdC5 * _t111);
            const auto _t130 = (_t131 - _t138);
            const auto _t143 = (_t144 + _t141);
            const auto _t157 = (_t11 * _t158);
            const auto _t193 = ((_t186 + (_t21 * ((((_t182 - _t180) + (gRdC5 * (_t179 + _t178))) - (gEamp_Rout * ((_t172 + _t169) + (_t25 * _t194)))) + _t184))) - (gEamp_Rin * (((_t153 - _t157) - (gEamp_Rout * ((_t172 + (gEamp_Rin * _t194)) + _t169))) + _t174)));
            const auto _t195 = ((_t59 * _t5) - (_t21 * (((_t70 + _t68) - (gEamp_Rout * (_t63 + _t61))) - (gR9C9 * (_t80 - _t72)))));
            const auto _t207 = (((gEamp_Rin * ((_t69 * _t6) - ((_t11 * (gEamp_Rin * _t208)) + (gEamp_Rout * (((gEamp_Rin * _t210) + _t211) + _t116))))) - (_t21 * ((((gRdC5 * ((gRdC5 * _t208) + _t80)) - (_t11 * (_t25 * _t208))) - (gEamp_Rout * ((_t211 + (_t25 * _t210)) + _t116))) + (_t69 * _t27)))) + (_t59 * _t231));
            const auto _t47 = (gC12 * _t48);
            const auto _t52 = (_t49 * _t4);
            const auto _t58 = ((_t59 * _t22) + (gEamp_Rin * (((gEamp_Rout * (_t61 + _t63)) - (_t68 + _t70)) - (gR9C9 * (_t72 - _t80)))));
            const auto _t99 = ((_t59 * _t232) - ((_t21 * ((((gRdC5 * ((gRdC5 * _t108) + _t61)) - (_t11 * ((_t25 * _t108) + _t109))) - (_t69 * _t26)) - (gR9C9 * ((_t110 - (_t25 * _t113)) - _t116)))) + (gEamp_Rin * (((_t11 * ((gEamp_Rin * _t108) + _t109)) + (_t69 * _t16)) + (gR9C9 * ((_t110 - (gEamp_Rin * _t113)) - _t116))))));
            const auto _t129 = (_t130 + _t141);
            const auto _t152 = (((gEamp_Rin * ((_t153 - (_t157 + (gEamp_Rout * (((gEamp_Rin * _t163) + _t169) + _t172)))) + _t174)) - (_t21 * (((((gRdC5 * (_t178 + _t179)) - _t180) - (gEamp_Rout * ((_t169 + (_t25 * _t163)) + _t172))) + _t182) + _t184))) - _t186);
            const auto _t214 = (((_t21 * ((((gRdC5 * _t198) - (_t11 * _t205)) + (gEamp_Rout * _t129)) - (_t69 * _t40))) - (gEamp_Rin * (((gEamp_Rout * _t143) - (_t11 * _t206)) - (_t69 * _t46)))) - (_t59 * _t233));
            const auto _t51 = (_t1 * _t52);
            const auto _t126 = (((_t21 * ((((_t11 * ((_t25 * _t127) + _t128)) - (gRdC5 * ((gRdC5 * _t127) + _t90))) + (_t69 * _t35)) - (gR9C9 * _t129))) - (gEamp_Rin * (((_t11 * ((gEamp_Rin * _t127) + _t128)) + (_t69 * _t45)) - (gR9C9 * _t143)))) + (_t59 * _t234));
            const auto vo = (((zC13 * _t0) + (gC13 * (_t47 - _t51))) * _t53);
            const auto vpos = (((gC13 * (((_t1 * (gC13 * _t58)) - (zC12 * _t216)) + (gC12 * (gC13 * _t86)))) - (_t57 * (((_t1 * (_t3 * _t58)) - (zC12 * _t217)) + (gC12 * (_t3 * _t86))))) * _t53);
            const auto tC4 = (gC4 * (vpos - 0));
            const auto vbw_Eamp = (((gC13 * (((_t1 * (gC13 * _t99)) + (zC12 * _t218)) - (gC12 * (gC13 * _t126)))) - (_t57 * (((_t1 * (_t3 * _t99)) + (zC12 * _t219)) - (gC12 * (_t3 * _t126))))) * _t53);
            const auto tEamp_Ccomp = (gEamp_Ccomp * (vbw_Eamp - 0));
            const auto vf = (((gC13 * ((_t1 * (gC13 * _t152)) - (zC12 * _t56))) - (_t57 * ((_t1 * (_t3 * _t152)) - (zC12 * _t28)))) * _t53);
            const auto vg = (((_t57 * ((zC12 * _t2) + (gC12 * (_t3 * _t193)))) - (gC13 * ((zC12 * _t55) + (gC12 * (gC13 * _t193))))) * _t53);
            const auto tC12 = (gC12 * (vf - vg));
            const auto vneg = (((gC13 * (((_t1 * (gC13 * _t195)) - (zC12 * _t220)) - (gC12 * (gC13 * _t197)))) - (_t57 * (((_t1 * (_t3 * _t195)) - (zC12 * _t221)) - (gC12 * (_t3 * _t197))))) * _t53);
            const auto vs = (((zC13 * _t54) - (_t57 * (_t51 - _t47))) * _t53);
            const auto tC13 = (gC13 * (vs - vo));
            const auto vR6C3 = (vpos - vi);
            const auto tC11 = (gC11 * (vf - 0));
            const auto v2 = (((gC13 * (((_t1 * (gC13 * _t200)) - (zC12 * _t222)) - (gC12 * (gC13 * _t204)))) - (_t57 * (((_t1 * (_t3 * _t200)) - (zC12 * _t223)) - (gC12 * (_t3 * _t204))))) * _t53);
            const auto vRdC5 = (vneg - v2);
            const auto vdp = (((gC13 * (((_t1 * (gC13 * _t207)) - (gC12 * (gC13 * _t214))) + (zC12 * _t224))) - (_t57 * (((_t1 * (_t3 * _t207)) - (gC12 * (_t3 * _t214))) + (zC12 * _t225)))) * _t53);
            const auto vR9C9 = (v2 - vdp);
            const auto _t199 = (vneg - 0);
            const auto vR7C6 = _t199;
            const auto vR8C7 = _t199;
            
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

static void reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc = 0.0f)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto _2N5485_Beta = params._2N5485_Beta;
    const auto _2N5485_vp = params._2N5485_vp;
    const auto Eamp_Aol = params.Eamp_Aol;
    const auto Eamp_Vos = params.Eamp_Vos;
    const auto Eamp_Ios = params.Eamp_Ios;
    const auto Eamp_Ibias = params.Eamp_Ibias;
    const auto D1N914_Is = params.D1N914_Is;
    const auto D1N914_vt = params.D1N914_vt;
    
    const auto Vpp = params.Vpp;
    
    const auto vpp = Vpp;
    
    const auto Vp = params.Vp;
    
    const auto vp = Vp;
    
    const auto gR5 = 1.0 / params.R5;
    
    const auto gRL = 1.0 / params.RL;
    
    const auto gR4 = 1.0 / params.R4;
    
    const auto gC4 = 2.0 * sample_rate * params.C4;
    
    const auto gEamp_Rin = 1.0 / params.Eamp_Rin;
    
    const auto gEamp_Rcomp = 1.0 / params.Eamp_Rcomp;
    
    const auto gEamp_Ccomp = 2.0 * sample_rate * params.Eamp_Ccomp;
    
    const auto gEamp_Rout = 1.0 / params.Eamp_Rout;
    
    const auto gR11 = 1.0 / params.R11;
    
    const auto gC12 = 2.0 * sample_rate * params.C12;
    
    const auto R7 = params.R7;
    const auto C6 = params.C6;
    const auto gR7C6 = (2.0 * sample_rate * C6) / (1.0 + 2.0 * sample_rate * R7 * C6);
    const auto gnR7C6 = gR7C6 * (2.0 * sample_rate * R7 * C6 - 1.0) / (2.0 * sample_rate * C6);
    
    const auto gC13 = 2.0 * sample_rate * params.C13;
    
    const auto R6 = params.R6;
    const auto C3 = params.C3;
    const auto gR6C3 = (2.0 * sample_rate * C3) / (1.0 + 2.0 * sample_rate * R6 * C3);
    const auto gnR6C3 = gR6C3 * (2.0 * sample_rate * R6 * C3 - 1.0) / (2.0 * sample_rate * C3);
    
    const auto gC11 = 2.0 * sample_rate * params.C11;
    
    const auto R8 = params.R8;
    const auto C7 = params.C7;
    const auto gR8C7 = (2.0 * sample_rate * C7) / (1.0 + 2.0 * sample_rate * R8 * C7);
    const auto gnR8C7 = gR8C7 * (2.0 * sample_rate * R8 * C7 - 1.0) / (2.0 * sample_rate * C7);
    
    const auto gR13 = 1.0 / params.R13;
    
    const auto Rd = params.Rd;
    const auto C5 = params.C5;
    const auto gRdC5 = 2.0 * sample_rate * C5 + (1.0 / Rd);
    const auto gzRdC5 = 4.0 * sample_rate * C5;
    
    const auto R9 = params.R9;
    const auto C9 = params.C9;
    const auto gR9C9 = (2.0 * sample_rate * C9) / (1.0 + 2.0 * sample_rate * R9 * C9);
    const auto gnR9C9 = gR9C9 * (2.0 * sample_rate * R9 * C9 - 1.0) / (2.0 * sample_rate * C9);
    
    const auto RfR10 = sum(params.Rf, params.R10);
    const auto gRfR10 = 1.0 / RfR10;
    
    const auto vcrit_D1N914_vt = D1N914_vt * std::log(D1N914_vt / (std::sqrt(2.0) * D1N914_Is));
    
    const auto vi = vi_dc;

    double vGSJ1 = 0;
    double vD1D2 = 0;

    // --- Newton-Raphson solve: J1
    const auto _J1_t4 = (1.0 / 1000.0);
    const auto _J1_t5 = (1.0 / (gR13 + (1.0 / 1000000000.0)));
    for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
    {
        const auto _J1_t3 = (vGSJ1 - _2N5485_vp);
        const auto _J1_t2 = (_J1_t3 + _J1_t4);
        const auto _J1_t1 = ((_2N5485_Beta * (_J1_t2 * _J1_t2)) * _J1_t5);
        const auto _J1_t0 = (_J1_t1 + vGSJ1);
        const auto res_vGSJ1 = (-_J1_t0);
        const auto delta_vGSJ1 = (-(_J1_t0 / (((_2N5485_Beta * (_J1_t2 + _J1_t2)) * _J1_t5) + 1.0)));
    
        auto residual_norm_sq = 0.0;
        residual_norm_sq += res_vGSJ1 * res_vGSJ1;
        auto step_norm_sq = 0.0;
        step_norm_sq += delta_vGSJ1 * delta_vGSJ1;
    
        vGSJ1 = limit_jfet_vgs(vGSJ1 + (delta_vGSJ1), _2N5485_vp);
    
        if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
            break;
        
    }
    // --- Newton-Raphson solve: D1D2
    const auto _D1D2_t4 = (-1.0 / Rd);
    const auto _D1D2_t5 = (1.0 / 1000000000.0);
    const auto _D1D2_t8 = (gEamp_Rcomp + gEamp_Rout);
    const auto _D1D2_t10 = (gRfR10 + _D1D2_t5);
    const auto _D1D2_t20 = (gEamp_Rin + _D1D2_t5);
    const auto _D1D2_t23 = (1.0 / Rd);
    const auto _D1D2_t27 = (_D1D2_t10 * _D1D2_t10);
    const auto _D1D2_t28 = (gRfR10 * gRfR10);
    const auto _D1D2_t39 = (1.0 / D1N914_vt);
    const auto _D1D2_t45 = (Eamp_Aol * gEamp_Rcomp);
    const auto _D1D2_t3 = (_D1D2_t4 - gEamp_Rout);
    const auto _D1D2_t7 = (_D1D2_t8 + _D1D2_t5);
    const auto _D1D2_t22 = (gEamp_Rin + _D1D2_t23);
    const auto _D1D2_t26 = (_D1D2_t27 - _D1D2_t28);
    const auto _D1D2_t29 = (gEamp_Rout * _D1D2_t26);
    const auto _D1D2_t32 = (gEamp_Rcomp * _D1D2_t26);
    const auto _D1D2_t44 = (_D1D2_t45 * _D1D2_t10);
    const auto _D1D2_t2 = (_D1D2_t3 - _D1D2_t5);
    const auto _D1D2_t21 = (_D1D2_t22 + _D1D2_t5);
    const auto _D1D2_t25 = (_D1D2_t7 * _D1D2_t26);
    const auto _D1D2_t31 = (Eamp_Aol * _D1D2_t32);
    const auto _D1D2_t43 = (_D1D2_t44 * D1N914_Is);
    const auto _D1D2_t30 = (_D1D2_t31 / Rd);
    const auto _D1D2_t24 = (1.0 / ((gEamp_Rin * ((_D1D2_t2 * (gEamp_Rin * _D1D2_t25)) + (gEamp_Rout * ((gEamp_Rin * _D1D2_t29) - _D1D2_t30)))) - (_D1D2_t20 * (((_D1D2_t2 * (_D1D2_t21 * _D1D2_t25)) + ((_D1D2_t25 / Rd) / Rd)) + (gEamp_Rout * ((_D1D2_t21 * _D1D2_t29) - _D1D2_t30))))));
    const auto _D1D2_t46 = ((_D1D2_t7 * _D1D2_t10) * D1N914_Is);
    const auto _D1D2_t47 = (1.0 / (Rd * Rd));
    for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
    {
        const auto _D1D2_t14 = (vD1D2 / D1N914_vt);
        const auto _D1D2_t13 = exp(_D1D2_t14);
        const auto _D1D2_t15 = (1.0 / _D1D2_t13);
        const auto _D1D2_t37 = (_D1D2_t13 / D1N914_vt);
        const auto _D1D2_t38 = (_D1D2_t39 / _D1D2_t13);
        const auto _D1D2_t12 = (_D1D2_t13 - _D1D2_t15);
        const auto _D1D2_t36 = (_D1D2_t37 + _D1D2_t38);
        const auto _D1D2_t11 = (D1N914_Is * _D1D2_t12);
        const auto _D1D2_t35 = (D1N914_Is * _D1D2_t36);
        const auto _D1D2_t42 = (_D1D2_t43 * _D1D2_t36);
        const auto _D1D2_t9 = (_D1D2_t10 * _D1D2_t11);
        const auto _D1D2_t16 = (gEamp_Rout * _D1D2_t9);
        const auto _D1D2_t19 = (gEamp_Rcomp * _D1D2_t9);
        const auto _D1D2_t34 = (_D1D2_t10 * _D1D2_t35);
        const auto _D1D2_t40 = (gEamp_Rout * _D1D2_t34);
        const auto _D1D2_t41 = (_D1D2_t42 / Rd);
        const auto _D1D2_t6 = (_D1D2_t7 * _D1D2_t9);
        const auto _D1D2_t18 = (Eamp_Aol * _D1D2_t19);
        const auto _D1D2_t33 = (_D1D2_t7 * _D1D2_t34);
        const auto _D1D2_t17 = (_D1D2_t18 / Rd);
        const auto _D1D2_t1 = (((gEamp_Rin * ((_D1D2_t2 * (gEamp_Rin * _D1D2_t6)) + (gEamp_Rout * ((gEamp_Rin * _D1D2_t16) - _D1D2_t17)))) - (_D1D2_t20 * ((((_D1D2_t6 / Rd) / Rd) + (_D1D2_t2 * (_D1D2_t21 * _D1D2_t6))) + (gEamp_Rout * ((_D1D2_t21 * _D1D2_t16) - _D1D2_t17))))) * _D1D2_t24);
        const auto _D1D2_t0 = (_D1D2_t1 + vD1D2);
        const auto res_vD1D2 = (-_D1D2_t0);
        const auto delta_vD1D2 = (-(_D1D2_t0 / ((((gEamp_Rin * ((_D1D2_t2 * (gEamp_Rin * _D1D2_t33)) + (gEamp_Rout * ((gEamp_Rin * _D1D2_t40) - _D1D2_t41)))) - (_D1D2_t20 * ((((_D1D2_t46 * _D1D2_t36) * _D1D2_t47) + (_D1D2_t2 * (_D1D2_t21 * _D1D2_t33))) + (gEamp_Rout * ((_D1D2_t21 * _D1D2_t40) - _D1D2_t41))))) * _D1D2_t24) + 1.0)));
    
        auto residual_norm_sq = 0.0;
        residual_norm_sq += res_vD1D2 * res_vD1D2;
        auto step_norm_sq = 0.0;
        step_norm_sq += delta_vD1D2 * delta_vD1D2;
    
        vD1D2 = limit_junction_voltage(vD1D2 + (delta_vD1D2), vD1D2, D1N914_vt, vcrit_D1N914_vt);
    
        if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
            break;
        
    }
    const auto zC4 = ((gC4 * (((Eamp_Ibias + (Eamp_Ios / 2.0)) * ((((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))) / Rd) / Rd)) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) + (gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * ((Eamp_Ibias - (Eamp_Ios / 2.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((((Eamp_Aol * Eamp_Vos) * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd) + ((Eamp_Ibias - (Eamp_Ios / 2.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))))))))) / ((gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * ((((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))) / Rd) / Rd)) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd)))))));
    const auto zEamp_Ccomp = ((gEamp_Ccomp * (((gEamp_Rin * ((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * ((gEamp_Rin * ((Eamp_Aol * Eamp_Vos) * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + ((Eamp_Ibias - (Eamp_Ios / 2.0)) * (Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * ((Eamp_Aol * Eamp_Vos) * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + ((Eamp_Ibias - (Eamp_Ios / 2.0)) * (Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))))) + (((((Eamp_Aol * Eamp_Vos) * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd) + ((Eamp_Ibias - (Eamp_Ios / 2.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) / Rd)))) - ((Eamp_Ibias + (Eamp_Ios / 2.0)) * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * ((gEamp_Rin * (Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) - (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))))) + (((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd)) / Rd))))) / ((gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * ((((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))) / Rd) / Rd)) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd)))))));
    const auto zC12 = (-((gC12 * ((gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (gRfR10 * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt))))))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (gRfR10 * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))) - ((Eamp_Aol * (gEamp_Rcomp * (gRfR10 * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))) / Rd))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * (((((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (gRfR10 * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt))))))) / Rd) / Rd) + ((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (gRfR10 * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))))) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (gRfR10 * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))) - ((Eamp_Aol * (gEamp_Rcomp * (gRfR10 * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))) / Rd))))))) / ((gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * ((((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))) / Rd) / Rd)) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))))));
    const auto zR7C6 = ((gR7C6 * (((gEamp_Rin + (1.0 / 1000000000.0)) * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * ((Eamp_Ibias - (Eamp_Ios / 2.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * (((Eamp_Ibias - (Eamp_Ios / 2.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) + (((Eamp_Aol * Eamp_Vos) * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) + ((Eamp_Ibias + (Eamp_Ios / 2.0)) * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))))) / ((gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * ((((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))) / Rd) / Rd)) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd)))))));
    const auto zC13 = ((gC13 * ((_2N5485_Beta * ((vGSJ1 - _2N5485_vp) + (1.0 / 1000.0))) * ((vGSJ1 - _2N5485_vp) + (1.0 / 1000.0)))) / (gR13 + (1.0 / 1000000000.0)));
    const auto zR6C3 = (gR6C3 * (((((Eamp_Ibias + (Eamp_Ios / 2.0)) * ((((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))) / Rd) / Rd)) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) + (gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * ((Eamp_Ibias - (Eamp_Ios / 2.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((((Eamp_Aol * Eamp_Vos) * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd) + ((Eamp_Ibias - (Eamp_Ios / 2.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))))))) / ((gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * ((((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))) / Rd) / Rd)) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))))) - vi));
    const auto zC11 = (-((gC11 * ((gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (gRfR10 * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt))))))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (gRfR10 * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))) - ((Eamp_Aol * (gEamp_Rcomp * (gRfR10 * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))) / Rd))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * (((((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (gRfR10 * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt))))))) / Rd) / Rd) + ((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (gRfR10 * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))))) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (gRfR10 * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))) - ((Eamp_Aol * (gEamp_Rcomp * (gRfR10 * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))) / Rd))))))) / ((gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * ((((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))) / Rd) / Rd)) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))))));
    const auto zR8C7 = ((gR8C7 * (((gEamp_Rin + (1.0 / 1000000000.0)) * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * ((Eamp_Ibias - (Eamp_Ios / 2.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * (((Eamp_Ibias - (Eamp_Ios / 2.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) + (((Eamp_Aol * Eamp_Vos) * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) + ((Eamp_Ibias + (Eamp_Ios / 2.0)) * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))))) / ((gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * ((((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))) / Rd) / Rd)) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd)))))));
    const auto zRdC5 = ((gzRdC5 * (((((gEamp_Rin + (1.0 / 1000000000.0)) * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * ((Eamp_Ibias - (Eamp_Ios / 2.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * (((Eamp_Ibias - (Eamp_Ios / 2.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) + (((Eamp_Aol * Eamp_Vos) * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) + ((Eamp_Ibias + (Eamp_Ios / 2.0)) * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd)))))) / ((gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * ((((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))) / Rd) / Rd)) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))))) + (((((gEamp_Rin + (1.0 / 1000000000.0)) * ((((Eamp_Ibias - (Eamp_Ios / 2.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd) - (gEamp_Rout * (((Eamp_Ibias - (Eamp_Ios / 2.0)) * (Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * ((Eamp_Aol * Eamp_Vos) * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))))))) + ((Eamp_Ibias + (Eamp_Ios / 2.0)) * (((gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd) - (gEamp_Rout * ((gEamp_Rin * (Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) - (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))))))))) + (gEamp_Rin * (gEamp_Rout * ((gEamp_Rin * ((Eamp_Aol * Eamp_Vos) * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + ((Eamp_Ibias - (Eamp_Ios / 2.0)) * (Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))))))) / ((gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * ((((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))) / Rd) / Rd)) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))))))) / 2.0);
    const auto zR9C9 = (gR9C9 * ((((gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * ((gRfR10 + (1.0 / 1000000000.0)) * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt))))))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * ((gRfR10 + (1.0 / 1000000000.0)) * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))) - ((Eamp_Aol * (gEamp_Rcomp * ((gRfR10 + (1.0 / 1000000000.0)) * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))) / Rd))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * (((((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * ((gRfR10 + (1.0 / 1000000000.0)) * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt))))))) / Rd) / Rd) + ((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * ((gRfR10 + (1.0 / 1000000000.0)) * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))))) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * ((gRfR10 + (1.0 / 1000000000.0)) * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))) - ((Eamp_Aol * (gEamp_Rcomp * ((gRfR10 + (1.0 / 1000000000.0)) * (D1N914_Is * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))))) / Rd)))))) / ((gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * ((((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))) / Rd) / Rd)) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))))) - (((((gEamp_Rin + (1.0 / 1000000000.0)) * ((((Eamp_Ibias - (Eamp_Ios / 2.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd) - (gEamp_Rout * (((Eamp_Ibias - (Eamp_Ios / 2.0)) * (Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * ((Eamp_Aol * Eamp_Vos) * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))))))) + ((Eamp_Ibias + (Eamp_Ios / 2.0)) * (((gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd) - (gEamp_Rout * ((gEamp_Rin * (Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) - (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))))))))) + (gEamp_Rin * (gEamp_Rout * ((gEamp_Rin * ((Eamp_Aol * Eamp_Vos) * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + ((Eamp_Ibias - (Eamp_Ios / 2.0)) * (Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))))))) / ((gEamp_Rin * (((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (gEamp_Rin * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (gEamp_Rout * ((gEamp_Rin * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd))))) - ((gEamp_Rin + (1.0 / 1000000000.0)) * ((((((-1.0 / Rd) - gEamp_Rout) - (1.0 / 1000000000.0)) * (((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))))) + (((((gEamp_Rcomp + gEamp_Rout) + (1.0 / 1000000000.0)) * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))) / Rd) / Rd)) + (gEamp_Rout * ((((gEamp_Rin + (1.0 / Rd)) + (1.0 / 1000000000.0)) * (gEamp_Rout * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) - ((Eamp_Aol * (gEamp_Rcomp * (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)))) / Rd)))))))));

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
