#include "neo.h"

NeoObject *NEO_bigfloat_unset() {
    NeoObject *obj = NEO_object_unset();
    obj->prototype = NeoBigFloat;
    obj->v = malloc(sizeof(NeoBigFloatValue));
    return obj;
}

NeoObject *NEO_bigfloat(mpfr_t value) {
    NeoObject *obj = NEO_bigfloat_unset();
    mpfr_init_set(NEO_vBigFloat(obj), value, MPFR_RNDN);
    return obj;
}

NeoObject *NEO_bigfloat_str(char *string) {
    NeoObject *obj = NEO_bigfloat_unset();
    NeoBigFloatValue *v = malloc(sizeof(NeoBigFloatValue));
    mpfr_init_set_str(v->value, string, 10, MPFR_RNDN);
    obj->v = v;
    return obj;
}

NeoObject *NEO_bigfloat_negate(NeoObject *a) {
    mpfr_t a_value;
    mpfr_init_set(a_value, NEO_vBigFloat(a), MPFR_RNDN);
    mpfr_neg(a_value, a_value, MPFR_RNDN);
    return NEO_bigfloat(a_value);
}

NeoObject *NEO_bigfloat_add(NeoObject *a, NeoObject *b) {
    mpfr_t a_value;
    mpfr_init_set(a_value, NEO_vBigFloat(a), MPFR_RNDN);
    if (b->prototype == NeoBigFloat) {
        mpfr_add(a_value, a_value, NEO_vBigFloat(b), MPFR_RNDN);
        return NEO_bigfloat(a_value);
    }
    if (b->prototype == NeoDouble) {
        mpfr_add_d(a_value, a_value, NEO_vDouble(b), MPFR_RNDN);
        return NEO_bigfloat(a_value);
    }
    if (b->prototype == NeoInt) {
        mpfr_add_si(a_value, a_value, NEO_vInt(b), MPFR_RNDN);
        return NEO_bigfloat(a_value);
    }
    NEO_throw_error("RuntimeError: bigfloat + object is not supported.");
}

NeoObject *NEO_bigfloat_subtract(NeoObject *a, NeoObject *b) {
    mpfr_t a_value;
    mpfr_init_set(a_value, NEO_vBigFloat(a), MPFR_RNDN);
    if (b->prototype == NeoBigFloat) {
        mpfr_sub(a_value, a_value, NEO_vBigFloat(b), MPFR_RNDN);
        return NEO_bigfloat(a_value);
    }
    if (b->prototype == NeoDouble) {
        mpfr_sub_d(a_value, a_value, NEO_vDouble(b), MPFR_RNDN);
        return NEO_bigfloat(a_value);
    }
    if (b->prototype == NeoInt) {
        mpfr_sub_si(a_value, a_value, NEO_vInt(b), MPFR_RNDN);
        return NEO_bigfloat(a_value);
    }
    NEO_throw_error("RuntimeError: bigfloat - object is not supported.");
}

NeoObject *NEO_bigfloat_multiply(NeoObject *a, NeoObject *b) {
    mpfr_t a_value;
    mpfr_init_set(a_value, NEO_vBigFloat(a), MPFR_RNDN);
    if (b->prototype == NeoBigFloat) {
        mpfr_mul(a_value, a_value, NEO_vBigFloat(b), MPFR_RNDN);
        return NEO_bigfloat(a_value);
    }
    if (b->prototype == NeoDouble) {
        mpfr_mul_d(a_value, a_value, NEO_vDouble(b), MPFR_RNDN);
        return NEO_bigfloat(a_value);
    }
    if (b->prototype == NeoInt) {
        mpfr_mul_si(a_value, a_value, NEO_vInt(b), MPFR_RNDN);
        return NEO_bigfloat(a_value);
    }
    NEO_throw_error("RuntimeError: bigfloat * object is not supported.");
}

NeoObject *NEO_bigfloat_divide(NeoObject *a, NeoObject *b) {
    mpfr_t a_value;
    mpfr_init_set(a_value, NEO_vBigFloat(a), MPFR_RNDN);
    if (b->prototype == NeoBigFloat) {
        mpfr_div(a_value, a_value, NEO_vBigFloat(b), MPFR_RNDN);
        return NEO_bigfloat(a_value);
    }
    if (b->prototype == NeoDouble) {
        mpfr_div_d(a_value, a_value, NEO_vDouble(b), MPFR_RNDN);
        return NEO_bigfloat(a_value);
    }
    if (b->prototype == NeoInt) {
        mpfr_div_si(a_value, a_value, NEO_vInt(b), MPFR_RNDN);
        return NEO_bigfloat(a_value);
    }
    NEO_throw_error("RuntimeError: bigfloat / object is not supported.");
}

NeoObject *NEO_bigfloat_modulo(NeoObject *a, NeoObject *b) {
    mpfr_t a_value;
    mpfr_init_set(a_value, NEO_vBigFloat(a), MPFR_RNDN);
    if (b->prototype == NeoBigFloat) {
        mpfr_fmod(a_value, a_value, NEO_vBigFloat(b), MPFR_RNDN);
        return NEO_bigfloat(a_value);
    }
    if (b->prototype == NeoDouble) {
        NEO_double2mpfr(NEO_vDouble(b), b_value);
        mpfr_fmod(a_value, a_value, b_value, MPFR_RNDN);
        return NEO_bigfloat(a_value);
    }
    if (b->prototype == NeoInt) {
        NEO_double2mpfr(NEO_vInt(b), b_value);
        mpfr_fmod(a_value, a_value, b_value, MPFR_RNDN);
        return NEO_bigfloat(a_value);
    }
    NEO_throw_error("RuntimeError: bigfloat % object is not supported.");
}

NeoObject *NEO_bigfloat_power(NeoObject *a, NeoObject *b) {
    mpfr_t a_value;
    mpfr_init_set(a_value, NEO_vBigFloat(a), MPFR_RNDN);
    if (b->prototype == NeoBigFloat) {
        mpfr_pow(a_value, a_value, NEO_vBigFloat(b), MPFR_RNDN);
        return NEO_bigfloat(a_value);
    }
    if (b->prototype == NeoDouble) {
        NEO_double2mpfr(NEO_vDouble(b), b_value);
        mpfr_pow(a_value, a_value, b_value, MPFR_RNDN);
        return NEO_bigfloat(a_value);
    }
    if (b->prototype == NeoInt) {
        NEO_double2mpfr(NEO_vInt(b), b_value);
        mpfr_pow(a_value, a_value, b_value, MPFR_RNDN);
        return NEO_bigfloat(a_value);
    }
    NEO_throw_error("RuntimeError: bigfloat ^ object is not supported.");
}

NeoObject *NEO_bigfloat_greater_than(NeoObject *a, NeoObject *b) {
    mpfr_t a_value;
    mpfr_init_set(a_value, NEO_vBigFloat(a), MPFR_RNDN);
    if (b->prototype == NeoBigFloat) {
        return NEO_boolean(mpfr_cmp(a_value, NEO_vBigFloat(b)) > 0);
    }
    if (b->prototype == NeoDouble) {
        return NEO_boolean(mpfr_cmp_d(a_value, NEO_vDouble(b)) > 0);
    }
    if (b->prototype == NeoInt) {
        return NEO_boolean(mpfr_cmp_si(a_value, NEO_vInt(b)) > 0);
    }
    NEO_throw_error("RuntimeError: bigfloat > object is not supported.");
}

NeoObject *NEO_bigfloat_less_than(NeoObject *a, NeoObject *b) {
    mpfr_t a_value;
    mpfr_init_set(a_value, NEO_vBigFloat(a), MPFR_RNDN);
    if (b->prototype == NeoBigFloat) {
        return NEO_boolean(mpfr_cmp(a_value, NEO_vBigFloat(b)) < 0);
    }
    if (b->prototype == NeoDouble) {
        return NEO_boolean(mpfr_cmp_d(a_value, NEO_vDouble(b)) < 0);
    }
    if (b->prototype == NeoInt) {
        return NEO_boolean(mpfr_cmp_si(a_value, NEO_vInt(b)) < 0);
    }
    NEO_throw_error("RuntimeError: bigfloat < object is not supported.");
}

NeoObject *NEO_bigfloat_equals(NeoObject *a, NeoObject *b) {
    mpfr_t a_value;
    mpfr_init_set(a_value, NEO_vBigFloat(a), MPFR_RNDN);
    if (b->prototype == NeoBigFloat) {
        return NEO_boolean(mpfr_cmp(a_value, NEO_vBigFloat(b)) == 0);
    }
    if (b->prototype == NeoDouble) {
        return NEO_boolean(mpfr_cmp_d(a_value, NEO_vDouble(b)) == 0);
    }
    if (b->prototype == NeoInt) {
        return NEO_boolean(mpfr_cmp_si(a_value, NEO_vInt(b)) == 0);
    }
    NEO_throw_error("RuntimeError: bigfloat == object is not supported.");
}

NeoObject *NEO_bigfloat_not(NeoObject *a) {
    mpfr_t a_value;
    mpfr_init_set(a_value, NEO_vBigFloat(a), MPFR_RNDN);
    return NEO_boolean(mpfr_sgn(a_value) != 0);
}