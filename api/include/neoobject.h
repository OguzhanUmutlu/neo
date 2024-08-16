#ifndef NEO_OBJECT_H
#define NEO_OBJECT_H

#include "neo.h"

NeoObject *NEO_object();

NeoObject *NEO_object_unset();

NeoObject *NEO_get_object_property(NeoObject *obj, char *key);

NeoObject *NEO_call_object_property(NeoObject *obj, char *key,
                                    NeoObject *baseObject, NeoObject **args,
                                    size_t arg_count,
                                    NeoHashMap *kwargs);

void NEO_set_object_property(NeoObject *obj, char *key, NeoObject *value);

void NEO_delete_object_property(NeoObject *obj, char *key);

#endif