# syntax=docker/dockerfile:1
#
# Native DevContainer / local image: system LLVM clang + libomp (no ROCm / CUDA).
# Point .devcontainer/devcontainer.json at this file (or switch to rocm/nvcc).
#
#   docker build -f llvm.dockerfile -t openbimrl-native:llvm .
#
# Supported platform: linux/amd64 only.

ARG BAZELISK_VERSION=1.29.0

FROM ubuntu:24.04

ARG BAZELISK_VERSION

ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC
ENV CC=/usr/bin/clang
ENV CXX=/usr/bin/clang++
ENV OPENBIMRL_USE_PREBUILT_IFCOPENSHELL=ON
ENV OPENBIMRL_IFCOPENSHELL_PREFIX=/opt/ifcopenshell

RUN apt-get update && apt-get install -y --no-install-recommends \
        ca-certificates curl git \
        clang libomp-dev libomp5 \
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
