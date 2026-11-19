# Matrixmultiplikation: alle Stufen (SYCL) und die CUDA-Gegenstücke
#
# Quellcode liegt je Stufe in StufeN/{Normal,Sycl,Cuda}/, gebaut wird nach bin/.
#
#   make                         -> alle SYCL-Stufen (Intel oneAPI icpx)
#   make CXX=$DPCPP_HOME/bin/clang++   -> Open-Source-DPC++
#   make COMPILER=acpp           -> AdaptiveCpp
#   make TILE=32                 -> andere Kachelgröße für Stufe 4
#   make WG=32                   -> andere Work-Group-Größe für Stufe 3
#   make kacheln                 -> Stufe 4 einmal pro Kachelgröße (TILES)
#   make cuda cuda-kacheln       -> CUDA-Gegenstücke + cuBLAS (braucht nvcc)
#   make BIN=/anderer/ordner     -> woanders hin bauen (dachs/alle_gpus.sh)
#
# NVIDIA-GPU mit DPC++:
#   make SYCL_TARGETS=nvptx64-nvidia-cuda,spir64 EXTRA=--cuda-path=$CUDA_HOME
# AMD-GPU mit DPC++, Architektur anpassen:
#   make SYCL_TARGETS=amdgcn-amd-amdhsa EXTRA="-Xsycl-target-backend --offload-arch=gfx942"
# CUDA für L40S und H100 (DACHS, Login-Knoten ohne GPU):
#   make cuda cuda-kacheln ARCH="-gencode arch=compute_89,code=sm_89 -gencode arch=compute_90,code=sm_90"

COMPILER ?= dpcpp
TILE     ?= 16
WG       ?= 16
BLOCK    ?= 16
TILES    ?= 8 16 32 64
SEQ_CXX  ?= g++
NVCC     ?= nvcc
ARCH     ?= -arch=native
BIN      ?= bin

INC := -I../gemeinsam -Igemeinsam

ifeq ($(COMPILER),acpp)
  CXX        := acpp
  SYCL_FLAGS := -O3
else
  # icpx (Intel oneAPI) wenn vorhanden, sonst clang++ aus dem DPC++-Release
  # (so auf DACHS, wo nur das Release aus ~/sycl installiert ist).
  # Übersteuern jederzeit mit:  make CXX=<compiler>
  CXX        := $(shell command -v icpx >/dev/null 2>&1 && echo icpx || echo clang++)
  SYCL_FLAGS := -fsycl -O3
  ifdef SYCL_TARGETS
    SYCL_FLAGS += -fsycl-targets=$(SYCL_TARGETS)
  endif
endif
SYCL_FLAGS += $(INC) $(EXTRA)
NVFLAGS    := -O3 -std=c++17 $(ARCH) $(INC) $(NVEXTRA)

COMMON  := ../gemeinsam/common.hpp
CUCOMMON := $(COMMON) gemeinsam/cuda_common.cuh

SYCL_BINS := stufe0_sequentiell stufe1_usm_shared stufe2_usm_device stufe2b_buffer \
             stufe3_ndrange stufe4_tiling stufe5_anpassung
CUDA_BINS := cuda1_managed cuda2_device cuda4_tiling cuda_cublas

all: $(addprefix $(BIN)/,$(SYCL_BINS))
kacheln: $(addprefix $(BIN)/stufe4_tiling_T,$(TILES))
cuda: $(addprefix $(BIN)/,$(CUDA_BINS))
cuda-kacheln: $(addprefix $(BIN)/cuda4_tiling_T,$(TILES))

$(BIN):
	mkdir -p $(BIN)

# --- SYCL ------------------------------------------------------------------
$(BIN)/stufe0_sequentiell: Stufe0/Normal/stufe0_sequentiell.cpp $(COMMON) | $(BIN)
	$(SEQ_CXX) -O3 -std=c++17 $(INC) -o $@ $<

$(BIN)/stufe1_usm_shared: Stufe1/Sycl/stufe1_usm_shared.cpp $(COMMON) | $(BIN)
	$(CXX) $(SYCL_FLAGS) -o $@ $<

$(BIN)/stufe2_usm_device: Stufe2/Sycl/stufe2_usm_device.cpp $(COMMON) | $(BIN)
	$(CXX) $(SYCL_FLAGS) -o $@ $<

$(BIN)/stufe2b_buffer: Stufe2b/Sycl/stufe2b_buffer.cpp $(COMMON) | $(BIN)
	$(CXX) $(SYCL_FLAGS) -o $@ $<

$(BIN)/stufe3_ndrange: Stufe3/Sycl/stufe3_ndrange.cpp $(COMMON) | $(BIN)
	$(CXX) $(SYCL_FLAGS) -DWGSIZE=$(WG) -o $@ $<

$(BIN)/stufe4_tiling: Stufe4/Sycl/stufe4_tiling.cpp $(COMMON) | $(BIN)
	$(CXX) $(SYCL_FLAGS) -DTILE=$(TILE) -o $@ $<

# Stufe 5 wählt die Kachelgröße (8, 16, 32, 64) erst zur Laufzeit
$(BIN)/stufe5_anpassung: Stufe5/Sycl/stufe5_anpassung.cpp $(COMMON) | $(BIN)
	$(CXX) $(SYCL_FLAGS) -o $@ $<

# Kachelgrößen-Experiment (Performance-Portabilität): stufe4_tiling_T8, _T16, …
$(BIN)/stufe4_tiling_T%: Stufe4/Sycl/stufe4_tiling.cpp $(COMMON) | $(BIN)
	$(CXX) $(SYCL_FLAGS) -DTILE=$* -o $@ $<

# --- CUDA ------------------------------------------------------------------
$(BIN)/cuda1_managed: Stufe1/Cuda/cuda1_managed.cu $(CUCOMMON) | $(BIN)
	$(NVCC) $(NVFLAGS) -DBLOCK=$(BLOCK) -o $@ $<

$(BIN)/cuda2_device: Stufe2/Cuda/cuda2_device.cu $(CUCOMMON) | $(BIN)
	$(NVCC) $(NVFLAGS) -DBLOCK=$(BLOCK) -o $@ $<

$(BIN)/cuda4_tiling: Stufe4/Cuda/cuda4_tiling.cu $(CUCOMMON) | $(BIN)
	$(NVCC) $(NVFLAGS) -DTILE=$(TILE) -o $@ $<

$(BIN)/cuda4_tiling_T%: Stufe4/Cuda/cuda4_tiling.cu $(CUCOMMON) | $(BIN)
	$(NVCC) $(NVFLAGS) -DTILE=$* -o $@ $<

$(BIN)/cuda_cublas: Referenz/cuBLAS/cuda_cublas.cu $(CUCOMMON) | $(BIN)
	$(NVCC) $(NVFLAGS) -o $@ $< -lcublas

clean:
	rm -rf $(BIN)

.PHONY: all kacheln cuda cuda-kacheln clean
