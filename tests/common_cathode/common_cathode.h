// Auto-generated with netlist_codegen version a65dc78.
// Command: netlist_codegen common_cathode.net common_cathode.h -type_name double

#pragma once

#include <cmath>

struct Params {
    double VCC = 2.5e+02;
    double Cin = 2.0e-08;
    double Rg = 1.0e+06;
    double Cout = 2.0e-08;
    double RL = 1.0e+06;
    double Rp = 1.0e+05;
    double _12AX7_DEMPWOLF_Gk = 2.242e-03;
    double _12AX7_DEMPWOLF_Mu = 1.032e+02;
    double _12AX7_DEMPWOLF_Gamma = 1.26e+00;
    double _12AX7_DEMPWOLF_Ck = 3.4e+00;
    double _12AX7_DEMPWOLF_Gg = 6.177e-04;
    double _12AX7_DEMPWOLF_Xi = 1.314e+00;
    double _12AX7_DEMPWOLF_Cg = 9.901e+00;
    double _12AX7_DEMPWOLF_Ig0 = 8.025e-08;
    double Rk = 1.5e+03;
    double Ck = 2.5e-05;
};

struct State {
    double zCin {};
    double zCout {};
    double zRkCk {};
    double vPKX1 {};
    double vGKX1 {};
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
    
    const auto VCC = params.VCC;
    
    const auto vp = VCC;
    
    const auto gCin = 2.0 * sample_rate * params.Cin;
    
    const auto gRg = 1.0 / params.Rg;
    
    const auto gCout = 2.0 * sample_rate * params.Cout;
    
    const auto gRL = 1.0 / params.RL;
    
    const auto gRp = 1.0 / params.Rp;
    
    const auto Rk = params.Rk;
    const auto Ck = params.Ck;
    const auto gRkCk = 2.0 * sample_rate * Ck + (1.0 / Rk);
    const auto gzRkCk = 4.0 * sample_rate * Ck;
    
    const auto _t0 = (gCout + gRp);
    const auto _t6 = (gCout * gRkCk);
    const auto _t7 = (VCC * gRkCk);
    const auto _t10 = (gCout + gRL);
    const auto _t9 = (_t10 * gRkCk);
    const auto _t8 = (1.0 / ((gCout * _t6) - (_t0 * _t9)));
    const auto _t12 = (gRp * (gCout * _t7));
    const auto _t13 = (1.0 / (gCin + gRg));
    const auto _t14 = (gRp * (_t10 * _t7));
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zCin = state[ch].zCin;
        auto zCout = state[ch].zCout;
        auto zRkCk = state[ch].zRkCk;
        auto vPKX1 = state[ch].vPKX1;
        auto vGKX1 = state[ch].vGKX1;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            // --- Newton-Raphson solve: X1
            const auto _X1_t20 = (gCout + gRL);
            const auto _X1_t23 = (gCout * gRkCk);
            const auto _X1_t25 = (gCout + gRp);
            const auto _X1_t37 = (_12AX7_DEMPWOLF_Xi - 1.0);
            const auto _X1_t38 = (1.0 / (gCin + gRg));
            const auto _X1_t50 = (_12AX7_DEMPWOLF_Gamma - 1.0);
            const auto _X1_t19 = (_X1_t20 * gRkCk);
            const auto _X1_t22 = (gCout * _X1_t23);
            const auto _X1_t24 = (_X1_t25 * _X1_t19);
            const auto _X1_t21 = (1.0 / (_X1_t22 - _X1_t24));
            const auto _X1_t71 = (_12AX7_DEMPWOLF_Gk * _12AX7_DEMPWOLF_Gamma);
            const auto _X1_t72 = ((gCin * vi) - zCin);
            const auto _X1_t73 = (gCout * (zCout * gRkCk));
            const auto _X1_t74 = (gRp * (_X1_t20 * (VCC * gRkCk)));
            const auto _X1_t75 = (_12AX7_DEMPWOLF_Gg * _12AX7_DEMPWOLF_Xi);
            for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
            {
                const auto _X1_t10 = (vPKX1 / _12AX7_DEMPWOLF_Mu);
                const auto _X1_t18 = (_12AX7_DEMPWOLF_Cg * vGKX1);
                const auto _X1_t9 = (_X1_t10 + vGKX1);
                const auto _X1_t17 = exp(_X1_t18);
                const auto _X1_t8 = (_12AX7_DEMPWOLF_Ck * _X1_t9);
                const auto _X1_t16 = (1.0 + _X1_t17);
                const auto _X1_t7 = exp(_X1_t8);
                const auto _X1_t15 = log(_X1_t16);
                const auto _X1_t6 = (1.0 + _X1_t7);
                const auto _X1_t14 = (_X1_t15 / _12AX7_DEMPWOLF_Cg);
                const auto _X1_t36 = pow(_X1_t14, _X1_t37);
                const auto _X1_t54 = (_X1_t36 * _X1_t17);
                const auto _X1_t66 = (_12AX7_DEMPWOLF_Mu * _X1_t6);
                const auto _X1_t5 = log(_X1_t6);
                const auto _X1_t13 = pow(_X1_t14, _12AX7_DEMPWOLF_Xi);
                const auto _X1_t53 = (_12AX7_DEMPWOLF_Xi * _X1_t54);
                const auto _X1_t4 = (_X1_t5 / _12AX7_DEMPWOLF_Ck);
                const auto _X1_t12 = (_12AX7_DEMPWOLF_Gg * _X1_t13);
                const auto _X1_t49 = pow(_X1_t4, _X1_t50);
                const auto _X1_t52 = (_12AX7_DEMPWOLF_Gg * _X1_t53);
                const auto _X1_t3 = pow(_X1_t4, _12AX7_DEMPWOLF_Gamma);
                const auto _X1_t11 = (_X1_t12 + _12AX7_DEMPWOLF_Ig0);
                const auto _X1_t48 = (_X1_t49 * _X1_t7);
                const auto _X1_t51 = (_X1_t52 / _X1_t16);
                const auto _X1_t2 = (_12AX7_DEMPWOLF_Gk * _X1_t3);
                const auto _X1_t47 = (_12AX7_DEMPWOLF_Gamma * _X1_t48);
                const auto _X1_t1 = (_X1_t2 - _X1_t11);
                const auto _X1_t31 = (zRkCk + _X1_t1);
                const auto _X1_t46 = (_12AX7_DEMPWOLF_Gk * _X1_t47);
                const auto _X1_t65 = (gCout * _X1_t46);
                const auto _X1_t69 = (_X1_t20 * _X1_t46);
                const auto _X1_t30 = (_X1_t31 + _X1_t11);
                const auto _X1_t33 = (_X1_t20 * _X1_t30);
                const auto _X1_t45 = (_X1_t46 / _X1_t6);
                const auto _X1_t64 = (gCout * _X1_t65);
                const auto _X1_t68 = (_X1_t25 * _X1_t69);
                const auto _X1_t29 = (gCout * _X1_t30);
                const auto _X1_t32 = (_X1_t25 * _X1_t33);
                const auto _X1_t44 = (_X1_t45 - _X1_t51);
                const auto _X1_t63 = (_X1_t64 / _X1_t66);
                const auto _X1_t67 = (_X1_t68 / _X1_t66);
                const auto _X1_t28 = (gCout * _X1_t29);
                const auto _X1_t43 = (_X1_t44 + _X1_t51);
                const auto _X1_t56 = (_X1_t20 * _X1_t43);
                const auto _X1_t62 = (_X1_t63 - _X1_t67);
                const auto _X1_t27 = (_X1_t28 - _X1_t32);
                const auto _X1_t42 = (gCout * _X1_t43);
                const auto _X1_t55 = (_X1_t25 * _X1_t56);
                const auto _X1_t61 = (_X1_t62 * _X1_t21);
                const auto _X1_t70 = ((((((((_X1_t71 * _X1_t49) * _X1_t7) * _X1_t20) * gRkCk) / _X1_t66) * _X1_t21) - _X1_t61) - 1.0);
                const auto _X1_t26 = (_X1_t27 * _X1_t21);
                const auto _X1_t41 = (gCout * _X1_t42);
                const auto _X1_t59 = (((_X1_t11 - _X1_t72) * _X1_t38) + _X1_t26);
                const auto _X1_t0 = ((((((_X1_t1 - zCout) * _X1_t19) + _X1_t73) - _X1_t74) * _X1_t21) - _X1_t26);
                const auto _X1_t34 = (vPKX1 - _X1_t0);
                const auto _X1_t40 = (_X1_t41 - _X1_t55);
                const auto _X1_t58 = (_X1_t59 + vGKX1);
                const auto _X1_t39 = (_X1_t40 * _X1_t21);
                const auto _X1_t57 = ((((_X1_t44 * _X1_t20) * gRkCk) * _X1_t21) - _X1_t39);
                const auto _X1_t35 = ((((((_X1_t75 * _X1_t36) * _X1_t17) / _X1_t16) * _X1_t38) + _X1_t39) + 1.0);
                const auto _X1_t60 = ((_X1_t57 * _X1_t61) - (_X1_t70 * _X1_t35));
                const auto res_vPKX1 = (_X1_t0 - vPKX1);
                const auto delta_vPKX1 = (-(((_X1_t34 * _X1_t35) + (_X1_t57 * _X1_t58)) / _X1_t60));
                const auto res_vGKX1 = (-_X1_t58);
                const auto delta_vGKX1 = (((_X1_t70 * _X1_t58) + (_X1_t34 * _X1_t61)) / _X1_t60);
            
                auto residual_norm_sq = 0.0;
                residual_norm_sq += res_vPKX1 * res_vPKX1;
                residual_norm_sq += res_vGKX1 * res_vGKX1;
                auto step_norm_sq = 0.0;
                step_norm_sq += delta_vPKX1 * delta_vPKX1;
                step_norm_sq += delta_vGKX1 * delta_vGKX1;
            
                vPKX1 = vPKX1 + (delta_vPKX1);
                vGKX1 = vGKX1 + (delta_vGKX1);
            
                if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
                    break;
                
            }

            const auto _t1 = (zCout * gRkCk);
            const auto _t5 = (_12AX7_DEMPWOLF_Gg * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi));
            const auto _t4 = (_t5 + _12AX7_DEMPWOLF_Ig0);
            const auto _t3 = ((_12AX7_DEMPWOLF_Gk * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - _t4);
            const auto _t11 = ((zRkCk + _t3) + _t4);
            const auto _t2 = (_t3 - zCout);
            const auto vo = ((((_t0 * _t1) + (_t2 * _t6)) - _t12) * _t8);
            const auto vg = (-((_t4 - ((gCin * vi) - zCin)) * _t13));
            const auto tCin = (gCin * (vi - vg));
            const auto vpl = ((((_t2 * _t9) + (gCout * _t1)) - _t14) * _t8);
            const auto tCout = (gCout * (vpl - vo));
            const auto vk = (((gCout * (gCout * _t11)) - (_t0 * (_t10 * _t11))) * _t8);
            const auto vRkCk = (vk - 0);
            
            zCin = 2 * tCin - zCin;
            zCout = 2 * tCout - zCout;
            zRkCk = gzRkCk * vRkCk - zRkCk; // RC parallel

            output[ch][n] = vo;
        }
        state[ch].zCin = zCin;
        state[ch].zCout = zCout;
        state[ch].zRkCk = zRkCk;
        state[ch].vPKX1 = vPKX1;
        state[ch].vGKX1 = vGKX1;
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
    
    const auto VCC = params.VCC;
    
    const auto vp = VCC;
    
    const auto gCin = 2.0 * sample_rate * params.Cin;
    
    const auto gRg = 1.0 / params.Rg;
    
    const auto gCout = 2.0 * sample_rate * params.Cout;
    
    const auto gRL = 1.0 / params.RL;
    
    const auto gRp = 1.0 / params.Rp;
    
    const auto Rk = params.Rk;
    const auto Ck = params.Ck;
    const auto gRkCk = 2.0 * sample_rate * Ck + (1.0 / Rk);
    const auto gzRkCk = 4.0 * sample_rate * Ck;
    
    const auto vi = vi_dc;

    double vPKX1 = 0;
    double vGKX1 = 0;

    // --- Newton-Raphson solve: X1
    const auto _X1_t21 = (1.0 / 1000000000.0);
    const auto _X1_t25 = (1.0 / Rk);
    const auto _X1_t33 = (_12AX7_DEMPWOLF_Gamma - 1.0);
    const auto _X1_t39 = (_12AX7_DEMPWOLF_Xi - 1.0);
    const auto _X1_t44 = (1.0 / (gRg + _X1_t21));
    const auto _X1_t51 = (_12AX7_DEMPWOLF_Gk * _12AX7_DEMPWOLF_Gamma);
    const auto _X1_t20 = (1.0 / (gRp + _X1_t21));
    const auto _X1_t24 = (1.0 / (_X1_t25 + _X1_t21));
    const auto _X1_t54 = (gRp * VCC);
    const auto _X1_t55 = (_12AX7_DEMPWOLF_Gg * _12AX7_DEMPWOLF_Xi);
    for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
    {
        const auto _X1_t11 = (vPKX1 / _12AX7_DEMPWOLF_Mu);
        const auto _X1_t19 = (_12AX7_DEMPWOLF_Cg * vGKX1);
        const auto _X1_t10 = (_X1_t11 + vGKX1);
        const auto _X1_t18 = exp(_X1_t19);
        const auto _X1_t9 = (_12AX7_DEMPWOLF_Ck * _X1_t10);
        const auto _X1_t17 = (1.0 + _X1_t18);
        const auto _X1_t8 = exp(_X1_t9);
        const auto _X1_t16 = log(_X1_t17);
        const auto _X1_t7 = (1.0 + _X1_t8);
        const auto _X1_t15 = (_X1_t16 / _12AX7_DEMPWOLF_Cg);
        const auto _X1_t38 = pow(_X1_t15, _X1_t39);
        const auto _X1_t52 = (_12AX7_DEMPWOLF_Mu * _X1_t7);
        const auto _X1_t6 = log(_X1_t7);
        const auto _X1_t14 = pow(_X1_t15, _12AX7_DEMPWOLF_Xi);
        const auto _X1_t37 = (_X1_t38 * _X1_t18);
        const auto _X1_t5 = (_X1_t6 / _12AX7_DEMPWOLF_Ck);
        const auto _X1_t13 = (_12AX7_DEMPWOLF_Gg * _X1_t14);
        const auto _X1_t32 = pow(_X1_t5, _X1_t33);
        const auto _X1_t36 = (_12AX7_DEMPWOLF_Xi * _X1_t37);
        const auto _X1_t50 = (_X1_t51 * _X1_t32);
        const auto _X1_t4 = pow(_X1_t5, _12AX7_DEMPWOLF_Gamma);
        const auto _X1_t12 = (_X1_t13 + _12AX7_DEMPWOLF_Ig0);
        const auto _X1_t31 = (_X1_t32 * _X1_t8);
        const auto _X1_t35 = (_12AX7_DEMPWOLF_Gg * _X1_t36);
        const auto _X1_t49 = (_X1_t50 * _X1_t8);
        const auto _X1_t3 = (_12AX7_DEMPWOLF_Gk * _X1_t4);
        const auto _X1_t30 = (_12AX7_DEMPWOLF_Gamma * _X1_t31);
        const auto _X1_t34 = (_X1_t35 / _X1_t17);
        const auto _X1_t48 = (_X1_t49 / _X1_t52);
        const auto _X1_t53 = (_X1_t48 * _X1_t24);
        const auto _X1_t2 = (_X1_t3 - _X1_t12);
        const auto _X1_t23 = (_X1_t2 + _X1_t12);
        const auto _X1_t29 = (_12AX7_DEMPWOLF_Gk * _X1_t30);
        const auto _X1_t47 = (((_X1_t48 * _X1_t20) + _X1_t53) + 1.0);
        const auto _X1_t22 = (_X1_t23 * _X1_t24);
        const auto _X1_t28 = (_X1_t29 / _X1_t7);
        const auto _X1_t43 = ((_X1_t12 * _X1_t44) + _X1_t22);
        const auto _X1_t1 = (((_X1_t2 - _X1_t54) * _X1_t20) + _X1_t22);
        const auto _X1_t27 = (_X1_t28 - _X1_t34);
        const auto _X1_t41 = (_X1_t27 + _X1_t34);
        const auto _X1_t42 = (_X1_t43 + vGKX1);
        const auto _X1_t0 = (_X1_t1 + vPKX1);
        const auto _X1_t40 = (_X1_t41 * _X1_t24);
        const auto _X1_t45 = ((((((_X1_t55 * _X1_t38) * _X1_t18) / _X1_t17) * _X1_t44) + _X1_t40) + 1.0);
        const auto _X1_t26 = ((_X1_t27 * _X1_t20) + _X1_t40);
        const auto _X1_t46 = ((_X1_t47 * _X1_t45) - (_X1_t26 * _X1_t53));
        const auto res_vPKX1 = (-_X1_t0);
        const auto delta_vPKX1 = (((_X1_t26 * _X1_t42) - (_X1_t0 * _X1_t45)) / _X1_t46);
        const auto res_vGKX1 = (-_X1_t42);
        const auto delta_vGKX1 = (((_X1_t0 * _X1_t53) - (_X1_t47 * _X1_t42)) / _X1_t46);
    
        auto residual_norm_sq = 0.0;
        residual_norm_sq += res_vPKX1 * res_vPKX1;
        residual_norm_sq += res_vGKX1 * res_vGKX1;
        auto step_norm_sq = 0.0;
        step_norm_sq += delta_vPKX1 * delta_vPKX1;
        step_norm_sq += delta_vGKX1 * delta_vGKX1;
    
        vPKX1 = vPKX1 + (delta_vPKX1);
        vGKX1 = vGKX1 + (delta_vGKX1);
    
        if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
            break;
        
    }
    const auto zCin = (gCin * (vi + (((_12AX7_DEMPWOLF_Gg * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0) / (gRg + (1.0 / 1000000000.0)))));
    const auto zCout = (-((gCout * (((_12AX7_DEMPWOLF_Gk * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) - (gRp * VCC))) / (gRp + (1.0 / 1000000000.0))));
    const auto zRkCk = (((gzRkCk * (((_12AX7_DEMPWOLF_Gk * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Ck * ((vPKX1 / _12AX7_DEMPWOLF_Mu) + vGKX1))))) / _12AX7_DEMPWOLF_Ck), _12AX7_DEMPWOLF_Gamma)) - ((_12AX7_DEMPWOLF_Gg * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0)) + ((_12AX7_DEMPWOLF_Gg * pow((log((1.0 + exp((_12AX7_DEMPWOLF_Cg * vGKX1)))) / _12AX7_DEMPWOLF_Cg), _12AX7_DEMPWOLF_Xi)) + _12AX7_DEMPWOLF_Ig0))) / ((1.0 / Rk) + (1.0 / 1000000000.0))) / 2.0);

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].vPKX1 = vPKX1;
        state[ch].vGKX1 = vGKX1;
        state[ch].zCin = zCin;
        state[ch].zCout = zCout;
        state[ch].zRkCk = zRkCk;
    }
}
