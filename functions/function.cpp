#include "lib.h"

namespace OpenBimRL::Engine::Functions
{

    get_input_pointer getInputPointer;
    get_input_double getInputDouble;
    get_input_integer getInputInt;
    get_input_string getInputString;

    set_output_pointer setOutputPointer;
    set_output_double setOutputDouble;
    set_output_integer setOutputInt;
    set_output_string setOutputString;
    set_output_array setOutputArray;
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

    OpenBimRL::Engine::Functions::getInputPointer = gip;
    OpenBimRL::Engine::Functions::getInputDouble = gid;
    OpenBimRL::Engine::Functions::getInputInt = gii;
    OpenBimRL::Engine::Functions::getInputString = gis;

    OpenBimRL::Engine::Functions::setOutputPointer = sop;
    OpenBimRL::Engine::Functions::setOutputDouble = sod;
    OpenBimRL::Engine::Functions::setOutputInt = soi;
    OpenBimRL::Engine::Functions::setOutputString = sos;
    OpenBimRL::Engine::Functions::setOutputArray = soa;
}