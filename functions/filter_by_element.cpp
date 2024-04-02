#include "functions.h"
#include "utils.h"

[[maybe_unused]] void filterByElement(void) {
    using namespace OpenBimRL::Engine::Types;
    JNA::String input = OpenBimRL::Engine::Functions::getInputString(0); // get input from graph

    // check for null
    if (input == nullptr) {
        OpenBimRL::Engine::Functions::setOutputPointer(0, nullptr);
        return;
    }

    const auto ifcClass = std::string(input); // create c++ string from char*

    IfcParse::IfcFile *file = OpenBimRL::Engine::Utils::getCurrentFile(); // get active file

    const auto instances = file->instances_by_type(ifcClass); // get instances by ifc class excluding subtypes
    const auto size = instances->size();
    if (!size) {
        OpenBimRL::Engine::Functions::setOutputPointer(0, nullptr);
        return;
    }

    const auto buffer = (OpenBimRL::Engine::Types::IFC::IfcObjectPointer *) OpenBimRL::Engine::Functions::setOutputArray(
            0, size);

    std::copy(instances->begin(), instances->end(), buffer);
}