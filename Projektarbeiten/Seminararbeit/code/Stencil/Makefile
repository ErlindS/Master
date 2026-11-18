# 2D-Stencil (Wärmeleitung, Jacobi, 5 Punkte): SYCL-Varianten und CUDA-Gegenstücke
#
# Quellcode liegt je Variante in <Nr>_<Name>/{Sycl,Cuda}/, gebaut wird nach bin/.
#
#   source ~/sycl/env.sh
#   make                                                   -> SYCL, nur CPU (SPIR-V)
#   make SYCL_TARGETS=nvptx64-nvidia-cuda,spir64 EXTRA=--cuda-path=$$CUDA_HOME
#                                                          -> SYCL für NVIDIA-GPU und CPU
#   make cuda                                              -> CUDA (-arch=native)
#   make cuda ARCH="-gencode arch=compute_89,code=sm_89 -gencode arch=compute_90,code=sm_90"
#                                                          -> CUDA für L40S und H100 (DACHS)
#   make ITER=200                                          -> mehr Stencil-Zeitschritte

SYCL_CXX     ?= clang++
ITER         ?= 100
SYCL_TARGETS ?=
NVCC         ?= nvcc
ARCH         ?= -arch=native
BIN          ?= bin

INC        := -I../gemeinsam
SYCL_FLAGS := -fsycl -O3 -std=c++17 -DITER=$(ITER) $(INC)
ifneq ($(SYCL_TARGETS),)
  SYCL_FLAGS += -fsycl-targets=$(SYCL_TARGETS)
endif
SYCL_FLAGS += $(EXTRA)
NVFLAGS    := -O3 -std=c++17 $(ARCH) -DITER=$(ITER) $(INC) $(NVEXTRA)

DEPS   := ../gemeinsam/common.hpp ../gemeinsam/probleme.hpp ../gemeinsam/sycl_rahmen.hpp
CUDEPS := ../gemeinsam/common.hpp ../gemeinsam/probleme.hpp ../gemeinsam/cuda_rahmen.cuh

all: sycl
sycl: $(BIN)/stencil1_2d $(BIN)/stencil1_2d_sync $(BIN)/stencil2_1d $(BIN)/stencil3_lokal
cuda: $(BIN)/cuda_stencil_global $(BIN)/cuda_stencil_shared

$(BIN):
	mkdir -p $(BIN)

$(BIN)/stencil1_2d: 1_2D/Sycl/stencil1_2d.cpp $(DEPS) | $(BIN)
	$(SYCL_CXX) $(SYCL_FLAGS) -o $@ $<

$(BIN)/stencil1_2d_sync: 1_2D/Sycl/stencil1_2d.cpp $(DEPS) | $(BIN)
	$(SYCL_CXX) $(SYCL_FLAGS) -DSYNC_JEDE -o $@ $<

$(BIN)/stencil2_1d: 2_1D/Sycl/stencil2_1d.cpp $(DEPS) | $(BIN)
	$(SYCL_CXX) $(SYCL_FLAGS) -o $@ $<

$(BIN)/stencil3_lokal: 3_Lokal/Sycl/stencil3_lokal.cpp $(DEPS) | $(BIN)
	$(SYCL_CXX) $(SYCL_FLAGS) -o $@ $<

$(BIN)/cuda_stencil_global: 1_2D/Cuda/cuda_stencil_global.cu $(CUDEPS) | $(BIN)
	$(NVCC) $(NVFLAGS) -o $@ $<

$(BIN)/cuda_stencil_shared: 3_Lokal/Cuda/cuda_stencil_shared.cu $(CUDEPS) | $(BIN)
	$(NVCC) $(NVFLAGS) -o $@ $<

clean:
	rm -rf $(BIN)

.PHONY: all sycl cuda clean
