#include "./ifc_geom_utils.h"

#include <ifcgeom/hybrid_kernel.h>

namespace OpenBimRL::Engine::Utils {

ifcopenshell::geometry::Settings defaultGeometrySettings() {
    ifcopenshell::geometry::Settings settings;
    settings.get<ifcopenshell::geometry::settings::ApplyDefaultMaterials>().value =
        true;
    settings.get<ifcopenshell::geometry::settings::UseWorldCoords>().value = true;
    settings.get<ifcopenshell::geometry::settings::NoWireIntersectionCheck>().value =
        true;
    settings.get<ifcopenshell::geometry::settings::DisableOpeningSubtractions>()
        .value = true;
    settings.get<ifcopenshell::geometry::settings::DisableBooleanResult>().value =
        true;
    settings.get<ifcopenshell::geometry::settings::IteratorOutput>().value =
        ifcopenshell::geometry::settings::TRIANGULATED;
    return settings;
}

std::unique_ptr<IfcGeom::Iterator> createGeometryIterator(
    IfcParse::IfcFile* file, const std::vector<IfcGeom::filter_t>& filters,
    int num_threads) {
    auto settings = defaultGeometrySettings();
    return std::make_unique<IfcGeom::Iterator>(
        ifcopenshell::geometry::kernels::construct(file, "opencascade", settings),
        settings, file, filters, num_threads);
}

void boundsToXzyArray(const ifcopenshell::geometry::taxonomy::point3& lower,
                      const ifcopenshell::geometry::taxonomy::point3& higher,
                      double out[6]) {
    const auto& l = lower.ccomponents();
    const auto& h = higher.ccomponents();
    out[0] = l[0];
    out[1] = l[2];
    out[2] = l[1];
    out[3] = h[0];
    out[4] = h[2];
    out[5] = h[1];
}

}  // namespace OpenBimRL::Engine::Utils
