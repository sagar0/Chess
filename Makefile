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

compile-check:
	@echo "Project layout and build configuration ready."

run:
	@echo "Chess binary not built yet. Continue the commit sequence."

clean:
	@echo "Nothing to clean."
