# ================================
# Compiler + flags
# ================================
CXX      := g++
CXXFLAGS := -std=gnu++17 -O2 -Wall -Wextra -fopenmp
LDFLAGS  := -fopenmp

# ================================
# Directories
# ================================
BINDIR := bin

# ================================
# Common sources (shared)
# ================================
COMMON_SRCS := \
	utils_functions/Data_loader.cpp \
	utils_functions/euclid.cpp

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
# Targets
# ================================
.PHONY: all lsh hc clean

all: lsh hc

lsh: $(LSH_BIN)
hc:  $(HC_BIN)

# ================================
# Linking
# ================================
$(LSH_BIN): $(LSH_OBJS) $(COMMON_OBJS)
	@mkdir -p $(BINDIR)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(HC_BIN): $(HC_OBJS) $(COMMON_OBJS)
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
		$(LSH_OBJS) \
		$(HC_OBJS) \
		$(COMMON_OBJS) \
		$(LSH_BIN) \
		$(HC_BIN)
