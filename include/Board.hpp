#pragma once

#include "Piece.hpp"
#include "Types.hpp"

#include <array>
#include <memory>

class Board {
public:
    const Piece* pieceAt(Position pos) const;
    bool isEmpty(Position pos) const;
    bool isEnemy(Position pos, Color color) const;
    bool isAlly(Position pos, Color color) const;
    bool isPathClear(Position from, Position to) const;

private:
    std::array<std::array<std::unique_ptr<Piece>, 8>, 8> grid_{};
};
