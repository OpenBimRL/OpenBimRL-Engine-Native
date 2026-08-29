#include <jni.h>

#include "openbimrl/build_config.hpp"

// GPU offload flags (see CMakeLists.txt target_compile_definitions).
#ifndef OPENBIMRL_COMPILED_ROCM_OFFLOAD
#define OPENBIMRL_COMPILED_ROCM_OFFLOAD 0
#endif
#ifndef OPENBIMRL_COMPILED_CUDA_OFFLOAD
#define OPENBIMRL_COMPILED_CUDA_OFFLOAD 0
#endif
#ifndef OPENBIMRL_COMPILED_GPU_OFFLOAD_ARCH
#define OPENBIMRL_COMPILED_GPU_OFFLOAD_ARCH ""
#endif

namespace {

constexpr bool gpuOffloadCompiled() {
#if OPENBIMRL_COMPILED_ROCM_OFFLOAD || OPENBIMRL_COMPILED_CUDA_OFFLOAD
    return true;
#else
    return false;
#endif
}

const char* compiledGpuOffloadArch() {
#if OPENBIMRL_COMPILED_ROCM_OFFLOAD || OPENBIMRL_COMPILED_CUDA_OFFLOAD
    return OPENBIMRL_COMPILED_GPU_OFFLOAD_ARCH;
#else
    return "";
#endif
}

jstring toJstring(JNIEnv* env, const char* text) {
    if (text == nullptr) {
        return env->NewStringUTF("");
    }
    return env->NewStringUTF(text);
}

jstring optionalArch(JNIEnv* env) {
    const char* arch = compiledGpuOffloadArch();
    if (arch == nullptr || arch[0] == '\0') {
        return nullptr;
    }
    return env->NewStringUTF(arch);
}

}  // namespace

extern "C" JNIEXPORT jstring JNICALL
Java_de_rub_bi_inf_nativelib_NativeEngine_nativeLibraryVersion(JNIEnv* env, jclass) {
    return toJstring(env, OPENBIMRL_NATIVE_VERSION);
}

extern "C" JNIEXPORT jstring JNICALL
Java_de_rub_bi_inf_nativelib_NativeEngine_nativeBuildDate(JNIEnv* env, jclass) {
    return toJstring(env, OPENBIMRL_NATIVE_BUILD_DATE);
}

extern "C" JNIEXPORT jstring JNICALL
Java_de_rub_bi_inf_nativelib_NativeEngine_nativeBuildCompiler(JNIEnv* env, jclass) {
    return toJstring(env, OPENBIMRL_NATIVE_BUILD_COMPILER);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_de_rub_bi_inf_nativelib_NativeEngine_isGpuOffloadEnabled(JNIEnv*, jclass) {
    return gpuOffloadCompiled() ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT jstring JNICALL
Java_de_rub_bi_inf_nativelib_NativeEngine_gpuOffloadArch(JNIEnv* env, jclass) {
    return optionalArch(env);
}
