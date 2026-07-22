// Auto-generated with netlist_codegen version 25c9320.
// Command: netlist_codegen eq_filter2.net eq_filter2.h -type_name double

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
    const auto _t19 = (_t5 * gR22);
    const auto _t21 = ((gR23 + gC11) + gR22);
    const auto _t24 = ((gRq1m * (gRq1m * gR22)) - (_t4 * _t19));
    const auto _t31 = (gRf1R18C9 * _t24);
    const auto _t3 = (((_t4 * ((_t5 * _t6) - (gR21 * gR21))) - (gRq1m * ((gRq1m * _t6) + (gR21 * gRq1p)))) - (gRq1p * ((gRq1m * gR21) + (_t5 * gRq1p))));
    const auto _t13 = (gRf1R18C9 * _t3);
    const auto _t18 = ((gRq1m * (gR21 * gR22)) + (gRq1p * _t19));
    const auto _t20 = (gRf1R18C9 * _t18);
    const auto _t23 = (gR19Rf2C10 * _t24);
    const auto _t27 = (gR16Rboostp * _t13);
    const auto _t29 = (gR19Rf2C10 * _t18);
    const auto _t32 = (gR24Rq2 * _t27);
    const auto _t2 = (gR19Rf2C10 * _t3);
    const auto _t17 = ((gRf1R18C9 + gR19Rf2C10) * _t18);
    const auto _t26 = (gR23 * _t27);
    const auto _t30 = (gR16Rboostp * _t2);
    const auto _t0 = (_t1 * _t2);
    const auto _t16 = ((gR24Rq2 * ((_t1 * _t17) - (gRf1R18C9 * _t20))) + (gR23 * (gR64 * _t13)));
    const auto _t22 = (gR24Rq2 * _t0);
    const auto _t15 = (((gC11 * _t16) - (_t21 * _t22)) - (gR22 * (gR24Rq2 * (_t1 * _t23))));
    const auto _t25 = (1.0 / ((gR12C8 * _t15) - (gR13 * (gC11 * _t26))));
    const auto _t33 = (gR23 * _t0);
    const auto _t34 = ((gR23 * (gR64 * _t2)) - (gR24Rq2 * (gRf1R18C9 * _t29)));
    const auto _t35 = (gR23 * _t30);
    const auto _t36 = (((gC11 * (gR24Rq2 * (gR16Rboostp * _t17))) - (_t21 * (gR24Rq2 * _t30))) - (gR22 * (gR24Rq2 * (gR16Rboostp * _t23))));
    const auto _t37 = (gC11 * (gR24Rq2 * (gR16Rboostp * _t20)));
    const auto _t38 = (gR24Rq2 * (_t1 * _t29));
    const auto _t39 = ((_t21 * _t32) + (gR22 * (gR24Rq2 * (gR16Rboostp * _t31))));
    const auto _t40 = (gC11 * _t32);
    for (int ch = 0; ch < num_channels; ++ch)
    {
        auto zR12C8 = state[ch].zR12C8;
        auto zRf1R18C9 = state[ch].zRf1R18C9;
        auto zC11 = state[ch].zC11;
        auto zR19Rf2C10 = state[ch].zR19Rf2C10;
        for (int n = 0; n < num_samples; ++n)
        {
            const auto vi = input[ch][n];

            const auto _t10 = (zRf1R18C9 - zR19Rf2C10);
            const auto _t12 = ((gRboostmR11 * vi) + zRf1R18C9);
            const auto _t14 = (zR12C8 - (gR50 * vi));
            const auto _t9 = (_t10 * _t3);
            const auto _t28 = (_t10 * _t18);
            const auto _t8 = (_t1 * _t9);
            const auto _t11 = (_t12 * _t13);
            const auto _t7 = (_t8 - _t11);
            const auto vo = (((gR13 * ((zC11 * _t33) - (gC11 * (gR23 * _t7)))) + (_t14 * _t15)) * _t25);
            const auto vR12C8 = (vo - 0);
            const auto v13 = (((gR12C8 * (((gC11 * ((gR24Rq2 * ((_t12 * _t17) - (gRf1R18C9 * _t28))) + (gR23 * (gR64 * _t9)))) - ((_t21 * (gR24Rq2 * (_t12 * _t2))) + (zC11 * _t34))) - (gR22 * (gR24Rq2 * (_t12 * _t23))))) - ((gR13 * ((gC11 * (gR23 * (gR16Rboostp * _t9))) - (zC11 * _t35))) - (_t14 * _t36))) * _t25);
            const auto vn4 = (((_t14 * _t37) + (gR12C8 * ((gC11 * (gR24Rq2 * ((_t12 * _t20) - (_t1 * _t28)))) + (zC11 * _t38)))) * _t25);
            const auto vRf1R18C9 = (v13 - vn4);
            const auto v15 = (((gR12C8 * (((zC11 * _t16) - (_t21 * (gR24Rq2 * _t7))) + (gR22 * (gR24Rq2 * ((_t12 * _t31) - (_t1 * (_t10 * _t24))))))) - ((gR13 * (zC11 * _t26)) - (_t14 * _t39))) * _t25);
            const auto vc11 = (((_t14 * _t40) + (gR12C8 * ((zC11 * _t22) + (gC11 * (gR24Rq2 * (_t11 - _t8)))))) * _t25);
            const auto tC11 = (gC11 * (v15 - vc11));
            const auto vR19Rf2C10 = (vn4 - v15);
            
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
