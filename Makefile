# Compiler
CXX = g++

BITS = 64

# Compiler flags
TBB_PATH = /opt/tbb-2021.8.0 
CXXFLAGS = -MMD -ggdb -O3 -std=gnu++17 -m$(BITS) -I/opt/tbb-2021.8.0/include
LDFLAGS	 = -m$(BITS) -lpthread -lrt -L/opt/tbb-2021.8.0/libtbb.so -ltbb

# Directories
SRCDIR = src
BINDIR = bin
ODIR  = obj64

# Target executable name
TARGET = $(BINDIR)/kmeans-serial

# Source files
SRCS = $(SRCDIR)/kmeans-serial.cpp

# Create the .o names from the CXXFILES
OFILES = $(patsubst %, $(ODIR)/%.o, $(CXXFILES))

# Create .d files to store dependency information, so that we don't need to
# clean every time before running make
DFILES = $(patsubst %.o, %.d, $(OFILES))

# Object files
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

# clean up everything by clobbering the output folder
clean:
	@echo cleaning up...
	@rm -rf $(ODIR)

# build an .o file from a .cc file
$(ODIR)/%.o: %.cc
	@echo [CXX] $< "-->" $@
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

# Link rule for building the target from .o files
$(TARGET): $(OFILES)
	@echo [LD] $^ "-->" $@
	@$(CXX) -o $@ $^ $(LDFLAGS)

.cpp.o:
    $(CXX) $(CXXFLAGS) -c $<  -o $@

# Remember that 'all' and 'clean' aren't real targets
.PHONY: all clean

# Pull in all dependencies
-include $(DFILES)