#include "functions.h"
#include "utils.h"

[[maybe_unused]] void calculatingBuildingBounds() {
    SerializerSettings settings;
    settings.set(IfcGeom::IteratorSettings::APPLY_DEFAULT_MATERIALS, true);
    settings.set(IfcGeom::IteratorSettings::USE_WORLD_COORDS, true);
    settings.set(IfcGeom::IteratorSettings::NO_WIRE_INTERSECTION_CHECK, true);
    settings.set(IfcGeom::IteratorSettings::DISABLE_OPENING_SUBTRACTIONS, true);
    settings.set(IfcGeom::IteratorSettings::DISABLE_BOOLEAN_RESULT, true);

    IfcGeom::Iterator geom_iterator(
        settings, OpenBimRL::Engine::Utils::getCurrentFile(),
        (std::int32_t)std::thread::hardware_concurrency());

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
        // Convert XYZ into XZY cause some Idiot thought it would be beneficial
        // to use XZ plane as ground
        lower.X(), lower.Z(), lower.Y(), higher.X(), higher.Z(), higher.Y(),
    };

    // copy memory from stack into JVM provided memory
    std::memcpy(buffer, values, mem_size);
}