// Auto-generated with netlist_codegen version de49196.
// Command: netlist_codegen pedal_drive.net pedal_drive.h -opt_port_matrix -type_name double

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


[[maybe_unused]] static auto limit_junction_voltage = [](auto v_new, auto v_old, auto vt, auto vcrit)
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
    return v_new;
};

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

static constexpr auto newton_tol_sq = 0.0001;
static constexpr int newton_max_iter = 10;


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
    
    const auto _t8 = (((gR20 + gR19C5) + gC6) + gR17C12);
    const auto _t9 = (gR21 * gC6);
    const auto _t10 = ((gCout + gR18) + gR17C12);
    const auto _t17 = (gR17C12 * _t9);
    const auto _t22 = (gCout + gRout);
    const auto _t36 = (Vp * gC6);
    const auto _t37 = (_t22 * _t17);
    const auto _t7 = ((gC6 * _t9) - (_t8 * _t9));
    const auto _t21 = (_t22 * _t7);
    const auto _t31 = (gR21 * _t36);
    const auto _t35 = (gR17C12 * _t31);
    const auto _t6 = (gCout * _t7);
    const auto _t18 = (gC6 * _t31);
    const auto _t19 = (_t8 * _t31);
    const auto _t20 = (1.0 / (((_t10 * _t21) - (gCout * _t6)) + (gR17C12 * _t37)));
    const auto _t26 = (_t18 - _t19);
    const auto _t30 = (_t22 * _t35);
    const auto _t34 = (gR18 * _t30);
    const auto _t38 = (gR18 * (gCout * _t26));
    const auto _t39 = (gR18 * (_t22 * _t26));
    const auto _Q1_D3D4_zt8 = (gCout + gRout);
    const auto _Q1_D3D4_zt16 = (gR21 * 1.0);
    const auto _Q1_D3D4_zt19 = (gCout + gR18);
    const auto _Q1_D3D4_zt21 = (gR21 * gC6);
    const auto _Q1_D3D4_zt29 = (gR20 + gR19C5);
    const auto _Q1_D3D4_zt31 = (gC6 * 1.0);
    const auto _Q1_D3D4_zt35 = (-1.0);
    const auto _Q1_D3D4_zt46 = (gC6 * _Q1_D3D4_zt16);
    const auto _Q1_D3D4_zt50 = (_Q1_D3D4_zt21 * 1.0);
    const auto _Q1_D3D4_zt52 = (gCout * _Q1_D3D4_zt50);
    const auto _Q1_D3D4_zt78 = (gC6 * _Q1_D3D4_zt21);
    const auto _Q1_D3D4_zt94 = (gR17C12 * _Q1_D3D4_zt21);
    const auto _Q1_D3D4_zt4 = (_Q1_D3D4_zt8 * _Q1_D3D4_zt50);
    const auto _Q1_D3D4_zt7 = (_Q1_D3D4_zt29 + gC6);
    const auto _Q1_D3D4_zt12 = (_Q1_D3D4_zt7 + gR17C12);
    const auto _Q1_D3D4_zt18 = (_Q1_D3D4_zt19 + gR17C12);
    const auto _Q1_D3D4_zt25 = (_Q1_D3D4_zt12 * _Q1_D3D4_zt31);
    const auto _Q1_D3D4_zt28 = (gC6 * _Q1_D3D4_zt31);
    const auto _Q1_D3D4_zt30 = (gR17C12 * _Q1_D3D4_zt31);
    const auto _Q1_D3D4_zt33 = (_Q1_D3D4_zt8 * _Q1_D3D4_zt94);
    const auto _Q1_D3D4_zt39 = (_Q1_D3D4_zt28 - _Q1_D3D4_zt25);
    const auto _Q1_D3D4_zt41 = (_Q1_D3D4_zt8 * _Q1_D3D4_zt30);
    const auto _Q1_D3D4_zt43 = (_Q1_D3D4_zt8 * _Q1_D3D4_zt46);
    const auto _Q1_D3D4_zt45 = (gCout * _Q1_D3D4_zt46);
    const auto _Q1_D3D4_zt47 = (_Q1_D3D4_zt33 * 1.0);
    const auto _Q1_D3D4_zt49 = (_Q1_D3D4_zt18 * _Q1_D3D4_zt4);
    const auto _Q1_D3D4_zt51 = (gCout * _Q1_D3D4_zt52);
    const auto _Q1_D3D4_zt53 = (_Q1_D3D4_zt12 * _Q1_D3D4_zt21);
    const auto _Q1_D3D4_zt55 = (gR17C12 * _Q1_D3D4_zt4);
    const auto _Q1_D3D4_zt56 = (_Q1_D3D4_zt33 * _Q1_D3D4_zt35);
    const auto _Q1_D3D4_zt59 = (_Q1_D3D4_zt12 * _Q1_D3D4_zt16);
    const auto _Q1_D3D4_zt1 = (_Q1_D3D4_zt49 - _Q1_D3D4_zt51);
    const auto _Q1_D3D4_zt6 = (_Q1_D3D4_zt78 - _Q1_D3D4_zt53);
    const auto _Q1_D3D4_zt10 = (-_Q1_D3D4_zt56);
    const auto _Q1_D3D4_zt17 = (gR17C12 * _Q1_D3D4_zt43);
    const auto _Q1_D3D4_zt22 = (gCout * _Q1_D3D4_zt6);
    const auto _Q1_D3D4_zt27 = (gCout * _Q1_D3D4_zt39);
    const auto _Q1_D3D4_zt32 = (gR17C12 * _Q1_D3D4_zt33);
    const auto _Q1_D3D4_zt38 = (_Q1_D3D4_zt8 * _Q1_D3D4_zt39);
    const auto _Q1_D3D4_zt40 = (gR17C12 * _Q1_D3D4_zt41);
    const auto _Q1_D3D4_zt42 = (_Q1_D3D4_zt18 * _Q1_D3D4_zt43);
    const auto _Q1_D3D4_zt44 = (gCout * _Q1_D3D4_zt45);
    const auto _Q1_D3D4_zt58 = (_Q1_D3D4_zt8 * _Q1_D3D4_zt6);
    const auto _Q1_D3D4_zt62 = (_Q1_D3D4_zt58 * 1.0);
    const auto _Q1_D3D4_zt3 = (gCout * _Q1_D3D4_zt22);
    const auto _Q1_D3D4_zt11 = (_Q1_D3D4_zt58 * _Q1_D3D4_zt35);
    const auto _Q1_D3D4_zt15 = (_Q1_D3D4_zt42 - _Q1_D3D4_zt44);
    const auto _Q1_D3D4_zt20 = (_Q1_D3D4_zt18 * _Q1_D3D4_zt58);
    const auto _Q1_D3D4_zt24 = (_Q1_D3D4_zt18 * _Q1_D3D4_zt38);
    const auto _Q1_D3D4_zt26 = (gCout * _Q1_D3D4_zt27);
    const auto _Q1_D3D4_zt37 = (_Q1_D3D4_zt24 - _Q1_D3D4_zt26);
    const auto _Q1_D3D4_zt61 = (_Q1_D3D4_zt62 + _Q1_D3D4_zt17);
    const auto _Q1_D3D4_zt2 = (_Q1_D3D4_zt20 - _Q1_D3D4_zt3);
    const auto _Q1_D3D4_zt5 = (_Q1_D3D4_zt2 + _Q1_D3D4_zt32);
    const auto _Q1_D3D4_zt14 = (_Q1_D3D4_zt15 - _Q1_D3D4_zt47);
    const auto _Q1_D3D4_zt34 = (_Q1_D3D4_zt10 / _Q1_D3D4_zt5);
    const auto _Q1_D3D4_zt36 = (_Q1_D3D4_zt37 + _Q1_D3D4_zt40);
    const auto _Q1_D3D4_zt54 = (_Q1_D3D4_zt55 / _Q1_D3D4_zt5);
    const auto _Q1_D3D4_zt57 = (_Q1_D3D4_zt11 / _Q1_D3D4_zt5);
    const auto _Q1_D3D4_zt60 = (_Q1_D3D4_zt61 / _Q1_D3D4_zt5);
    const auto _Q1_D3D4_zt0 = (_Q1_D3D4_zt1 / _Q1_D3D4_zt5);
    const auto _Q1_D3D4_zt9 = (_Q1_D3D4_zt34 - _Q1_D3D4_zt57);
    const auto _Q1_D3D4_zt13 = (_Q1_D3D4_zt14 / _Q1_D3D4_zt5);
    const auto _Q1_D3D4_zt23 = (_Q1_D3D4_zt36 / _Q1_D3D4_zt5);
    const auto _Q1_D3D4_zt48 = (_Q1_D3D4_zt0 - _Q1_D3D4_zt54);
    const auto _Q1_D3D4_Z0_0 = _Q1_D3D4_zt48;
    const auto _Q1_D3D4_Z0_1 = _Q1_D3D4_zt9;
    const auto _Q1_D3D4_Z0_2 = (_Q1_D3D4_zt13 - _Q1_D3D4_zt60);
    const auto _Q1_D3D4_Z1_0 = (_Q1_D3D4_zt0 + (-_Q1_D3D4_zt23));
    const auto _Q1_D3D4_Z1_1 = (_Q1_D3D4_zt34 - _Q1_D3D4_zt23);
    const auto _Q1_D3D4_Z1_2 = _Q1_D3D4_zt13;
    const auto _Q1_D3D4_Z2_0 = _Q1_D3D4_zt48;
    const auto _Q1_D3D4_Z2_1 = _Q1_D3D4_zt9;
    const auto _Q1_D3D4_Z2_2 = ((((((_Q1_D3D4_zt18 * (_Q1_D3D4_zt8 * _Q1_D3D4_zt59)) - (gCout * (gCout * _Q1_D3D4_zt59))) - (gR17C12 * (_Q1_D3D4_zt8 * (gR17C12 * _Q1_D3D4_zt16)))) - _Q1_D3D4_zt47) / _Q1_D3D4_zt5) - _Q1_D3D4_zt60);
    double c0__Q1_D3D4_voc2;
    double c__Q1_D3D4_voc2[5];
    double c0__Q1_D3D4_voc0;
    double c__Q1_D3D4_voc0[5];
    double c0__Q1_D3D4_zt85;
    double c__Q1_D3D4_zt85[5];
    double c0__Q1_D3D4_zt101;
    double c__Q1_D3D4_zt101[5];
    double c0__Q1_D3D4_voc1;
    double c__Q1_D3D4_voc1[5];
    
    for (int _k = 0; _k <= 5; ++_k)
    {
        const auto vi = (_k == 1) ? 1.0 : 0.0;
        const auto zCout = (_k == 2) ? 1.0 : 0.0;
        const auto zR19C5 = (_k == 3) ? 1.0 : 0.0;
        const auto zC6 = (_k == 4) ? 1.0 : 0.0;
        const auto zR17C12 = (_k == 5) ? 1.0 : 0.0;
        const auto _Q1_D3D4_zt65 = (gR19C5 * vi);
        const auto _Q1_D3D4_zt69 = (_Q1_D3D4_zt65 + zR19C5);
        const auto _Q1_D3D4_zt75 = (zCout - zR17C12);
        const auto _Q1_D3D4_zt83 = (Vp * gC6);
        const auto _Q1_D3D4_zt89 = (_Q1_D3D4_zt69 + zC6);
        const auto _Q1_D3D4_zt92 = (_Q1_D3D4_zt89 + zR17C12);
        const auto _Q1_D3D4_zt93 = (gR21 * zC6);
        const auto _Q1_D3D4_zt99 = (zCout * _Q1_D3D4_zt94);
        const auto _Q1_D3D4_zt105 = (gR21 * _Q1_D3D4_zt83);
        const auto _Q1_D3D4_zt66 = (gC6 * _Q1_D3D4_zt93);
        const auto _Q1_D3D4_zt80 = (-_Q1_D3D4_zt92);
        const auto _Q1_D3D4_zt84 = (_Q1_D3D4_zt12 * _Q1_D3D4_zt105);
        const auto _Q1_D3D4_zt88 = (_Q1_D3D4_zt21 * _Q1_D3D4_zt80);
        const auto _Q1_D3D4_zt95 = (_Q1_D3D4_zt33 * _Q1_D3D4_zt75);
        const auto _Q1_D3D4_zt97 = (_Q1_D3D4_zt12 * _Q1_D3D4_zt93);
        const auto _Q1_D3D4_zt98 = (_Q1_D3D4_zt97 + _Q1_D3D4_zt88);
        const auto _Q1_D3D4_zt100 = (gR17C12 * _Q1_D3D4_zt105);
        const auto _Q1_D3D4_zt104 = (gC6 * _Q1_D3D4_zt105);
        const auto _Q1_D3D4_zt68 = (_Q1_D3D4_zt88 + _Q1_D3D4_zt66);
        const auto _Q1_D3D4_zt74 = (_Q1_D3D4_zt58 * _Q1_D3D4_zt75);
        const auto _Q1_D3D4_zt77 = (zCout * _Q1_D3D4_zt6);
        const auto _Q1_D3D4_zt79 = (_Q1_D3D4_zt8 * _Q1_D3D4_zt68);
        const auto _Q1_D3D4_zt82 = (_Q1_D3D4_zt104 - _Q1_D3D4_zt84);
        const auto _Q1_D3D4_zt87 = (_Q1_D3D4_zt18 * _Q1_D3D4_zt79);
        const auto _Q1_D3D4_zt91 = (gCout * _Q1_D3D4_zt68);
        const auto _Q1_D3D4_zt96 = (_Q1_D3D4_zt8 * _Q1_D3D4_zt100);
        const auto _Q1_D3D4_zt102 = (gR17C12 * _Q1_D3D4_zt79);
        const auto _Q1_D3D4_zt103 = (_Q1_D3D4_zt8 * _Q1_D3D4_zt82);
        const auto _Q1_D3D4_zt70 = (gR18 * _Q1_D3D4_zt96);
        const auto _Q1_D3D4_zt76 = (gCout * _Q1_D3D4_zt77);
        const auto _Q1_D3D4_zt81 = (gR18 * _Q1_D3D4_zt103);
        const auto _Q1_D3D4_zt90 = (_Q1_D3D4_zt91 - _Q1_D3D4_zt99);
        const auto _Q1_D3D4_zt67 = (gCout * _Q1_D3D4_zt90);
        const auto _Q1_D3D4_zt73 = (_Q1_D3D4_zt74 - _Q1_D3D4_zt76);
        const auto _Q1_D3D4_zt64 = (_Q1_D3D4_zt87 - _Q1_D3D4_zt67);
        const auto _Q1_D3D4_zt72 = (_Q1_D3D4_zt73 + _Q1_D3D4_zt102);
        const auto _Q1_D3D4_zt86 = (_Q1_D3D4_zt64 - _Q1_D3D4_zt95);
        const auto _Q1_D3D4_zt63 = (_Q1_D3D4_zt86 - _Q1_D3D4_zt70);
        const auto _Q1_D3D4_zt71 = (_Q1_D3D4_zt72 + _Q1_D3D4_zt81);
        const auto _Q1_D3D4_zt85 = (_Q1_D3D4_zt63 / _Q1_D3D4_zt5);
        const auto _Q1_D3D4_zt101 = (_Q1_D3D4_zt71 / _Q1_D3D4_zt5);
        const auto _Q1_D3D4_voc0 = (_Q1_D3D4_zt85 - _Q1_D3D4_zt101);
        const auto _Q1_D3D4_voc1 = _Q1_D3D4_zt85;
        const auto _Q1_D3D4_voc2 = (((((((_Q1_D3D4_zt18 * (_Q1_D3D4_zt8 * _Q1_D3D4_zt98)) - (gCout * ((gCout * _Q1_D3D4_zt98) - _Q1_D3D4_zt99))) - (gR17C12 * (_Q1_D3D4_zt8 * (gR17C12 * _Q1_D3D4_zt93)))) - _Q1_D3D4_zt70) - _Q1_D3D4_zt95) / _Q1_D3D4_zt5) - _Q1_D3D4_zt101);
        if (_k == 0) {
            c0__Q1_D3D4_voc2 = _Q1_D3D4_voc2;
            c0__Q1_D3D4_voc0 = _Q1_D3D4_voc0;
            c0__Q1_D3D4_zt85 = _Q1_D3D4_zt85;
            c0__Q1_D3D4_zt101 = _Q1_D3D4_zt101;
            c0__Q1_D3D4_voc1 = _Q1_D3D4_voc1;
        } else {
            c__Q1_D3D4_voc2[_k - 1] = _Q1_D3D4_voc2 - c0__Q1_D3D4_voc2;
            c__Q1_D3D4_voc0[_k - 1] = _Q1_D3D4_voc0 - c0__Q1_D3D4_voc0;
            c__Q1_D3D4_zt85[_k - 1] = _Q1_D3D4_zt85 - c0__Q1_D3D4_zt85;
            c__Q1_D3D4_zt101[_k - 1] = _Q1_D3D4_zt101 - c0__Q1_D3D4_zt101;
            c__Q1_D3D4_voc1[_k - 1] = _Q1_D3D4_voc1 - c0__Q1_D3D4_voc1;
        }
    }
    
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

            // --- Newton-Raphson solve (N-port): Q1_D3D4
            const auto _Q1_D3D4_voc2 = c0__Q1_D3D4_voc2 + c__Q1_D3D4_voc2[0] * vi + c__Q1_D3D4_voc2[1] * zCout + c__Q1_D3D4_voc2[2] * zR19C5 + c__Q1_D3D4_voc2[3] * zC6 + c__Q1_D3D4_voc2[4] * zR17C12;
            const auto _Q1_D3D4_voc0 = c0__Q1_D3D4_voc0 + c__Q1_D3D4_voc0[0] * vi + c__Q1_D3D4_voc0[1] * zCout + c__Q1_D3D4_voc0[2] * zR19C5 + c__Q1_D3D4_voc0[3] * zC6 + c__Q1_D3D4_voc0[4] * zR17C12;
            const auto _Q1_D3D4_zt85 = c0__Q1_D3D4_zt85 + c__Q1_D3D4_zt85[0] * vi + c__Q1_D3D4_zt85[1] * zCout + c__Q1_D3D4_zt85[2] * zR19C5 + c__Q1_D3D4_zt85[3] * zC6 + c__Q1_D3D4_zt85[4] * zR17C12;
            const auto _Q1_D3D4_zt101 = c0__Q1_D3D4_zt101 + c__Q1_D3D4_zt101[0] * vi + c__Q1_D3D4_zt101[1] * zCout + c__Q1_D3D4_zt101[2] * zR19C5 + c__Q1_D3D4_zt101[3] * zC6 + c__Q1_D3D4_zt101[4] * zR17C12;
            const auto _Q1_D3D4_voc1 = c0__Q1_D3D4_voc1 + c__Q1_D3D4_voc1[0] * vi + c__Q1_D3D4_voc1[1] * zCout + c__Q1_D3D4_voc1[2] * zR19C5 + c__Q1_D3D4_voc1[3] * zC6 + c__Q1_D3D4_voc1[4] * zR17C12;
            const auto _Q1_D3D4_zt65 = (gR19C5 * vi);
            const auto _Q1_D3D4_zt69 = (_Q1_D3D4_zt65 + zR19C5);
            const auto _Q1_D3D4_zt75 = (zCout - zR17C12);
            const auto _Q1_D3D4_zt83 = (Vp * gC6);
            const auto _Q1_D3D4_zt89 = (_Q1_D3D4_zt69 + zC6);
            const auto _Q1_D3D4_zt92 = (_Q1_D3D4_zt89 + zR17C12);
            const auto _Q1_D3D4_zt93 = (gR21 * zC6);
            const auto _Q1_D3D4_zt99 = (zCout * _Q1_D3D4_zt94);
            const auto _Q1_D3D4_zt105 = (gR21 * _Q1_D3D4_zt83);
            const auto _Q1_D3D4_zt66 = (gC6 * _Q1_D3D4_zt93);
            const auto _Q1_D3D4_zt80 = (-_Q1_D3D4_zt92);
            const auto _Q1_D3D4_zt84 = (_Q1_D3D4_zt12 * _Q1_D3D4_zt105);
            const auto _Q1_D3D4_zt88 = (_Q1_D3D4_zt21 * _Q1_D3D4_zt80);
            const auto _Q1_D3D4_zt95 = (_Q1_D3D4_zt33 * _Q1_D3D4_zt75);
            const auto _Q1_D3D4_zt97 = (_Q1_D3D4_zt12 * _Q1_D3D4_zt93);
            const auto _Q1_D3D4_zt98 = (_Q1_D3D4_zt97 + _Q1_D3D4_zt88);
            const auto _Q1_D3D4_zt100 = (gR17C12 * _Q1_D3D4_zt105);
            const auto _Q1_D3D4_zt104 = (gC6 * _Q1_D3D4_zt105);
            const auto _Q1_D3D4_zt68 = (_Q1_D3D4_zt88 + _Q1_D3D4_zt66);
            const auto _Q1_D3D4_zt74 = (_Q1_D3D4_zt58 * _Q1_D3D4_zt75);
            const auto _Q1_D3D4_zt77 = (zCout * _Q1_D3D4_zt6);
            const auto _Q1_D3D4_zt79 = (_Q1_D3D4_zt8 * _Q1_D3D4_zt68);
            const auto _Q1_D3D4_zt82 = (_Q1_D3D4_zt104 - _Q1_D3D4_zt84);
            const auto _Q1_D3D4_zt87 = (_Q1_D3D4_zt18 * _Q1_D3D4_zt79);
            const auto _Q1_D3D4_zt91 = (gCout * _Q1_D3D4_zt68);
            const auto _Q1_D3D4_zt96 = (_Q1_D3D4_zt8 * _Q1_D3D4_zt100);
            const auto _Q1_D3D4_zt102 = (gR17C12 * _Q1_D3D4_zt79);
            const auto _Q1_D3D4_zt103 = (_Q1_D3D4_zt8 * _Q1_D3D4_zt82);
            const auto _Q1_D3D4_zt70 = (gR18 * _Q1_D3D4_zt96);
            const auto _Q1_D3D4_zt76 = (gCout * _Q1_D3D4_zt77);
            const auto _Q1_D3D4_zt81 = (gR18 * _Q1_D3D4_zt103);
            const auto _Q1_D3D4_zt90 = (_Q1_D3D4_zt91 - _Q1_D3D4_zt99);
            const auto _Q1_D3D4_zt67 = (gCout * _Q1_D3D4_zt90);
            const auto _Q1_D3D4_zt73 = (_Q1_D3D4_zt74 - _Q1_D3D4_zt76);
            const auto _Q1_D3D4_zt64 = (_Q1_D3D4_zt87 - _Q1_D3D4_zt67);
            const auto _Q1_D3D4_zt72 = (_Q1_D3D4_zt73 + _Q1_D3D4_zt102);
            const auto _Q1_D3D4_zt86 = (_Q1_D3D4_zt64 - _Q1_D3D4_zt95);
            const auto _Q1_D3D4_zt63 = (_Q1_D3D4_zt86 - _Q1_D3D4_zt70);
            const auto _Q1_D3D4_zt71 = (_Q1_D3D4_zt72 + _Q1_D3D4_zt81);
            const auto _Q1_D3D4_pt38 = (1.0 / D1N914_vt);
            for (int newton_iter = 0; newton_iter < newton_max_iter; ++newton_iter)
            {
                const auto _Q1_D3D4_pt1 = (vBEQ1 / Q2N5089_vt);
                const auto _Q1_D3D4_pt4 = (vBCQ1 / Q2N5089_vt);
                const auto _Q1_D3D4_pt6 = (vD3D4 / D1N914_vt);
                const auto _Q1_D3D4_pt9 = math_exp_approx(_Q1_D3D4_pt4);
                const auto _Q1_D3D4_pt0 = math_exp_approx(_Q1_D3D4_pt1);
                const auto _Q1_D3D4_pt3 = (_Q1_D3D4_pt9 - 1.0);
                const auto _Q1_D3D4_pt5 = math_exp_approx(_Q1_D3D4_pt6);
                const auto _Q1_D3D4_pt8 = (_Q1_D3D4_pt9 / Q2N5089_vt);
                const auto _Q1_D3D4_pt10 = (Q2N5089_Is * _Q1_D3D4_pt0);
                const auto _Q1_D3D4_pt2 = (_Q1_D3D4_pt3 / Q2N5089_BetaR);
                const auto _Q1_D3D4_pt7 = (_Q1_D3D4_pt10 / Q2N5089_vt);
                const auto _Q1_D3D4_i0 = (Q2N5089_Is * (((_Q1_D3D4_pt0 - 1.0) / Q2N5089_BetaF) + _Q1_D3D4_pt2));
                const auto _Q1_D3D4_i1 = (Q2N5089_Is * ((_Q1_D3D4_pt0 - _Q1_D3D4_pt9) - _Q1_D3D4_pt2));
                const auto _Q1_D3D4_i2 = (D1N914_Is * (_Q1_D3D4_pt5 - (1.0 / _Q1_D3D4_pt5)));
                const auto _Q1_D3D4_g0_0 = (((Q2N5089_Is * _Q1_D3D4_pt9) / Q2N5089_vt) / Q2N5089_BetaR);
                const auto _Q1_D3D4_g0_1 = (_Q1_D3D4_pt7 / Q2N5089_BetaF);
                const auto _Q1_D3D4_g1_0 = (-(Q2N5089_Is * (_Q1_D3D4_pt8 + (_Q1_D3D4_pt8 / Q2N5089_BetaR))));
                const auto _Q1_D3D4_g1_1 = _Q1_D3D4_pt7;
                const auto _Q1_D3D4_g2_2 = (D1N914_Is * ((_Q1_D3D4_pt5 / D1N914_vt) + (_Q1_D3D4_pt38 / _Q1_D3D4_pt5)));
                const auto _Q1_D3D4_pt11 = (((_Q1_D3D4_voc0 + (_Q1_D3D4_Z0_0 * _Q1_D3D4_i0)) + (_Q1_D3D4_Z0_1 * _Q1_D3D4_i1)) + (_Q1_D3D4_Z0_2 * _Q1_D3D4_i2));
                const auto _Q1_D3D4_pt12 = (((_Q1_D3D4_voc1 + (_Q1_D3D4_Z1_0 * _Q1_D3D4_i0)) + (_Q1_D3D4_Z1_1 * _Q1_D3D4_i1)) + (_Q1_D3D4_Z1_2 * _Q1_D3D4_i2));
                const auto _Q1_D3D4_pt13 = (((_Q1_D3D4_voc2 + (_Q1_D3D4_Z2_0 * _Q1_D3D4_i0)) + (_Q1_D3D4_Z2_1 * _Q1_D3D4_i1)) + (_Q1_D3D4_Z2_2 * _Q1_D3D4_i2));
                const auto _Q1_D3D4_pt14 = (vBCQ1 - _Q1_D3D4_pt11);
                const auto _Q1_D3D4_pt16 = (((_Q1_D3D4_Z1_0 * _Q1_D3D4_g0_1) + (_Q1_D3D4_Z1_1 * _Q1_D3D4_g1_1)) - 1.0);
                const auto _Q1_D3D4_pt17 = ((_Q1_D3D4_Z2_2 * _Q1_D3D4_g2_2) - 1.0);
                const auto _Q1_D3D4_pt18 = (_Q1_D3D4_Z1_2 * _Q1_D3D4_g2_2);
                const auto _Q1_D3D4_pt19 = ((_Q1_D3D4_Z2_0 * _Q1_D3D4_g0_1) + (_Q1_D3D4_Z2_1 * _Q1_D3D4_g1_1));
                const auto _Q1_D3D4_pt20 = ((_Q1_D3D4_Z0_0 * _Q1_D3D4_g0_1) + (_Q1_D3D4_Z0_1 * _Q1_D3D4_g1_1));
                const auto _Q1_D3D4_pt23 = (vBEQ1 - _Q1_D3D4_pt12);
                const auto _Q1_D3D4_pt24 = (vD3D4 - _Q1_D3D4_pt13);
                const auto _Q1_D3D4_pt25 = (_Q1_D3D4_Z0_2 * _Q1_D3D4_g2_2);
                const auto _Q1_D3D4_pt27 = (((_Q1_D3D4_Z0_0 * _Q1_D3D4_g0_0) + (_Q1_D3D4_Z0_1 * _Q1_D3D4_g1_0)) - 1.0);
                const auto _Q1_D3D4_pt29 = ((_Q1_D3D4_Z1_0 * _Q1_D3D4_g0_0) + (_Q1_D3D4_Z1_1 * _Q1_D3D4_g1_0));
                const auto _Q1_D3D4_pt30 = ((_Q1_D3D4_Z2_0 * _Q1_D3D4_g0_0) + (_Q1_D3D4_Z2_1 * _Q1_D3D4_g1_0));
                const auto _Q1_D3D4_pt31 = ((_Q1_D3D4_pt29 * _Q1_D3D4_pt19) - (_Q1_D3D4_pt16 * _Q1_D3D4_pt30));
                const auto _Q1_D3D4_pt32 = (_Q1_D3D4_pt18 * _Q1_D3D4_pt24);
                const auto _Q1_D3D4_pt34 = (_Q1_D3D4_pt23 * _Q1_D3D4_pt30);
                const auto _Q1_D3D4_pt35 = (_Q1_D3D4_pt16 * _Q1_D3D4_pt24);
                const auto _Q1_D3D4_pt36 = (_Q1_D3D4_pt23 * _Q1_D3D4_pt19);
                const auto _Q1_D3D4_pt37 = (_Q1_D3D4_pt29 * _Q1_D3D4_pt24);
                const auto _Q1_D3D4_pt15 = ((_Q1_D3D4_pt16 * _Q1_D3D4_pt17) - (_Q1_D3D4_pt18 * _Q1_D3D4_pt19));
                const auto _Q1_D3D4_pt22 = (_Q1_D3D4_pt23 * _Q1_D3D4_pt17);
                const auto _Q1_D3D4_pt28 = ((_Q1_D3D4_pt29 * _Q1_D3D4_pt17) - (_Q1_D3D4_pt18 * _Q1_D3D4_pt30));
                const auto _Q1_D3D4_pt33 = (_Q1_D3D4_pt37 - _Q1_D3D4_pt34);
                const auto _Q1_D3D4_pt21 = (_Q1_D3D4_pt22 - _Q1_D3D4_pt32);
                const auto _Q1_D3D4_pt26 = (((_Q1_D3D4_pt27 * _Q1_D3D4_pt15) - (_Q1_D3D4_pt20 * _Q1_D3D4_pt28)) + (_Q1_D3D4_pt25 * _Q1_D3D4_pt31));
                const auto res_vBCQ1 = (_Q1_D3D4_pt11 - vBCQ1);
                const auto res_vBEQ1 = (_Q1_D3D4_pt12 - vBEQ1);
                const auto res_vD3D4 = (_Q1_D3D4_pt13 - vD3D4);
                const auto delta_vBCQ1 = ((((_Q1_D3D4_pt14 * _Q1_D3D4_pt15) - (_Q1_D3D4_pt20 * _Q1_D3D4_pt21)) + (_Q1_D3D4_pt25 * (_Q1_D3D4_pt36 - _Q1_D3D4_pt35))) / _Q1_D3D4_pt26);
                const auto delta_vBEQ1 = ((((_Q1_D3D4_pt27 * _Q1_D3D4_pt21) - (_Q1_D3D4_pt14 * _Q1_D3D4_pt28)) + (_Q1_D3D4_pt25 * _Q1_D3D4_pt33)) / _Q1_D3D4_pt26);
                const auto delta_vD3D4 = ((((_Q1_D3D4_pt27 * (_Q1_D3D4_pt35 - _Q1_D3D4_pt36)) - (_Q1_D3D4_pt20 * _Q1_D3D4_pt33)) + (_Q1_D3D4_pt14 * _Q1_D3D4_pt31)) / _Q1_D3D4_pt26);
            
                auto residual_norm_sq = 0.0;
                residual_norm_sq += res_vBCQ1 * res_vBCQ1;
                residual_norm_sq += res_vBEQ1 * res_vBEQ1;
                residual_norm_sq += res_vD3D4 * res_vD3D4;
                auto step_norm_sq = 0.0;
                step_norm_sq += delta_vBCQ1 * delta_vBCQ1;
                step_norm_sq += delta_vBEQ1 * delta_vBEQ1;
                step_norm_sq += delta_vD3D4 * delta_vD3D4;
            
                vBCQ1 = limit_junction_voltage(vBCQ1 + (delta_vBCQ1), vBCQ1, Q2N5089_vt, vcrit_Q2N5089_vt);
                vBEQ1 = limit_junction_voltage(vBEQ1 + (delta_vBEQ1), vBEQ1, Q2N5089_vt, vcrit_Q2N5089_vt);
                vD3D4 = limit_junction_voltage_sym(vD3D4 + (delta_vD3D4), vD3D4, D1N914_vt, vcrit_D1N914_vt);
            
                if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
                    break;
                
            }

            const auto _t1 = math_exp_approx((vBEQ1 / Q2N5089_vt));
            const auto _t2 = math_exp_approx((vBCQ1 / Q2N5089_vt));
            const auto _t3 = (_t2 - 1.0);
            const auto _t5 = math_exp_approx((vD3D4 / D1N914_vt));
            const auto _t13 = (_t3 / Q2N5089_BetaR);
            const auto _t4 = (D1N914_Is * (_t5 - (1.0 / _t5)));
            const auto _t12 = ((Q2N5089_Is * (((_t1 - 1.0) / Q2N5089_BetaF) + _t13)) - ((((gR19C5 * vi) + zR19C5) + zC6) + zR17C12));
            const auto _t16 = (zCout * _t17);
            const auto _t23 = (zCout * _t7);
            const auto _t33 = (zC6 + _t4);
            const auto _t0 = (((zCout - zR17C12) - (Q2N5089_Is * ((_t1 - _t2) - _t13))) + _t4);
            const auto _t11 = (_t12 * _t9);
            const auto _t15 = (gR21 * _t33);
            const auto _t29 = (_t0 * _t37);
            const auto _t32 = ((_t8 * _t15) + _t11);
            const auto _t14 = (gC6 * _t15);
            const auto _t25 = (_t11 + _t14);
            const auto _t28 = (gCout * _t25);
            const auto _t24 = (_t22 * _t25);
            const auto _t27 = (_t28 - _t16);
            const auto vo = (((((_t0 * _t6) - (_t10 * _t23)) + (gR17C12 * _t27)) + _t38) * _t20);
            const auto vc = (((((_t0 * _t21) - (gCout * _t23)) + (gR17C12 * _t24)) + _t39) * _t20);
            const auto tCout = (gCout * (vc - vo));
            const auto vb = (((((_t10 * _t24) - (gCout * _t27)) - _t29) - _t34) * _t20);
            const auto vR19C5 = (vb - vi);
            const auto vd = ((((((_t10 * (_t22 * _t32)) - (gCout * ((gCout * _t32) - _t16))) - (gR17C12 * (_t22 * (gR17C12 * _t15)))) - _t34) - _t29) * _t20);
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

static float reset (Params params, State* state, int num_channels, float sample_rate, float vi_dc = 0.0f)
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

    const auto _Q1_D3D4_zt13 = (-1.0);
    const auto _Q1_D3D4_zt20 = (-1.0 / R17);
    const auto _Q1_D3D4_zt22 = (_Q1_D3D4_zt20 - gR18);
    const auto _Q1_D3D4_zt35 = (1.0 / R17);
    const auto _Q1_D3D4_zt45 = (1.0 / 1000000000.0);
    const auto _Q1_D3D4_zt53 = (gR20 + _Q1_D3D4_zt35);
    const auto _Q1_D3D4_zt3 = (gR21 + _Q1_D3D4_zt45);
    const auto _Q1_D3D4_zt5 = (_Q1_D3D4_zt3 * 1.0);
    const auto _Q1_D3D4_zt16 = (_Q1_D3D4_zt53 + _Q1_D3D4_zt45);
    const auto _Q1_D3D4_zt21 = (_Q1_D3D4_zt16 * 1.0);
    const auto _Q1_D3D4_zt39 = (_Q1_D3D4_zt22 - _Q1_D3D4_zt45);
    const auto _Q1_D3D4_zt40 = (gRout + _Q1_D3D4_zt45);
    const auto _Q1_D3D4_zt1 = (_Q1_D3D4_zt40 * _Q1_D3D4_zt5);
    const auto _Q1_D3D4_zt4 = (_Q1_D3D4_zt39 * _Q1_D3D4_zt1);
    const auto _Q1_D3D4_zt7 = (_Q1_D3D4_zt16 * _Q1_D3D4_zt3);
    const auto _Q1_D3D4_zt23 = (_Q1_D3D4_zt40 * _Q1_D3D4_zt7);
    const auto _Q1_D3D4_zt28 = (_Q1_D3D4_zt40 * 1.0);
    const auto _Q1_D3D4_zt29 = (_Q1_D3D4_zt40 * _Q1_D3D4_zt21);
    const auto _Q1_D3D4_zt33 = (_Q1_D3D4_zt1 / R17);
    const auto _Q1_D3D4_zt36 = (-_Q1_D3D4_zt33);
    const auto _Q1_D3D4_zt38 = (_Q1_D3D4_zt40 * _Q1_D3D4_zt3);
    const auto _Q1_D3D4_zt41 = (_Q1_D3D4_zt23 * _Q1_D3D4_zt13);
    const auto _Q1_D3D4_zt43 = (-_Q1_D3D4_zt41);
    const auto _Q1_D3D4_zt2 = (_Q1_D3D4_zt38 / R17);
    const auto _Q1_D3D4_zt6 = (_Q1_D3D4_zt39 * _Q1_D3D4_zt23);
    const auto _Q1_D3D4_zt10 = (_Q1_D3D4_zt23 * 1.0);
    const auto _Q1_D3D4_zt12 = (_Q1_D3D4_zt38 * 1.0);
    const auto _Q1_D3D4_zt14 = (_Q1_D3D4_zt38 * _Q1_D3D4_zt13);
    const auto _Q1_D3D4_zt19 = (_Q1_D3D4_zt39 * _Q1_D3D4_zt29);
    const auto _Q1_D3D4_zt25 = (_Q1_D3D4_zt12 / R17);
    const auto _Q1_D3D4_zt27 = (_Q1_D3D4_zt28 / R17);
    const auto _Q1_D3D4_zt32 = (_Q1_D3D4_zt14 / R17);
    const auto _Q1_D3D4_zt34 = (_Q1_D3D4_zt2 / R17);
    const auto _Q1_D3D4_zt37 = (_Q1_D3D4_zt34 + _Q1_D3D4_zt6);
    const auto _Q1_D3D4_zt42 = (_Q1_D3D4_zt43 / _Q1_D3D4_zt37);
    const auto _Q1_D3D4_zt0 = (_Q1_D3D4_zt36 / _Q1_D3D4_zt37);
    const auto _Q1_D3D4_zt9 = (-_Q1_D3D4_zt10);
    const auto _Q1_D3D4_zt15 = (_Q1_D3D4_zt4 / _Q1_D3D4_zt37);
    const auto _Q1_D3D4_zt24 = (-_Q1_D3D4_zt25);
    const auto _Q1_D3D4_zt26 = (_Q1_D3D4_zt27 / R17);
    const auto _Q1_D3D4_zt31 = (-_Q1_D3D4_zt32);
    const auto _Q1_D3D4_zt8 = (_Q1_D3D4_zt9 / _Q1_D3D4_zt37);
    const auto _Q1_D3D4_zt11 = (_Q1_D3D4_zt24 / _Q1_D3D4_zt37);
    const auto _Q1_D3D4_zt18 = (_Q1_D3D4_zt26 + _Q1_D3D4_zt19);
    const auto _Q1_D3D4_zt30 = (_Q1_D3D4_zt31 / _Q1_D3D4_zt37);
    const auto _Q1_D3D4_zt17 = (_Q1_D3D4_zt18 / _Q1_D3D4_zt37);
    const auto _Q1_D3D4_Z0_0 = (_Q1_D3D4_zt0 - _Q1_D3D4_zt15);
    const auto _Q1_D3D4_Z0_1 = (_Q1_D3D4_zt8 - _Q1_D3D4_zt11);
    const auto _Q1_D3D4_Z0_2 = (_Q1_D3D4_zt42 - _Q1_D3D4_zt30);
    const auto _Q1_D3D4_Z1_0 = ((-_Q1_D3D4_zt15) + (-_Q1_D3D4_zt17));
    const auto _Q1_D3D4_Z1_1 = (-(_Q1_D3D4_zt11 + _Q1_D3D4_zt17));
    const auto _Q1_D3D4_Z1_2 = (-_Q1_D3D4_zt30);
    const auto _Q1_D3D4_Z2_0 = (_Q1_D3D4_zt0 - _Q1_D3D4_zt0);
    const auto _Q1_D3D4_Z2_1 = (_Q1_D3D4_zt8 - _Q1_D3D4_zt8);
    const auto _Q1_D3D4_Z2_2 = (_Q1_D3D4_zt42 - _Q1_D3D4_zt42);
    // --- Newton-Raphson solve (N-port): Q1_D3D4
    const auto _Q1_D3D4_zt49 = (_Q1_D3D4_zt3 * Vp);
    const auto _Q1_D3D4_zt52 = (_Q1_D3D4_zt16 * _Q1_D3D4_zt49);
    const auto _Q1_D3D4_zt54 = (_Q1_D3D4_zt40 * _Q1_D3D4_zt52);
    const auto _Q1_D3D4_zt44 = (_Q1_D3D4_zt40 * _Q1_D3D4_zt49);
    const auto _Q1_D3D4_zt48 = (gR18 * _Q1_D3D4_zt44);
    const auto _Q1_D3D4_zt51 = (gR18 * _Q1_D3D4_zt54);
    const auto _Q1_D3D4_zt47 = (_Q1_D3D4_zt48 / R17);
    const auto _Q1_D3D4_zt50 = (_Q1_D3D4_zt51 / _Q1_D3D4_zt37);
    const auto _Q1_D3D4_zt46 = (_Q1_D3D4_zt47 / _Q1_D3D4_zt37);
    const auto _Q1_D3D4_voc0 = (_Q1_D3D4_zt50 - _Q1_D3D4_zt46);
    const auto _Q1_D3D4_voc1 = (-_Q1_D3D4_zt46);
    const auto _Q1_D3D4_voc2 = (_Q1_D3D4_zt50 - _Q1_D3D4_zt50);
    const auto _Q1_D3D4_pt38 = (1.0 / D1N914_vt);
    for (int newton_iter = 0; newton_iter < 10000; ++newton_iter)
    {
        const auto _Q1_D3D4_pt1 = (vBEQ1 / Q2N5089_vt);
        const auto _Q1_D3D4_pt4 = (vBCQ1 / Q2N5089_vt);
        const auto _Q1_D3D4_pt6 = (vD3D4 / D1N914_vt);
        const auto _Q1_D3D4_pt9 = math_exp_approx(_Q1_D3D4_pt4);
        const auto _Q1_D3D4_pt0 = math_exp_approx(_Q1_D3D4_pt1);
        const auto _Q1_D3D4_pt3 = (_Q1_D3D4_pt9 - 1.0);
        const auto _Q1_D3D4_pt5 = math_exp_approx(_Q1_D3D4_pt6);
        const auto _Q1_D3D4_pt8 = (_Q1_D3D4_pt9 / Q2N5089_vt);
        const auto _Q1_D3D4_pt10 = (Q2N5089_Is * _Q1_D3D4_pt0);
        const auto _Q1_D3D4_pt2 = (_Q1_D3D4_pt3 / Q2N5089_BetaR);
        const auto _Q1_D3D4_pt7 = (_Q1_D3D4_pt10 / Q2N5089_vt);
        const auto _Q1_D3D4_i0 = (Q2N5089_Is * (((_Q1_D3D4_pt0 - 1.0) / Q2N5089_BetaF) + _Q1_D3D4_pt2));
        const auto _Q1_D3D4_i1 = (Q2N5089_Is * ((_Q1_D3D4_pt0 - _Q1_D3D4_pt9) - _Q1_D3D4_pt2));
        const auto _Q1_D3D4_i2 = (D1N914_Is * (_Q1_D3D4_pt5 - (1.0 / _Q1_D3D4_pt5)));
        const auto _Q1_D3D4_g0_0 = (((Q2N5089_Is * _Q1_D3D4_pt9) / Q2N5089_vt) / Q2N5089_BetaR);
        const auto _Q1_D3D4_g0_1 = (_Q1_D3D4_pt7 / Q2N5089_BetaF);
        const auto _Q1_D3D4_g1_0 = (-(Q2N5089_Is * (_Q1_D3D4_pt8 + (_Q1_D3D4_pt8 / Q2N5089_BetaR))));
        const auto _Q1_D3D4_g1_1 = _Q1_D3D4_pt7;
        const auto _Q1_D3D4_g2_2 = (D1N914_Is * ((_Q1_D3D4_pt5 / D1N914_vt) + (_Q1_D3D4_pt38 / _Q1_D3D4_pt5)));
        const auto _Q1_D3D4_pt11 = (((_Q1_D3D4_voc0 + (_Q1_D3D4_Z0_0 * _Q1_D3D4_i0)) + (_Q1_D3D4_Z0_1 * _Q1_D3D4_i1)) + (_Q1_D3D4_Z0_2 * _Q1_D3D4_i2));
        const auto _Q1_D3D4_pt12 = (((_Q1_D3D4_voc1 + (_Q1_D3D4_Z1_0 * _Q1_D3D4_i0)) + (_Q1_D3D4_Z1_1 * _Q1_D3D4_i1)) + (_Q1_D3D4_Z1_2 * _Q1_D3D4_i2));
        const auto _Q1_D3D4_pt13 = (((_Q1_D3D4_voc2 + (_Q1_D3D4_Z2_0 * _Q1_D3D4_i0)) + (_Q1_D3D4_Z2_1 * _Q1_D3D4_i1)) + (_Q1_D3D4_Z2_2 * _Q1_D3D4_i2));
        const auto _Q1_D3D4_pt14 = (vBCQ1 - _Q1_D3D4_pt11);
        const auto _Q1_D3D4_pt16 = (((_Q1_D3D4_Z1_0 * _Q1_D3D4_g0_1) + (_Q1_D3D4_Z1_1 * _Q1_D3D4_g1_1)) - 1.0);
        const auto _Q1_D3D4_pt17 = ((_Q1_D3D4_Z2_2 * _Q1_D3D4_g2_2) - 1.0);
        const auto _Q1_D3D4_pt18 = (_Q1_D3D4_Z1_2 * _Q1_D3D4_g2_2);
        const auto _Q1_D3D4_pt19 = ((_Q1_D3D4_Z2_0 * _Q1_D3D4_g0_1) + (_Q1_D3D4_Z2_1 * _Q1_D3D4_g1_1));
        const auto _Q1_D3D4_pt20 = ((_Q1_D3D4_Z0_0 * _Q1_D3D4_g0_1) + (_Q1_D3D4_Z0_1 * _Q1_D3D4_g1_1));
        const auto _Q1_D3D4_pt23 = (vBEQ1 - _Q1_D3D4_pt12);
        const auto _Q1_D3D4_pt24 = (vD3D4 - _Q1_D3D4_pt13);
        const auto _Q1_D3D4_pt25 = (_Q1_D3D4_Z0_2 * _Q1_D3D4_g2_2);
        const auto _Q1_D3D4_pt27 = (((_Q1_D3D4_Z0_0 * _Q1_D3D4_g0_0) + (_Q1_D3D4_Z0_1 * _Q1_D3D4_g1_0)) - 1.0);
        const auto _Q1_D3D4_pt29 = ((_Q1_D3D4_Z1_0 * _Q1_D3D4_g0_0) + (_Q1_D3D4_Z1_1 * _Q1_D3D4_g1_0));
        const auto _Q1_D3D4_pt30 = ((_Q1_D3D4_Z2_0 * _Q1_D3D4_g0_0) + (_Q1_D3D4_Z2_1 * _Q1_D3D4_g1_0));
        const auto _Q1_D3D4_pt31 = ((_Q1_D3D4_pt29 * _Q1_D3D4_pt19) - (_Q1_D3D4_pt16 * _Q1_D3D4_pt30));
        const auto _Q1_D3D4_pt32 = (_Q1_D3D4_pt18 * _Q1_D3D4_pt24);
        const auto _Q1_D3D4_pt34 = (_Q1_D3D4_pt23 * _Q1_D3D4_pt30);
        const auto _Q1_D3D4_pt35 = (_Q1_D3D4_pt16 * _Q1_D3D4_pt24);
        const auto _Q1_D3D4_pt36 = (_Q1_D3D4_pt23 * _Q1_D3D4_pt19);
        const auto _Q1_D3D4_pt37 = (_Q1_D3D4_pt29 * _Q1_D3D4_pt24);
        const auto _Q1_D3D4_pt15 = ((_Q1_D3D4_pt16 * _Q1_D3D4_pt17) - (_Q1_D3D4_pt18 * _Q1_D3D4_pt19));
        const auto _Q1_D3D4_pt22 = (_Q1_D3D4_pt23 * _Q1_D3D4_pt17);
        const auto _Q1_D3D4_pt28 = ((_Q1_D3D4_pt29 * _Q1_D3D4_pt17) - (_Q1_D3D4_pt18 * _Q1_D3D4_pt30));
        const auto _Q1_D3D4_pt33 = (_Q1_D3D4_pt37 - _Q1_D3D4_pt34);
        const auto _Q1_D3D4_pt21 = (_Q1_D3D4_pt22 - _Q1_D3D4_pt32);
        const auto _Q1_D3D4_pt26 = (((_Q1_D3D4_pt27 * _Q1_D3D4_pt15) - (_Q1_D3D4_pt20 * _Q1_D3D4_pt28)) + (_Q1_D3D4_pt25 * _Q1_D3D4_pt31));
        const auto res_vBCQ1 = (_Q1_D3D4_pt11 - vBCQ1);
        const auto res_vBEQ1 = (_Q1_D3D4_pt12 - vBEQ1);
        const auto res_vD3D4 = (_Q1_D3D4_pt13 - vD3D4);
        const auto delta_vBCQ1 = ((((_Q1_D3D4_pt14 * _Q1_D3D4_pt15) - (_Q1_D3D4_pt20 * _Q1_D3D4_pt21)) + (_Q1_D3D4_pt25 * (_Q1_D3D4_pt36 - _Q1_D3D4_pt35))) / _Q1_D3D4_pt26);
        const auto delta_vBEQ1 = ((((_Q1_D3D4_pt27 * _Q1_D3D4_pt21) - (_Q1_D3D4_pt14 * _Q1_D3D4_pt28)) + (_Q1_D3D4_pt25 * _Q1_D3D4_pt33)) / _Q1_D3D4_pt26);
        const auto delta_vD3D4 = ((((_Q1_D3D4_pt27 * (_Q1_D3D4_pt35 - _Q1_D3D4_pt36)) - (_Q1_D3D4_pt20 * _Q1_D3D4_pt33)) + (_Q1_D3D4_pt14 * _Q1_D3D4_pt31)) / _Q1_D3D4_pt26);
    
        auto residual_norm_sq = 0.0;
        residual_norm_sq += res_vBCQ1 * res_vBCQ1;
        residual_norm_sq += res_vBEQ1 * res_vBEQ1;
        residual_norm_sq += res_vD3D4 * res_vD3D4;
        auto step_norm_sq = 0.0;
        step_norm_sq += delta_vBCQ1 * delta_vBCQ1;
        step_norm_sq += delta_vBEQ1 * delta_vBEQ1;
        step_norm_sq += delta_vD3D4 * delta_vD3D4;
    
        vBCQ1 = limit_junction_voltage(vBCQ1 + (delta_vBCQ1), vBCQ1, Q2N5089_vt, vcrit_Q2N5089_vt);
        vBEQ1 = limit_junction_voltage(vBEQ1 + (delta_vBEQ1), vBEQ1, Q2N5089_vt, vcrit_Q2N5089_vt);
        vD3D4 = limit_junction_voltage_sym(vD3D4 + (delta_vD3D4), vD3D4, D1N914_vt, vcrit_D1N914_vt);
    
        if (residual_norm_sq < newton_tol_sq && step_norm_sq < newton_tol_sq)
            break;
        
    }
    const auto zCout = (-((gCout * ((gR18 * ((gRout + (1.0 / 1000000000.0)) * (((gR20 + (1.0 / R17)) + (1.0 / 1000000000.0)) * ((gR21 + (1.0 / 1000000000.0)) * Vp)))) - ((((Q2N5089_Is * ((math_exp_approx((vBEQ1 / Q2N5089_vt)) - math_exp_approx((vBCQ1 / Q2N5089_vt))) - ((math_exp_approx((vBCQ1 / Q2N5089_vt)) - 1.0) / Q2N5089_BetaR))) - (D1N914_Is * (math_exp_approx((vD3D4 / D1N914_vt)) - (1.0 / math_exp_approx((vD3D4 / D1N914_vt)))))) * ((gRout + (1.0 / 1000000000.0)) * (((gR20 + (1.0 / R17)) + (1.0 / 1000000000.0)) * (gR21 + (1.0 / 1000000000.0))))) + (((gRout + (1.0 / 1000000000.0)) * ((Q2N5089_Is * (((math_exp_approx((vBEQ1 / Q2N5089_vt)) - 1.0) / Q2N5089_BetaF) + ((math_exp_approx((vBCQ1 / Q2N5089_vt)) - 1.0) / Q2N5089_BetaR))) * (gR21 + (1.0 / 1000000000.0)))) / R17)))) / (((((gRout + (1.0 / 1000000000.0)) * (gR21 + (1.0 / 1000000000.0))) / R17) / R17) + ((((-1.0 / R17) - gR18) - (1.0 / 1000000000.0)) * ((gRout + (1.0 / 1000000000.0)) * (((gR20 + (1.0 / R17)) + (1.0 / 1000000000.0)) * (gR21 + (1.0 / 1000000000.0))))))));
    const auto zR19C5 = (-(gR19C5 * ((((((((-1.0 / R17) - gR18) - (1.0 / 1000000000.0)) * ((gRout + (1.0 / 1000000000.0)) * ((Q2N5089_Is * (((math_exp_approx((vBEQ1 / Q2N5089_vt)) - 1.0) / Q2N5089_BetaF) + ((math_exp_approx((vBCQ1 / Q2N5089_vt)) - 1.0) / Q2N5089_BetaR))) * (gR21 + (1.0 / 1000000000.0))))) - ((((Q2N5089_Is * ((math_exp_approx((vBEQ1 / Q2N5089_vt)) - math_exp_approx((vBCQ1 / Q2N5089_vt))) - ((math_exp_approx((vBCQ1 / Q2N5089_vt)) - 1.0) / Q2N5089_BetaR))) - (D1N914_Is * (math_exp_approx((vD3D4 / D1N914_vt)) - (1.0 / math_exp_approx((vD3D4 / D1N914_vt)))))) * ((gRout + (1.0 / 1000000000.0)) * (gR21 + (1.0 / 1000000000.0)))) / R17)) + ((gR18 * ((gRout + (1.0 / 1000000000.0)) * ((gR21 + (1.0 / 1000000000.0)) * Vp))) / R17)) / (((((gRout + (1.0 / 1000000000.0)) * (gR21 + (1.0 / 1000000000.0))) / R17) / R17) + ((((-1.0 / R17) - gR18) - (1.0 / 1000000000.0)) * ((gRout + (1.0 / 1000000000.0)) * (((gR20 + (1.0 / R17)) + (1.0 / 1000000000.0)) * (gR21 + (1.0 / 1000000000.0))))))) + vi)));
    const auto zC6 = (gC6 * (((((gR18 * ((gRout + (1.0 / 1000000000.0)) * (((gR20 + (1.0 / R17)) + (1.0 / 1000000000.0)) * ((gR21 + (1.0 / 1000000000.0)) * Vp)))) - (((gRout + (1.0 / 1000000000.0)) * ((Q2N5089_Is * (((math_exp_approx((vBEQ1 / Q2N5089_vt)) - 1.0) / Q2N5089_BetaF) + ((math_exp_approx((vBCQ1 / Q2N5089_vt)) - 1.0) / Q2N5089_BetaR))) * (gR21 + (1.0 / 1000000000.0)))) / R17)) - (((Q2N5089_Is * ((math_exp_approx((vBEQ1 / Q2N5089_vt)) - math_exp_approx((vBCQ1 / Q2N5089_vt))) - ((math_exp_approx((vBCQ1 / Q2N5089_vt)) - 1.0) / Q2N5089_BetaR))) - (D1N914_Is * (math_exp_approx((vD3D4 / D1N914_vt)) - (1.0 / math_exp_approx((vD3D4 / D1N914_vt)))))) * ((gRout + (1.0 / 1000000000.0)) * (((gR20 + (1.0 / R17)) + (1.0 / 1000000000.0)) * (gR21 + (1.0 / 1000000000.0)))))) / (((((gRout + (1.0 / 1000000000.0)) * (gR21 + (1.0 / 1000000000.0))) / R17) / R17) + ((((-1.0 / R17) - gR18) - (1.0 / 1000000000.0)) * ((gRout + (1.0 / 1000000000.0)) * (((gR20 + (1.0 / R17)) + (1.0 / 1000000000.0)) * (gR21 + (1.0 / 1000000000.0))))))) - (((((((-1.0 / R17) - gR18) - (1.0 / 1000000000.0)) * ((gRout + (1.0 / 1000000000.0)) * ((Q2N5089_Is * (((math_exp_approx((vBEQ1 / Q2N5089_vt)) - 1.0) / Q2N5089_BetaF) + ((math_exp_approx((vBCQ1 / Q2N5089_vt)) - 1.0) / Q2N5089_BetaR))) * (gR21 + (1.0 / 1000000000.0))))) - ((((Q2N5089_Is * ((math_exp_approx((vBEQ1 / Q2N5089_vt)) - math_exp_approx((vBCQ1 / Q2N5089_vt))) - ((math_exp_approx((vBCQ1 / Q2N5089_vt)) - 1.0) / Q2N5089_BetaR))) - (D1N914_Is * (math_exp_approx((vD3D4 / D1N914_vt)) - (1.0 / math_exp_approx((vD3D4 / D1N914_vt)))))) * ((gRout + (1.0 / 1000000000.0)) * (gR21 + (1.0 / 1000000000.0)))) / R17)) + ((gR18 * ((gRout + (1.0 / 1000000000.0)) * ((gR21 + (1.0 / 1000000000.0)) * Vp))) / R17)) / (((((gRout + (1.0 / 1000000000.0)) * (gR21 + (1.0 / 1000000000.0))) / R17) / R17) + ((((-1.0 / R17) - gR18) - (1.0 / 1000000000.0)) * ((gRout + (1.0 / 1000000000.0)) * (((gR20 + (1.0 / R17)) + (1.0 / 1000000000.0)) * (gR21 + (1.0 / 1000000000.0)))))))));
    const auto zR17C12 = ((gzR17C12 * ((((gR18 * ((gRout + (1.0 / 1000000000.0)) * (((gR20 + (1.0 / R17)) + (1.0 / 1000000000.0)) * ((gR21 + (1.0 / 1000000000.0)) * Vp)))) - ((((Q2N5089_Is * ((math_exp_approx((vBEQ1 / Q2N5089_vt)) - math_exp_approx((vBCQ1 / Q2N5089_vt))) - ((math_exp_approx((vBCQ1 / Q2N5089_vt)) - 1.0) / Q2N5089_BetaR))) - (D1N914_Is * (math_exp_approx((vD3D4 / D1N914_vt)) - (1.0 / math_exp_approx((vD3D4 / D1N914_vt)))))) * ((gRout + (1.0 / 1000000000.0)) * (((gR20 + (1.0 / R17)) + (1.0 / 1000000000.0)) * (gR21 + (1.0 / 1000000000.0))))) + (((gRout + (1.0 / 1000000000.0)) * ((Q2N5089_Is * (((math_exp_approx((vBEQ1 / Q2N5089_vt)) - 1.0) / Q2N5089_BetaF) + ((math_exp_approx((vBCQ1 / Q2N5089_vt)) - 1.0) / Q2N5089_BetaR))) * (gR21 + (1.0 / 1000000000.0)))) / R17))) / (((((gRout + (1.0 / 1000000000.0)) * (gR21 + (1.0 / 1000000000.0))) / R17) / R17) + ((((-1.0 / R17) - gR18) - (1.0 / 1000000000.0)) * ((gRout + (1.0 / 1000000000.0)) * (((gR20 + (1.0 / R17)) + (1.0 / 1000000000.0)) * (gR21 + (1.0 / 1000000000.0))))))) - (((((((-1.0 / R17) - gR18) - (1.0 / 1000000000.0)) * ((gRout + (1.0 / 1000000000.0)) * ((Q2N5089_Is * (((math_exp_approx((vBEQ1 / Q2N5089_vt)) - 1.0) / Q2N5089_BetaF) + ((math_exp_approx((vBCQ1 / Q2N5089_vt)) - 1.0) / Q2N5089_BetaR))) * (gR21 + (1.0 / 1000000000.0))))) - ((((Q2N5089_Is * ((math_exp_approx((vBEQ1 / Q2N5089_vt)) - math_exp_approx((vBCQ1 / Q2N5089_vt))) - ((math_exp_approx((vBCQ1 / Q2N5089_vt)) - 1.0) / Q2N5089_BetaR))) - (D1N914_Is * (math_exp_approx((vD3D4 / D1N914_vt)) - (1.0 / math_exp_approx((vD3D4 / D1N914_vt)))))) * ((gRout + (1.0 / 1000000000.0)) * (gR21 + (1.0 / 1000000000.0)))) / R17)) + ((gR18 * ((gRout + (1.0 / 1000000000.0)) * ((gR21 + (1.0 / 1000000000.0)) * Vp))) / R17)) / (((((gRout + (1.0 / 1000000000.0)) * (gR21 + (1.0 / 1000000000.0))) / R17) / R17) + ((((-1.0 / R17) - gR18) - (1.0 / 1000000000.0)) * ((gRout + (1.0 / 1000000000.0)) * (((gR20 + (1.0 / R17)) + (1.0 / 1000000000.0)) * (gR21 + (1.0 / 1000000000.0))))))))) / 2.0);

    const auto vo_dc_out = 0.0;

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
    return vo_dc_out;
}

