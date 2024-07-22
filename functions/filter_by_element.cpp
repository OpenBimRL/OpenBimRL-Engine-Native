#include "functions.h"
#include "utils.h"

[[maybe_unused]] void filterByElement(void) {
    using namespace OpenBimRL::Engine;
    Types::JNA::String input =
        Functions::getInputString(0);  // get input from graph

    // check for null
    if (input == nullptr) {
        Functions::setOutputPointer(0, nullptr);
        return;
    }

    const auto ifcClass = std::string(input);  // create c++ string from char*

    const auto file = Utils::getCurrentFile();  // get active file

    const auto instances = file->instances_by_type(
        ifcClass);  // get instances by ifc class excluding subtypes
    const auto size = instances->size();
    if (!size) {
        Functions::setOutputPointer(0, nullptr);
        return;
    }

    const auto buffer =
        (Types::IFC::IfcObjectPointer *)Functions::setOutputArray(
            0, size * sizeof(void *));

    std::copy(instances->begin(), instances->end(), buffer);
}