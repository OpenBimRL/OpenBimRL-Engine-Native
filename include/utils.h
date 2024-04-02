#ifndef OPENBIMRL_NATIVE_UTILS_H
#define OPENBIMRL_NATIVE_UTILS_H

#include <ifcparse/IfcFile.h>

namespace OpenBimRL::Engine::Utils {
    bool isIFC4();
    bool isIFC2x3();
    IfcParse::IfcFile *getCurrentFile();
}

#endif //OPENBIMRL_NATIVE_UTILS_H
