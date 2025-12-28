# ================================
# Compiler + flags
# ================================
CXX      := g++
CXXFLAGS := -std=gnu++17 -O2 -Wall -Wextra -fopenmp
LDFLAGS  := -fopenmp

# If IVF headers are included like: #include "kmeans.h"
# you probably need:
CXXFLAGS += -I. -Iutils_functions -Iivf


# ================================
# Directories
# ================================
BINDIR := bin

# ================================
# Common sources (shared)
# ================================
COMMON_SRCS := \
	utils_functions/Data_loader.cpp \
	utils_functions/euclid.cpp \
	utils_functions/nearest_neighbor.cpp

COMMON_OBJS := $(COMMON_SRCS:.cpp=.o)

# ================================
# LSH
# ================================
LSH_MAIN := LSHmain.cpp
LSH_SRCS := $(LSH_MAIN) Euclidean_Hashing.cpp
LSH_OBJS := $(LSH_SRCS:.cpp=.o)
LSH_BIN  := $(BINDIR)/LSHmain

# ================================
# Hypercube
# ================================
HC_MAIN := HCmain.cpp
HC_SRCS := $(HC_MAIN) Hypercube.cpp
HC_OBJS := $(HC_SRCS:.cpp=.o)
HC_BIN  := $(BINDIR)/HCmain

# ================================
# IVF (shared between IVFFlat & IVFPQ)
# ================================
IVF_SHARED_SRCS := ivf/kmeans.cpp
IVF_SHARED_OBJS := $(IVF_SHARED_SRCS:.cpp=.o)

# ================================
# IVFFlat
# ================================
IVFFLAT_MAIN := ivf/ivfflat_main.cpp
IVFFLAT_SRCS := $(IVFFLAT_MAIN) ivf/ivf_flat.cpp
IVFFLAT_OBJS := $(IVFFLAT_SRCS:.cpp=.o)
IVFFLAT_BIN  := $(BINDIR)/IVFFlatMain

# ================================
# IVFPQ
# ================================
IVFPQ_MAIN := ivf/ivfpq_main.cpp
IVFPQ_SRCS := $(IVFPQ_MAIN) ivf/ivf_pq.cpp
IVFPQ_OBJS := $(IVFPQ_SRCS:.cpp=.o)
IVFPQ_BIN  := $(BINDIR)/IVFPQMain

# ================================
# Targets
# ================================
.PHONY: all lsh hc ivfflat ivfpq clean

all: lsh hc ivfflat ivfpq

lsh: $(LSH_BIN)
hc:  $(HC_BIN)
ivfflat: $(IVFFLAT_BIN)
ivfpq:   $(IVFPQ_BIN)

# ================================
# Linking
# ================================
$(LSH_BIN): $(LSH_OBJS) $(COMMON_OBJS)
	@mkdir -p $(BINDIR)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(HC_BIN): $(HC_OBJS) $(COMMON_OBJS)
	@mkdir -p $(BINDIR)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(IVFFLAT_BIN): $(IVFFLAT_OBJS) $(IVF_SHARED_OBJS) $(COMMON_OBJS)
	@mkdir -p $(BINDIR)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(IVFPQ_BIN): $(IVFPQ_OBJS) $(IVF_SHARED_OBJS) $(COMMON_OBJS)
	@mkdir -p $(BINDIR)
	$(CXX) $^ -o $@ $(LDFLAGS)

# ================================
# Compilation
# ================================
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ================================
# Clean
# ================================
clean:
	rm -f \
		$(LSH_OBJS) $(HC_OBJS) $(COMMON_OBJS) \
		$(IVFFLAT_OBJS) $(IVFPQ_OBJS) $(IVF_SHARED_OBJS) \
		$(LSH_BIN) $(HC_BIN) $(IVFFLAT_BIN) $(IVFPQ_BIN)
