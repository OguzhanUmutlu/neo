#ifndef NEO_BIGFLOAT_H
#define NEO_BIGFLOAT_H

#include "neo.h"

NeoObject *NEO_bigfloat_unset();

NeoObject *NEO_bigfloat(mpfr_t value);

NeoObject *NEO_bigfloat_str(char *string);

NeoObject *NEO_bigfloat_negate(NeoObject *a);

NeoObject *NEO_bigfloat_add(NeoObject *a, NeoObject *b);

NeoObject *NEO_bigfloat_subtract(NeoObject *a, NeoObject *b);

NeoObject *NEO_bigfloat_multiply(NeoObject *a, NeoObject *b);

NeoObject *NEO_bigfloat_divide(NeoObject *a, NeoObject *b);

NeoObject *NEO_bigfloat_modulo(NeoObject *a, NeoObject *b);

NeoObject *NEO_bigfloat_power(NeoObject *a, NeoObject *b);

NeoObject *NEO_bigfloat_greater_than(NeoObject *a, NeoObject *b);

NeoObject *NEO_bigfloat_less_than(NeoObject *a, NeoObject *b);

NeoObject *NEO_bigfloat_equals(NeoObject *a, NeoObject *b);

NeoObject *NEO_bigfloat_not(NeoObject *a);

#endif