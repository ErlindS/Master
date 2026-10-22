# Build für alle Stufen
#
#   make                         -> Intel oneAPI DPC++ (icpx)
#   make COMPILER=acpp           -> AdaptiveCpp
#   make TILE=32                 -> andere Kachelgröße für Stufe 3
#   make kacheln                 -> Stufe 3 einmal pro Kachelgröße (TILES)
#
# NVIDIA-GPU mit DPC++ (braucht das CUDA-Plugin für oneAPI):
#   make SYCL_TARGETS=nvptx64-nvidia-cuda
# AMD-GPU mit DPC++ (braucht das ROCm-Plugin für oneAPI), Architektur anpassen:
#   make SYCL_TARGETS=amdgcn-amd-amdhsa EXTRA="-Xsycl-target-backend --offload-arch=gfx1030"

COMPILER ?= dpcpp
TILE     ?= 16
TILES    ?= 8 16 32 64
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

stufe1_usm_shared: stufe1_usm_shared.cpp common.hpp
	$(CXX) $(SYCL_FLAGS) -o $@ $<

stufe2_usm_device: stufe2_usm_device.cpp common.hpp
	$(CXX) $(SYCL_FLAGS) -o $@ $<

stufe3_tiling: stufe3_tiling.cpp common.hpp
	$(CXX) $(SYCL_FLAGS) -DTILE=$(TILE) -o $@ $<

# Kachelgrößen-Experiment (Performance-Portabilität): stufe3_tiling_T8, _T16, …
kacheln: $(addprefix stufe3_tiling_T,$(TILES))

stufe3_tiling_T%: stufe3_tiling.cpp common.hpp
	$(CXX) $(SYCL_FLAGS) -DTILE=$* -o $@ $<

clean:
	rm -f $(BINS) stufe3_tiling_T*

.PHONY: all clean kacheln
