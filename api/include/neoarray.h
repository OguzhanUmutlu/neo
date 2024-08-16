#ifndef NEO_ARRAY_H
#define NEO_ARRAY_H

#include "neo.h"

NeoObject *NEO_array();

NeoObject *NEO_array_push(NeoObject *this, NeoObject **args, size_t arg_count, NeoHashMap *kwargs);

void internal_NEO_array_push(NeoObject *this, NeoObject *value);

#endif