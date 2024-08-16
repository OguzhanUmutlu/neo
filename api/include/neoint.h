#ifndef NEO_INT_H
#define NEO_INT_H

#include "neo.h"

NeoObject *NEO_int(int number);

NeoObject *NEO_int_negate(NeoObject *a);

NeoObject *NEO_int_add(NeoObject *a, NeoObject *b);

NeoObject *NEO_int_subtract(NeoObject *a, NeoObject *b);

NeoObject *NEO_int_multiply(NeoObject *a, NeoObject *b);

NeoObject *NEO_int_divide(NeoObject *a, NeoObject *b);

NeoObject *NEO_int_modulo(NeoObject *a, NeoObject *b);

NeoObject *NEO_int_power(NeoObject *a, NeoObject *b);

NeoObject *NEO_int_bit_and(NeoObject *a, NeoObject *b);

NeoObject *NEO_int_bit_or(NeoObject *a, NeoObject *b);

NeoObject *NEO_int_xor(NeoObject *a, NeoObject *b);

NeoObject *NEO_int_shift_right(NeoObject *a, NeoObject *b);

NeoObject *NEO_int_shift_left(NeoObject *a, NeoObject *b);

NeoObject *NEO_int_bit_not(NeoObject *a);

NeoObject *NEO_int_greater_than(NeoObject *a, NeoObject *b);

NeoObject *NEO_int_less_than(NeoObject *a, NeoObject *b);

NeoObject *NEO_int_equals(NeoObject *a, NeoObject *b);

NeoObject *NEO_int_not(NeoObject *a);

#endif