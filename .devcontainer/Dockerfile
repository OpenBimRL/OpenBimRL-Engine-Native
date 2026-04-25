FROM rocm/dev-ubuntu-22.04:7.1.1 AS rocm-llvm
FROM mcr.microsoft.com/devcontainers/cpp:1-debian-12

USER root

COPY --from=rocm-llvm /opt/rocm-7.1.1/ /opt/rocm/

RUN apt-get update && apt-get install -y --no-install-recommends \
    xfonts-scalable libocct-data-exchange-dev libocct-draw-dev libocct-foundation-dev \
    libocct-modeling-algorithms-dev libocct-modeling-data-dev libocct-ocaf-dev \
    libocct-visualization-dev libboost-all-dev libhdf5-dev libmpfr-dev libgmp3-dev \
    && ln -sf /opt/rocm/llvm/bin/clang /usr/local/bin/clang \
    && ln -sf /opt/rocm/llvm/bin/clang++ /usr/local/bin/clang++ \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

ENV ROCM_PATH=/opt/rocm
ENV PATH=${ROCM_PATH}/llvm/bin:${PATH}
ENV CC=${ROCM_PATH}/llvm/bin/clang
ENV CXX=${ROCM_PATH}/llvm/bin/clang++