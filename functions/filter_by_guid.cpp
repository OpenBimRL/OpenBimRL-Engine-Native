#include "functions.h"
#include "utils.h"

#include <ifcparse/IfcSchema.h>
#include <ifcparse/IfcFile.h>


[[maybe_unused]] void filterByGUID()
{
    using namespace OpenBimRL::Engine::Types;
    IfcUtil::IfcBaseClass *ptr = nullptr; // the return pointer variable

    const auto input = OpenBimRL::Engine::Functions::getInputString(0); // get input from graph

    // check for null
    if (input == nullptr)
    {
        OpenBimRL::Engine::Functions::setOutputPointer(0, ptr); // return null
        return;
    }

    const auto guid = std::string(input); // create c++ string from char*
    IfcParse::IfcFile *file = OpenBimRL::Engine::Utils::getCurrentFile(); // get active file

    try
    {
        ptr = file->instance_by_guid(guid); // find ifc obj by guid
    }
        // thank you IfcOpenShell for documenting that this error exists...
    catch (const IfcParse::IfcException &)
    {
    }

    // set Pointer to ifc obj
    OpenBimRL::Engine::Functions::setOutputPointer(0, ptr);
}