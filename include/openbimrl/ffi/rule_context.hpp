#ifndef OPENBIMRL_NATIVE_FFI_RULE_CONTEXT_H
#define OPENBIMRL_NATIVE_FFI_RULE_CONTEXT_H

#include <cstddef>
#include <cstdint>
#include <functional>

namespace OpenBimRL::Native::Ffi {

using GetInputPointerFn = void* (*)(uint32_t);
using GetInputDoubleFn = double (*)(uint32_t);
using GetInputIntegerFn = uint32_t (*)(uint32_t);
using GetInputStringFn = const char* (*)(uint32_t);
using SetOutputPointerFn = void (*)(uint32_t, void*);
using SetOutputDoubleFn = void (*)(uint32_t, double);
using SetOutputIntegerFn = void (*)(uint32_t, uint32_t);
using SetOutputStringFn = void (*)(uint32_t, const char*);
using SetOutputArrayFn = void* (*)(uint32_t, std::size_t);

struct RuleContext {
    std::function<void*(uint32_t)> getInputPointer;
    std::function<double(uint32_t)> getInputDouble;
    std::function<uint32_t(uint32_t)> getInputInt;
    std::function<const char*(uint32_t)> getInputString;

    std::function<void(uint32_t, void*)> setOutputPointer;
    std::function<void(uint32_t, double)> setOutputDouble;
    std::function<void(uint32_t, uint32_t)> setOutputInt;
    std::function<void(uint32_t, const char*)> setOutputString;
    std::function<void*(uint32_t, std::size_t)> setOutputArray;

    static RuleContext& current();
};

}  // namespace OpenBimRL::Native::Ffi

#endif  // OPENBIMRL_NATIVE_FFI_RULE_CONTEXT_H
