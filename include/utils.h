#ifndef OPENBIMRL_NATIVE_UTILS_H
#define OPENBIMRL_NATIVE_UTILS_H
#include "types.h"

#include <string>
#include <iostream>
#include <fstream>

#include <TColgp_Array2OfPnt.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>

#include <Geom_BSplineSurface.hxx>

#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_NurbsConvert.hxx>

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>

#include <BRepAlgoAPI_Cut.hxx>

#include <Standard_Version.hxx>

#include <ifcparse/Ifc4.h>
#define IfcSchema (Ifc4)

#define SCHEMA_VERSIONS=(2x3)(4)
#define SCHEMA_SEQ = (2x3)(4)

#include <ifcparse/IfcBaseClass.h>
#include <ifcparse/IfcHierarchyHelper.h>
#include <ifcgeom/IfcGeom.h>
#include <ifcgeom_schema_agnostic/empty.h>


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
