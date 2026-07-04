#ifndef OPENBIMRL_NATIVE_IFC_GEOM_UTILS_H
#define OPENBIMRL_NATIVE_IFC_GEOM_UTILS_H

#include <memory>
#include <vector>

#include <ifcgeom/ConversionSettings.h>
#include <ifcgeom/IfcGeomFilter.h>
#include <ifcgeom/Iterator.h>
#include <ifcgeom/taxonomy.h>
#include <ifcparse/IfcFile.h>

namespace OpenBimRL::Engine::Utils {

ifcopenshell::geometry::Settings defaultGeometrySettings();

std::unique_ptr<IfcGeom::Iterator> createGeometryIterator(
    IfcParse::IfcFile* file, const std::vector<IfcGeom::filter_t>& filters = {},
    int num_threads = 1);

void boundsToXzyArray(const ifcopenshell::geometry::taxonomy::point3& lower,
                      const ifcopenshell::geometry::taxonomy::point3& higher,
                      double out[6]);

}  // namespace OpenBimRL::Engine::Utils

#endif  // OPENBIMRL_NATIVE_IFC_GEOM_UTILS_H
