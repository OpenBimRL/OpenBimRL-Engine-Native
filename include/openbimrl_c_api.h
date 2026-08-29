#ifndef OPENBIMRL_C_API_H
#define OPENBIMRL_C_API_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Load IFC file into the active session. Returns true on success. */
bool initIfc(const char* path);

#ifdef __cplusplus
}
#endif

#endif /* OPENBIMRL_C_API_H */
