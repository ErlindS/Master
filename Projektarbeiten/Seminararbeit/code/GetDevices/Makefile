# Build für alle Stufen
#
#   make                         -> Intel oneAPI DPC++ (icpx)
#   make COMPILER=acpp           -> AdaptiveCpp
#   make TILE=32                 -> andere Kachelgröße für Stufe 3
#
# NVIDIA-GPU mit DPC++ (braucht das CUDA-Plugin für oneAPI):
#   make SYCL_TARGETS=nvptx64-nvidia-cuda
# AMD-GPU mit DPC++ (braucht das ROCm-Plugin für oneAPI), Architektur anpassen:
#   make SYCL_TARGETS=amdgcn-amd-amdhsa EXTRA="-Xsycl-target-backend --offload-arch=gfx1030"

COMPILER ?= dpcpp
TILE     ?= 16
SEQ_CXX  ?= g++

ifeq ($(COMPILER),acpp)
  CXX        := acpp
  SYCL_FLAGS := -O3
else
  CXX        := icpx
  SYCL_FLAGS := -fsycl -O3
  ifdef SYCL_TARGETS
    SYCL_FLAGS += -fsycl-targets=$(SYCL_TARGETS)
  endif
endif

SYCL_FLAGS += $(EXTRA)

BINS := stufe0_sequentiell stufe1_usm_shared stufe2_usm_device stufe3_tiling

all: $(BINS)

stufe0_sequentiell: stufe0_sequentiell.cpp common.hpp
	$(SEQ_CXX) -O3 -std=c++17 -o $@ $<

clean:
	rm -f $(BINS)

.PHONY: all clean
