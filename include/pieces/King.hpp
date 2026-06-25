#pragma once

#include "Piece.hpp"
#include "Types.hpp"

class King : public Piece {
public:
    explicit King(Color color);

    PieceType type() const override { return PieceType::King; }
    char symbol() const override;
    bool isValidMove(const Board& board, Position from, Position to) const override;
};
