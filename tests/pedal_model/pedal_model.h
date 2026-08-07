// Auto-generated with netlist_codegen version ea0eac5.
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
    
    const auto _t2 = (gC13 + gR13);
    const auto _t4 = (gC4 + gR6C3);
    const auto _t7 = (gR9C9 + gRfR10);
    const auto _t8 = (gR11 + gC12);
    const auto _t13 = ((gC12 + gC11) + gRfR10);
    const auto _t24 = (gRL + gC13);
    const auto _t41 = (gRfR10 * gR9C9);
    const auto _t44 = ((gR7C6 + gR8C7) + gRdC5);
    const auto _t48 = (_t4 * (gRdC5 * (gC12 * gRfR10)));
    const auto _t50 = (_t13 * gR9C9);
    const auto _t52 = (gC12 * gR9C9);
    const auto _t6 = (gC12 * _t7);
    const auto _t12 = ((_t13 * _t7) - (gRfR10 * gRfR10));
    const auto _t43 = (_t44 * _t41);
    const auto _t5 = (gRdC5 * _t6);
    const auto _t11 = (gRdC5 * _t12);
    const auto _t3 = (_t4 * _t5);
    const auto _t10 = (_t4 * _t11);
    const auto _t22 = (gC13 * _t3);
    const auto _t23 = (gC13 * _t10);
    const auto _t1 = (_t2 * _t3);
    const auto _t9 = (_t2 * _t10);
    const auto _t21 = ((gC12 * _t22) - (_t8 * _t23));
    const auto _t0 = ((gC12 * _t1) - (_t8 * _t9));
    const auto _t20 = (1.0 / ((gC13 * _t21) - (_t24 * _t0)));
    const auto _t53 = (gC13 * _t48);
    const auto _t54 = (_t2 * _t48);
    const auto _t55 = (1.0 / 1000.0);
    const auto _t56 = (_t44 * _t12);
    const auto _t57 = (_t44 * _t6);
    const auto _t58 = (_t44 * _t50);
    const auto _t59 = (_t44 * _t52);
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zC4 = state[ch].zC4;
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
            const auto _J1_D1D2_t3 = (gC4 + gR6C3);
            const auto _J1_D1D2_t8 = (gC12 + gC11);
            const auto _J1_D1D2_t9 = (gR9C9 + gRfR10);
            const auto _J1_D1D2_t10 = (gRfR10 * gRfR10);
            const auto _J1_D1D2_t15 = (gR6C3 * vi);
            const auto _J1_D1D2_t18 = (gR7C6 + gR8C7);
            const auto _J1_D1D2_t19 = (gRfR10 * gR9C9);
            const auto _J1_D1D2_t25 = (zC12 + zC11);
            const auto _J1_D1D2_t35 = (zR7C6 + zR8C7);
            const auto _J1_D1D2_t36 = (gRL + gC13);
            const auto _J1_D1D2_t38 = (gC13 + gR13);
            const auto _J1_D1D2_t46 = (gC12 * _J1_D1D2_t9);
            const auto _J1_D1D2_t48 = (gR11 + gC12);
            const auto _J1_D1D2_t59 = (1.0 / 1000.0);
            const auto _J1_D1D2_t68 = (1.0 / D1N914_vt);
            const auto _J1_D1D2_t79 = (gC12 * gRfR10);
            const auto _J1_D1D2_t85 = (_J1_D1D2_t25 * gRfR10);
            const auto _J1_D1D2_t97 = (gC12 * gR9C9);
            const auto _J1_D1D2_t7 = (_J1_D1D2_t8 + gRfR10);
            const auto _J1_D1D2_t14 = (_J1_D1D2_t15 + zR6C3);
            const auto _J1_D1D2_t17 = (_J1_D1D2_t18 + gRdC5);
            const auto _J1_D1D2_t24 = (_J1_D1D2_t25 * _J1_D1D2_t9);
            const auto _J1_D1D2_t34 = (_J1_D1D2_t35 + zRdC5);
            const auto _J1_D1D2_t45 = (gRdC5 * _J1_D1D2_t46);
            const auto _J1_D1D2_t78 = (gRdC5 * _J1_D1D2_t79);
            const auto _J1_D1D2_t88 = (_J1_D1D2_t7 * gR9C9);
            const auto _J1_D1D2_t90 = (_J1_D1D2_t17 * _J1_D1D2_t88);
            const auto _J1_D1D2_t96 = (_J1_D1D2_t34 * _J1_D1D2_t97);
            const auto _J1_D1D2_t99 = (_J1_D1D2_t17 * _J1_D1D2_t97);
            const auto _J1_D1D2_t6 = (_J1_D1D2_t7 * _J1_D1D2_t9);
            const auto _J1_D1D2_t13 = (zC4 + _J1_D1D2_t14);
            const auto _J1_D1D2_t16 = (_J1_D1D2_t17 * _J1_D1D2_t19);
            const auto _J1_D1D2_t33 = (_J1_D1D2_t34 * _J1_D1D2_t19);
            const auto _J1_D1D2_t44 = (_J1_D1D2_t3 * _J1_D1D2_t45);
            const auto _J1_D1D2_t52 = (_J1_D1D2_t38 * _J1_D1D2_t44);
            const auto _J1_D1D2_t77 = (_J1_D1D2_t3 * _J1_D1D2_t78);
            const auto _J1_D1D2_t87 = (_J1_D1D2_t34 * _J1_D1D2_t88);
            const auto _J1_D1D2_t89 = (_J1_D1D2_t13 * _J1_D1D2_t90);
            const auto _J1_D1D2_t98 = (_J1_D1D2_t13 * _J1_D1D2_t99);
            const auto _J1_D1D2_t5 = (_J1_D1D2_t6 - _J1_D1D2_t10);
            const auto _J1_D1D2_t12 = (_J1_D1D2_t13 * _J1_D1D2_t16);
            const auto _J1_D1D2_t43 = (gC13 * _J1_D1D2_t44);
            const auto _J1_D1D2_t51 = (gC12 * _J1_D1D2_t52);
            const auto _J1_D1D2_t4 = (gRdC5 * _J1_D1D2_t5);
            const auto _J1_D1D2_t42 = (gC12 * _J1_D1D2_t43);
            const auto _J1_D1D2_t2 = (_J1_D1D2_t3 * _J1_D1D2_t4);
            const auto _J1_D1D2_t37 = (_J1_D1D2_t38 * _J1_D1D2_t2);
            const auto _J1_D1D2_t53 = (_J1_D1D2_t48 * _J1_D1D2_t37);
            const auto _J1_D1D2_t1 = (gC13 * _J1_D1D2_t2);
            const auto _J1_D1D2_t47 = (_J1_D1D2_t48 * _J1_D1D2_t1);
            const auto _J1_D1D2_t50 = (_J1_D1D2_t51 - _J1_D1D2_t53);
            const auto _J1_D1D2_t41 = (_J1_D1D2_t42 - _J1_D1D2_t47);
            const auto _J1_D1D2_t49 = (_J1_D1D2_t36 * _J1_D1D2_t50);
            const auto _J1_D1D2_t40 = (gC13 * _J1_D1D2_t41);
            const auto _J1_D1D2_t39 = (1.0 / (_J1_D1D2_t40 - _J1_D1D2_t49));
            const auto _J1_D1D2_t104 = (zC12 * (gC13 * _J1_D1D2_t77));
            const auto _J1_D1D2_t105 = (zC12 * (_J1_D1D2_t38 * _J1_D1D2_t77));
            const auto _J1_D1D2_t106 = (zC12 * _J1_D1D2_t1);
            const auto _J1_D1D2_t107 = (zC12 * _J1_D1D2_t37);
            const auto _J1_D1D2_t108 = (zC13 * _J1_D1D2_t41);
            for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
            {
                const auto _J1_D1D2_t31 = (vD1D2 / D1N914_vt);
                const auto _J1_D1D2_t58 = (vGSJ1 - _2N5485_vp);
                const auto _J1_D1D2_t30 = exp(_J1_D1D2_t31);
                const auto _J1_D1D2_t32 = (1.0 / _J1_D1D2_t30);
                const auto _J1_D1D2_t57 = (_J1_D1D2_t58 + _J1_D1D2_t59);
                const auto _J1_D1D2_t66 = (_J1_D1D2_t30 / D1N914_vt);
                const auto _J1_D1D2_t67 = (_J1_D1D2_t68 / _J1_D1D2_t30);
                const auto _J1_D1D2_t103 = (_J1_D1D2_t57 + _J1_D1D2_t57);
                const auto _J1_D1D2_t29 = (_J1_D1D2_t30 - _J1_D1D2_t32);
                const auto _J1_D1D2_t56 = (_J1_D1D2_t57 * _J1_D1D2_t57);
                const auto _J1_D1D2_t65 = (_J1_D1D2_t66 + _J1_D1D2_t67);
                const auto _J1_D1D2_t102 = (_2N5485_Beta * _J1_D1D2_t103);
                const auto _J1_D1D2_t28 = (D1N914_Is * _J1_D1D2_t29);
                const auto _J1_D1D2_t55 = (_2N5485_Beta * _J1_D1D2_t56);
                const auto _J1_D1D2_t64 = (D1N914_Is * _J1_D1D2_t65);
                const auto _J1_D1D2_t71 = (_J1_D1D2_t7 * _J1_D1D2_t64);
                const auto _J1_D1D2_t74 = (gRfR10 * _J1_D1D2_t64);
                const auto _J1_D1D2_t27 = (zR9C9 + _J1_D1D2_t28);
                const auto _J1_D1D2_t54 = (zC13 + _J1_D1D2_t55);
                const auto _J1_D1D2_t63 = (gC12 * _J1_D1D2_t64);
                const auto _J1_D1D2_t70 = (gRdC5 * _J1_D1D2_t71);
                const auto _J1_D1D2_t73 = (gRdC5 * _J1_D1D2_t74);
                const auto _J1_D1D2_t86 = (_J1_D1D2_t7 * _J1_D1D2_t27);
                const auto _J1_D1D2_t95 = (gC12 * _J1_D1D2_t27);
                const auto _J1_D1D2_t26 = (gRfR10 * _J1_D1D2_t27);
                const auto _J1_D1D2_t62 = (gRdC5 * _J1_D1D2_t63);
                const auto _J1_D1D2_t69 = (_J1_D1D2_t3 * _J1_D1D2_t70);
                const auto _J1_D1D2_t72 = (_J1_D1D2_t3 * _J1_D1D2_t73);
                const auto _J1_D1D2_t84 = (_J1_D1D2_t85 - _J1_D1D2_t86);
                const auto _J1_D1D2_t94 = (gRdC5 * _J1_D1D2_t95);
                const auto _J1_D1D2_t23 = (_J1_D1D2_t24 - _J1_D1D2_t26);
                const auto _J1_D1D2_t61 = (_J1_D1D2_t3 * _J1_D1D2_t62);
                const auto _J1_D1D2_t83 = (gRdC5 * _J1_D1D2_t84);
                const auto _J1_D1D2_t93 = (_J1_D1D2_t94 + _J1_D1D2_t96);
                const auto _J1_D1D2_t22 = (gRdC5 * _J1_D1D2_t23);
                const auto _J1_D1D2_t82 = (_J1_D1D2_t83 - _J1_D1D2_t87);
                const auto _J1_D1D2_t92 = (_J1_D1D2_t3 * _J1_D1D2_t93);
                const auto _J1_D1D2_t21 = (_J1_D1D2_t22 - _J1_D1D2_t33);
                const auto _J1_D1D2_t81 = (_J1_D1D2_t3 * _J1_D1D2_t82);
                const auto _J1_D1D2_t91 = (_J1_D1D2_t92 - _J1_D1D2_t98);
                const auto _J1_D1D2_t20 = (_J1_D1D2_t3 * _J1_D1D2_t21);
                const auto _J1_D1D2_t60 = ((((_J1_D1D2_t36 * ((gC12 * (_J1_D1D2_t38 * _J1_D1D2_t61)) - (_J1_D1D2_t48 * (_J1_D1D2_t38 * _J1_D1D2_t69)))) - (gC13 * ((gC12 * (gC13 * _J1_D1D2_t61)) - (_J1_D1D2_t48 * (gC13 * _J1_D1D2_t69))))) * _J1_D1D2_t39) - 1.0);
                const auto _J1_D1D2_t80 = (_J1_D1D2_t81 + _J1_D1D2_t89);
                const auto _J1_D1D2_t101 = (((_J1_D1D2_t36 * ((gC12 * (_J1_D1D2_t102 * _J1_D1D2_t44)) - (_J1_D1D2_t48 * (_J1_D1D2_t102 * _J1_D1D2_t2)))) * _J1_D1D2_t39) - 1.0);
                const auto _J1_D1D2_t11 = (_J1_D1D2_t12 + _J1_D1D2_t20);
                const auto _J1_D1D2_t76 = (((gC13 * (_J1_D1D2_t104 - ((_J1_D1D2_t48 * (gC13 * _J1_D1D2_t80)) + (gC12 * (gC13 * _J1_D1D2_t91))))) - (_J1_D1D2_t36 * (_J1_D1D2_t105 - ((_J1_D1D2_t48 * (_J1_D1D2_t38 * _J1_D1D2_t80)) + (gC12 * (_J1_D1D2_t38 * _J1_D1D2_t91)))))) * _J1_D1D2_t39);
                const auto _J1_D1D2_t100 = (_J1_D1D2_t101 * _J1_D1D2_t60);
                const auto _J1_D1D2_t0 = ((((gC13 * (_J1_D1D2_t106 - (gC12 * (gC13 * _J1_D1D2_t11)))) - (_J1_D1D2_t36 * (_J1_D1D2_t107 - (gC12 * (_J1_D1D2_t38 * _J1_D1D2_t11))))) * _J1_D1D2_t39) - ((_J1_D1D2_t108 - (_J1_D1D2_t36 * ((gC12 * (_J1_D1D2_t54 * _J1_D1D2_t44)) - (_J1_D1D2_t48 * (_J1_D1D2_t54 * _J1_D1D2_t2))))) * _J1_D1D2_t39));
                const auto _J1_D1D2_t75 = (vD1D2 - _J1_D1D2_t76);
                const auto res_vGSJ1 = (_J1_D1D2_t0 - vGSJ1);
                const auto delta_vGSJ1 = ((((vGSJ1 - _J1_D1D2_t0) * _J1_D1D2_t60) - ((((gC13 * (gC12 * (gC13 * _J1_D1D2_t72))) - (_J1_D1D2_t36 * (gC12 * (_J1_D1D2_t38 * _J1_D1D2_t72)))) * _J1_D1D2_t39) * _J1_D1D2_t75)) / _J1_D1D2_t100);
                const auto res_vD1D2 = (_J1_D1D2_t76 - vD1D2);
                const auto delta_vD1D2 = ((_J1_D1D2_t101 * _J1_D1D2_t75) / _J1_D1D2_t100);
            
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

            const auto _t17 = ((vGSJ1 - _2N5485_vp) + _t55);
            const auto _t26 = (zC4 + ((gR6C3 * vi) + zR6C3));
            const auto _t35 = (zC12 + zC11);
            const auto _t38 = exp((vD1D2 / D1N914_vt));
            const auto _t40 = ((zR7C6 + zR8C7) + zRdC5);
            const auto _t16 = (zC13 + (_2N5485_Beta * (_t17 * _t17)));
            const auto _t34 = (_t35 * _t7);
            const auto _t37 = (zR9C9 + (D1N914_Is * (_t38 - (1.0 / _t38))));
            const auto _t39 = (_t40 * _t41);
            const auto _t46 = ((_t4 * (_t40 * _t12)) - (_t26 * _t56));
            const auto _t47 = ((_t4 * (_t40 * _t6)) - (_t26 * _t57));
            const auto _t49 = ((_t4 * ((gRdC5 * ((_t35 * gRfR10) - (_t13 * _t37))) - (_t40 * _t50))) + (_t26 * _t58));
            const auto _t51 = ((_t4 * ((gRdC5 * (gC12 * _t37)) + (_t40 * _t52))) - (_t26 * _t59));
            const auto _t25 = (_t26 * _t5);
            const auto _t27 = (_t26 * _t11);
            const auto _t36 = (gRfR10 * _t37);
            const auto _t42 = (_t26 * _t43);
            const auto _t15 = (_t16 * _t10);
            const auto _t19 = (_t16 * _t3);
            const auto _t33 = (_t34 - _t36);
            const auto _t14 = (_t8 * _t15);
            const auto _t18 = (gC12 * _t19);
            const auto _t32 = (gRdC5 * _t33);
            const auto _t31 = (_t32 - _t39);
            const auto _t30 = (_t4 * _t31);
            const auto _t45 = (_t42 + _t30);
            const auto _t29 = (_t30 + _t42);
            const auto vo = (((zC13 * _t0) + (gC13 * (_t14 - _t18))) * _t20);
            const auto vneg = (((gC13 * ((gC12 * (gC13 * _t25)) - (_t8 * (gC13 * _t27)))) - (_t24 * ((gC12 * (_t2 * _t25)) - (_t8 * (_t2 * _t27))))) * _t20);
            const auto vf = (((gC13 * ((zC12 * _t22) - (_t8 * (gC13 * _t29)))) - (_t24 * ((zC12 * _t1) - (_t8 * (_t2 * _t29))))) * _t20);
            const auto vg = (((gC13 * ((zC12 * _t23) - (gC12 * (gC13 * _t45)))) - (_t24 * ((zC12 * _t9) - (gC12 * (_t2 * _t45))))) * _t20);
            const auto tC12 = (gC12 * (vf - vg));
            const auto vs = (((zC13 * _t21) - (_t24 * (_t18 - _t14))) * _t20);
            const auto tC13 = (gC13 * (vs - vo));
            const auto vR6C3 = (vneg - vi);
            const auto tC11 = (gC11 * (vf - 0));
            const auto v2 = (((gC13 * ((_t8 * (gC13 * _t46)) - (gC12 * (gC13 * _t47)))) - (_t24 * ((_t8 * (_t2 * _t46)) - (gC12 * (_t2 * _t47))))) * _t20);
            const auto vRdC5 = (vneg - v2);
            const auto vdp = (((gC13 * ((zC12 * _t53) - ((_t8 * (gC13 * _t49)) + (gC12 * (gC13 * _t51))))) - (_t24 * ((zC12 * _t54) - ((_t8 * (_t2 * _t49)) + (gC12 * (_t2 * _t51)))))) * _t20);
            const auto vR9C9 = (v2 - vdp);
            const auto _t28 = (vneg - 0);
            const auto tC4 = (gC4 * _t28);
            const auto vR7C6 = _t28;
            const auto vR8C7 = _t28;
            
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
        state[ch].vD1D2 = vD1D2;
    }
}

static void reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc = 0.0f)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto _2N5485_Beta = params._2N5485_Beta;
    const auto _2N5485_vp = params._2N5485_vp;
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
    for (int newton_iter = 0; newton_iter < 10000; ++newton_iter)
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
    const auto _D1D2_t3 = (1.0 / 1000000000.0);
    const auto _D1D2_t2 = (gRfR10 + _D1D2_t3);
    const auto _D1D2_t6 = (1.0 / ((_D1D2_t2 * _D1D2_t2) - (gRfR10 * gRfR10)));
    const auto _D1D2_t7 = (_D1D2_t2 * D1N914_Is);
    const auto _D1D2_t8 = (1.0 / D1N914_vt);
    for (int newton_iter = 0; newton_iter < 10000; ++newton_iter)
    {
        const auto _D1D2_t5 = (vD1D2 / D1N914_vt);
        const auto _D1D2_t4 = exp(_D1D2_t5);
        const auto _D1D2_t1 = ((_D1D2_t2 * (D1N914_Is * (_D1D2_t4 - (1.0 / _D1D2_t4)))) * _D1D2_t6);
        const auto _D1D2_t0 = (_D1D2_t1 + vD1D2);
        const auto res_vD1D2 = (-_D1D2_t0);
        const auto delta_vD1D2 = (-(_D1D2_t0 / (((_D1D2_t7 * ((_D1D2_t4 / D1N914_vt) + (_D1D2_t8 / _D1D2_t4))) * _D1D2_t6) + 1.0)));
    
        auto residual_norm_sq = 0.0;
        residual_norm_sq += res_vD1D2 * res_vD1D2;
        auto step_norm_sq = 0.0;
        step_norm_sq += delta_vD1D2 * delta_vD1D2;
    
        vD1D2 = limit_junction_voltage(vD1D2 + (delta_vD1D2), vD1D2, D1N914_vt, vcrit_D1N914_vt);
    
        if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
            break;
        
    }
    const auto zC4 = 0.0;
    const auto zC12 = (-((gC12 * ((gRfR10 * D1N914_Is) * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))) / (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))));
    const auto zR7C6 = 0.0;
    const auto zC13 = ((gC13 * ((_2N5485_Beta * ((vGSJ1 - _2N5485_vp) + (1.0 / 1000.0))) * ((vGSJ1 - _2N5485_vp) + (1.0 / 1000.0)))) / (gR13 + (1.0 / 1000000000.0)));
    const auto zR6C3 = (-(gR6C3 * vi));
    const auto zC11 = (-((gC11 * ((gRfR10 * D1N914_Is) * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))) / (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))));
    const auto zR8C7 = 0.0;
    const auto zRdC5 = 0.0;
    const auto zR9C9 = ((gR9C9 * (((gRfR10 + (1.0 / 1000000000.0)) * D1N914_Is) * (exp((vD1D2 / D1N914_vt)) - (1.0 / exp((vD1D2 / D1N914_vt)))))) / (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)));

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].vGSJ1 = vGSJ1;
        state[ch].vD1D2 = vD1D2;
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
}
