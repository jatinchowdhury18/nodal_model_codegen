// Auto-generated with netlist_codegen version ac50416.
// Command: netlist_codegen pedal_model.net pedal_model.h -opt_port_matrix -type_name double

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


[[maybe_unused]] static auto limit_junction_voltage_sym = [](auto v_new, auto v_old, auto vt, auto vcrit)
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

[[maybe_unused]] static auto limit_jfet_vgs = [](auto v_new, auto vp)
{
    if (v_new < vp) return vp;
    return v_new;
};

static constexpr auto newton_tol_sq = 0.000001;
static constexpr int newton_max_iter = 20;


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
    const auto _t23 = (gRL + gC13);
    const auto _t37 = ((gR7C6 + gR8C7) + gRdC5);
    const auto _t41 = (gRfR10 * gR9C9);
    const auto _t48 = (_t4 * (gRdC5 * (gC12 * gRfR10)));
    const auto _t50 = (_t13 * gR9C9);
    const auto _t52 = (gC12 * gR9C9);
    const auto _t6 = (gC12 * _t7);
    const auto _t12 = ((_t13 * _t7) - (gRfR10 * gRfR10));
    const auto _t40 = (_t37 * _t41);
    const auto _t5 = (gRdC5 * _t6);
    const auto _t11 = (gRdC5 * _t12);
    const auto _t3 = (_t4 * _t5);
    const auto _t10 = (_t4 * _t11);
    const auto _t21 = (gC13 * _t3);
    const auto _t22 = (gC13 * _t10);
    const auto _t1 = (_t2 * _t3);
    const auto _t9 = (_t2 * _t10);
    const auto _t20 = ((gC12 * _t21) - (_t8 * _t22));
    const auto _t0 = ((gC12 * _t1) - (_t8 * _t9));
    const auto _t19 = (1.0 / ((gC13 * _t20) - (_t23 * _t0)));
    const auto _t53 = (gC13 * _t48);
    const auto _t54 = (_t2 * _t48);
    const auto _t55 = (1.0 / 1000.0);
    const auto _t56 = (_t37 * _t12);
    const auto _t57 = (_t37 * _t6);
    const auto _t58 = (_t37 * _t50);
    const auto _t59 = (_t37 * _t52);
    double c0_vRdC5;
    double c_vRdC5[10];
    
    for (int _k = 0; _k <= 10; ++_k)
    {
        const auto vi = (_k == 1) ? 1.0 : 0.0;
        const auto zC4 = (_k == 2) ? 1.0 : 0.0;
        const auto zC12 = (_k == 3) ? 1.0 : 0.0;
        const auto zR7C6 = (_k == 4) ? 1.0 : 0.0;
        const auto zC13 = (_k == 5) ? 1.0 : 0.0;
        const auto zR6C3 = (_k == 6) ? 1.0 : 0.0;
        const auto zC11 = (_k == 7) ? 1.0 : 0.0;
        const auto zR8C7 = (_k == 8) ? 1.0 : 0.0;
        const auto zRdC5 = (_k == 9) ? 1.0 : 0.0;
        const auto zR9C9 = (_k == 10) ? 1.0 : 0.0;
        const auto _t25 = (zC4 + ((gR6C3 * vi) + zR6C3));
        const auto _t36 = ((zR7C6 + zR8C7) + zRdC5);
        const auto _t46 = ((_t4 * (_t36 * _t12)) - (_t25 * _t56));
        const auto _t47 = ((_t4 * (_t36 * _t6)) - (_t25 * _t57));
        const auto _t24 = (_t25 * _t5);
        const auto _t26 = (_t25 * _t11);
        const auto vneg = (((gC13 * ((gC12 * (gC13 * _t24)) - (_t8 * (gC13 * _t26)))) - (_t23 * ((gC12 * (_t2 * _t24)) - (_t8 * (_t2 * _t26))))) * _t19);
        const auto v2 = (((gC13 * ((_t8 * (gC13 * _t46)) - (gC12 * (gC13 * _t47)))) - (_t23 * ((_t8 * (_t2 * _t46)) - (gC12 * (_t2 * _t47))))) * _t19);
        const auto vRdC5 = (vneg - v2);
        if (_k == 0) {
            c0_vRdC5 = vRdC5;
        } else {
            c_vRdC5[_k - 1] = vRdC5 - c0_vRdC5;
        }
    }
    
    const auto _J1_D1D2_zt4 = (gC12 + gC11);
    const auto _J1_D1D2_zt11 = (gRfR10 * 1.0);
    const auto _J1_D1D2_zt12 = (-_J1_D1D2_zt11);
    const auto _J1_D1D2_zt16 = (gRfR10 * gRfR10);
    const auto _J1_D1D2_zt23 = (gRL + gC13);
    const auto _J1_D1D2_zt25 = (_J1_D1D2_zt4 + gRfR10);
    const auto _J1_D1D2_zt26 = (gC12 * 1.0);
    const auto _J1_D1D2_zt31 = (gR9C9 + gRfR10);
    const auto _J1_D1D2_zt42 = (gC13 + gR13);
    const auto _J1_D1D2_zt43 = (gC4 + gR6C3);
    const auto _J1_D1D2_zt52 = (gR11 + gC12);
    const auto _J1_D1D2_zt6 = (gC12 * _J1_D1D2_zt31);
    const auto _J1_D1D2_zt10 = (gRdC5 * _J1_D1D2_zt12);
    const auto _J1_D1D2_zt22 = (gRdC5 * _J1_D1D2_zt26);
    const auto _J1_D1D2_zt24 = (_J1_D1D2_zt25 * 1.0);
    const auto _J1_D1D2_zt30 = (_J1_D1D2_zt25 * _J1_D1D2_zt31);
    const auto _J1_D1D2_zt34 = (_J1_D1D2_zt30 - _J1_D1D2_zt16);
    const auto _J1_D1D2_zt0 = (gRdC5 * _J1_D1D2_zt6);
    const auto _J1_D1D2_zt1 = (gRdC5 * _J1_D1D2_zt34);
    const auto _J1_D1D2_zt3 = (_J1_D1D2_zt43 * _J1_D1D2_zt0);
    const auto _J1_D1D2_zt9 = (_J1_D1D2_zt43 * _J1_D1D2_zt10);
    const auto _J1_D1D2_zt20 = (-_J1_D1D2_zt24);
    const auto _J1_D1D2_zt21 = (_J1_D1D2_zt43 * _J1_D1D2_zt22);
    const auto _J1_D1D2_zt29 = (_J1_D1D2_zt43 * _J1_D1D2_zt1);
    const auto _J1_D1D2_zt33 = (_J1_D1D2_zt42 * _J1_D1D2_zt3);
    const auto _J1_D1D2_zt51 = (gC13 * _J1_D1D2_zt3);
    const auto _J1_D1D2_zt8 = (_J1_D1D2_zt42 * _J1_D1D2_zt29);
    const auto _J1_D1D2_zt14 = (gC12 * _J1_D1D2_zt51);
    const auto _J1_D1D2_zt15 = (gC13 * _J1_D1D2_zt29);
    const auto _J1_D1D2_zt17 = (gC12 * _J1_D1D2_zt33);
    const auto _J1_D1D2_zt19 = (gRdC5 * _J1_D1D2_zt20);
    const auto _J1_D1D2_zt28 = (_J1_D1D2_zt52 * _J1_D1D2_zt15);
    const auto _J1_D1D2_zt7 = (_J1_D1D2_zt52 * _J1_D1D2_zt8);
    const auto _J1_D1D2_zt18 = (_J1_D1D2_zt43 * _J1_D1D2_zt19);
    const auto _J1_D1D2_zt27 = (_J1_D1D2_zt14 - _J1_D1D2_zt28);
    const auto _J1_D1D2_zt2 = (gC13 * _J1_D1D2_zt27);
    const auto _J1_D1D2_zt5 = (_J1_D1D2_zt17 - _J1_D1D2_zt7);
    const auto _J1_D1D2_zt32 = (_J1_D1D2_zt23 * _J1_D1D2_zt5);
    const auto _J1_D1D2_zt13 = (_J1_D1D2_zt2 - _J1_D1D2_zt32);
    const auto _J1_D1D2_Z0_0 = (-((-(_J1_D1D2_zt23 * ((gC12 * (_J1_D1D2_zt3 * 1.0)) - (_J1_D1D2_zt52 * (_J1_D1D2_zt29 * 1.0))))) / _J1_D1D2_zt13));
    const auto _J1_D1D2_Z0_1 = (((gC13 * (-(gC12 * (gC13 * _J1_D1D2_zt9)))) - (_J1_D1D2_zt23 * (-(gC12 * (_J1_D1D2_zt42 * _J1_D1D2_zt9))))) / _J1_D1D2_zt13);
    const auto _J1_D1D2_Z1_1 = (((gC13 * (-((_J1_D1D2_zt52 * (gC13 * _J1_D1D2_zt18)) + (gC12 * (gC13 * _J1_D1D2_zt21))))) - (_J1_D1D2_zt23 * (-((_J1_D1D2_zt52 * (_J1_D1D2_zt42 * _J1_D1D2_zt18)) + (gC12 * (_J1_D1D2_zt42 * _J1_D1D2_zt21)))))) / _J1_D1D2_zt13);
    double c0__J1_D1D2_voc1;
    double c__J1_D1D2_voc1[10];
    double c0__J1_D1D2_voc0;
    double c__J1_D1D2_voc0[10];
    double c0__J1_D1D2_zt72;
    double c__J1_D1D2_zt72[10];
    
    for (int _k = 0; _k <= 10; ++_k)
    {
        const auto vi = (_k == 1) ? 1.0 : 0.0;
        const auto zC4 = (_k == 2) ? 1.0 : 0.0;
        const auto zC12 = (_k == 3) ? 1.0 : 0.0;
        const auto zR7C6 = (_k == 4) ? 1.0 : 0.0;
        const auto zC13 = (_k == 5) ? 1.0 : 0.0;
        const auto zR6C3 = (_k == 6) ? 1.0 : 0.0;
        const auto zC11 = (_k == 7) ? 1.0 : 0.0;
        const auto zR8C7 = (_k == 8) ? 1.0 : 0.0;
        const auto zRdC5 = (_k == 9) ? 1.0 : 0.0;
        const auto zR9C9 = (_k == 10) ? 1.0 : 0.0;
        const auto _J1_D1D2_zt36 = (gR6C3 * vi);
        const auto _J1_D1D2_zt38 = (gRfR10 * gR9C9);
        const auto _J1_D1D2_zt41 = (gRfR10 * zR9C9);
        const auto _J1_D1D2_zt57 = (zC12 + zC11);
        const auto _J1_D1D2_zt58 = (_J1_D1D2_zt25 * zR9C9);
        const auto _J1_D1D2_zt59 = (_J1_D1D2_zt25 * gR9C9);
        const auto _J1_D1D2_zt69 = (gC12 * gR9C9);
        const auto _J1_D1D2_zt71 = (gC12 * gRfR10);
        const auto _J1_D1D2_zt77 = (zR7C6 + zR8C7);
        const auto _J1_D1D2_zt78 = (gC12 * zR9C9);
        const auto _J1_D1D2_zt80 = (gR7C6 + gR8C7);
        const auto _J1_D1D2_zt35 = (_J1_D1D2_zt36 + zR6C3);
        const auto _J1_D1D2_zt37 = (_J1_D1D2_zt80 + gRdC5);
        const auto _J1_D1D2_zt46 = (_J1_D1D2_zt37 * _J1_D1D2_zt38);
        const auto _J1_D1D2_zt49 = (_J1_D1D2_zt57 * _J1_D1D2_zt31);
        const auto _J1_D1D2_zt54 = (gRdC5 * _J1_D1D2_zt71);
        const auto _J1_D1D2_zt56 = (_J1_D1D2_zt57 * gRfR10);
        const auto _J1_D1D2_zt61 = (zC4 + _J1_D1D2_zt35);
        const auto _J1_D1D2_zt62 = (_J1_D1D2_zt37 * _J1_D1D2_zt59);
        const auto _J1_D1D2_zt66 = (gRdC5 * _J1_D1D2_zt78);
        const auto _J1_D1D2_zt68 = (_J1_D1D2_zt77 + zRdC5);
        const auto _J1_D1D2_zt75 = (_J1_D1D2_zt56 - _J1_D1D2_zt58);
        const auto _J1_D1D2_zt76 = (_J1_D1D2_zt68 * _J1_D1D2_zt59);
        const auto _J1_D1D2_zt79 = (_J1_D1D2_zt37 * _J1_D1D2_zt69);
        const auto _J1_D1D2_zt45 = (_J1_D1D2_zt61 * _J1_D1D2_zt46);
        const auto _J1_D1D2_zt48 = (_J1_D1D2_zt49 - _J1_D1D2_zt41);
        const auto _J1_D1D2_zt50 = (_J1_D1D2_zt68 * _J1_D1D2_zt38);
        const auto _J1_D1D2_zt53 = (_J1_D1D2_zt43 * _J1_D1D2_zt54);
        const auto _J1_D1D2_zt55 = (gRdC5 * _J1_D1D2_zt75);
        const auto _J1_D1D2_zt60 = (_J1_D1D2_zt61 * _J1_D1D2_zt62);
        const auto _J1_D1D2_zt67 = (_J1_D1D2_zt68 * _J1_D1D2_zt69);
        const auto _J1_D1D2_zt70 = (_J1_D1D2_zt61 * _J1_D1D2_zt79);
        const auto _J1_D1D2_zt74 = (_J1_D1D2_zt55 - _J1_D1D2_zt76);
        const auto _J1_D1D2_zt40 = (gRdC5 * _J1_D1D2_zt48);
        const auto _J1_D1D2_zt65 = (_J1_D1D2_zt66 + _J1_D1D2_zt67);
        const auto _J1_D1D2_zt73 = (_J1_D1D2_zt43 * _J1_D1D2_zt74);
        const auto _J1_D1D2_zt39 = (_J1_D1D2_zt40 - _J1_D1D2_zt50);
        const auto _J1_D1D2_zt47 = (_J1_D1D2_zt43 * _J1_D1D2_zt39);
        const auto _J1_D1D2_zt64 = (_J1_D1D2_zt43 * _J1_D1D2_zt65);
        const auto _J1_D1D2_zt72 = (_J1_D1D2_zt73 + _J1_D1D2_zt60);
        const auto _J1_D1D2_zt44 = (_J1_D1D2_zt45 + _J1_D1D2_zt47);
        const auto _J1_D1D2_zt63 = (_J1_D1D2_zt64 - _J1_D1D2_zt70);
        const auto _J1_D1D2_voc0 = ((((gC13 * ((zC12 * _J1_D1D2_zt15) - (gC12 * (gC13 * _J1_D1D2_zt44)))) - (_J1_D1D2_zt23 * ((zC12 * _J1_D1D2_zt8) - (gC12 * (_J1_D1D2_zt42 * _J1_D1D2_zt44))))) / _J1_D1D2_zt13) - (((zC13 * _J1_D1D2_zt27) - (_J1_D1D2_zt23 * ((gC12 * (_J1_D1D2_zt3 * zC13)) - (_J1_D1D2_zt52 * (_J1_D1D2_zt29 * zC13))))) / _J1_D1D2_zt13));
        const auto _J1_D1D2_voc1 = (((gC13 * ((zC12 * (gC13 * _J1_D1D2_zt53)) - ((_J1_D1D2_zt52 * (gC13 * _J1_D1D2_zt72)) + (gC12 * (gC13 * _J1_D1D2_zt63))))) - (_J1_D1D2_zt23 * ((zC12 * (_J1_D1D2_zt42 * _J1_D1D2_zt53)) - ((_J1_D1D2_zt52 * (_J1_D1D2_zt42 * _J1_D1D2_zt72)) + (gC12 * (_J1_D1D2_zt42 * _J1_D1D2_zt63)))))) / _J1_D1D2_zt13);
        if (_k == 0) {
            c0__J1_D1D2_voc1 = _J1_D1D2_voc1;
            c0__J1_D1D2_voc0 = _J1_D1D2_voc0;
            c0__J1_D1D2_zt72 = _J1_D1D2_zt72;
        } else {
            c__J1_D1D2_voc1[_k - 1] = _J1_D1D2_voc1 - c0__J1_D1D2_voc1;
            c__J1_D1D2_voc0[_k - 1] = _J1_D1D2_voc0 - c0__J1_D1D2_voc0;
            c__J1_D1D2_zt72[_k - 1] = _J1_D1D2_zt72 - c0__J1_D1D2_zt72;
        }
    }
    
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

            // --- Newton-Raphson solve (N-port): J1_D1D2
            const auto _J1_D1D2_voc1 = c0__J1_D1D2_voc1 + c__J1_D1D2_voc1[0] * vi + c__J1_D1D2_voc1[1] * zC4 + c__J1_D1D2_voc1[2] * zC12 + c__J1_D1D2_voc1[3] * zR7C6 + c__J1_D1D2_voc1[4] * zC13 + c__J1_D1D2_voc1[5] * zR6C3 + c__J1_D1D2_voc1[6] * zC11 + c__J1_D1D2_voc1[7] * zR8C7 + c__J1_D1D2_voc1[8] * zRdC5 + c__J1_D1D2_voc1[9] * zR9C9;
            const auto _J1_D1D2_voc0 = c0__J1_D1D2_voc0 + c__J1_D1D2_voc0[0] * vi + c__J1_D1D2_voc0[1] * zC4 + c__J1_D1D2_voc0[2] * zC12 + c__J1_D1D2_voc0[3] * zR7C6 + c__J1_D1D2_voc0[4] * zC13 + c__J1_D1D2_voc0[5] * zR6C3 + c__J1_D1D2_voc0[6] * zC11 + c__J1_D1D2_voc0[7] * zR8C7 + c__J1_D1D2_voc0[8] * zRdC5 + c__J1_D1D2_voc0[9] * zR9C9;
            const auto _J1_D1D2_zt72 = c0__J1_D1D2_zt72 + c__J1_D1D2_zt72[0] * vi + c__J1_D1D2_zt72[1] * zC4 + c__J1_D1D2_zt72[2] * zC12 + c__J1_D1D2_zt72[3] * zR7C6 + c__J1_D1D2_zt72[4] * zC13 + c__J1_D1D2_zt72[5] * zR6C3 + c__J1_D1D2_zt72[6] * zC11 + c__J1_D1D2_zt72[7] * zR8C7 + c__J1_D1D2_zt72[8] * zRdC5 + c__J1_D1D2_zt72[9] * zR9C9;
            const auto _J1_D1D2_zt36 = (gR6C3 * vi);
            const auto _J1_D1D2_zt38 = (gRfR10 * gR9C9);
            const auto _J1_D1D2_zt41 = (gRfR10 * zR9C9);
            const auto _J1_D1D2_zt57 = (zC12 + zC11);
            const auto _J1_D1D2_zt58 = (_J1_D1D2_zt25 * zR9C9);
            const auto _J1_D1D2_zt59 = (_J1_D1D2_zt25 * gR9C9);
            const auto _J1_D1D2_zt69 = (gC12 * gR9C9);
            const auto _J1_D1D2_zt71 = (gC12 * gRfR10);
            const auto _J1_D1D2_zt77 = (zR7C6 + zR8C7);
            const auto _J1_D1D2_zt78 = (gC12 * zR9C9);
            const auto _J1_D1D2_zt80 = (gR7C6 + gR8C7);
            const auto _J1_D1D2_zt35 = (_J1_D1D2_zt36 + zR6C3);
            const auto _J1_D1D2_zt37 = (_J1_D1D2_zt80 + gRdC5);
            const auto _J1_D1D2_zt46 = (_J1_D1D2_zt37 * _J1_D1D2_zt38);
            const auto _J1_D1D2_zt49 = (_J1_D1D2_zt57 * _J1_D1D2_zt31);
            const auto _J1_D1D2_zt54 = (gRdC5 * _J1_D1D2_zt71);
            const auto _J1_D1D2_zt56 = (_J1_D1D2_zt57 * gRfR10);
            const auto _J1_D1D2_zt61 = (zC4 + _J1_D1D2_zt35);
            const auto _J1_D1D2_zt62 = (_J1_D1D2_zt37 * _J1_D1D2_zt59);
            const auto _J1_D1D2_zt66 = (gRdC5 * _J1_D1D2_zt78);
            const auto _J1_D1D2_zt68 = (_J1_D1D2_zt77 + zRdC5);
            const auto _J1_D1D2_zt75 = (_J1_D1D2_zt56 - _J1_D1D2_zt58);
            const auto _J1_D1D2_zt76 = (_J1_D1D2_zt68 * _J1_D1D2_zt59);
            const auto _J1_D1D2_zt79 = (_J1_D1D2_zt37 * _J1_D1D2_zt69);
            const auto _J1_D1D2_zt45 = (_J1_D1D2_zt61 * _J1_D1D2_zt46);
            const auto _J1_D1D2_zt48 = (_J1_D1D2_zt49 - _J1_D1D2_zt41);
            const auto _J1_D1D2_zt50 = (_J1_D1D2_zt68 * _J1_D1D2_zt38);
            const auto _J1_D1D2_zt53 = (_J1_D1D2_zt43 * _J1_D1D2_zt54);
            const auto _J1_D1D2_zt55 = (gRdC5 * _J1_D1D2_zt75);
            const auto _J1_D1D2_zt60 = (_J1_D1D2_zt61 * _J1_D1D2_zt62);
            const auto _J1_D1D2_zt67 = (_J1_D1D2_zt68 * _J1_D1D2_zt69);
            const auto _J1_D1D2_zt70 = (_J1_D1D2_zt61 * _J1_D1D2_zt79);
            const auto _J1_D1D2_zt74 = (_J1_D1D2_zt55 - _J1_D1D2_zt76);
            const auto _J1_D1D2_zt40 = (gRdC5 * _J1_D1D2_zt48);
            const auto _J1_D1D2_zt65 = (_J1_D1D2_zt66 + _J1_D1D2_zt67);
            const auto _J1_D1D2_zt73 = (_J1_D1D2_zt43 * _J1_D1D2_zt74);
            const auto _J1_D1D2_zt39 = (_J1_D1D2_zt40 - _J1_D1D2_zt50);
            const auto _J1_D1D2_zt47 = (_J1_D1D2_zt43 * _J1_D1D2_zt39);
            const auto _J1_D1D2_zt64 = (_J1_D1D2_zt43 * _J1_D1D2_zt65);
            const auto _J1_D1D2_zt44 = (_J1_D1D2_zt45 + _J1_D1D2_zt47);
            const auto _J1_D1D2_zt63 = (_J1_D1D2_zt64 - _J1_D1D2_zt70);
            const auto _J1_D1D2_pt0 = (1.0 / 1000.0);
            const auto _J1_D1D2_pt11 = (1.0 / D1N914_vt);
            for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
            {
                const auto _J1_D1D2_pt2 = (vGSJ1 - _2N5485_vp);
                const auto _J1_D1D2_pt4 = (vD1D2 / D1N914_vt);
                const auto _J1_D1D2_pt1 = (_J1_D1D2_pt2 + _J1_D1D2_pt0);
                const auto _J1_D1D2_pt3 = math_exp_approx(_J1_D1D2_pt4);
                const auto _J1_D1D2_i0 = (_2N5485_Beta * (_J1_D1D2_pt1 * _J1_D1D2_pt1));
                const auto _J1_D1D2_i1 = (D1N914_Is * (_J1_D1D2_pt3 - (1.0 / _J1_D1D2_pt3)));
                const auto _J1_D1D2_g0_0 = (_2N5485_Beta * (_J1_D1D2_pt1 + _J1_D1D2_pt1));
                const auto _J1_D1D2_g1_1 = (D1N914_Is * ((_J1_D1D2_pt3 / D1N914_vt) + (_J1_D1D2_pt11 / _J1_D1D2_pt3)));
                const auto _J1_D1D2_pt5 = ((_J1_D1D2_voc0 + (_J1_D1D2_Z0_0 * _J1_D1D2_i0)) + (_J1_D1D2_Z0_1 * _J1_D1D2_i1));
                const auto _J1_D1D2_pt6 = (_J1_D1D2_voc1 + (_J1_D1D2_Z1_1 * _J1_D1D2_i1));
                const auto _J1_D1D2_pt7 = ((_J1_D1D2_Z1_1 * _J1_D1D2_g1_1) - 1.0);
                const auto _J1_D1D2_pt8 = (vD1D2 - _J1_D1D2_pt6);
                const auto _J1_D1D2_pt10 = ((_J1_D1D2_Z0_0 * _J1_D1D2_g0_0) - 1.0);
                const auto _J1_D1D2_pt9 = (_J1_D1D2_pt10 * _J1_D1D2_pt7);
                const auto res_vGSJ1 = (_J1_D1D2_pt5 - vGSJ1);
                const auto res_vD1D2 = (_J1_D1D2_pt6 - vD1D2);
                const auto delta_vGSJ1 = ((((vGSJ1 - _J1_D1D2_pt5) * _J1_D1D2_pt7) - ((_J1_D1D2_Z0_1 * _J1_D1D2_g1_1) * _J1_D1D2_pt8)) / _J1_D1D2_pt9);
                const auto delta_vD1D2 = ((_J1_D1D2_pt10 * _J1_D1D2_pt8) / _J1_D1D2_pt9);
            
                auto residual_norm_sq = 0.0;
                residual_norm_sq += res_vGSJ1 * res_vGSJ1;
                residual_norm_sq += res_vD1D2 * res_vD1D2;
                auto step_norm_sq = 0.0;
                step_norm_sq += delta_vGSJ1 * delta_vGSJ1;
                step_norm_sq += delta_vD1D2 * delta_vD1D2;
            
                vGSJ1 = limit_jfet_vgs(vGSJ1 + (delta_vGSJ1), _2N5485_vp);
                vD1D2 = limit_junction_voltage_sym(vD1D2 + (delta_vD1D2), vD1D2, D1N914_vt, vcrit_D1N914_vt);
            
                if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
                    break;
                
            }

            const auto vRdC5 = c0_vRdC5 + c_vRdC5[0] * vi + c_vRdC5[1] * zC4 + c_vRdC5[2] * zC12 + c_vRdC5[3] * zR7C6 + c_vRdC5[4] * zC13 + c_vRdC5[5] * zR6C3 + c_vRdC5[6] * zC11 + c_vRdC5[7] * zR8C7 + c_vRdC5[8] * zRdC5 + c_vRdC5[9] * zR9C9;
            const auto _t16 = ((vGSJ1 - _2N5485_vp) + _t55);
            const auto _t25 = (zC4 + ((gR6C3 * vi) + zR6C3));
            const auto _t31 = (zC12 + zC11);
            const auto _t34 = math_exp_approx((vD1D2 / D1N914_vt));
            const auto _t36 = ((zR7C6 + zR8C7) + zRdC5);
            const auto _t43 = (_t31 * _t7);
            const auto _t15 = (zC13 + (_2N5485_Beta * (_t16 * _t16)));
            const auto _t33 = (zR9C9 + (D1N914_Is * (_t34 - (1.0 / _t34))));
            const auto _t35 = (_t36 * _t41);
            const auto _t46 = ((_t4 * (_t36 * _t12)) - (_t25 * _t56));
            const auto _t47 = ((_t4 * (_t36 * _t6)) - (_t25 * _t57));
            const auto _t49 = ((_t4 * ((gRdC5 * ((_t31 * gRfR10) - (_t13 * _t33))) - (_t36 * _t50))) + (_t25 * _t58));
            const auto _t51 = ((_t4 * ((gRdC5 * (gC12 * _t33)) + (_t36 * _t52))) - (_t25 * _t59));
            const auto _t24 = (_t25 * _t5);
            const auto _t26 = (_t25 * _t11);
            const auto _t32 = (gRfR10 * _t33);
            const auto _t39 = (_t25 * _t40);
            const auto _t42 = (_t43 - _t32);
            const auto _t18 = (_t15 * _t3);
            const auto _t30 = (gRdC5 * _t42);
            const auto _t44 = (_t15 * _t10);
            const auto _t14 = (_t8 * _t44);
            const auto _t17 = (gC12 * _t18);
            const auto _t29 = (_t30 - _t35);
            const auto _t28 = (_t4 * _t29);
            const auto _t38 = (_t39 + _t28);
            const auto _t27 = (_t28 + _t39);
            const auto vo = (((zC13 * _t0) + (gC13 * (_t14 - _t17))) * _t19);
            const auto vneg = (((gC13 * ((gC12 * (gC13 * _t24)) - (_t8 * (gC13 * _t26)))) - (_t23 * ((gC12 * (_t2 * _t24)) - (_t8 * (_t2 * _t26))))) * _t19);
            const auto vf = (((gC13 * ((zC12 * _t21) - (_t8 * (gC13 * _t27)))) - (_t23 * ((zC12 * _t1) - (_t8 * (_t2 * _t27))))) * _t19);
            const auto vg = (((gC13 * ((zC12 * _t22) - (gC12 * (gC13 * _t38)))) - (_t23 * ((zC12 * _t9) - (gC12 * (_t2 * _t38))))) * _t19);
            const auto tC12 = (gC12 * (vf - vg));
            const auto vs = (((zC13 * _t20) - (_t23 * (_t17 - _t14))) * _t19);
            const auto tC13 = (gC13 * (vs - vo));
            const auto vR6C3 = (vneg - vi);
            const auto tC11 = (gC11 * (vf - 0));
            const auto v2 = (((gC13 * ((_t8 * (gC13 * _t46)) - (gC12 * (gC13 * _t47)))) - (_t23 * ((_t8 * (_t2 * _t46)) - (gC12 * (_t2 * _t47))))) * _t19);
            const auto vdp = (((gC13 * ((zC12 * _t53) - ((_t8 * (gC13 * _t49)) + (gC12 * (gC13 * _t51))))) - (_t23 * ((zC12 * _t54) - ((_t8 * (_t2 * _t49)) + (gC12 * (_t2 * _t51)))))) * _t19);
            const auto vR9C9 = (v2 - vdp);
            const auto _t45 = (vneg - 0);
            const auto tC4 = (gC4 * _t45);
            const auto vR7C6 = _t45;
            const auto vR8C7 = _t45;
            
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

static float reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc = 0.0f)
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

    const auto _J1_Z0_0 = (-(1.0 / (gR13 + (1.0 / 1000000000.0))));
    // --- Newton-Raphson solve (N-port): J1
    const auto _J1_pt1 = (1.0 / 1000.0);
    for (int newton_iter = 0; newton_iter < 10000; ++newton_iter)
    {
        const auto _J1_pt0 = (vGSJ1 - _2N5485_vp);
        const auto _J1_pt2 = (_J1_pt0 + _J1_pt1);
        const auto _J1_i0 = (_2N5485_Beta * (_J1_pt2 * _J1_pt2));
        const auto _J1_g0_0 = (_2N5485_Beta * (_J1_pt2 + _J1_pt2));
        const auto _J1_pt3 = (_J1_Z0_0 * _J1_i0);
        const auto res_vGSJ1 = (_J1_pt3 - vGSJ1);
        const auto delta_vGSJ1 = ((vGSJ1 - _J1_pt3) / ((_J1_Z0_0 * _J1_g0_0) - 1.0));
    
        auto residual_norm_sq = 0.0;
        residual_norm_sq += res_vGSJ1 * res_vGSJ1;
        auto step_norm_sq = 0.0;
        step_norm_sq += delta_vGSJ1 * delta_vGSJ1;
    
        vGSJ1 = limit_jfet_vgs(vGSJ1 + (delta_vGSJ1), _2N5485_vp);
    
        if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
            break;
        
    }
    const auto _D1D2_zt1 = (1.0 / 1000000000.0);
    const auto _D1D2_zt0 = (gRfR10 + _D1D2_zt1);
    const auto _D1D2_Z0_0 = (-((_D1D2_zt0 * 1.0) / ((_D1D2_zt0 * _D1D2_zt0) - (gRfR10 * gRfR10))));
    // --- Newton-Raphson solve (N-port): D1D2
    const auto _D1D2_pt3 = (1.0 / D1N914_vt);
    for (int newton_iter = 0; newton_iter < 10000; ++newton_iter)
    {
        const auto _D1D2_pt0 = (vD1D2 / D1N914_vt);
        const auto _D1D2_pt1 = math_exp_approx(_D1D2_pt0);
        const auto _D1D2_i0 = (D1N914_Is * (_D1D2_pt1 - (1.0 / _D1D2_pt1)));
        const auto _D1D2_g0_0 = (D1N914_Is * ((_D1D2_pt1 / D1N914_vt) + (_D1D2_pt3 / _D1D2_pt1)));
        const auto _D1D2_pt2 = (_D1D2_Z0_0 * _D1D2_i0);
        const auto res_vD1D2 = (_D1D2_pt2 - vD1D2);
        const auto delta_vD1D2 = ((vD1D2 - _D1D2_pt2) / ((_D1D2_Z0_0 * _D1D2_g0_0) - 1.0));
    
        auto residual_norm_sq = 0.0;
        residual_norm_sq += res_vD1D2 * res_vD1D2;
        auto step_norm_sq = 0.0;
        step_norm_sq += delta_vD1D2 * delta_vD1D2;
    
        vD1D2 = limit_junction_voltage_sym(vD1D2 + (delta_vD1D2), vD1D2, D1N914_vt, vcrit_D1N914_vt);
    
        if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
            break;
        
    }
    const auto zC4 = 0.0;
    const auto zC12 = (-((gC12 * ((gRfR10 * D1N914_Is) * (math_exp_approx((vD1D2 / D1N914_vt)) - (1.0 / math_exp_approx((vD1D2 / D1N914_vt)))))) / (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))));
    const auto zR7C6 = 0.0;
    const auto zC13 = ((gC13 * ((_2N5485_Beta * ((vGSJ1 - _2N5485_vp) + (1.0 / 1000.0))) * ((vGSJ1 - _2N5485_vp) + (1.0 / 1000.0)))) / (gR13 + (1.0 / 1000000000.0)));
    const auto zR6C3 = (-(gR6C3 * vi));
    const auto zC11 = (-((gC11 * ((gRfR10 * D1N914_Is) * (math_exp_approx((vD1D2 / D1N914_vt)) - (1.0 / math_exp_approx((vD1D2 / D1N914_vt)))))) / (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10))));
    const auto zR8C7 = 0.0;
    const auto zRdC5 = 0.0;
    const auto zR9C9 = ((gR9C9 * (((gRfR10 + (1.0 / 1000000000.0)) * D1N914_Is) * (math_exp_approx((vD1D2 / D1N914_vt)) - (1.0 / math_exp_approx((vD1D2 / D1N914_vt)))))) / (((gRfR10 + (1.0 / 1000000000.0)) * (gRfR10 + (1.0 / 1000000000.0))) - (gRfR10 * gRfR10)));

    const auto vo_dc_out = 0.0;

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
    return vo_dc_out;
}

