#include <jni.h>

#include <cstring>
#include <string>
#include <vector>

#include "jni_util.hpp"
#include "openbimrl/geometry/service.hpp"
#include "openbimrl/geometry/types.hpp"
#include "openbimrl/model/entity.hpp"
#include "openbimrl/model/session.hpp"
#include "openbimrl/pathfinding/edge_costs.hpp"
#include "openbimrl/pathfinding/types.hpp"
#include "openbimrl/properties/data.hpp"

using OpenBimRL::Native::Geometry::boundsOf;
using OpenBimRL::Native::Geometry::boundsToXzyArray;
using OpenBimRL::Native::Geometry::buildingBounds;
using OpenBimRL::Native::Geometry::ElementFrame;
using OpenBimRL::Native::Geometry::extractFrame;
using OpenBimRL::Native::Geometry::extractSegmentsXY;
using OpenBimRL::Native::Geometry::footprintPolygonXY;
using OpenBimRL::Native::Jni::catchNative;
using OpenBimRL::Native::Jni::jstringToStd;
using OpenBimRL::Native::Jni::throwIfNoSession;
using OpenBimRL::Native::Jni::toDoubleArray;
using OpenBimRL::Native::Jni::toJstring;
using OpenBimRL::Native::Jni::toLongArray;
using OpenBimRL::Native::Jni::toStringArray;
using OpenBimRL::Native::Model::ActiveSession;
using OpenBimRL::Native::Model::IfcEntityRef;
using OpenBimRL::Native::Model::IfcSession;
using OpenBimRL::Native::Pathfinding::computeEdgeCosts;
using OpenBimRL::Native::Pathfinding::Edge;
using OpenBimRL::Native::Pathfinding::Point2;
using OpenBimRL::Native::Pathfinding::Segment2;
using OpenBimRL::Native::Properties::snapshot;

namespace {

IfcEntityRef entityFromHandle(jlong handle) {
    return IfcEntityRef{reinterpret_cast<void*>(handle)};
}

jlong handleFromEntity(IfcEntityRef entity) {
    return reinterpret_cast<jlong>(entity.ptr);
}

}  // namespace

extern "C" JNIEXPORT jboolean JNICALL
Java_de_rub_bi_inf_nativelib_NativeEngine_initIfc(JNIEnv* env, jclass,
                                                  jstring path) {
    return catchNative(env, [&]() -> jboolean {
        const std::string filePath = jstringToStd(env, path);
        auto session = IfcSession::open(filePath);
        if (!session) return JNI_FALSE;
        ActiveSession::set(std::move(session));
        return JNI_TRUE;
    });
}

extern "C" JNIEXPORT jlongArray JNICALL
Java_de_rub_bi_inf_nativelib_NativeEngine_filterByElement(JNIEnv* env, jclass,
                                                          jstring ifcType) {
    return catchNative(env, [&]() -> jlongArray {
        auto* session = ActiveSession::get();
        if (session == nullptr) {
            throwIfNoSession(env);
            return nullptr;
        }
        const auto instances = session->instancesOf(jstringToStd(env, ifcType));
        std::vector<jlong> handles;
        handles.reserve(instances.size());
        for (const auto& instance : instances) {
            handles.push_back(handleFromEntity(instance));
        }
        return toLongArray(env, handles);
    });
}

extern "C" JNIEXPORT jlong JNICALL
Java_de_rub_bi_inf_nativelib_NativeEngine_filterByGuid(JNIEnv* env, jclass,
                                                       jstring guid) {
    return catchNative(env, [&]() -> jlong {
        auto* session = ActiveSession::get();
        if (session == nullptr) {
            throwIfNoSession(env);
            return 0L;
        }
        return handleFromEntity(
            session->instanceByGuid(jstringToStd(env, guid)));
    });
}

extern "C" JNIEXPORT jdoubleArray JNICALL
Java_de_rub_bi_inf_nativelib_NativeEngine_getBoundingBox(JNIEnv* env, jclass,
                                                         jlong handle) {
    return catchNative(env, [&]() -> jdoubleArray {
        auto* session = ActiveSession::get();
        if (session == nullptr) {
            throwIfNoSession(env);
            return nullptr;
        }
        const auto entity = entityFromHandle(handle);
        if (!entity) return nullptr;
        const auto bounds = boundsOf(*session, entity);
        if (!bounds) return nullptr;
        double values[6];
        boundsToXzyArray(*bounds, values);
        return toDoubleArray(env, values, 6);
    });
}

extern "C" JNIEXPORT jdoubleArray JNICALL
Java_de_rub_bi_inf_nativelib_NativeEngine_getElementFrame(JNIEnv* env, jclass,
                                                          jlong handle) {
    return catchNative(env, [&]() -> jdoubleArray {
        const auto entity = entityFromHandle(handle);
        if (!entity) return nullptr;
        ElementFrame frame{};
        if (!extractFrame(entity, frame)) return nullptr;
        const double values[9] = {
            frame.point[0], frame.point[1], frame.point[2],
            frame.axisX[0], frame.axisX[1], frame.axisX[2],
            frame.axisZ[0], frame.axisZ[1], frame.axisZ[2],
        };
        return toDoubleArray(env, values, 9);
    });
}

extern "C" JNIEXPORT jint JNICALL
Java_de_rub_bi_inf_nativelib_NativeEngine_getElementFrameSource(JNIEnv* env,
                                                                jclass,
                                                                jlong handle) {
    return catchNative(env, [&]() -> jint {
        const auto entity = entityFromHandle(handle);
        if (!entity) return 0;
        ElementFrame frame{};
        if (!extractFrame(entity, frame)) return 0;
        return static_cast<jint>(frame.source);
    });
}

extern "C" JNIEXPORT jdoubleArray JNICALL
Java_de_rub_bi_inf_nativelib_NativeEngine_calculatingBuildingBounds(JNIEnv* env,
                                                                    jclass) {
    return catchNative(env, [&]() -> jdoubleArray {
        auto* session = ActiveSession::get();
        if (session == nullptr) {
            throwIfNoSession(env);
            return nullptr;
        }
        const auto bounds = buildingBounds(*session);
        if (!bounds) return nullptr;
        double values[6];
        boundsToXzyArray(*bounds, values);
        return toDoubleArray(env, values, 6);
    });
}

extern "C" JNIEXPORT jdoubleArray JNICALL
Java_de_rub_bi_inf_nativelib_NativeEngine_footprintPolygonXY(JNIEnv* env, jclass,
                                                             jlong handle) {
    return catchNative(env, [&]() -> jdoubleArray {
        auto* session = ActiveSession::get();
        if (session == nullptr) {
            throwIfNoSession(env);
            return nullptr;
        }
        const auto entity = entityFromHandle(handle);
        if (!entity) return toDoubleArray(env, std::vector<double>{});
        return toDoubleArray(env, footprintPolygonXY(*session, entity));
    });
}

extern "C" JNIEXPORT jstring JNICALL
Java_de_rub_bi_inf_nativelib_NativeEngine_ifcGuid(JNIEnv* env, jclass,
                                                   jlong handle) {
    return catchNative(env, [&]() -> jstring {
        auto* session = ActiveSession::get();
        if (session == nullptr) {
            throwIfNoSession(env);
            return nullptr;
        }
        const auto data = snapshot(*session, entityFromHandle(handle));
        return toJstring(env, data.GUID);
    });
}

extern "C" JNIEXPORT jstring JNICALL
Java_de_rub_bi_inf_nativelib_NativeEngine_ifcClass(JNIEnv* env, jclass,
                                                   jlong handle) {
    return catchNative(env, [&]() -> jstring {
        auto* session = ActiveSession::get();
        if (session == nullptr) {
            throwIfNoSession(env);
            return nullptr;
        }
        const auto data = snapshot(*session, entityFromHandle(handle));
        return toJstring(env, data.ifcClass);
    });
}

extern "C" JNIEXPORT jobjectArray JNICALL
Java_de_rub_bi_inf_nativelib_NativeEngine_ifcPropertiesFlat(JNIEnv* env, jclass,
                                                            jlong handle) {
    return catchNative(env, [&]() -> jobjectArray {
        auto* session = ActiveSession::get();
        if (session == nullptr) {
            throwIfNoSession(env);
            return nullptr;
        }
        const auto data = snapshot(*session, entityFromHandle(handle));
        std::vector<std::string> flat;
        for (const auto& [psetName, properties] : data.propertySets) {
            for (const auto& [propertyName, propertyValue] : properties) {
                flat.push_back(psetName);
                flat.push_back(propertyName);
                flat.push_back(propertyValue);
            }
        }
        return toStringArray(env, flat);
    });
}

extern "C" JNIEXPORT jobjectArray JNICALL
Java_de_rub_bi_inf_nativelib_NativeEngine_ifcQuantityKeys(JNIEnv* env, jclass,
                                                          jlong handle) {
    return catchNative(env, [&]() -> jobjectArray {
        auto* session = ActiveSession::get();
        if (session == nullptr) {
            throwIfNoSession(env);
            return nullptr;
        }
        const auto data = snapshot(*session, entityFromHandle(handle));
        std::vector<std::string> keys;
        for (const auto& [qsetName, quantities] : data.quantitySets) {
            for (const auto& [quantityName, quantityValue] : quantities) {
                (void)quantityValue;
                keys.push_back(qsetName + "\t" + quantityName);
            }
        }
        return toStringArray(env, keys);
    });
}

extern "C" JNIEXPORT jdoubleArray JNICALL
Java_de_rub_bi_inf_nativelib_NativeEngine_ifcQuantityValues(JNIEnv* env, jclass,
                                                            jlong handle) {
    return catchNative(env, [&]() -> jdoubleArray {
        auto* session = ActiveSession::get();
        if (session == nullptr) {
            throwIfNoSession(env);
            return nullptr;
        }
        const auto data = snapshot(*session, entityFromHandle(handle));
        std::vector<double> values;
        for (const auto& [qsetName, quantities] : data.quantitySets) {
            (void)qsetName;
            for (const auto& [quantityName, quantityValue] : quantities) {
                (void)quantityName;
                values.push_back(quantityValue);
            }
        }
        return toDoubleArray(env, values);
    });
}

extern "C" JNIEXPORT jdoubleArray JNICALL
Java_de_rub_bi_inf_nativelib_NativeEngine_calculatePathEdgeCosts(
    JNIEnv* env, jclass, jdoubleArray pointsXY, jint pointCount,
    jintArray edgePointIndices, jint edgeCount, jlongArray passageHandles,
    jlongArray obstacleHandles) {
    return catchNative(env, [&]() -> jdoubleArray {
        if (edgeCount <= 0) return toDoubleArray(env, std::vector<double>{});

        jdouble* pointsRaw = env->GetDoubleArrayElements(pointsXY, nullptr);
        jint* edgeRaw = env->GetIntArrayElements(edgePointIndices, nullptr);
        if (pointsRaw == nullptr || edgeRaw == nullptr) {
            if (pointsRaw != nullptr) {
                env->ReleaseDoubleArrayElements(pointsXY, pointsRaw, JNI_ABORT);
            }
            if (edgeRaw != nullptr) {
                env->ReleaseIntArrayElements(edgePointIndices, edgeRaw, JNI_ABORT);
            }
            return nullptr;
        }

        auto* session = ActiveSession::get();
        std::vector<Segment2> passages;
        std::vector<Segment2> obstacles;
        if (session != nullptr) {
            if (passageHandles != nullptr) {
                const jsize passageCount = env->GetArrayLength(passageHandles);
                jlong* passageRaw =
                    env->GetLongArrayElements(passageHandles, nullptr);
                if (passageRaw != nullptr) {
                    for (jsize i = 0; i < passageCount; ++i) {
                        const auto entity = entityFromHandle(passageRaw[i]);
                        if (!entity) continue;
                        const auto local = extractSegmentsXY(*session, entity);
                        passages.insert(passages.end(), local.begin(), local.end());
                    }
                    env->ReleaseLongArrayElements(passageHandles, passageRaw,
                                                  JNI_ABORT);
                }
            }
            if (obstacleHandles != nullptr) {
                const jsize obstacleCount = env->GetArrayLength(obstacleHandles);
                jlong* obstacleRaw =
                    env->GetLongArrayElements(obstacleHandles, nullptr);
                if (obstacleRaw != nullptr) {
                    for (jsize i = 0; i < obstacleCount; ++i) {
                        const auto entity = entityFromHandle(obstacleRaw[i]);
                        if (!entity) continue;
                        const auto local = extractSegmentsXY(*session, entity);
                        obstacles.insert(obstacles.end(), local.begin(),
                                         local.end());
                    }
                    env->ReleaseLongArrayElements(obstacleHandles, obstacleRaw,
                                                  JNI_ABORT);
                }
            }
        }

        std::vector<Point2> points(static_cast<std::size_t>(pointCount));
        for (jint i = 0; i < pointCount; ++i) {
            points[static_cast<std::size_t>(i)] = {
                pointsRaw[i * 2], pointsRaw[i * 2 + 1]};
        }
        std::vector<Edge> edges(static_cast<std::size_t>(edgeCount));
        for (jint i = 0; i < edgeCount; ++i) {
            edges[static_cast<std::size_t>(i)] = {
                static_cast<std::uint32_t>(edgeRaw[i * 2]),
                static_cast<std::uint32_t>(edgeRaw[i * 2 + 1])};
        }

        std::vector<double> costs(static_cast<std::size_t>(edgeCount));
        computeEdgeCosts(points, edges, passages, obstacles, costs);

        env->ReleaseDoubleArrayElements(pointsXY, pointsRaw, JNI_ABORT);
        env->ReleaseIntArrayElements(edgePointIndices, edgeRaw, JNI_ABORT);
        return toDoubleArray(env, costs);
    });
}
