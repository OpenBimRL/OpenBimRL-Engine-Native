FROM mcr.microsoft.com/devcontainers/cpp:1-debian-12

RUN apt update && apt install -y xfonts-scalable libocct-data-exchange-dev libocct-draw-dev libocct-foundation-dev libocct-modeling-algorithms-dev libocct-modeling-data-dev libocct-ocaf-dev libocct-visualization-dev libboost-all-dev libhdf5-dev libmpfr-dev libgmp3-dev

ENV CC=/usr/bin/clang
ENV CXX=/usr/bin/clang++