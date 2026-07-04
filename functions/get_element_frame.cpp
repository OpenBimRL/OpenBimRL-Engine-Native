#include "functions.h"
#include "element_frame.h"

[[maybe_unused]] void getElementFrame(void) {
    using namespace OpenBimRL::Engine;
    const auto ifcObject =
        (Types::IFC::IfcObjectPointer)Functions::getInputPointer(0);
    if (!ifcObject) return;

    Utils::ElementFrame elementFrame{};
    if (!Utils::getElementFrame(ifcObject, elementFrame)) return;

    const auto mem_size = 10 * sizeof(double);
    auto buffer = Functions::setOutputArray(0, mem_size);
    std::memcpy(buffer, elementFrame.point, 3 * sizeof(double));
    std::memcpy(static_cast<char*>(buffer) + 3 * sizeof(double), elementFrame.axisX,
                3 * sizeof(double));
    std::memcpy(static_cast<char*>(buffer) + 6 * sizeof(double), elementFrame.axisZ,
                3 * sizeof(double));
    Functions::setOutputInt(1, static_cast<uint32_t>(elementFrame.source));
}
