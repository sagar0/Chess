CXX      := c++
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -Iinclude
LDFLAGS  :=

GTEST_CXXFLAGS := -I/opt/homebrew/include
GTEST_LDFLAGS  := -L/opt/homebrew/lib -lgtest

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

SRCS := src/main.cpp $(CORE_LIB_SRCS)
OBJS := $(SRCS:.cpp=.o)
TARGET := chess

TEST_SRCS := tests/test_main.cpp tests/test_board.cpp tests/test_game_store.cpp
TEST_OBJS := $(CORE_LIB_SRCS:.cpp=.o) $(TEST_SRCS:.cpp=.o)
TEST_RUNNER := test_runner

.PHONY: all run test clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^

$(TEST_RUNNER): $(TEST_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(GTEST_LDFLAGS)

test: $(TEST_RUNNER)
	./$(TEST_RUNNER)

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

tests/%.o: tests/%.cpp
	$(CXX) $(CXXFLAGS) $(GTEST_CXXFLAGS) -c -o $@ $<

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) $(TEST_OBJS) $(TEST_RUNNER)
