#ifndef NEO_DOUBLE_H
#define NEO_DOUBLE_H

#include "neo.h"

NeoObject *NEO_double(double number);

NeoObject *NEO_double_negate(NeoObject *a);

NeoObject *NEO_double_add(NeoObject *a, NeoObject *b);

NeoObject *NEO_double_subtract(NeoObject *a, NeoObject *b);

NeoObject *NEO_double_multiply(NeoObject *a, NeoObject *b);

NeoObject *NEO_double_divide(NeoObject *a, NeoObject *b);

NeoObject *NEO_double_modulo(NeoObject *a, NeoObject *b);

NeoObject *NEO_double_power(NeoObject *a, NeoObject *b);

NeoObject *NEO_double_bit_and(NeoObject *a, NeoObject *b);

NeoObject *NEO_double_bit_or(NeoObject *a, NeoObject *b);

NeoObject *NEO_double_xor(NeoObject *a, NeoObject *b);

NeoObject *NEO_double_shift_right(NeoObject *a, NeoObject *b);

NeoObject *NEO_double_shift_left(NeoObject *a, NeoObject *b);

NeoObject *NEO_double_bit_not(NeoObject *a);

NeoObject *NEO_double_greater_than(NeoObject *a, NeoObject *b);

NeoObject *NEO_double_less_than(NeoObject *a, NeoObject *b);

NeoObject *NEO_double_equals(NeoObject *a, NeoObject *b);

NeoObject *NEO_double_not(NeoObject *a);

#endif