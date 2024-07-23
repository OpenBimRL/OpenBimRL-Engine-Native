#include <ifcparse/Ifc4.h>

#include <BRepBndLib.hxx>

#include "functions.h"
#include "utils.h"

[[maybe_unused]] void getBoundingBox(void) {
    using namespace OpenBimRL::Engine;
    const auto ifcObject =
        (Types::IFC::IfcObjectPointer)Functions::getInputPointer(0);
    if (!ifcObject) return;

    const auto guid = Utils::getGUID(ifcObject);

    SerializerSettings settings;
    settings.set(IfcGeom::IteratorSettings::APPLY_DEFAULT_MATERIALS, true);
    settings.set(IfcGeom::IteratorSettings::USE_WORLD_COORDS, true);
    settings.set(IfcGeom::IteratorSettings::NO_WIRE_INTERSECTION_CHECK, true);
    settings.set(IfcGeom::IteratorSettings::DISABLE_OPENING_SUBTRACTIONS, true);
    settings.set(IfcGeom::IteratorSettings::DISABLE_BOOLEAN_RESULT, true);

    std::vector<IfcGeom::filter_t> filters;

    filters.emplace_back(std::function(([guid](IfcUtil::IfcBaseEntity* entity) {
        return guid == Utils::getGUID(entity);
    })));

    IfcGeom::Iterator geom_iterator(
        settings, OpenBimRL::Engine::Utils::getCurrentFile(), filters);

    geom_iterator.initialize();

    geom_iterator.compute_bounds(true);
    const auto lower = geom_iterator.bounds_min();
    const auto higher = geom_iterator.bounds_max();

    // prepare buffer from JVM
    const auto mem_size = 6 * sizeof(Standard_Real);
    auto buffer = OpenBimRL::Engine::Functions::setOutputArray(
        0, mem_size);  // JVM collects garbage automatically
    const Standard_Real values[6] = {
        // construct array
        lower.X(), lower.Z(), lower.Y(), higher.X(), higher.Z(), higher.Y(),
    };

    // copy memory from stack into JVM provided memory
    std::memcpy(buffer, values, mem_size);
}
