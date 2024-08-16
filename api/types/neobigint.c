#include "neo.h"

NeoObject *NEO_bigint_unset() {
    NeoObject *obj = NEO_object_unset();
    obj->prototype = NeoBigInt;
    obj->v = malloc(sizeof(NeoBigIntValue));
    return obj;
}

NeoObject *NEO_bigint(mpz_t value) {
    NeoObject *obj = NEO_bigint_unset();
    mpz_init_set(NEO_vBigInt(obj), value);
    return obj;
}

NeoObject *NEO_bigint_str(char *string) {
    NeoObject *obj = NEO_bigint_unset();
    NeoBigIntValue *v = malloc(sizeof(NeoBigIntValue));
    mpz_init_set_str(v->value, string, 10);
    obj->v = v;
    return obj;
}

NeoObject *NEO_bigint_negate(NeoObject *a) {
    mpz_t a_value;
    mpz_init_set(a_value, NEO_vBigInt(a));
    mpz_neg(a_value, a_value);
    return NEO_bigint(a_value);
}

NeoObject *NEO_bigint_add(NeoObject *a, NeoObject *b) {
    mpz_t a_value;
    mpz_init_set(a_value, NEO_vBigInt(a));
    if (b->prototype == NeoBigInt) {
        mpz_add(a_value, a_value, NEO_vBigInt(b));
        return NEO_bigint(a_value);
    }
    if (b->prototype == NeoInt) {
        int b_value = NEO_vInt(b);
        if (b_value < 0) {
            mpz_add_ui(a_value, a_value, -b_value);
        } else {
            mpz_add_ui(a_value, a_value, b_value);
        }
        return NEO_bigint(a_value);
    }
    if (b->prototype == NeoDouble) {
        NEO_mpz2mpfr(a_value, a_mpfr);
        mpfr_add_d(a_mpfr, a_mpfr, NEO_vDouble(b), MPFR_RNDN);
        return NEO_bigfloat(a_mpfr);
    }
    if (b->prototype == NeoBigFloat) {
        NEO_mpz2mpfr(a_value, a_mpfr);
        mpfr_add(a_mpfr, a_mpfr, NEO_vBigFloat(b), MPFR_RNDN);
        return NEO_bigfloat(a_mpfr);
    }
    NEO_throw_error("RuntimeError: bigint + object is not supported.");
}

NeoObject *NEO_bigint_subtract(NeoObject *a, NeoObject *b) {
    mpz_t a_value;
    mpz_init_set(a_value, NEO_vBigInt(a));
    if (b->prototype == NeoBigInt) {
        mpz_sub(a_value, a_value, NEO_vBigInt(b));
        return NEO_bigint(a_value);
    }
    if (b->prototype == NeoInt) {
        int b_value = NEO_vInt(b);
        if (b_value < 0) {
            mpz_sub_ui(a_value, a_value, -b_value);
        } else {
            mpz_sub_ui(a_value, a_value, b_value);
        }
        return NEO_bigint(a_value);
    }
    if (b->prototype == NeoDouble) {
        NEO_mpz2mpfr(a_value, a_mpfr);
        mpfr_sub_d(a_mpfr, a_mpfr, NEO_vDouble(b), MPFR_RNDN);
        return NEO_bigfloat(a_mpfr);
    }
    if (b->prototype == NeoBigFloat) {
        NEO_mpz2mpfr(a_value, a_mpfr);
        mpfr_sub(a_mpfr, a_mpfr, NEO_vBigFloat(b), MPFR_RNDN);
        return NEO_bigfloat(a_mpfr);
    }
    NEO_throw_error("RuntimeError: bigint - object is not supported.");
}

NeoObject *NEO_bigint_multiply(NeoObject *a, NeoObject *b) {
    mpz_t a_value;
    mpz_init_set(a_value, NEO_vBigInt(a));
    if (b->prototype == NeoBigInt) {
        mpz_mul(a_value, a_value, NEO_vBigInt(b));
        return NEO_bigint(a_value);
    }
    if (b->prototype == NeoInt) {
        int b_value = NEO_vInt(b);
        if (b_value < 0) {
            mpz_mul_ui(a_value, a_value, -b_value);
        } else {
            mpz_mul_ui(a_value, a_value, b_value);
        }
        return NEO_bigint(a_value);
    }
    if (b->prototype == NeoDouble) {
        NEO_mpz2mpfr(a_value, a_mpfr);
        mpfr_mul_d(a_mpfr, a_mpfr, NEO_vDouble(b), MPFR_RNDN);
        return NEO_bigfloat(a_mpfr);
    }
    if (b->prototype == NeoBigFloat) {
        NEO_mpz2mpfr(a_value, a_mpfr);
        mpfr_mul(a_mpfr, a_mpfr, NEO_vBigFloat(b), MPFR_RNDN);
        return NEO_bigfloat(a_mpfr);
    }
    NEO_throw_error("RuntimeError: bigint * object is not supported.");
}

NeoObject *NEO_bigint_divide(NeoObject *a, NeoObject *b) {
    mpz_t a_value;
    mpz_init_set(a_value, NEO_vBigInt(a));
    if (b->prototype == NeoBigInt) {
        mpz_t b_value;
        mpz_init_set(b_value, NEO_vBigInt(b));
        mpz_div(a_value, a_value, b_value);
        mpz_clear(b_value);
        return NEO_bigint(a_value);
    }
    if (b->prototype == NeoInt) {
        int b_value = NEO_vInt(b);
        if (b_value < 0) {
            mpz_div_ui(a_value, a_value, -b_value);
        } else {
            mpz_div_ui(a_value, a_value, b_value);
        }
        return NEO_bigint(a_value);
    }
    if (b->prototype == NeoDouble) {
        NEO_mpz2mpfr(a_value, a_mpfr);
        mpfr_div_d(a_mpfr, a_mpfr, NEO_vDouble(b), MPFR_RNDN);
        return NEO_bigfloat(a_mpfr);
    }
    if (b->prototype == NeoBigFloat) {
        NEO_mpz2mpfr(a_value, a_mpfr);
        mpfr_div(a_mpfr, a_mpfr, NEO_vBigFloat(b), MPFR_RNDN);
        return NEO_bigfloat(a_mpfr);
    }
    NEO_throw_error("RuntimeError: bigint / object is not supported.");
}

NeoObject *NEO_bigint_modulo(NeoObject *a, NeoObject *b) {
    mpz_t a_value;
    mpz_init_set(a_value, NEO_vBigInt(a));
    if (b->prototype == NeoBigInt) {
        mpz_t b_value;
        mpz_init_set(b_value, NEO_vBigInt(b));
        mpz_mod(a_value, a_value, b_value);
        mpz_clear(b_value);
        return NEO_bigint(a_value);
    }
    if (b->prototype == NeoInt) {
        int b_value = NEO_vInt(b);
        if (b_value < 0) {
            mpz_mod_ui(a_value, a_value, -b_value);
        } else {
            mpz_mod_ui(a_value, a_value, b_value);
        }
        return NEO_bigint(a_value);
    }
    if (b->prototype == NeoDouble) {
        NEO_mpz2mpfr(a_value, a_mpfr);
        NEO_double2mpfr(NEO_vDouble(b), b_mpfr);
        mpfr_fmod(a_mpfr, a_mpfr, b_mpfr, MPFR_RNDN);
        return NEO_bigfloat(a_mpfr);
    }
    if (b->prototype == NeoBigFloat) {
        NEO_mpz2mpfr(a_value, a_mpfr);
        mpfr_fmod(a_mpfr, a_mpfr, NEO_vBigFloat(b), MPFR_RNDN);
        return NEO_bigfloat(a_mpfr);
    }
    NEO_throw_error("RuntimeError: bigint % object is not supported.");
}

NeoObject *NEO_bigint_power(NeoObject *a, NeoObject *b) {
    mpz_t a_value;
    mpz_init_set(a_value, NEO_vBigInt(a));
    if (b->prototype == NeoBigInt) {
        mpz_t b_value;
        mpz_init_set(b_value, NEO_vBigInt(b));
        mpz_pow_ui(a_value, a_value, mpz_get_ui(b_value));
        mpz_clear(b_value);
        return NEO_bigint(a_value);
    }
    if (b->prototype == NeoInt) {
        int b_value = NEO_vInt(b);
        if (b_value < 0) {
            NEO_mpz2mpfr(a_value, a_mpfr);
            mpfr_pow_si(a_mpfr, a_mpfr, b_value, MPFR_RNDN);
            return NEO_bigfloat(a_mpfr);
        }
        mpz_pow_ui(a_value, a_value, b_value);
        return NEO_bigint(a_value);
    }
    if (b->prototype == NeoDouble) {
        NEO_mpz2mpfr(a_value, a_mpfr);
        NEO_double2mpfr(NEO_vDouble(b), b_mpfr);
        mpfr_pow(a_mpfr, a_mpfr, b_mpfr, MPFR_RNDN);
        return NEO_bigfloat(a_mpfr);
    }
    if (b->prototype == NeoBigFloat) {
        NEO_mpz2mpfr(a_value, a_mpfr);
        mpfr_pow(a_mpfr, a_mpfr, NEO_vBigFloat(b), MPFR_RNDN);
        return NEO_bigfloat(a_mpfr);
    }
    NEO_throw_error("RuntimeError: bigint ** object is not supported.");
}

NeoObject *NEO_bigint_greater_than(NeoObject *a, NeoObject *b) {
    mpz_t a_value;
    mpz_init_set(a_value, NEO_vBigInt(a));
    if (b->prototype == NeoBigInt) {
        mpz_t b_value;
        mpz_init_set(b_value, NEO_vBigInt(b));
        return NEO_boolean(mpz_cmp(a_value, b_value) > 0);
    }
    if (b->prototype == NeoInt) {
        int b_value = NEO_vInt(b);
        return NEO_boolean(mpz_cmp_ui(a_value, b_value) > 0);
    }
    if (b->prototype == NeoDouble) {
        return NEO_boolean(mpz_cmp_d(a_value, NEO_vDouble(b)) > 0);
    }
    if (b->prototype == NeoBigFloat) {
        return NEO_boolean(mpz_cmp(a_value, NEO_vBigInt(b)) > 0);
    }
    NEO_throw_error("RuntimeError: bigint > object is not supported.");
}

NeoObject *NEO_bigint_less_than(NeoObject *a, NeoObject *b) {
    mpz_t a_value;
    mpz_init_set(a_value, NEO_vBigInt(a));
    if (b->prototype == NeoBigInt) {
        mpz_t b_value;
        mpz_init_set(b_value, NEO_vBigInt(b));
        return NEO_boolean(mpz_cmp(a_value, b_value) < 0);
    }
    if (b->prototype == NeoInt) {
        int b_value = NEO_vInt(b);
        return NEO_boolean(mpz_cmp_ui(a_value, b_value) < 0);
    }
    if (b->prototype == NeoDouble) {
        return NEO_boolean(mpz_cmp_d(a_value, NEO_vDouble(b)) < 0);
    }
    if (b->prototype == NeoBigFloat) {
        return NEO_boolean(mpz_cmp(a_value, NEO_vBigInt(b)) < 0);
    }
    NEO_throw_error("RuntimeError: bigint < object is not supported.");
}

NeoObject *NEO_bigint_equals(NeoObject *a, NeoObject *b) {
    mpz_t a_value;
    mpz_init_set(a_value, NEO_vBigInt(a));
    if (b->prototype == NeoBigInt) {
        mpz_t b_value;
        mpz_init_set(b_value, NEO_vBigInt(b));
        return NEO_boolean(mpz_cmp(a_value, b_value) == 0);
    }
    if (b->prototype == NeoInt) {
        int b_value = NEO_vInt(b);
        return NEO_boolean(mpz_cmp_ui(a_value, b_value) == 0);
    }
    if (b->prototype == NeoDouble) {
        return NEO_boolean(mpz_cmp_d(a_value, NEO_vDouble(b)) == 0);
    }
    if (b->prototype == NeoBigFloat) {
        return NEO_boolean(mpz_cmp(a_value, NEO_vBigInt(b)) == 0);
    }
    NEO_throw_error("RuntimeError: bigint == object is not supported.");
}

NeoObject *NEO_bigint_not(NeoObject *a) {
    return NEO_boolean(mpz_sgn(NEO_vBigInt(a)) == 0);
}