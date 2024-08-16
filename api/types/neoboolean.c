#include "neo.h"

NeoObject *NEO_boolean(bool value) {
    return value ? NeoTrue : NeoFalse;
}