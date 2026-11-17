# Zusätzliche Fallbeispiele: 2D-Stencil und Reduktion
#
#   source ~/sycl/env.sh
#   make                                                   -> nur CPU (SPIR-V)
#   make SYCL_TARGETS=nvptx64-nvidia-cuda,spir64 EXTRA=--cuda-path=$CUDA_HOME
#                                                          -> NVIDIA-GPU und CPU
#   make ITER=200                                          -> mehr Stencil-Zeitschritte
#   make -C cuda                                           -> native CUDA-Gegenstücke

SYCL_CXX     ?= clang++   # DPC++ (Open Source); bei Intel oneAPI: icpx
ITER         ?= 100
SYCL_TARGETS ?=
SYCL_FLAGS   := -fsycl -O3 -std=c++17 -DITER=$(ITER)
ifneq ($(SYCL_TARGETS),)
  SYCL_FLAGS += -fsycl-targets=$(SYCL_TARGETS)
endif
SYCL_FLAGS += $(EXTRA)

DEPS := probleme.hpp sycl_rahmen.hpp ../sycl-matmul/common.hpp

STENCIL   := stencil1_2d stencil1_2d_sync stencil2_1d stencil3_lokal
REDUKTION := reduktion1_atomar reduktion2_lokal reduktion3_gruppe reduktion4_builtin
BINS      := $(STENCIL) $(REDUKTION)

all: $(BINS)

stencil1_2d_sync: stencil1_2d.cpp $(DEPS)
	$(SYCL_CXX) $(SYCL_FLAGS) -DSYNC_JEDE -o $@ $<

%: %.cpp $(DEPS)
	$(SYCL_CXX) $(SYCL_FLAGS) -o $@ $<

clean:
	rm -f $(BINS)

.PHONY: all clean
