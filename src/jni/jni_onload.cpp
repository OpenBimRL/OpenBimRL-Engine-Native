#include "jni_bind.h"

#include <memory>

namespace {
std::unique_ptr<jni::JvmRef<jni::kDefaultJvm>> g_jvm;
}  // namespace

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* /*reserved*/) {
    g_jvm = std::make_unique<jni::JvmRef<jni::kDefaultJvm>>(vm);
    return JNI_VERSION_1_6;
}
