#include "openbimrl_c_api.h"

#include <iostream>

#include "openbimrl/ffi/rule_context.hpp"
#include "openbimrl/model/session.hpp"

using OpenBimRL::Native::Ffi::RuleContext;
using OpenBimRL::Native::Model::ActiveSession;
using OpenBimRL::Native::Model::IfcSession;

extern "C" void init_function(openbimrl_get_input_pointer gip,
                              openbimrl_get_input_double gid,
                              openbimrl_get_input_integer gii,
                              openbimrl_get_input_string gis,
                              openbimrl_set_output_pointer sop,
                              openbimrl_set_output_double sod,
                              openbimrl_set_output_integer soi,
                              openbimrl_set_output_string sos,
                              openbimrl_set_output_array soa) {
    auto& ctx = RuleContext::current();
    ctx.getInputPointer = gip;
    ctx.getInputDouble = gid;
    ctx.getInputInt = gii;
    ctx.getInputString = gis;
    ctx.setOutputPointer = sop;
    ctx.setOutputDouble = sod;
    ctx.setOutputInt = soi;
    ctx.setOutputString = sos;
    ctx.setOutputArray = soa;
}

extern "C" bool initIfc(const char* path) {
    auto session = IfcSession::open(path ? path : "");
    if (!session) return false;
    ActiveSession::set(std::move(session));
    return true;
}
