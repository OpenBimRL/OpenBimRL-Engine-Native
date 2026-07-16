#include "openbimrl/geometry/service.hpp"

#include <ifcgeom/hybrid_kernel.h>

#include "openbimrl/model/session.hpp"

namespace OpenBimRL::Native::Geometry {

ifcopenshell::geometry::Settings defaultGeometrySettings() {
    ifcopenshell::geometry::Settings settings;
    settings.get<ifcopenshell::geometry::settings::ApplyDefaultMaterials>()
        .value = true;
    settings.get<ifcopenshell::geometry::settings::UseWorldCoords>().value =
        true;
    settings.get<ifcopenshell::geometry::settings::NoWireIntersectionCheck>()
        .value = true;
    settings.get<ifcopenshell::geometry::settings::DisableOpeningSubtractions>()
        .value = true;
    settings.get<ifcopenshell::geometry::settings::DisableBooleanResult>()
        .value = true;
    settings.get<ifcopenshell::geometry::settings::IteratorOutput>().value =
        ifcopenshell::geometry::settings::TRIANGULATED;
    return settings;
}

std::unique_ptr<IfcGeom::Iterator> createGeometryIterator(
    IfcParse::IfcFile* file, const std::vector<IfcGeom::filter_t>& filters,
    int numThreads) {
    auto settings = defaultGeometrySettings();
    return std::make_unique<IfcGeom::Iterator>(
        ifcopenshell::geometry::kernels::construct(file, "opencascade",
                                                   settings),
        settings, file, filters, numThreads);
}

std::unique_ptr<IfcGeom::Iterator> iteratorForEntity(
    const Model::IfcSession& session, Model::IfcEntityRef entity,
    int numThreads) {
    const auto guid = Model::getGuid(entity);
    std::vector<IfcGeom::filter_t> filters;
    filters.emplace_back(std::function(([guid](IfcUtil::IfcBaseEntity* e) {
        return guid == Model::getGuid(Model::IfcEntityRef{e});
    })));
    return createGeometryIterator(session.file(), filters, numThreads);
}

std::optional<Bounds3d> boundsOf(const Model::IfcSession& session,
                                 Model::IfcEntityRef entity) {
    auto geomIterator = iteratorForEntity(session, entity);
    if (!geomIterator->initialize()) return std::nullopt;
    geomIterator->compute_bounds(true);
    const auto& lower = geomIterator->bounds_min().ccomponents();
    const auto& higher = geomIterator->bounds_max().ccomponents();
    return Bounds3d{lower[0], lower[1], lower[2], higher[0], higher[1],
                    higher[2]};
}

std::optional<Bounds3d> buildingBounds(const Model::IfcSession& session,
                                       int numThreads) {
    auto geomIterator =
        createGeometryIterator(session.file(), {}, numThreads);
    if (!geomIterator->initialize()) return std::nullopt;
    geomIterator->compute_bounds(true);
    const auto& lower = geomIterator->bounds_min().ccomponents();
    const auto& higher = geomIterator->bounds_max().ccomponents();
    return Bounds3d{lower[0], lower[1], lower[2], higher[0], higher[1],
                    higher[2]};
}

void boundsToXzyArray(const Bounds3d& bounds, double out[6]) {
    out[0] = bounds.minX;
    out[1] = bounds.minZ;
    out[2] = bounds.minY;
    out[3] = bounds.maxX;
    out[4] = bounds.maxZ;
    out[5] = bounds.maxY;
}

}  // namespace OpenBimRL::Native::Geometry
