#include "neo.h"

#define NEO_int_add_overflow(a, b) ((a > 0 && b > 0 && a > INT64_MAX - b) || (a < 0 && b < 0 && a < INT64_MIN - b))
#define NEO_int_subtract_overflow(a, b) ((b < 0 && a > INT64_MAX + b) || (b > 0 && a < INT64_MIN + b))
#define NEO_int_multiply_overflow(a, b) ((a > 0 && b > 0 && a > INT64_MAX / b) || (a < 0 && b < 0 && a < INT64_MIN / b))

NeoObject *NEO_int(int number) {
    NeoObject *obj = NEO_object_unset();
    obj->prototype = NeoInt;
    NeoIntValue *v = malloc(sizeof(NeoIntValue));
    v->value = number;
    obj->v = v;
    return obj;
}

NeoObject *NEO_int_negate(NeoObject *a) {
    return NEO_int(-NEO_vInt(a));
}

NeoObject *NEO_int_add(NeoObject *a, NeoObject *b) {
    int a_value = NEO_vInt(a);
    if (b->prototype == NeoInt) {
        int b_value = NEO_vInt(b);
        if (b_value == 0) return NEO_int(a_value);
        if (NEO_int_add_overflow(a_value, b_value)) {
            mpz_t result;
            mpz_init_set_si(result, a_value);
            if (b_value > 0) {
                mpz_add_ui(result, result, b_value);
            } else {
                mpz_sub_ui(result, result, -b_value);
            }
            return NEO_bigint(result);
        }
        return NEO_int(a_value + NEO_vInt(b));
    }
    if (b->prototype == NeoDouble) {
        return NEO_double(a_value + NEO_vDouble(b));
    }
    if (b->prototype == NeoBigInt) {
        mpz_t result;
        mpz_init_set_si(result, a_value);
        mpz_add(result, result, NEO_vBigInt(b));
        return NEO_bigint(result);
    }
    if (b->prototype == NeoBigFloat) {
        NEO_int2bigfloat(a_value, result);
        mpfr_add(NEO_vBigFloat(result), NEO_vBigFloat(result), NEO_vBigFloat(b),
                 MPFR_RNDN);
        return result;
    }
    NEO_throw_error("RuntimeError: int + object is not supported.");
}

NeoObject *NEO_int_subtract(NeoObject *a, NeoObject *b) {
    int a_value = NEO_vInt(a);
    if (b->prototype == NeoInt) {
        int b_value = NEO_vInt(b);
        if (a_value == 0) return NEO_int(-NEO_vInt(b));
        if (b_value == 0) return NEO_int(a_value);
        if (NEO_int_subtract_overflow(a_value, NEO_vInt(b))) {
            mpz_t result;
            mpz_init_set_si(result, a_value);
            if (b_value > 0) {
                mpz_sub_ui(result, result, b_value);
            } else {
                mpz_add_ui(result, result, -b_value);
            }
            return NEO_bigint(result);
        }
        return NEO_int(a_value - b_value);
    }
    if (b->prototype == NeoDouble) {
        return NEO_double(a_value - NEO_vDouble(b));
    }
    if (b->prototype == NeoBigInt) {
        mpz_t result;
        mpz_init_set_si(result, a_value);
        mpz_sub(result, result, NEO_vBigInt(b));
        return NEO_bigint(result);
    }
    if (b->prototype == NeoBigFloat) {
        NEO_int2bigfloat(NEO_vInt(a), result);
        mpfr_sub(NEO_vBigFloat(result), NEO_vBigFloat(result), NEO_vBigFloat(b),
                 MPFR_RNDN);
        return result;
    }
    NEO_throw_error("RuntimeError: int - object is not supported.");
}

NeoObject *NEO_int_multiply(NeoObject *a, NeoObject *b) {
    int a_value = NEO_vInt(a);
    if (b->prototype == NeoInt) {
        int b_value = NEO_vInt(b);
        if (b_value == 0) return NEO_int(0);
        if (NEO_int_multiply_overflow(a_value, b_value)) {
            mpz_t result;
            mpz_init_set_si(result, a_value);
            mpz_mul_si(result, result, b_value);
            return NEO_bigint(result);
        }
        return NEO_int(a_value * b_value);
    }
    if (b->prototype == NeoDouble) {
        return NEO_double(a_value * NEO_vDouble(b));
    }
    if (b->prototype == NeoBigInt) {
        mpz_t result;
        mpz_init_set_si(result, a_value);
        mpz_mul(result, result, NEO_vBigInt(b));
        return NEO_bigint(result);
    }
    if (b->prototype == NeoBigFloat) {
        NEO_int2bigfloat(NEO_vInt(a), result);
        mpfr_mul(NEO_vBigFloat(result), NEO_vBigFloat(result), NEO_vBigFloat(b),
                 MPFR_RNDN);
        return result;
    }
    if (b->prototype == NeoArray) {
        NeoObject *result = NEO_array();
        for (size_t i = 0; i < a_value; i++) {
            internal_NEO_array_push(result, b);
        }
        return result;
    }
    NEO_throw_error("RuntimeError: int * object is not supported.");
}

NeoObject *NEO_int_divide(NeoObject *a, NeoObject *b) {
    int a_value = NEO_vInt(a);
    if (b->prototype == NeoInt) {
        return NEO_double((double) a_value / (double) NEO_vInt(b));
    }
    if (b->prototype == NeoDouble) {
        return NEO_double((double) a_value / NEO_vDouble(b));
    }
    if (b->prototype == NeoBigInt) {
        NEO_int2bigfloat(a_value, result);
        NEO_mpz2mpfr(NEO_vBigInt(b), b_mpfr);
        mpfr_div(NEO_vBigFloat(result), NEO_vBigFloat(result), b_mpfr,
                 MPFR_RNDN);
        return result;
    }
    if (b->prototype == NeoBigFloat) {
        NEO_int2bigfloat(NEO_vInt(a), result);
        mpfr_div(NEO_vBigFloat(result), NEO_vBigFloat(result), NEO_vBigFloat(b),
                 MPFR_RNDN);
        return result;
    }
    NEO_throw_error("RuntimeError: int / object is not supported.");
}

NeoObject *NEO_int_modulo(NeoObject *a, NeoObject *b) {
    int a_value = NEO_vInt(a);
    if (b->prototype == NeoInt) {
        return NEO_int(a_value % NEO_vInt(b));
    }
    if (b->prototype == NeoDouble) {
        return NEO_double(fmod(a_value, NEO_vDouble(b)));
    }
    if (b->prototype == NeoBigInt) {
        NEO_int2bigint(a_value, result);
        mpz_mod(NEO_vBigInt(result), NEO_vBigInt(result), NEO_vBigInt(b));
        return result;
    }
    if (b->prototype == NeoBigFloat) {
        NEO_int2bigfloat(NEO_vInt(a), result);
        mpfr_fmod(NEO_vBigFloat(result), NEO_vBigFloat(result),
                  NEO_vBigFloat(b), MPFR_RNDN);
        return result;
    }
    NEO_throw_error("RuntimeError: int % object is not supported.");
}

NeoObject *NEO_int_power(NeoObject *a, NeoObject *b) {
    int a_value = NEO_vInt(a);
    if (b->prototype == NeoInt) {
        return NEO_int(pow(a_value, NEO_vInt(b)));
    }
    if (b->prototype == NeoDouble) {
        return NEO_double(pow(a_value, NEO_vDouble(b)));
    }
    if (b->prototype == NeoBigInt) {
        NEO_int2bigint(a_value, result);
        mpz_pow_ui(NEO_vBigInt(result), NEO_vBigInt(result),
                   mpz_get_ui(NEO_vBigInt(b)));
        return result;
    }
    if (b->prototype == NeoBigFloat) {
        NEO_int2bigfloat(NEO_vInt(a), result);
        mpfr_pow(NEO_vBigFloat(result), NEO_vBigFloat(result), NEO_vBigFloat(b),
                 MPFR_RNDN);
        return result;
    }
    NEO_throw_error("RuntimeError: int ** object is not supported.");

}

NeoObject *NEO_int_bit_and(NeoObject *a, NeoObject *b) {
    int a_value = NEO_vInt(a);
    if (b->prototype == NeoInt) {
        return NEO_int(a_value & NEO_vInt(b));
    }
    if (b->prototype == NeoDouble) {
        return NEO_int(a_value & (int) NEO_vDouble(b));
    }
    NEO_throw_error("RuntimeError: int & object is not supported.");

}

NeoObject *NEO_int_bit_or(NeoObject *a, NeoObject *b) {
    int a_value = NEO_vInt(a);
    if (b->prototype == NeoInt) {
        return NEO_int(a_value | NEO_vInt(b));
    }
    if (b->prototype == NeoDouble) {
        return NEO_int(a_value | (int) NEO_vDouble(b));
    }
    NEO_throw_error("RuntimeError: int | object is not supported.");
}

NeoObject *NEO_int_xor(NeoObject *a, NeoObject *b) {
    int a_value = NEO_vInt(a);
    if (b->prototype == NeoInt) {
        return NEO_int(a_value ^ NEO_vInt(b));
    }
    if (b->prototype == NeoDouble) {
        return NEO_int(a_value ^ (int) NEO_vDouble(b));
    }
    NEO_throw_error("RuntimeError: int ^ object is not supported.");
}

NeoObject *NEO_int_shift_right(NeoObject *a, NeoObject *b) {
    int a_value = NEO_vInt(a);
    if (b->prototype == NeoInt) {
        return NEO_int(a_value >> NEO_vInt(b));
    }
    if (b->prototype == NeoDouble) {
        return NEO_int(a_value >> (int) NEO_vDouble(b));
    }
    NEO_throw_error("RuntimeError: int >> object is not supported.");
}

NeoObject *NEO_int_shift_left(NeoObject *a, NeoObject *b) {
    int a_value = NEO_vInt(a);
    if (b->prototype == NeoInt) {
        return NEO_int(a_value << NEO_vInt(b));
    }
    if (b->prototype == NeoDouble) {
        return NEO_int(a_value << (int) NEO_vDouble(b));
    }
    NEO_throw_error("RuntimeError: int << object is not supported.");
}

NeoObject *NEO_int_bit_not(NeoObject *a) {
    return NEO_int(~NEO_vInt(a));
}

NeoObject *NEO_int_greater_than(NeoObject *a, NeoObject *b) {
    int a_value = NEO_vInt(a);
    if (b->prototype == NeoInt) {
        return NEO_boolean(a_value > NEO_vInt(b));
    }
    if (b->prototype == NeoDouble) {
        return NEO_boolean(a_value > NEO_vDouble(b));
    }
    if (b->prototype == NeoBigInt) {
        return NEO_boolean(mpz_cmp_si(NEO_vBigInt(b), a_value) > 0);
    }
    if (b->prototype == NeoBigFloat) {
        return NEO_boolean(mpfr_cmp_si(NEO_vBigFloat(b), a_value) > 0);
    }
    NEO_throw_error("RuntimeError: int > object is not supported.");
}

NeoObject *NEO_int_less_than(NeoObject *a, NeoObject *b) {
    int a_value = NEO_vInt(a);
    if (b->prototype == NeoInt) {
        return NEO_boolean(a_value < NEO_vInt(b));
    }
    if (b->prototype == NeoDouble) {
        return NEO_boolean(a_value < NEO_vDouble(b));
    }
    if (b->prototype == NeoBigInt) {
        return NEO_boolean(mpz_cmp_si(NEO_vBigInt(b), a_value) < 0);
    }
    if (b->prototype == NeoBigFloat) {
        return NEO_boolean(mpfr_cmp_si(NEO_vBigFloat(b), a_value) < 0);
    }
    NEO_throw_error("RuntimeError: int < object is not supported.");
}

NeoObject *NEO_int_equals(NeoObject *a, NeoObject *b) {
    int a_value = NEO_vInt(a);
    if (b->prototype == NeoInt) {
        return NEO_boolean(a_value == NEO_vInt(b));
    }
    if (b->prototype == NeoDouble) {
        return NEO_boolean(a_value == NEO_vDouble(b));
    }
    if (b->prototype == NeoBigInt) {
        return NEO_boolean(mpz_cmp_d(NEO_vBigInt(b), a_value) == 0);
    }
    if (b->prototype == NeoBigFloat) {
        return NEO_boolean(mpfr_cmp_d(NEO_vBigFloat(b), a_value) == 0);
    }
    NEO_throw_error("RuntimeError: int == object is not supported.");
}

NeoObject *NEO_int_not(NeoObject *a) {
    return NEO_boolean(!NEO_vInt(a));
}