# compiler + flags
CXX      := g++
CXXFLAGS := -std=gnu++17 -O2 -Wall -Wextra -fopenmp
LDFLAGS  = -fopenmp

TARGET := LSHmain
SRCS := LSHmain.cpp Euclidean_Hashing.cpp
SRCS += utils_functions/Data_loader.cpp utils_functions/euclid.cpp
OBJS   := $(SRCS:.cpp=.o)


# default rule
all: $(TARGET)

# link
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)


clean:
	rm -f $(OBJS) $(TARGET)