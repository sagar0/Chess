CXX      := c++
# -fPIC is required so core objects can be linked into the Python shared module on Linux.
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -Iinclude -fPIC
LDFLAGS  :=

UNAME_S := $(shell uname -s)

# GoogleTest: prefer pkg-config (Linux/AL2), fall back to Homebrew on macOS.
GTEST_PKG_CFLAGS := $(shell pkg-config --cflags gtest 2>/dev/null || pkg-config --cflags GTest 2>/dev/null)
GTEST_PKG_LIBS := $(shell pkg-config --libs gtest 2>/dev/null || pkg-config --libs GTest 2>/dev/null)
ifneq ($(GTEST_PKG_CFLAGS),)
GTEST_CXXFLAGS ?= $(GTEST_PKG_CFLAGS)
GTEST_LDFLAGS ?= $(GTEST_PKG_LIBS)
else
GTEST_CXXFLAGS ?= -I/opt/homebrew/include
GTEST_LDFLAGS ?= -L/opt/homebrew/lib -lgtest
endif

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
PYTHON_HEADERS_OK := $(shell $(PYTHON) -c "import os, sysconfig, subprocess, sys; \
paths = []; \
inc = sysconfig.get_config_var('INCLUDEPY'); \
paths.append(inc) if inc else None; \
ver = f'{sys.version_info.major}.{sys.version_info.minor}'; \
paths.append('/Library/Developer/CommandLineTools/Library/Frameworks/Python3.framework/Versions/' + ver + '/Headers'); \
result = subprocess.run([sys.executable, '-m', 'pybind11', '--includes'], capture_output=True, text=True); \
paths.extend(token[2:] for token in result.stdout.split() if token.startswith('-I')); \
print('ok' if any(p and os.path.isfile(os.path.join(p, 'Python.h')) for p in paths) else '')" 2>/dev/null)
PYTHON_EXT_SUFFIX := $(shell $(PYTHON)-config --extension-suffix 2>/dev/null)
ifeq ($(PYTHON_EXT_SUFFIX),)
PYTHON_EXT_SUFFIX := $(shell $(PYTHON) -c "import sysconfig; print(sysconfig.get_config_var('EXT_SUFFIX') or '.so')" 2>/dev/null)
endif
PYTHON_LDFLAGS := $(shell $(PYTHON)-config --ldflags 2>/dev/null)
BINDINGS_TARGET := bindings/$(BINDINGS_MODULE)$(PYTHON_EXT_SUFFIX)
BINDINGS_LDFLAGS := -shared -fPIC
ifeq ($(UNAME_S),Darwin)
BINDINGS_LDFLAGS += -undefined dynamic_lookup
else
BINDINGS_LDFLAGS += $(PYTHON_LDFLAGS)
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

.PHONY: all cli bindings test run clean pybind11-check gtest-check

all: cli

cli: $(TARGET)

bindings: pybind11-check $(BINDINGS_TARGET)

pybind11-check:
	@test -n "$(PYBIND11_INCLUDES)" || \
		(echo "pybind11 not found for $(PYTHON). Install with: $(PYTHON) -m pip install pybind11" && exit 1)
	@test "$(PYTHON_HEADERS_OK)" = "ok" || \
		(echo "Python development headers not found for $(PYTHON)." && \
		 echo "On Amazon Linux: sudo dnf install python3-devel  (or: sudo yum install python3-devel)" && exit 1)

gtest-check:
	@test -n "$(GTEST_PKG_CFLAGS)" || \
		test "$(UNAME_S)" = "Darwin" || \
		(echo "GoogleTest not found via pkg-config. Install gtest-devel:" && \
		 echo "  Amazon Linux: sudo dnf install gtest-devel" && \
		 echo "  Or set GTEST_CXXFLAGS / GTEST_LDFLAGS manually." && exit 1)

$(TARGET): $(CORE_OBJS) $(CLI_OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^

$(BINDINGS_TARGET): $(CORE_OBJS) $(BINDINGS_OBJ)
	$(CXX) $(BINDINGS_LDFLAGS) $(LDFLAGS) -o $@ $^

$(TEST_RUNNER): gtest-check $(TEST_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(TEST_OBJS) $(GTEST_LDFLAGS)

test: $(TEST_RUNNER)
	./$(TEST_RUNNER)

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

cli/%.o: cli/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

bindings/%.o: bindings/%.cpp
	$(CXX) $(CXXFLAGS) $(PYTHON_INCLUDES) -c -o $@ $<

tests/%.o: tests/%.cpp
	$(CXX) $(CXXFLAGS) $(GTEST_CXXFLAGS) -c -o $@ $<

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(CORE_OBJS) $(CLI_OBJ) $(BINDINGS_OBJ) $(TARGET) $(TEST_OBJS) $(TEST_RUNNER)
	rm -f bindings/$(BINDINGS_MODULE).so bindings/$(BINDINGS_MODULE).dylib bindings/$(BINDINGS_MODULE)*.so
