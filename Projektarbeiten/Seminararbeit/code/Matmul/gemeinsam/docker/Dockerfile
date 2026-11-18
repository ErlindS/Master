# Werkzeugkette für den Benchmark: LLVM + AdaptiveCpp (optional mit CUDA)
#
# Enthält nur Compiler und Laufzeit, keinen Quellcode. Der Code wird beim
# Start als Volume eingebunden (siehe setup.sh / setup.ps1), damit Änderungen
# am Code keinen Neubau des Images erfordern.
#
#   CPU:  docker build -t sycl-matmul:cpu docker/
#   GPU:  docker build -t sycl-matmul:cuda \
#           --build-arg BASE_IMAGE=nvidia/cuda:12.9.1-devel-ubuntu24.04 \
#           --build-arg WITH_CUDA=ON docker/

ARG BASE_IMAGE=ubuntu:24.04
FROM ${BASE_IMAGE}

# LLVM 20 ist die neueste von AdaptiveCpp 25.10 unterstützte Version und
# kennt bereits die Blackwell-Architektur (sm_120) der RTX-50-Serie.
ARG LLVM_VERSION=20
ARG ACPP_VERSION=v25.10.0
ARG WITH_CUDA=OFF

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential cmake git wget curl ca-certificates gnupg lsb-release \
        software-properties-common python3 \
        libboost-context-dev libboost-fiber-dev libnuma-dev \
    && rm -rf /var/lib/apt/lists/*

RUN wget -qO /tmp/llvm.sh https://apt.llvm.org/llvm.sh && chmod +x /tmp/llvm.sh \
    && /tmp/llvm.sh ${LLVM_VERSION} \
    && apt-get install -y --no-install-recommends \
        llvm-${LLVM_VERSION}-dev libclang-${LLVM_VERSION}-dev \
        clang-tools-${LLVM_VERSION} libomp-${LLVM_VERSION}-dev \
    && rm -rf /var/lib/apt/lists/* /tmp/llvm.sh

RUN git clone --depth 1 --branch ${ACPP_VERSION} \
        https://github.com/AdaptiveCpp/AdaptiveCpp /tmp/acpp \
    && cmake -S /tmp/acpp -B /tmp/acpp/build \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/opt/acpp \
        -DCMAKE_C_COMPILER=/usr/lib/llvm-${LLVM_VERSION}/bin/clang \
        -DCMAKE_CXX_COMPILER=/usr/lib/llvm-${LLVM_VERSION}/bin/clang++ \
        -DLLVM_DIR=/usr/lib/llvm-${LLVM_VERSION}/cmake \
        -DCLANG_EXECUTABLE_PATH=/usr/lib/llvm-${LLVM_VERSION}/bin/clang++ \
        -DWITH_CUDA_BACKEND=${WITH_CUDA} \
        -DCUDA_TOOLKIT_ROOT_DIR=/usr/local/cuda \
        -DWITH_ROCM_BACKEND=OFF \
        -DWITH_OPENCL_BACKEND=OFF \
        -DWITH_LEVEL_ZERO_BACKEND=OFF \
    && cmake --build /tmp/acpp/build -j"$(nproc)" \
    && cmake --install /tmp/acpp/build \
    && rm -rf /tmp/acpp

# HOME=/tmp: der Container läuft unter der Benutzer-ID des Hosts, AdaptiveCpp
# legt seinen Kernel-Cache unter $HOME ab.
ENV PATH=/opt/acpp/bin:${PATH} \
    HOME=/tmp

WORKDIR /work
