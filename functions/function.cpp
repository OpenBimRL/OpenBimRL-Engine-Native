#include "lib.h"

namespace OpenBimRL::Engine::Functions
{

    std::function<JNA::Pointer(uint32_t)> getInputPointer;
    std::function<double(uint32_t)> getInputDouble;
    std::function<uint32_t(uint32_t)> getInputInt;
    std::function<JNA::String(uint32_t)> getInputString;

// output functions
    std::function<void(uint32_t, JNA::Pointer)> setOutputPointer;
    std::function<void(uint32_t, double)> setOutputDouble;
    std::function<void(uint32_t, uint32_t)> setOutputInt;
    std::function<void(uint32_t, JNA::String)> setOutputString;
    std::function<JNA::Pointer(uint32_t, uint32_t)> setOutputArray;
}

void init_function(get_input_pointer gip,
                   get_input_double gid,
                   get_input_integer gii,
                   get_input_string gis,
                   set_output_pointer sop,
                   set_output_double sod,
                   set_output_integer soi,
                   set_output_string sos,
                   set_output_array soa)
{

    OpenBimRL::Engine::Functions::getInputPointer = std::function(gip);
    OpenBimRL::Engine::Functions::getInputDouble = std::function(gid);
    OpenBimRL::Engine::Functions::getInputInt = std::function(gii);
    OpenBimRL::Engine::Functions::getInputString = std::function(gis);

    OpenBimRL::Engine::Functions::setOutputPointer = std::function(sop);
    OpenBimRL::Engine::Functions::setOutputDouble = std::function(sod);
    OpenBimRL::Engine::Functions::setOutputInt = std::function(soi);
    OpenBimRL::Engine::Functions::setOutputString = std::function(sos);
    OpenBimRL::Engine::Functions::setOutputArray = std::function(soa);
}