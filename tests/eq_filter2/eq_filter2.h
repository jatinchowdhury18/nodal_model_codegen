// Auto-generated with netlist_codegen version d0fcc0c.
// Command: netlist_codegen eq_filter2.net eq_filter2.h -opt_port_matrix -type_name double

#pragma once

struct Params {
    double R13 = 4.75e+03;
    double R50 = 4.75e+03;
    double R23 = 7.5e+02;
    double Rboostm = 1.0e+04;
    double R11 = 1.0e+03;
    double R12 = 4.75e+03;
    double C8 = 3.3e-10;
    double R40 = 4.7e+03;
    double R17 = 1.8e+03;
    double R16 = 1.0e+03;
    double Rboostp = 1.5e+04;
    double R62 = 1.0e+05;
    double R24 = 7.5e+02;
    double Rq2 = 8.0e+03;
    double Rq1m = 8.0e+03;
    double R64 = 1.2e+04;
    double Rq1p = 2.0e+03;
    double Rf1 = 5.0e+04;
    double R18 = 4.3e+03;
    double C9 = 4.7e-09;
    double R20 = 4.7e+02;
    double C11 = 1.0e-04;
    double R22 = 9.1e+02;
    double R21 = 1.8e+03;
    double R19 = 5.1e+03;
    double Rf2 = 5.0e+04;
    double C10 = 4.7e-09;
};

struct State {
    double zR12C8 {};
    double zRf1R18C9 {};
    double zC11 {};
    double zR19Rf2C10 {};
};

static void compute (const float* const* input, float** output, int num_channels, int num_samples, Params params, State* state, float sample_rate)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto gR13 = 1.0 / params.R13;
    
    const auto gR50 = 1.0 / params.R50;
    
    const auto gR23 = 1.0 / params.R23;
    
    const auto RboostmR11 = sum(params.Rboostm, params.R11);
    const auto gRboostmR11 = 1.0 / RboostmR11;
    
    const auto R12 = params.R12;
    const auto C8 = params.C8;
    const auto gR12C8 = 2.0 * sample_rate * C8 + (1.0 / R12);
    const auto gzR12C8 = 4.0 * sample_rate * C8;
    
    const auto R40R17 = recip_sum(params.R40, params.R17);
    const auto gR40R17 = 1.0 / R40R17;
    
    const auto R16Rboostp = sum(params.R16, params.Rboostp);
    const auto gR16Rboostp = 1.0 / R16Rboostp;
    
    const auto gR62 = 1.0 / params.R62;
    
    const auto R24Rq2 = sum(params.R24, params.Rq2);
    const auto gR24Rq2 = 1.0 / R24Rq2;
    
    const auto gRq1m = 1.0 / params.Rq1m;
    
    const auto gR64 = 1.0 / params.R64;
    
    const auto gRq1p = 1.0 / params.Rq1p;
    
    const auto Rf1R18 = sum(params.Rf1, params.R18);
    const auto C9 = params.C9;
    const auto gRf1R18C9 = (2.0 * sample_rate * C9) / (1.0 + 2.0 * sample_rate * Rf1R18 * C9);
    const auto gnRf1R18C9 = gRf1R18C9 * (2.0 * sample_rate * Rf1R18 * C9 - 1.0) / (2.0 * sample_rate * C9);
    
    const auto gR20 = 1.0 / params.R20;
    
    const auto gC11 = 2.0 * sample_rate * params.C11;
    
    const auto gR22 = 1.0 / params.R22;
    
    const auto gR21 = 1.0 / params.R21;
    
    const auto R19Rf2 = sum(params.R19, params.Rf2);
    const auto C10 = params.C10;
    const auto gR19Rf2C10 = 2.0 * sample_rate * C10 + (1.0 / R19Rf2);
    const auto gzR19Rf2C10 = 4.0 * sample_rate * C10;
    
    const auto _t1 = ((((gRboostmR11 + gR40R17) + gR16Rboostp) + gR64) + gRf1R18C9);
    const auto _t4 = ((gR62 + gRq1m) + gRq1p);
    const auto _t5 = ((gRq1m + gR20) + gR21);
    const auto _t6 = ((gRq1p + gR22) + gR21);
    const auto _t17 = (_t5 * gR22);
    const auto _t19 = ((gR23 + gC11) + gR22);
    const auto _t22 = ((gRq1m * (gRq1m * gR22)) - (_t4 * _t17));
    const auto _t31 = (gRf1R18C9 * _t22);
    const auto _t3 = (((_t4 * ((_t5 * _t6) - (gR21 * gR21))) - (gRq1m * ((gRq1m * _t6) + (gR21 * gRq1p)))) - (gRq1p * ((gRq1m * gR21) + (_t5 * gRq1p))));
    const auto _t11 = (gRf1R18C9 * _t3);
    const auto _t16 = ((gRq1m * (gR21 * gR22)) + (gRq1p * _t17));
    const auto _t18 = (gRf1R18C9 * _t16);
    const auto _t21 = (gR19Rf2C10 * _t22);
    const auto _t25 = (gR16Rboostp * _t11);
    const auto _t28 = (gR19Rf2C10 * _t16);
    const auto _t32 = (gR24Rq2 * _t25);
    const auto _t2 = (gR19Rf2C10 * _t3);
    const auto _t15 = ((gRf1R18C9 + gR19Rf2C10) * _t16);
    const auto _t24 = (gR23 * _t25);
    const auto _t27 = (gR16Rboostp * _t2);
    const auto _t0 = (_t1 * _t2);
    const auto _t14 = ((gR24Rq2 * ((_t1 * _t15) - (gRf1R18C9 * _t18))) + (gR23 * (gR64 * _t11)));
    const auto _t20 = (gR24Rq2 * _t0);
    const auto _t13 = (((gC11 * _t14) - (_t19 * _t20)) - (gR22 * (gR24Rq2 * (_t1 * _t21))));
    const auto _t23 = (1.0 / ((gR12C8 * _t13) - (gR13 * (gC11 * _t24))));
    const auto _t33 = (gR23 * _t0);
    const auto _t34 = ((gR23 * (gR64 * _t2)) - (gR24Rq2 * (gRf1R18C9 * _t28)));
    const auto _t35 = (gR23 * _t27);
    const auto _t36 = (((gC11 * (gR24Rq2 * (gR16Rboostp * _t15))) - (_t19 * (gR24Rq2 * _t27))) - (gR22 * (gR24Rq2 * (gR16Rboostp * _t21))));
    const auto _t37 = (gC11 * (gR24Rq2 * (gR16Rboostp * _t18)));
    const auto _t38 = (gR24Rq2 * (_t1 * _t28));
    const auto _t39 = ((_t19 * _t32) + (gR22 * (gR24Rq2 * (gR16Rboostp * _t31))));
    const auto _t40 = (gC11 * _t32);
    double c0_vRf1R18C9;
    double c_vRf1R18C9[5];
    double c0_vR19Rf2C10;
    double c_vR19Rf2C10[5];
    double c0_tC11;
    double c_tC11[5];
    double c0_v13;
    double c_v13[5];
    double c0_v15;
    double c_v15[5];
    double c0_vR12C8;
    double c_vR12C8[5];
    double c0_vo;
    double c_vo[5];
    double c0_vn4;
    double c_vn4[5];
    
    for (int _k = 0; _k <= 5; ++_k)
    {
        const auto vi = (_k == 1) ? 1.0 : 0.0;
        const auto zR12C8 = (_k == 2) ? 1.0 : 0.0;
        const auto zRf1R18C9 = (_k == 3) ? 1.0 : 0.0;
        const auto zC11 = (_k == 4) ? 1.0 : 0.0;
        const auto zR19Rf2C10 = (_k == 5) ? 1.0 : 0.0;
        const auto _t9 = (zRf1R18C9 - zR19Rf2C10);
        const auto _t10 = ((gRboostmR11 * vi) + zRf1R18C9);
        const auto _t12 = (zR12C8 - (gR50 * vi));
        const auto _t8 = (_t9 * _t3);
        const auto _t26 = (_t9 * _t16);
        const auto _t29 = (_t1 * _t8);
        const auto _t30 = (_t10 * _t11);
        const auto _t7 = (_t29 - _t30);
        const auto vo = (((gR13 * ((zC11 * _t33) - (gC11 * (gR23 * _t7)))) + (_t12 * _t13)) * _t23);
        const auto vR12C8 = (vo - 0);
        const auto v13 = (((gR12C8 * (((gC11 * ((gR24Rq2 * ((_t10 * _t15) - (gRf1R18C9 * _t26))) + (gR23 * (gR64 * _t8)))) - ((_t19 * (gR24Rq2 * (_t10 * _t2))) + (zC11 * _t34))) - (gR22 * (gR24Rq2 * (_t10 * _t21))))) - ((gR13 * ((gC11 * (gR23 * (gR16Rboostp * _t8))) - (zC11 * _t35))) - (_t12 * _t36))) * _t23);
        const auto vn4 = (((_t12 * _t37) + (gR12C8 * ((gC11 * (gR24Rq2 * ((_t10 * _t18) - (_t1 * _t26)))) + (zC11 * _t38)))) * _t23);
        const auto vRf1R18C9 = (v13 - vn4);
        const auto v15 = (((gR12C8 * (((zC11 * _t14) - (_t19 * (gR24Rq2 * _t7))) + (gR22 * (gR24Rq2 * ((_t10 * _t31) - (_t1 * (_t9 * _t22))))))) - ((gR13 * (zC11 * _t24)) - (_t12 * _t39))) * _t23);
        const auto vc11 = (((_t12 * _t40) + (gR12C8 * ((zC11 * _t20) + (gC11 * (gR24Rq2 * (_t30 - _t29)))))) * _t23);
        const auto tC11 = (gC11 * (v15 - vc11));
        const auto vR19Rf2C10 = (vn4 - v15);
        if (_k == 0) {
            c0_vRf1R18C9 = vRf1R18C9;
            c0_vR19Rf2C10 = vR19Rf2C10;
            c0_tC11 = tC11;
            c0_v13 = v13;
            c0_v15 = v15;
            c0_vR12C8 = vR12C8;
            c0_vo = vo;
            c0_vn4 = vn4;
        } else {
            c_vRf1R18C9[_k - 1] = vRf1R18C9 - c0_vRf1R18C9;
            c_vR19Rf2C10[_k - 1] = vR19Rf2C10 - c0_vR19Rf2C10;
            c_tC11[_k - 1] = tC11 - c0_tC11;
            c_v13[_k - 1] = v13 - c0_v13;
            c_v15[_k - 1] = v15 - c0_v15;
            c_vR12C8[_k - 1] = vR12C8 - c0_vR12C8;
            c_vo[_k - 1] = vo - c0_vo;
            c_vn4[_k - 1] = vn4 - c0_vn4;
        }
    }
    
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zR12C8 = state[ch].zR12C8;
        auto zRf1R18C9 = state[ch].zRf1R18C9;
        auto zC11 = state[ch].zC11;
        auto zR19Rf2C10 = state[ch].zR19Rf2C10;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            const auto vRf1R18C9 = c0_vRf1R18C9 + c_vRf1R18C9[0] * vi + c_vRf1R18C9[1] * zR12C8 + c_vRf1R18C9[2] * zRf1R18C9 + c_vRf1R18C9[3] * zC11 + c_vRf1R18C9[4] * zR19Rf2C10;
            const auto vR19Rf2C10 = c0_vR19Rf2C10 + c_vR19Rf2C10[0] * vi + c_vR19Rf2C10[1] * zR12C8 + c_vR19Rf2C10[2] * zRf1R18C9 + c_vR19Rf2C10[3] * zC11 + c_vR19Rf2C10[4] * zR19Rf2C10;
            const auto tC11 = c0_tC11 + c_tC11[0] * vi + c_tC11[1] * zR12C8 + c_tC11[2] * zRf1R18C9 + c_tC11[3] * zC11 + c_tC11[4] * zR19Rf2C10;
            const auto v13 = c0_v13 + c_v13[0] * vi + c_v13[1] * zR12C8 + c_v13[2] * zRf1R18C9 + c_v13[3] * zC11 + c_v13[4] * zR19Rf2C10;
            const auto v15 = c0_v15 + c_v15[0] * vi + c_v15[1] * zR12C8 + c_v15[2] * zRf1R18C9 + c_v15[3] * zC11 + c_v15[4] * zR19Rf2C10;
            const auto vR12C8 = c0_vR12C8 + c_vR12C8[0] * vi + c_vR12C8[1] * zR12C8 + c_vR12C8[2] * zRf1R18C9 + c_vR12C8[3] * zC11 + c_vR12C8[4] * zR19Rf2C10;
            const auto vo = c0_vo + c_vo[0] * vi + c_vo[1] * zR12C8 + c_vo[2] * zRf1R18C9 + c_vo[3] * zC11 + c_vo[4] * zR19Rf2C10;
            const auto vn4 = c0_vn4 + c_vn4[0] * vi + c_vn4[1] * zR12C8 + c_vn4[2] * zRf1R18C9 + c_vn4[3] * zC11 + c_vn4[4] * zR19Rf2C10;
            const auto _t9 = (zRf1R18C9 - zR19Rf2C10);
            const auto _t10 = ((gRboostmR11 * vi) + zRf1R18C9);
            const auto _t12 = (zR12C8 - (gR50 * vi));
            const auto _t8 = (_t9 * _t3);
            const auto _t29 = (_t1 * _t8);
            const auto _t30 = (_t10 * _t11);
            const auto vc11 = (((_t12 * _t40) + (gR12C8 * ((zC11 * _t20) + (gC11 * (gR24Rq2 * (_t30 - _t29)))))) * _t23);
            
            zR12C8 = gzR12C8 * vR12C8 - zR12C8; // RC parallel
            zRf1R18C9 = gRf1R18C9 * (1 - gnRf1R18C9) * vRf1R18C9 + gnRf1R18C9 * zRf1R18C9; // RC series
            zC11 = 2 * tC11 - zC11;
            zR19Rf2C10 = gzR19Rf2C10 * vR19Rf2C10 - zR19Rf2C10; // RC parallel

            output[ch][n] = vo;
        }
        state[ch].zR12C8 = zR12C8;
        state[ch].zRf1R18C9 = zRf1R18C9;
        state[ch].zC11 = zC11;
        state[ch].zR19Rf2C10 = zR19Rf2C10;
    }
}

static float reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc = 0.0f)
{
    [[maybe_unused]] static constexpr auto sum = [](auto a, auto b) { return a + b; };
    [[maybe_unused]] static constexpr auto recip_sum = [](auto a, auto b) { return a * b / (a + b); };
    
    const auto gR13 = 1.0 / params.R13;
    
    const auto gR50 = 1.0 / params.R50;
    
    const auto gR23 = 1.0 / params.R23;
    
    const auto RboostmR11 = sum(params.Rboostm, params.R11);
    const auto gRboostmR11 = 1.0 / RboostmR11;
    
    const auto R12 = params.R12;
    const auto C8 = params.C8;
    const auto gR12C8 = 2.0 * sample_rate * C8 + (1.0 / R12);
    const auto gzR12C8 = 4.0 * sample_rate * C8;
    
    const auto R40R17 = recip_sum(params.R40, params.R17);
    const auto gR40R17 = 1.0 / R40R17;
    
    const auto R16Rboostp = sum(params.R16, params.Rboostp);
    const auto gR16Rboostp = 1.0 / R16Rboostp;
    
    const auto gR62 = 1.0 / params.R62;
    
    const auto R24Rq2 = sum(params.R24, params.Rq2);
    const auto gR24Rq2 = 1.0 / R24Rq2;
    
    const auto gRq1m = 1.0 / params.Rq1m;
    
    const auto gR64 = 1.0 / params.R64;
    
    const auto gRq1p = 1.0 / params.Rq1p;
    
    const auto Rf1R18 = sum(params.Rf1, params.R18);
    const auto C9 = params.C9;
    const auto gRf1R18C9 = (2.0 * sample_rate * C9) / (1.0 + 2.0 * sample_rate * Rf1R18 * C9);
    const auto gnRf1R18C9 = gRf1R18C9 * (2.0 * sample_rate * Rf1R18 * C9 - 1.0) / (2.0 * sample_rate * C9);
    
    const auto gR20 = 1.0 / params.R20;
    
    const auto gC11 = 2.0 * sample_rate * params.C11;
    
    const auto gR22 = 1.0 / params.R22;
    
    const auto gR21 = 1.0 / params.R21;
    
    const auto R19Rf2 = sum(params.R19, params.Rf2);
    const auto C10 = params.C10;
    const auto gR19Rf2C10 = 2.0 * sample_rate * C10 + (1.0 / R19Rf2);
    const auto gzR19Rf2C10 = 4.0 * sample_rate * C10;
    
    const auto vi = vi_dc;

    const auto zR12C8 = (-((((gzR12C8 * gR50) * vi) * R12) / 2.0));
    const auto zRf1R18C9 = (-((gRf1R18C9 * ((((gR50 * vi) * (((((gR23 + gR22) + (1.0 / 1000000000.0)) * (gR24Rq2 * (gR16Rboostp * (((((gR62 + gRq1m) + gRq1p) * (((((gRq1m + gR20) + gR21) + (1.0 / 1000000000.0)) * (((gRq1p + gR22) + gR21) + (1.0 / 1000000000.0))) - (gR21 * gR21))) - (gRq1m * ((gRq1m * (((gRq1p + gR22) + gR21) + (1.0 / 1000000000.0))) + (gR21 * gRq1p)))) - (gRq1p * ((gRq1m * gR21) + ((((gRq1m + gR20) + gR21) + (1.0 / 1000000000.0)) * gRq1p))))))) / R19Rf2) + ((gR22 * (gR24Rq2 * (gR16Rboostp * ((gRq1m * (gRq1m * gR22)) - (((gR62 + gRq1m) + gRq1p) * ((((gRq1m + gR20) + gR21) + (1.0 / 1000000000.0)) * gR22)))))) / R19Rf2))) - ((((((gR23 + gR22) + (1.0 / 1000000000.0)) * (gR24Rq2 * ((gRboostmR11 * vi) * (((((gR62 + gRq1m) + gRq1p) * (((((gRq1m + gR20) + gR21) + (1.0 / 1000000000.0)) * (((gRq1p + gR22) + gR21) + (1.0 / 1000000000.0))) - (gR21 * gR21))) - (gRq1m * ((gRq1m * (((gRq1p + gR22) + gR21) + (1.0 / 1000000000.0))) + (gR21 * gRq1p)))) - (gRq1p * ((gRq1m * gR21) + ((((gRq1m + gR20) + gR21) + (1.0 / 1000000000.0)) * gRq1p))))))) / R19Rf2) + ((gR22 * (gR24Rq2 * ((gRboostmR11 * vi) * ((gRq1m * (gRq1m * gR22)) - (((gR62 + gRq1m) + gRq1p) * ((((gRq1m + gR20) + gR21) + (1.0 / 1000000000.0)) * gR22)))))) / R19Rf2)) / R12)) * R12)) / (((((gR23 + gR22) + (1.0 / 1000000000.0)) * (gR24Rq2 * (((((gRboostmR11 + gR40R17) + gR16Rboostp) + gR64) + (1.0 / 1000000000.0)) * (((((gR62 + gRq1m) + gRq1p) * (((((gRq1m + gR20) + gR21) + (1.0 / 1000000000.0)) * (((gRq1p + gR22) + gR21) + (1.0 / 1000000000.0))) - (gR21 * gR21))) - (gRq1m * ((gRq1m * (((gRq1p + gR22) + gR21) + (1.0 / 1000000000.0))) + (gR21 * gRq1p)))) - (gRq1p * ((gRq1m * gR21) + ((((gRq1m + gR20) + gR21) + (1.0 / 1000000000.0)) * gRq1p))))))) / R19Rf2) + ((gR22 * (gR24Rq2 * (((((gRboostmR11 + gR40R17) + gR16Rboostp) + gR64) + (1.0 / 1000000000.0)) * ((gRq1m * (gRq1m * gR22)) - (((gR62 + gRq1m) + gRq1p) * ((((gRq1m + gR20) + gR21) + (1.0 / 1000000000.0)) * gR22)))))) / R19Rf2))));
    const auto zC11 = 0.0;
    const auto zR19Rf2C10 = 0.0;

    const auto vo_dc_out = (-((gR50 * vi) * R12));

    for (int ch = 0; ch < num_channels; ++ch)
    {
        state[ch].zR12C8 = zR12C8;
        state[ch].zRf1R18C9 = zRf1R18C9;
        state[ch].zC11 = zC11;
        state[ch].zR19Rf2C10 = zR19Rf2C10;
    }
    return vo_dc_out;
}

