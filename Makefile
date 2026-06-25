CXX      := c++
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -Iinclude
LDFLAGS  :=

ifdef DEBUG
CXXFLAGS += -g -O0
else
CXXFLAGS += -O2
endif

SRCS := src/main.cpp src/Board.cpp \
        src/pieces/Pawn.cpp src/pieces/Rook.cpp src/pieces/Knight.cpp \
        src/pieces/Bishop.cpp src/pieces/Queen.cpp src/pieces/King.cpp

OBJS := $(SRCS:.cpp=.o)
TARGET := chess

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
