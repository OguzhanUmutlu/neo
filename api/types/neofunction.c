#include "neo.h"

NeoObject *NEO_function(NeoFunctionValue func) {
    NeoObject *obj = NEO_object();
    obj->prototype = NeoFunction;
    obj->call = func;
    return obj;
}