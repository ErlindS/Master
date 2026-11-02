# Versuchsaufbau – Build
#
#   make dpcpp      SYCL mit DPC++ (CPU über OpenCL + NVIDIA über CUDA in EINER Datei)
#   make acpp       SYCL mit AdaptiveCpp (generischer Single-Pass-Compiler)
#   make cuda       native CUDA-Varianten + cuBLAS
#   make cpu        OpenBLAS-sgemm + OpenMP-Triade
#   make            dpcpp cuda cpu
#
# Compiler anpassen, z. B.:
#   make dpcpp DPCPP_CXX=$HOME/sycl_workspace/llvm/build/bin/clang++
#   make dpcpp DPCPP_EXTRA="-Xsycl-target-backend=nvptx64-nvidia-cuda --cuda-gpu-arch=sm_120"

DPCPP_CXX     ?= icpx
DPCPP_TARGETS ?= nvptx64-nvidia-cuda,spir64
DPCPP_EXTRA   ?=
ACPP          ?= acpp
ACPP_TARGETS  ?= generic
NVCC          ?= nvcc
NVCC_ARCH     ?= native
CXX           ?= g++

BIN := bin

DPCPP_FLAGS := -fsycl -O3 -std=c++17 -fsycl-targets=$(DPCPP_TARGETS) $(DPCPP_EXTRA) -DIMPL_NAME=\"dpcpp\"
ACPP_FLAGS  := -O3 -std=c++17 --acpp-targets=$(ACPP_TARGETS) -DIMPL_NAME=\"acpp\"
NVCC_FLAGS  := -O3 -std=c++17 -arch=$(NVCC_ARCH)
CPU_FLAGS   := -O3 -std=c++17 -march=native

all: dpcpp cuda cpu

dpcpp: $(BIN)/sycl_gemm_dpcpp $(BIN)/sycl_triad_dpcpp
acpp:  $(BIN)/sycl_gemm_acpp $(BIN)/sycl_triad_acpp
cuda:  $(BIN)/cuda_gemm $(BIN)/cuda_triad
cpu:   $(BIN)/cpu_blas $(BIN)/cpu_triad

$(BIN):
	mkdir -p $(BIN)

$(BIN)/sycl_%_dpcpp: sycl_%.cpp common.hpp | $(BIN)
	$(DPCPP_CXX) $(DPCPP_FLAGS) -o $@ $<

$(BIN)/sycl_%_acpp: sycl_%.cpp common.hpp | $(BIN)
	$(ACPP) $(ACPP_FLAGS) -o $@ $<

$(BIN)/cuda_gemm: cuda_gemm.cu common.hpp | $(BIN)
	$(NVCC) $(NVCC_FLAGS) -o $@ $< -lcublas

$(BIN)/cuda_triad: cuda_triad.cu common.hpp | $(BIN)
	$(NVCC) $(NVCC_FLAGS) -o $@ $<

$(BIN)/cpu_blas: cpu_blas.cpp common.hpp | $(BIN)
	$(CXX) $(CPU_FLAGS) -o $@ $< -lopenblas

$(BIN)/cpu_triad: cpu_triad.cpp common.hpp | $(BIN)
	$(CXX) $(CPU_FLAGS) -fopenmp -o $@ $<

clean:
	rm -rf $(BIN)

.PHONY: all dpcpp acpp cuda cpu clean
