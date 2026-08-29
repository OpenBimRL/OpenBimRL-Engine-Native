#ifndef OPENBIMRL_NATIVE_GEOMETRY_SERVICE_H
#define OPENBIMRL_NATIVE_GEOMETRY_SERVICE_H

#include <memory>
#include <optional>
#include <vector>

#include <ifcgeom/IfcGeomFilter.h>
#include <ifcgeom/Iterator.h>
#include <ifcparse/IfcFile.h>

#include "openbimrl/geometry/types.hpp"
#include "openbimrl/model/entity.hpp"
#include "openbimrl/model/session.hpp"
#include "openbimrl/pathfinding/types.hpp"

namespace OpenBimRL::Native::Geometry {

ifcopenshell::geometry::Settings defaultGeometrySettings();

std::unique_ptr<IfcGeom::Iterator> createGeometryIterator(
    IfcParse::IfcFile* file, const std::vector<IfcGeom::filter_t>& filters = {},
    int numThreads = 1);

std::unique_ptr<IfcGeom::Iterator> iteratorForEntity(
    const Model::IfcSession& session, Model::IfcEntityRef entity,
    int numThreads = 1);

std::optional<Bounds3d> boundsOf(const Model::IfcSession& session,
                                 Model::IfcEntityRef entity);

std::optional<Bounds3d> buildingBounds(const Model::IfcSession& session);

/** XY footprint triangle vertices (x,y pairs flattened). */
std::vector<double> footprintPolygonXY(const Model::IfcSession& session,
                                       Model::IfcEntityRef entity);

std::vector<Pathfinding::Segment2> extractSegmentsXY(
    const Model::IfcSession& session, Model::IfcEntityRef entity);

/** Remap engine (x,y,z) bounds to viewer/engine XZY array used by JVM. */
void boundsToXzyArray(const Bounds3d& bounds, double out[6]);

}  // namespace OpenBimRL::Native::Geometry

#endif  // OPENBIMRL_NATIVE_GEOMETRY_SERVICE_H
