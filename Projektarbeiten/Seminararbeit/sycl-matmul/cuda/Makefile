# Native CUDA-Vergleichsprogramme
#
#   make                       -> für die GPU dieses Rechners (-arch=native)
#   make ARCH="-gencode arch=compute_89,code=sm_89 -gencode arch=compute_90,code=sm_90"
#                              -> L40S (sm_89) und H100 (sm_90), z. B. auf DACHS
#   make BLOCK=32              -> andere Blockgröße für cuda1/cuda2
#   make kacheln               -> cuda4_tiling_T8 … _T64 (TILES)

NVCC  ?= nvcc
ARCH  ?= -arch=native
BLOCK ?= 16
TILE  ?= 16
TILES ?= 8 16 32 64
NVFLAGS := -O3 -std=c++17 $(ARCH) $(EXTRA)

BINS := cuda1_managed cuda2_device cuda4_tiling cuda_cublas

all: $(BINS)

cuda1_managed: cuda1_managed.cu cuda_common.cuh ../common.hpp
	$(NVCC) $(NVFLAGS) -DBLOCK=$(BLOCK) -o $@ $<

cuda2_device: cuda2_device.cu cuda_common.cuh ../common.hpp
	$(NVCC) $(NVFLAGS) -DBLOCK=$(BLOCK) -o $@ $<

cuda4_tiling: cuda4_tiling.cu cuda_common.cuh ../common.hpp
	$(NVCC) $(NVFLAGS) -DTILE=$(TILE) -o $@ $<

cuda_cublas: cuda_cublas.cu cuda_common.cuh ../common.hpp
	$(NVCC) $(NVFLAGS) -o $@ $< -lcublas

kacheln: $(addprefix cuda4_tiling_T,$(TILES))

cuda4_tiling_T%: cuda4_tiling.cu cuda_common.cuh ../common.hpp
	$(NVCC) $(NVFLAGS) -DTILE=$* -o $@ $<

clean:
	rm -f $(BINS) cuda4_tiling_T*

.PHONY: all clean kacheln
