#include <thread>

#include "functions.h"
#include "utils.h"
#include "ifc_geom_utils.h"

[[maybe_unused]] void calculatingBuildingBounds() {
    auto geom_iterator = OpenBimRL::Engine::Utils::createGeometryIterator(
        OpenBimRL::Engine::Utils::getCurrentFile(), {},
        static_cast<int>(std::thread::hardware_concurrency()));

    geom_iterator->initialize();
    geom_iterator->compute_bounds(true);
    const auto lower = geom_iterator->bounds_min();
    const auto higher = geom_iterator->bounds_max();

    const auto mem_size = 6 * sizeof(Standard_Real);
    auto buffer = OpenBimRL::Engine::Functions::setOutputArray(0, mem_size);
    Standard_Real values[6];
    OpenBimRL::Engine::Utils::boundsToXzyArray(lower, higher, values);
    std::memcpy(buffer, values, mem_size);
}
