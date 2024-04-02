#include "library.h"

#include <iostream>
#include <filesystem>
#include <ifcparse/IfcFile.h>

int hello() {
    std::cout << "Looking for file in path: " << std::filesystem::current_path() << std::endl;

    // Parse the IFC file provided in fileName
    auto file = new IfcParse::IfcFile("../test/resources/test.ifc");
    if (!file->good())
    {
        std::cerr << "Unable to parse .ifc file" << std::endl;
        return 1;
    }

    return 0;
}
