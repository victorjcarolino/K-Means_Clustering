CXX ?= c++
CXXFLAGS ?= -O3 -std=c++17
TBB_CFLAGS ?= $(shell pkg-config --cflags tbb 2>/dev/null)
TBB_LIBS ?= $(shell pkg-config --libs tbb 2>/dev/null || echo -ltbb)
PROGRAMS := kmeans-serial better-kmeans-serial kmeans-parallel
TARGETS := $(addprefix bin/,$(PROGRAMS))

.PHONY: all test clean
all: $(TARGETS)

bin:
	mkdir -p $@

bin/%: src/%.cpp | bin
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(TBB_CFLAGS) $< -o $@ $(LDFLAGS) $(TBB_LIBS)

test: all
	python3 tests/regression.py

clean:
	$(RM) $(TARGETS)
