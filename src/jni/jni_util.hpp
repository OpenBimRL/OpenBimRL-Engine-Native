#ifndef OPENBIMRL_JNI_UTIL_H
#define OPENBIMRL_JNI_UTIL_H

#include <jni.h>

#include <stdexcept>
#include <string>
#include <vector>

namespace OpenBimRL::Native::Jni {

inline void throwNativeEngineException(JNIEnv* env, const char* message) {
    jclass exceptionClass =
        env->FindClass("de/rub/bi/inf/nativelib/NativeEngineException");
    if (exceptionClass == nullptr) {
        env->ExceptionClear();
        env->ThrowNew(env->FindClass("java/lang/RuntimeException"), message);
        return;
    }
    env->ThrowNew(exceptionClass, message);
}

inline void throwIfNoSession(JNIEnv* env) {
    throwNativeEngineException(env, "No active IFC session. Call initIfc first.");
}

inline std::string jstringToStd(JNIEnv* env, jstring value) {
    if (value == nullptr) return {};
    const char* chars = env->GetStringUTFChars(value, nullptr);
    if (chars == nullptr) return {};
    std::string result(chars);
    env->ReleaseStringUTFChars(value, chars);
    return result;
}

inline jstring toJstring(JNIEnv* env, const std::string& value) {
    return env->NewStringUTF(value.c_str());
}

inline jdoubleArray toDoubleArray(JNIEnv* env, const std::vector<double>& values) {
    jdoubleArray array = env->NewDoubleArray(static_cast<jsize>(values.size()));
    if (array == nullptr || values.empty()) return array;
    env->SetDoubleArrayRegion(array, 0, static_cast<jsize>(values.size()),
                              values.data());
    return array;
}

inline jdoubleArray toDoubleArray(JNIEnv* env, const double* values, jsize size) {
    jdoubleArray array = env->NewDoubleArray(size);
    if (array == nullptr || size == 0) return array;
    env->SetDoubleArrayRegion(array, 0, size, values);
    return array;
}

inline jlongArray toLongArray(JNIEnv* env, const std::vector<jlong>& values) {
    jlongArray array = env->NewLongArray(static_cast<jsize>(values.size()));
    if (array == nullptr || values.empty()) return array;
    env->SetLongArrayRegion(array, 0, static_cast<jsize>(values.size()),
                            values.data());
    return array;
}

inline jobjectArray toStringArray(JNIEnv* env, const std::vector<std::string>& values) {
    jclass stringClass = env->FindClass("java/lang/String");
    if (stringClass == nullptr) return nullptr;
    jobjectArray array = env->NewObjectArray(static_cast<jsize>(values.size()),
                                             stringClass, nullptr);
    if (array == nullptr) return nullptr;
    for (jsize i = 0; i < static_cast<jsize>(values.size()); ++i) {
        jstring item = toJstring(env, values[i]);
        env->SetObjectArrayElement(array, i, item);
        env->DeleteLocalRef(item);
    }
    return array;
}

template <typename Fn>
auto catchNative(JNIEnv* env, Fn fn) -> decltype(fn()) {
    using ReturnType = decltype(fn());
    try {
        return fn();
    } catch (const std::exception& ex) {
        throwNativeEngineException(env, ex.what());
    } catch (...) {
        throwNativeEngineException(env, "Unknown native error");
    }
    if constexpr (std::is_same_v<ReturnType, jboolean>) {
        return static_cast<jboolean>(JNI_FALSE);
    } else if constexpr (std::is_same_v<ReturnType, jlong>) {
        return static_cast<jlong>(0);
    } else if constexpr (std::is_same_v<ReturnType, jint>) {
        return static_cast<jint>(0);
    } else if constexpr (std::is_pointer_v<ReturnType>) {
        return static_cast<ReturnType>(nullptr);
    } else {
        return ReturnType{};
    }
}

}  // namespace OpenBimRL::Native::Jni

#endif  // OPENBIMRL_JNI_UTIL_H
