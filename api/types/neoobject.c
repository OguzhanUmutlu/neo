#include <errno.h>
#include "neo.h"

NeoObject *NEO_object_unset() {
    NeoObject *obj = malloc(sizeof(NeoObject));
    obj->properties = NULL;
    obj->v = NULL;
    obj->ref_count = 1; // by creating it you are referencing it
    obj->prototype = NULL;
    obj->call = NULL;
    return obj;
}

NeoObject *NEO_object() {
    NeoObject *obj = NEO_object_unset();
    obj->properties = NEO_create_hashmap(32);
    return obj;
}

NeoObject *NEO_get_object_property(NeoObject *obj, char *key) {
    if (obj == NULL) {
        NEO_throw_error("RuntimeError: Cannot index into null.");
    }
    if (obj->prototype == NeoArray) {
        NeoArrayValue *array = (NeoArrayValue *) obj->v;
        if (strcmp(key, "length") == 0) {
            return NEO_int((int) array->length);
        }

        int index_value = (int) strtol(key, NULL, 10);

        if (errno != 0) {
            NEO_throw_error("RuntimeError: Invalid index.");
        }

        if (index_value < 0) {
            index_value += (int) array->length;
        }

        if (index_value < 0 || index_value >= array->length) {
            NEO_throw_error("RuntimeError: Index out of bounds.");
        }

        NeoObject *value = array->values[index_value];
        NEO_reference(value); // should be dereferenced after the index usage.

        return value;
    }
    if (obj->properties == NULL) {
        return NULL;
    }
    NeoObject *found = NEO_hashmap_search(obj->properties, key);
    if (found != NULL) {
        NEO_reference(found); // should be dereferenced after the index usage.
        return found;
    }

    if (obj->prototype == NULL) {
        return NULL;
    }
    return NEO_get_object_property(obj->prototype, key);
}

NeoObject *NEO_call_object_property(NeoObject *obj, char *key,
                                    NeoObject *baseObject, NeoObject **args,
                                    size_t arg_count,
                                    NeoHashMap *kwargs) {
    NeoObject *prop = NEO_get_object_property(obj, key);
    NeoObject *result = NEO_call(prop, baseObject, args, arg_count, kwargs);
    NEO_dereference(prop);
    return result;
}

void NEO_set_object_property(NeoObject *obj, char *key, NeoObject *value) {
    if (obj->properties == NULL) {
        NEO_throw_error("RuntimeError: Cannot set property on non-objects.");
    }
    NEO_hashmap_set(obj->properties, key, value);
}

void NEO_delete_object_property(NeoObject *obj, char *key) {
    if (obj->properties == NULL) {
        NEO_throw_error("RuntimeError: Cannot delete property on non-objects.");
    }
    NEO_hashmap_delete(obj->properties, key);
}