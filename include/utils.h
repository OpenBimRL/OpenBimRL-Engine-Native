#ifndef OPENBIMRL_NATIVE_UTILS_H
#define OPENBIMRL_NATIVE_UTILS_H
#include "types.h"

#include <string>
#include <iostream>
#include <fstream>

#include <ifcparse/Ifc4.h>
#define IfcSchema Ifc4

#define SCHEMA_VERSIONS (2x3)(4)
#define SCHEMA_SEQ (2x3)(4)

#include <ifcparse/IfcBaseClass.h>
#include <ifcparse/IfcHierarchyHelper.h>
#include <ifcgeom/IfcGeom.h>


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
