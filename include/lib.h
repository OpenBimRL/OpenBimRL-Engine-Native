#ifndef OPENBIMRL_NATIVE_LIB_H
#define OPENBIMRL_NATIVE_LIB_H

#include "types.h"


extern "C" {
using namespace OpenBimRL::Engine::Types;

[[maybe_unused]] std::size_t request_ifc_object_json_size(JNA::Pointer);
[[maybe_unused]] void ifc_object_to_json(JNA::Pointer, JNA::Pointer);

/// @brief initialize library and load file into memory
/// @param path to file
/// @return true if the library was successfully loaded
[[maybe_unused]] bool initIfc(JNA::String);

using namespace Functions;
[[maybe_unused]] void init_function(get_input_pointer,
                   get_input_double,
                   get_input_integer,
                   get_input_string,
                   set_output_pointer,
                   set_output_double,
                   set_output_integer,
                   set_output_string,
                   set_output_array);

}

#endif //OPENBIMRL_NATIVE_LIB_H