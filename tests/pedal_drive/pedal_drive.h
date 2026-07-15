// Auto-generated with netlist_codegen version e0d236b.
// Command: netlist_codegen pedal_drive.net pedal_drive.h -type_name double

#pragma once

#include <cmath>

struct Params {
    double Cout = 9.999999e-08;
    double Rout = 1.0e+05;
    double R20 = 1.0e+05;
    double R21 = 1.5e+02;
    double Vp = 9.0e+00;
    double R18 = 1.0e+04;
    double Q2N5089_Is = 5.0e-14;
    double Q2N5089_vt = 2.585e-02;
    double Q2N5089_BetaF = 6.0e+02;
    double Q2N5089_BetaR = 5.0e+01;
    double R19 = 1.0e+04;
    double C5 = 9.999999e-08;
    double C6 = 1.0e-06;
    double R17 = 4.7e+05;
    double C12 = 4.7e-10;
    double D1N914_Is = 2.52e-09;
    double D1N914_vt = 4.52892e-02;
};

struct State {
    double zCout {};
    double zR19C5 {};
    double zC6 {};
    double zR17C12 {};
    double vBCQ1 {};
    double vBEQ1 {};
    double vD3D4 {};
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
    return v_new;
};

static constexpr auto newton_tol_sq = 1.0e-04;
static constexpr int newton_max_iter = 10;

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto Q2N5089_Is = params.Q2N5089_Is;
    const auto Q2N5089_vt = params.Q2N5089_vt;
    const auto Q2N5089_BetaF = params.Q2N5089_BetaF;
    const auto Q2N5089_BetaR = params.Q2N5089_BetaR;
    const auto D1N914_Is = params.D1N914_Is;
    const auto D1N914_vt = params.D1N914_vt;
    
    const auto gCout = 2.0 * sample_rate * params.Cout;
    
    const auto gRout = 1.0 / params.Rout;
    
    const auto gR20 = 1.0 / params.R20;
    
    const auto gR21 = 1.0 / params.R21;
    
    const auto Vp = params.Vp;
    
    const auto vp = Vp;
    
    const auto gR18 = 1.0 / params.R18;
    
    const auto R19 = params.R19;
    const auto C5 = params.C5;
    const auto gR19C5 = (2.0 * sample_rate * C5) / (1.0 + 2.0 * sample_rate * R19 * C5);
    const auto gnR19C5 = gR19C5 * (2.0 * sample_rate * R19 * C5 - 1.0) / (2.0 * sample_rate * C5);
    
    const auto gC6 = 2.0 * sample_rate * params.C6;
    
    const auto R17 = params.R17;
    const auto C12 = params.C12;
    const auto gR17C12 = 2.0 * sample_rate * C12 + (1.0 / R17);
    const auto gzR17C12 = 4.0 * sample_rate * C12;
    
    const auto vcrit_Q2N5089_vt = Q2N5089_vt * std::log(Q2N5089_vt / (std::sqrt(2.0) * Q2N5089_Is));
    const auto vcrit_D1N914_vt = D1N914_vt * std::log(D1N914_vt / (std::sqrt(2.0) * D1N914_Is));
    
    const auto _t9 = (gR21 * gC6);
    const auto _t16 = (gR17C12 * _t9);
    const auto _t17 = ((gCout + gR18) + gR17C12);
    const auto _t20 = (((gR20 + gR19C5) + gC6) + gR17C12);
    const auto _t26 = (Vp * gC6);
    const auto _t30 = (gCout + gRout);
    const auto _t31 = (_t30 * _t16);
    const auto _t19 = ((gC6 * _t9) - (_t20 * _t9));
    const auto _t22 = (gCout * _t19);
    const auto _t25 = (gR21 * _t26);
    const auto _t27 = (_t20 * _t25);
    const auto _t29 = (_t30 * _t19);
    const auto _t36 = (gR17C12 * _t25);
    const auto _t24 = (gC6 * _t25);
    const auto _t28 = (1 / (((_t17 * _t29) - (gCout * _t22)) + (gR17C12 * _t31)));
    const auto _t35 = (_t30 * _t36);
    const auto _t23 = (_t24 - _t27);
    const auto _t34 = (gR18 * _t35);
    const auto _t38 = (gR18 * (gCout * _t23));
    const auto _t39 = (gR18 * (_t30 * _t23));
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zCout = state[ch].zCout;
        auto zR19C5 = state[ch].zR19C5;
        auto zC6 = state[ch].zC6;
        auto zR17C12 = state[ch].zR17C12;
        auto vBCQ1 = state[ch].vBCQ1;
        auto vBEQ1 = state[ch].vBEQ1;
        auto vD3D4 = state[ch].vD3D4;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            // --- Newton-Raphson solve: Q1_D3D4
            const auto _Q1_D3D4_t7 = (gCout + gR18);
            const auto _Q1_D3D4_t9 = (gCout + gRout);
            const auto _Q1_D3D4_t26 = (gR19C5 * vi);
            const auto _Q1_D3D4_t27 = (gR21 * gC6);
            const auto _Q1_D3D4_t40 = (gR17C12 * _Q1_D3D4_t27);
            const auto _Q1_D3D4_t47 = (zCout - zR17C12);
            const auto _Q1_D3D4_t48 = (_Q1_D3D4_t9 * _Q1_D3D4_t40);
            const auto _Q1_D3D4_t53 = (Vp * gC6);
            const auto _Q1_D3D4_t59 = (gC6 * _Q1_D3D4_t27);
            const auto _Q1_D3D4_t63 = (gR20 + gR19C5);
            const auto _Q1_D3D4_t66 = (gR17C12 * _Q1_D3D4_t48);
            const auto _Q1_D3D4_t112 = (1 / D1N914_vt);
            const auto _Q1_D3D4_t6 = (_Q1_D3D4_t7 + gR17C12);
            const auto _Q1_D3D4_t25 = (_Q1_D3D4_t26 + zR19C5);
            const auto _Q1_D3D4_t39 = (zCout * _Q1_D3D4_t40);
            const auto _Q1_D3D4_t52 = (gR21 * _Q1_D3D4_t53);
            const auto _Q1_D3D4_t62 = (_Q1_D3D4_t63 + gC6);
            const auto _Q1_D3D4_t78 = (gC6 * _Q1_D3D4_t52);
            const auto _Q1_D3D4_t24 = (_Q1_D3D4_t25 + zC6);
            const auto _Q1_D3D4_t51 = (gR17C12 * _Q1_D3D4_t52);
            const auto _Q1_D3D4_t61 = (_Q1_D3D4_t62 + gR17C12);
            const auto _Q1_D3D4_t79 = (_Q1_D3D4_t61 * _Q1_D3D4_t52);
            const auto _Q1_D3D4_t23 = (_Q1_D3D4_t24 + zR17C12);
            const auto _Q1_D3D4_t50 = (_Q1_D3D4_t9 * _Q1_D3D4_t51);
            const auto _Q1_D3D4_t60 = (_Q1_D3D4_t61 * _Q1_D3D4_t27);
            const auto _Q1_D3D4_t77 = (_Q1_D3D4_t78 - _Q1_D3D4_t79);
            const auto _Q1_D3D4_t49 = (gR18 * _Q1_D3D4_t50);
            const auto _Q1_D3D4_t58 = (_Q1_D3D4_t59 - _Q1_D3D4_t60);
            const auto _Q1_D3D4_t65 = (gCout * _Q1_D3D4_t58);
            const auto _Q1_D3D4_t74 = (zCout * _Q1_D3D4_t58);
            const auto _Q1_D3D4_t76 = (_Q1_D3D4_t9 * _Q1_D3D4_t77);
            const auto _Q1_D3D4_t57 = (_Q1_D3D4_t9 * _Q1_D3D4_t58);
            const auto _Q1_D3D4_t64 = (gCout * _Q1_D3D4_t65);
            const auto _Q1_D3D4_t73 = (gCout * _Q1_D3D4_t74);
            const auto _Q1_D3D4_t75 = (gR18 * _Q1_D3D4_t76);
            const auto _Q1_D3D4_t56 = (_Q1_D3D4_t6 * _Q1_D3D4_t57);
            const auto _Q1_D3D4_t55 = (_Q1_D3D4_t56 - _Q1_D3D4_t64);
            const auto _Q1_D3D4_t54 = (1 / (_Q1_D3D4_t55 + _Q1_D3D4_t66));
            for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
            {
                const auto _Q1_D3D4_t18 = (vBEQ1 / Q2N5089_vt);
                const auto _Q1_D3D4_t22 = (vBCQ1 / Q2N5089_vt);
                const auto _Q1_D3D4_t34 = (vD3D4 / D1N914_vt);
                const auto _Q1_D3D4_t17 = exp(_Q1_D3D4_t18);
                const auto _Q1_D3D4_t21 = exp(_Q1_D3D4_t22);
                const auto _Q1_D3D4_t33 = exp(_Q1_D3D4_t34);
                const auto _Q1_D3D4_t35 = (1 / _Q1_D3D4_t33);
                const auto _Q1_D3D4_t45 = (_Q1_D3D4_t17 - _Q1_D3D4_t21);
                const auto _Q1_D3D4_t91 = (Q2N5089_Is * _Q1_D3D4_t17);
                const auto _Q1_D3D4_t97 = (_Q1_D3D4_t91 * _Q1_D3D4_t48);
                const auto _Q1_D3D4_t103 = (_Q1_D3D4_t91 / Q2N5089_vt);
                const auto _Q1_D3D4_t110 = (_Q1_D3D4_t33 / D1N914_vt);
                const auto _Q1_D3D4_t111 = (_Q1_D3D4_t112 / _Q1_D3D4_t33);
                const auto _Q1_D3D4_t162 = (Q2N5089_Is * _Q1_D3D4_t21);
                const auto _Q1_D3D4_t170 = (_Q1_D3D4_t21 / Q2N5089_vt);
                const auto _Q1_D3D4_t171 = (_Q1_D3D4_t170 / Q2N5089_BetaR);
                const auto _Q1_D3D4_t185 = (_Q1_D3D4_t162 / Q2N5089_vt);
                const auto _Q1_D3D4_t16 = (_Q1_D3D4_t17 - 1);
                const auto _Q1_D3D4_t20 = (_Q1_D3D4_t21 - 1);
                const auto _Q1_D3D4_t32 = (_Q1_D3D4_t33 - _Q1_D3D4_t35);
                const auto _Q1_D3D4_t90 = (_Q1_D3D4_t91 * _Q1_D3D4_t27);
                const auto _Q1_D3D4_t95 = (gCout * _Q1_D3D4_t90);
                const auto _Q1_D3D4_t96 = (_Q1_D3D4_t97 / Q2N5089_vt);
                const auto _Q1_D3D4_t102 = (_Q1_D3D4_t103 / Q2N5089_BetaF);
                const auto _Q1_D3D4_t109 = (_Q1_D3D4_t110 + _Q1_D3D4_t111);
                const auto _Q1_D3D4_t161 = (_Q1_D3D4_t162 * _Q1_D3D4_t27);
                const auto _Q1_D3D4_t166 = (gCout * _Q1_D3D4_t161);
                const auto _Q1_D3D4_t169 = (_Q1_D3D4_t170 + _Q1_D3D4_t171);
                const auto _Q1_D3D4_t184 = (_Q1_D3D4_t185 / Q2N5089_BetaR);
                const auto _Q1_D3D4_t15 = (_Q1_D3D4_t16 / Q2N5089_BetaF);
                const auto _Q1_D3D4_t19 = (_Q1_D3D4_t20 / Q2N5089_BetaR);
                const auto _Q1_D3D4_t31 = (D1N914_Is * _Q1_D3D4_t32);
                const auto _Q1_D3D4_t44 = (_Q1_D3D4_t45 - _Q1_D3D4_t19);
                const auto _Q1_D3D4_t46 = (_Q1_D3D4_t47 + _Q1_D3D4_t31);
                const auto _Q1_D3D4_t89 = (_Q1_D3D4_t9 * _Q1_D3D4_t90);
                const auto _Q1_D3D4_t94 = (gCout * _Q1_D3D4_t95);
                const auto _Q1_D3D4_t101 = (_Q1_D3D4_t102 + _Q1_D3D4_t103);
                const auto _Q1_D3D4_t108 = (D1N914_Is * _Q1_D3D4_t109);
                const auto _Q1_D3D4_t113 = (_Q1_D3D4_t108 * _Q1_D3D4_t48);
                const auto _Q1_D3D4_t131 = (gR17C12 * _Q1_D3D4_t89);
                const auto _Q1_D3D4_t160 = (_Q1_D3D4_t9 * _Q1_D3D4_t161);
                const auto _Q1_D3D4_t165 = (gCout * _Q1_D3D4_t166);
                const auto _Q1_D3D4_t168 = (Q2N5089_Is * _Q1_D3D4_t169);
                const auto _Q1_D3D4_t176 = (gR17C12 * _Q1_D3D4_t160);
                const auto _Q1_D3D4_t183 = (_Q1_D3D4_t184 - _Q1_D3D4_t168);
                const auto _Q1_D3D4_t14 = (_Q1_D3D4_t15 + _Q1_D3D4_t19);
                const auto _Q1_D3D4_t30 = (zC6 + _Q1_D3D4_t31);
                const auto _Q1_D3D4_t43 = (Q2N5089_Is * _Q1_D3D4_t44);
                const auto _Q1_D3D4_t88 = (_Q1_D3D4_t6 * _Q1_D3D4_t89);
                const auto _Q1_D3D4_t93 = (_Q1_D3D4_t94 / Q2N5089_vt);
                const auto _Q1_D3D4_t100 = (_Q1_D3D4_t101 * gC6);
                const auto _Q1_D3D4_t104 = (_Q1_D3D4_t61 * _Q1_D3D4_t100);
                const auto _Q1_D3D4_t107 = (gR21 * _Q1_D3D4_t108);
                const auto _Q1_D3D4_t118 = (gC6 * _Q1_D3D4_t107);
                const auto _Q1_D3D4_t125 = (gCout * _Q1_D3D4_t118);
                const auto _Q1_D3D4_t130 = (_Q1_D3D4_t131 / Q2N5089_vt);
                const auto _Q1_D3D4_t159 = (_Q1_D3D4_t6 * _Q1_D3D4_t160);
                const auto _Q1_D3D4_t164 = (_Q1_D3D4_t165 / Q2N5089_vt);
                const auto _Q1_D3D4_t167 = (_Q1_D3D4_t168 * _Q1_D3D4_t48);
                const auto _Q1_D3D4_t175 = (_Q1_D3D4_t176 / Q2N5089_vt);
                const auto _Q1_D3D4_t182 = (_Q1_D3D4_t183 * gC6);
                const auto _Q1_D3D4_t186 = (_Q1_D3D4_t61 * _Q1_D3D4_t182);
                const auto _Q1_D3D4_t13 = (Q2N5089_Is * _Q1_D3D4_t14);
                const auto _Q1_D3D4_t29 = (gR21 * _Q1_D3D4_t30);
                const auto _Q1_D3D4_t42 = (_Q1_D3D4_t43 - _Q1_D3D4_t46);
                const auto _Q1_D3D4_t72 = (_Q1_D3D4_t42 * _Q1_D3D4_t57);
                const auto _Q1_D3D4_t87 = (_Q1_D3D4_t88 / Q2N5089_vt);
                const auto _Q1_D3D4_t92 = (_Q1_D3D4_t93 / Q2N5089_BetaF);
                const auto _Q1_D3D4_t99 = (gC6 * _Q1_D3D4_t100);
                const auto _Q1_D3D4_t106 = (_Q1_D3D4_t61 * _Q1_D3D4_t107);
                const auto _Q1_D3D4_t117 = (_Q1_D3D4_t9 * _Q1_D3D4_t118);
                const auto _Q1_D3D4_t119 = (_Q1_D3D4_t108 * _Q1_D3D4_t57);
                const auto _Q1_D3D4_t123 = (_Q1_D3D4_t6 * _Q1_D3D4_t117);
                const auto _Q1_D3D4_t124 = (gCout * _Q1_D3D4_t125);
                const auto _Q1_D3D4_t129 = (_Q1_D3D4_t130 / Q2N5089_BetaF);
                const auto _Q1_D3D4_t133 = (_Q1_D3D4_t91 * _Q1_D3D4_t57);
                const auto _Q1_D3D4_t141 = (_Q1_D3D4_t13 + _Q1_D3D4_t43);
                const auto _Q1_D3D4_t147 = (_Q1_D3D4_t61 * _Q1_D3D4_t29);
                const auto _Q1_D3D4_t158 = (_Q1_D3D4_t159 / Q2N5089_vt);
                const auto _Q1_D3D4_t163 = (_Q1_D3D4_t164 / Q2N5089_BetaR);
                const auto _Q1_D3D4_t174 = (_Q1_D3D4_t175 / Q2N5089_BetaR);
                const auto _Q1_D3D4_t177 = (_Q1_D3D4_t168 * _Q1_D3D4_t57);
                const auto _Q1_D3D4_t181 = (gC6 * _Q1_D3D4_t182);
                const auto _Q1_D3D4_t12 = (_Q1_D3D4_t13 - _Q1_D3D4_t23);
                const auto _Q1_D3D4_t28 = (gC6 * _Q1_D3D4_t29);
                const auto _Q1_D3D4_t41 = (_Q1_D3D4_t42 * _Q1_D3D4_t48);
                const auto _Q1_D3D4_t71 = (_Q1_D3D4_t72 + _Q1_D3D4_t73);
                const auto _Q1_D3D4_t86 = (_Q1_D3D4_t87 / Q2N5089_BetaF);
                const auto _Q1_D3D4_t98 = (_Q1_D3D4_t99 - _Q1_D3D4_t104);
                const auto _Q1_D3D4_t116 = (gR17C12 * _Q1_D3D4_t117);
                const auto _Q1_D3D4_t122 = (_Q1_D3D4_t123 - _Q1_D3D4_t124);
                const auto _Q1_D3D4_t132 = (_Q1_D3D4_t133 / Q2N5089_vt);
                const auto _Q1_D3D4_t140 = (_Q1_D3D4_t141 * gC6);
                const auto _Q1_D3D4_t142 = (_Q1_D3D4_t61 * _Q1_D3D4_t140);
                const auto _Q1_D3D4_t157 = (_Q1_D3D4_t158 / Q2N5089_BetaR);
                const auto _Q1_D3D4_t173 = (_Q1_D3D4_t174 + _Q1_D3D4_t177);
                const auto _Q1_D3D4_t180 = (_Q1_D3D4_t181 - _Q1_D3D4_t186);
                const auto _Q1_D3D4_t11 = (_Q1_D3D4_t12 * _Q1_D3D4_t27);
                const auto _Q1_D3D4_t85 = (_Q1_D3D4_t86 - _Q1_D3D4_t92);
                const auto _Q1_D3D4_t115 = (_Q1_D3D4_t116 + _Q1_D3D4_t119);
                const auto _Q1_D3D4_t121 = (_Q1_D3D4_t122 - _Q1_D3D4_t113);
                const auto _Q1_D3D4_t128 = (_Q1_D3D4_t129 - _Q1_D3D4_t132);
                const auto _Q1_D3D4_t139 = (gC6 * _Q1_D3D4_t140);
                const auto _Q1_D3D4_t146 = (_Q1_D3D4_t147 + _Q1_D3D4_t11);
                const auto _Q1_D3D4_t156 = (_Q1_D3D4_t157 - _Q1_D3D4_t163);
                const auto _Q1_D3D4_t10 = (_Q1_D3D4_t11 + _Q1_D3D4_t28);
                const auto _Q1_D3D4_t38 = (gCout * _Q1_D3D4_t10);
                const auto _Q1_D3D4_t84 = (_Q1_D3D4_t85 + _Q1_D3D4_t96);
                const auto _Q1_D3D4_t114 = (_Q1_D3D4_t115 * _Q1_D3D4_t54);
                const auto _Q1_D3D4_t120 = (_Q1_D3D4_t121 * _Q1_D3D4_t54);
                const auto _Q1_D3D4_t127 = (_Q1_D3D4_t128 * _Q1_D3D4_t54);
                const auto _Q1_D3D4_t138 = (_Q1_D3D4_t139 - _Q1_D3D4_t142);
                const auto _Q1_D3D4_t148 = (_Q1_D3D4_t120 - _Q1_D3D4_t114);
                const auto _Q1_D3D4_t155 = (_Q1_D3D4_t156 - _Q1_D3D4_t167);
                const auto _Q1_D3D4_t172 = (_Q1_D3D4_t173 * _Q1_D3D4_t54);
                const auto _Q1_D3D4_t8 = (_Q1_D3D4_t9 * _Q1_D3D4_t10);
                const auto _Q1_D3D4_t37 = (_Q1_D3D4_t38 - _Q1_D3D4_t39);
                const auto _Q1_D3D4_t70 = (gR17C12 * _Q1_D3D4_t8);
                const auto _Q1_D3D4_t83 = (_Q1_D3D4_t84 * _Q1_D3D4_t54);
                const auto _Q1_D3D4_t105 = (((((((_Q1_D3D4_t6 * (_Q1_D3D4_t9 * _Q1_D3D4_t106)) - (gCout * (gCout * _Q1_D3D4_t106))) - (gR17C12 * (_Q1_D3D4_t9 * (gR17C12 * _Q1_D3D4_t107)))) - _Q1_D3D4_t113) * _Q1_D3D4_t54) - _Q1_D3D4_t114) - 1);
                const auto _Q1_D3D4_t126 = (_Q1_D3D4_t83 - _Q1_D3D4_t127);
                const auto _Q1_D3D4_t154 = (_Q1_D3D4_t155 * _Q1_D3D4_t54);
                const auto _Q1_D3D4_t179 = (_Q1_D3D4_t154 - ((((_Q1_D3D4_t6 * (_Q1_D3D4_t9 * _Q1_D3D4_t180)) - (gCout * (gCout * _Q1_D3D4_t180))) + (gR17C12 * (_Q1_D3D4_t9 * (gR17C12 * _Q1_D3D4_t182)))) * _Q1_D3D4_t54));
                const auto _Q1_D3D4_t5 = (_Q1_D3D4_t6 * _Q1_D3D4_t8);
                const auto _Q1_D3D4_t36 = (gCout * _Q1_D3D4_t37);
                const auto _Q1_D3D4_t69 = (_Q1_D3D4_t70 - _Q1_D3D4_t71);
                const auto _Q1_D3D4_t82 = ((_Q1_D3D4_t83 - ((((_Q1_D3D4_t6 * (_Q1_D3D4_t9 * _Q1_D3D4_t98)) - (gCout * (gCout * _Q1_D3D4_t98))) + (gR17C12 * (_Q1_D3D4_t9 * (gR17C12 * _Q1_D3D4_t100)))) * _Q1_D3D4_t54)) - 1);
                const auto _Q1_D3D4_t153 = (_Q1_D3D4_t154 - _Q1_D3D4_t172);
                const auto _Q1_D3D4_t178 = ((_Q1_D3D4_t179 * _Q1_D3D4_t105) - (_Q1_D3D4_t120 * _Q1_D3D4_t153));
                const auto _Q1_D3D4_t187 = ((_Q1_D3D4_t179 * _Q1_D3D4_t126) - (_Q1_D3D4_t82 * _Q1_D3D4_t153));
                const auto _Q1_D3D4_t4 = (_Q1_D3D4_t5 - _Q1_D3D4_t36);
                const auto _Q1_D3D4_t68 = (_Q1_D3D4_t69 + _Q1_D3D4_t75);
                const auto _Q1_D3D4_t81 = ((_Q1_D3D4_t82 * _Q1_D3D4_t105) - (_Q1_D3D4_t120 * _Q1_D3D4_t126));
                const auto _Q1_D3D4_t152 = (_Q1_D3D4_t153 - 1);
                const auto _Q1_D3D4_t3 = (_Q1_D3D4_t4 + _Q1_D3D4_t41);
                const auto _Q1_D3D4_t67 = (_Q1_D3D4_t68 * _Q1_D3D4_t54);
                const auto _Q1_D3D4_t145 = (((((((_Q1_D3D4_t6 * (_Q1_D3D4_t9 * _Q1_D3D4_t146)) - (gCout * ((gCout * _Q1_D3D4_t146) - _Q1_D3D4_t39))) - (gR17C12 * (_Q1_D3D4_t9 * (gR17C12 * _Q1_D3D4_t29)))) - _Q1_D3D4_t49) + _Q1_D3D4_t41) * _Q1_D3D4_t54) - _Q1_D3D4_t67);
                const auto _Q1_D3D4_t151 = (((_Q1_D3D4_t152 * _Q1_D3D4_t81) - (_Q1_D3D4_t126 * _Q1_D3D4_t178)) + (_Q1_D3D4_t148 * _Q1_D3D4_t187));
                const auto _Q1_D3D4_t2 = (_Q1_D3D4_t3 - _Q1_D3D4_t49);
                const auto _Q1_D3D4_t144 = (vD3D4 - _Q1_D3D4_t145);
                const auto _Q1_D3D4_t150 = (_Q1_D3D4_t82 * _Q1_D3D4_t144);
                const auto _Q1_D3D4_t189 = (_Q1_D3D4_t179 * _Q1_D3D4_t144);
                const auto _Q1_D3D4_t1 = (_Q1_D3D4_t2 * _Q1_D3D4_t54);
                const auto _Q1_D3D4_t137 = (_Q1_D3D4_t1 - ((((_Q1_D3D4_t6 * (_Q1_D3D4_t9 * _Q1_D3D4_t138)) - (gCout * (gCout * _Q1_D3D4_t138))) + (gR17C12 * (_Q1_D3D4_t9 * (gR17C12 * _Q1_D3D4_t140)))) * _Q1_D3D4_t54));
                const auto _Q1_D3D4_t143 = (_Q1_D3D4_t120 * _Q1_D3D4_t144);
                const auto _Q1_D3D4_t0 = (_Q1_D3D4_t1 - _Q1_D3D4_t67);
                const auto _Q1_D3D4_t80 = (vBCQ1 - _Q1_D3D4_t0);
                const auto _Q1_D3D4_t136 = (vBEQ1 - _Q1_D3D4_t137);
                const auto _Q1_D3D4_t149 = (_Q1_D3D4_t136 * _Q1_D3D4_t126);
                const auto _Q1_D3D4_t190 = (_Q1_D3D4_t136 * _Q1_D3D4_t153);
                const auto _Q1_D3D4_t135 = (_Q1_D3D4_t136 * _Q1_D3D4_t105);
                const auto _Q1_D3D4_t188 = (_Q1_D3D4_t189 - _Q1_D3D4_t190);
                const auto _Q1_D3D4_t134 = (_Q1_D3D4_t135 - _Q1_D3D4_t143);
                const auto res_vBCQ1 = (_Q1_D3D4_t0 - vBCQ1);
                const auto delta_vBCQ1 = ((((_Q1_D3D4_t80 * _Q1_D3D4_t81) - (_Q1_D3D4_t126 * _Q1_D3D4_t134)) + (_Q1_D3D4_t148 * (_Q1_D3D4_t149 - _Q1_D3D4_t150))) / _Q1_D3D4_t151);
                const auto res_vBEQ1 = (_Q1_D3D4_t137 - vBEQ1);
                const auto delta_vBEQ1 = ((((_Q1_D3D4_t152 * _Q1_D3D4_t134) - (_Q1_D3D4_t80 * _Q1_D3D4_t178)) + (_Q1_D3D4_t148 * _Q1_D3D4_t188)) / _Q1_D3D4_t151);
                const auto res_vD3D4 = (_Q1_D3D4_t145 - vD3D4);
                const auto delta_vD3D4 = ((((_Q1_D3D4_t152 * (_Q1_D3D4_t150 - _Q1_D3D4_t149)) - (_Q1_D3D4_t126 * _Q1_D3D4_t188)) + (_Q1_D3D4_t80 * _Q1_D3D4_t187)) / _Q1_D3D4_t151);
            
                auto residual_norm_sq = 0;
                residual_norm_sq += res_vBCQ1 * res_vBCQ1;
                residual_norm_sq += res_vBEQ1 * res_vBEQ1;
                residual_norm_sq += res_vD3D4 * res_vD3D4;
                auto step_norm_sq = 0;
                step_norm_sq += delta_vBCQ1 * delta_vBCQ1;
                step_norm_sq += delta_vBEQ1 * delta_vBEQ1;
                step_norm_sq += delta_vD3D4 * delta_vD3D4;
            
                vBCQ1 = limit_junction_voltage(vBCQ1 + delta_vBCQ1, vBCQ1, Q2N5089_vt, vcrit_Q2N5089_vt);
                vBEQ1 = limit_junction_voltage(vBEQ1 + delta_vBEQ1, vBEQ1, Q2N5089_vt, vcrit_Q2N5089_vt);
                vD3D4 = limit_junction_voltage(vD3D4 + delta_vD3D4, vD3D4, D1N914_vt, vcrit_D1N914_vt);
            
                if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
                    break;
                
            }

            const auto _t5 = exp((vBEQ1 / Q2N5089_vt));
            const auto _t8 = exp((vBCQ1 / Q2N5089_vt));
            const auto _t14 = exp((vD3D4 / D1N914_vt));
            const auto _t7 = (_t8 - 1);
            const auto _t13 = (D1N914_Is * (_t14 - (1 / _t14)));
            const auto _t15 = (zCout * _t16);
            const auto _t6 = (_t7 / Q2N5089_BetaR);
            const auto _t12 = (zC6 + _t13);
            const auto _t18 = (zCout * _t19);
            const auto _t21 = ((Q2N5089_Is * ((_t5 - _t8) - _t6)) - ((zCout - zR17C12) + _t13));
            const auto _t33 = (_t21 * _t31);
            const auto _t4 = ((Q2N5089_Is * (((_t5 - 1) / Q2N5089_BetaF) + _t6)) - ((((gR19C5 * vi) + zR19C5) + zC6) + zR17C12));
            const auto _t11 = (gR21 * _t12);
            const auto _t3 = (_t4 * _t9);
            const auto _t10 = (gC6 * _t11);
            const auto _t37 = ((_t20 * _t11) + _t3);
            const auto _t2 = (_t3 + _t10);
            const auto _t32 = (_t30 * _t2);
            const auto _t1 = (gCout * _t2);
            const auto _t0 = (_t1 - _t15);
            const auto vo = ((((gR17C12 * _t0) - ((_t17 * _t18) + (_t21 * _t22))) + _t38) * _t28);
            const auto vc = ((((gR17C12 * _t32) - ((_t21 * _t29) + (gCout * _t18))) + _t39) * _t28);
            const auto tCout = (gCout * (vc - vo));
            const auto vb = (((((_t17 * _t32) - (gCout * _t0)) + _t33) - _t34) * _t28);
            const auto vR19C5 = (vb - vi);
            const auto vd = ((((((_t17 * (_t30 * _t37)) - (gCout * ((gCout * _t37) - _t15))) - (gR17C12 * (_t30 * (gR17C12 * _t11)))) - _t34) + _t33) * _t28);
            const auto tC6 = (gC6 * (vb - vd));
            const auto vR17C12 = (vb - vc);
            
            zCout = 2 * tCout - zCout;
            zR19C5 = gR19C5 * (1 - gnR19C5) * vR19C5 + gnR19C5 * zR19C5; // RC series
            zC6 = 2 * tC6 - zC6;
            zR17C12 = gzR17C12 * vR17C12 - zR17C12; // RC parallel

            output[ch][n] = vo;
        }
        state[ch].zCout = zCout;
        state[ch].zR19C5 = zR19C5;
        state[ch].zC6 = zC6;
        state[ch].zR17C12 = zR17C12;
        state[ch].vBCQ1 = vBCQ1;
        state[ch].vBEQ1 = vBEQ1;
        state[ch].vD3D4 = vD3D4;
    }
}

static void reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc = 0.0f)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto Q2N5089_Is = params.Q2N5089_Is;
    const auto Q2N5089_vt = params.Q2N5089_vt;
    const auto Q2N5089_BetaF = params.Q2N5089_BetaF;
    const auto Q2N5089_BetaR = params.Q2N5089_BetaR;
    const auto D1N914_Is = params.D1N914_Is;
    const auto D1N914_vt = params.D1N914_vt;
    
    const auto gCout = 2.0 * sample_rate * params.Cout;
    
    const auto gRout = 1.0 / params.Rout;
    
    const auto gR20 = 1.0 / params.R20;
    
    const auto gR21 = 1.0 / params.R21;
    
    const auto Vp = params.Vp;
    
    const auto vp = Vp;
    
    const auto gR18 = 1.0 / params.R18;
    
    const auto R19 = params.R19;
    const auto C5 = params.C5;
    const auto gR19C5 = (2.0 * sample_rate * C5) / (1.0 + 2.0 * sample_rate * R19 * C5);
    const auto gnR19C5 = gR19C5 * (2.0 * sample_rate * R19 * C5 - 1.0) / (2.0 * sample_rate * C5);
    
    const auto gC6 = 2.0 * sample_rate * params.C6;
    
    const auto R17 = params.R17;
    const auto C12 = params.C12;
    const auto gR17C12 = 2.0 * sample_rate * C12 + (1.0 / R17);
    const auto gzR17C12 = 4.0 * sample_rate * C12;
    
    const auto vcrit_Q2N5089_vt = Q2N5089_vt * std::log(Q2N5089_vt / (std::sqrt(2.0) * Q2N5089_Is));
    const auto vcrit_D1N914_vt = D1N914_vt * std::log(D1N914_vt / (std::sqrt(2.0) * D1N914_Is));
    
    const auto vi = vi_dc;

    double vBCQ1 = 0;
    double vBEQ1 = 0;
    double vD3D4 = 0;

    // --- Newton-Raphson solve: Q1_D3D4
    const auto _Q1_D3D4_t7 = (1 / R17);
    const auto _Q1_D3D4_t8 = (gR21 * Vp);
    const auto _Q1_D3D4_t38 = (gRout * gR21);
    const auto _Q1_D3D4_t41 = (-1 / R17);
    const auto _Q1_D3D4_t50 = (gRout * _Q1_D3D4_t8);
    const auto _Q1_D3D4_t61 = (1 / D1N914_vt);
    const auto _Q1_D3D4_t67 = (gRout * Q2N5089_Is);
    const auto _Q1_D3D4_t68 = (1 / (Q2N5089_vt * Q2N5089_BetaF));
    const auto _Q1_D3D4_t89 = (1 / (R17 * R17));
    const auto _Q1_D3D4_t114 = (1 / (Q2N5089_vt * Q2N5089_BetaR));
    const auto _Q1_D3D4_t6 = (gR20 + _Q1_D3D4_t7);
    const auto _Q1_D3D4_t27 = (_Q1_D3D4_t6 * gR21);
    const auto _Q1_D3D4_t37 = (_Q1_D3D4_t38 / R17);
    const auto _Q1_D3D4_t40 = (_Q1_D3D4_t41 - gR18);
    const auto _Q1_D3D4_t49 = (gR18 * _Q1_D3D4_t50);
    const auto _Q1_D3D4_t5 = (_Q1_D3D4_t6 * _Q1_D3D4_t8);
    const auto _Q1_D3D4_t26 = (gRout * _Q1_D3D4_t27);
    const auto _Q1_D3D4_t36 = (_Q1_D3D4_t37 / R17);
    const auto _Q1_D3D4_t39 = (_Q1_D3D4_t40 * _Q1_D3D4_t26);
    const auto _Q1_D3D4_t48 = (_Q1_D3D4_t49 / R17);
    const auto _Q1_D3D4_t4 = (gRout * _Q1_D3D4_t5);
    const auto _Q1_D3D4_t35 = (1 / (_Q1_D3D4_t36 + _Q1_D3D4_t39));
    const auto _Q1_D3D4_t3 = (gR18 * _Q1_D3D4_t4);
    for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
    {
        const auto _Q1_D3D4_t16 = (vBEQ1 / Q2N5089_vt);
        const auto _Q1_D3D4_t18 = (vBCQ1 / Q2N5089_vt);
        const auto _Q1_D3D4_t24 = (vD3D4 / D1N914_vt);
        const auto _Q1_D3D4_t15 = exp(_Q1_D3D4_t16);
        const auto _Q1_D3D4_t17 = exp(_Q1_D3D4_t18);
        const auto _Q1_D3D4_t20 = (_Q1_D3D4_t17 - 1);
        const auto _Q1_D3D4_t23 = exp(_Q1_D3D4_t24);
        const auto _Q1_D3D4_t25 = (1 / _Q1_D3D4_t23);
        const auto _Q1_D3D4_t34 = (_Q1_D3D4_t15 - 1);
        const auto _Q1_D3D4_t59 = (_Q1_D3D4_t23 / D1N914_vt);
        const auto _Q1_D3D4_t60 = (_Q1_D3D4_t61 / _Q1_D3D4_t23);
        const auto _Q1_D3D4_t66 = (_Q1_D3D4_t67 * _Q1_D3D4_t15);
        const auto _Q1_D3D4_t71 = (Q2N5089_Is * _Q1_D3D4_t15);
        const auto _Q1_D3D4_t81 = (_Q1_D3D4_t71 * gR21);
        const auto _Q1_D3D4_t85 = (_Q1_D3D4_t71 * gRout);
        const auto _Q1_D3D4_t88 = (_Q1_D3D4_t71 / Q2N5089_vt);
        const auto _Q1_D3D4_t113 = (_Q1_D3D4_t67 * _Q1_D3D4_t17);
        const auto _Q1_D3D4_t118 = (_Q1_D3D4_t17 / Q2N5089_vt);
        const auto _Q1_D3D4_t119 = (_Q1_D3D4_t118 / Q2N5089_BetaR);
        const auto _Q1_D3D4_t127 = (Q2N5089_Is * _Q1_D3D4_t17);
        const auto _Q1_D3D4_t135 = (_Q1_D3D4_t127 / Q2N5089_vt);
        const auto _Q1_D3D4_t14 = (_Q1_D3D4_t15 - _Q1_D3D4_t17);
        const auto _Q1_D3D4_t19 = (_Q1_D3D4_t20 / Q2N5089_BetaR);
        const auto _Q1_D3D4_t22 = (_Q1_D3D4_t23 - _Q1_D3D4_t25);
        const auto _Q1_D3D4_t33 = (_Q1_D3D4_t34 / Q2N5089_BetaF);
        const auto _Q1_D3D4_t58 = (_Q1_D3D4_t59 + _Q1_D3D4_t60);
        const auto _Q1_D3D4_t65 = (_Q1_D3D4_t66 * gR21);
        const auto _Q1_D3D4_t70 = (_Q1_D3D4_t71 * _Q1_D3D4_t26);
        const auto _Q1_D3D4_t80 = (gRout * _Q1_D3D4_t81);
        const auto _Q1_D3D4_t84 = (_Q1_D3D4_t85 * gR21);
        const auto _Q1_D3D4_t87 = (_Q1_D3D4_t88 / Q2N5089_BetaF);
        const auto _Q1_D3D4_t112 = (_Q1_D3D4_t113 * gR21);
        const auto _Q1_D3D4_t117 = (_Q1_D3D4_t118 + _Q1_D3D4_t119);
        const auto _Q1_D3D4_t126 = (_Q1_D3D4_t127 * gR21);
        const auto _Q1_D3D4_t134 = (_Q1_D3D4_t135 / Q2N5089_BetaR);
        const auto _Q1_D3D4_t13 = (_Q1_D3D4_t14 - _Q1_D3D4_t19);
        const auto _Q1_D3D4_t21 = (D1N914_Is * _Q1_D3D4_t22);
        const auto _Q1_D3D4_t32 = (_Q1_D3D4_t33 + _Q1_D3D4_t19);
        const auto _Q1_D3D4_t57 = (D1N914_Is * _Q1_D3D4_t58);
        const auto _Q1_D3D4_t64 = (_Q1_D3D4_t65 * _Q1_D3D4_t68);
        const auto _Q1_D3D4_t69 = (_Q1_D3D4_t70 / Q2N5089_vt);
        const auto _Q1_D3D4_t79 = (_Q1_D3D4_t40 * _Q1_D3D4_t80);
        const auto _Q1_D3D4_t83 = (_Q1_D3D4_t84 / Q2N5089_vt);
        const auto _Q1_D3D4_t86 = (_Q1_D3D4_t87 + _Q1_D3D4_t88);
        const auto _Q1_D3D4_t111 = (_Q1_D3D4_t112 * _Q1_D3D4_t114);
        const auto _Q1_D3D4_t116 = (Q2N5089_Is * _Q1_D3D4_t117);
        const auto _Q1_D3D4_t125 = (gRout * _Q1_D3D4_t126);
        const auto _Q1_D3D4_t130 = (_Q1_D3D4_t116 * gRout);
        const auto _Q1_D3D4_t133 = (_Q1_D3D4_t134 - _Q1_D3D4_t116);
        const auto _Q1_D3D4_t12 = (Q2N5089_Is * _Q1_D3D4_t13);
        const auto _Q1_D3D4_t31 = (Q2N5089_Is * _Q1_D3D4_t32);
        const auto _Q1_D3D4_t56 = (_Q1_D3D4_t57 * gRout);
        const auto _Q1_D3D4_t63 = (_Q1_D3D4_t64 / R17);
        const auto _Q1_D3D4_t73 = (_Q1_D3D4_t69 + _Q1_D3D4_t63);
        const auto _Q1_D3D4_t78 = (_Q1_D3D4_t79 / Q2N5089_vt);
        const auto _Q1_D3D4_t82 = (_Q1_D3D4_t83 / R17);
        const auto _Q1_D3D4_t93 = (_Q1_D3D4_t56 * _Q1_D3D4_t6);
        const auto _Q1_D3D4_t99 = (_Q1_D3D4_t31 + _Q1_D3D4_t12);
        const auto _Q1_D3D4_t110 = (_Q1_D3D4_t111 / R17);
        const auto _Q1_D3D4_t115 = (_Q1_D3D4_t116 * _Q1_D3D4_t26);
        const auto _Q1_D3D4_t124 = (_Q1_D3D4_t40 * _Q1_D3D4_t125);
        const auto _Q1_D3D4_t129 = (_Q1_D3D4_t130 * gR21);
        const auto _Q1_D3D4_t11 = (_Q1_D3D4_t12 - _Q1_D3D4_t21);
        const auto _Q1_D3D4_t30 = (_Q1_D3D4_t31 * gR21);
        const auto _Q1_D3D4_t47 = (_Q1_D3D4_t11 * _Q1_D3D4_t38);
        const auto _Q1_D3D4_t55 = (_Q1_D3D4_t56 * gR21);
        const auto _Q1_D3D4_t72 = (_Q1_D3D4_t73 * _Q1_D3D4_t35);
        const auto _Q1_D3D4_t77 = (_Q1_D3D4_t78 / Q2N5089_BetaF);
        const auto _Q1_D3D4_t92 = (_Q1_D3D4_t93 * gR21);
        const auto _Q1_D3D4_t109 = (_Q1_D3D4_t110 - _Q1_D3D4_t115);
        const auto _Q1_D3D4_t123 = (_Q1_D3D4_t124 / Q2N5089_vt);
        const auto _Q1_D3D4_t128 = (_Q1_D3D4_t129 / R17);
        const auto _Q1_D3D4_t10 = (_Q1_D3D4_t11 * _Q1_D3D4_t26);
        const auto _Q1_D3D4_t29 = (gRout * _Q1_D3D4_t30);
        const auto _Q1_D3D4_t45 = (_Q1_D3D4_t40 * _Q1_D3D4_t29);
        const auto _Q1_D3D4_t46 = (_Q1_D3D4_t47 / R17);
        const auto _Q1_D3D4_t54 = (_Q1_D3D4_t55 / R17);
        const auto _Q1_D3D4_t62 = (((_Q1_D3D4_t63 + _Q1_D3D4_t69) * _Q1_D3D4_t35) - _Q1_D3D4_t72);
        const auto _Q1_D3D4_t76 = (_Q1_D3D4_t77 - _Q1_D3D4_t82);
        const auto _Q1_D3D4_t91 = (_Q1_D3D4_t92 * _Q1_D3D4_t35);
        const auto _Q1_D3D4_t108 = (_Q1_D3D4_t109 * _Q1_D3D4_t35);
        const auto _Q1_D3D4_t122 = (_Q1_D3D4_t123 / Q2N5089_BetaR);
        const auto _Q1_D3D4_t136 = (_Q1_D3D4_t108 + ((_Q1_D3D4_t115 - _Q1_D3D4_t110) * _Q1_D3D4_t35));
        const auto _Q1_D3D4_t28 = (_Q1_D3D4_t29 / R17);
        const auto _Q1_D3D4_t44 = (_Q1_D3D4_t45 - _Q1_D3D4_t46);
        const auto _Q1_D3D4_t53 = (_Q1_D3D4_t54 * _Q1_D3D4_t35);
        const auto _Q1_D3D4_t75 = (_Q1_D3D4_t76 * _Q1_D3D4_t35);
        const auto _Q1_D3D4_t90 = ((_Q1_D3D4_t91 - _Q1_D3D4_t91) - 1);
        const auto _Q1_D3D4_t94 = (_Q1_D3D4_t72 + _Q1_D3D4_t75);
        const auto _Q1_D3D4_t103 = (_Q1_D3D4_t91 - _Q1_D3D4_t53);
        const auto _Q1_D3D4_t121 = (_Q1_D3D4_t122 + _Q1_D3D4_t128);
        const auto _Q1_D3D4_t9 = (_Q1_D3D4_t10 + _Q1_D3D4_t28);
        const auto _Q1_D3D4_t43 = (_Q1_D3D4_t44 + _Q1_D3D4_t48);
        const auto _Q1_D3D4_t74 = ((_Q1_D3D4_t75 + ((((gRout * _Q1_D3D4_t86) * _Q1_D3D4_t89) + (_Q1_D3D4_t40 * (gRout * (_Q1_D3D4_t6 * _Q1_D3D4_t86)))) * _Q1_D3D4_t35)) + 1);
        const auto _Q1_D3D4_t120 = (_Q1_D3D4_t121 * _Q1_D3D4_t35);
        const auto _Q1_D3D4_t132 = (_Q1_D3D4_t120 + ((((gRout * _Q1_D3D4_t133) * _Q1_D3D4_t89) + (_Q1_D3D4_t40 * (gRout * (_Q1_D3D4_t6 * _Q1_D3D4_t133)))) * _Q1_D3D4_t35));
        const auto _Q1_D3D4_t137 = ((_Q1_D3D4_t132 * _Q1_D3D4_t62) + (_Q1_D3D4_t74 * _Q1_D3D4_t136));
        const auto _Q1_D3D4_t2 = (_Q1_D3D4_t3 - _Q1_D3D4_t9);
        const auto _Q1_D3D4_t42 = (_Q1_D3D4_t43 * _Q1_D3D4_t35);
        const auto _Q1_D3D4_t52 = ((_Q1_D3D4_t53 * _Q1_D3D4_t62) - (_Q1_D3D4_t74 * _Q1_D3D4_t90));
        const auto _Q1_D3D4_t98 = (_Q1_D3D4_t42 + (((((gRout * _Q1_D3D4_t99) / R17) / R17) + (_Q1_D3D4_t40 * (gRout * (_Q1_D3D4_t6 * _Q1_D3D4_t99)))) * _Q1_D3D4_t35));
        const auto _Q1_D3D4_t107 = ((_Q1_D3D4_t108 + _Q1_D3D4_t120) + 1);
        const auto _Q1_D3D4_t131 = ((_Q1_D3D4_t132 * _Q1_D3D4_t90) + (_Q1_D3D4_t53 * _Q1_D3D4_t136));
        const auto _Q1_D3D4_t1 = (_Q1_D3D4_t2 * _Q1_D3D4_t35);
        const auto _Q1_D3D4_t97 = (_Q1_D3D4_t98 + vBEQ1);
        const auto _Q1_D3D4_t102 = (_Q1_D3D4_t1 - (((_Q1_D3D4_t3 - _Q1_D3D4_t28) - _Q1_D3D4_t10) * _Q1_D3D4_t35));
        const auto _Q1_D3D4_t104 = (_Q1_D3D4_t97 * _Q1_D3D4_t62);
        const auto _Q1_D3D4_t106 = (((_Q1_D3D4_t107 * _Q1_D3D4_t52) + (_Q1_D3D4_t94 * _Q1_D3D4_t131)) + (_Q1_D3D4_t103 * _Q1_D3D4_t137));
        const auto _Q1_D3D4_t139 = (_Q1_D3D4_t97 * _Q1_D3D4_t136);
        const auto _Q1_D3D4_t0 = (_Q1_D3D4_t1 - _Q1_D3D4_t42);
        const auto _Q1_D3D4_t51 = (vBCQ1 - _Q1_D3D4_t0);
        const auto _Q1_D3D4_t96 = (_Q1_D3D4_t97 * _Q1_D3D4_t90);
        const auto _Q1_D3D4_t101 = (vD3D4 - _Q1_D3D4_t102);
        const auto _Q1_D3D4_t105 = (_Q1_D3D4_t74 * _Q1_D3D4_t101);
        const auto _Q1_D3D4_t140 = (_Q1_D3D4_t132 * _Q1_D3D4_t101);
        const auto _Q1_D3D4_t100 = (_Q1_D3D4_t53 * _Q1_D3D4_t101);
        const auto _Q1_D3D4_t138 = (_Q1_D3D4_t139 - _Q1_D3D4_t140);
        const auto _Q1_D3D4_t95 = (_Q1_D3D4_t96 + _Q1_D3D4_t100);
        const auto res_vBCQ1 = (_Q1_D3D4_t0 - vBCQ1);
        const auto delta_vBCQ1 = (-((((_Q1_D3D4_t51 * _Q1_D3D4_t52) + (_Q1_D3D4_t94 * _Q1_D3D4_t95)) + (_Q1_D3D4_t103 * (_Q1_D3D4_t104 + _Q1_D3D4_t105))) / _Q1_D3D4_t106));
        const auto res_vBEQ1 = (-_Q1_D3D4_t97);
        const auto delta_vBEQ1 = (-((((_Q1_D3D4_t51 * _Q1_D3D4_t131) - (_Q1_D3D4_t107 * _Q1_D3D4_t95)) + (_Q1_D3D4_t103 * _Q1_D3D4_t138)) / _Q1_D3D4_t106));
        const auto res_vD3D4 = (_Q1_D3D4_t102 - vD3D4);
        const auto delta_vD3D4 = (-((((_Q1_D3D4_t107 * (_Q1_D3D4_t105 + _Q1_D3D4_t104)) + (_Q1_D3D4_t94 * _Q1_D3D4_t138)) - (_Q1_D3D4_t51 * _Q1_D3D4_t137)) / _Q1_D3D4_t106));
    
        auto residual_norm_sq = 0;
        residual_norm_sq += res_vBCQ1 * res_vBCQ1;
        residual_norm_sq += res_vBEQ1 * res_vBEQ1;
        residual_norm_sq += res_vD3D4 * res_vD3D4;
        auto step_norm_sq = 0;
        step_norm_sq += delta_vBCQ1 * delta_vBCQ1;
        step_norm_sq += delta_vBEQ1 * delta_vBEQ1;
        step_norm_sq += delta_vD3D4 * delta_vD3D4;
    
        vBCQ1 = limit_junction_voltage(vBCQ1 + delta_vBCQ1, vBCQ1, Q2N5089_vt, vcrit_Q2N5089_vt);
        vBEQ1 = limit_junction_voltage(vBEQ1 + delta_vBEQ1, vBEQ1, Q2N5089_vt, vcrit_Q2N5089_vt);
        vD3D4 = limit_junction_voltage(vD3D4 + delta_vD3D4, vD3D4, D1N914_vt, vcrit_D1N914_vt);
    
        if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
            break;
        
    }
    const auto zCout = (-((gCout * ((gR18 * (gRout * ((gR20 + (1 / R17)) * (gR21 * Vp)))) - ((((Q2N5089_Is * ((exp((vBEQ1 / Q2N5089_vt)) - exp((vBCQ1 / Q2N5089_vt))) - ((exp((vBCQ1 / Q2N5089_vt)) - 1) / Q2N5089_BetaR))) - (D1N914_Is * (exp((vD3D4 / D1N914_vt)) - (1 / exp((vD3D4 / D1N914_vt)))))) * (gRout * ((gR20 + (1 / R17)) * gR21))) + ((gRout * ((Q2N5089_Is * (((exp((vBEQ1 / Q2N5089_vt)) - 1) / Q2N5089_BetaF) + ((exp((vBCQ1 / Q2N5089_vt)) - 1) / Q2N5089_BetaR))) * gR21)) / R17)))) / ((((gRout * gR21) / R17) / R17) + (((-1 / R17) - gR18) * (gRout * ((gR20 + (1 / R17)) * gR21))))));
    const auto zR19C5 = (-(gR19C5 * (((((((-1 / R17) - gR18) * (gRout * ((Q2N5089_Is * (((exp((vBEQ1 / Q2N5089_vt)) - 1) / Q2N5089_BetaF) + ((exp((vBCQ1 / Q2N5089_vt)) - 1) / Q2N5089_BetaR))) * gR21))) - ((((Q2N5089_Is * ((exp((vBEQ1 / Q2N5089_vt)) - exp((vBCQ1 / Q2N5089_vt))) - ((exp((vBCQ1 / Q2N5089_vt)) - 1) / Q2N5089_BetaR))) - (D1N914_Is * (exp((vD3D4 / D1N914_vt)) - (1 / exp((vD3D4 / D1N914_vt)))))) * (gRout * gR21)) / R17)) + ((gR18 * (gRout * (gR21 * Vp))) / R17)) / ((((gRout * gR21) / R17) / R17) + (((-1 / R17) - gR18) * (gRout * ((gR20 + (1 / R17)) * gR21))))) + vi)));
    const auto zC6 = (gC6 * (((((gR18 * (gRout * ((gR20 + (1 / R17)) * (gR21 * Vp)))) - ((gRout * ((Q2N5089_Is * (((exp((vBEQ1 / Q2N5089_vt)) - 1) / Q2N5089_BetaF) + ((exp((vBCQ1 / Q2N5089_vt)) - 1) / Q2N5089_BetaR))) * gR21)) / R17)) - (((Q2N5089_Is * ((exp((vBEQ1 / Q2N5089_vt)) - exp((vBCQ1 / Q2N5089_vt))) - ((exp((vBCQ1 / Q2N5089_vt)) - 1) / Q2N5089_BetaR))) - (D1N914_Is * (exp((vD3D4 / D1N914_vt)) - (1 / exp((vD3D4 / D1N914_vt)))))) * (gRout * ((gR20 + (1 / R17)) * gR21)))) / ((((gRout * gR21) / R17) / R17) + (((-1 / R17) - gR18) * (gRout * ((gR20 + (1 / R17)) * gR21))))) - ((((((-1 / R17) - gR18) * (gRout * ((Q2N5089_Is * (((exp((vBEQ1 / Q2N5089_vt)) - 1) / Q2N5089_BetaF) + ((exp((vBCQ1 / Q2N5089_vt)) - 1) / Q2N5089_BetaR))) * gR21))) - ((((Q2N5089_Is * ((exp((vBEQ1 / Q2N5089_vt)) - exp((vBCQ1 / Q2N5089_vt))) - ((exp((vBCQ1 / Q2N5089_vt)) - 1) / Q2N5089_BetaR))) - (D1N914_Is * (exp((vD3D4 / D1N914_vt)) - (1 / exp((vD3D4 / D1N914_vt)))))) * (gRout * gR21)) / R17)) + ((gR18 * (gRout * (gR21 * Vp))) / R17)) / ((((gRout * gR21) / R17) / R17) + (((-1 / R17) - gR18) * (gRout * ((gR20 + (1 / R17)) * gR21)))))));
    const auto zR17C12 = ((gzR17C12 * ((((gR18 * (gRout * ((gR20 + (1 / R17)) * (gR21 * Vp)))) - ((((Q2N5089_Is * ((exp((vBEQ1 / Q2N5089_vt)) - exp((vBCQ1 / Q2N5089_vt))) - ((exp((vBCQ1 / Q2N5089_vt)) - 1) / Q2N5089_BetaR))) - (D1N914_Is * (exp((vD3D4 / D1N914_vt)) - (1 / exp((vD3D4 / D1N914_vt)))))) * (gRout * ((gR20 + (1 / R17)) * gR21))) + ((gRout * ((Q2N5089_Is * (((exp((vBEQ1 / Q2N5089_vt)) - 1) / Q2N5089_BetaF) + ((exp((vBCQ1 / Q2N5089_vt)) - 1) / Q2N5089_BetaR))) * gR21)) / R17))) / ((((gRout * gR21) / R17) / R17) + (((-1 / R17) - gR18) * (gRout * ((gR20 + (1 / R17)) * gR21))))) - ((((((-1 / R17) - gR18) * (gRout * ((Q2N5089_Is * (((exp((vBEQ1 / Q2N5089_vt)) - 1) / Q2N5089_BetaF) + ((exp((vBCQ1 / Q2N5089_vt)) - 1) / Q2N5089_BetaR))) * gR21))) - ((((Q2N5089_Is * ((exp((vBEQ1 / Q2N5089_vt)) - exp((vBCQ1 / Q2N5089_vt))) - ((exp((vBCQ1 / Q2N5089_vt)) - 1) / Q2N5089_BetaR))) - (D1N914_Is * (exp((vD3D4 / D1N914_vt)) - (1 / exp((vD3D4 / D1N914_vt)))))) * (gRout * gR21)) / R17)) + ((gR18 * (gRout * (gR21 * Vp))) / R17)) / ((((gRout * gR21) / R17) / R17) + (((-1 / R17) - gR18) * (gRout * ((gR20 + (1 / R17)) * gR21))))))) / 2);

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].vBCQ1 = vBCQ1;
        state[ch].vBEQ1 = vBEQ1;
        state[ch].vD3D4 = vD3D4;
        state[ch].zCout = zCout;
        state[ch].zR19C5 = zR19C5;
        state[ch].zC6 = zC6;
        state[ch].zR17C12 = zR17C12;
    }
}
