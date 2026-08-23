// Auto-generated with netlist_codegen version 5608cd2.
// Command: netlist_codegen eq73.net eq73.h -opt_port_matrix -type_name float

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
    const auto _t13 = (gC45 * 128.0f);
    const auto _t14 = ((((gR2p + gR2m) + gC45) + gR19) * 1024.0f);
    const auto _t16 = (((gR2m + gR17) + gC36) * 128.0f);
    const auto _t17 = (gC36 * 128.0f);
    const auto _t18 = (gC45 * 1024.0f);
    const auto _t19 = ((_t11 * _t17) + (_t16 * _t13));
    const auto _t20 = (gR2p * 128.0f);
    const auto _t22 = (gR2p * 1024.0f);
    const auto _t24 = (gC35 * 128.0f);
    const auto _t25 = (_t16 * _t24);
    const auto _t27 = (_t11 * _t24);
    const auto _t28 = ((((gR1p + gC46) + gRbias1) + gR1mC26) * 512.0f);
    const auto _t30 = ((gR18 + gR1mC26) * 512.0f);
    const auto _t31 = (gR1p * 2048.0f);
    const auto _t33 = (gR1p * 512.0f);
    const auto _t34 = (gC46 * 512.0f);
    const auto _t42 = (gR19 * 512.0f);
    const auto _t51 = (gC47 * 512.0f);
    const auto _t55 = (gR18 * 512.0f);
    const auto _t62 = (gR17 * 128.0f);
    const auto _t63 = (_t62 * _t17);
    const auto _t65 = (_t62 * _t24);
    const auto _t93 = (_t62 * _t13);
    const auto _t112 = (_t14 * _t63);
    const auto _t113 = (_t9 * _t93);
    const auto _t10 = ((_t11 * _t12) + (_t17 * _t13));
    const auto _t15 = ((_t16 * _t12) - (_t17 * _t17));
    const auto _t23 = (_t17 * _t24);
    const auto _t26 = (((_t22 * _t19) + (_t14 * _t25)) - (_t9 * _t27));
    const auto _t61 = (_t62 * _t12);
    const auto _t64 = ((_t9 * _t65) - (_t22 * _t63));
    const auto _t111 = (_t112 + _t113);
    const auto _t118 = ((_t22 * _t61) + (_t18 * _t65));
    const auto _t125 = (((_t7 * ((_t18 * _t93) - (_t14 * _t61))) + (_t20 * _t118)) + (_t24 * ((_t22 * _t93) + (_t14 * _t65))));
    const auto _t8 = (((_t9 * _t10) - (_t14 * _t15)) + (_t18 * _t19));
    const auto _t21 = (((_t22 * _t15) + (_t9 * _t23)) + (_t18 * _t25));
    const auto _t60 = ((_t9 * _t61) + (_t18 * _t63));
    const auto _t92 = ((_t20 * _t60) + (_t24 * _t111));
    const auto _t110 = ((_t7 * _t111) + (_t20 * _t64));
    const auto _t124 = (_t30 * _t125);
    const auto _t6 = (((_t7 * _t8) + (_t20 * _t21)) + (_t24 * _t26));
    const auto _t29 = (_t30 * _t6);
    const auto _t32 = (_t33 * _t29);
    const auto _t54 = (_t55 * _t6);
    const auto _t59 = ((_t7 * _t60) - (_t24 * _t64));
    const auto _t66 = (_t4 * _t59);
    const auto _t91 = (_t30 * _t92);
    const auto _t94 = (_t33 * _t91);
    const auto _t109 = (_t30 * _t110);
    const auto _t114 = (_t33 * _t109);
    const auto _t123 = (_t33 * _t124);
    const auto _t132 = (_t33 * _t54);
    const auto _t5 = (_t4 * _t6);
    const auto _t53 = (_t4 * _t54);
    const auto _t58 = (_t30 * _t59);
    const auto _t67 = (_t33 * _t58);
    const auto _t3 = ((_t4 * _t5) - (_t28 * _t29));
    const auto _t52 = (_t2 * _t53);
    const auto _t57 = ((_t28 * _t58) - (_t4 * _t66));
    const auto _t1 = ((_t2 * _t3) + (_t31 * _t32));
    const auto _t56 = ((_t2 * _t57) - (_t31 * _t67));
    const auto _t50 = (1.0f / (((_t51 * _t1) - (_t34 * _t52)) + (_t42 * _t56)));
    const auto _t134 = (_t31 * _t53);
    const auto _t135 = ((_t2 * ((_t4 * (_t4 * _t92)) - (_t28 * _t91))) + (_t31 * _t94));
    const auto _t136 = ((_t2 * ((_t28 * _t109) - (_t4 * (_t4 * _t110)))) - (_t31 * _t114));
    const auto _t137 = ((_t2 * ((_t28 * _t124) - (_t4 * (_t4 * _t125)))) - (_t31 * _t123));
    const auto _t138 = ((_t31 * _t132) - (_t2 * (_t28 * _t54)));
    const auto _t139 = (_t33 * _t5);
    const auto _t140 = (_t33 * _t66);
    const auto _t141 = (((_t22 * _t10) + (_t14 * _t23)) + (_t18 * _t27));
    float c0_tC36;
    float c_tC36[8];
    float c0_tC35;
    float c_tC35[8];
    float c0_tC45;
    float c_tC45[8];
    float c0_v17;
    float c_v17[8];
    float c0_vR1mC26;
    float c_vR1mC26[8];
    float c0_v14;
    float c_v14[8];
    float c0_v45;
    float c_v45[8];
    float c0_v18;
    float c_v18[8];
    float c0_vR13C25;
    float c_vR13C25[8];
    float c0_vch;
    float c_vch[8];
    float c0_tC47;
    float c_tC47[8];
    float c0_vpl;
    float c_vpl[8];
    float c0_tC46;
    float c_tC46[8];
    float c0_vib;
    float c_vib[8];
    float c0_vph;
    float c_vph[8];
    
    for (int _k = 0; _k <= 8; ++_k)
    {
        const auto vis = (_k == 1) ? 1.0f : 0.0f;
        const auto zC47 = (_k == 2) ? 1.0f : 0.0f;
        const auto zR13C25 = (_k == 3) ? 1.0f : 0.0f;
        const auto zC46 = (_k == 4) ? 1.0f : 0.0f;
        const auto zC35 = (_k == 5) ? 1.0f : 0.0f;
        const auto zC36 = (_k == 6) ? 1.0f : 0.0f;
        const auto zC45 = (_k == 7) ? 1.0f : 0.0f;
        const auto zR1mC26 = (_k == 8) ? 1.0f : 0.0f;
        const auto _t0 = ((zC46 - zC47) * 512.0f);
        const auto _t35 = ((zR13C25 - (gR13C25 * vis)) * 2048.0f);
        const auto _t39 = (zR1mC26 * 512.0f);
        const auto _t41 = ((zC46 + zR1mC26) * 512.0f);
        const auto _t44 = (((zC35 + zC36) + zC45) * 128.0f);
        const auto _t45 = (zC36 * 128.0f);
        const auto _t46 = (zC45 * 1024.0f);
        const auto _t47 = (((gR14 * vis) + zC35) * 128.0f);
        const auto _t49 = (_t45 * _t24);
        const auto _t78 = (_t9 * _t49);
        const auto _t89 = (_t45 * _t12);
        const auto _t100 = (_t46 * _t63);
        const auto _t104 = (_t45 * _t17);
        const auto _t106 = (_t45 * _t13);
        const auto _t107 = (_t46 * _t19);
        const auto _t108 = (_t46 * _t25);
        const auto _t117 = (_t62 * _t44);
        const auto _t121 = (_t11 * _t44);
        const auto _t122 = (_t17 * _t44);
        const auto _t130 = (_t89 - _t122);
        const auto _t133 = (_t16 * _t44);
        const auto _t87 = (_t133 - _t104);
        const auto _t88 = (_t9 * _t130);
        const auto _t99 = (_t9 * _t117);
        const auto _t103 = (_t104 - _t133);
        const auto _t105 = (_t121 + _t106);
        const auto _t120 = (((_t47 * _t141) - ((_t7 * ((_t18 * _t105) - ((_t14 * _t130) + (_t46 * _t10)))) + (_t20 * (((_t22 * _t130) - (_t46 * _t23)) + (_t18 * _t49))))) - (_t24 * (((_t22 * _t105) + (_t14 * _t49)) + (_t46 * _t27))));
        const auto _t129 = (_t46 * _t15);
        const auto _t131 = (_t22 * _t87);
        const auto _t48 = (_t108 + _t131);
        const auto _t74 = (_t18 * _t87);
        const auto _t75 = (_t47 * _t21);
        const auto _t77 = (_t48 + _t78);
        const auto _t84 = (_t129 + _t88);
        const auto _t96 = ((_t47 * _t60) - (_t24 * (_t100 + _t99)));
        const auto _t98 = ((_t47 * _t64) - (_t7 * (_t99 + _t100)));
        const auto _t102 = (((_t7 * (((_t14 * _t103) + (_t9 * _t105)) + _t107)) - (_t20 * (((_t22 * _t103) - _t78) - _t108))) + (_t47 * _t26));
        const auto _t116 = (((_t7 * ((_t18 * _t117) - (_t46 * _t61))) - (_t47 * _t118)) + (_t24 * ((_t22 * _t117) + (_t46 * _t65))));
        const auto _t119 = (_t30 * _t120);
        const auto _t40 = (_t41 * _t29);
        const auto _t73 = (_t74 - _t84);
        const auto _t76 = (_t24 * _t77);
        const auto _t79 = (_t39 * _t59);
        const auto _t80 = (_t39 * _t6);
        const auto _t81 = (_t35 * _t32);
        const auto _t86 = (((_t47 * _t8) + (_t20 * _t73)) + (_t24 * (((_t14 * _t87) - _t107) - (_t9 * (_t106 + _t121)))));
        const auto _t90 = (_t30 * _t86);
        const auto _t95 = (_t30 * _t96);
        const auto _t97 = (_t30 * _t98);
        const auto _t101 = (_t30 * _t102);
        const auto _t115 = (_t30 * _t116);
        const auto _t128 = (_t7 * _t73);
        const auto _t38 = (_t4 * _t80);
        const auto _t68 = (_t41 * _t58);
        const auto _t72 = (_t128 - _t75);
        const auto _t85 = (_t35 * _t67);
        const auto _t127 = ((_t75 - _t128) - (_t24 * ((_t131 + _t108) + _t78)));
        const auto _t37 = (_t38 - _t40);
        const auto _t71 = (_t72 + _t76);
        const auto _t126 = (_t30 * _t127);
        const auto _t36 = (_t2 * _t37);
        const auto _t43 = (_t30 * _t71);
        const auto _t70 = (_t55 * _t71);
        const auto _t69 = (_t70 - _t79);
        const auto _t83 = (_t4 * _t69);
        const auto _t82 = (_t68 + _t83);
        const auto vib = ((((_t0 * _t1) - (_t34 * (_t81 + _t36))) + (_t42 * ((_t2 * ((_t28 * _t43) - (_t4 * (_t4 * _t71)))) - (_t31 * (_t33 * _t43))))) * _t50);
        const auto tC47 = (gC47 * (0 - vib));
        const auto vch = ((((_t34 * (_t35 * _t53)) - ((_t51 * ((_t35 * _t3) - (_t31 * _t37))) + (_t0 * _t134))) + (_t42 * ((_t31 * _t82) - (_t35 * _t57)))) * _t50);
        const auto vR13C25 = (vis - vch);
        const auto vph = ((((_t51 * (_t36 + _t81)) - (_t0 * _t52)) + (_t42 * ((_t2 * _t82) - _t85))) * _t50);
        const auto tC46 = (gC46 * (vph - 0));
        const auto v14 = (((((_t51 * ((_t2 * ((_t4 * (_t4 * _t86)) - (_t28 * _t90))) + (_t31 * (_t33 * _t90)))) - (_t34 * ((_t2 * ((_t41 * _t91) - (_t4 * ((_t39 * _t92) - (_t55 * _t86))))) - (_t35 * _t94)))) - (_t0 * _t135)) + (_t42 * ((_t2 * ((_t28 * _t95) - (_t4 * (_t4 * _t96)))) - (_t31 * (_t33 * _t95))))) * _t50);
        const auto v45 = ((((_t42 * ((_t2 * ((_t28 * _t97) - (_t4 * (_t4 * _t98)))) - (_t31 * (_t33 * _t97)))) - ((_t51 * ((_t31 * (_t33 * _t101)) - (_t2 * ((_t28 * _t101) - (_t4 * (_t4 * _t102)))))) + (_t34 * ((_t2 * ((_t41 * _t109) - (_t4 * ((_t55 * _t102) + (_t39 * _t110))))) - (_t35 * _t114))))) + (_t0 * _t136)) * _t50);
        const auto tC35 = (gC35 * (v14 - v45));
        const auto v17 = ((((_t42 * ((_t2 * ((_t28 * _t115) - (_t4 * (_t4 * _t116)))) - (_t31 * (_t33 * _t115)))) - ((_t51 * ((_t31 * (_t33 * _t119)) - (_t2 * ((_t28 * _t119) - (_t4 * (_t4 * _t120)))))) + (_t34 * ((_t35 * _t123) - (_t2 * ((_t4 * ((_t55 * _t120) - (_t39 * _t125))) + (_t41 * _t124))))))) - (_t0 * _t137)) * _t50);
        const auto tC36 = (gC36 * (v17 - v45));
        const auto vpl = (((_t0 * _t56) - ((_t51 * ((_t31 * (_t33 * _t126)) - (_t2 * ((_t28 * _t126) - (_t4 * (_t4 * _t127)))))) + (_t34 * ((_t2 * (_t68 - (_t4 * ((_t55 * _t127) + _t79)))) - _t85)))) * _t50);
        const auto tC45 = (gC45 * (vpl - v45));
        const auto v18 = ((((_t0 * _t138) - ((_t51 * (((_t31 * (_t33 * _t80)) - (_t2 * ((_t28 * _t80) - (_t41 * _t5)))) - (_t35 * _t139))) + (_t34 * ((_t35 * _t132) - (_t2 * (_t41 * _t54)))))) + (_t42 * (((_t2 * ((_t28 * _t69) + (_t41 * _t66))) - (_t31 * (_t33 * _t69))) - (_t35 * _t140)))) * _t50);
        const auto vR1mC26 = (vph - v18);
        if (_k == 0) {
            c0_tC36 = tC36;
            c0_tC35 = tC35;
            c0_tC45 = tC45;
            c0_v17 = v17;
            c0_vR1mC26 = vR1mC26;
            c0_v14 = v14;
            c0_v45 = v45;
            c0_v18 = v18;
            c0_vR13C25 = vR13C25;
            c0_vch = vch;
            c0_tC47 = tC47;
            c0_vpl = vpl;
            c0_tC46 = tC46;
            c0_vib = vib;
            c0_vph = vph;
        } else {
            c_tC36[_k - 1] = tC36 - c0_tC36;
            c_tC35[_k - 1] = tC35 - c0_tC35;
            c_tC45[_k - 1] = tC45 - c0_tC45;
            c_v17[_k - 1] = v17 - c0_v17;
            c_vR1mC26[_k - 1] = vR1mC26 - c0_vR1mC26;
            c_v14[_k - 1] = v14 - c0_v14;
            c_v45[_k - 1] = v45 - c0_v45;
            c_v18[_k - 1] = v18 - c0_v18;
            c_vR13C25[_k - 1] = vR13C25 - c0_vR13C25;
            c_vch[_k - 1] = vch - c0_vch;
            c_tC47[_k - 1] = tC47 - c0_tC47;
            c_vpl[_k - 1] = vpl - c0_vpl;
            c_tC46[_k - 1] = tC46 - c0_tC46;
            c_vib[_k - 1] = vib - c0_vib;
            c_vph[_k - 1] = vph - c0_vph;
        }
    }
    
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

            const auto tC36 = c0_tC36 + c_tC36[0] * vis + c_tC36[1] * zC47 + c_tC36[2] * zR13C25 + c_tC36[3] * zC46 + c_tC36[4] * zC35 + c_tC36[5] * zC36 + c_tC36[6] * zC45 + c_tC36[7] * zR1mC26;
            const auto tC35 = c0_tC35 + c_tC35[0] * vis + c_tC35[1] * zC47 + c_tC35[2] * zR13C25 + c_tC35[3] * zC46 + c_tC35[4] * zC35 + c_tC35[5] * zC36 + c_tC35[6] * zC45 + c_tC35[7] * zR1mC26;
            const auto tC45 = c0_tC45 + c_tC45[0] * vis + c_tC45[1] * zC47 + c_tC45[2] * zR13C25 + c_tC45[3] * zC46 + c_tC45[4] * zC35 + c_tC45[5] * zC36 + c_tC45[6] * zC45 + c_tC45[7] * zR1mC26;
            const auto v17 = c0_v17 + c_v17[0] * vis + c_v17[1] * zC47 + c_v17[2] * zR13C25 + c_v17[3] * zC46 + c_v17[4] * zC35 + c_v17[5] * zC36 + c_v17[6] * zC45 + c_v17[7] * zR1mC26;
            const auto vR1mC26 = c0_vR1mC26 + c_vR1mC26[0] * vis + c_vR1mC26[1] * zC47 + c_vR1mC26[2] * zR13C25 + c_vR1mC26[3] * zC46 + c_vR1mC26[4] * zC35 + c_vR1mC26[5] * zC36 + c_vR1mC26[6] * zC45 + c_vR1mC26[7] * zR1mC26;
            const auto v14 = c0_v14 + c_v14[0] * vis + c_v14[1] * zC47 + c_v14[2] * zR13C25 + c_v14[3] * zC46 + c_v14[4] * zC35 + c_v14[5] * zC36 + c_v14[6] * zC45 + c_v14[7] * zR1mC26;
            const auto v45 = c0_v45 + c_v45[0] * vis + c_v45[1] * zC47 + c_v45[2] * zR13C25 + c_v45[3] * zC46 + c_v45[4] * zC35 + c_v45[5] * zC36 + c_v45[6] * zC45 + c_v45[7] * zR1mC26;
            const auto v18 = c0_v18 + c_v18[0] * vis + c_v18[1] * zC47 + c_v18[2] * zR13C25 + c_v18[3] * zC46 + c_v18[4] * zC35 + c_v18[5] * zC36 + c_v18[6] * zC45 + c_v18[7] * zR1mC26;
            const auto vR13C25 = c0_vR13C25 + c_vR13C25[0] * vis + c_vR13C25[1] * zC47 + c_vR13C25[2] * zR13C25 + c_vR13C25[3] * zC46 + c_vR13C25[4] * zC35 + c_vR13C25[5] * zC36 + c_vR13C25[6] * zC45 + c_vR13C25[7] * zR1mC26;
            const auto vch = c0_vch + c_vch[0] * vis + c_vch[1] * zC47 + c_vch[2] * zR13C25 + c_vch[3] * zC46 + c_vch[4] * zC35 + c_vch[5] * zC36 + c_vch[6] * zC45 + c_vch[7] * zR1mC26;
            const auto tC47 = c0_tC47 + c_tC47[0] * vis + c_tC47[1] * zC47 + c_tC47[2] * zR13C25 + c_tC47[3] * zC46 + c_tC47[4] * zC35 + c_tC47[5] * zC36 + c_tC47[6] * zC45 + c_tC47[7] * zR1mC26;
            const auto vpl = c0_vpl + c_vpl[0] * vis + c_vpl[1] * zC47 + c_vpl[2] * zR13C25 + c_vpl[3] * zC46 + c_vpl[4] * zC35 + c_vpl[5] * zC36 + c_vpl[6] * zC45 + c_vpl[7] * zR1mC26;
            const auto tC46 = c0_tC46 + c_tC46[0] * vis + c_tC46[1] * zC47 + c_tC46[2] * zR13C25 + c_tC46[3] * zC46 + c_tC46[4] * zC35 + c_tC46[5] * zC36 + c_tC46[6] * zC45 + c_tC46[7] * zR1mC26;
            const auto vib = c0_vib + c_vib[0] * vis + c_vib[1] * zC47 + c_vib[2] * zR13C25 + c_vib[3] * zC46 + c_vib[4] * zC35 + c_vib[5] * zC36 + c_vib[6] * zC45 + c_vib[7] * zR1mC26;
            const auto vph = c0_vph + c_vph[0] * vis + c_vph[1] * zC47 + c_vph[2] * zR13C25 + c_vph[3] * zC46 + c_vph[4] * zC35 + c_vph[5] * zC36 + c_vph[6] * zC45 + c_vph[7] * zR1mC26;
            
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
    const auto _t4 = (gLf * 4096.0f);
    const auto _t5 = ((gCf + gLf) * 64.0f);
    const auto _t10 = (gCf * 64.0f);
    const auto _t15 = ((gLf + gR24) * 4096.0f);
    const auto _t17 = (gLf * 64.0f);
    const auto _t20 = (gR23 * 4096.0f);
    const auto _t22 = (gR25 * 8192.0f);
    const auto _t24 = (gR3p * 64.0f);
    const auto _t25 = (gC60 * 4096.0f);
    const auto _t9 = (_t10 * _t4);
    const auto _t14 = (_t10 * _t15);
    const auto _t16 = ((_t5 * _t15) - (_t17 * _t4));
    const auto _t23 = (_t24 * _t9);
    const auto _t26 = (_t24 * _t16);
    const auto _t33 = (_t24 * _t14);
    const auto _t13 = ((_t10 * _t14) - (_t2 * _t16));
    const auto _t19 = (_t20 * _t13);
    const auto _t21 = (1.0f / ((_t22 * _t19) - (_t0 * ((_t1 * _t23) - (_t25 * _t13)))));
    const auto _t38 = (_t20 * _t26);
    const auto _t39 = (_t20 * _t33);
    const auto _t40 = (_t20 * _t23);
    float c0_tCf;
    float c_tCf[4];
    float c0_tLf;
    float c_tLf[4];
    float c0_vpt;
    float c_vpt[4];
    float c0_vc;
    float c_vc[4];
    float c0_vl;
    float c_vl[4];
    float c0_tC60;
    float c_tC60[4];
    float c0_vb;
    float c_vb[4];
    
    for (int _k = 0; _k <= 4; ++_k)
    {
        const auto vib = (_k == 1) ? 1.0f : 0.0f;
        const auto zC60 = (_k == 2) ? 1.0f : 0.0f;
        const auto zCf = (_k == 3) ? 1.0f : 0.0f;
        const auto zLf = (_k == 4) ? 1.0f : 0.0f;
        const auto _t3 = ((zCf + zLf) * 64.0f);
        const auto _t6 = (zLf * 4096.0f);
        const auto _t8 = (((gR3m * vib) + zCf) * 64.0f);
        const auto _t12 = (zC60 * 4096.0f);
        const auto _t18 = ((gR21 * vib) * 8192.0f);
        const auto _t29 = (_t3 * _t15);
        const auto _t31 = (_t3 * _t4);
        const auto _t32 = (_t5 * _t6);
        const auto _t35 = (_t17 * _t6);
        const auto _t36 = (_t10 * _t6);
        const auto _t11 = (_t10 * _t36);
        const auto _t28 = (_t35 - _t29);
        const auto _t30 = (_t31 - _t32);
        const auto _t34 = ((_t2 * _t28) + (_t8 * _t14));
        const auto _t7 = (_t8 * _t9);
        const auto _t27 = ((_t8 * _t16) + (_t10 * _t28));
        const auto _t37 = ((_t11 - (_t2 * (_t32 - _t31))) - _t7);
        const auto vb = (-(((_t0 * ((_t1 * (((_t2 * _t30) - _t7) + _t11)) - (_t12 * _t13))) + (_t18 * _t19)) * _t21));
        const auto tC60 = (gC60 * (vb - 0));
        const auto vpt = (((_t18 * _t38) - ((_t0 * (((_t25 * _t27) + (_t12 * _t26)) + (_t1 * (_t24 * _t30)))) + (_t22 * (_t20 * _t27)))) * _t21);
        const auto vc = (((_t18 * _t39) - ((_t0 * (((_t25 * _t34) + (_t12 * _t33)) - (_t1 * (_t24 * _t36)))) + (_t22 * (_t20 * _t34)))) * _t21);
        const auto tCf = (gCf * (vpt - vc));
        const auto vl = ((((_t22 * (_t20 * _t37)) - (_t0 * ((_t12 * _t23) - (_t25 * _t37)))) + (_t18 * _t40)) * _t21);
        const auto tLf = (gLf * (vc - vl));
        if (_k == 0) {
            c0_tCf = tCf;
            c0_tLf = tLf;
            c0_vpt = vpt;
            c0_vc = vc;
            c0_vl = vl;
            c0_tC60 = tC60;
            c0_vb = vb;
        } else {
            c_tCf[_k - 1] = tCf - c0_tCf;
            c_tLf[_k - 1] = tLf - c0_tLf;
            c_vpt[_k - 1] = vpt - c0_vpt;
            c_vc[_k - 1] = vc - c0_vc;
            c_vl[_k - 1] = vl - c0_vl;
            c_tC60[_k - 1] = tC60 - c0_tC60;
            c_vb[_k - 1] = vb - c0_vb;
        }
    }
    
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zC60 = state[ch].zC60;
        auto zCf = state[ch].zCf;
        auto zLf = state[ch].zLf;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vib = input[ch][n];

            const auto tCf = c0_tCf + c_tCf[0] * vib + c_tCf[1] * zC60 + c_tCf[2] * zCf + c_tCf[3] * zLf;
            const auto tLf = c0_tLf + c_tLf[0] * vib + c_tLf[1] * zC60 + c_tLf[2] * zCf + c_tLf[3] * zLf;
            const auto vpt = c0_vpt + c_vpt[0] * vib + c_vpt[1] * zC60 + c_vpt[2] * zCf + c_vpt[3] * zLf;
            const auto vc = c0_vc + c_vc[0] * vib + c_vc[1] * zC60 + c_vc[2] * zCf + c_vc[3] * zLf;
            const auto vl = c0_vl + c_vl[0] * vib + c_vl[1] * zC60 + c_vl[2] * zCf + c_vl[3] * zLf;
            const auto tC60 = c0_tC60 + c_tC60[0] * vib + c_tC60[1] * zC60 + c_tC60[2] * zCf + c_tC60[3] * zLf;
            const auto vb = c0_vb + c_vb[0] * vib + c_vb[1] * zC60 + c_vb[2] * zCf + c_vb[3] * zLf;
            
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
    
    const auto _t2 = (((gC22 + gL1) + gR27C21) * 4.0f);
    const auto _t4 = (gC22 * 4.0f);
    const auto _t7 = ((gC22 + gR9) * 4.0f);
    const auto _t6 = ((_t4 * _t4) - (_t7 * _t2));
    const auto _t9 = (gRll + gR10);
    const auto _t10 = (1.0f / (gRll * _t6));
    const auto _t11 = (1.0f / _t6);
    float c0_tC22;
    float c_tC22[4];
    float c0_tL1;
    float c_tL1[4];
    
    for (int _k = 0; _k <= 4; ++_k)
    {
        const auto vb = (_k == 1) ? 1.0f : 0.0f;
        const auto zC22 = (_k == 2) ? 1.0f : 0.0f;
        const auto zL1 = (_k == 3) ? 1.0f : 0.0f;
        const auto zR27C21 = (_k == 4) ? 1.0f : 0.0f;
        const auto _t1 = (zC22 * 4.0f);
        const auto _t5 = (((zC22 - zL1) + ((gR27C21 * vb) - zR27C21)) * 4.0f);
        const auto _t8 = (_t1 * _t2);
        const auto _t3 = (_t4 * _t5);
        const auto _t0 = (_t8 - _t3);
        const auto vl2 = (((_t1 * _t4) - (_t7 * _t5)) * _t11);
        const auto vnh = (_t0 * _t11);
        const auto tC22 = (gC22 * (vl2 - vnh));
        const auto tL1 = (gL1 * (vl2 - 0));
        if (_k == 0) {
            c0_tC22 = tC22;
            c0_tL1 = tL1;
        } else {
            c_tC22[_k - 1] = tC22 - c0_tC22;
            c_tL1[_k - 1] = tL1 - c0_tL1;
        }
    }
    
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zC22 = state[ch].zC22;
        auto zL1 = state[ch].zL1;
        auto zR27C21 = state[ch].zR27C21;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vb = input[ch][n];

            const auto tC22 = c0_tC22 + c_tC22[0] * vb + c_tC22[1] * zC22 + c_tC22[2] * zL1 + c_tC22[3] * zR27C21;
            const auto tL1 = c0_tL1 + c_tL1[0] * vb + c_tL1[1] * zC22 + c_tL1[2] * zL1 + c_tL1[3] * zR27C21;
            const auto _t1 = (zC22 * 4.0f);
            const auto _t5 = (((zC22 - zL1) + ((gR27C21 * vb) - zR27C21)) * 4.0f);
            const auto _t8 = (_t1 * _t2);
            const auto _t3 = (_t4 * _t5);
            const auto _t0 = (_t8 - _t3);
            const auto vo = ((_t9 * _t0) * _t10);
            const auto vl2 = (((_t1 * _t4) - (_t7 * _t5)) * _t11);
            const auto vnh = (_t0 * _t11);
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
