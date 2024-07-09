#ifndef OPENBIMRL_NATIVE_UTILS_H
#define OPENBIMRL_NATIVE_UTILS_H
#include <fstream>
#include <iostream>
#include <string>

#include "types.h"

#define SCHEMA_VERSIONS (2x3)(4)
#define SCHEMA_SEQ (2x3)(4)

#include <ifcgeom_schema_agnostic/IfcGeomElement.h>
#include <ifcgeom_schema_agnostic/Kernel.h>

#include <TopoDS_Compound.hxx>

namespace OpenBimRL::Engine::Utils {
bool isIFC4();
bool isIFC2x3();
void setSilent(bool);
IfcParse::IfcFile* getCurrentFile();
TopoDS_Compound create_shape(IfcGeom::IteratorSettings&, IfcUtil::IfcBaseClass*,
                             IfcUtil::IfcBaseClass* = nullptr);

std::optional<TopoDS_Compound> create_shape_default(IfcUtil::IfcBaseClass*);

using namespace OpenBimRL::Engine::Types;
IFC::IfcData getData(IFC::IfcObjectPointer);
void populateProperties(IFC::IfcData&, IFC::IfcObjectPointer);
}  // namespace OpenBimRL::Engine::Utils

#endif  // OPENBIMRL_NATIVE_UTILS_H
