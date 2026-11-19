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
#   make COMPILER=acpp EXTRA=--acpp-targets=generic         -> AdaptiveCpp (z. B. AMD MI300A)
#   make ITER=200                                          -> mehr Stencil-Zeitschritte

SYCL_CXX     ?= clang++
ITER         ?= 100
SYCL_TARGETS ?=
NVCC         ?= nvcc
ARCH         ?= -arch=native
BIN          ?= bin

INC        := -I../gemeinsam
ifeq ($(COMPILER),acpp)
  # AdaptiveCpp: Ziel über EXTRA=--acpp-targets=... oder ACPP_TARGETS
  SYCL_CXX   := acpp
  SYCL_FLAGS := -O3 -std=c++17 -DITER=$(ITER) $(INC)
else
  SYCL_FLAGS := -fsycl -O3 -std=c++17 -DITER=$(ITER) $(INC)
  ifneq ($(SYCL_TARGETS),)
    SYCL_FLAGS += -fsycl-targets=$(SYCL_TARGETS)
  endif
endif
SYCL_FLAGS += $(EXTRA)
NVFLAGS    := -O3 -std=c++17 $(ARCH) -DITER=$(ITER) $(INC) $(NVEXTRA)

DEPS   := ../gemeinsam/common.hpp ../gemeinsam/probleme.hpp ../gemeinsam/sycl_rahmen.hpp
CUDEPS := ../gemeinsam/common.hpp ../gemeinsam/probleme.hpp ../gemeinsam/cuda_rahmen.cuh

all: sycl
sycl: $(BIN)/stencil1_2d $(BIN)/stencil1_2d_sync $(BIN)/stencil2_1d $(BIN)/stencil3_lokal \
      $(BIN)/stencil4_gemeinsam $(BIN)/stencil5_buffer $(BIN)/stencil5_buffer_wieder \
      $(BIN)/stencil6_gruppe $(BIN)/stencil7_geraet $(BIN)/stencil7_geraet_flach
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

$(BIN)/stencil4_gemeinsam: 4_Gemeinsam/Sycl/stencil4_gemeinsam.cpp $(DEPS) | $(BIN)
	$(SYCL_CXX) $(SYCL_FLAGS) -o $@ $<

$(BIN)/stencil5_buffer: 5_Buffer/Sycl/stencil5_buffer.cpp $(DEPS) | $(BIN)
	$(SYCL_CXX) $(SYCL_FLAGS) -o $@ $<

$(BIN)/stencil5_buffer_wieder: 5_Buffer/Sycl/stencil5_buffer.cpp $(DEPS) | $(BIN)
	$(SYCL_CXX) $(SYCL_FLAGS) -DBUFFER_WIEDER -o $@ $<

$(BIN)/stencil6_gruppe: 6_Gruppe/Sycl/stencil6_gruppe.cpp $(DEPS) | $(BIN)
	$(SYCL_CXX) $(SYCL_FLAGS) -o $@ $<

$(BIN)/stencil7_geraet: 7_Geraet/Sycl/stencil7_geraet.cpp $(DEPS) | $(BIN)
	$(SYCL_CXX) $(SYCL_FLAGS) -o $@ $<

$(BIN)/stencil7_geraet_flach: 7_Geraet/Sycl/stencil7_geraet.cpp $(DEPS) | $(BIN)
	$(SYCL_CXX) $(SYCL_FLAGS) -DLADEN_FLACH -o $@ $<

$(BIN)/cuda_stencil_global: 1_2D/Cuda/cuda_stencil_global.cu $(CUDEPS) | $(BIN)
	$(NVCC) $(NVFLAGS) -o $@ $<

$(BIN)/cuda_stencil_shared: 3_Lokal/Cuda/cuda_stencil_shared.cu $(CUDEPS) | $(BIN)
	$(NVCC) $(NVFLAGS) -o $@ $<

clean:
	rm -rf $(BIN)

.PHONY: all sycl cuda clean
