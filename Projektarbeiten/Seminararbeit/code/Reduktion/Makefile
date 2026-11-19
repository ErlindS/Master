# Reduktion (Summe von n Floats): SYCL-Varianten und CUDA-Gegenstücke
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
sycl: $(BIN)/reduktion1_atomar $(BIN)/reduktion2_lokal $(BIN)/reduktion3_gruppe \
      $(BIN)/reduktion4_builtin $(BIN)/reduktion5_gemeinsam $(BIN)/reduktion6_buffer \
      $(BIN)/reduktion6_buffer_wieder $(BIN)/reduktion7_geraet
cuda: $(BIN)/cuda_reduktion_atomar $(BIN)/cuda_reduktion_shared $(BIN)/cuda_reduktion_cub

$(BIN):
	mkdir -p $(BIN)

$(BIN)/reduktion1_atomar: 1_Atomar/Sycl/reduktion1_atomar.cpp $(DEPS) | $(BIN)
	$(SYCL_CXX) $(SYCL_FLAGS) -o $@ $<

$(BIN)/reduktion2_lokal: 2_Lokal/Sycl/reduktion2_lokal.cpp $(DEPS) | $(BIN)
	$(SYCL_CXX) $(SYCL_FLAGS) -o $@ $<

$(BIN)/reduktion3_gruppe: 3_Gruppe/Sycl/reduktion3_gruppe.cpp $(DEPS) | $(BIN)
	$(SYCL_CXX) $(SYCL_FLAGS) -o $@ $<

$(BIN)/reduktion4_builtin: 4_Eingebaut/Sycl/reduktion4_builtin.cpp $(DEPS) | $(BIN)
	$(SYCL_CXX) $(SYCL_FLAGS) -o $@ $<

$(BIN)/reduktion5_gemeinsam: 5_Gemeinsam/Sycl/reduktion5_gemeinsam.cpp $(DEPS) | $(BIN)
	$(SYCL_CXX) $(SYCL_FLAGS) -o $@ $<

$(BIN)/reduktion6_buffer: 6_Buffer/Sycl/reduktion6_buffer.cpp $(DEPS) | $(BIN)
	$(SYCL_CXX) $(SYCL_FLAGS) -o $@ $<

$(BIN)/reduktion6_buffer_wieder: 6_Buffer/Sycl/reduktion6_buffer.cpp $(DEPS) | $(BIN)
	$(SYCL_CXX) $(SYCL_FLAGS) -DBUFFER_WIEDER -o $@ $<

$(BIN)/reduktion7_geraet: 7_Geraet/Sycl/reduktion7_geraet.cpp $(DEPS) | $(BIN)
	$(SYCL_CXX) $(SYCL_FLAGS) -o $@ $<

$(BIN)/cuda_reduktion_atomar: 1_Atomar/Cuda/cuda_reduktion_atomar.cu $(CUDEPS) | $(BIN)
	$(NVCC) $(NVFLAGS) -o $@ $<

$(BIN)/cuda_reduktion_shared: 2_Lokal/Cuda/cuda_reduktion_shared.cu $(CUDEPS) | $(BIN)
	$(NVCC) $(NVFLAGS) -o $@ $<

$(BIN)/cuda_reduktion_cub: 4_Eingebaut/Cuda/cuda_reduktion_cub.cu $(CUDEPS) | $(BIN)
	$(NVCC) $(NVFLAGS) -o $@ $<

clean:
	rm -rf $(BIN)

.PHONY: all sycl cuda clean
