#include "openbimrl_c_api.h"

#include "openbimrl/model/session.hpp"

using OpenBimRL::Native::Model::ActiveSession;
using OpenBimRL::Native::Model::IfcSession;

extern "C" bool initIfc(const char* path) {
    auto session = IfcSession::open(path ? path : "");
    if (!session) return false;
    ActiveSession::set(std::move(session));
    return true;
}
