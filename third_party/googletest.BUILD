# Minimal gtest/gmock for Bazel 9 (loads rules_cc; no Abseil).
# Mirrors the upstream googletest BUILD layout without bare cc_* builtins.

load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "gtest",
    srcs = glob(
        [
            "googletest/src/*.cc",
            "googletest/src/*.h",
            "googletest/include/gtest/**/*.h",
            "googlemock/src/*.cc",
            "googlemock/include/gmock/**/*.h",
        ],
        exclude = [
            "googletest/src/gtest-all.cc",
            "googletest/src/gtest_main.cc",
            "googlemock/src/gmock-all.cc",
            "googlemock/src/gmock_main.cc",
        ],
    ),
    hdrs = glob([
        "googletest/include/gtest/*.h",
        "googletest/include/gtest/internal/*.h",
        "googletest/include/gtest/internal/custom/*.h",
        "googlemock/include/gmock/*.h",
        "googlemock/include/gmock/internal/*.h",
        "googlemock/include/gmock/internal/custom/*.h",
    ]),
    includes = [
        "googlemock",
        "googlemock/include",
        "googletest",
        "googletest/include",
    ],
    linkopts = select({
        "@platforms//os:windows": [],
        "//conditions:default": ["-pthread"],
    }),
)

cc_library(
    name = "gtest_main",
    srcs = ["googletest/src/gtest_main.cc"],
    deps = [":gtest"],
)
