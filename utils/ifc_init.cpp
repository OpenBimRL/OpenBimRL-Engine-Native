#include "utils.h"
#include "lib.h"

#include <iostream>
#include <ifcparse/IfcFile.h>

static IfcParse::IfcFile *currentFile;
static bool silent = false;

// hopefully this doesn't leak memory... ¯\_(ツ)_/¯
static void setCurrentFile(IfcParse::IfcFile *newFile)
{
    // clang-tidy: if not necessary due to deleting null pointer has no effect
    delete currentFile;
    currentFile = newFile;
}

IfcParse::IfcFile *OpenBimRL::Engine::Utils::getCurrentFile() {
    return currentFile;
}

void OpenBimRL::Engine::Utils::setSilent(bool s) {
    silent = s;
}

[[maybe_unused]]
bool initIfc(JNA::String fileName) {

    if (!silent)
        Logger::SetOutput(&std::cout, &std::cout);

    if (!fileName)
    {
        std::cerr << "no file given" << std::endl;
        return false;
    }

    // Parse the IFC file provided in fileName
    auto file = new IfcParse::IfcFile(fileName);
    if (!file->good())
    {
        std::cerr << "Unable to parse .ifc file" << std::endl;
        return false;
    }

    setCurrentFile(file);

    return true;
}

bool OpenBimRL::Engine::Utils::isIFC2x3() {
    return getCurrentFile()->schema()->name() == "IFC2x3";
}

bool OpenBimRL::Engine::Utils::isIFC4()
{
    return getCurrentFile()->schema()->name() == "IFC4";
}
