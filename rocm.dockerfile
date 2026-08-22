# syntax=docker/dockerfile:1
#
# Native DevContainer / local image: ROCm LLVM for --config=rocm_offload.
# Host CC/CXX point at ROCm clang (no separate distro clang required).
# Point .devcontainer/devcontainer.json at this file (or switch to llvm/nvcc).
#
#   docker build -f rocm.dockerfile -t openbimrl-native:rocm .
#
# Supported platform: linux/amd64 only.

ARG BAZELISK_VERSION=1.29.0

FROM rocm/dev-ubuntu-24.04:7.1.1

ARG BAZELISK_VERSION

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC
ENV ROCM_PATH=/opt/rocm
ENV PATH=${ROCM_PATH}/llvm/bin:${PATH}
ENV CC=/opt/rocm/llvm/bin/clang
ENV CXX=/opt/rocm/llvm/bin/clang++
ENV OPENBIMRL_USE_PREBUILT_IFCOPENSHELL=ON
ENV OPENBIMRL_IFCOPENSHELL_PREFIX=/opt/ifcopenshell

RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates curl git \
        cmake ninja-build make \
        libeigen3-dev \
        xfonts-scalable \
        libboost-all-dev \
        libocct-foundation-dev libocct-modeling-algorithms-dev libocct-modeling-data-dev \
        libocct-ocaf-dev libocct-visualization-dev libocct-data-exchange-dev libocct-draw-dev \
        libhdf5-dev libgmp-dev libmpfr-dev libxml2-dev \
        libgl1-mesa-dev libx11-dev libxext-dev libxi-dev libxmu-dev libxt-dev \
        libtbb-dev tcl-dev tk-dev occt-misc \
    && curl -fsSL -o /usr/local/bin/bazelisk \
         "https://github.com/bazelbuild/bazelisk/releases/download/v${BAZELISK_VERSION}/bazelisk-linux-amd64" \
    && chmod +x /usr/local/bin/bazelisk \
    && ln -sf /usr/local/bin/bazelisk /usr/local/bin/bazel \
    && rm -rf /var/lib/apt/lists/*
