#include "openbimrl/ffi/rule_context.hpp"

namespace OpenBimRL::Native::Ffi {

RuleContext& RuleContext::current() {
    thread_local RuleContext ctx;
    return ctx;
}

}  // namespace OpenBimRL::Native::Ffi
