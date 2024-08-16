#include "neo.h"

NeoObject *NEO_string(char *string, size_t length) {
    NeoObject *obj = NEO_object_unset();
    obj->prototype = NeoString;
    NeoStringValue *v = malloc(sizeof(NeoStringValue));
    v->value = string;
    v->length = length;
    obj->v = v;
    return obj;
}

NeoObject *NEO_string2(char *string) {
    NeoObject *obj = NEO_object_unset();
    obj->prototype = NeoString;
    NeoStringValue *v = malloc(sizeof(NeoStringValue));
    v->value = string;
    v->length = strlen(string);
    obj->v = v;
    return obj;
}

NeoObject *NEO_string3(char *string) {
    NeoObject *obj = NEO_object_unset();
    obj->prototype = NeoString;
    NeoStringValue *v = malloc(sizeof(NeoStringValue));
    string = strdup(string);
    v->value = string;
    v->length = strlen(string);
    obj->v = v;
    return obj;
}