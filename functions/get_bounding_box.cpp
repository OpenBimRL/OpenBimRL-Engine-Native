#include <BRepBndLib.hxx>

#include "functions.h"
#include "utils.h"
#include "ifc_geom_utils.h"

[[maybe_unused]] void getBoundingBox(void) {
    using namespace OpenBimRL::Engine;
    const auto ifcObject =
        (Types::IFC::IfcObjectPointer)Functions::getInputPointer(0);
    if (!ifcObject) return;

    const auto guid = Utils::getGUID(ifcObject);

    std::vector<IfcGeom::filter_t> filters;
    filters.emplace_back(std::function(([guid](IfcUtil::IfcBaseEntity* entity) {
        return guid == Utils::getGUID(entity);
    })));

    auto geom_iterator =
        Utils::createGeometryIterator(Utils::getCurrentFile(), filters);

    geom_iterator->initialize();
    geom_iterator->compute_bounds(true);
    const auto lower = geom_iterator->bounds_min();
    const auto higher = geom_iterator->bounds_max();

    const auto mem_size = 6 * sizeof(Standard_Real);
    auto buffer = Functions::setOutputArray(0, mem_size);
    Standard_Real values[6];
    Utils::boundsToXzyArray(lower, higher, values);
    std::memcpy(buffer, values, mem_size);
}
