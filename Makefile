CXX      := c++
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -Iinclude
LDFLAGS  :=

GTEST_CXXFLAGS := -I/opt/homebrew/include
GTEST_LDFLAGS  := -L/opt/homebrew/lib -lgtest

PYTHON       ?= python3
BINDINGS_MODULE := chess_engine
PYBIND11_INCLUDES := $(shell $(PYTHON) -m pybind11 --includes 2>/dev/null)
PYTHON_HEADER_INCLUDE := $(shell $(PYTHON) -c "import sys, sysconfig, os; \
ver = f'{sys.version_info.major}.{sys.version_info.minor}'; \
candidates = [sysconfig.get_config_var('INCLUDEPY'), \
'/Library/Developer/CommandLineTools/Library/Frameworks/Python3.framework/Versions/' + ver + '/Headers']; \
found = next((p for p in candidates if p and os.path.isfile(os.path.join(p, 'Python.h'))), ''); \
print('-I' + found if found else '')" 2>/dev/null)
PYTHON_INCLUDES := $(PYBIND11_INCLUDES) $(PYTHON_HEADER_INCLUDE)
PYTHON_EXT_SUFFIX := $(shell $(PYTHON)-config --extension-suffix 2>/dev/null)
ifeq ($(PYTHON_EXT_SUFFIX),)
PYTHON_EXT_SUFFIX := $(shell $(PYTHON) -c "import sysconfig; print(sysconfig.get_config_var('EXT_SUFFIX') or '.so')" 2>/dev/null)
endif
BINDINGS_TARGET := bindings/$(BINDINGS_MODULE)$(PYTHON_EXT_SUFFIX)
BINDINGS_LDFLAGS := -shared -fPIC
ifeq ($(shell uname -s),Darwin)
BINDINGS_LDFLAGS += -undefined dynamic_lookup
endif

ifdef DEBUG
CXXFLAGS += -g -O0
else
CXXFLAGS += -O2
endif

CORE_LIB_SRCS := src/Board.cpp \
                 src/GameSnapshot.cpp \
                 src/FileGameStore.cpp \
                 src/pieces/Pawn.cpp src/pieces/Rook.cpp src/pieces/Knight.cpp \
                 src/pieces/Bishop.cpp src/pieces/Queen.cpp src/pieces/King.cpp

CORE_OBJS := $(CORE_LIB_SRCS:.cpp=.o)

CLI_SRC := cli/main_cli.cpp
CLI_OBJ := cli/main_cli.o
TARGET := chess

BINDINGS_SRC := bindings/python_bindings.cpp
BINDINGS_OBJ := bindings/python_bindings.o

TEST_SRCS := tests/test_main.cpp tests/test_board.cpp tests/test_game_store.cpp
TEST_OBJS := $(CORE_OBJS) $(TEST_SRCS:.cpp=.o)
TEST_RUNNER := test_runner

.PHONY: all cli bindings test run clean pybind11-check

all: cli

cli: $(TARGET)

bindings: pybind11-check $(BINDINGS_TARGET)

pybind11-check:
	@test -n "$(PYBIND11_INCLUDES)" || \
		(echo "pybind11 not found for $(PYTHON). Install with: $(PYTHON) -m pip install pybind11" && exit 1)
	@test -n "$(PYTHON_HEADER_INCLUDE)" || \
		(echo "Python development headers not found for $(PYTHON)." && exit 1)

$(TARGET): $(CORE_OBJS) $(CLI_OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^

$(BINDINGS_TARGET): $(CORE_OBJS) $(BINDINGS_OBJ)
	$(CXX) $(BINDINGS_LDFLAGS) $(LDFLAGS) -o $@ $^

$(TEST_RUNNER): $(TEST_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(GTEST_LDFLAGS)

test: $(TEST_RUNNER)
	./$(TEST_RUNNER)

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

cli/%.o: cli/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

bindings/%.o: bindings/%.cpp
	$(CXX) $(CXXFLAGS) $(PYTHON_INCLUDES) -fPIC -c -o $@ $<

tests/%.o: tests/%.cpp
	$(CXX) $(CXXFLAGS) $(GTEST_CXXFLAGS) -c -o $@ $<

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(CORE_OBJS) $(CLI_OBJ) $(BINDINGS_OBJ) $(TARGET) $(TEST_OBJS) $(TEST_RUNNER)
	rm -f bindings/$(BINDINGS_MODULE).so bindings/$(BINDINGS_MODULE).dylib bindings/$(BINDINGS_MODULE)*.so
