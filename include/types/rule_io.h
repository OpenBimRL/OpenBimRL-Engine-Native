#ifndef OPENBIMRL_NATIVE_RULE_IO_H
#define OPENBIMRL_NATIVE_RULE_IO_H

#include "types.h"

namespace OpenBimRL::Engine::Types::Functions {
// input function
    typedef JNA::Pointer (*get_input_pointer)(uint32_t);

    typedef double (*get_input_double)(uint32_t);

    typedef uint32_t (*get_input_integer)(uint32_t);

    typedef JNA::String (*get_input_string)(uint32_t);

// output functions
    typedef void (*set_output_pointer)(uint32_t, JNA::Pointer);

    typedef void (*set_output_double)(uint32_t, double);

    typedef void (*set_output_integer)(uint32_t, uint32_t);

    typedef void (*set_output_string)(uint32_t, JNA::String);

    typedef JNA::Pointer (*set_output_array)(uint32_t, uint32_t);

}

namespace OpenBimRL::Engine::Functions {
    // input function
    using namespace OpenBimRL::Engine::Types;
    extern std::function<JNA::Pointer(uint32_t)> getInputPointer;
    extern std::function<double(uint32_t)> getInputDouble;
    extern std::function<uint32_t(uint32_t)> getInputInt;
    extern std::function<JNA::String(uint32_t)> getInputString;

// output functions
    extern std::function<void(uint32_t, JNA::Pointer)> setOutputPointer;
    extern std::function<void(uint32_t, double)> setOutputDouble;
    extern std::function<void(uint32_t, uint32_t)> setOutputInt;
    extern std::function<void(uint32_t, JNA::String)> setOutputString;
    extern std::function<JNA::Pointer(uint32_t, uint32_t)> setOutputArray;
}
#endif // OPENBIMRL_NATIVE_RULE_IO_H
