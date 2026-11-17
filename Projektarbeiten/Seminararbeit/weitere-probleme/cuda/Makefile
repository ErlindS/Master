# Native CUDA-Gegenstücke zu Stencil und Reduktion
#
#   make                       -> für die GPU dieses Rechners (-arch=native)
#   make ARCH="-gencode arch=compute_89,code=sm_89 -gencode arch=compute_90,code=sm_90"
#                              -> L40S und H100 (DACHS)

NVCC  ?= nvcc
ARCH  ?= -arch=native
ITER  ?= 100
NVFLAGS := -O3 -std=c++17 $(ARCH) -DITER=$(ITER) $(EXTRA)

DEPS := cuda_rahmen.cuh ../probleme.hpp ../../sycl-matmul/common.hpp
BINS := cuda_stencil_global cuda_stencil_shared \
        cuda_reduktion_atomar cuda_reduktion_shared cuda_reduktion_cub

all: $(BINS)

%: %.cu $(DEPS)
	$(NVCC) $(NVFLAGS) -o $@ $<

clean:
	rm -f $(BINS)

.PHONY: all clean
