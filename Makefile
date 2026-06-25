CXX      := c++
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -Iinclude
LDFLAGS  :=

ifdef DEBUG
CXXFLAGS += -g -O0
else
CXXFLAGS += -O2
endif

SRCS := src/Board.cpp \
        src/pieces/Pawn.cpp src/pieces/Rook.cpp src/pieces/Knight.cpp \
        src/pieces/Bishop.cpp src/pieces/Queen.cpp src/pieces/King.cpp

OBJS := $(SRCS:.cpp=.o)

.PHONY: all run clean

all: $(OBJS)

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

run:
	@echo "Link the chess binary after main.cpp is added."

clean:
	rm -f $(OBJS)
