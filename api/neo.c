#include "neo.h"
#include <mpfr.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>


// let a = 10.5 + 5.32 + b.a[5 + 7].d()[1:5:6]

NeoObject *NeoInt;
NeoObject *NeoDouble;
NeoObject *NeoBigInt;
NeoObject *NeoBigFloat;
NeoObject *NeoString;
NeoObject *NeoBoolean;
NeoObject *NeoArray;
NeoObject *NeoFunction;
NeoObject *NeoTrue;
NeoObject *NeoFalse;
NeoObject *NEO_argc;
NeoObject *NEO_argv;
char *currentStackTrace;
NeoHashMap *NeoEmptyHashmap;
NeoObject *NeoGlobPrint;
NeoObject *NeoGlobInput;

unsigned int NEO_hash(const char *key, int size) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % size;
}

NeoHashMap *NEO_create_hashmap(int size) {
    NeoHashMap *map = malloc(sizeof(NeoHashMap));
    map->size = size;
    map->count = 0;
    map->buckets = calloc(size, sizeof(NeoHashNode *));
    return map;
}

void NEO_hashmap_set(NeoHashMap *map, const char *key, NeoObject *value) {
    unsigned int index = NEO_hash(key, map->size);
    NeoHashNode *node = map->buckets[index];

    while (node != NULL) {
        if (strcmp(node->key, key) == 0) {
            if (node->value == value) return;
            NEO_dereference(node->value);
            node->value = value;
            NEO_reference(value);
            return;
        }
        node = node->next;
    }

    ++map->count;
    NeoHashNode *new_node = malloc(sizeof(NeoHashNode));
    new_node->key = strdup(key);
    new_node->value = value;
    new_node->next = map->buckets[index];
    map->buckets[index] = new_node;
    NEO_reference(value);
}

NeoObject *NEO_hashmap_search(NeoHashMap *map, const char *key) {
    unsigned int index = NEO_hash(key, map->size);
    NeoHashNode *node = map->buckets[index];
    while (node != NULL) {
        if (strcmp(node->key, key) == 0) {
            return node->value;
        }
        node = node->next;
    }
    return NULL;
}

void NEO_hashmap_delete(NeoHashMap *map, const char *key) {
    unsigned int index = NEO_hash(key, map->size);
    NeoHashNode *node = map->buckets[index];
    NeoHashNode *prev = NULL;

    while (node != NULL) {
        if (strcmp(node->key, key) == 0) {
            if (prev == NULL) {
                map->buckets[index] = node->next;
            } else {
                prev->next = node->next;
            }
            NEO_dereference(node->value);
            free(node->key);
            free(node);
            --map->count;
            return;
        }
        prev = node;
        node = node->next;
    }
}

void NEO_free_hashmap(NeoHashMap *map) {
    for (int i = 0; i < map->size; i++) {
        NeoHashNode *node = map->buckets[i];
        while (node != NULL) {
            NeoHashNode *temp = node;
            node = node->next;
            free(temp->key);
            free(temp);
        }
    }
    free(map->buckets);
    free(map);
}

void NEO_free_unsafe(NeoObject *obj) {
    if (obj->prototype == NeoBigInt) {
        mpz_clear(NEO_vBigInt(obj));
    } else if (obj->prototype == NeoBigFloat) {
        mpfr_clear(NEO_vBigFloat(obj));
    } else if (obj->prototype == NeoString) {
        free(NEO_vString(obj)->value);
    } else if (obj->prototype == NeoArray) {
        NeoArrayValue *array = NEO_vArray(obj);
        assert(array != NULL);
        for (size_t i = 0; i < array->length; ++i) {
            NEO_dereference(array->values[i]);
        }
        free(array->values);
    }

    if (obj->v != NULL) {
        free(obj->v);
    }

    NeoHashMap *props = obj->properties;
    if (props != NULL) {
        for (int i = 0; i < props->size; i++) {
            NeoHashNode *node = props->buckets[i];
            while (node != NULL) {
                NEO_dereference(node->value);
                node = node->next;
            }
        }
        NEO_free_hashmap(props);
    }

    free(obj);
}

void NEO_dereference(NeoObject *obj) {
    if (obj == NULL || obj->ref_count <= 0 || obj->prototype == NeoBoolean) {
        return;
    }
    --obj->ref_count;
    if (obj->ref_count == 0) {
        NEO_free_unsafe(obj);
    }
}

void NEO_reference(NeoObject *obj) {
    if (obj == NULL || obj->prototype == NeoBoolean) {
        return;
    }
    ++obj->ref_count;
}

bool NEO_get_truthy(NeoObject *obj) {
    if (obj == NULL) {
        return false;
    }
    if (obj->prototype == NeoBoolean) {
        return obj == NeoTrue;
    }
    if (obj->prototype == NeoInt) {
        return NEO_vInt(obj) != 0;
    }
    if (obj->prototype == NeoDouble) {
        return NEO_vDouble(obj) != 0;
    }
    if (obj->prototype == NeoBigInt) {
        return mpz_sgn(NEO_vBigInt(obj)) != 0;
    }
    if (obj->prototype == NeoBigFloat) {
        return mpfr_sgn(NEO_vBigFloat(obj)) != 0;
    }
    if (obj->prototype == NeoString) {
        return NEO_vString(obj)->length != 0;
    }
    if (obj->prototype == NeoArray) {
        return NEO_vArray(obj)->length != 0;
    }
    if (obj->prototype == NeoFunction) {
        return true;
    }
    return true;
}

void NEO_string_stop_at_zero(char *string) {
    bool seen_dot = false;
    for (size_t i = 0;; ++i) {
        if (string[i] == '\0') return;
        if (string[i] == '.') {
            seen_dot = true;
        }
        if (string[i] == '0' && seen_dot) {
            if (string[i - 1] == '.') {
                string[i - 1] = '\0';
                return;
            }
            string[i] = '\0';
            return;
        }
    }
}

size_t NEO_get_mpfr_str_length(const mpfr_t x) {
    if (mpfr_zero_p(x)) return 1;
    long exp = mpfr_get_exp(x) * log10(2);
    return (exp > 0 ? (size_t) exp + 1 : 1) + (size_t) (mpfr_get_prec(x) * log10(2)) + 3;
}

size_t NEO_get_mpz_str_length(const mpz_t x) {
    return (size_t) (mpz_sizeinbase(x, 2) * 0.30102) + 4;
}

char *NEO_to_string(NeoObject *obj) {
    if (obj == NULL) {
        return strdup("null");
    }
    if (obj->prototype == NeoInt) {
        char *buf = malloc(64);
        sprintf(buf, "%ld", NEO_vInt(obj));
        NEO_string_stop_at_zero(buf);
        return buf;
    }
    if (obj->prototype == NeoDouble) {
        char *buf = malloc(64);
        sprintf(buf, "%f", NEO_vDouble(obj));
        NEO_string_stop_at_zero(buf);
        return buf;
    }
    if (obj->prototype == NeoBigInt) {
        char *buf = malloc(NEO_get_mpz_str_length(NEO_vBigInt(obj)));
        gmp_sprintf(buf, "%Zd", NEO_vBigInt(obj));
        NEO_string_stop_at_zero(buf);
        size_t len = strlen(buf);
        buf[len] = 'n';
        buf[len + 1] = '\0';
        return buf;
    }
    if (obj->prototype == NeoBigFloat) {
        char *buf = malloc(NEO_get_mpfr_str_length(NEO_vBigFloat(obj)));
        mpfr_sprintf(buf, "%.16Rf", NEO_vBigFloat(obj));
        NEO_string_stop_at_zero(buf);
        size_t len = strlen(buf);
        buf[len] = 'n';
        buf[len + 1] = '\0';
        return buf;
    }
    if (obj->prototype == NeoString) {
        return strdup(NEO_vString(obj)->value);
    }
    if (obj->prototype == NeoBoolean) {
        return strdup(obj == NeoTrue ? "true" : "false");
    }
    NeoObject *res = NEO_call_object_property(obj, "__str__", obj, NULL, 0, NEO_create_hashmap(0));
    if (res->prototype != NeoString) {
        // return the address
        char *buf = malloc(64);
        sprintf(buf, "%p", obj);
        return buf;
    }
    return NEO_vString(res)->value;
}

void internal_NEO_print(NeoObject *obj) {
    char *str = NEO_to_string(obj);
    printf("%s", str);
    free(str);
}

void internal_NEO_println(NeoObject *obj) {
    internal_NEO_print(obj);
    printf("\n");
}

NeoObject *NEO_glob_print(
        NeoObject *this, NeoObject **args, size_t arg_count, NeoHashMap *kwargs) {
    bool inspect = NEO_get_truthy(NEO_hashmap_search(kwargs, "inspect"));
    char *str;
    char *(*format_func)(NeoObject *) = inspect ? NEO_format_object : NEO_to_string;
    for (size_t i = 0; i < arg_count; ++i) {
        if (i != 0) printf(" ");
        str = format_func(args[i]);
        printf("%s", str);
        free(str);
    }
    NeoObject *end = NEO_hashmap_search(kwargs, "end");
    if (end == NULL) {
        printf("\n");
    } else {
        char *end_value = NEO_to_string(end);
        printf("%s", end_value);
        free(end_value);
    }
    return NULL;
}

char *internal_NEO_input() {
    size_t bufferSize = 128;
    size_t position = 0;
    char *buffer = malloc(bufferSize * sizeof(char));
    int c;

    while (1) {
        c = getchar();

        if (c == '\n' || c == EOF) {
            buffer[position] = '\0';
            break;
        } else {
            buffer[position] = c;
        }
        if ((++position) >= bufferSize) {
            bufferSize *= 2;
            buffer = realloc(buffer, bufferSize);
        }
    }

    return buffer;
}

NeoObject *NEO_glob_input(
        NeoObject *this, NeoObject **args, size_t arg_count, NeoHashMap *kwargs) {
    if (arg_count > 0) {
        internal_NEO_print(args[0]);
    }

    char *str = internal_NEO_input();
    return NEO_string(str, strlen(str));
}

#define NEO_DefineOperation(op, key)                                           \
    NeoObject *NEO_##op(NeoObject *a, NeoObject *b) {                          \
        if (a->prototype == NeoInt) {                                          \
            return NEO_int_##op(a, b);                                         \
        }                                                                      \
        if (a->prototype == NeoDouble) {                                       \
            return NEO_double_##op(a, b);                                      \
        }                                                                      \
        if (a->prototype == NeoBigInt) {                                       \
            return NEO_bigint_##op(a, b);                                      \
        }                                                                      \
        if (a->prototype == NeoBigFloat) {                                     \
            return NEO_bigfloat_##op(a, b);                                    \
        }                                                                      \
        NeoObject **args = malloc(sizeof(NeoObject *));                        \
        args[0] = b;                                                           \
        return NEO_call_object_property(a, key, a, args, 1, NeoEmptyHashmap);  \
    }

#define NEO_DefineBitwiseOperation(op, key)                                    \
    NeoObject *NEO_##op(NeoObject *a, NeoObject *b) {                          \
        if (a->prototype == NeoInt) {                                          \
            return NEO_int_##op(a, b);                                         \
        }                                                                      \
        if (a->prototype == NeoDouble) {                                       \
            return NEO_double_##op(a, b);                                      \
        }                                                                      \
        NeoObject **args = malloc(sizeof(NeoObject *));                        \
        args[0] = b;                                                           \
        return NEO_call_object_property(a, key, a, args, 1, NeoEmptyHashmap);  \
    }

NEO_DefineOperation(add, "__add__")

NEO_DefineOperation(subtract, "__sub__")

NEO_DefineOperation(multiply, "__mul__")

NEO_DefineOperation(divide, "__div__")

NEO_DefineOperation(modulo, "__mod__")

NEO_DefineOperation(power, "__pow__")

NEO_DefineBitwiseOperation(bit_and, "__band__")

NEO_DefineBitwiseOperation(bit_or, "__bor__")

NEO_DefineBitwiseOperation(xor, "__xor__")

NEO_DefineBitwiseOperation(shift_right, "__shr__")

NEO_DefineBitwiseOperation(shift_left, "__shl__")

NeoObject *NEO_bit_not(NeoObject *a) {
    if (a->prototype == NeoInt) {
        return NEO_int_bit_not(a);
    }
    if (a->prototype == NeoDouble) {
        return NEO_double_bit_not(a);
    }
    return NEO_call_object_property(a, "__bnot__", a, NULL, 0, NeoEmptyHashmap);
}

NEO_DefineOperation(greater_than, "__gt__")

NEO_DefineOperation(less_than, "__lt__")

NeoObject *NEO_equals(NeoObject *a, NeoObject *b) {
    if (a->prototype == NeoInt) {
        return NEO_int_equals(a, b);
    }
    if (a->prototype == NeoDouble) {
        return NEO_double_equals(a, b);
    }
    if (a->prototype == NeoBigInt) {
        return NEO_bigint_equals(a, b);
    }
    if (a->prototype == NeoBigFloat) {
        return NEO_bigfloat_equals(a, b);
    }
    if (a->prototype == NeoString) {
        if (b->prototype != NeoString) {
            return NeoFalse;
        }

        return NEO_boolean(strcmp(NEO_vString(a)->value, NEO_vString(b)->value) == 0);
    }
    NeoObject **args = malloc(sizeof(NeoObject *));
    args[0] = b;
    return NEO_call_object_property(a, "__eq__", a, args, 1, NeoEmptyHashmap);
}

NeoObject *NEO_not(NeoObject *a) {
    if (a->prototype == NeoInt) {
        return NEO_int_not(a);
    }
    if (a->prototype == NeoDouble) {
        return NEO_double_not(a);
    }
    if (a->prototype == NeoBigInt) {
        return NEO_bigint_not(a);
    }
    if (a->prototype == NeoBigFloat) {
        return NEO_bigfloat_not(a);
    }
    return NEO_call_object_property(a, "__not__", a, NULL, 0, NeoEmptyHashmap);
}

NeoObject *NEO_not_equals(NeoObject *a, NeoObject *b) {
    return NEO_equals(a, b) == NeoTrue ? NeoFalse : NeoTrue;
}

NeoObject *NEO_greater_or_equals(NeoObject *a, NeoObject *b) {
    return NEO_less_than(a, b) == NeoTrue ? NeoFalse : NeoTrue;
}

NeoObject *NEO_less_or_equals(NeoObject *a, NeoObject *b) {
    return NEO_greater_than(a, b) == NeoTrue ? NeoFalse : NeoTrue;
}

NeoObject *NEO_negate(NeoObject *a) {
    if (a->prototype == NeoInt) {
        return NEO_int_not(a);
    }
    if (a->prototype == NeoDouble) {
        return NEO_double_not(a);
    }
    if (a->prototype == NeoBigInt) {
        return NEO_bigint_not(a);
    }
    if (a->prototype == NeoBigFloat) {
        return NEO_bigfloat_not(a);
    }
    return NEO_call_object_property(a, "__negate__", a, NULL, 0, NeoEmptyHashmap);
}

NeoObject *NEO_call(
        NeoObject *obj, NeoObject *baseObject, NeoObject **args, size_t arg_count, NeoHashMap *kwargs) {
    if (obj->call == NULL) {
        NEO_throw_error("RuntimeError: Cannot call a non-function.");
    }
    return obj->call(baseObject, args, arg_count, kwargs);
};

char *NEO_format_object(NeoObject *obj) {
    if (obj == NULL) {
        return "null";
    }
    if (obj->prototype == NeoInt || obj->prototype == NeoDouble) {
        char *str = NEO_to_string(obj);
        char *clr = (char *) malloc(5 + strlen(str) + 5 + 1);
        sprintf(clr, "\x1b[33m%s\x1b[0m", str);
        free(str);
        return clr;
    }
    if (obj->prototype == NeoBigInt || obj->prototype == NeoBigFloat) {
        char *str = NEO_to_string(obj);
        str[strlen(str) - 1] = '\0';
        char *clr = (char *) malloc(5 + strlen(str) + 5 + 1);
        sprintf(clr, "\x1b[33m%s\x1b[0m", str);
        free(str);
        return clr;
    }
    if (obj->prototype == NeoString) {
        size_t len = 0;
        char *value = NEO_vString(obj)->value;
        size_t value_len = NEO_vString(obj)->length;
        for (size_t i = 0; i < value_len; ++i) {
            if (value[i] == '\\' || value[i] == '"' || value[i] == '\n' || value[i] == '\r' || value[i] == '\t' ||
                value[i] == '\b' || value[i] == '\f' || value[i] == '\v') {
                len += 2;
            } else {
                ++len;
            }
        }
        char *str = malloc(len + 3 + 10);
        char *str_ptr = str;
        *str_ptr++ = '\x1b';
        *str_ptr++ = '[';
        *str_ptr++ = '3';
        *str_ptr++ = '2';
        *str_ptr++ = 'm';
        *str_ptr++ = '"';
        char *value_ptr = value;
        while (*value_ptr) {
            if (*value_ptr == '\\' || *value_ptr == '"') {
                *str_ptr++ = '\\';
                *str_ptr++ = *value_ptr++;
            } else if (*value_ptr == '\n') {
                *str_ptr++ = '\\';
                *str_ptr++ = 'n';
            } else if (*value_ptr == '\r') {
                *str_ptr++ = '\\';
                *str_ptr++ = 'r';
            } else if (*value_ptr == '\t') {
                *str_ptr++ = '\\';
                *str_ptr++ = 't';
            } else if (*value_ptr == '\b') {
                *str_ptr++ = '\\';
                *str_ptr++ = 'b';
            } else if (*value_ptr == '\f') {
                *str_ptr++ = '\\';
                *str_ptr++ = 'f';
            } else if (*value_ptr == '\v') {
                *str_ptr++ = '\\';
                *str_ptr++ = 'v';
            } else {
                *str_ptr++ = *value_ptr++;
            }
        }
        *str_ptr++ = '"';
        *str_ptr++ = '\x1b';
        *str_ptr++ = '[';
        *str_ptr++ = '0';
        *str_ptr++ = 'm';
        *str_ptr = '\0';
        return str;
    }
    if (obj->prototype == NeoBoolean) {
        return strdup(obj == NeoTrue ? "true" : "false");
    }
    if (obj->prototype == NeoArray) {
        NeoArrayValue *array = NEO_vArray(obj);
        assert(array != NULL); // sanity check
        size_t array_len = array->length;
        if (array_len == 0) {
            return strdup("[]");
        }
        size_t len = 0;
        for (int i = 0; i < array_len; ++i) {
            len += strlen(NEO_format_object(array->values[i])) + 2;
        }
        char *str = malloc(len + 6);
        char *str_ptr = str;
        *str_ptr++ = '[';
        *str_ptr++ = ' ';
        for (int i = 0; i < array_len; ++i) {
            char *json = NEO_format_object(array->values[i]);
            size_t json_len = strlen(json);
            memcpy(str_ptr, json, json_len);
            str_ptr += json_len;
            if (i < array_len - 1) {
                *str_ptr++ = ',';
                *str_ptr++ = ' ';
            }
            free(json);
        }
        *str_ptr++ = ' ';
        *str_ptr++ = ']';
        *str_ptr = '\0';
        return str;
    }
    if (obj->prototype == NeoFunction) {
        return strdup(BLUE "[Function]" RESET);
    }
    if (obj->properties->size == 0) {
        return strdup("{}");
    }
    size_t len = 0;

    NeoHashMap *props = obj->properties;

    for (int i = 0; i < props->size; i++) {
        NeoHashNode *node = props->buckets[i];
        while (node != NULL) {
            len += strlen(node->key) + strlen(NEO_format_object(node->value)) + 4;
            node = node->next;
        }
    }

    char *str = malloc(len + 6);
    char *str_ptr = str;
    *str_ptr++ = '{';
    *str_ptr++ = ' ';

    for (int i = 0; i < props->size; i++) {
        NeoHashNode *node = props->buckets[i];
        while (node != NULL) {
            char *value_str = NEO_format_object(node->value);
            size_t key_len = strlen(node->key);
            size_t value_len = strlen(value_str);
            memcpy(str_ptr, node->key, key_len);
            str_ptr += key_len;
            *str_ptr++ = ':';
            *str_ptr++ = ' ';
            memcpy(str_ptr, value_str, value_len);
            str_ptr += value_len;
            *str_ptr++ = ',';
            *str_ptr++ = ' ';
            free(value_str);
            node = node->next;
        }
    }

    str_ptr -= 2;

    *str_ptr++ = ' ';
    *str_ptr++ = '}';
    *str_ptr = '\0';
    return str;
}

NeoObject **NEO_make_object_array(int count, ...) {
    NeoObject **args = malloc(count * sizeof(NeoObject *));
    if (!args) {
        return NULL;
    }
    va_list ap;
    va_start(ap, count);
    for (int i = 0; i < count; i++) {
        args[i] = va_arg(ap, NeoObject *);
    }
    va_end(ap);
    return args;
}

char *NEO_version() {
    return "0.0.1";
}

void NEO_init(int argc, char *argv[]) {
    NeoInt = NEO_object_unset();
    NeoDouble = NEO_object_unset();
    NeoArray = NEO_object();
    NeoBigInt = NEO_object_unset();
    NeoBigFloat = NEO_object_unset();
    NeoString = NEO_object_unset();
    NeoBoolean = NEO_object_unset();
    NeoFunction = NEO_object_unset();
    NeoTrue = NEO_object_unset();
    NeoFalse = NEO_object_unset();

    NeoTrue->prototype = NeoBoolean;
    NeoFalse->prototype = NeoBoolean;
    NeoTrue->v = malloc(sizeof(NeoBooleanValue));
    NeoFalse->v = malloc(sizeof(NeoBooleanValue));

    ((NeoBooleanValue *) NeoTrue->v)->value = true;
    ((NeoBooleanValue *) NeoFalse->v)->value = false;

    NEO_set_object_property(NeoArray, "push", NEO_function(NEO_array_push));

    NEO_argc = NEO_int(argc);
    NEO_argv = NEO_array();
    for (int i = 0; i < argc; i++) {
        internal_NEO_array_push(NEO_argv, NEO_string(argv[i], strlen(argv[i])));
    }

    NeoGlobPrint = NEO_function(NEO_glob_print);
    NeoGlobInput = NEO_function(NEO_glob_input);
    NeoEmptyHashmap = NEO_create_hashmap(1);
}

void NEO_exit(int code) {
    NEO_free_unsafe(NeoInt);
    NEO_free_unsafe(NeoDouble);
    NEO_free_unsafe(NeoArray);
    NEO_free_unsafe(NeoBigInt);
    NEO_free_unsafe(NeoBigFloat);
    NEO_free_unsafe(NeoString);
    NEO_free_unsafe(NeoBoolean);
    NEO_free_unsafe(NeoFunction);
    NEO_free_unsafe(NeoTrue);
    NEO_free_unsafe(NeoFalse);
    NEO_free_unsafe(NEO_argc);
    NEO_free_unsafe(NEO_argv);
    NEO_free_unsafe(NeoGlobPrint);
    NEO_free_unsafe(NeoGlobInput);
    NEO_free_hashmap(NeoEmptyHashmap);
    exit(code);
}