#ifndef NEO_BIGINT_H
#define NEO_BIGINT_H

#include "neo.h"

NeoObject *NEO_bigint_unset();

NeoObject *NEO_bigint(mpz_t value);

NeoObject *NEO_bigint_str(char *string);

NeoObject *NEO_bigint_negate(NeoObject *a);

NeoObject *NEO_bigint_add(NeoObject *a, NeoObject *b);

NeoObject *NEO_bigint_subtract(NeoObject *a, NeoObject *b);

NeoObject *NEO_bigint_multiply(NeoObject *a, NeoObject *b);

NeoObject *NEO_bigint_divide(NeoObject *a, NeoObject *b);

NeoObject *NEO_bigint_modulo(NeoObject *a, NeoObject *b);

NeoObject *NEO_bigint_power(NeoObject *a, NeoObject *b);

NeoObject *NEO_bigint_greater_than(NeoObject *a, NeoObject *b);

NeoObject *NEO_bigint_less_than(NeoObject *a, NeoObject *b);

NeoObject *NEO_bigint_equals(NeoObject *a, NeoObject *b);

NeoObject *NEO_bigint_not(NeoObject *a);

#endif