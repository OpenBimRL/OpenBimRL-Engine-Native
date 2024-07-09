#ifndef OPENBIMRL_NATIVE_FUNCTIONS_H
#define OPENBIMRL_NATIVE_FUNCTIONS_H

#include "types.h"

extern "C" {
[[maybe_unused]] void filterByElement(void);
[[maybe_unused]] void filterByGUID(void);
[[maybe_unused]] void getBoundingBox(void);
}

#endif  // OPENBIMRL_NATIVE_FUNCTIONS_H
