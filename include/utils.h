#ifndef OPENBIMRL_NATIVE_UTILS_H
#define OPENBIMRL_NATIVE_UTILS_H
// clang-format off
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#include "types.h"

#include <TopoDS_Compound.hxx>

// clang-format on

namespace OpenBimRL::Engine::Utils {
bool isIFC4();
bool isIFC2x3();
bool isIFC4X3();
std::string getGUID(Types::IFC::IfcObjectPointer);
void setSilent(bool);
IfcParse::IfcFile* getCurrentFile();

using namespace OpenBimRL::Engine::Types;
IFC::IfcData getData(IFC::IfcObjectPointer);
void populateProperties(IFC::IfcData&, IFC::IfcObjectPointer);
}  // namespace OpenBimRL::Engine::Utils

#endif  // OPENBIMRL_NATIVE_UTILS_H
