# File being made
FILE := naive-kmeans-parallel

# Compiler
CXX := g++

# Default size
BITS := 64

# Compiler flags
TBB_PATH = /opt/tbb-2021.8.0 
CXXFLAGS = -MMD -ggdb -std=gnu++17 -O3 -m$(BITS) -I/opt/tbb-2021.8.0/include
LDFLAGS	 = -m$(BITS) -lpthread -lrt -L/opt/tbb-2021.8.0/lib64 -ltbb

# Directories
SRC_DIR := src
BIN_DIR := bin
OBJ_DIR  := obj64

# Source files
SRCS := $(SRC_DIR)/$(FILE).cpp

# Object files
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEP_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.d,$(SRCS))

# Targets
TARGET := $(BIN_DIR)/$(FILE)

all: $(TARGET)

# clean up everything by clobbering the output folder
clean:
	@echo cleaning up...
	@rm -rf $(OBJ_DIR) $(BIN_DIR)

# build the bin directory
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJ_FILES) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

-include $(DEP_FILES)