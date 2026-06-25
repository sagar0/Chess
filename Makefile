CXX      := c++
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -Iinclude
LDFLAGS  :=

ifdef DEBUG
CXXFLAGS += -g -O0
else
CXXFLAGS += -O2
endif

.PHONY: all run clean compile-check

all: compile-check

compile-check: include/Types.hpp
	@printf '#include "Types.hpp"\nint main(){Position p{};(void)p;return 0;}\n' | \
		$(CXX) $(CXXFLAGS) -x c++ -c - -o .types-smoke.o
	@rm -f .types-smoke.o

run:
	@echo "Chess binary not built yet. Continue the commit sequence."

clean:
	@rm -f .types-smoke.o
