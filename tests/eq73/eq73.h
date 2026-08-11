// Auto-generated with netlist_codegen version ce593e9.
// Command: netlist_codegen eq73.net eq73.h -type_name float

#pragma once

struct Params_shelf {
    float C47 = 4.7e-10f;
    float R20 = 1.0e+04f;
    float R1p = 5.0e+03f;
    float R13 = 6.2e+02f;
    float C25 = 1.0e-08f;
    float R18 = 6.2e+02f;
    float C46 = 1.5e-08f;
    float Rbias1 = 1.0e+08f;
    float R14 = 6.2e+03f;
    float R2p = 5.0e+03f;
    float R2m = 5.0e+03f;
    float R17 = 6.2e+03f;
    float C35 = 9.999999e-08f;
    float C36 = 9.999999e-08f;
    float C45 = 1.0e-08f;
    float Rbias2 = 1.0e+08f;
    float R19 = 1.2e+04f;
    float R1m = 5.0e+03f;
    float C26 = 1.0e-08f;
};

struct State_shelf {
    float zC47 {};
    float zR13C25 {};
    float zC46 {};
    float zC35 {};
    float zC36 {};
    float zC45 {};
    float zR1mC26 {};
};

static void compute_shelf (const float* const* input, float** output, int num_channels, int num_samples, Params_shelf params, State_shelf* state, float sample_rate)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto gC47 = 2.0f * sample_rate * params.C47;
    
    const auto gR20 = 1.0f / params.R20;
    
    const auto gR1p = 1.0f / params.R1p;
    
    const auto R13 = params.R13;
    const auto C25 = params.C25;
    const auto gR13C25 = (2.0f * sample_rate * C25) / (1.0f + 2.0f * sample_rate * R13 * C25);
    const auto gnR13C25 = gR13C25 * (2.0f * sample_rate * R13 * C25 - 1.0f) / (2.0f * sample_rate * C25);
    
    const auto gR18 = 1.0f / params.R18;
    
    const auto gC46 = 2.0f * sample_rate * params.C46;
    
    const auto gRbias1 = 1.0f / params.Rbias1;
    
    const auto gR14 = 1.0f / params.R14;
    
    const auto gR2p = 1.0f / params.R2p;
    
    const auto gR2m = 1.0f / params.R2m;
    
    const auto gR17 = 1.0f / params.R17;
    
    const auto gC35 = 2.0f * sample_rate * params.C35;
    
    const auto gC36 = 2.0f * sample_rate * params.C36;
    
    const auto gC45 = 2.0f * sample_rate * params.C45;
    
    const auto gRbias2 = 1.0f / params.Rbias2;
    
    const auto gR19 = 1.0f / params.R19;
    
    const auto R1m = params.R1m;
    const auto C26 = params.C26;
    const auto gR1mC26 = (2.0f * sample_rate * C26) / (1.0f + 2.0f * sample_rate * R1m * C26);
    const auto gnR1mC26 = gR1mC26 * (2.0f * sample_rate * R1m * C26 - 1.0f) / (2.0f * sample_rate * C26);
    
    const auto _t2 = ((gR1p + gR13C25) * 2048.0f);
    const auto _t4 = (gR1mC26 * 512.0f);
    const auto _t7 = (((gR14 + gR2p) + gC35) * 128.0f);
    const auto _t9 = (gR2m * 1024.0f);
    const auto _t11 = (gR2m * 128.0f);
    const auto _t12 = ((((gC35 + gC36) + gC45) + gRbias2) * 128.0f);
    const auto _t13 = (gC36 * 128.0f);
    const auto _t14 = (gC45 * 128.0f);
    const auto _t15 = ((((gR2p + gR2m) + gC45) + gR19) * 1024.0f);
    const auto _t17 = (((gR2m + gR17) + gC36) * 128.0f);
    const auto _t18 = (gC45 * 1024.0f);
    const auto _t19 = ((_t11 * _t13) + (_t17 * _t14));
    const auto _t20 = (gR2p * 128.0f);
    const auto _t22 = (gR2p * 1024.0f);
    const auto _t24 = (gC35 * 128.0f);
    const auto _t25 = (_t17 * _t24);
    const auto _t27 = (_t11 * _t24);
    const auto _t28 = ((((gR1p + gC46) + gRbias1) + gR1mC26) * 512.0f);
    const auto _t30 = ((gR18 + gR1mC26) * 512.0f);
    const auto _t31 = (gR1p * 2048.0f);
    const auto _t33 = (gR1p * 512.0f);
    const auto _t34 = (gC46 * 512.0f);
    const auto _t44 = (gR19 * 512.0f);
    const auto _t73 = (gC47 * 512.0f);
    const auto _t77 = (gR18 * 512.0f);
    const auto _t84 = (gR17 * 128.0f);
    const auto _t85 = (_t84 * _t13);
    const auto _t87 = (_t84 * _t24);
    const auto _t105 = (_t15 * _t85);
    const auto _t107 = (_t84 * _t14);
    const auto _t10 = ((_t11 * _t12) + (_t13 * _t14));
    const auto _t16 = ((_t17 * _t12) - (_t13 * _t13));
    const auto _t23 = (_t13 * _t24);
    const auto _t26 = (((_t22 * _t19) + (_t15 * _t25)) - (_t9 * _t27));
    const auto _t83 = (_t84 * _t12);
    const auto _t86 = ((_t9 * _t87) - (_t22 * _t85));
    const auto _t106 = (_t9 * _t107);
    const auto _t125 = ((_t22 * _t83) + (_t18 * _t87));
    const auto _t130 = (((_t7 * ((_t18 * _t107) - (_t15 * _t83))) + (_t20 * _t125)) + (_t24 * ((_t22 * _t107) + (_t15 * _t87))));
    const auto _t8 = (((_t9 * _t10) - (_t15 * _t16)) + (_t18 * _t19));
    const auto _t21 = (((_t22 * _t16) + (_t9 * _t23)) + (_t18 * _t25));
    const auto _t82 = ((_t9 * _t83) + (_t18 * _t85));
    const auto _t104 = (_t105 + _t106);
    const auto _t121 = ((_t7 * _t104) + (_t20 * _t86));
    const auto _t129 = (_t30 * _t130);
    const auto _t6 = (((_t7 * _t8) + (_t20 * _t21)) + (_t24 * _t26));
    const auto _t29 = (_t30 * _t6);
    const auto _t32 = (_t33 * _t29);
    const auto _t76 = (_t77 * _t6);
    const auto _t81 = ((_t7 * _t82) - (_t24 * _t86));
    const auto _t88 = (_t4 * _t81);
    const auto _t103 = ((_t20 * _t82) + (_t24 * _t104));
    const auto _t120 = (_t30 * _t121);
    const auto _t122 = (_t33 * _t120);
    const auto _t128 = (_t33 * _t129);
    const auto _t133 = (_t33 * _t76);
    const auto _t5 = (_t4 * _t6);
    const auto _t75 = (_t4 * _t76);
    const auto _t80 = (_t30 * _t81);
    const auto _t89 = (_t33 * _t80);
    const auto _t102 = (_t30 * _t103);
    const auto _t108 = (_t33 * _t102);
    const auto _t3 = ((_t4 * _t5) - (_t28 * _t29));
    const auto _t74 = (_t2 * _t75);
    const auto _t79 = ((_t28 * _t80) - (_t4 * _t88));
    const auto _t1 = ((_t2 * _t3) + (_t31 * _t32));
    const auto _t78 = ((_t2 * _t79) - (_t31 * _t89));
    const auto _t72 = (1.0f / (((_t73 * _t1) - (_t34 * _t74)) + (_t44 * _t78)));
    const auto _t134 = (_t31 * _t75);
    const auto _t135 = ((_t2 * ((_t4 * (_t4 * _t103)) - (_t28 * _t102))) + (_t31 * _t108));
    const auto _t136 = ((_t2 * ((_t28 * _t120) - (_t4 * (_t4 * _t121)))) - (_t31 * _t122));
    const auto _t137 = ((_t2 * ((_t28 * _t129) - (_t4 * (_t4 * _t130)))) - (_t31 * _t128));
    const auto _t138 = ((_t31 * _t133) - (_t2 * (_t28 * _t76)));
    const auto _t139 = (_t33 * _t5);
    const auto _t140 = (_t33 * _t88);
    const auto _t141 = (((_t22 * _t10) + (_t15 * _t23)) + (_t18 * _t27));
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zC47 = state[ch].zC47;
        auto zR13C25 = state[ch].zR13C25;
        auto zC46 = state[ch].zC46;
        auto zC35 = state[ch].zC35;
        auto zC36 = state[ch].zC36;
        auto zC45 = state[ch].zC45;
        auto zR1mC26 = state[ch].zR1mC26;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vis = input[ch][n];

            const auto _t0 = ((zC46 - zC47) * 512.0f);
            const auto _t36 = ((zR13C25 - (gR13C25 * vis)) * 2048.0f);
            const auto _t41 = (zR1mC26 * 512.0f);
            const auto _t43 = ((zC46 + zR1mC26) * 512.0f);
            const auto _t53 = (((zC35 + zC36) + zC45) * 128.0f);
            const auto _t55 = (zC36 * 128.0f);
            const auto _t58 = (zC45 * 1024.0f);
            const auto _t61 = (_t55 * _t12);
            const auto _t62 = (_t13 * _t53);
            const auto _t64 = (((gR14 * vis) + zC35) * 128.0f);
            const auto _t68 = (_t58 * _t25);
            const auto _t71 = (_t55 * _t24);
            const auto _t98 = (_t58 * _t19);
            const auto _t99 = (_t55 * _t14);
            const auto _t100 = (_t11 * _t53);
            const auto _t111 = (_t58 * _t85);
            const auto _t113 = (_t84 * _t53);
            const auto _t119 = (_t100 + _t99);
            const auto _t52 = (_t17 * _t53);
            const auto _t54 = (_t55 * _t13);
            const auto _t57 = (_t58 * _t16);
            const auto _t60 = (_t61 - _t62);
            const auto _t70 = (_t9 * _t71);
            const auto _t112 = (_t9 * _t113);
            const auto _t115 = ((_t64 * _t86) - (_t7 * (_t112 + _t111)));
            const auto _t118 = (_t54 - _t52);
            const auto _t127 = (((_t64 * _t141) - ((_t7 * ((_t18 * _t119) - ((_t15 * _t60) + (_t58 * _t10)))) + (_t20 * (((_t22 * _t60) - (_t58 * _t23)) + (_t18 * _t71))))) - (_t24 * (((_t22 * _t119) + (_t15 * _t71)) + (_t58 * _t27))));
            const auto _t51 = (_t52 - _t54);
            const auto _t59 = (_t9 * _t60);
            const auto _t63 = (_t64 * _t21);
            const auto _t69 = (_t22 * _t51);
            const auto _t110 = ((_t64 * _t82) - (_t24 * (_t111 + _t112)));
            const auto _t114 = (_t30 * _t115);
            const auto _t117 = (((_t7 * (((_t15 * _t118) + (_t9 * _t119)) + _t98)) - (_t20 * (((_t22 * _t118) - _t70) - _t68))) + (_t64 * _t26));
            const auto _t124 = (((_t7 * ((_t18 * _t113) - (_t58 * _t83))) - (_t64 * _t125)) + (_t24 * ((_t22 * _t113) + (_t58 * _t87))));
            const auto _t126 = (_t30 * _t127);
            const auto _t35 = (_t36 * _t32);
            const auto _t40 = (_t41 * _t6);
            const auto _t42 = (_t43 * _t29);
            const auto _t50 = (_t18 * _t51);
            const auto _t56 = (_t57 + _t59);
            const auto _t67 = (_t68 + _t69);
            const auto _t95 = (_t41 * _t81);
            const auto _t109 = (_t30 * _t110);
            const auto _t116 = (_t30 * _t117);
            const auto _t123 = (_t30 * _t124);
            const auto _t39 = (_t4 * _t40);
            const auto _t49 = (_t50 - _t56);
            const auto _t66 = (_t67 + _t70);
            const auto _t91 = (_t43 * _t80);
            const auto _t96 = (_t36 * _t89);
            const auto _t97 = (((_t64 * _t8) + (_t20 * _t49)) + (_t24 * (((_t15 * _t51) - _t98) - (_t9 * (_t99 + _t100)))));
            const auto _t101 = (_t30 * _t97);
            const auto _t38 = (_t39 - _t42);
            const auto _t48 = (_t7 * _t49);
            const auto _t65 = (_t24 * _t66);
            const auto _t132 = ((_t63 - _t48) - (_t24 * ((_t69 + _t68) + _t70)));
            const auto _t37 = (_t2 * _t38);
            const auto _t47 = (_t48 - _t63);
            const auto _t131 = (_t30 * _t132);
            const auto _t46 = (_t47 + _t65);
            const auto _t94 = (_t77 * _t46);
            const auto _t45 = (_t30 * _t46);
            const auto _t93 = (_t94 - _t95);
            const auto _t92 = (_t4 * _t93);
            const auto _t90 = (_t91 + _t92);
            const auto vib = ((((_t0 * _t1) - (_t34 * (_t35 + _t37))) + (_t44 * ((_t2 * ((_t28 * _t45) - (_t4 * (_t4 * _t46)))) - (_t31 * (_t33 * _t45))))) * _t72);
            const auto tC47 = (gC47 * (0 - vib));
            const auto vch = ((((_t34 * (_t36 * _t75)) - ((_t73 * ((_t36 * _t3) - (_t31 * _t38))) + (_t0 * _t134))) + (_t44 * ((_t31 * _t90) - (_t36 * _t79)))) * _t72);
            const auto vR13C25 = (vis - vch);
            const auto vph = ((((_t73 * (_t37 + _t35)) - (_t0 * _t74)) + (_t44 * ((_t2 * _t90) - _t96))) * _t72);
            const auto tC46 = (gC46 * (vph - 0));
            const auto v14 = (((((_t73 * ((_t2 * ((_t4 * (_t4 * _t97)) - (_t28 * _t101))) + (_t31 * (_t33 * _t101)))) - (_t34 * ((_t2 * ((_t43 * _t102) - (_t4 * ((_t41 * _t103) - (_t77 * _t97))))) - (_t36 * _t108)))) - (_t0 * _t135)) + (_t44 * ((_t2 * ((_t28 * _t109) - (_t4 * (_t4 * _t110)))) - (_t31 * (_t33 * _t109))))) * _t72);
            const auto v45 = ((((_t44 * ((_t2 * ((_t28 * _t114) - (_t4 * (_t4 * _t115)))) - (_t31 * (_t33 * _t114)))) - ((_t73 * ((_t31 * (_t33 * _t116)) - (_t2 * ((_t28 * _t116) - (_t4 * (_t4 * _t117)))))) + (_t34 * ((_t2 * ((_t43 * _t120) - (_t4 * ((_t77 * _t117) + (_t41 * _t121))))) - (_t36 * _t122))))) + (_t0 * _t136)) * _t72);
            const auto tC35 = (gC35 * (v14 - v45));
            const auto v17 = ((((_t44 * ((_t2 * ((_t28 * _t123) - (_t4 * (_t4 * _t124)))) - (_t31 * (_t33 * _t123)))) - ((_t73 * ((_t31 * (_t33 * _t126)) - (_t2 * ((_t28 * _t126) - (_t4 * (_t4 * _t127)))))) + (_t34 * ((_t36 * _t128) - (_t2 * ((_t4 * ((_t77 * _t127) - (_t41 * _t130))) + (_t43 * _t129))))))) - (_t0 * _t137)) * _t72);
            const auto tC36 = (gC36 * (v17 - v45));
            const auto vpl = (((_t0 * _t78) - ((_t73 * ((_t31 * (_t33 * _t131)) - (_t2 * ((_t28 * _t131) - (_t4 * (_t4 * _t132)))))) + (_t34 * ((_t2 * (_t91 - (_t4 * ((_t77 * _t132) + _t95)))) - _t96)))) * _t72);
            const auto tC45 = (gC45 * (vpl - v45));
            const auto v18 = ((((_t0 * _t138) - ((_t73 * (((_t31 * (_t33 * _t40)) - (_t2 * ((_t28 * _t40) - (_t43 * _t5)))) - (_t36 * _t139))) + (_t34 * ((_t36 * _t133) - (_t2 * (_t43 * _t76)))))) + (_t44 * (((_t2 * ((_t28 * _t93) + (_t43 * _t88))) - (_t31 * (_t33 * _t93))) - (_t36 * _t140)))) * _t72);
            const auto vR1mC26 = (vph - v18);
            
            zC47 = 2 * tC47 - zC47;
            zR13C25 = gR13C25 * (1 - gnR13C25) * vR13C25 + gnR13C25 * zR13C25; // RC series
            zC46 = 2 * tC46 - zC46;
            zC35 = 2 * tC35 - zC35;
            zC36 = 2 * tC36 - zC36;
            zC45 = 2 * tC45 - zC45;
            zR1mC26 = gR1mC26 * (1 - gnR1mC26) * vR1mC26 + gnR1mC26 * zR1mC26; // RC series

            output[ch][n] = vib;
        }
        state[ch].zC47 = zC47;
        state[ch].zR13C25 = zR13C25;
        state[ch].zC46 = zC46;
        state[ch].zC35 = zC35;
        state[ch].zC36 = zC36;
        state[ch].zC45 = zC45;
        state[ch].zR1mC26 = zR1mC26;
    }
}

static float reset_shelf (Params_shelf params, State_shelf* state, int num_channels, float sample_rate, float vis_dc = 0.0f)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto gC47 = 2.0f * sample_rate * params.C47;
    
    const auto gR20 = 1.0f / params.R20;
    
    const auto gR1p = 1.0f / params.R1p;
    
    const auto R13 = params.R13;
    const auto C25 = params.C25;
    const auto gR13C25 = (2.0f * sample_rate * C25) / (1.0f + 2.0f * sample_rate * R13 * C25);
    const auto gnR13C25 = gR13C25 * (2.0f * sample_rate * R13 * C25 - 1.0f) / (2.0f * sample_rate * C25);
    
    const auto gR18 = 1.0f / params.R18;
    
    const auto gC46 = 2.0f * sample_rate * params.C46;
    
    const auto gRbias1 = 1.0f / params.Rbias1;
    
    const auto gR14 = 1.0f / params.R14;
    
    const auto gR2p = 1.0f / params.R2p;
    
    const auto gR2m = 1.0f / params.R2m;
    
    const auto gR17 = 1.0f / params.R17;
    
    const auto gC35 = 2.0f * sample_rate * params.C35;
    
    const auto gC36 = 2.0f * sample_rate * params.C36;
    
    const auto gC45 = 2.0f * sample_rate * params.C45;
    
    const auto gRbias2 = 1.0f / params.Rbias2;
    
    const auto gR19 = 1.0f / params.R19;
    
    const auto R1m = params.R1m;
    const auto C26 = params.C26;
    const auto gR1mC26 = (2.0f * sample_rate * C26) / (1.0f + 2.0f * sample_rate * R1m * C26);
    const auto gnR1mC26 = gR1mC26 * (2.0f * sample_rate * R1m * C26 - 1.0f) / (2.0f * sample_rate * C26);
    
    const auto vis = vis_dc;

    const auto zC47 = (-((gC47 * ((gR1p * (gR1p * ((gR18 + (1.0f / 1000000000.0f)) * ((gR14 * vis) * (gR2p * (((gR2m + gR17) + (1.0f / 1000000000.0f)) * (gRbias2 + (1.0f / 1000000000.0f)))))))) - ((gR1p + (1.0f / 1000000000.0f)) * (((gR1p + gRbias1) + (1.0f / 1000000000.0f)) * ((gR18 + (1.0f / 1000000000.0f)) * ((gR14 * vis) * (gR2p * (((gR2m + gR17) + (1.0f / 1000000000.0f)) * (gRbias2 + (1.0f / 1000000000.0f)))))))))) / (((gR1p + (1.0f / 1000000000.0f)) * (((gR1p + gRbias1) + (1.0f / 1000000000.0f)) * ((gR18 + (1.0f / 1000000000.0f)) * (((gR14 + gR2p) + (1.0f / 1000000000.0f)) * (gR2m * (gR17 * (gRbias2 + (1.0f / 1000000000.0f)))))))) - (gR1p * (gR1p * ((gR18 + (1.0f / 1000000000.0f)) * (((gR14 + gR2p) + (1.0f / 1000000000.0f)) * (gR2m * (gR17 * (gRbias2 + (1.0f / 1000000000.0f)))))))))));
    const auto zR13C25 = (gR13C25 * vis);
    const auto zC46 = 0.0f;
    const auto zC35 = ((gC35 * (((gR1p + (1.0f / 1000000000.0f)) * (((gR1p + gRbias1) + (1.0f / 1000000000.0f)) * ((gR18 + (1.0f / 1000000000.0f)) * ((gR14 * vis) * (gR2m * (gR17 * (gRbias2 + (1.0f / 1000000000.0f)))))))) - (gR1p * (gR1p * ((gR18 + (1.0f / 1000000000.0f)) * ((gR14 * vis) * (gR2m * (gR17 * (gRbias2 + (1.0f / 1000000000.0f)))))))))) / (((gR1p + (1.0f / 1000000000.0f)) * (((gR1p + gRbias1) + (1.0f / 1000000000.0f)) * ((gR18 + (1.0f / 1000000000.0f)) * (((gR14 + gR2p) + (1.0f / 1000000000.0f)) * (gR2m * (gR17 * (gRbias2 + (1.0f / 1000000000.0f)))))))) - (gR1p * (gR1p * ((gR18 + (1.0f / 1000000000.0f)) * (((gR14 + gR2p) + (1.0f / 1000000000.0f)) * (gR2m * (gR17 * (gRbias2 + (1.0f / 1000000000.0f))))))))));
    const auto zC36 = ((gC36 * ((gR1p * (gR1p * ((gR18 + (1.0f / 1000000000.0f)) * ((gR14 * vis) * (gR2p * (gR17 * (gRbias2 + (1.0f / 1000000000.0f)))))))) - ((gR1p + (1.0f / 1000000000.0f)) * (((gR1p + gRbias1) + (1.0f / 1000000000.0f)) * ((gR18 + (1.0f / 1000000000.0f)) * ((gR14 * vis) * (gR2p * (gR17 * (gRbias2 + (1.0f / 1000000000.0f)))))))))) / (((gR1p + (1.0f / 1000000000.0f)) * (((gR1p + gRbias1) + (1.0f / 1000000000.0f)) * ((gR18 + (1.0f / 1000000000.0f)) * (((gR14 + gR2p) + (1.0f / 1000000000.0f)) * (gR2m * (gR17 * (gRbias2 + (1.0f / 1000000000.0f)))))))) - (gR1p * (gR1p * ((gR18 + (1.0f / 1000000000.0f)) * (((gR14 + gR2p) + (1.0f / 1000000000.0f)) * (gR2m * (gR17 * (gRbias2 + (1.0f / 1000000000.0f))))))))));
    const auto zC45 = 0.0f;
    const auto zR1mC26 = (-((gR1mC26 * ((gR1p * (gR1p * (gR18 * ((gR14 * vis) * (gR2p * (((gR2m + gR17) + (1.0f / 1000000000.0f)) * (gRbias2 + (1.0f / 1000000000.0f)))))))) - ((gR1p + (1.0f / 1000000000.0f)) * (((gR1p + gRbias1) + (1.0f / 1000000000.0f)) * (gR18 * ((gR14 * vis) * (gR2p * (((gR2m + gR17) + (1.0f / 1000000000.0f)) * (gRbias2 + (1.0f / 1000000000.0f)))))))))) / (((gR1p + (1.0f / 1000000000.0f)) * (((gR1p + gRbias1) + (1.0f / 1000000000.0f)) * ((gR18 + (1.0f / 1000000000.0f)) * (((gR14 + gR2p) + (1.0f / 1000000000.0f)) * (gR2m * (gR17 * (gRbias2 + (1.0f / 1000000000.0f)))))))) - (gR1p * (gR1p * ((gR18 + (1.0f / 1000000000.0f)) * (((gR14 + gR2p) + (1.0f / 1000000000.0f)) * (gR2m * (gR17 * (gRbias2 + (1.0f / 1000000000.0f)))))))))));

    const auto vib_dc_out = (((gR1p * (gR1p * ((gR18 + (1.0f / 1000000000.0f)) * ((gR14 * vis) * (gR2p * (((gR2m + gR17) + (1.0f / 1000000000.0f)) * (gRbias2 + (1.0f / 1000000000.0f)))))))) - ((gR1p + (1.0f / 1000000000.0f)) * (((gR1p + gRbias1) + (1.0f / 1000000000.0f)) * ((gR18 + (1.0f / 1000000000.0f)) * ((gR14 * vis) * (gR2p * (((gR2m + gR17) + (1.0f / 1000000000.0f)) * (gRbias2 + (1.0f / 1000000000.0f))))))))) / (((gR1p + (1.0f / 1000000000.0f)) * (((gR1p + gRbias1) + (1.0f / 1000000000.0f)) * ((gR18 + (1.0f / 1000000000.0f)) * (((gR14 + gR2p) + (1.0f / 1000000000.0f)) * (gR2m * (gR17 * (gRbias2 + (1.0f / 1000000000.0f)))))))) - (gR1p * (gR1p * ((gR18 + (1.0f / 1000000000.0f)) * (((gR14 + gR2p) + (1.0f / 1000000000.0f)) * (gR2m * (gR17 * (gRbias2 + (1.0f / 1000000000.0f))))))))));

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].zC47 = zC47;
        state[ch].zR13C25 = zR13C25;
        state[ch].zC46 = zC46;
        state[ch].zC35 = zC35;
        state[ch].zC36 = zC36;
        state[ch].zC45 = zC45;
        state[ch].zR1mC26 = zR1mC26;
    }
    return vib_dc_out;
}

struct Params_bell {
    float R21 = 6.8e+03f;
    float R23 = 8.2e+03f;
    float R25 = 6.8e+03f;
    float C60 = 1.5e-10f;
    float R3p = 5.0e+03f;
    float R3m = 5.0e+03f;
    float Cf = 2.2e-07f;
    float Lf = 2.0e+00f;
    float R24 = 4.7e+03f;
    float R26 = 1.0e+04f;
};

struct State_bell {
    float zC60 {};
    float zCf {};
    float zLf {};
};

static void compute_bell (const float* const* input, float** output, int num_channels, int num_samples, Params_bell params, State_bell* state, float sample_rate)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto gR21 = 1.0f / params.R21;
    
    const auto gR23 = 1.0f / params.R23;
    
    const auto gR25 = 1.0f / params.R25;
    
    const auto gC60 = 2.0f * sample_rate * params.C60;
    
    const auto gR3p = 1.0f / params.R3p;
    
    const auto gR3m = 1.0f / params.R3m;
    
    const auto gCf = 2.0f * sample_rate * params.Cf;
    
    const auto gLf = 1.0f / (2.0f * sample_rate * params.Lf);
    
    const auto gR24 = 1.0f / params.R24;
    
    const auto gR26 = 1.0f / params.R26;
    
    const auto _t0 = (((gR21 + gR23) + gR25) * 8192.0f);
    const auto _t1 = (gR24 * 4096.0f);
    const auto _t2 = (((gR3p + gR3m) + gCf) * 64.0f);
    const auto _t6 = (gLf * 4096.0f);
    const auto _t8 = ((gCf + gLf) * 64.0f);
    const auto _t13 = (gCf * 64.0f);
    const auto _t19 = ((gLf + gR24) * 4096.0f);
    const auto _t21 = (gLf * 64.0f);
    const auto _t24 = (gR23 * 4096.0f);
    const auto _t26 = (gR25 * 8192.0f);
    const auto _t28 = (gR3p * 64.0f);
    const auto _t29 = (gC60 * 4096.0f);
    const auto _t12 = (_t13 * _t6);
    const auto _t18 = (_t13 * _t19);
    const auto _t20 = ((_t8 * _t19) - (_t21 * _t6));
    const auto _t27 = (_t28 * _t12);
    const auto _t30 = (_t28 * _t20);
    const auto _t35 = (_t28 * _t18);
    const auto _t17 = ((_t13 * _t18) - (_t2 * _t20));
    const auto _t23 = (_t24 * _t17);
    const auto _t25 = (1.0f / ((_t26 * _t23) - (_t0 * ((_t1 * _t27) - (_t29 * _t17)))));
    const auto _t38 = (_t24 * _t30);
    const auto _t39 = (_t24 * _t35);
    const auto _t40 = (_t24 * _t27);
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zC60 = state[ch].zC60;
        auto zCf = state[ch].zCf;
        auto zLf = state[ch].zLf;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vib = input[ch][n];

            const auto _t5 = ((zCf + zLf) * 64.0f);
            const auto _t9 = (zLf * 4096.0f);
            const auto _t11 = (((gR3m * vib) + zCf) * 64.0f);
            const auto _t15 = (_t13 * _t9);
            const auto _t16 = (zC60 * 4096.0f);
            const auto _t22 = ((gR21 * vib) * 8192.0f);
            const auto _t33 = (_t21 * _t9);
            const auto _t34 = (_t5 * _t19);
            const auto _t4 = (_t5 * _t6);
            const auto _t7 = (_t8 * _t9);
            const auto _t14 = (_t13 * _t15);
            const auto _t32 = (_t33 - _t34);
            const auto _t36 = ((_t2 * _t32) + (_t11 * _t18));
            const auto _t3 = (_t4 - _t7);
            const auto _t10 = (_t11 * _t12);
            const auto _t31 = ((_t11 * _t20) + (_t13 * _t32));
            const auto _t37 = ((_t14 - (_t2 * (_t7 - _t4))) - _t10);
            const auto vb = (-(((_t0 * ((_t1 * (((_t2 * _t3) - _t10) + _t14)) - (_t16 * _t17))) + (_t22 * _t23)) * _t25));
            const auto tC60 = (gC60 * (vb - 0));
            const auto vpt = (((_t22 * _t38) - ((_t0 * (((_t29 * _t31) + (_t16 * _t30)) + (_t1 * (_t28 * _t3)))) + (_t26 * (_t24 * _t31)))) * _t25);
            const auto vc = (((_t22 * _t39) - ((_t0 * (((_t29 * _t36) + (_t16 * _t35)) - (_t1 * (_t28 * _t15)))) + (_t26 * (_t24 * _t36)))) * _t25);
            const auto tCf = (gCf * (vpt - vc));
            const auto vl = ((((_t26 * (_t24 * _t37)) - (_t0 * ((_t16 * _t27) - (_t29 * _t37)))) + (_t22 * _t40)) * _t25);
            const auto tLf = (gLf * (vc - vl));
            
            zC60 = 2 * tC60 - zC60;
            zCf = 2 * tCf - zCf;
            zLf = 2 * tLf + zLf;

            output[ch][n] = vb;
        }
        state[ch].zC60 = zC60;
        state[ch].zCf = zCf;
        state[ch].zLf = zLf;
    }
}

static float reset_bell (Params_bell params, State_bell* state, int num_channels, float sample_rate, float vib_dc = 0.0f)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto gR21 = 1.0f / params.R21;
    
    const auto gR23 = 1.0f / params.R23;
    
    const auto gR25 = 1.0f / params.R25;
    
    const auto gC60 = 2.0f * sample_rate * params.C60;
    
    const auto gR3p = 1.0f / params.R3p;
    
    const auto gR3m = 1.0f / params.R3m;
    
    const auto gCf = 2.0f * sample_rate * params.Cf;
    
    const auto gLf = 1.0f / (2.0f * sample_rate * params.Lf);
    
    const auto gR24 = 1.0f / params.R24;
    
    const auto gR26 = 1.0f / params.R26;
    
    const auto vib = vib_dc;

    const auto zC60 = (-(((gC60 * gR21) * vib) / gR25));
    const auto zCf = (-((gCf * (((gR21 * vib) * gR3p) - ((gR25 * gR3m) * vib))) / (gR25 * ((gR3p + gR3m) + (1.0f / 1000000000.0f)))));

    const auto vb_dc_out = (-((gR21 * vib) / gR25));

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].zC60 = zC60;
        state[ch].zCf = zCf;
    }
    return vb_dc_out;
}

struct Params_hpf {
    float Rll = 1.0e+04f;
    float C22 = 2.2e-06f;
    float L1 = 1.0e+01f;
    float R9 = 5.1e+03f;
    float R10 = 1.5e+04f;
    float R27 = 1.8e+03f;
    float C21 = 2.2e-06f;
};

struct State_hpf {
    float zC22 {};
    float zL1 {};
    float zR27C21 {};
};

static void compute_hpf (const float* const* input, float** output, int num_channels, int num_samples, Params_hpf params, State_hpf* state, float sample_rate)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto gRll = 1.0f / params.Rll;
    
    const auto gC22 = 2.0f * sample_rate * params.C22;
    
    const auto gL1 = 1.0f / (2.0f * sample_rate * params.L1);
    
    const auto gR9 = 1.0f / params.R9;
    
    const auto gR10 = 1.0f / params.R10;
    
    const auto R27 = params.R27;
    const auto C21 = params.C21;
    const auto gR27C21 = (2.0f * sample_rate * C21) / (1.0f + 2.0f * sample_rate * R27 * C21);
    const auto gnR27C21 = gR27C21 * (2.0f * sample_rate * R27 * C21 - 1.0f) / (2.0f * sample_rate * C21);
    
    const auto _t3 = (((gC22 + gL1) + gR27C21) * 4.0f);
    const auto _t5 = (gC22 * 4.0f);
    const auto _t8 = ((gC22 + gR9) * 4.0f);
    const auto _t7 = ((_t5 * _t5) - (_t8 * _t3));
    const auto _t9 = (gRll + gR10);
    const auto _t10 = (1.0f / (gRll * _t7));
    const auto _t11 = (1.0f / _t7);
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zC22 = state[ch].zC22;
        auto zL1 = state[ch].zL1;
        auto zR27C21 = state[ch].zR27C21;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vb = input[ch][n];

            const auto _t2 = (zC22 * 4.0f);
            const auto _t6 = (((zC22 - zL1) + ((gR27C21 * vb) - zR27C21)) * 4.0f);
            const auto _t1 = (_t2 * _t3);
            const auto _t4 = (_t5 * _t6);
            const auto _t0 = (_t1 - _t4);
            const auto vo = ((_t9 * _t0) * _t10);
            const auto vl2 = (((_t2 * _t5) - (_t8 * _t6)) * _t11);
            const auto vnh = (_t0 * _t11);
            const auto tC22 = (gC22 * (vl2 - vnh));
            const auto tL1 = (gL1 * (vl2 - 0));
            const auto vR27C21 = (vb - vl2);
            
            zC22 = 2 * tC22 - zC22;
            zL1 = 2 * tL1 + zL1;
            zR27C21 = gR27C21 * (1 - gnR27C21) * vR27C21 + gnR27C21 * zR27C21; // RC series

            output[ch][n] = vo;
        }
        state[ch].zC22 = zC22;
        state[ch].zL1 = zL1;
        state[ch].zR27C21 = zR27C21;
    }
}

static float reset_hpf (Params_hpf params, State_hpf* state, int num_channels, float sample_rate, float vb_dc = 0.0f)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto gRll = 1.0f / params.Rll;
    
    const auto gC22 = 2.0f * sample_rate * params.C22;
    
    const auto gL1 = 1.0f / (2.0f * sample_rate * params.L1);
    
    const auto gR9 = 1.0f / params.R9;
    
    const auto gR10 = 1.0f / params.R10;
    
    const auto R27 = params.R27;
    const auto C21 = params.C21;
    const auto gR27C21 = (2.0f * sample_rate * C21) / (1.0f + 2.0f * sample_rate * R27 * C21);
    const auto gnR27C21 = gR27C21 * (2.0f * sample_rate * R27 * C21 - 1.0f) / (2.0f * sample_rate * C21);
    
    const auto vb = vb_dc;

    const auto zC22 = 0.0f;
    const auto zR27C21 = (gR27C21 * vb);

    const auto vo_dc_out = 0.0f;

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].zC22 = zC22;
        state[ch].zR27C21 = zR27C21;
    }
    return vo_dc_out;
}

struct Params {
    Params_shelf shelf;
    Params_bell bell;
    Params_hpf hpf;
};

struct State {
    State_shelf shelf;
    State_bell bell;
    State_hpf hpf;
};

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    for (int ch = 0; ch < num_channels; ++ch)
    {
        compute_shelf(&input[ch], &output[ch], 1, num_samples, params.shelf, &state[ch].shelf, sample_rate);
        compute_bell(&output[ch], &output[ch], 1, num_samples, params.bell, &state[ch].bell, sample_rate);
        compute_hpf(&output[ch], &output[ch], 1, num_samples, params.hpf, &state[ch].hpf, sample_rate);
    }
}

static void reset (Params params, State* state, int num_channels, float sample_rate, float vis_dc = 0.0f)
{
    for (int ch = 0; ch < num_channels; ++ch)
    {
        const float shelf_dc = reset_shelf(params.shelf, &state[ch].shelf, 1, sample_rate, vis_dc);
        const float bell_dc = reset_bell(params.bell, &state[ch].bell, 1, sample_rate, shelf_dc);
        const float hpf_dc = reset_hpf(params.hpf, &state[ch].hpf, 1, sample_rate, bell_dc);
    }
}
