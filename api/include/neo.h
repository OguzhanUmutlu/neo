#ifndef NEO_API_H
#define NEO_API_H

#include <gmp.h>
#include <mpfr.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define NEO_vInt(x) (((NeoIntValue *)x->v)->value)
#define NEO_vDouble(x) (((NeoDoubleValue *)x->v)->value)
#define NEO_vBigInt(x) (((NeoBigIntValue *)x->v)->value)
#define NEO_vBigFloat(x) (((NeoBigFloatValue *)x->v)->value)
#define NEO_pInt(x) ((NeoIntValue *)x->v)
#define NEO_pDouble(x) ((NeoDoubleValue *)x->v)
#define NEO_pBigInt(x) ((NeoBigIntValue *)x->v)
#define NEO_pBigFloat(x) ((NeoBigFloatValue *)x->v)
#define NEO_vString(x) ((NeoStringValue *)x->v)
#define NEO_vArray(x) ((NeoArrayValue *)x->v)
#define NEO_vFunction(x) ((NeoFunctionValue *)x->v)

#define NEO_int2bigint(a, result)                                              \
    NeoObject *result = NEO_bigint_unset();                                    \
    mpz_init_set_si(NEO_vBigInt(result), a);

#define NEO_int2bigfloat(a, result)                                            \
    NeoObject *result = NEO_bigfloat_unset();                                  \
    mpfr_init_set_si(NEO_vBigFloat(result), a, MPFR_RNDN);

#define NEO_double2bigint(a, result)                                           \
    NeoObject *result = NEO_bigint_unset();                                    \
    mpz_init_set_d(NEO_vBigInt(result), a, MPFR_RNDN);

#define NEO_double2bigfloat(a, result)                                         \
    NeoObject *result = NEO_bigfloat_unset();                                  \
    mpfr_init_set_d(NEO_vBigFloat(result), a, MPFR_RNDN);

#define NEO_int2mpz(a, result)                                                 \
    mpz_t result;                                                              \
    mpz_init_set_si(result, a);

#define NEO_double2mpfr(a, result)                                             \
    mpfr_t result;                                                             \
    mpfr_init_set_d(result, a, MPFR_RNDN);

#define NEO_mpz2mpfr(a, result)                                                \
    mpfr_t result;                                                             \
    mpfr_init_set_z(result, a, MPFR_RNDN);

#define NEO_mpfr2mpz(a, result)                                                \
    mpz_t result;                                                              \
    mpfr_get_z(result, a, MPFR_RNDN);

#define BLUE "\x1b[34m"
#define RESET "\x1b[0m"

typedef struct NeoObject NeoObject;
typedef struct NeoHashNode NeoHashNode;

struct NeoHashNode {
    char *key;
    NeoObject *value;
    NeoHashNode *next;
};

typedef struct {
    NeoHashNode **buckets;
    int size; // amount of buckets
    int count; // amount of keys
} NeoHashMap;

typedef struct {
    int64_t value;
} NeoIntValue;
typedef struct {
    double value;
} NeoDoubleValue;
typedef struct {
    mpz_t value;
} NeoBigIntValue;
typedef struct {
    mpfr_t value;
} NeoBigFloatValue;
typedef struct {
    bool value;
} NeoBooleanValue;
typedef struct {
    char *value;
    size_t length;
} NeoStringValue;
typedef struct {
    NeoObject **values;
    size_t length;
} NeoArrayValue;

typedef NeoObject *(*NeoFunctionValue)(
        NeoObject *, NeoObject **, size_t,
        NeoHashMap *); // source object, args, arg_count, kwargs

typedef NeoObject *(*NeoTernaryOperation)(NeoObject *, NeoObject *,
                                          NeoObject *);

typedef NeoObject *(*NeoBinaryOperation)(NeoObject *, NeoObject *);

typedef NeoObject *(*NeoUnaryOperation)(NeoObject *);

struct NeoObject {
    NeoObject *prototype;

    void *v;

    NeoHashMap *properties;

    int ref_count;

    NeoFunctionValue call;
};

extern NeoObject *NeoInt;
extern NeoObject *NeoDouble;
extern NeoObject *NeoBigInt;
extern NeoObject *NeoBigFloat;
extern NeoObject *NeoString;
extern NeoObject *NeoBoolean;
extern NeoObject *NeoArray;
extern NeoObject *NeoFunction;
extern NeoObject *NeoTrue;
extern NeoObject *NeoFalse;
extern NeoObject *NeoGlobPrint;
extern NeoObject *NeoGlobInput;
extern NeoHashMap *NeoEmptyHashmap;
extern char *currentStackTrace;

#define NEO_throw_error(message) printf("%s\n%s\n", currentStackTrace, message); exit(1)

unsigned int NEO_hash(const char *key, int size);

NeoHashMap *NEO_create_hashmap(int size);

void NEO_hashmap_set(NeoHashMap *map, const char *key, NeoObject *value);

NeoObject *NEO_hashmap_search(NeoHashMap *map, const char *key);

void NEO_hashmap_delete(NeoHashMap *map, const char *key);

void NEO_free_hashmap(NeoHashMap *map);

void NEO_free(NeoObject *obj);

void NEO_reference(NeoObject *obj);

void NEO_dereference(NeoObject *obj);

bool NEO_get_truthy(NeoObject *obj);

void internal_NEO_print(NeoObject *obj);

void internal_NEO_println(NeoObject *obj);

NeoObject *NEO_glob_print(NeoObject *this, NeoObject **args, size_t arg_count, NeoHashMap *kwargs);

NeoObject *NEO_add(NeoObject *a, NeoObject *b);

NeoObject *NEO_subtract(NeoObject *a, NeoObject *b);

NeoObject *NEO_multiply(NeoObject *a, NeoObject *b);

NeoObject *NEO_divide(NeoObject *a, NeoObject *b);

NeoObject *NEO_modulo(NeoObject *a, NeoObject *b);

NeoObject *NEO_power(NeoObject *a, NeoObject *b);

NeoObject *NEO_bit_and(NeoObject *a, NeoObject *b);

NeoObject *NEO_bit_or(NeoObject *a, NeoObject *b);

NeoObject *NEO_xor(NeoObject *a, NeoObject *b);

NeoObject *NEO_shift_right(NeoObject *a, NeoObject *b);

NeoObject *NEO_shift_left(NeoObject *a, NeoObject *b);

NeoObject *NEO_bit_not(NeoObject *a);

NeoObject *NEO_greater_than(NeoObject *a, NeoObject *b);

NeoObject *NEO_less_than(NeoObject *a, NeoObject *b);

NeoObject *NEO_equals(NeoObject *a, NeoObject *b);

NeoObject *NEO_not_equals(NeoObject *a, NeoObject *b);

NeoObject *NEO_greater_or_equals(NeoObject *a, NeoObject *b);

NeoObject *NEO_less_or_equals(NeoObject *a, NeoObject *b);

NeoObject *NEO_not(NeoObject *a);

NeoObject *NEO_call(
        NeoObject *obj, NeoObject *baseObject, NeoObject **args, size_t arg_count, NeoHashMap *kwargs);

char *NEO_to_string(NeoObject *obj);

char *NEO_format_object(NeoObject *obj);

NeoObject **NEO_make_object_array(int count, ...);

void NEO_init(int argc, char *argv[]);

void NEO_exit(int code);

#include "neoarray.h"
#include "neobigfloat.h"
#include "neobigint.h"
#include "neoboolean.h"
#include "neodouble.h"
#include "neofunction.h"
#include "neoint.h"
#include "neoobject.h"
#include "neostring.h"

#endif