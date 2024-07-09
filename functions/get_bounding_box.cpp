#include <ifcparse/Ifc4.h>

#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>

#include "functions.h"
#include "utils.h"

[[maybe_unused]] void getBoundingBox(void) {
    using namespace OpenBimRL::Engine;
    const auto ifcObject =
        (Types::IFC::IfcObjectPointer)Functions::getInputPointer(0);
    if (!ifcObject) return;

    const auto element = Utils::create_shape_default(ifcObject);

    if (!element.has_value()) return;

    Bnd_Box bbox;

    BRepBndLib::AddOptimal(
        element.value(),
        bbox);  // automatically expands the box to fit given geometry by adding
                // points util every point is inside the box
    const auto corner_min = bbox.CornerMin(), corner_max = bbox.CornerMax();

    // prepare buffer from JVM
    const auto mem_size = 6 * sizeof(Standard_Real);
    auto buffer = Functions::setOutputArray(
        0, mem_size);  // JVM collects garbage automatically
    const Standard_Real values[6] = {
        // construct array
        corner_min.X(), corner_min.Y(), corner_min.Z(),
        corner_max.X(), corner_max.Y(), corner_max.Z(),
    };

    // copy memory from stack into JVM provided memory
    std::memcpy(buffer, values, mem_size);
}
