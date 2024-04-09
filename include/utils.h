#ifndef OPENBIMRL_NATIVE_UTILS_H
#define OPENBIMRL_NATIVE_UTILS_H
#include "types.h"

#include <ifcparse/IfcFile.h>

namespace OpenBimRL::Engine::Utils {
    bool isIFC4();
    bool isIFC2x3();
    void setSilent(bool);
    IfcParse::IfcFile *getCurrentFile();

    using namespace OpenBimRL::Engine::Types;
    IFC::IfcData getData(IFC::IfcObjectPointer);
    void populateProperties(IFC::IfcData&, IFC::IfcObjectPointer);
}

#endif //OPENBIMRL_NATIVE_UTILS_H
