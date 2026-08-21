// Generates inline exp/log/pow approximations (math_exp_approx et al.) as
// plain C/C++ functions instead of linking std::exp/std::log/std::pow.
// Ported from github.com/Chowdhury-DSP/math_approx.
//
// Depends on backend.jai (Backend, Transcendental_Approx, Flop_Count) and
// sym_algebra (Expr/Expr_ID, print_expr) for emit_const_decl.

#import "Basic";

#import "backend";
#import "sym_algebra";

// C++: "std::<name>"
// C: "<name>" or "<name>f" for float
math_fn :: (backend: Backend, name: string) -> string {
    if backend.is_cpp  return tprint("std::%", name);
    if backend.type_name == "float"  return tprint("%f", name);
    return name;
}

// Returns the name to call for a math function, honoring the backend's
// transcendental approximation settings for exp/log/pow. Other functions
// (fabs, etc.) always resolve to their exact std:: name, there's no
// approximation for them here.
transcendental_fn_name :: (backend: Backend, name: string) -> string {
    if !backend.transcendental_approx.exact {
        if name == "exp"  return "math_exp_approx";
        if name == "log"  return "math_log_approx";
        if name == "pow"  return "math_pow_approx";
    }
    return math_fn(backend, name);
}

// sym_algebra emits complex math functions like `math_fn_exp`,
// so here we replace them with the proper C/C++ equivalent (or the
// generated approximation, see transcendental_fn_name()).
resolve_math_fns :: (code: string, backend: Backend) -> string {
    found := String.contains(code, "math_fn_");
    if !found  return code;

    result := code;
    result = String.replace(result, "math_fn_exp", transcendental_fn_name(backend, "exp"));
    result = String.replace(result, "math_fn_log", transcendental_fn_name(backend, "log"));
    result = String.replace(result, "math_fn_pow", transcendental_fn_name(backend, "pow"));
    return result;
}

// Emits `<prefix>const <decl_type> <name> = <expr>;\n`.
emit_const_decl :: (builder: *String_Builder, backend: Backend, name: string, e: Expr_ID, exprs: [] Expr, suffix: string, prefix := "") {
    print_to_builder(builder, "%1const %2 %3 = %4;\n", prefix, backend.decl_type, name, resolve_math_fns(print_expr(e, exprs, suffix), backend));
}

// The approximation helpers bit-cast a float to its integer representation
// and back (see math_bits_from_float/math_float_from_bits), so cstdint is
// needed alongside cmath/math.h whenever they might be emitted.
math_include :: (backend: Backend) -> string {
    needs_stdint := !backend.transcendental_approx.exact;
    if backend.is_cpp {
        return ifx needs_stdint then "#include <cmath>\n#include <cstdint>\n\n" else "#include <cmath>\n\n";
    }
    return ifx needs_stdint then "#include <math.h>\n#include <stdint.h>\n\n" else "#include <math.h>\n\n";
}

// Coefficients for the order-N polynomial approximating 2^x on [0, 1],
// coeffs[i] is the coefficient of x^i. Used by math_exp_approx (exp(x) =
// 2^floor(x*log2(e)) * 2^frac(x*log2(e))).
pow2_approx_coeffs :: (order: int, c1_continuous: bool) -> [] float64 {
    if c1_continuous {
        if order == {
            case 3; return .[1, 0.693147180560, 0.227411277760, 0.0794415416798];
            case 4; return .[1, 0.693147180560, 0.241068174795, 0.0521277476109, 0.0136568970345];
            case 5; return .[1, 0.693147180560, 0.240184132673, 0.0557830666741, 0.00899838527231, 0.00188723482038];
            case 6; return .[1, 0.693147180560, 0.240228250686, 0.0554875633068, 0.00967475272129, 0.00124453797252, 0.000217714753229];
            case 7; return .[1, 0.693147180559945, 0.240226202240181, 0.0555072492957270, 0.00960612128901630, 0.00135551454943593, 0.000133154170702612, 0.0000245778949916153];
        }
    } else {
        if order == {
            case 1; return .[1, 1];
            case 2; return .[1, 0.660233956811, 0.339766043189];
            case 3; return .[1, 0.695424387485, 0.226307586882, 0.0782680256330];
            case 4; return .[1, 0.693032120001, 0.241379754777, 0.0520324008177, 0.0135557244044];
            case 5; return .[1, 0.693152270576, 0.240156326598, 0.0558229130202, 0.00899009909264, 0.00187839071291];
            case 6; return .[1, 0.693147003658, 0.240229787107, 0.0554833098983, 0.00967911763840, 0.00124359387839, 0.000217187820427];
            case 7; return .[1, 0.693147201030637, 0.240226092549669, 0.0555070350342468, 0.00960825566419915, 0.00135107295099880, 0.000136898688977877, 0.0000234440812713967];
        }
    }
    assert(false, "Unsupported exp approximation order % (c1_continuous=%). Supported: 1-7 (3-7 if c1_continuous).", order, c1_continuous);
    return .[];
}

// Coefficients for the order-N polynomial approximating log2(x) on [1, 2],
// same index convention as pow2_approx_coeffs above.
log2_approx_coeffs :: (order: int, c1_continuous: bool) -> [] float64 {
    if c1_continuous {
        if order == {
            case 3; return .[-2.21347520444, 3.14829792933, -1.09886528622, 0.164042561333];
            case 4; return .[-2.55185920824, 4.16344994072, -2.19861329856, 0.671618567027, -0.0845960009489];
            case 5; return .[-2.82807214111, 5.17788146374, -3.65368350361, 1.68976432066, -0.432338320780, 0.0464481811023];
            case 6; return .[-3.06081857306, 6.19242937536, -5.46521465640, 3.38542517475, -1.31007090775, 0.284794437502, -0.0265448504094];
        }
    } else {
        if order == {
            case 1; return .[-1, 1];
            case 2; return .[-1.69311082948, 2.03966624422, -0.346555414741];
            case 3; return .[-2.16417056258, 3.06469586582, -1.05974531422, 0.159220010975];
            case 4; return .[-2.51982743265, 4.08637809379, -2.13412984371, 0.649709537672, -0.0821303550902];
            case 5; return .[-2.80534277658, 5.10697088382, -3.56885211615, 1.64139451414, -0.419319345483, 0.0451488402558];
            case 6; return .[-3.04376925958, 6.12708086513, -5.36371998242, 3.30388341157, -1.27446900713, 0.276834061071, -0.0258400886535];
        }
    }
    assert(false, "Unsupported log approximation order % (c1_continuous=%). Supported: 1-6 (3-6 if c1_continuous).", order, c1_continuous);
    return .[];
}

// Builds a C/C++ expression string evaluating the polynomial given by
// `coeffs` (coeffs[i] = coefficient of x^i), using Estrin's scheme: pair
// coefficients into "digits" (c[2k] + c[2k+1]*x), then combine those digits
// with a balanced binary tree in x*x (each level squares the running power
// of x*x) instead of a sequential fold. This gives the compiler independent
// sub-expressions to evaluate in parallel, same as math_approx's own
// hand-unrolled templates do for their higher orders, just generated
// generically for any order instead of transcribed by hand per order. The
// result is the same polynomial value up to floating-point associativity.
//
// Returns both the C/C++ expression text and its exact op count in one
// pass, so the two can never drift apart: count_flops_expr() (codegen.jai)
// needs the same number this actually costs at runtime, not a
// hand-maintained estimate.
emit_poly_estrin :: (coeffs: [] float64, x_var: string, xsq_var: string, suffix: string) -> (text: string, flops: Flop_Count) {
    assert(coeffs.count > 0, "emit_poly_estrin: coeffs must be non-empty");

    // formatFloat's trailing_width reserves a fixed column count and pads
    // with spaces once zero_removal strips trailing zeros short of it, so
    // the result needs trimming before it's usable as a literal.
    fmt :: (v: float64) -> string {
        // ONE_ZERO_AFTER_DECIMAL (not the default YES) keeps "1.0" rather
        // than stripping it down to "1", which isn't a valid C/C++ float
        // literal once the type suffix gets appended ("1f" doesn't parse).
        return String.trim_right(tprint("%", formatFloat(v, trailing_width = 17, zero_removal = .ONE_ZERO_AFTER_DECIMAL)));
    }

    digits: [..] string;
    flops: Flop_Count;
    i := 0;
    while i < coeffs.count {
        if i + 1 < coeffs.count {
            array_add(*digits, tprint("(%1%2 + %3%2 * %4)", fmt(coeffs[i]), suffix, fmt(coeffs[i + 1]), x_var));
            flops.muls += 1;
            flops.adds += 1;
            i += 2;
        } else {
            array_add(*digits, tprint("%1%2", fmt(coeffs[i]), suffix));
            i += 1;
        }
    }

    // digits[k] is the coefficient of (x*x)^k; combine with a balanced split
    // (not a sequential fold) so independent halves can evaluate in parallel:
    // value = low_half(digits[lo..mid-1]) + (x*x)^half_len * high_half(digits[mid..hi]).
    estrin :: (digits: [] string, lo: int, hi: int, xsq_var: string, flops: *Flop_Count) -> string {
        if lo == hi  return digits[lo];

        half_len := (hi - lo + 1) / 2;
        mid := lo + half_len;
        low  := estrin(digits, lo, mid - 1, xsq_var, flops);
        high := estrin(digits, mid, hi, xsq_var, flops);

        xsq_pow := xsq_var;
        for 1..half_len - 1 { xsq_pow = tprint("%1 * %2", xsq_pow, xsq_var); flops.muls += 1; }
        if half_len > 1  xsq_pow = tprint("(%1)", xsq_pow);

        flops.muls += 1; // xsq_pow * high
        flops.adds += 1; // low + ...
        return tprint("(%1 + %2 * (%3))", low, xsq_pow, high);
    }

    text := estrin(digits, 0, digits.count - 1, xsq_var, *flops);
    return text, flops;
}

// Fixed op cost of math_exp_approx/math_log_approx's own wrapper code, on
// top of whatever emit_poly_estrin() says the polynomial itself costs. See
// generate_transcendental_approx_helpers() for the actual code these
// numbers are counting.
EXP_APPROX_WRAPPER_FLOPS :: Flop_Count.{ muls = 3, adds = 1 }; // x *= log2e; f_sq = f*f; scale * (poly)  |  f = x - l
LOG_APPROX_WRAPPER_FLOPS :: Flop_Count.{ muls = 2, adds = 1 }; // vf_sq = vf*vf; ln2 * (...)              |  e + poly
POW_APPROX_EXTRA_FLOPS   :: Flop_Count.{ muls = 1 };           // y * log(x), on top of one exp + one log call

add_flop_count :: (dst: *Flop_Count, src: Flop_Count) {
    dst.muls += src.muls;
    dst.divs += src.divs;
    dst.adds += src.adds;
    dst.transcendentals += src.transcendentals;
}

// Caching exp/log flop flop counts per approximation order
Flop_Cache_Entry :: struct { key: int; flops: Flop_Count; }
exp_flop_cache: [..] Flop_Cache_Entry;
log_flop_cache: [..] Flop_Cache_Entry;

flop_cache_key :: (approx: Transcendental_Approx) -> int {
    return approx.order * 2 + cast(int) approx.c1_continuous;
}

exp_approx_flop_count :: (approx: Transcendental_Approx) -> Flop_Count {
    key := flop_cache_key(approx);
    for e : exp_flop_cache  if e.key == key  return e.flops;
    _, poly_flops := emit_poly_estrin(pow2_approx_coeffs(approx.order, approx.c1_continuous), "f", "f_sq", "");
    result := EXP_APPROX_WRAPPER_FLOPS;
    add_flop_count(*result, poly_flops);
    array_add(*exp_flop_cache, .{ key = key, flops = result });
    return result;
}

log_approx_flop_count :: (approx: Transcendental_Approx) -> Flop_Count {
    key := flop_cache_key(approx);
    for e : log_flop_cache  if e.key == key  return e.flops;
    _, poly_flops := emit_poly_estrin(log2_approx_coeffs(approx.order, approx.c1_continuous), "vf", "vf_sq", "");
    result := LOG_APPROX_WRAPPER_FLOPS;
    add_flop_count(*result, poly_flops);
    array_add(*log_flop_cache, .{ key = key, flops = result });
    return result;
}

pow_approx_flop_count :: (approx: Transcendental_Approx) -> Flop_Count {
    result := POW_APPROX_EXTRA_FLOPS;
    add_flop_count(*result, exp_approx_flop_count(approx));
    add_flop_count(*result, log_approx_flop_count(approx));
    return result;
}

// Emits math_exp_approx / math_log_approx / math_pow_approx as plain
// functions. Ported from math_approx::exp/log (pow_approx.hpp, log_approx.hpp).
generate_transcendental_approx_helpers :: (backend: Backend) -> string {
    approx := backend.transcendental_approx;
    is_double := backend.type_name == "double";
    type_name := backend.type_name;
    suffix    := ifx is_double then "" else "f";
    int_type  := ifx is_double then "int64_t" else "int32_t";
    bias      := ifx is_double then "1023" else "127";
    shift     := ifx is_double then "52" else "23";
    clamp     := ifx is_double then "-1022.0" else "-126.0f";
    log2e     := ifx is_double then "1.4426950408889634074" else "1.4426950408889634f";
    ln2       := ifx is_double then "0.69314718055994530942" else "0.6931471805599453f";
    exp_mask  := ifx is_double then "0x7ff0000000000000LL" else "0x7f800000";
    one_mask  := ifx is_double then "0x3ff0000000000000LL" else "0x3f800000";

    exp_poly, _ := emit_poly_estrin(pow2_approx_coeffs(approx.order, approx.c1_continuous), "f", "f_sq", suffix);
    log_poly, _ := emit_poly_estrin(log2_approx_coeffs(approx.order, approx.c1_continuous), "vf", "vf_sq", suffix);

    template := #string DONE
static %4 math_bits_from_float(%1 x) { union { %1 f; %4 i; } u; u.f = x; return u.i; }
static %1 math_float_from_bits(%4 i) { union { %4 i; %1 f; } u; u.i = i; return u.f; }

static %1 math_exp_approx(%1 x) {
    x *= %2;
    if (x < %3) x = %3;
    const %4 xi = (%4) x;
    const %4 l = x < (%1) 0 ? xi - 1 : xi;
    const %1 f = x - (%1) l;
    const %1 f_sq = f * f;
    const %4 vi = (l + %5) << %6;
    return math_float_from_bits(vi) * (%7);
}

static %1 math_log_approx(%1 x) {
    const %4 vi = math_bits_from_float(x);
    const %4 ex = vi & %8;
    const %4 e = (ex >> %6) - %5;
    const %4 vfi = (vi - ex) | %9;
    const %1 vf = math_float_from_bits(vfi);
    const %1 vf_sq = vf * vf;
    return %10 * ((%1) e + (%11));
}

static %1 math_pow_approx(%1 x, %1 y) {
    return math_exp_approx(y * math_log_approx(x));
}

DONE;
    return tprint(template, type_name, log2e, clamp, int_type, bias, shift, exp_poly, exp_mask, one_mask, ln2, log_poly);
}
