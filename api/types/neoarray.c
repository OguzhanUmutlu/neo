#include "neo.h"

NeoObject *NEO_array() {
    NeoObject *obj = NEO_object();
    obj->prototype = NeoArray;
    NeoArrayValue *v = malloc(sizeof(NeoArrayValue));
    v->values = malloc(1);
    v->length = 0;
    obj->v = v;
    return obj;
}

void internal_NEO_array_push(NeoObject *this, NeoObject *value) {
    NeoArrayValue *v = this->v;

    v->values = realloc(v->values, (v->length + 1) * sizeof(NeoObject *));
    v->values[v->length++] = value;
    NEO_reference(value);
}

NeoObject *NEO_array_push(
        NeoObject *this, NeoObject **args, size_t arg_count, NeoHashMap *kwargs) {
    if (arg_count == 0) {
        return this;
    }
    for (size_t i = 0; i < arg_count; i++) {
        internal_NEO_array_push(this, args[i]);
    }
    return this;
}
