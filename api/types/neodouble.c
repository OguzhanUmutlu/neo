#include "neo.h"

NeoObject *NEO_double(double number) {
    NeoObject *obj = NEO_object_unset();
    obj->prototype = NeoDouble;
    NeoDoubleValue *v = malloc(sizeof(NeoDoubleValue));
    v->value = number;
    obj->v = v;
    return obj;
}

NeoObject *NEO_double_negate(NeoObject *a) {
    return NEO_double(-NEO_vDouble(a));
}

NeoObject *NEO_double_add(NeoObject *a, NeoObject *b) {
    double a_value = NEO_vDouble(a);
    if (b->prototype == NeoDouble) {
        return NEO_double(a_value + NEO_vDouble(b));
    }
    if (b->prototype == NeoInt) {
        return NEO_double(a_value + NEO_vInt(b));
    }
    if (b->prototype == NeoBigInt) {
        NEO_double2bigfloat(a_value, result);
        NEO_mpz2mpfr(NEO_vBigInt(b), b_mpfr);
        mpfr_add(NEO_vBigFloat(result), NEO_vBigFloat(result), b_mpfr,
                 MPFR_RNDN);
        return result;
    }
    if (b->prototype == NeoBigFloat) {
        NEO_double2bigfloat(a_value, result);
        mpfr_add(NEO_vBigFloat(result), NEO_vBigFloat(result), NEO_vBigFloat(b),
                 MPFR_RNDN);
        return result;
    }
    NEO_throw_error("RuntimeError: double + object is not supported.");
}

NeoObject *NEO_double_subtract(NeoObject *a, NeoObject *b) {
    double a_value = NEO_vDouble(a);
    if (b->prototype == NeoDouble) {
        return NEO_double(a_value - NEO_vDouble(b));
    }
    if (b->prototype == NeoInt) {
        return NEO_double(a_value - NEO_vInt(b));
    }
    if (b->prototype == NeoBigInt) {
        NEO_double2bigfloat(a_value, result);
        NEO_mpz2mpfr(NEO_vBigInt(b), b_mpfr);
        mpfr_sub(NEO_vBigFloat(result), NEO_vBigFloat(result), b_mpfr,
                 MPFR_RNDN);
        return result;
    }
    if (b->prototype == NeoBigFloat) {
        NEO_double2bigfloat(a_value, result);
        mpfr_sub(NEO_vBigFloat(result), NEO_vBigFloat(result), NEO_vBigFloat(b),
                 MPFR_RNDN);
        return result;
    }
    NEO_throw_error("RuntimeError: double - object is not supported.");
}

NeoObject *NEO_double_multiply(NeoObject *a, NeoObject *b) {
    double a_value = NEO_vDouble(a);
    if (b->prototype == NeoDouble) {
        return NEO_double(a_value * NEO_vDouble(b));
    }
    if (b->prototype == NeoInt) {
        return NEO_double(a_value * NEO_vInt(b));
    }
    if (b->prototype == NeoBigInt) {
        NEO_double2bigfloat(a_value, result);
        NEO_mpz2mpfr(NEO_vBigInt(b), b_mpfr);
        mpfr_mul(NEO_vBigFloat(result), NEO_vBigFloat(result), b_mpfr,
                 MPFR_RNDN);
        return result;
    }
    if (b->prototype == NeoBigFloat) {
        NEO_double2bigfloat(a_value, result);
        mpfr_mul(NEO_vBigFloat(result), NEO_vBigFloat(result), NEO_vBigFloat(b),
                 MPFR_RNDN);
        return result;
    }
    NEO_throw_error("RuntimeError: double * object is not supported.");
}

NeoObject *NEO_double_divide(NeoObject *a, NeoObject *b) {
    double a_value = NEO_vDouble(a);
    if (b->prototype == NeoDouble) {
        return NEO_double(a_value / NEO_vDouble(b));
    }
    if (b->prototype == NeoInt) {
        return NEO_double(a_value / NEO_vInt(b));
    }
    if (b->prototype == NeoBigInt) {
        NEO_double2bigfloat(a_value, result);
        NEO_mpz2mpfr(NEO_vBigInt(b), b_mpfr);
        mpfr_div(NEO_vBigFloat(result), NEO_vBigFloat(result), b_mpfr,
                 MPFR_RNDN);
        return result;
    }
    if (b->prototype == NeoBigFloat) {
        NEO_double2bigfloat(a_value, result);
        mpfr_div(NEO_vBigFloat(result), NEO_vBigFloat(result), NEO_vBigFloat(b),
                 MPFR_RNDN);
        return result;
    }
    NEO_throw_error("RuntimeError: double / object is not supported.");
}

NeoObject *NEO_double_modulo(NeoObject *a, NeoObject *b) {
    double a_value = NEO_vDouble(a);
    if (b->prototype == NeoDouble) {
        return NEO_double(fmod(a_value, NEO_vDouble(b)));
    }
    if (b->prototype == NeoInt) {
        return NEO_double(fmod(a_value, NEO_vInt(b)));
    }
    if (b->prototype == NeoBigInt) {
        NEO_double2bigfloat(a_value, result);
        NEO_mpz2mpfr(NEO_vBigInt(b), b_mpfr);
        mpfr_fmod(NEO_vBigFloat(result), NEO_vBigFloat(result), b_mpfr,
                  MPFR_RNDN);
        return result;
    }
    if (b->prototype == NeoBigFloat) {
        NEO_double2bigfloat(a_value, result);
        mpfr_fmod(NEO_vBigFloat(result), NEO_vBigFloat(result),
                  NEO_vBigFloat(b), MPFR_RNDN);
        return result;
    }
    NEO_throw_error("RuntimeError: double % object is not supported.");
}

NeoObject *NEO_double_power(NeoObject *a, NeoObject *b) {
    double a_value = NEO_vDouble(a);
    if (b->prototype == NeoDouble) {
        return NEO_double(pow(a_value, NEO_vDouble(b)));
    }
    if (b->prototype == NeoInt) {
        return NEO_double(pow(a_value, NEO_vInt(b)));
    }
    if (b->prototype == NeoBigInt) {
        NEO_double2bigfloat(a_value, result);
        NEO_mpz2mpfr(NEO_vBigInt(b), b_mpfr);
        mpfr_pow(NEO_vBigFloat(result), NEO_vBigFloat(result), b_mpfr,
                 MPFR_RNDN);
        return result;
    }
    if (b->prototype == NeoBigFloat) {
        NEO_double2bigfloat(a_value, result);
        mpfr_pow(NEO_vBigFloat(result), NEO_vBigFloat(result), NEO_vBigFloat(b),
                 MPFR_RNDN);
        return result;
    }
    NEO_throw_error("RuntimeError: double ** object is not supported.");
}

NeoObject *NEO_double_bit_and(NeoObject *a, NeoObject *b) {
    double a_value = NEO_vDouble(a);
    if (b->prototype == NeoDouble) {
        return NEO_int((int)a_value & (int)NEO_vDouble(b));
    }
    if (b->prototype == NeoInt) {
        return NEO_int((int)a_value & NEO_vInt(b));
    }
    NEO_throw_error("RuntimeError: double & object is not supported.");
}

NeoObject *NEO_double_bit_or(NeoObject *a, NeoObject *b) {
    double a_value = NEO_vDouble(a);
    if (b->prototype == NeoDouble) {
        return NEO_int((int)a_value | (int)NEO_vDouble(b));
    }
    if (b->prototype == NeoInt) {
        return NEO_int((int)a_value | NEO_vInt(b));
    }
    NEO_throw_error("RuntimeError: double | object is not supported.");
}

NeoObject *NEO_double_xor(NeoObject *a, NeoObject *b) {
    double a_value = NEO_vDouble(a);
    if (b->prototype == NeoDouble) {
        return NEO_int((int)a_value ^ (int)NEO_vDouble(b));
    }
    if (b->prototype == NeoInt) {
        return NEO_int((int)a_value ^ NEO_vInt(b));
    }
    NEO_throw_error("RuntimeError: double ^ object is not supported.");
}

NeoObject *NEO_double_shift_right(NeoObject *a, NeoObject *b) {
    double a_value = NEO_vDouble(a);
    if (b->prototype == NeoDouble) {
        return NEO_int((int)a_value >> (int)NEO_vDouble(b));
    }
    if (b->prototype == NeoInt) {
        return NEO_int((int)a_value >> NEO_vInt(b));
    }
    NEO_throw_error("RuntimeError: double >> object is not supported.");
}

NeoObject *NEO_double_shift_left(NeoObject *a, NeoObject *b) {
    double a_value = NEO_vDouble(a);
    if (b->prototype == NeoDouble) {
        return NEO_int((int)a_value << (int)NEO_vDouble(b));
    }
    if (b->prototype == NeoInt) {
        return NEO_int((int)a_value << NEO_vInt(b));
    }
    NEO_throw_error("RuntimeError: double << object is not supported.");
}

NeoObject *NEO_double_bit_not(NeoObject *a) {
    double a_value = NEO_vDouble(a);
    return NEO_int(~(int)a_value);
}

NeoObject *NEO_double_greater_than(NeoObject *a, NeoObject *b) {
    double a_value = NEO_vDouble(a);
    if (b->prototype == NeoDouble) {
        return NEO_boolean(a_value > NEO_vDouble(b));
    }
    if (b->prototype == NeoInt) {
        return NEO_boolean(a_value > NEO_vInt(b));
    }
    if (b->prototype == NeoBigInt) {
        NEO_double2bigfloat(a_value, a_mpfr);
        NEO_mpz2mpfr(NEO_vBigInt(b), b_mpfr);
        return NEO_boolean(mpfr_cmp(NEO_vBigFloat(a_mpfr), b_mpfr) > 0);
    }
    if (b->prototype == NeoBigFloat) {
        NEO_double2bigfloat(a_value, a_mpfr);
        return NEO_boolean(mpfr_cmp(NEO_vBigFloat(a_mpfr), NEO_vBigFloat(b)) >
                           0);
    }
    NEO_throw_error("RuntimeError: double > object is not supported.");
}

NeoObject *NEO_double_less_than(NeoObject *a, NeoObject *b) {
    double a_value = NEO_vDouble(a);
    if (b->prototype == NeoDouble) {
        return NEO_boolean(a_value < NEO_vDouble(b));
    }
    if (b->prototype == NeoInt) {
        return NEO_boolean(a_value < NEO_vInt(b));
    }
    if (b->prototype == NeoBigInt) {
        NEO_double2bigfloat(a_value, a_mpfr);
        NEO_mpz2mpfr(NEO_vBigInt(b), b_mpfr);
        return NEO_boolean(mpfr_cmp(NEO_vBigFloat(a_mpfr), b_mpfr) < 0);
    }
    if (b->prototype == NeoBigFloat) {
        NEO_double2bigfloat(a_value, a_mpfr);
        return NEO_boolean(mpfr_cmp(NEO_vBigFloat(a_mpfr), NEO_vBigFloat(b)) <
                           0);
    }
    NEO_throw_error("RuntimeError: double < object is not supported.");
}

NeoObject *NEO_double_equals(NeoObject *a, NeoObject *b) {
    double a_value = NEO_vDouble(a);
    if (b->prototype == NeoDouble) {
        return NEO_boolean(a_value == NEO_vDouble(b));
    }
    if (b->prototype == NeoInt) {
        return NEO_boolean(a_value == NEO_vInt(b));
    }
    if (b->prototype == NeoBigInt) {
        NEO_double2bigfloat(a_value, a_mpfr);
        NEO_mpz2mpfr(NEO_vBigInt(b), b_mpfr);
        return NEO_boolean(mpfr_cmp(NEO_vBigFloat(a_mpfr), b_mpfr) == 0);
    }
    if (b->prototype == NeoBigFloat) {
        NEO_double2bigfloat(a_value, a_mpfr);
        return NEO_boolean(mpfr_cmp(NEO_vBigFloat(a_mpfr), NEO_vBigFloat(b)) ==
                           0);
    }
    NEO_throw_error("RuntimeError: double == object is not supported.");
}

NeoObject *NEO_double_not(NeoObject *a) {
    double a_value = NEO_vDouble(a);
    return NEO_boolean(!a_value);
}